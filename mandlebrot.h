#ifndef LIGHTS_MANDLEBROT_H
#define LIGHTS_MANDLEBROT_H
#include "text.h"
//#define DOUBLE_PRECISION
//PARTICLES

#define MAX_DIMENSION _max(MATRIX_HEIGHT,MATRIX_WIDTH)
#define MIN_DIMENSION _min(MATRIX_HEIGHT,MATRIX_WIDTH)

class MANDLEBROT: public LIGHT_SKETCH {
  public:
    MANDLEBROT () {setup();}
    ~MANDLEBROT () {}
  private:
    #define NUM_MANDLEBROT_EFFECTS 2
    int current_effect = 0;
    bool zoom_in = 0;
    bool zoom_out = 0;
    bool move_up = 0;
    bool move_down = 0;
    bool move_left = 0;
    bool move_right = 0;

#ifdef DOUBLE_PRECISION
    double scale = 1;
    double offset_x = 0;
    double offset_y = 0;
#else
    float scale = 1;
    float offset_x = 0;
    float offset_y = 0;
#endif

    uint32_t sample_cnt = 0;

    struct PIXEL {
        uint32_t ival = 0;
        uint16_t samples_taken = 0;
        uint8_t samples_remaining = 1;
        uint8_t done = 0;
    };

    PIXEL pixels[NUM_LEDS];
    bool reset_pixels = 0;
    bool was_moving = 0;
    uint16_t iterations = 64;
    uint16_t old_iterations = 64;
    uint16_t layer_frequency = 256;
    uint32_t user_color_shift = 0;
    uint32_t old_user_color_shift = 0;
    uint16_t color_shift = 0;
    bool auto_color_shift = 0;
    int16_t auto_color_shift_speed = 0;
    uint8_t layer_tightness = 20;
    CRGB color1 = CRGB(255,0,0);
    CRGB color2 = CRGB(0,255,0);
    CRGB color3 = CRGB(0,0,255);


  public:
    void reset() {
    }

    void setup() {
        // control_variables.add(sharpen_amount, "Sharpen", 0, 9);
        // control_variables.add(threshold, "Threshold", 1, 255);
        // control_variables.add(current_font, "Font", 1, NUM_FONTS);
        // control_variables.add(font_scale, "Scale", 1, 256);
        // control_variables.add(text, "Text");
        control_variables.add(zoom_in, "Zoom In", 1, 'q'-32); 
        control_variables.add(zoom_out, "Zoom Out", 1, 'a'-32); 
        control_variables.add(move_up,"Up", 1, 38);
        control_variables.add(move_left,"Left", 1, 37);
        control_variables.add(move_right,"Right", 1, 39);
        control_variables.add(move_down,"Down", 1, 40);
        control_variables.add(iterations,"Iterations", 1, 2048);
        control_variables.add(layer_frequency,"Layer Frequency", 1, 4096);
        control_variables.add(user_color_shift,"Color Shift", 0, 512);
        control_variables.add(auto_color_shift,"Auto Shift");
        control_variables.add(auto_color_shift_speed,"Color Shift Speed", -256, 256);
        control_variables.add(layer_tightness,"Layer Tightness", 0, 254);
        control_variables.add(color1,"Color 1");
        control_variables.add(color2,"Color 2");
        control_variables.add(color3,"Color 3");
        for (int i = 0; i < NUM_LEDS; i++) {
            pixels[i].samples_remaining = 1;
        }
        
    }

    void next_effect() {
        current_effect++;
        current_effect %= NUM_MANDLEBROT_EFFECTS;
    }

    void loop() {
        static uint8_t old_leds[NUM_LEDS];
        if (current_effect) {
            for(int i = 0; i < NUM_LEDS; i++) {
                if (leds[i].b > old_leds[i]) {
                    leds[i].g = 255;
                }
                if (leds[i].b < old_leds[i]) {
                    leds[i].r = 255;
                }
            }
        }
        LED_show();
        
        for(int i = 0; i < NUM_LEDS; i++) {
            old_leds[i] = leds[i].r + leds[i].g + leds[i].b;
            //optimization, don't re-render black pixels when zooming/panning
            //if(old_leds[i] == 0 && reset_pixels == 1) {
            //    samples_remaining[i] = 0;
            //}
        }
        
        LED_black();

        switch (current_effect) {
            case 0:
                handle_mandlebrot();
                break;
            case 1:
                handle_mandlebrot();
                break;
            case 2:
                break;
            default:
                break;

        }
    }

