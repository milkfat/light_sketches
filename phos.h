
class PHOS {
    public:
    #define PSIZE_X 42
    #define PSIZE_Y 42
    #define SIZE_PIX 5


    #define FPS 30 // frames per second setting

    uint16_t pixels[PSIZE_Y][PSIZE_X];

    float getVal(uint32_t x,uint32_t y) {
        return FC.decompress(pixels[y][x]);
    }
    
    private:
    uint16_t delta[PSIZE_Y][PSIZE_X];


    void setVal(uint32_t x, uint32_t y, float val) {
        float old = getVal(x,y);
        pixels[y][x] = FC.compress(val);
        delta[y][x] = FC.compress(val-old);
    }



    float getDel(uint32_t x, uint32_t y) {
        return FC.decompress(delta[y][x]);
    }



    #define ADVANCE 0.1f


    float boost(float val) {
        return val+0.5f;
    }
    float boost2(float val) {
        return val-0.5f;
    }
    
    void processPix(uint16_t x, uint16_t y) {
        // get block values
        float vals = 0;
        uint8_t vals_cnt = 0;
        float dels = 0;
        for (int iy = 0; iy < 3; iy++) {
            float ty = y+iy-1;
            if ( ty < 0 || ty >= PSIZE_Y ) {
                continue;
            }
            for (int ix = 0; ix < 3; ix++) {
                float tx = x+ix-1;
                if ( tx < 0 || tx >= PSIZE_X) {
                    continue;
                }
                vals += getVal(tx, ty);
                float tempDel = getDel(tx, ty);
                dels += tempDel * abs(tempDel);
                vals_cnt++;
            }
        }

        

        // compute shit


        float me = getVal(x, y);
        float avg = vals/vals_cnt;

        float ddd = dels;
        if ( ddd < 0 ) {
            ddd = -sqrt(-ddd);
        } else {
            ddd = sqrt(ddd);
        }

        float out;
        
        if ( me > 1 ) {
            out = 0.5f;
        } else if ( me < 0.3 ) {
            out = me + ADVANCE;
        } else {
            if ( ddd > 0 ) {
                out = me + ddd*.3f;
            } else {
                out = me + ddd*.9f;
            }
        }
        out = (5*out + avg)/6;
        
        //update value
        setVal(x, y, out);
    }


    #define fullUpdateEvery 3
    #define fullFrameUpdateSpeed 6
    int16_t curIdx = 0;
    uint16_t pixOrder[(PSIZE_Y * PSIZE_X) * fullUpdateEvery];
    uint16_t updatePerFrame = (PSIZE_Y*PSIZE_X*fullUpdateEvery) / fullFrameUpdateSpeed;

    void next_effect() {}

    void reset() {}
    
    
    public:
    void setup() {

        for (int y = 0; y < PSIZE_Y; y++) {
            for (int x = 0; x < PSIZE_X; x++) {
                setVal(x, y, random(255)/255.f);
            }
        }
/*
        for (int y = 0; y < 50; y++) {
            for (int x = 0; x < 50; x++) {
                setVal(x+PSIZE_X/2-25, y+PSIZE_Y/2-25, 1);
            }
        }
*/
        for (int i = 0; i < (PSIZE_Y * PSIZE_X) * fullUpdateEvery; i++) {
            pixOrder[i] = i%(PSIZE_Y * PSIZE_X);
        }
        
    }


    void loop() {
        static uint32_t phosphene_time = millis();
        if (millis() - 33 > phosphene_time) {
            phosphene_time = millis();

            if (curIdx >= (PSIZE_Y*PSIZE_X)) {
                //random.shuffle(pixOrder);
                for (int i = 0; i < (PSIZE_Y * PSIZE_X) * fullUpdateEvery; i++) {
                    uint16_t temp = pixOrder[i];
                    uint16_t r = random((PSIZE_Y * PSIZE_X) * fullUpdateEvery);
                    pixOrder[i] = pixOrder[r];
                    pixOrder[r] = temp;
                }
                curIdx = 0;
            }
            
            int16_t nextIdx = curIdx + updatePerFrame;
            while (curIdx < nextIdx && curIdx < (PSIZE_Y*PSIZE_X)) {
                uint16_t coord = pixOrder[curIdx];
                processPix(coord%PSIZE_X, coord/PSIZE_X);
                curIdx += 1;
            }

            
        }
    }
};   
