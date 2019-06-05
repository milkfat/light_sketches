//NEON

#if MATRIX_WIDTH > MATRIX_HEIGHT
#define GRID_WIDTH MATRIX_HEIGHT
#define GRID_HEIGHT MATRIX_WIDTH
#else
#define GRID_WIDTH MATRIX_WIDTH
#define GRID_HEIGHT MATRIX_HEIGHT
#endif

class NEON: public LIGHT_SKETCH {
  public:
    NEON () {setup();}
    ~NEON () {}
#define NUM_POINTS 24


  private:
    long x;
    long y;
    uint8_t b;
    int w;
    uint8_t gb;
    uint8_t spd;
    long x0;
    long y0;
    long x1;
    long y1;
    long x2;
    long y2;
    long x3;
    long my_points[NUM_POINTS][2];



    //POINT points[NUM_POINTS];

  public:

    void next_effect() {
    }

    void setup() {
      for (int i = 0; i < NUM_POINTS; i++) {
        
        my_points[i][0] = GRID_WIDTH * 256L / 2;
        my_points[i][1] = (GRID_HEIGHT * 256L * -2) / 20 + (GRID_HEIGHT * 256L * i) / 20;

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






      //CALCULATE POINTS


      for (int i = 0; i < NUM_POINTS; i++) {
        //movement in horizonal direction


        //find next target
        int temp = 0;
        while (temp == 0) {
          temp = 1;
          int distx = 0;
          //add movement in a random direction
          for (int s = 0; s < 10; s++) {
            distx += random(101) - 50;
          }

          long newx = my_points[i][0] + distx;

          //calculate moving average to get smoother movement
          //newx = my_my_points[i][0] + (newx - my_my_points[i][0]) / 2;


          if (newx > (GRID_WIDTH - 1) * 256L) {
            newx = (GRID_WIDTH - 1) * 256L;
          }
          if (newx < 0) {
            newx = 0;
          }


          //vertical movement
          long disty = 0;
          for (int s = 0; s < 10; s++) {
            disty += random(101) - 50;
          }
          long newy = my_points[i][1] + disty;
          //calculate moving average to get smoother movement
          //newy = my_newy + (newy - my_newy) / 2;

          //constrain vertical height
          newy = my_points[i][1] + disty;

          if (newy >  (GRID_HEIGHT * 256L * -2) / 20 + (GRID_HEIGHT * 256L * i) / 20 + 5 * 256L) {
            newy =  (GRID_HEIGHT * 256L * -2) / 20 + (GRID_HEIGHT * 256L * i) / 20 + 5 * 256L;
          }
          if (newy <  (GRID_HEIGHT * 256L * -2) / 20 + (GRID_HEIGHT * 256L * i) / 20 - 5 * 256L) {
            newy =  (GRID_HEIGHT * 256L * -2) / 20 + (GRID_HEIGHT * 256L * i) / 20 - 5 * 256L;
          }


          if (i > 0 && newy < my_points[i - 1][1]) {
            //do not allow points to cross under a lower point
            if ( ( ( newx - my_points[i - 1][0] ) > 0 ) != ( ( (newx - distx) - my_points[i - 1][0] ) > 0 ) ) {
              continue;
            }
            //do not allow points within three pixels if they are under a lower point
            else if ( abs(newx - my_points[i - 1][0]) < 3 * 256) {
              continue;
            }
            //do not allow within 3 pixels of another point
            if (
              newx < my_points[i - 1][0] + 3 * 256L &&
              newx + 3 * 256L > my_points[i - 1][0] &&
              newy < my_points[i - 1][1] + 3 * 256L &&
              newy + 3 * 256L > my_points[i - 1][1]
            ) {
              continue;
            }
          }


          if (i < NUM_POINTS - 1 && newy > my_points[i + 1][1]) {
            //do not allow points to cross above a higher point
            if ( ( ( newx - my_points[i + 1][0] ) > 0 ) != ( ( (newx - distx) - my_points[i + 1][0] ) > 0 ) ) {
              continue;
            }
            //do not allow points within three pixels if they are over a higher point
            else if ( abs(newx - my_points[i + 1][0]) < 3 * 256L) {
              continue;
            }
            //do not allow a point near another point
            if (
              newx < my_points[i + 1][0] + 3 * 256L &&
              newx + 3 * 256L > my_points[i + 1][0] &&
              newy < my_points[i + 1][1] + 3 * 256L &&
              newy + 3 * 256L > my_points[i + 1][1]
            ) {
              continue;

            }

          }

          //do not allow points to get near a point that is 2 positions away
          if (i > 1 && newy - my_points[i - 2][1] < GRID_HEIGHT/21 * 256L) {
            continue;
          }

          if (i < NUM_POINTS - 2 && my_points[i + 2][1] - newy < GRID_HEIGHT/21 * 256L) {
            continue;
          }

          //move the point
          my_points[i][0] = newx;
          my_points[i][1] = newy;
          break;
        }

        //end calculating new target






        
      }
      //END CALCULATE POINTS

      //draw curve
      //swap X and Y axis if our display width is greater than the height
#if MATRIX_WIDTH > MATRIX_HEIGHT
      matt_curve(my_points, NUM_POINTS, default_color, default_saturation, 255, true, false, false);
#else
      matt_curve(my_points, NUM_POINTS, default_color, default_saturation, 255, false, false, false);
#endif
      

    }


};


LIGHT_SKETCHES::REGISTER<NEON> neon("neon");
//END NEON