    void handle_mandlebrot() {
        //
        //  pseudocode from Wikipedia:
        //  https://en.wikipedia.org/wiki/Mandelbrot_set
        //
        // for each pixel (Px, Py) on the screen do
        //     x0 = scaled x coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.5, 1))
        //     y0 = scaled y coordinate of pixel (scaled to lie in the Mandelbrot Y scale (-1, 1))
        //     x := 0.0
        //     y := 0.0
        //     iteration := 0
        //     max_iteration := 1000
        //     while (x×x + y×y ≤ 2×2 AND iteration < max_iteration) do
        //         xtemp := x×x - y×y + x0
        //         y := 2×x×y + y0
        //         x := xtemp
        //         iteration := iteration + 1
        
        //     color := palette[iteration]
        //     plot(Px, Py, color)
        static uint32_t stop_time = millis();
        uint16_t elapsed_time = millis() - stop_time + 16;
        uint16_t elapsed_frames = elapsed_time/16;
        

#ifdef DOUBLE_PRECISION
        const double x_pitch = 3.5/MIN_DIMENSION;
        const double y_pitch = 3.5/MIN_DIMENSION;
#else
        const float x_pitch = 3.5f/MIN_DIMENSION;
        const float y_pitch = 3.5f/MIN_DIMENSION;
#endif

        stop_time = millis() + 16; //limit fps
        if (auto_color_shift) {
            user_color_shift+=auto_color_shift_speed;
        }
        if (user_color_shift != old_user_color_shift) {
            color_shift = user_color_shift;
            
        } 
        old_user_color_shift = user_color_shift;
        while (millis() < stop_time && sample_cnt < UINT32_MAX/iterations) {
#ifdef DOUBLE_PRECISION
            double x_rand = 0;
            double y_rand = 0;
#else
            float x_rand = 0;
            float y_rand = 0;
#endif

            if (sample_cnt > 1) {
                x_rand = ((random(INT32_MAX)-INT32_MAX/2)*x_pitch*2)/INT32_MAX;
                y_rand = ((random(INT32_MAX)-INT32_MAX/2)*y_pitch*2)/INT32_MAX;
            }
            const int largest_dimension = msb(_min(MATRIX_HEIGHT,MATRIX_WIDTH));
            const int initial_jump = largest_dimension*2;

            static int jump_x = initial_jump;
            static int jump_y = initial_jump;

            if (reset_pixels) {
                jump_x = initial_jump;
                jump_y = initial_jump;
                for (int i = 0; i < NUM_LEDS; i++) {
                    pixels[i].done = 0;
                }
            }

            while (millis() < stop_time) {
                static int Py = (jump_y/2)%largest_dimension;

                if (Py >= MATRIX_HEIGHT) {
                    if (jump_x > jump_y) {
                        jump_x /= 2;
                    } else {
                        jump_y /= 2;
                        jump_x = initial_jump;
                    }

                    if (!jump_y) {
                        jump_y = initial_jump;
                        jump_x = initial_jump;
                        for (int i = 0; i < NUM_LEDS; i++) {
                            pixels[i].done = 0;
                        }
                    }
                    Py = (jump_y/2)%largest_dimension;
                }

                int led = XY((jump_x/2)%largest_dimension,Py) - jump_x;
                for (int Px = (jump_x/2)%largest_dimension; Px < MATRIX_WIDTH; Px+=jump_x) {
                    led+=jump_x;
                    if(pixels[led].done || pixels[led].samples_remaining==0 || pixels[led].samples_taken==65535) continue;
                    pixels[led].done = 1;
                    //calculate our position
#ifdef DOUBLE_PRECISION
                    double x0 = (Px+MIN_DIMENSION/2-MATRIX_WIDTH/2)*x_pitch + x_rand - 1.75;
                    double y0 = (Py+MIN_DIMENSION/2-MATRIX_HEIGHT/2)*y_pitch + y_rand - 1.75;
#else
                    float x0 = (Px+MIN_DIMENSION/2-MATRIX_WIDTH/2)*x_pitch + x_rand - 1.75f;
                    float y0 = (Py+MIN_DIMENSION/2-MATRIX_HEIGHT/2)*y_pitch + y_rand - 1.75f;
#endif
                    //scale our position
                    x0 *= scale;
                    y0 *= scale;

                    //center our x position (Mandlebrot set is -2.5 to 1)
                    x0 -= .75f;

                    //add any amount that we have moved
                    x0 += offset_x;
                    y0 += offset_y;
#ifdef DOUBLE_PRECISION
                    double x = 0;
                    double y = 0;
#else
                    float x = 0;
                    float y = 0;
#endif
                    uint16_t iteration = iterations;
                    while (x*x + y*y <= 2*2 && --iteration) {
#ifdef DOUBLE_PRECISION
                        double xtemp = x*x - y*y + x0;
#else
                        float xtemp = x*x - y*y + x0;
#endif
                        y = 2*x*y + y0;
                        x = xtemp;
                    }
                    // int last_avg = 0;
                    // if(pixels[led].samples_taken) {
                    //     last_avg = pixels[led].ival/pixels[led].samples_taken;
                    // }
                    pixels[led].ival += iteration;
                    pixels[led].samples_taken++;
                    //int avg = pixels[led].ival/pixels[led].samples_taken;
                    if(pixels[led].ival == 0) {
                    //if(last_avg == avg) {
                        //optimization for two possible reasons:
                        // 1) we reached max iteration
                        // 2) our average sample value did not change
                        //don't sample this pixel or adjacent pixels next time
                        pixels[led].samples_remaining = 0;
                        // if (led > 0) {
                        //     pixels[led-1].samples_remaining = 0;
                        //     if (led >= MATRIX_WIDTH) {
                        //         pixels[led-MATRIX_WIDTH].samples_remaining = 0;
                        //     }
                        // }
                        // if (led < MATRIX_WIDTH-1) {
                        //     pixels[led+1].samples_remaining = 0;
                        //     if (led < (MATRIX_WIDTH*MATRIX_HEIGHT)-MATRIX_WIDTH) {
                        //         pixels[led+MATRIX_WIDTH].samples_remaining = 0;
                        //     }
                        // }
                    } else {
                        //do sample this pixel and adjacent pixels next time
                        if (led > 0) {
                            pixels[led-1].samples_remaining = 1;
                            if (led >= MATRIX_WIDTH) {
                                pixels[led-MATRIX_WIDTH].samples_remaining = 1;
                            }
                        }
                        if (led < MATRIX_WIDTH-1) {
                            pixels[led+1].samples_remaining = 1;
                            if (led < (MATRIX_WIDTH*MATRIX_HEIGHT)-MATRIX_WIDTH) {
                                pixels[led+MATRIX_WIDTH].samples_remaining = 1;
                            }
                        }
                    }
                }
                Py += jump_y;
            }
            sample_cnt++;
        }

        //draw our accumulated data to the screen
        bool use_top = 0;
        bool use_left = 0;
        for (int i = 0; i < NUM_LEDS; i++) {
            if ( !(i%MATRIX_WIDTH) ) {
                if (pixels[i].samples_taken) {
                    use_left = 1;
                    use_top = 0;
                } else if (i >= MATRIX_WIDTH) {
                    use_left = 0;
                    use_top = 1;
                }
            }
            if(!pixels[i].samples_taken) {
                if (use_top) {
                    leds[i] = leds[i-MATRIX_WIDTH];
                } else if (use_left) {
                    leds[i] = leds[i-1];
                }
                // else if (pixels[i-MATRIX_WIDTH].samples_taken) {
                //    leds[i] = leds[i-MATRIX_WIDTH];
                // }
                continue;
            }
            if(!pixels[i].ival) {
                continue;
            }
            uint32_t temp0 = pixels[i].ival;
            temp0/=pixels[i].samples_taken;
            uint16_t depth = temp0%=iterations; //save this for later
            temp0 = iterations - temp0;
            temp0+=color_shift;
            uint16_t iter = layer_frequency;
            temp0%=iter;

            int16_t temp = temp0;
            uint16_t c1a = abs(temp - (iter/6));
            uint16_t c1b = abs(temp+iter - (iter/6));
            uint16_t c1c = abs(temp-iter - (iter/6));
            uint16_t c1 = _min(c1a,c1b);
            c1 = _min(c1,c1c);
            uint16_t c2a = abs(temp - (iter/2));
            uint16_t c2b = abs(temp+iter - (iter/2));
            uint16_t c2c = abs(temp-iter - (iter/2));
            uint16_t c2 = _min(c2a,c2b);
            c2 = _min(c2,c2c);
            uint16_t c3a = abs(temp - (iter - iter/6));
            uint16_t c3b = abs(temp+iter - (iter - iter/6));
            uint16_t c3c = abs(temp-iter - (iter - iter/6));
            uint16_t c3 = _min(c3a,c3b);
            c3 = _min(c3,c3c);


            depth = iterations - depth;
            depth = (depth*depth)/iterations;
            depth = iterations - depth;
            if (c1 < layer_tightness) {
                c1 = layer_tightness - c1;
                nblend(leds[i],color1,(c1*255*depth)/(layer_tightness*iterations));
            }
            if (c2 < layer_tightness) {
                c2 = layer_tightness - c2;
                nblend(leds[i],color2,(c2*255*depth)/(layer_tightness*iterations));
            }
            if (c3 < layer_tightness) {
                c3 = layer_tightness - c3;
                nblend(leds[i],color3,(c3*255*depth)/(layer_tightness*iterations));
            }
        }


        reset_pixels = 0;
        //movement via key input
        if (zoom_in) {
            uint16_t cnt = elapsed_frames;
            while (cnt) {
                scale *= .98f;
                cnt--;
            }

            reset_pixels = 1;
        }
        if (zoom_out) {
            uint16_t cnt = elapsed_frames;
            while (cnt) {
                scale *= 1.02f;
                cnt--;
            }
            reset_pixels = 1;
        }
        if (move_up) {
            uint16_t cnt = elapsed_frames;
            while (cnt) {
                offset_y += .025f*scale*(MAX_DIMENSION/MIN_DIMENSION);
                cnt--;
            }
            reset_pixels = 1;
            
        }
        if (move_down) {
            uint16_t cnt = elapsed_frames;
            while (cnt) {
                offset_y -= .025f*scale*(MAX_DIMENSION/MIN_DIMENSION);
                cnt--;
            }
            reset_pixels = 1;
            
        }
        if (move_right) {
            uint16_t cnt = elapsed_frames;
            while (cnt) {
                offset_x += .025f*scale*(MAX_DIMENSION/MIN_DIMENSION);
                cnt--;
            }
            reset_pixels = 1;
            
        }
        if (move_left) {
            uint16_t cnt = elapsed_frames;
            while (cnt) {
                offset_x -= .025f*scale*(MAX_DIMENSION/MIN_DIMENSION);
                cnt--;
            }
            reset_pixels = 1;
            
        }


        

        //reset if zoom or position or number of iterations changes
        if (old_iterations != iterations) {
            reset_pixels=1;
        }
        if (reset_pixels) {
            was_moving = 1;
            sample_cnt = 0;
            for (int i = 0; i < NUM_LEDS; i++) {
                pixels[i].samples_remaining = 1;
                pixels[i].samples_taken = 0;
                pixels[i].ival = 0;
            }
        } else if (was_moving == 1) {
            was_moving = 0;
            for (int i = 0; i < NUM_LEDS; i++) {
                pixels[i].samples_remaining = 1;
            }
        }

        old_iterations = iterations;

    }

};

LIGHT_SKETCHES::REGISTER<MANDLEBROT> mandlebrot("mandlebrot");

#endif