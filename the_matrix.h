#ifndef LIGHTS_THE_MATRIX_H
#define LIGHTS_THE_MATRIX_H

#include "text.h"

//THE_MATRIX

class THE_MATRIX: public LIGHT_SKETCH {
  public:
    THE_MATRIX () {setup();}
    ~THE_MATRIX () {}
  private:
    Z_BUF _z_buffer;
    struct LETTER_STREAM {
        uint32_t move_time = 0;
        int32_t y = MATRIX_HEIGHT;
        int32_t x = 0;
        uint8_t old_chr = 'A';
        uint8_t chr = 'A';
        bool draw = 1;
        uint32_t change_time = 0;
        bool change = 1;
        uint8_t scale = 255;
        uint32_t relocate_time = 0;
        uint16_t speed = 500;
        uint8_t font;
    };

    #define NUM_LETTER_STREAMS 5
    LETTER_STREAM letter_streams[NUM_LETTER_STREAMS];

    uint8_t r_char() {
        return random(33,127);
    }

    #define NUM_GRASS 5
    VECTOR3 grass[NUM_GRASS];

    uint8_t font_sizes[7] = {11,14,14,6,6,4,6};

  public:
    void reset() {
    }

    void setup() {
        for (int i = 0; i < NUM_GRASS; i++) {
            grass[i].x = random(1,100)*256;
            if (random(2)) {
                grass[i].x*=-1;
            }
            grass[i].y = random(200*256);
            grass[i].z = 100000;
            // grass[i].z = 0;
            // grass[i].x = 0;
            // grass[i].y = 100*256;
        }
        for (int i = 0; i < NUM_LETTER_STREAMS; i++) {
            letter_streams[i].move_time = 0;
            letter_streams[i].y = random(MATRIX_HEIGHT);
            letter_streams[i].x = random(MATRIX_WIDTH);
            letter_streams[i].old_chr = r_char();
            letter_streams[i].chr = r_char();
            letter_streams[i].draw = 1;
            letter_streams[i].change_time = 0;
            letter_streams[i].change = 1;
            letter_streams[i].scale = random(64,256);
            letter_streams[i].scale = 128;
            letter_streams[i].relocate_time = 0;
            letter_streams[i].speed = random(1000,2000);
            letter_streams[i].font = random(1,8);
            
        }
    }

    void next_effect() {
    }
    void loop() {

        //do_triangle_thing();

        LED_show();
        z_buffer->reset();
        static uint8_t cnt = 0;
        cnt++;
        for (int i = 0; i < NUM_LEDS; i++) {
            if (cnt%4 == 0) {
                if (leds[i].r > 128) {
                    leds[i].r *=.92;
                    leds[i].g *=.99;
                    leds[i].b *=.92;
                } else if (leds[i].r > 0) {
                    leds[i].r *= .97;
                    leds[i].g *= .99;
                    leds[i].b *= .97;
                } else {
                    leds[i].g *= .99;
                }
            }
        }
        //LED_black();
        #define FONT_SIZE 6
        for (int i = 0; i < NUM_LETTER_STREAMS; i++) {
            if (millis() > letter_streams[i].move_time) {
                int r = random(letter_streams[i].speed);
                r = (r*r)/2000;
                letter_streams[i].move_time = millis() + r;
                letter_streams[i].y -= (font_sizes[letter_streams[i].font-1]*(letter_streams[i].scale+1))/256;
                if (random(80) == 0 || letter_streams[i].y < 0) {
                    letter_streams[i].relocate_time = millis() + random(2000,20000);
                    if (random(2)) {
                        letter_streams[i].y = random(MATRIX_HEIGHT);
                    } else {
                        letter_streams[i].y = MATRIX_HEIGHT + random(FONT_SIZE);
                    }
                    letter_streams[i].x = random(MATRIX_WIDTH);
                    letter_streams[i].scale = random(64,256);
                    letter_streams[i].scale = 128;
                    letter_streams[i].speed = random(800,1000);
                    letter_streams[i].font = random(1,8);
                }
                

                letter_streams[i].old_chr = letter_streams[i].chr;
                letter_streams[i].chr = r_char();
                letter_streams[i].draw = 1;
            }
            //if (millis() > letter_streams[i].relocate_time || letter_streams[i].y < 0) {
            
            if (millis() > letter_streams[i].change_time) {
                letter_streams[i].change_time = millis() + random(100,1000);
                letter_streams[i].old_chr = letter_streams[i].chr;
                letter_streams[i].chr = r_char();
                letter_streams[i].draw = 1;

            }
            if (letter_streams[i].draw) {
                letter_streams[i].draw = 0;
                draw_character(letter_streams[i].font, letter_streams[i].old_chr, letter_streams[i].x*256, letter_streams[i].y*256, CRGB(255,255,255), letter_streams[i].scale, 1);
                draw_character(letter_streams[i].font, letter_streams[i].chr, letter_streams[i].x*256, letter_streams[i].y*256, CRGB(255,255,255), letter_streams[i].scale);
            }
        }
    }
    void do_triangle_thing() {

        for(int i = 0; i < NUM_GRASS; i++) {
            VECTOR3 va(grass[i].x,        -164*256+grass[i].y,grass[i].z);
            VECTOR3 vb(grass[i].x-256*25, -104*256+grass[i].y,grass[i].z);
            VECTOR3 vc(grass[i].x,        -104*256+grass[i].y,grass[i].z);
            VECTOR3 vd(grass[i].x-256*25, -164*256+grass[i].y,grass[i].z);

            VECTOR3 pa;
            VECTOR3 pb;
            VECTOR3 pc;
            VECTOR3 pd;

            //rotate with our global matrix
            static uint8_t rx = 0;
            static uint8_t ry = 0;
            static uint8_t rz = 0;
            rx+=1;
            ry-=2;
            rz+=3;
            led_screen.matrix.rotate(va, pa);
            led_screen.matrix.rotate(vb, pb);
            led_screen.matrix.rotate(vc, pc);
            led_screen.matrix.rotate(vd, pd);
            //translate vectors to coordinates
            //scale_z(pa);
            //scale_z(pb);
            //scale_z(pc);
            //scale_z(pc);
            //correct 3d perspective
            led_screen.perspective(pa);
            led_screen.perspective(pb);
            led_screen.perspective(pc);
            led_screen.perspective(pd);

            VECTOR3 norm(0,0,255);
            VECTOR3 n;
            led_screen.matrix.rotate(norm, n);
            //VECTOR3 norm2(0,0,255);
            VECTOR3 norm2(0,255,0);
            VECTOR3 n2;
            led_screen.matrix.rotate(norm2, n2);

            bool test1;
            bool test2;

            

            test2 = draw_triangle_fine( pa,pc,pb,n,n2,n2,CRGB(25,100,25) );
            test1 = draw_triangle_fine( pa,pb,pd,n,n2,n,CRGB(25,100,25) );


            grass[i].z+=128;
            if (grass[i].z > led_screen.camera_position.z || !(test1 || test2)) {
                grass[i].z = random(10000,30000);
                grass[i].x = random(1,100)*256;
                if (random(2)) {
                    grass[i].x*=-1;
                }
                grass[i].y = random(200*256);
            }
        }
    }

};

LIGHT_SKETCHES::REGISTER<THE_MATRIX> the_matrix("The Matrix");

#endif