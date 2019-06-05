
//CINCO DE MAYO

class CINCO_DE_MAYO: public LIGHT_SKETCH {
  public:
        CINCO_DE_MAYO () {setup();}
    ~CINCO_DE_MAYO () {}
  private:
    unsigned long time0 = millis();
    int z2 = 0;
    uint8_t current_effect = 1;
#define NUM_SPRITES 10
    struct SPRITE {
        int32_t x = 4*256;
        int32_t y = -20*256L;
        int16_t vx = 0;
        int16_t vy = 0;
        uint8_t angle = random(256);
        int8_t dir = random(-5,6);
        
    };

    SPRITE sprites[NUM_SPRITES];


    void rotate_pixel (alpha_pixel ap[], uint8_t image_width, uint8_t image_height, uint8_t image_x, uint8_t image_y, uint32_t x, uint32_t y, uint8_t angle, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
       

        int angle_sin = sin8(angle)-127;
        int angle_cos = cos8(angle)-127;
        
        int32_t u = ( (angle_cos) * (image_x*256-(256*image_width)/2) )/128L - ( (angle_sin) * (image_y*256L-(256*image_height)/2) )/128L;
        int32_t v = ( (angle_sin) * (image_x*256-(256*image_width)/2) )/128L + ( (angle_cos) * (image_y*256L-(256*image_height)/2) )/128L;
        
        blendXY_RGBA(ap, u+x, v+y, r, g, b, a);
        
    }


    void draw_image (int32_t x, int32_t y, uint8_t angle) {
        alpha_pixel ap[MATRIX_HEIGHT*MATRIX_WIDTH+1];
        for (int image_x = 0; image_x < 12; image_x++) {
            for (int image_y = 0; image_y < 18; image_y++) {
                uint8_t * p = &maraca[(image_y*12+image_x)*4];
                uint8_t b = *p;
                uint8_t g = *(p+1);
                uint8_t r = *(p+2);
                uint8_t a = *(p+3);
                rotate_pixel(ap, 12, 18, image_x, image_y, x, y, angle, r, g, b, a);
            }
        }
        for (x = 0; x < MATRIX_WIDTH; x++) {
            for (y = 0; y < MATRIX_HEIGHT; y++) {
                if (ap[XY(x,y)].cnt > 0) {
                    leds[XY(x,y)].r = (leds[XY(x,y)].r*(255-ap[XY(x,y)].a/ap[XY(x,y)].cnt))/255;
                    leds[XY(x,y)].r = leds[XY(x,y)].r + ((ap[XY(x,y)].r/ap[XY(x,y)].cnt)*(ap[XY(x,y)].a/ap[XY(x,y)].cnt))/255;
                    leds[XY(x,y)].g = (leds[XY(x,y)].g*(255-ap[XY(x,y)].a/ap[XY(x,y)].cnt))/255;
                    leds[XY(x,y)].g = leds[XY(x,y)].g + ((ap[XY(x,y)].g/ap[XY(x,y)].cnt)*(ap[XY(x,y)].a/ap[XY(x,y)].cnt))/255;
                    leds[XY(x,y)].b = (leds[XY(x,y)].b*(255-ap[XY(x,y)].a/ap[XY(x,y)].cnt))/255;
                    leds[XY(x,y)].b = leds[XY(x,y)].b + ((ap[XY(x,y)].b/ap[XY(x,y)].cnt)*(ap[XY(x,y)].a/ap[XY(x,y)].cnt))/255;
                }
            }
        }
    }

