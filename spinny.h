#ifndef LIGHTS_SPINNY_H
#define LIGHTS_SPINNY_H

//SPINNY

#define NUM_SPINNY_EFFECTS 2
class SPINNY: public LIGHT_SKETCH {
  public:
    SPINNY () {setup();}
   ~SPINNY () {}
  private:
   Z_BUF _z_buffer;
   int current_effect = 0;
   struct SQUARE {
       VECTOR3 pos;
       VECTOR3 spd;
       VECTOR3 rot;
       VECTOR3 rotdist;
       VECTOR3 rotspd;
       CRGB rgb;
       int32_t step;
   };

   #define NUM_SQUARES 40
   SQUARE squares[NUM_SQUARES];
  public:
    void reset() {
        setup();
    }

    void setup() {
        led_screen.screen_distance = 300*256;
        led_screen.light_falloff = 10;
        for (int i = 0; i < NUM_SQUARES; i++) {
            squares[i].pos.x = random(MATRIX_WIDTH*512)-MATRIX_WIDTH*256;
            squares[i].pos.y = random(MATRIX_HEIGHT*512)-MATRIX_HEIGHT*256;
            squares[i].pos.z = random(1650*256)-1500*256;
            squares[i].pos.x *= 1;
            squares[i].pos.y *= 1;
            squares[i].spd = VECTOR3(0,0,0);
            squares[i].rot = VECTOR3(random(65536),random(65536),random(65536));
            squares[i].rotspd = VECTOR3(random(64)-32,random(64)-32,random(64)-32);
            squares[i].rotspd *= 15;
            uint8_t r = random(3);
            switch (r) {
                case 0:
                    squares[i].rotdist = VECTOR3(random(65536)-32768,0,0);
                    break;
                case 1:
                    squares[i].rotdist = VECTOR3(0,random(65536)-32768,0);
                    break;
            }
            squares[i].rgb = CHSV(random(256),255,255);
            squares[i].step = random(150+256)-150;;

        }

        bool sorted = false;
        while (!sorted) {
            sorted = true;
            for (int i = 0; i < NUM_SQUARES-1; i++) {
                if (squares[i].pos.z > squares[i+1].pos.z) {
                    SQUARE temp = squares[i];
                    squares[i] = squares[i+1];
                    squares[i+1] = temp;
                    sorted = false;
                }
            }
        }

    }

    void next_effect() {
        current_effect++;
        current_effect%=NUM_SPINNY_EFFECTS;
    }

