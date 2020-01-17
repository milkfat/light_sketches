#ifndef LIGHTS_SIMPLEX_NOISE_H
#define LIGHTS_SIMPLEX_NOISE_H

//SIMPLEX NOISE

class SIMPLEX: public LIGHT_SKETCH {
  public:
        SIMPLEX () {setup();}
    ~SIMPLEX () {}
  private:
    unsigned long time0 = millis();
    int z2 = 0;
    uint8_t current_effect = 1;

  public:
    void reset() {

    }

    void setup() {

    }

    void next_effect() {
      current_effect++;
      if(current_effect > 3) {
        current_effect = 0;
      }
    }
    uint32_t loop_time = 0;
    void loop() {
      if (millis() - 32 > loop_time) {
        loop_time = millis();
        static uint8_t zoom4 = 64;
        static uint8_t zoom3 = 48;
        static uint8_t zoom2 = 32;
        static uint8_t zoom = 16;

        static uint8_t val4 = 0;
        static uint8_t val3 = 0;
        static uint8_t val2 = 0;
        static uint8_t val = 0;

        static uint8_t *v4 = &val4;
        static uint8_t *v3 = &val3;
        static uint8_t *v2 = &val2;
        static uint8_t *v = &val;

        static uint8_t v4_color = 48;
        static uint8_t v3_color = 160;
        static uint8_t v2_color = 96;
        static uint8_t v_color = 0;

        CRGB rgb4;
        hsv2rgb_rainbow(CHSV(default_color+48, default_saturation, 255), rgb4);
        CRGB rgb3;
        hsv2rgb_rainbow(CHSV(default_color+160, default_saturation, 255), rgb3);
        CRGB rgb2;
        hsv2rgb_rainbow(CHSV(default_color+96, default_saturation, 255), rgb2);
        CRGB rgb;
        hsv2rgb_rainbow(CHSV(default_color, default_saturation, 255), rgb);

        CRGB rgb_v4;
        hsv2rgb_rainbow(CHSV(default_color+v4_color, default_saturation, 255),rgb_v4);
        CRGB rgb_v3;
        hsv2rgb_rainbow(CHSV(default_color+v3_color, default_saturation, 255),rgb_v3);
        CRGB rgb_v2;
        hsv2rgb_rainbow(CHSV(default_color+v2_color, default_saturation, 255),rgb_v2);
        CRGB rgb_v;
        hsv2rgb_rainbow(CHSV(default_color+v_color, default_saturation, 255),rgb_v);
        
        uint8_t start_y = 0;
        start_y++;
        start_y%=2;
        for (int y2 = 0; y2 < MATRIX_HEIGHT; y2++) {
          int y = y2<<5;
          for (int x2 = 0; x2 < MATRIX_WIDTH; x2++) {
            int x = x2<<5;
            int z = z2<<4;
            //flip the x/y axis to create 4 layers from one
            int i = XY(x2,y2);
            //int i_top = XY(x2,y2+1);
            //int i_bottom = XY(x2,y2-1);
            

            //zoomy simplex
            if (current_effect == 3) {
              static uint32_t zoom = 0;
              zoom+=3;
              x = x2 - (MATRIX_WIDTH/2);
              y = y2 - (MATRIX_HEIGHT/2);
              z = _max(abs(x), abs(y));
              int mult = _max(MATRIX_WIDTH/2+48, MATRIX_HEIGHT/2+48) - z;
              mult = (mult*mult)/_max(MATRIX_WIDTH/2+48, MATRIX_HEIGHT/2+48);
              val = _min(_max(inoise16(x*mult*256, y*mult*256, z*32*256-zoom)/256,48),255);
              val = (val * val * val)/(256*256);
              leds[i] += CHSV(default_color, default_saturation, val);
            }

            //simple simplex
            if (current_effect == 0) {
              val = _min(_max(inoise8(x, y, z),48),255);
              val = (val * val * val)/(256*256);
              leds[i] += CHSV(default_color, default_saturation, val);
            }

            //four layer simplex, plasma-ish effect
            if (current_effect == 1) {
              
              int i2 = XY(MATRIX_WIDTH - 1 - x2,MATRIX_HEIGHT - 1 - y2);
              int i3 = XY(MATRIX_WIDTH - 1 - x2,y2);
              int i4 = XY(x2,MATRIX_HEIGHT - 1 - y2);
              
              val4 = _min(_max(inoise8(4096+x, 4096+y, 4096+z),48),255);
              if (val4 > 96 && val4 < 160) {
                val4 = 31-abs(val4 - 128);
              } else {
                val4 = 0;
              }
              if (val4 > 0) {
                //layer 4
                nblend(leds[i],rgb4, val4);
                //layer 3
                nblend(leds[i2],rgb3, val4*2);
                //layer 2
                nblend(leds[i3],rgb2, val4*4);
                //layer 1
                nblend(leds[i4],rgb, val4*8);
              }

            }


            

            //four layer simplex, plasma-ish effect with zoom
            if (current_effect == 2) {

              //fourth layer
              //simplex noise
              int x_stuff = x2-(MATRIX_WIDTH/2);
              int y_stuff = y2-(MATRIX_HEIGHT/2);
              uint8_t noise4 = inoise8(4096+x_stuff*zoom4, 4096+y_stuff*zoom4, 4096+z);
              val4 = _min(_max(noise4,48),255);
              //filter simplex noise with a min and max threshold
              if (val4 > 96 && val4 < 160) {
                //normalize our values to 0-255
                val4 = 31-abs(val4 - 128);
                val4 *= 8;
                //fading effect when zooming
                uint8_t bri = abs(zoom4-8); //fade in & out
                if (zoom4 < 8) {
                  bri*=8; //quickly fade out
                }
                val4 = _max(val4 - (val4*bri)/56,0);
              } else {
                //values outside of our threshold are zeroed
                val4 = 0;
              }

              //third layer
              uint8_t noise3 = inoise8(2048+x_stuff*zoom3, 2048+y_stuff*zoom3, 2048+z);
              val3 = _min(_max(noise3,48),255);
              if (val3 > 96 && val3 < 160) {
                val3 = 31-abs(val3- 128);
                val3 *= 8;
                uint8_t bri = abs(zoom3-8);
                if (zoom3 < 8) {
                  bri*=8;
                }
                val3 = _max(val3 - (val3*bri)/56,0);
              } else {
                val3 = 0;
              }
              
              //secondary layer
              uint8_t noise2 = inoise8(1024+x_stuff*zoom2, 1024+y_stuff*zoom2, 1024+z);
              val2 = _min(_max(noise2,48),255);
              if (val2 > 96 && val2 < 160) {
                val2 = 31-abs(val2 - 128);
                val2 *= 8;
                uint8_t bri = abs(zoom2-8);
                if (zoom2 < 8) {
                  bri *= 8;
                }
                val2 = _max(val2 - (val2*bri)/56,0);
              } else {
                val2 = 0;
              }

              //primary layer
              uint8_t noise = inoise8(x_stuff*zoom, y_stuff*zoom, z);
              val = _min(_max(noise,48),255);
              if (val > 96 && val < 160) {
                val = 31-abs(val - 128);
                val *= 8;
                uint8_t bri = abs(zoom-8);
                if (zoom < 8) {
                  bri*=8;
                }
                val = _max(val - (val*bri)/56,0);
              } else {
                val = 0;
              }


              if (*v4 > 0) {
                nblend(leds[i],rgb_v4,*v4);
              }

              if (*v3 > 0) {
                nblend(leds[i],rgb_v3,*v3);
              }

              if (*v2 > 0) {
                nblend(leds[i],rgb_v2,*v2);
              }
              
              if (*v > 0) {
                nblend(leds[i],rgb_v,*v);
              }


              //leds[i_top] = leds[i];
              //leds[i_top] /= 2;
              //leds[i_bottom] += leds[i_top];
              
            }




            
          }
        }
        #ifdef SIMPLEX_QUARTER_RES
        for (int y = MATRIX_HEIGHT-1; y >= 0; y--) {
          for (int x = MATRIX_WIDTH-1; x >= 0; x--) {
            
            int led = XY(x,y);


            //
            //  c      d
            //   3  4
            //  a1  2  b
            //

            uint16_t red = 0; 
            uint16_t green = 0;
            uint16_t blue = 0;


            int a = XY(x>>1,y>>1);
            red += leds[a].r;
            green += leds[a].g;
            blue += leds[a].b;
            uint8_t total = 1;

            if (x%2) {
              
              int b = XY((x+1)>>1,y>>1);
              red += leds[b].r;
              green += leds[b].g;
              blue += leds[b].b;
              total++;
            }

            if (y%2) {

              int c = XY(x>>1,(y+1)>>1);
              red += leds[c].r;
              green += leds[c].g;
              blue += leds[c].b;
              total++;

              if (x%2) {

                int d = XY((x+1)>>1,(y+1)>>1);
                red += leds[d].r;
                green += leds[d].g;
                blue += leds[d].b;
                total++;

              }
            }

            leds[led].r = red/total;
            leds[led].g = green/total;
            leds[led].b = blue/total;

          }
        }
        #endif
        
        zoom4--;
        if (zoom4 > 63) {
          zoom4 = 63;
          v = &val;
          v2 = &val2;
          v3 = &val3;
          v4 = &val4;
          v_color = 0;
          v2_color = 96;
          v3_color = 160;
          v4_color = 48;
        }
        
        zoom3--;
        if (zoom3 > 63) {
          zoom3 = 63;
          v = &val4;
          v2 = &val;
          v3 = &val2;
          v4 = &val3;
          v_color = 48;
          v2_color = 0;
          v3_color = 96;
          v4_color = 160;
        }
        
        zoom2--;
        if (zoom2 > 63) {
          zoom2 = 63;
          v = &val3;
          v2 = &val4;
          v3 = &val;
          v4 = &val2;
          v_color = 160;
          v2_color = 48;
          v3_color = 0;
          v4_color = 96;
        }
        
        zoom--;
        if (zoom > 63) {
          zoom = 63;
          v = &val2;
          v2 = &val3;
          v3 = &val4;
          v4 = &val;
          v_color = 96;
          v2_color = 160;
          v3_color = 48;
          v4_color = 0;
        }
        
        z2++;

        LED_show(); //update LEDs
        LED_black();
      }
    }
};

LIGHT_SKETCHES::REGISTER<SIMPLEX> simplex("simplex");

#endif