    uint8_t maraca[12*18*4] = {
        /*Pixel format: Red: 8 bit, Green: 8 bit, Blue: 8 bit, Alpha: 8 bit*/
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x04, 0x14, 0x20, 0x27, 0x3e, 0x4d, 0x97, 0x46, 0x70, 0x82, 0xe7, 0x45, 0x6e, 0x80, 0xf8, 0x2f, 0x4c, 0x58, 0xd4, 0x14, 0x20, 0x25, 0x63, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2d, 0x2f, 0x05, 0x07, 0x72, 0xdf, 0x36, 0x4d, 0xd8, 0xff, 0x87, 0xbb, 0xf8, 0xff, 0x9f, 0xdb, 0xfa, 0xff, 0x96, 0xde, 0xfe, 0xff, 0x69, 0xa6, 0xc0, 0xff, 0x2e, 0x4a, 0x55, 0x98, 0x08, 0x0f, 0x0d, 0x0f, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x17, 0x1a, 0x10, 0x0b, 0x11, 0x62, 0xac, 0x14, 0x14, 0xd1, 0xff, 0x2d, 0x2a, 0xe4, 0xff, 0x31, 0x2f, 0xe3, 0xff, 0x54, 0x60, 0xe9, 0xff, 0x7f, 0xa6, 0xf0, 0xff, 0x83, 0xc3, 0xf8, 0xff, 0x5c, 0x94, 0xba, 0xff, 0x20, 0x34, 0x46, 0x5b, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x27, 0x40, 0x53, 0x34, 0x47, 0x6c, 0xa1, 0xff, 0x5a, 0x6e, 0xf0, 0xff, 0x40, 0x46, 0xe5, 0xff, 0x30, 0x2f, 0xe3, 0xff, 0x2d, 0x2a, 0xe2, 0xff, 0x28, 0x25, 0xe1, 0xff, 0x2d, 0x36, 0xe0, 0xff, 0x34, 0x51, 0xe3, 0xff, 0x10, 0x1a, 0x57, 0xc0, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x22, 0x37, 0x40, 0x6f, 0x81, 0xc4, 0xdc, 0xff, 0xae, 0xe6, 0xf9, 0xff, 0x8d, 0xb3, 0xf2, 0xff, 0x63, 0x77, 0xeb, 0xff, 0x3f, 0x46, 0xe5, 0xff, 0x24, 0x22, 0xe1, 0xff, 0x12, 0x0e, 0xd8, 0xff, 0x00, 0x00, 0xcb, 0xff, 0x00, 0x00, 0x5b, 0xec, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x21, 0x34, 0x3d, 0x88, 0x8a, 0xd6, 0xfa, 0xff, 0x9b, 0xdc, 0xf6, 0xff, 0xa7, 0xe5, 0xf6, 0xff, 0x9d, 0xd9, 0xf5, 0xff, 0x79, 0xab, 0xef, 0xff, 0x58, 0x7e, 0xe9, 0xff, 0x34, 0x4c, 0xdc, 0xff, 0x14, 0x23, 0xca, 0xff, 0x02, 0x04, 0x4e, 0xcb, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x0f, 0x23, 0x1d, 0x80, 0x4d, 0x98, 0x90, 0xff, 0x87, 0xd2, 0xf3, 0xff, 0x91, 0xd8, 0xfb, 0xff, 0x8d, 0xd4, 0xf3, 0xff, 0x8b, 0xd6, 0xf0, 0xff, 0x85, 0xd1, 0xef, 0xff, 0x70, 0xb7, 0xe8, 0xff, 0x49, 0x81, 0xbb, 0xff, 0x11, 0x1f, 0x3b, 0x73, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x01, 0x15, 0x01, 0x64, 0x07, 0x48, 0x0e, 0xff, 0x26, 0x73, 0x47, 0xff, 0x5c, 0xa8, 0xa9, 0xff, 0x82, 0xcf, 0xf1, 0xff, 0x86, 0xd3, 0xf9, 0xff, 0x7c, 0xca, 0xed, 0xff, 0x72, 0xc4, 0xe6, 0xff, 0x3a, 0x66, 0x75, 0xe4, 0x1a, 0x2e, 0x2f, 0x20, 
        0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x0d, 0x00, 0x00, 0x1b, 0x00, 0x37, 0x00, 0x34, 0x00, 0xff, 0x00, 0x4e, 0x01, 0xff, 0x07, 0x53, 0x0e, 0xff, 0x1b, 0x65, 0x33, 0xff, 0x3e, 0x87, 0x78, 0xff, 0x56, 0xa5, 0xb1, 0xff, 0x40, 0x73, 0x85, 0xff, 0x20, 0x38, 0x44, 0x4f, 0x00, 0x00, 0x00, 0x03, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x38, 0x29, 0x23, 0x0f, 0x3b, 0x1f, 0xff, 0x00, 0x41, 0x00, 0xff, 0x00, 0x3e, 0x00, 0xff, 0x00, 0x40, 0x00, 0xff, 0x02, 0x40, 0x04, 0xff, 0x04, 0x29, 0x08, 0xec, 0x07, 0x18, 0x10, 0x50, 0x11, 0x02, 0x22, 0x03, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x57, 0x68, 0x2c, 0x49, 0x82, 0x9a, 0xff, 0x53, 0xa0, 0xae, 0xff, 0x1f, 0x63, 0x41, 0xff, 0x04, 0x37, 0x09, 0xff, 0x00, 0x17, 0x00, 0xa7, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x1f, 0x37, 0x41, 0x70, 0x5f, 0xa8, 0xc7, 0xff, 0x65, 0xb3, 0xd5, 0xff, 0x48, 0x7c, 0x96, 0xe8, 0x11, 0x1a, 0x24, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x11, 0x20, 0x26, 0x14, 0x36, 0x61, 0x72, 0xd8, 0x69, 0xba, 0xdc, 0xff, 0x54, 0x95, 0xb0, 0xe8, 0x17, 0x29, 0x32, 0x5b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x23, 0x3e, 0x49, 0x74, 0x59, 0x9e, 0xbb, 0xff, 0x61, 0xab, 0xcb, 0xfc, 0x44, 0x78, 0x8e, 0x9f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x1c, 0x2e, 0x35, 0x3f, 0x59, 0x92, 0xaa, 0xf7, 0x71, 0xc2, 0xe4, 0xff, 0x5d, 0xa5, 0xc3, 0xe8, 0x16, 0x29, 0x30, 0x50, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x3e, 0x64, 0x74, 0xcf, 0x9a, 0xdf, 0xf9, 0xff, 0x89, 0xcf, 0xeb, 0xff, 0x59, 0x97, 0xb1, 0xdb, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x43, 0x6c, 0x7d, 0xef, 0xa0, 0xe7, 0xff, 0xff, 0x8f, 0xd1, 0xec, 0xff, 0x39, 0x5f, 0x6f, 0xb4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x26, 0x3e, 0x48, 0x4f, 0x3e, 0x5f, 0x6d, 0xdf, 0x3d, 0x5e, 0x6c, 0xc4, 0x15, 0x25, 0x2b, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        };

