
//TEST2D

class TEST2D: public LIGHT_SKETCH {
  public:
    TEST2D () {setup();}
    ~TEST2D() {}

  private:
#define NUM_BALLS_TEST2D 150
    unsigned long frame_time = 0;
    int ballpos = 0;
    int amp = 255;
    int amp_rising = 0;
#define NUM_EFFECTS 5
    uint8_t current_effect = 0;
    int spiral_speed = 0;
    uint8_t fade_rate = 255;

    class BALL {


      public:

        long ypos = -3 * 255;
        long xpos = -3 * 255;
        int yvel = 0;
        int xvel = 0;
        uint8_t hue = 0;
        uint8_t sat = 255;

        void upd() {
          xpos += xvel;
          ypos += yvel;
          if (filter_particles == 1 ) {
            blendXY(leds, xpos, ypos, hue, sat, 255);
          } else {

            drawXY_fine(leds, xpos, ypos, hue, sat, 255);
          }
          //        if (xpos >= (MATRIX_WIDTH-1)*255L-xvel) {
          //          xvel = -abs(xvel);
          //        }
          //        if (xpos <= 0+abs(xvel)) {
          //          xvel = abs(xvel);
          //        }
          //if (ypos >= (MATRIX_HEIGHT-1)*255L-yvel) {
          //  yvel = -abs(yvel);
          //}
          //if (ypos <= 0+abs(yvel)) {
          //  yvel = abs(yvel);
          //}
        }

        void reset(int x, int y) {
          ypos = y;
          xpos = x;
          yvel = random(5, 128);
          if (random(2) == 1) {
            yvel *= -1;
          }
          xvel = random(5, 128);
          if (random(2) == 1) {
            xvel *= -1;
          }
          hue = random(255);
        }




    };

    BALL balls[NUM_BALLS_TEST2D];

  public:
    void reset() {
      int x = random(255, (MATRIX_WIDTH - 2) * 255L);
      int y = random(255, (MATRIX_HEIGHT - 2) * 255L);
      for (int i = ballpos; i < ballpos + 10 && i < NUM_BALLS_TEST2D; i++) {
        balls[i].reset(x, y);
      }
      ballpos += 10;
      ballpos %= NUM_BALLS_TEST2D;
    }

    void audio_wave() {
      static uint8_t angle = 0;
      static uint8_t angle_speed = 32;
      for (int i = 0; i < 2; i++) {
        balls[ballpos].ypos = (MATRIX_HEIGHT * 255L) / 2;
        balls[ballpos].xpos = (7 * 255) / 2 + ((sin8(angle) - 127) * 7L * amp) / 255L;
        balls[ballpos].xvel = 0;
        balls[ballpos].yvel = 255;
        if (i == 1) {
          balls[ballpos].yvel = -255;
        }
        balls[ballpos].ypos -= balls[ballpos].yvel / 2;
        balls[ballpos].hue = 160;
        balls[ballpos].sat = 255;
        ballpos++;
        ballpos %= NUM_BALLS_TEST2D;


      }
      if (amp_rising == 0) {
        amp *= .90;
      }
      angle += angle_speed;
    }

    void bars() {
      for (int i = 0; i < MATRIX_WIDTH; i++) {
        for (int j = 0; j < 8; j++) {
          balls[ballpos].xpos = i * 256L;
          balls[ballpos].ypos = (MATRIX_HEIGHT * 256L) / 2;
          balls[ballpos].xvel = 0;
          balls[ballpos].yvel = 1024;
          if (j % 2 == 1) {
            balls[ballpos].yvel *= -1;
          }
          balls[ballpos].ypos -= balls[ballpos].yvel * (j / 2) / 4;
          balls[ballpos].hue = random(255);
          balls[ballpos].sat = 0;
          ballpos++;
          ballpos %= NUM_BALLS_TEST2D;
        }
      }
    }

    void spiral() {
      for (int i = 0; i < 4; i++) {
        static uint8_t angle = 0;
        balls[ballpos].xpos = (MATRIX_WIDTH * 255L) / 2;
        balls[ballpos].ypos = (MATRIX_HEIGHT * 255L) / 2;
        balls[ballpos].xvel = (sin8(angle) - 127) * 4;
        balls[ballpos].yvel = (cos8(angle) - 127) * 4;
        balls[ballpos].hue = random(255);
        balls[ballpos].sat = 255;
        angle += 1 + (8 * spiral_speed) / 255;
        ballpos++;
        ballpos %= NUM_BALLS_TEST2D;
      }
      spiral_speed *= .90;
    }

    void bubble() {
      //radius
      uint8_t r = random(1, 10);
      //speed
      int v = random(60, 120);
      int x = random(0, (MATRIX_WIDTH - 1) * 255L);
      int y = -1 - r * 127;
      for (int i = 0; i < 16; i++) {
        balls[ballpos].xvel = sin8(i * 16) - 127;
        balls[ballpos].yvel = cos8(i * 16) - 127;
        balls[ballpos].xpos = x + balls[ballpos].xvel * r;
        balls[ballpos].ypos = y + balls[ballpos].yvel * r;
        balls[ballpos].xvel = 0;
        balls[ballpos].yvel = v;
        balls[ballpos].hue = 160;
        balls[ballpos].sat = 255;
        ballpos++;
        ballpos %= NUM_BALLS_TEST2D;
      }
    }

    void next_effect() {
      current_effect += 1;
      current_effect %= NUM_EFFECTS;
    }

    void setup() {
      reset();
    }

    void loop() {
      fade_rate = 255;
      if (effect_beat == 1) {
        effect_beat = 0;
        amp_rising = 1;
        if (current_effect == 0) {
          spiral_speed = 255;
        } else if (current_effect == 2 ) {
          reset();
        } else if (current_effect == 3) {
          bubble();
        } else if (current_effect == 4) {
          bars();
        }
      }

      if (amp_rising == 1) {
        if (amp < 255) {
          amp += 64;
        } else {
          amp = 255;
          amp_rising = 0;
        }

      }
      if (millis() - 16 > frame_time) {
        frame_time = millis();

        if (current_effect == 0) {
          spiral();
        } else if (current_effect == 1) {
          audio_wave();
        } else if (current_effect == 4) {
          fade_rate = 128;
        }

        for (int i = 0; i < NUM_BALLS_TEST2D; i++) {
          balls[i].upd();
        }


        LED_show();
        LED_black();

        for (int i = 0; i < NUM_LEDS; i++) //blank string
        {
          leds[i].fadeToBlackBy( fade_rate );
        }

      }
    }

};


LIGHT_SKETCHES::REGISTER<TEST2D> test2d("test2d");
//END TEST2D
