

//SHAPES

class SHAPES: public LIGHT_SKETCH {
    public:
        SHAPES () {setup();}
    ~SHAPES () {}
    
    private:
#define NUM_CIRCLES_SHAPES (MATRIX_HEIGHT/18)

    class CIRCLE {
      public:
        long y;
        uint8_t stp;
    };


  private:
    uint8_t amp = 64;
    long x1;
    long y1;
    long x2;
    long y2;
    uint8_t angle = 0;
    uint8_t angle_speed = 30;
    uint8_t current_variation = 2;

    CIRCLE circles[NUM_CIRCLES_SHAPES];



  public:

    void reset() {

    }


    void next_effect() {
      current_variation++;
      current_variation %= 3;
    }

    void setup() {
      amp = 64;
      x1 = 0;
      y1 = 128 * 256L;
      x2 = (7 * 256L) / 2;
      y2 = 0;

      for (int i = 0; i < NUM_CIRCLES_SHAPES; i++) {
        circles[i].y = (MATRIX_HEIGHT * 256L * i) / (NUM_CIRCLES_SHAPES - 1);
        circles[i].stp = (i * 256) / NUM_CIRCLES_SHAPES;
      }
      reset_circle_angles();
    }

    void vibrate() {
      draw_line_fine(leds, x1, y1, x2, y2);


      if (effect_beat == 1) {
        effect_beat = 0;
        amp = 255;
      }

      x1 = 7 * 256L / 2 + ((sin8(angle) - 128L) * 5L * 256L * amp) / (256L * 128);

      if (amp > 1) {
        amp *= .98;
      }
      angle += angle_speed;





    }

    void circle_effect() {
      for (int i = 0; i < NUM_CIRCLES_SHAPES; i++) {
        uint8_t stp2 = sin8(circles[i].stp);
        draw_circle_fine(((MATRIX_WIDTH-1) * 256L) / 2, circles[i].y, (((MATRIX_HEIGHT*256L) / ((NUM_CIRCLES_SHAPES)*2)) * stp2) / 256);
        circles[i].y += 25;
        circles[i].stp += 4;
        if ( circles[i].y > (MATRIX_HEIGHT + 4) * 256 ) {
          int j = i+1;
          if (j > NUM_CIRCLES_SHAPES-1) {
            j = 0;
          }
          circles[i].y = circles[j].y - (MATRIX_HEIGHT*256L) / (NUM_CIRCLES_SHAPES - 1);
        }
      }

    }

    void test_curve() {

      //for (int i = 0; i < 3; i++) {
        long my_points[5][2];

        my_points[0][0] = MATRIX_WIDTH*198;
        my_points[0][1] = 5*256;
        my_points[1][0] = MATRIX_WIDTH*128;
        my_points[1][1] = 30*256;
        my_points[2][0] = MATRIX_WIDTH*128;
        my_points[2][1] = 60*256;
        my_points[3][0] = MATRIX_WIDTH*18;
        my_points[3][1] = 70*256;
        my_points[4][0] = MATRIX_WIDTH*208;
        my_points[4][1] = 150*256;

        static uint16_t mod = 0;
        uint16_t l = 34*256;
        mod+=32;
        my_points[2][0] += l*sin16(mod)/32768;
        my_points[2][1] += l*cos16(mod)/32768;

        matt_curve8(my_points, 5, 96, default_saturation, 255, false, true, true);
      //}




    }

    void loop() {

      if (current_variation == 0) {
        vibrate();
      } else if (current_variation == 1) {
        circle_effect();
      } else if (current_variation == 2) {
        test_curve();
      }


      LED_show();
      LED_black();

    }


};


LIGHT_SKETCHES::REGISTER<SHAPES> shapes("shapes");

//END SHAPES