  public:
    void reset() {

    }

    void setup() {
      for (int i = 0; i < NUM_SPRITES; i++) {
        sprites[i].x = 4*256;
        sprites[i].y = -20*256L;
        sprites[i].vx = 0;
        sprites[i].vy = 0;
        sprites[i].angle = random(256);
        sprites[i].dir = random(-5,6);
      }
    }

    void next_effect() {
      current_effect++;
      if(current_effect > 2) {
        current_effect = 0;
      }
    }

//0xEACA3E,0xAFA842,0xF7B791,0xE8D496,0xC6DB9F
//0xAD4132,0xF4AB2E,0x56733C,0xC09074,0x784742

    void loop() {
      static unsigned long t = millis();

      if (millis() - 16 > t) {
        t = millis();
      
        if (current_effect == 0) {
              static uint8_t cycle = 0;
              cycle+=2;
              
              for (int x = 0; x < MATRIX_WIDTH; x++) {
                  for (int y = 0; y < MATRIX_HEIGHT; y++) {
                      if ( y < 25 ) {
                          uint8_t v = sin8(cycle);
                          leds[XY(x,y)] = CRGB(
                            cie((40*v)/256),
                            cie((195*v)/256),
                            cie((168*v)/256));
                      } else if (y < 50 ) {
                          uint8_t v = sin8(cycle+48);
                          leds[XY(x,y)] = CRGB(
                            cie((238*v)/256),
                            cie((225*v)/256),
                            cie((95*v)/256));
                      } else if (y < 75 ) {
                          uint8_t v = sin8(cycle+96);
                          leds[XY(x,y)] = CRGB(
                            cie((238*v)/256),
                            cie((0*v)/256),
                            cie((0*v)/256));
                      } else if (y < 100 ) {
                          uint8_t v = sin8(cycle+144);
                          leds[XY(x,y)] = CRGB(
                            cie((176*v)/256),
                            cie((17*v)/256),
                            cie((145*v)/256));
                      } else {
                          uint8_t v = sin8(cycle+192);
                          leds[XY(x,y)] = CRGB(
                            cie((162*v)/256),
                            cie((204*v)/256),
                            cie((44*v)/256));
                      }
                  }
              }
          } else if (current_effect == 1) {
              for (int i = 0; i < NUM_SPRITES; i++) {
                  SPRITE * s = &sprites[i];
                  s->angle += s->dir;
                  s->y += s->vy;
                  s->vy--;
                  s->x += s->vx;
                  if (s->y < -10*256) {
                      s->y = (MATRIX_HEIGHT+random(10,20))*256;
                      s->x = random(MATRIX_WIDTH*384L)-2*256;
                      s->vx = random(-10,10);
                      if (s->x < 0) {
                          s->vx = 20;
                      }
                      if (s->x > (MATRIX_WIDTH-1)*256L) {
                          s->vx = -20;
                      }
                      s->vy = random(0,200) - 200;
                      s->angle = random(256);
                      s->dir = random(-5,6);
                      while (s->dir == 0) {
                          s->dir = random(-5,6);
                      }
                  }
                  draw_image(s->x, s->y, s->angle);
              }
          }
  
          
          LED_show();
          LED_black();
      }
    }
};

LIGHT_SKETCHES::REGISTER<CINCO_DE_MAYO> cinco_de_mayo("cinco_de_mayo");