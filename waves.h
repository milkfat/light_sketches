#ifndef LIGHTS_WAVES_H
#define LIGHTS_WAVES_H

//WAVES

class WAVES: public LIGHT_SKETCH {

  public:
    WAVES () {setup();}
    ~WAVES () {}
  
  private:

  int threshold = -128*256;
  int light_x = 100;
  int light_y = 100;
  int spec_x = 15;
  int spec_y = 15;

  int16_t height_map[HEIGHTMAP_HEIGHT][HEIGHTMAP_WIDTH];

  uint8_t current_variation = 0;
  class pixel {
    public:
      int16_t spd = 0;
      int16_t height = 0;
      uint8_t cnt = 0;
  };

  pixel grid[HEIGHTMAP_HEIGHT][HEIGHTMAP_WIDTH];

  public:

    void next_effect() {
      current_variation++;
      current_variation %= 1;
    }

    void setup() {
      height_map_ptr = &height_map;
      control_variables.add(threshold,"Threshold",-256*256,256*256);
      control_variables.add(light_x,"Light X",-255,255);
      control_variables.add(light_y,"Light Y",-255,255);
      control_variables.add(spec_x,"Specular X",-255,255);
      control_variables.add(spec_y,"Specular Y",-255,255);
    }

    void reset() {

    }
    unsigned long time0 = millis();
    unsigned long frame_time = millis();
    bool tap = false;
    uint8_t phase = 0;
    void loop() {
      if (millis() - 16 > frame_time) {
        frame_time = millis();

        LED_show();
        LED_black();
        static int rx = random(0, HEIGHTMAP_WIDTH);
        static int ry = random(0, HEIGHTMAP_HEIGHT);
        if ( millis() > time0 ) {

          tap = true;

        }

        uint8_t calc_cnt = 0;

        while (calc_cnt != 3) {
          calc_cnt++;

          static uint8_t damp_cnt = 0;
          damp_cnt++;
          damp_cnt %= 4;

          //update speeds (transfer forces between adjacent nodes)
          for (int x = 0; x < HEIGHTMAP_WIDTH; x++) {
            for (int y = 0; y < HEIGHTMAP_HEIGHT; y++) {

              int force = 0;

              if ( y < (HEIGHTMAP_HEIGHT - 1) ) {
                //top right
                if ( x > 0 ) {
                  force += grid[y + 1][x - 1].height - grid[y][x].height;
                }
                //top
                force += grid[y + 1][x].height - grid[y][x].height;
                //top left
                if ( x < (HEIGHTMAP_WIDTH - 1) ) {
                  force += grid[y + 1][x + 1].height - grid[y][x].height;
                }
              }


              if ( y > 0 ) {
                //bottom left
                if ( x > 0 ) {
                  force += grid[y - 1][x - 1].height - grid[y][x].height;
                }
                //bottom
                force += grid[y - 1][x].height - grid[y][x].height;
                //bottom right
                if ( x < (HEIGHTMAP_WIDTH - 1) ) {
                  force += grid[y - 1][x + 1].height - grid[y][x].height;
                }
              }


              //left
              if ( x > 0 ) {
                force += grid[y][x - 1].height - grid[y][x].height;
              }
              //right
              if ( x < (HEIGHTMAP_WIDTH - 1) ) {
                force += grid[y][x + 1].height - grid[y][x].height;
              }

              //centering force
              if (tap && x == rx && y == ry) {
                static int previous_force_mag = 0;
                int force_mag = abs(force);

                //phase 0 = neighbor cells are accelerating downward
                if (phase == 0 && force_mag < previous_force_mag) {
                  phase = 1;
                }

                //phase 1 = neighbor cells are decelerating downward
                if ( phase == 1 && force_mag > previous_force_mag) {
                  //neighbor cells have reached their lowest point and have started moving upward
                  //we remove our "tap"
                  tap = false;
                  time0 = millis() + random(1000);
                  rx = random(0, HEIGHTMAP_WIDTH);
                  ry = random(0, HEIGHTMAP_HEIGHT);
                  previous_force_mag = 0;
                  phase = 0;
                } else if (phase!=2 ) {
                  force += -100000 - grid[y][x].height;
                  previous_force_mag = force_mag;
                }


              } else {
                force += 0 - grid[y][x].height / 12;
              }

              for (uint8_t i = 0; i < NUM_POINTERS; i++) {
                if (pointers[i].down && abs(x - pointers[i].x) <= 1 && abs(y - pointers[i].y) <= 1) {
                  force += -100 * pointers[i].pressure - grid[y][x].height;
                }
              }

              //dampen
              if (!damp_cnt) {
                grid[y][x].spd *= .99;
              }

              //add force
              grid[y][x].spd = grid[y][x].spd + force / 100;

            }
          }

          for (int x = 0; x < HEIGHTMAP_WIDTH; x++) {
            for (int y = 0; y < HEIGHTMAP_HEIGHT; y++) {
              grid[y][x].height = _min(_max(grid[y][x].height + grid[y][x].spd, -32768), 32767);
            }
          }

        }

        //update heights
        for (int x = 0; x < HEIGHTMAP_WIDTH; x++) {
          for (int y = 0; y < HEIGHTMAP_HEIGHT; y++) {

            //draw LED
            //drawXY(led_screen, x, y, 160, 255, _min(_max(grid[x][y].height*2, -128),127)+128);
            height_map[y][x] = grid[y][x].height;
          }
        }

        //height_map_to_LED();
        height_map_to_LED(threshold, light_x, light_y, spec_x, spec_y);
        
        for (int i = 0; i < NUM_LEDS; i++) {
          //led_mask2[i]=255;
        }
        
      }
    }


};

LIGHT_SKETCHES::REGISTER<WAVES> waves("waves");

#endif