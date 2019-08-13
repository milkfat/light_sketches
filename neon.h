#ifndef LIGHTS_NEON_H
#define LIGHTS_NEON_H

//NEON

#if MATRIX_WIDTH > MATRIX_HEIGHT
#define NEON_GRID_WIDTH MATRIX_HEIGHT
#define NEON_GRID_HEIGHT MATRIX_WIDTH
#else
#define NEON_GRID_WIDTH MATRIX_WIDTH
#define NEON_GRID_HEIGHT MATRIX_HEIGHT
#endif

class NEON: public LIGHT_SKETCH {
  public:
    NEON () {setup();}
    ~NEON () {}
#define NUM_POINTS_NEON 24


  private:
    int32_t x;
    int32_t y;
    uint8_t b;
    int w;
    uint8_t gb;
    uint8_t spd;
    int32_t x0;
    int32_t y0;
    int32_t x1;
    int32_t y1;
    int32_t x2;
    int32_t y2;
    int32_t x3;
    int32_t my_points[NUM_POINTS_NEON][2];



    //POINT points[NUM_POINTS_NEON];

  public:

    void next_effect() {
    }

    void setup() {
      for (int i = 0; i < NUM_POINTS_NEON; i++) {
        
        my_points[i][0] = NEON_GRID_WIDTH * 256L / 2;
        my_points[i][1] = (NEON_GRID_HEIGHT * 256L * -2) / 20 + (NEON_GRID_HEIGHT * 256L * i) / 20;

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


      for (int i = 0; i < NUM_POINTS_NEON; i++) {
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

          int32_t newx = my_points[i][0] + distx;

          //calculate moving average to get smoother movement
          //newx = my_my_points[i][0] + (newx - my_my_points[i][0]) / 2;


          if (newx > (NEON_GRID_WIDTH - 1) * 256L) {
            newx = (NEON_GRID_WIDTH - 1) * 256L;
          }
          if (newx < 0) {
            newx = 0;
          }


          //vertical movement
          int32_t disty = 0;
          for (int s = 0; s < 10; s++) {
            disty += random(101) - 50;
          }
          int32_t newy = my_points[i][1] + disty;
          //calculate moving average to get smoother movement
          //newy = my_newy + (newy - my_newy) / 2;

          //constrain vertical height
          newy = my_points[i][1] + disty;

          if (newy >  (NEON_GRID_HEIGHT * 256L * -2) / 20 + (NEON_GRID_HEIGHT * 256L * i) / 20 + 5 * 256L) {
            newy =  (NEON_GRID_HEIGHT * 256L * -2) / 20 + (NEON_GRID_HEIGHT * 256L * i) / 20 + 5 * 256L;
          }
          if (newy <  (NEON_GRID_HEIGHT * 256L * -2) / 20 + (NEON_GRID_HEIGHT * 256L * i) / 20 - 5 * 256L) {
            newy =  (NEON_GRID_HEIGHT * 256L * -2) / 20 + (NEON_GRID_HEIGHT * 256L * i) / 20 - 5 * 256L;
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


          if (i < NUM_POINTS_NEON - 1 && newy > my_points[i + 1][1]) {
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
          if (i > 1 && newy - my_points[i - 2][1] < NEON_GRID_HEIGHT/21 * 256L) {
            continue;
          }

          if (i < NUM_POINTS_NEON - 2 && my_points[i + 2][1] - newy < NEON_GRID_HEIGHT/21 * 256L) {
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
      matt_curve(my_points, NUM_POINTS_NEON, default_color, default_saturation, 255, true, false, false);
#else
      matt_curve(my_points, NUM_POINTS_NEON, default_color, default_saturation, 255, false, false, false);
#endif
      

    }


};


LIGHT_SKETCHES::REGISTER<NEON> neon("neon");
//END NEON

#endif