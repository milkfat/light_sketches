#ifndef LIGHTS_PHOSPHENE_H
#define LIGHTS_PHOSPHENE_H

class PHOSPHENE: public LIGHT_SKETCH {
  public:
        PHOSPHENE () {setup();}
    ~PHOSPHENE () {}

    #define SIZE_X (MATRIX_WIDTH)
    #define SIZE_Y (MATRIX_HEIGHT)


    #define FPS 60 // frames per second setting

    int16_t pixels[SIZE_Y][SIZE_X];
    int16_t delta[SIZE_Y][SIZE_X];


    inline void setVal(const uint32_t& x, const uint32_t& y, const int16_t& val) {
        int16_t old = getVal(x,y);
        pixels[y][x] = val;
        delta[y][x] = val-old;
    }

    inline int16_t getVal(const uint32_t& x,const uint32_t& y) {
        return pixels[y][x];
    }


    inline int16_t getDel(const uint32_t& x, const uint32_t& y) {
        return delta[y][x];
    }


    #define PHOSPHENE_ADVANCE 1638

    
    inline void processPix(const uint32_t& x, const uint32_t& y) {
        // get block values
        int32_t vals = 0;
        int32_t vals_cnt = 0;
        int32_t dels = 0;

        for (int iy = 0; iy < 3; iy++) {
            int32_t ty = y+iy-1;
            if ( ty < 0 || ty >= SIZE_Y ) {
                continue;
            }
            for (int ix = 0; ix < 3; ix++) {
                int32_t tx = x+ix-1;
                if ( tx < 0 || tx >= SIZE_X) {
                    continue;
                }
                vals += getVal(tx, ty);
                int32_t tempDel = getDel(tx, ty)/256;
                dels += tempDel * abs(tempDel);
                vals_cnt++;
            }
        }

        // compute shit


        int32_t me = getVal(x, y);
        int32_t avg = vals/vals_cnt;

        int32_t ddd = dels;

        //std::cout << ddd << "\n";
        if ( ddd < 0 ) {
            ddd = -sqrt16(-ddd);
        } else {
            ddd = sqrt16(ddd);
        }
        ddd*=256;
        
        int32_t out;
        
        if ( me > 16383 ) {
            //if over this, then drop off
            out = 8192;
        } else if ( me < 4916 ) {
            //if less than this, then start advancing (stop dropping off)
            out = me + PHOSPHENE_ADVANCE;
        } else {
            if ( ddd > 0 ) {
                //grow faster as neighbors grow faster
                out = me + (ddd*37)/100;
            } else if (ddd < -4456) {
                //drop off as neighbors drop off
                out = me + (ddd*9)/10;
            } else {
                out = me + PHOSPHENE_ADVANCE;
            }
        }
        //try not to get too far away from neighbors
        out = (3*out + avg)/4;
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
                uint32_t val=random(16384);
                uint8_t n = inoise8(x*20, y*20, 0);
                val = (val*n)/255;
                val = (val*n)/255;
                val = (val*n)/255;
                setVal(x, y, val);
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

           


            for (int y = 0; y < SIZE_Y; y++) {
                for (int x = 0; x < SIZE_X; x++) {
                    
                    leds[XY(x,y)].g = _min(_max(pixels[y][x],0),16833) >> 6;

                }
            }

            LED_show();
            LED_black();
        }
    }

};   


LIGHT_SKETCHES::REGISTER<PHOSPHENE> phosphene("phosphene");

#endif