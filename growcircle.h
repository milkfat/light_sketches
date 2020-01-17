#ifndef LIGHTS_GROWCIRCLE_H
#define LIGHTS_GROWCIRCLE_H

//GROWING CIRCLE THING

class GROWCIRCLE: public LIGHT_SKETCH {
  public:
    GROWCIRCLE () {setup();}
    ~GROWCIRCLE () {}
  private:
#define MAX_LIGHT 40

    class CIRCLE_THING {
      public:
        uint8_t dists[NUM_LEDS]; //distance of this LED from the origination
        uint8_t done = 1;
        int stp0 = 0;
        int stp = 0;
        int current_width0 = 0;
        uint8_t current_width = 0;
        uint8_t current_hue = 0;
        uint8_t next_hue = 196;
        uint8_t level_min = 16;
        uint8_t level_max = 80;
        uint8_t next_level_min = 30;
        uint8_t next_level_max = 94;
        int x;
        int y;
        uint8_t order = 0;
        bool processed = false;

    };
#define SPARKLE_AMOUNT 5
#define NUM_CIRCLES_GROWCIRCLE 1
    CIRCLE_THING circles[NUM_CIRCLES_GROWCIRCLE];
    unsigned long time0 = millis();
    unsigned long time1 = millis();
    uint8_t next_circle = 0;
    uint16_t next_order = 0;


    uint8_t counter = 0; //counts 0-255,0-255,0-255,etc.
    uint8_t band_width = 10; //speed_multiplier * 10 is pretty good, lower = thinner, higher = fatter
    uint8_t flasher_zone = 50;
    uint16_t sparkle[SPARKLE_AMOUNT] = {0};
    uint8_t fps = 0;
    unsigned long time_fps = millis() + 1000;

    uint8_t hues[NUM_LEDS]; //current hue of this LED
    uint8_t sats[NUM_LEDS]; //current saturation of this LED
    uint8_t levels[NUM_LEDS]; //current brightness of this LED
    char level_deltas[NUM_LEDS]; //rate of change in brightness for this LED
    uint8_t flashers[NUM_LEDS]; //state of flashing for this LED


    //lines of LEDS with the x,y coordinates for each end
    int sects[6][6] =
    {
      {  0, 31, 60, 0,  60, 30},
      { 31, 23, 60, 30, 40, 40},
      { 54, 54, 40, 40, 0, 20},
      {108, 21,  0, 20, 0},
      {129, 71,  0, 0,  60, 0},
    };


  public:
    void reset() {

    }
    void next_effect() {

    }

    void setup() {


      //initial light levels
      for (int c = 0; c < NUM_CIRCLES_GROWCIRCLE; c++) {
        for (int i = 0; i < NUM_LEDS; i++) {
          levels[i] = random(circles[c].level_min, circles[c].level_max + 1);
          level_deltas[i] = random(1, 5);
          level_deltas[i] = (level_deltas[i] * level_deltas[i]) / 4;
          if (random(2) == 0) {
            level_deltas[i] *= -1;
          }
        }
      }


    }

