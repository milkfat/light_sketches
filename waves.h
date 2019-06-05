
//WAVES

class WAVES: public LIGHT_SKETCH {
    public:
        WAVES () {setup();}
        ~WAVES () {}
        private:
    uint8_t current_variation = 0;

    class pixel {
      public:
        int16_t spd = 0;
        int16_t height = 0;
        uint8_t cnt = 0;
    };

    pixel grid[HEIGHTMAP_WIDTH][HEIGHTMAP_HEIGHT];

  public:
    void next_effect() {
      current_variation++;
      current_variation %= 1;
    }

    void setup() {

    }

    void reset() {

    }
    unsigned long time0 = millis();
    unsigned long frame_time = millis();
    bool tap = false;
    void loop() {
      if (millis() - 16 > frame_time) {
        frame_time = millis();

        LED_show();
        LED_black();
        static int rx = 0;
        static int ry = 0;
        if ( millis() - 500 > time0 ) {

          tap = true;

          if ( millis() - 650 > time0 ) {
            tap = false;
            time0 = millis();
            rx = random(0, HEIGHTMAP_WIDTH);
            ry = random(0, HEIGHTMAP_HEIGHT);
          }
        }

        //update speeds (transfer forces between adjacent nodes)
        for (int x = 0; x < HEIGHTMAP_WIDTH; x++) {
          for (int y = 0; y < HEIGHTMAP_HEIGHT; y++) {

            int force = 0;

            if ( y < (HEIGHTMAP_HEIGHT - 1) ) {
              //top right
              if ( x > 0 ) {
                force += grid[x - 1][y + 1].height - grid[x][y].height;
              }
              //top
              force += grid[x][y + 1].height - grid[x][y].height;
              //top left
              if ( x < (HEIGHTMAP_WIDTH - 1) ) {
                force += grid[x + 1][y + 1].height - grid[x][y].height;
              }
            }


            if ( y > 0 ) {
              //bottom left
              if ( x > 0 ) {
                force += grid[x - 1][y - 1].height - grid[x][y].height;
              }
              //bottom
              force += grid[x][y - 1].height - grid[x][y].height;
              //bottom right
              if ( x < (HEIGHTMAP_WIDTH - 1) ) {
                force += grid[x + 1][y - 1].height - grid[x][y].height;
              }
            }


            //left
            if ( x > 0 ) {
              force += grid[x - 1][y].height - grid[x][y].height;
            }
            //right
            if ( x < (HEIGHTMAP_WIDTH - 1) ) {
              force += grid[x + 1][y].height - grid[x][y].height;
            }

            //centering force
            if (tap && x == rx && y == ry) {
              force += -30000 - grid[x][y].height;
            } else if (tap && abs(x - rx) <= 1 && abs(y - ry) <= 1) {
              force += -10000 - grid[x][y].height;
            } else {
              force += 0 - grid[x][y].height / 4;
            }

            for (uint8_t i = 0; i < NUM_POINTERS; i++) {
              if (pointers[i].down && abs(x - pointers[i].x) <= 1 && abs(y - pointers[i].y) <= 1) {
                force += -100 * pointers[i].pressure - grid[x][y].height;
              }
            }

            //dampen
            grid[x][y].spd *= .99;

            //add force
            grid[x][y].spd = grid[x][y].spd + force / 20;

          }
        }

        //update heights
        for (int x = 0; x < HEIGHTMAP_WIDTH; x++) {
          for (int y = 0; y < HEIGHTMAP_HEIGHT; y++) {
            grid[x][y].height = _min(_max(grid[x][y].height + grid[x][y].spd, -32768), 32767);

            //draw LED
            //drawXY(leds, x, y, 160, 255, _min(_max(grid[x][y].height*2, -128),127)+128);
            height_map[x][y] = grid[x][y].height;
          }
        }

        height_map_to_LED();
        
        for (int i = 0; i < NUM_LEDS; i++) {
          led_mask2[i]=255;
        }
        
      }
    }


};

LIGHT_SKETCHES::REGISTER<WAVES> waves("waves");