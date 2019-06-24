#define HALF_SIZE


class PHOSPHENE: public LIGHT_SKETCH {
  public:
        PHOSPHENE () {setup();}
    ~PHOSPHENE () {}
    
    #ifdef HALF_SIZE
    #define GRID_DIVISOR 2
    #else
    #define GRID_DIVISOR 1
    #endif

    #define SIZE_X (MATRIX_WIDTH/GRID_DIVISOR)
    #define SIZE_Y (MATRIX_HEIGHT/GRID_DIVISOR)


    #define FPS 60 // frames per second setting

    uint16_t pixels[SIZE_Y][SIZE_X];
    uint16_t delta[SIZE_Y][SIZE_X];


    void setVal(uint32_t x, uint32_t y, float val) {
        float old = getVal(x,y);
        pixels[y][x] = FC.compress(val);
        delta[y][x] = FC.compress(val-old);
    }

    float getVal(uint32_t x,uint32_t y) {
        return FC.decompress(pixels[y][x]);
    }


    float getDel(uint32_t x, uint32_t y) {
        return FC.decompress(delta[y][x]);
    }


    #define ADVANCE 0.1f

    
    void processPix(uint32_t x, uint32_t y) {
        // get block values
        float vals = 0;
        float vals_cnt = 0;
        float dels = 0;

        for (int iy = 0; iy < 3; iy++) {
            float ty = y+iy-1;
            if ( ty < 0 || ty >= SIZE_Y ) {
                continue;
            }
            for (int ix = 0; ix < 3; ix++) {
                float tx = x+ix-1;
                if ( tx < 0 || tx >= SIZE_X) {
                    continue;
                }
                vals += getVal(tx, ty);
                float tempDel = getDel(tx, ty);
                dels += tempDel * fabs(tempDel);
                vals_cnt++;
            }
        }

        // compute shit


        float me = getVal(x, y);
        float avg = (vals*1.f)/(vals_cnt*1.f);

        float ddd = dels;

        
        if ( ddd < 0 ) {
            ddd = -sqrt(-ddd);
        } else {
            ddd = sqrt(ddd);
        }

        
        float out;
        
        if ( me > 1 ) {
            //if over this, then drop off
            out = 0.5f;
        } else if ( me < 0.3f ) {
            //if less than this, then start advancing (stop dropping off)
            out = me + ADVANCE;
        } else {
            if ( ddd > 0.f ) {
                //grow faster as neighbors grow faster
                out = me + ddd*.37f;
            } else if (ddd < -.15f) {
                //drop off as neighbors drop off
                out = me + ddd*.9f;
            } else {
                out = me + ADVANCE;
            }
        }
        //try not to get too far away from neighbors
        out = (3.f*out + avg)/4.f;
        //update value
        setVal(x, y, out);
        
    }


    #define fullUpdateEvery 3
    #define fullFrameUpdateSpeed 4
    int32_t curIdx = 0;

    uint32_t updatePerFrame = (SIZE_Y*SIZE_X) / fullFrameUpdateSpeed;  //number of pixels to be processed per frame

    void next_effect() {}

    void reset() {}

    void setup() {

        for (int y = 0; y < SIZE_Y; y++) {
            for (int x = 0; x < SIZE_X; x++) {
                pixels[y][x] = 0;
                delta[y][x] = 0;
                setVal(x, y, random(255)/455.f);
            }
        }        
    }
    

    void loop() {
        static uint32_t phosphene_time = millis();
        if (millis() - 1000/FPS > phosphene_time) {
            phosphene_time = millis();



            curIdx = 0;
            while (curIdx < updatePerFrame) {
                processPix(random(SIZE_X), random(SIZE_Y));
                curIdx++;
            }

            //interpolate between pixels 
           


            for (int x = 0; x < SIZE_X; x++) {
                for (int y = 0; y < SIZE_Y; y++) {
                    

                    float bri = FC.decompress(pixels[y][x]);
                    CRGB rgb = CHSV(96,255,_min(_max(bri*255,0),255));
                    #ifdef HALF_SIZE
                    leds[XY(x*2,y*2)] = rgb;
                    rgb.r /= 2;
                    rgb.g /= 2;
                    rgb.b /= 2;
                    leds[XY(x*2-1,y*2)] += rgb;
                    leds[XY(x*2,y*2-1)] += rgb;
                    leds[XY(x*2,y*2+1)] += rgb;
                    leds[XY(x*2+1,y*2)] += rgb;
                    rgb.r /= 2;
                    rgb.g /= 2;
                    rgb.b /= 2;
                    leds[XY(x*2-1,y*2-1)] += rgb;
                    leds[XY(x*2-1,y*2+1)] += rgb;
                    leds[XY(x*2+1,y*2-1)] += rgb;
                    leds[XY(x*2+1,y*2+1)] += rgb;
                    #else
                    leds[XY(x,y)] += rgb;
                    #endif

               
                }
            }
/*            
            for (int x = 0; x < MATRIX_WIDTH; x++) {
                for (int y = 0; y < MATRIX_HEIGHT; y++) {
                    

                    float bri = FC.decompress(pixels[y][x]);
                    
                    leds[XY(x,y)].g = _min(_max(bri*255,0),255);

               
                }
            }
*/

            LED_show();
            LED_black();
        }
    }

};   


LIGHT_SKETCHES::REGISTER<PHOSPHENE> phosphene("phosphene");