    void loop() {
        LED_show();
        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] *= 0;
        }
        LED_black();
        //draw squares
        for (int i = 0; i < NUM_SQUARES; i++) {
            squares[i].spd.z += 20;
            squares[i].pos += squares[i].spd;
            squares[i].step+=8;

            //reset square to be close to the camera if it is too far away
            if (squares[i].pos.z < -1550*256) {
                squares[i].pos.x = random(MATRIX_WIDTH*512)-MATRIX_WIDTH*256;
                squares[i].pos.y = random(MATRIX_HEIGHT*512)-MATRIX_HEIGHT*256;
                squares[i].pos.z = 175*256;
                squares[i].pos.x *= 1;
                squares[i].pos.y *= 1;
                squares[i].spd = VECTOR3(0,0,0);
                squares[i].rot = VECTOR3(random(65536),random(65536),random(65536));
                squares[i].rotspd = VECTOR3(random(64)-32,random(64)-32,random(64)-32);
                squares[i].rotspd *= 15;
                uint8_t r = random(3);
                switch (r) {
                    case 0:
                        squares[i].rotdist = VECTOR3(random(65536)-32768,0,0);
                        break;
                    case 1:
                        squares[i].rotdist = VECTOR3(0,random(65536)-32768,0);
                        break;
                }
                squares[i].step = 10000;


                //shift to end of array
                SQUARE temp = squares[i];
                for (int j = i; j < NUM_SQUARES-1; j++) {
                    squares[j] = squares[j+1];
                }
                squares[NUM_SQUARES-1] = temp;
            }

            //reset square to be far away if it is too close to the camera
            if (squares[i].pos.z > 175*256) {
                squares[i].pos.x = random(MATRIX_WIDTH*512)-MATRIX_WIDTH*256;
                squares[i].pos.y = random(MATRIX_HEIGHT*512)-MATRIX_HEIGHT*256;
                squares[i].pos.z = -1500*256;
                squares[i].pos.x *= 1;
                squares[i].pos.y *= 1;
                squares[i].spd = VECTOR3(0,0,0);
                squares[i].rot = VECTOR3(random(65536),random(65536),random(65536));
                squares[i].rotspd = VECTOR3(random(64)-32,random(64)-32,random(64)-32);
                squares[i].rotspd *= 15;
                uint8_t r = random(3);
                switch (r) {
                    case 0:
                        squares[i].rotdist = VECTOR3(random(65536)-32768,0,0);
                        break;
                    case 1:
                        squares[i].rotdist = VECTOR3(0,random(65536)-32768,0);
                        break;
                }
                squares[i].step = 10000;

                //shift to beginning of array
                SQUARE temp = squares[i];
                for (int j = i; j > 0; j--) {
                    squares[j] = squares[j-1];
                }
                squares[0] = temp;

            }
            VECTOR3 rot = squares[i].rot;
            if (squares[i].step <= 0) {
                //do nothing
            } else if (squares[i].step < 256) {
                //calculate our rotation
                rot.x += (ease8In(squares[i].step)*squares[i].rotdist.x)/256;
                rot.y += (ease8In(squares[i].step)*squares[i].rotdist.y)/256;
                rot.z += (ease8In(squares[i].step)*squares[i].rotdist.z)/256;
            } else {
                //generate a new rotation target
                squares[i].rot += squares[i].rotdist;
                uint8_t r = random(2);
                switch (r) {
                    case 0:
                        squares[i].rotdist = VECTOR3(random(65536)-32768,0,0);
                        break;
                    case 1:
                        squares[i].rotdist = VECTOR3(0,random(65536)-32768,0);
                        break;
                }
                squares[i].step = random(150)-150;
                rot = squares[i].rot;
            }

            //squares[i].rot += squares[i].rotspd;
            //define four points of a square
            VECTOR3 a = VECTOR3(-256*10,-256*10,0);
            VECTOR3 b = VECTOR3(256*10,-256*10,0);
            VECTOR3 c = VECTOR3(256*10,256*10,0);
            VECTOR3 d = VECTOR3(-256*10,256*10,0);
            VECTOR3 norm = VECTOR3(0,0,255);

            //rotate the square using its rotation value
            rotate16(a, rot);
            rotate16(b, rot);
            rotate16(c, rot);
            rotate16(d, rot);
            rotate16(norm, rot);

            //move the square to its location
            a+=squares[i].pos;
            b+=squares[i].pos;
            c+=squares[i].pos;
            d+=squares[i].pos;

            //rotate with our global matrix
            led_screen.matrix.rotate(a);
            led_screen.matrix.rotate(b);
            led_screen.matrix.rotate(c);
            led_screen.matrix.rotate(d);
            led_screen.matrix.rotate(norm);

            //calculate 3d perspective
            led_screen.perspective(a);
            led_screen.perspective(b);
            led_screen.perspective(c);
            led_screen.perspective(d);

            a.z = squares[i].pos.z;
            b.z = squares[i].pos.z;
            c.z = squares[i].pos.z;
            d.z = squares[i].pos.z;

            y_buffer->reset();
            reset_x_buffer();


            //uint8_t bri_r = _min(_max((abs(norm.x)*7)/8,0)+32,255);
            //uint8_t bri_g = _min(_max((abs(norm.y)*7)/8,0)+32,255);
            uint8_t bri_b = _min(_max((abs(norm.z)*7)/8,0)+32,255);
            CRGB rgb = squares[i].rgb;
            //rgb = CRGB(bri_r,bri_g,bri_b);
            color_scale(rgb, bri_b);

            draw_line_fine(led_screen, a, b, rgb, squares[i].pos.z, 255, 255, true, false, true, false);
            draw_line_fine(led_screen, b, c, rgb, squares[i].pos.z, 255, 255, true, false, true, false);
            draw_line_fine(led_screen, c, d, rgb, squares[i].pos.z, 255, 255, true, false, true, false);
            draw_line_fine(led_screen, d, a, rgb, squares[i].pos.z, 255, 255, true, false, true, false);

            if (current_effect) {
                fill_shape(squares[i].pos.z+512, rgb);
            }

        }
      
    }

};

LIGHT_SKETCHES::REGISTER<SPINNY> spinny("spinny");

#endif