    void loop() {

      for (uint8_t i = 0; i < NUM_POINTERS; i++) {
        if (pointers[i].new_press) {
          circles[next_circle + 1].x = pointers[i].x;
          circles[next_circle + 1].y = pointers[i].y;
          circles[next_circle + 1].done = 2; //done == 2 means this is a "pointer" circle (as opposed to the automatic circe)
          next_circle++;
          next_circle %= _max(NUM_CIRCLES_GROWCIRCLE - 1,1);
        }
      }

      for (uint8_t c = 0; c < NUM_CIRCLES_GROWCIRCLE; c++) {
        //draw a new expanding circle starting at a random point outside the string
        if ( (circles[c].done == 1 && c == 0) || circles[c].done == 2) {
          //give this circle a higher priority than previous circles
          circles[c].order = next_order;
          next_order++;
          circles[c].current_hue = circles[c].next_hue;
          circles[c].level_min = circles[c].next_level_min;
          circles[c].level_max = circles[c].next_level_max;
          circles[c].next_level_min = random(16, MAX_LIGHT + 1);
          circles[c].next_level_max = circles[c].next_level_min + 64;
          while (abs(circles[c].current_hue - circles[c].next_hue) < 48) {
            circles[c].next_hue = random(0, 256);
          }
          uint8_t r = random(0, 4);
          int x1 = random(-MATRIX_WIDTH, (MATRIX_WIDTH * 2));
          int y1 = random((MATRIX_HEIGHT / 4), (MATRIX_HEIGHT * 3 / 4));
          if (circles[c].done == 2) {
            x1 = circles[c].x;
            y1 = circles[c].y;
          }
          circles[c].stp0 = 0;
          circles[c].current_width0 = 0;
          circles[c].stp = 0;
          circles[c].current_width = 0;
          //figure out the distance of each pixel from the starting point
          for (uint8_t x2 = 0; x2 < MATRIX_WIDTH; x2++) {
            for (int y2 = 0; y2 < MATRIX_HEIGHT; y2++) {
              circles[c].dists[XY(x2, y2)] = sqrt(sq((x2 - x1) * 1L) + sq((y2 - y1) * 1L));
            }
          }

          circles[c].done = 0;
        }
      }

      //shade pixels according to their distance from starting point

        //PROCESS CIRCLES
        for (int i = 0; i < NUM_CIRCLES_GROWCIRCLE; i++) {
          circles[i].processed = false;
        }

        uint8_t processed_cnt = 0;

        while (processed_cnt < NUM_CIRCLES_GROWCIRCLE) {
          processed_cnt++;
          uint8_t highest_order = -1;
          int c = -1;
          for (uint8_t i = 0; i < NUM_CIRCLES_GROWCIRCLE; i++) {
            if (circles[i].done == 0 && !circles[i].processed && circles[i].order < highest_order) {
              highest_order = circles[i].order;
              c = i;
            }
          }

          if (c == -1) {
            break;
          }

          circles[c].processed = true;

          if (circles[c].done == 0) {
            circles[c].done = 1;
            
            for (int i = 0; i < NUM_LEDS; i++) {
              int distance = circles[c].dists[i] - circles[c].stp;
              uint8_t hue = hues[i];
              uint8_t low_bri = circles[c].level_min;
              uint8_t high_bri = circles[c].level_max;
              if (distance > -band_width * 15) {
                circles[c].done = 0;
              }

              //lower saturation to create a band of white
              uint8_t sat = 255;
              if (circles[c].current_width > 0) {
                sat = (_min(abs(distance), circles[c].current_width) * 255) / circles[c].current_width;
              }

              //smooth the change in saturation
              sats[i] = _max(sats[i] - (255 - ease8InOutQuad(sat)), 0);
              //make the white band a bit brighter
              //uint8_t bri = (255 - sat) / 2;

              //change hue on pass
              if (distance < 0) {
                hues[i] = circles[c].next_hue;
                sats[i] = (sats[i] + ease8InOutQuad(sat)) / 2;
                low_bri = circles[c].next_level_min;
                high_bri = circles[c].next_level_max;

                //flashers trail behind the band

                //add a random flashes to create a trail behind the moving band
                if (-distance > 0 && -distance < flasher_zone) {

                int16_t flasher_odds = abs(distance);
                flasher_odds *= flasher_odds;
                  
                  //uint8_t flasher_done = 0;
                  uint8_t recent_flash = 0;

                  if ( random(flasher_odds) == 0) {
                    //Avoid flashing multiple side-by-side LEDs simultaneously.
                    if (recent_flash == 0) {
                      recent_flash = 1;
                      flashers[i] = ((flasher_zone + distance) * 128) / (flasher_zone - 5 ) + 128;
                    } else {
                      recent_flash = 0;
                    }
                    //flashers[i] = 255;
                    //flasher_done = 1;
                  }
                }


              }



              //give the LEDS some differences in brightness
              if (counter % 2 == 1) {
                if (distance == -1) {
                  levels[i] = random(low_bri, high_bri + 1);
                } else {
                  levels[i] = _max(levels[i] + level_deltas[i], 0);
                }
                if (levels[i] <= low_bri && level_deltas[i] < 0) {
                  level_deltas[i] = random(1, 5);

                } else if (levels[i] >= high_bri && level_deltas[i] > 0) {
                  level_deltas[i] = random(1, 5);
                  level_deltas[i] *= -1;
                }
              }
            }

            //move the circle "out" one step
            if ( circles[c].current_width < band_width ) {
              circles[c].current_width0+=128;
              circles[c].current_width = circles[c].current_width0/256;
            } else {
              circles[c].stp0+=128;
              circles[c].stp = circles[c].stp0/256;
            }
          }
        }

        counter++;

        for (int i = 0; i < NUM_LEDS; i++) {
          CRGB rgb;
          uint8_t bri = (255 - sats[i]) / 2;
          hsv2rgb_rainbow(CHSV(hues[i], sats[i], _min(levels[i] + bri, 255)), rgb);
          leds[adjust(i)] = rgb;
          sats[i] = 255;
        }
        //END PROCESSING OF CIRCLES




        //draw flashers
        for (int i = 0; i < NUM_LEDS; i++) {
          if (flashers[i] > 0) {
            CRGB rgb;
            hsv2rgb_rainbow(CHSV(0, 0, flashers[i]), rgb);
            leds[adjust(i)] += rgb;
            flashers[i] = (flashers[i] * 9) / 10;
          }
        }

        for (int i = 0; i < SPARKLE_AMOUNT; i++) {
          levels[sparkle[i]] = _min(levels[sparkle[i]] + 64, 255);
          if (levels[sparkle[i]] == 255) {
            level_deltas[sparkle[i]] = -16;
            sparkle[i] = random(NUM_LEDS);
          }
        }

        LED_show();
        LED_black();

      


    }
};

LIGHT_SKETCHES::REGISTER<GROWCIRCLE> growcircle("growcircle");
//END GROWING CIRCLE THING

#endif