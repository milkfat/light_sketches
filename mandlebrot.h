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
    uint32_t Ivals[NUM_LEDS] = {0};
    uint16_t samples_taken[NUM_LEDS] = {0};
    uint8_t samples_remaining[NUM_LEDS] = {1};
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
        control_variables.add(layer_frequency,"Layer Frequency", 1, 256);
        control_variables.add(user_color_shift,"Color Shift", 0, 512);
        control_variables.add(auto_color_shift,"Auto Shift");
        control_variables.add(auto_color_shift_speed,"Color Shift Speed", -256, 256);
        control_variables.add(layer_tightness,"Layer Tightness", 0, 254);
        control_variables.add(color1,"Color 1");
        control_variables.add(color2,"Color 2");
        control_variables.add(color3,"Color 3");
        for (int i = 0; i < NUM_LEDS; i++) {
            samples_remaining[i] = 1;
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
            static int start_line = MATRIX_WIDTH/2;
            static int line = MATRIX_WIDTH/2;
            static int jump = MATRIX_WIDTH;
            if (reset_pixels) {
                start_line = MATRIX_WIDTH/2;
                line = MATRIX_WIDTH/2;
                jump = MATRIX_WIDTH;
            }
            while (millis() < stop_time) {
                int Px = line;
                line += jump;
                if (line > MATRIX_WIDTH-1) {
                    line%=MATRIX_WIDTH;
                    jump/=2;
                    line-=jump/2;
                    if (jump == start_line) {
                        start_line /= 2;
                        line = start_line;
                        jump = MATRIX_WIDTH/2;

                        if (jump == 1) {
                            start_line = MATRIX_WIDTH/2;
                            line = MATRIX_WIDTH/2;
                            jump = MATRIX_WIDTH;
                        }
                    }
                    Px = line;
                }
                for (int Py = 0; Py < MATRIX_HEIGHT; Py++) {
                    int led = XY(Px,Py);
                    if(samples_remaining[led]==0 || samples_taken[led]==65535) continue;
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
                    Ivals[led] += iteration;
                    samples_taken[led]++;
                    if(Ivals[led] == 0) {
                        samples_remaining[led] = 0;
                    } else {
                        samples_remaining[XY(Px+1,Py)] = 1;
                        samples_remaining[XY(Px-1,Py)] = 1;
                        samples_remaining[XY(Px,Py+1)] = 1;
                        samples_remaining[XY(Px,Py-1)] = 1;
                    }
                }
            }
            sample_cnt++;
        }

        //draw our accumulated data to the screen
        CRGB old_color = CRGB(0,0,0);
        for (int i = 0; i < NUM_LEDS; i++) {
            if(!Ivals[i]) {
                if (samples_taken[i]) {
                    old_color = CRGB(0,0,0);
                } else {
                    leds[i] = old_color;
                }
                continue;
            }
            uint32_t temp0 = Ivals[i];
            temp0/=samples_taken[i];
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
            old_color = leds[i];
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
        if (reset_pixels || old_iterations != iterations) {
            was_moving = 1;
            sample_cnt = 0;
            for (int x = 0; x < MATRIX_WIDTH; x++) {
                for (int y = 0; y < MATRIX_HEIGHT; y++) {
                    int i = XY(x,y);
                    samples_remaining[i] = 1;
                    samples_taken[i] = 0;
                    Ivals[i] = 0;
                }
            }
        } else if (was_moving == 1) {
            was_moving = 0;
            for (int x = 0; x < MATRIX_WIDTH; x++) {
                for (int y = 0; y < MATRIX_HEIGHT; y++) {
                    int i = XY(x,y);
                    samples_remaining[i] = 1;
                }
            }
        }

        old_iterations = iterations;

    }

};

LIGHT_SKETCHES::REGISTER<MANDLEBROT> mandlebrot("mandlebrot");

#endif