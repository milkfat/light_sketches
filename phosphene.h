


class PHOSPHENE: public LIGHT_SKETCH {
  public:
        PHOSPHENE () {setup();}
    ~PHOSPHENE () {}

    #define SIZE_X (MATRIX_WIDTH/2)
    #define SIZE_Y (MATRIX_HEIGHT/2)
    #define SIZE_PIX 5


    #define FPS 30 // frames per second setting

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
    #define fullFrameUpdateSpeed 6
    int32_t curIdx = 0;
    uint32_t pixOrder[(SIZE_Y * SIZE_X) * fullUpdateEvery];
    uint32_t updatePerFrame = (SIZE_Y*SIZE_X*fullUpdateEvery) / fullFrameUpdateSpeed;

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
/*
        for (int y = 0; y < 50; y++) {
            for (int x = 0; x < 50; x++) {
                setVal(x+SIZE_X/2-25, y+SIZE_Y/2-25, 1);
            }
        }
*/
        for (int i = 0; i < (SIZE_Y * SIZE_X) * fullUpdateEvery; i++) {
            pixOrder[i] = i%(SIZE_Y * SIZE_X);
        }
        
    }


    void loop() {
        static uint32_t phosphene_time = millis();
        if (millis() - 50 > phosphene_time) {
            phosphene_time = millis();

            if (curIdx >= (SIZE_Y*SIZE_X)) {
                //random.shuffle(pixOrder);
                for (int i = 0; i < (SIZE_Y * SIZE_X) * fullUpdateEvery; i++) {
                    uint32_t temp = pixOrder[i];
                    uint32_t r = random((SIZE_Y * SIZE_X) * fullUpdateEvery);
                    pixOrder[i] = pixOrder[r];
                    pixOrder[r] = temp;
                }
                curIdx = 0;
            }
            
            int32_t nextIdx = curIdx + updatePerFrame;
            while (curIdx < nextIdx && curIdx < (SIZE_Y*SIZE_X)) {
                uint32_t coord = pixOrder[curIdx];
                processPix(coord%SIZE_X, coord/SIZE_X);
                curIdx += 1;
            }

            //each pixel is comprised of four sub-pixels
            //
            //  AB  AB  AB
            //  CD  CD  CD
            //
            //  AB  AB  AB
            //  CD  CD  CD
            //
            //  AB  AB  AB
            //  CD  CD  CD
            //
            //

            //interpolate between pixels 
           


            for (int x = 0; x < SIZE_X; x++) {
                for (int y = 0; y < SIZE_Y; y++) {
                    

                    float bri = FC.decompress(pixels[y][x]);
                    
                    leds[XY(x*2-1,y*2-1)].g += (uint8_t)_min(_max((bri*255)/4,0),255);
                    leds[XY(x*2-1,y*2)].g += (uint8_t)_min(_max((bri*255)/2,0),255);
                    leds[XY(x*2-1,y*2+1)].g += (uint8_t)_min(_max((bri*255)/4,0),255);
                    leds[XY(x*2,y*2-1)].g += (uint8_t)_min(_max((bri*255)/2,0),255);
                    leds[XY(x*2,y*2)].g = (uint8_t)_min(_max(bri*255,0),255);
                    leds[XY(x*2,y*2+1)].g += (uint8_t)_min(_max((bri*255)/2,0),255);
                    leds[XY(x*2+1,y*2-1)].g += (uint8_t)_min(_max((bri*255)/4,0),255);
                    leds[XY(x*2+1,y*2)].g += (uint8_t)_min(_max((bri*255)/2,0),255);
                    leds[XY(x*2+1,y*2+1)].g += (uint8_t)_min(_max((bri*255)/4,0),255);

               
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