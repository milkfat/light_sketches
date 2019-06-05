//CURVY

#if MATRIX_WIDTH > MATRIX_HEIGHT
#define GRID_WIDTH MATRIX_HEIGHT
#define GRID_HEIGHT MATRIX_WIDTH
#else
#define GRID_WIDTH MATRIX_WIDTH
#define GRID_HEIGHT MATRIX_HEIGHT
#endif

class CURVY: public LIGHT_SKETCH {
  public:
    CURVY () {setup();}
    ~CURVY () {}
#define NUM_POINTS 10
#define NUM_THINGS 10


  private:
    long my_points[NUM_THINGS][NUM_POINTS][2];



    //POINT points[NUM_POINTS];

  public:

    void next_effect() {
    }

    void setup() {
        for (int i = 0; i < NUM_THINGS; i++) {
            int h = (random(32)+150);
            for (int j = 0; j < NUM_POINTS; j++) {
                my_points[i][j][1] = j * (MATRIX_HEIGHT*h)/NUM_POINTS;
            }
        }
    }

    void reset() {
      setup();
      for (int i = 0; i < NUM_LEDS; i++)
      {
        leds[i] = CRGB::Black;
      }

    }
    void loop() {

      LED_show();
      for (int i = 0; i < NUM_LEDS; i++)
      {
        leds[i] = CRGB::Black;
      }
      
      static int y = 0;
      y-=2;
      for (int i = 0; i < NUM_THINGS; i++) {
        int offset = fmix32(i)%(MATRIX_WIDTH*256) + 768;
        int speed = fmix32(i)%(64);
        for (int j = 0; j < NUM_POINTS; j++) {
          my_points[i][j][0] = MATRIX_WIDTH*(_min(_max(inoise16(y*2, my_points[i][j][1]*4+y*(128+speed), i * 65536)/128,0),512)-256);
          my_points[i][j][0] = (my_points[i][j][0] * _min(j,NUM_POINTS)) / NUM_POINTS;
          my_points[i][j][0] += offset;

        }
          
          //draw curve
          //swap X and Y axis if our display width is greater than the height
          uint8_t b = (i*255)/NUM_THINGS;
          #if MATRIX_WIDTH > MATRIX_HEIGHT
          matt_curve8(my_points[i], NUM_POINTS, 96, default_saturation, (b*b)/255, true, false, true);
          #else
          matt_curve8(my_points[i], NUM_POINTS, 96, default_saturation, (b*b)/255, false, false, true);
          #endif
      }

      



    }


};


LIGHT_SKETCHES::REGISTER<CURVY> curvy("curvy");
//END CURVY