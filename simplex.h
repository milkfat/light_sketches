
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
      if(current_effect > 2) {
        current_effect = 0;
      }
    }

    void loop() {
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
      
      for (int x2 = 0; x2 < MATRIX_WIDTH; x2++) {
          int x = x2*32;
        for (int y2 = 0; y2 < MATRIX_HEIGHT; y2++) {
          int y = y2*32;
          int z = z2*14;
          //flip the x/y axis to create 4 layers from one
          int i = XY(x2,y2);
          int i2 = XY(MATRIX_WIDTH - 1 - x2,MATRIX_HEIGHT - 1 - y2);
          int i3 = XY(MATRIX_WIDTH - 1 - x2,y2);
          int i4 = XY(x2,MATRIX_HEIGHT - 1 - y2);
          

          //simple simplex
          if (current_effect == 0) {
            val = _min(_max(inoise8(x, y, z),48),255);
            val = (val * val * val)/(256*256);
            leds[i] += CHSV(default_color, default_saturation, val);
          }

          //four layer simplex, plasma-ish effect
          if (current_effect == 1) {

            
            val4 = _min(_max(inoise8(4096+x, 4096+y, 4096+z),48),255);
            if (val4 > 96 && val4 < 160) {
              val4 = 31-abs(val4 - 128);
            } else {
              val4 = 0;
            }
            if (val4 > 0) {
              //layer 4
              leds[i].fadeToBlackBy( val4 );
              leds[i] += CHSV(default_color+48, default_saturation, val4);
              //layer 3
              leds[i2].fadeToBlackBy( val4*2 );
              leds[i2] += CHSV(default_color+160, default_saturation, val4*2);
              //layer 2
              leds[i3].fadeToBlackBy( val4*4 );
              leds[i3] += CHSV(default_color+96, default_saturation, val4*4);
              //layer 1
              leds[i4].fadeToBlackBy( val4*8 );
              leds[i4] += CHSV(default_color, default_saturation, val4*8);
            }

          }


          

          //four layer simplex, plasma-ish effect with zoom
          if (current_effect == 2) {

            //fourth layer
            //simplex noise
            val4 = _min(_max(inoise8(4096+(x2-MATRIX_WIDTH/2)*zoom4, 4096+(y2-MATRIX_HEIGHT/2)*zoom4, 4096+z),48),255);
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
            val3 = _min(_max(inoise8(2048+(x2-MATRIX_WIDTH/2)*zoom3, 2048+(y2-MATRIX_HEIGHT/2)*zoom3, 2048+z),48),255);
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
            val2 = _min(_max(inoise8(1024+(x2-MATRIX_WIDTH/2)*zoom2, 1024+(y2-MATRIX_HEIGHT/2)*zoom2, 1024+z),48),255);
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
            val = _min(_max(inoise8((x2-MATRIX_WIDTH/2)*zoom, (y2-MATRIX_HEIGHT/2)*zoom, z),48),255);
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
              leds[i] -= *v4;
              leds[i] += CHSV(default_color+v4_color, default_saturation, *v4);
            }

            if (*v3 > 0) {
              leds[i] -= *v3;
              leds[i] += CHSV(default_color+v3_color, default_saturation, *v3);
            }

            if (*v2 > 0) {
              leds[i] -= *v2;
              leds[i] += CHSV(default_color+v2_color, default_saturation, *v2);
            }
            
            if (*v > 0) {
              leds[i] -= *v;
              leds[i] += CHSV(default_color+v_color, default_saturation, *v);
            }
            
          }




          
        }
      }
      
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
};

LIGHT_SKETCHES::REGISTER<SIMPLEX> simplex("simplex");