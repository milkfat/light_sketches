
class PHOS {
    public:
    #define PSIZE_X 42
    #define PSIZE_Y 42
    #define SIZE_PIX 5


    #define FPS 60 // frames per second setting

    uint16_t pixels[PSIZE_Y][PSIZE_X];

    float getVal(const uint32_t& x,const uint32_t& y) {
        return FC.decompress(pixels[y][x]);
    }
    
    private:
    uint16_t delta[PSIZE_Y][PSIZE_X];

    void setVal(const uint32_t& x, const uint32_t& y, const float& val) {
        float old = getVal(x,y);
        pixels[y][x] = FC.compress(val);
        delta[y][x] = FC.compress(val-old);
    }

    float getDel(const uint32_t& x, const uint32_t& y) {
        return FC.decompress(delta[y][x]);
    }

    #define ADVANCE 0.1f
    
    void processPix(const uint32_t& x, const uint32_t& y) {
        // get block values
        float vals = 0;
        float vals_cnt = 0;
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
    int16_t curIdx = 0;
    uint16_t updatePerFrame = (PSIZE_Y*PSIZE_X) / fullFrameUpdateSpeed;

    void next_effect() {}

    void reset() {}
    
    
    public:
    void setup() {

        for (int y = 0; y < PSIZE_Y; y++) {
            for (int x = 0; x < PSIZE_X; x++) {
                pixels[y][x] = 0;
                delta[y][x] = 0;
                setVal(x, y, random(255)/255.f);
            }
        }
        
    }


    void loop() {
        static uint32_t phosphene_time = millis();
        if (millis() - 1000/FPS > phosphene_time) {
            phosphene_time = millis();

            curIdx = 0;
            while (curIdx < updatePerFrame) {
                processPix(random(PSIZE_X), random(PSIZE_Y));
                curIdx++;
            }

            
        }
    }
};   
