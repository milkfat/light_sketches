//FIRE

//this class uses malloc for the grid, because it gets HUGE with large displays
//128x32 uses ~213KB
//should probaby optimize that somehow
#define FIRE_GRID_WIDTH (MATRIX_WIDTH/2)
#define FIRE_GRID_HEIGHT (MATRIX_HEIGHT)

#define FIRE_GRID_VELOCITY_MIN -120000
#define FIRE_GRID_VELOCITY_MAX 120000
#define FIRE_GRID_MIN INT16_MIN
#define FIRE_GRID_MAX INT16_MAX

class FIRE2: public LIGHT_SKETCH {
    public:
      FIRE2 () {setup();}
      ~FIRE2 () {}

    private:
    uint16_t* order;
    uint8_t current_variation = 1;
    uint8_t grid_calcs = 1;
    uint8_t velocity_calcs = 1;
    uint8_t density_calcs = 1;
    uint8_t heat_acceleration = 1;
    uint8_t draw_flame = 0;
    uint8_t draw_fuel = 1;
    uint8_t draw_air = 0;
    uint8_t draw_smoke = 0;
    uint8_t buffer_mod = 0;
    uint8_t smoke_divisor = 1;

    bool flame_on = false;

    struct pixel {
        int16_t smoke;
        int16_t fuel;
        int16_t air;
        cint18 vx;
        cint18 vy;
    };

    pixel grid[FIRE_GRID_HEIGHT][FIRE_GRID_WIDTH];

    void velocity_transfer(pixel& source_cell, pixel& target_cell, int32_t& moved_fuel, int32_t& moved_air, int32_t& moved_smoke, int32_t& moved_total, const bool& debug = false ) { 
      int32_t existing_total = target_cell.fuel + target_cell.air + target_cell.smoke;
      if (moved_total + existing_total == 0) {
        return; //avoid divide by zero
      }
      int32_t total = moved_total + existing_total;
      // if (debug) {
      //   std::cout << "VB tvy: " << target_cell.vy
      //     << " tvx: " << target_cell.vx
      //     << " tf: " << target_cell.fuel
      //     << " ta: " << target_cell.air
      //     << " ts: " << target_cell.smoke
      //     << " ms: " << moved_smoke
      //     << " mf: " << moved_fuel
      //     << " ma: " << moved_air
      //     << " mt: " << moved_total
      //     << " et: " << existing_total
      //     << "\n";
      // }
      target_cell.smoke += moved_smoke;
      target_cell.fuel += moved_fuel;
      target_cell.air += moved_air;
      target_cell.vx = ((source_cell.vx * moved_total) + (target_cell.vx * existing_total))/total;
      target_cell.vy = ((source_cell.vy * moved_total) + (target_cell.vy * existing_total))/total;
      //source_cell.vx *= .998f;
      //source_cell.vy *= .998f;
      // if (debug) {
      //   std::cout << "VA tvy: " << target_cell.vy
      //     << " tvx: " << target_cell.vx
      //     << " tf: " << target_cell.fuel
      //     << " ta: " << target_cell.air
      //     << " ts: " << target_cell.smoke
      //     << "\n";
      // }
    }

    void pull_particles(pixel& source_cell, pixel& target_cell, int32_t& pressure, const int& x_dir, const int& y_dir) {

      //find the total of the target cell
      int32_t target_cell_total = target_cell.fuel + target_cell.air + target_cell.smoke;
      //float ratio = (pressure+0.f)/target_cell_total;

      if ( target_cell_total > pressure && target_cell_total != 0 ) {
        int32_t smoke = (pressure * target_cell.smoke)/target_cell_total;
        int32_t fuel = (pressure * target_cell.fuel)/target_cell_total;
        int32_t air = (pressure * target_cell.air)/target_cell_total;
        
        //record the removed particles
        target_cell.smoke -= smoke;
        target_cell.fuel -= fuel;
        target_cell.air -= air;
        
        //find the total number of moved particles
        int32_t moved_total = fuel+air+smoke;

        //add the moved particles to the source cell, calculate velocities
        int32_t existing_total = source_cell.fuel + source_cell.air + source_cell.smoke;
        int32_t total = moved_total + existing_total;
        if (total != 0) {
          source_cell.fuel += fuel;
          source_cell.air += air;
          source_cell.smoke += smoke;
          source_cell.vx = ((source_cell.vx * existing_total) + (target_cell.vx * moved_total))/total;
          source_cell.vy = ((source_cell.vy * existing_total) + (target_cell.vy * moved_total))/total;

          //add a bit of velocity to the target cell (sucking effect?)
          target_cell.vx += moved_total * x_dir;
          target_cell.vy += moved_total * y_dir;
        }
        
      }
    }

    void pull_air(pixel& source_cell, const int32_t& pressure) {
      //pull air from off the grid
      int32_t existing_total = source_cell.fuel + source_cell.air + source_cell.smoke;
      int32_t total = pressure + existing_total;
      if (total != 0) {
        source_cell.vx = (source_cell.vx * existing_total)/total;
        source_cell.vy = (source_cell.vy * existing_total)/total;
      }
      source_cell.air += pressure;
    }

    #define NUM_GAS 20
    struct GAS {
      int32_t x = -1;
      int32_t y = -1;
      int32_t vx = 50;
      int32_t vy = 100;
      int32_t fuel = 10;
      int32_t air = 5;
    };

    GAS gas[NUM_GAS];

    struct FLAMETHROWER_CYCLE {
      bool active = false;
      bool fuel_on = false;
      uint8_t y_from = 0;
      uint8_t y_to = 0;
    };
    #define NUM_FLAMETHROWER_CYCLES 10
    FLAMETHROWER_CYCLE flamethrower_cycles[NUM_FLAMETHROWER_CYCLES];
    uint8_t current_flamethrower_cycle = 0;
    FLAMETHROWER_CYCLE * get_current_flame() {
      current_flamethrower_cycle++;
      current_flamethrower_cycle%=NUM_FLAMETHROWER_CYCLES;
      return &flamethrower_cycles[current_flamethrower_cycle];
    }

  public:
    void next_effect() {
      current_variation++;
      //change some of the grid physics based on which effect we are drawing
      //0 = candle
      //1 = roman candle
      //2 = flamepot / flamethrower
      current_variation %= 4;
      if (current_variation == 3) {
        grid_calcs = 2;
        velocity_calcs = 2;
        heat_acceleration = 1;
        draw_fuel = 1;
        draw_flame = 0;
        buffer_mod = 1;
        draw_air = 1;
        draw_smoke = 1;
        smoke_divisor = 255;
      } else if (current_variation == 2) {
        grid_calcs = 1;
        velocity_calcs = 1;
        heat_acceleration = 1;
        draw_fuel = 1;
        draw_flame = 0;
        buffer_mod = 0;
        draw_air = 0;
        draw_smoke = 0;
        smoke_divisor = 255;
      } else if (current_variation == 1) {
        grid_calcs = 1;
        velocity_calcs = 2;
        heat_acceleration = 1;
        draw_fuel = 0;
        draw_flame = 1;
        buffer_mod = 0;
        draw_air = 0;
        draw_smoke = 0;
        smoke_divisor = 1;
      } else if (current_variation == 0) {
        grid_calcs = 2;
        velocity_calcs = 2;
        heat_acceleration = 1;
        draw_fuel = 1;
        draw_flame = 0;
        buffer_mod = 0;
        draw_air = 0;
        draw_smoke = 0;
        smoke_divisor = 1;
      }
    }

    void setup() {
      // for (int i = 0; i < FIRE_GRID_HEIGHT; i++) {
      //   yorder[i] = i;
      // }
      // for (int i = 0; i < FIRE_GRID_WIDTH; i++) {
      //   xorder[i] = i;
      // }
      order = reinterpret_cast<uint16_t*>(height_map[0]);
      for (int i = 0; i < FIRE_GRID_WIDTH*FIRE_GRID_HEIGHT; i++) {
        order[i] = i;
      }
      
      for (int i = 0; i < NUM_GAS; i++) {
        gas[i].y = -1;
      }

      //initialize our grid
      for (int y = 0; y < FIRE_GRID_HEIGHT; y++) {
        for (int x = 0; x < FIRE_GRID_WIDTH; x++) {
        //grid_temp[x] = (pixel *) malloc (FIRE_GRID_HEIGHT * sizeof(pixel));
          grid[y][x].smoke=0;
          grid[y][x].fuel=0;
          grid[y][x].air=10000; //10000 = base density
          grid[y][x].vx=0; //10000 = max velocity? (all particles move out)
          grid[y][x].vy=0;
        }
      }


      //initialize the first effect
      next_effect();

    }

    void reset() {

    }
    private:
    


    int fuel_cycle = 0;
    int fuel_cycle_speed = 5;
    int fuel_time = millis();
    unsigned long frame_time = millis();

    //MAIN LOOP
    public:
    void loop() {
      //60 fps max
      if (millis() > 1000 && millis() - 16 > frame_time) {
        frame_time = millis();
        // std::cout << "Smoke: " << grid[45][28].smoke 
        //   << "Air: " << grid[45][28].air
        //   << "Fuel: " << grid[45][28].fuel
        //   << "vx: " << grid[45][28].vx
        //   << "vy: " << grid[45][28].vy
        //   << "\n";

        // debug
        // static int max_air=0;
        // static int max_fuel=0;
        // static int max_smoke=0;
        // static int max_vx=0;
        // static int max_vy=0;
        // static int min_air=0;
        // static int min_fuel=0;
        // static int min_smoke=0;
        // static int min_vx=0;
        // static int min_vy=0;
        // static uint8_t cnt = 0;
        // for (int y = 0; y < FIRE_GRID_WIDTH; y++) {
        //   for (int x = 0; x < FIRE_GRID_WIDTH; x++) {
            
        //     if (grid[y][x].air > max_air) {
        //       max_air = grid[y][x].air;
        //     }
        //     if (grid[y][x].fuel > max_fuel) {
        //       max_fuel = grid[y][x].fuel;
        //     }
        //     if (grid[y][x].smoke > max_smoke) {
        //       max_smoke = grid[y][x].smoke;
        //     }
        //     if (grid[y][x].vx > max_vx) {
        //       max_vx = grid[y][x].vx;
        //     }
        //     if (grid[y][x].vy > max_vy) {
        //       max_vy = grid[y][x].vy;
        //     }
        //     if (grid[y][x].air < min_air) {
        //       min_air = grid[y][x].air;
        //     }
        //     if (grid[y][x].fuel < min_fuel) {
        //       min_fuel = grid[y][x].fuel;
        //     }
        //     if (grid[y][x].smoke < min_smoke) {
        //       min_smoke = grid[y][x].smoke;
        //     }
        //     if (grid[y][x].vx < min_vx) {
        //       min_vx = grid[y][x].vx;
        //     }
        //     if (grid[y][x].vy < min_vy) {
        //       min_vy = grid[y][x].vy;
        //     }
        //   }
        // }
        // cnt++;
        // if (cnt == 0) {
        //   std::cout << _max(max_air,abs(min_air)) << " " << _max(max_fuel,abs(min_fuel)) << " " << _max(max_smoke,abs(min_smoke)) << " " << _max(max_vx,abs(min_vx)) << " " << _max(max_vy,abs(min_vy)) << "\n";
        // }
        //std::cout << max_air << " " << max_fuel << " " << max_smoke << " " << max_vx << " " << max_vy << "\n";
        //std::cout << min_air << " " << min_fuel << " " << min_smoke << " " << min_vx << " " << min_vy << "\n\n";

        //display and blackout

        LED_show();
        //LED_black();
        for (int i = 0; i < NUM_LEDS; i++) {
          leds[i].r >>= 2;
          leds[i].g >>= 2;
          leds[i].b >>= 2;
        }

        static unsigned long gas_time = millis();
        static uint8_t gas_pos = 0;
        static int gas_delay = 50;


        if (current_variation == 1) {
          //add roman candle fireball
          if(millis() - gas_delay > gas_time) {
            gas_time = millis();
            gas_delay = random(400);
            gas[gas_pos].vx = random(-70,71);
            gas[gas_pos].vy = random(8000, 12000);
            gas[gas_pos].x = 40000 - gas[gas_pos].vx;
            gas[gas_pos].y = 0;
            gas[gas_pos].fuel = 10;
            gas[gas_pos].air = 15;
            gas_pos++;
            if(gas_pos > NUM_GAS - 1) {
              gas_pos = 0;
            }
          }
        
        
          //update roman candle fireballs
          for (int i = 0; i < NUM_GAS; i++) {
            int j = 0;
            while (j < 5) {
              if (gas[i].y != -1) {
                gas[i].y += gas[i].vy;
                gas[i].vy *= .99f;
                gas[i].x += gas[i].vx;
                //gas[i].vx *= .99f;
                grid[gas[i].y/10000][gas[i].x/10000].vy = gas[i].vy;
                grid[gas[i].y/10000][gas[i].x/10000].fuel = _min(grid[gas[i].y/10000][gas[i].x/10000].fuel + gas[i].fuel*_max(_min(gas[i].vy,1600)-300,0), FIRE_GRID_MAX);
                grid[gas[i].y/10000][gas[i].x/10000].air = _min(grid[gas[i].y/10000][gas[i].x/10000].air + gas[i].air*_max(_min(gas[i].vy,1600)-150,0), FIRE_GRID_MAX);
              }
              if (gas[i].y > FIRE_GRID_HEIGHT*10000L || gas[i].vy == 0) {
                gas[i].y = -1;
              }
              j++;
            }
          }
        }

        
        //what this?
        if (1 == 2 && millis() > fuel_time) {
          int32_t fuel_amount = ease8Out(fuel_cycle);
          if (fuel_cycle > 255) {
            fuel_amount = 512-fuel_cycle;
          }

          int32_t fuel_range = (fuel_amount*7)/15;
          
          fuel_amount /= 16;
          fuel_amount += 1;
          
          //add fuel
          
          
          static uint8_t stp = 0;
          stp--;
          int ymin = 0;
          int ymax = fuel_range;
          
          if (fuel_cycle > 255) {
            fuel_cycle_speed = 12;
            ymin = 120 - fuel_range;
            ymax = 120;
          }

          
          
//          for (int y = ymin; y < ymax; y++) {
//            //grid[4][y].vx = -6000;
//            int rx = random(3,5);
//            int r = random(-50,50);
//            int rf = random(0, (y)*fuel_amount*3);
//            int rf2 = random(0, (y)*fuel_amount*3);
//            grid[rx][y].vx = r * abs(r);
//            grid[rx][y].vy = _min(grid[rx][y].vy + sq(120-y), FIRE_GRID_MAX);
//            grid[rx][y].fuel = _min(grid[rx][y].fuel + rf, FIRE_GRID_MAX);
//            grid[rx][y].air = _min(grid[rx][y].air + rf2, FIRE_GRID_MAX);
//          }
          
    
          fuel_cycle+=fuel_cycle_speed;
          if (fuel_cycle > 511) {
            fuel_cycle = 0;
            //fuel_cycle_speed = random(1,12);
            fuel_cycle_speed = random(2,12);
            fuel_time = millis()+random(3000);
          }
        }

        

     //uint32_t debug_time2 = micros();
        //CALCULATE GRID
        //burn fuel, move particles
        static uint8_t loop_cnt = 0;
        loop_cnt++;
        int grid_cnt = 0;
        //running the calculation multiple times can raise the maximum velocity of effects, but is less smooth/more jumpy
        while ( grid_cnt < grid_calcs ) {

          process_grid_effects(grid_cnt);
          grid_cnt++;
          //CALCULATE VELOCITIES
          int velocity_count = 0;
          int density_cnt = 0;
          while (velocity_count < 1) {
            velocity_count++;
            density_cnt++;

            
            //push particles based on velocity
            shuffle_order();

            for (int i = 0; i < FIRE_GRID_WIDTH*FIRE_GRID_HEIGHT; i++) {

              uint16_t y = order[i] / FIRE_GRID_WIDTH;
              uint16_t x = order[i] % FIRE_GRID_WIDTH;
              //uint16_t y = order[i] >> 4;
              //uint16_t x = order[i] & 0b0000000000001111;

            // for (int y = 0; y < FIRE_GRID_HEIGHT; y++) {
            //   for (int x = 0; x < FIRE_GRID_WIDTH; x++) {
                calculate_density(x,y,density_cnt);
                calculate_velocity(x,y);
              
            }
                
                
              
            
            

          }
          //END CALCULATE VELOCITIES
          

          }  
        //END CALCULATE GRID
        
     //debug_micros1 += micros() - debug_time2;
      }
    }//LOOP

  // void shuffle_yorder() {
  //   for (int i = 0; i < FIRE_GRID_HEIGHT; i++) {
  //     uint8_t rand = random(FIRE_GRID_HEIGHT);
  //     uint8_t temp = yorder[i];
  //     yorder[i] = yorder[rand];
  //     yorder[rand] = temp;
  //   }
  // }

  // void shuffle_xorder() {
  //   for (int j = 0; j < FIRE_GRID_WIDTH; j++) {
  //     uint8_t rand = random(FIRE_GRID_WIDTH);
  //     uint8_t temp = xorder[j];
  //     xorder[j] = xorder[rand];
  //     xorder[rand] = temp;
  //   }
  // }
  void shuffle_order() {
    for (int j = 0; j < FIRE_GRID_WIDTH*FIRE_GRID_HEIGHT; j++) {
      uint16_t rand = random(FIRE_GRID_WIDTH*FIRE_GRID_HEIGHT);
      uint16_t temp = order[j];
      order[j] = order[rand];
      order[rand] = temp;
    }
  }







    void process_grid_effects(const int& grid_cnt) {

      //burn fuel
      for (int y = 0; y < FIRE_GRID_HEIGHT; y++) {
        for (int x = 0; x < FIRE_GRID_WIDTH; x++) {
          pixel * cell = &grid[y][x];
          int32_t vx = cell->vx;
          int32_t vy = cell->vy;
          int32_t air = cell->air;
          int32_t fuel = cell->fuel;
          int32_t smoke = cell->smoke;
          //add drag
          //smoke to air

          if (smoke > 19) {
            int32_t smoke_dissipate = smoke/20;
            smoke -= smoke_dissipate;
            air += smoke_dissipate;
          }

          //burn fuel
          if (fuel > 0 && air > 0) {
            int32_t burned_fuel = _min(fuel/4, air);
            fuel -= burned_fuel*4;
            air -= burned_fuel;
            smoke += burned_fuel*5;
            if (heat_acceleration == 1) {
              vy = _min(vy + (fuel*fuel)/10, 30000);
            }
            //draw flame

            if(draw_flame == 1 && burned_fuel > 9) {
              drawXY(leds, x, y, _max(_min((burned_fuel*2)/6, 255), 0)/10, 255 - _max(_min(burned_fuel/2-255, 255), 32)/4, _max(_min(burned_fuel, 255)/2, 0));
              //drawXY(leds, x*2+1, y*2, _max(_min((burned_fuel*2)/6, 255), 0)/10, 255 - _max(_min(burned_fuel/2-255, 255), 32)/4, _max(_min(burned_fuel/2, 255), 0));
              //drawXY(leds, x*2, y*2+1, _max(_min((burned_fuel*2)/6, 255), 0)/10, 255 - _max(_min(burned_fuel/2-255, 255), 32)/4, _max(_min(burned_fuel/2, 255), 0));
              //drawXY(leds, x*2+1, y*2+1, _max(_min((burned_fuel*2)/6, 255), 0)/10, 255 - _max(_min(burned_fuel/2-255, 255), 32)/4, _max(_min(burned_fuel/2, 255), 0));
            } else {
              //cell->vy *= .9f;
            }
            
          }




            if(draw_fuel == 1 && fuel != 0) {
              uint16_t fuel2 = 0;
              int low_x = _max(x-2, 0);
              int high_x = _min(x+2, FIRE_GRID_WIDTH);
              int low_y = _max(y-2, 0);
              int high_y = _min(y+2, FIRE_GRID_HEIGHT);
              int cnt = 1;
              for (int y2 = low_y; y2 < high_y; y2++) {
                for (int x2 = low_x; x2 < high_x; x2++) {
                  if (grid[y2][x2].fuel > 0) {
                    int dist = 1 + abs(x - x2) + abs(y - y2);
                    fuel2+=grid[y2][x2].fuel/dist;
                    cnt++;
                  }
                }
              }
              
              fuel2 = ((_max(fuel2, 1000) >> 1) - 500) >> 4;

              if (fuel2 > 0) {
                uint8_t h = _max(_min(fuel2, 200), 0) >> 4;
                uint8_t s = 255 - (_max(_min(fuel2-255, 255), 32) >> 2);
                uint8_t v = _max(_min(fuel2, 255), 0);

                drawXY(leds, x, y, h, s, v); 
              }
                
              //drawXY(leds, x*2+1, y*2, _max(_min((cell->fuel-500)/15, 255), 0)/20, 255 - _max(_min((cell->fuel-500)/15-255, 255), 32)/4, _max(_min((cell->fuel-500)/15, 255), 0)); 
              //drawXY(leds, x*2, y*2+1, _max(_min((cell->fuel-500)/15, 255), 0)/20, 255 - _max(_min((cell->fuel-500)/15-255, 255), 32)/4, _max(_min((cell->fuel-500)/15, 255), 0)); 
              //drawXY(leds, x*2+1, y*2+1, _max(_min((cell->fuel-500)/15, 255), 0)/20, 255 - _max(_min((cell->fuel-500)/15-255, 255), 32)/4, _max(_min((cell->fuel-500)/15, 255), 0)); 
            }




          //draw air and smoke for debugging
          if (draw_air) {
            drawXY(leds, x, y, 96, 255, _min(_max((air)/100,0),255));
          }
          if (draw_smoke) {
            drawXY(leds, x, y, 160, 255, _min(_max((smoke)/100,0),255));
          }
          //constrain to avoid huge stuff/overflow
           cell->fuel = _max(_min(fuel, FIRE_GRID_MAX), FIRE_GRID_MIN);
           cell->air = _max(_min(air, FIRE_GRID_MAX), FIRE_GRID_MIN);
           cell->smoke = _max(_min(smoke, FIRE_GRID_MAX), FIRE_GRID_MIN);
           cell->vx = _max(_min(vx, FIRE_GRID_VELOCITY_MAX), FIRE_GRID_VELOCITY_MIN);
           cell->vy = _max(_min(vy, FIRE_GRID_VELOCITY_MAX), FIRE_GRID_VELOCITY_MIN);
        }
      }

      //////////////////////////////////////
      //flamethrower effect
      if (current_variation == 2 /*&& grid_cnt == grid_cnt*/) {
        

        static FLAMETHROWER_CYCLE * current_flame = get_current_flame();
        if (button2_down && !flame_on) {
          flame_on = true;
          current_flame->fuel_on = true;
          current_flame->active = true;
          for (int y = 0; y < FIRE_GRID_HEIGHT; y++) {
            for (int x = 0; x < FIRE_GRID_WIDTH; x++) {
              grid[y][x].smoke += grid[y][x].air;
              grid[y][x].air = 0;
              if (y > 30) {
                grid[y][x].vy = 30000;
              }
            }
          }
        }

        if (!button2_down && flame_on) {
          flame_on = false;
          current_flame->fuel_on = false;
          current_flame = get_current_flame();
        }
        
        // static uint32_t flamethrower_time = millis();
        // if (millis() - 100 < flamethrower_time) {
        //   current_flame->fuel_on = true;
        //   current_flame->active = true;
        // } else if (millis() - 350 > flamethrower_time) {
        //   flamethrower_time = millis();
        // } else {
        //   current_flame->fuel_on = false;
        //   current_flame = get_current_flame();
        // }


        #define FLAMETHROWER_SPEED 24
        for (int i = 0; i < NUM_FLAMETHROWER_CYCLES; i++) {
          FLAMETHROWER_CYCLE * flame = &flamethrower_cycles[i];
          if (flame->active) {

            //move the front of the flame
            if(flame->y_to < 255-FLAMETHROWER_SPEED) {
              flame->y_to+=FLAMETHROWER_SPEED;
            }

            //move the rear of flame
            if ( (!flame->fuel_on) && flame->y_from < 255-FLAMETHROWER_SPEED) {
              flame->y_from += FLAMETHROWER_SPEED;
            }

            //flame is done?
            if (flame->y_to >= 255-FLAMETHROWER_SPEED && flame->y_from >= 255-FLAMETHROWER_SPEED) {
              flame->active = false;
              flame->y_from = 0;
              flame->y_to = 0;
            }

            uint8_t y_start = (ease8Out(flame->y_from)*100)/255;
            uint8_t y_end = (ease8Out(flame->y_to)*100)/255;


            //add fuel to the grid
            for (int i = y_start; i < y_end; i++) {
              int width = i/25;
              int x = FIRE_GRID_WIDTH/2-width;
              x += random(width*2+2);
              grid[i][x].vy += random(10000);
              grid[i][x].vx += random(8000)-4000;
              int r = random(1000)+500;
              if (flame->y_to < 240) {
                r+=1000;
              }
              grid[i][x].fuel += (r*i)/10+500;
              grid[i][x].air += 0;

              //NEAT FIRE
              // int y = 20;
              // grid[y][i].vy += random(10000);
              // grid[y][i].vx += random(8000)-4000;
              // int r = random(1000);
              // r = (r*r)/1000;
              // r = (r*r)/1000;
              // grid[y][i].fuel += r;
              
              
            }
          }
        }
      }

      //////////////////////////////////////
      //candle effect
      if (current_variation == 0 && grid_cnt == 1) {
        //candle flame
        //grid calcs = 1
        for (int i =45; i < 50; i++) {
          int x = (FIRE_GRID_WIDTH-1)/2+random(0,2);
          //grid[x][i-5].vy += (50-i)*random(200);
          grid[i-14][x].vx += (i)*random(50)-(i)*25;
          int r = random((i+1)*200, (i+1)*400);
          r = (r*r)/((i+1)*400);
          grid[i-14][x].fuel += r*2;
          //grid[x][i-5].air += random(0,i*75);
        }
      }
    }




    

    void calculate_velocity(int x, int y) {

      pixel* source_cell=&grid[y][x];

      //transfer particles between cells based on velocity (10000 max velocity, all particles move)
      int32_t total = source_cell->fuel + source_cell->air + source_cell->smoke;
      int32_t total_moved_fuel = 0;
      int32_t total_moved_air = 0;
      int32_t total_moved_smoke = 0;
      int32_t v_total = abs(source_cell->vx)+abs(source_cell->vy);
      
      if (v_total != 0) {
        int32_t v_total_constrain = _min(v_total, 30000);
        int32_t x_portion = (abs(source_cell->vx)*v_total_constrain) / v_total;
        int32_t y_portion = (abs(source_cell->vy)*v_total_constrain) / v_total;
        
        int div = 25000;
        if (source_cell->vx > 0) {
          int32_t moved_fuel = (source_cell->fuel * x_portion) / div;
          int32_t moved_air = (source_cell->air * x_portion) / div;
          int32_t moved_smoke = (source_cell->smoke * x_portion) / div;
          int32_t moved_total = moved_fuel + moved_air + moved_smoke;
          
          if (moved_total > 0 && x < FIRE_GRID_WIDTH - 1) {
            pixel* right_cell=&grid[y][x+1];
            velocity_transfer(*source_cell, *right_cell, moved_fuel, moved_air, moved_smoke, moved_total);
          }
          
          total_moved_smoke += moved_smoke;
          total_moved_fuel += moved_fuel;
          total_moved_air += moved_air;  
        }

        if (source_cell->vx < 0) {
          int32_t moved_fuel = (source_cell->fuel * x_portion) / div;
          int32_t moved_air = (source_cell->air * x_portion) / div;
          int32_t moved_smoke = (source_cell->smoke * x_portion) / div;
          int32_t moved_total = moved_fuel + moved_air + moved_smoke;
          if (moved_total > 0 && x > 0) {
            pixel* left_cell=&grid[y][x-1];
            velocity_transfer(*source_cell, *left_cell, moved_fuel, moved_air, moved_smoke, moved_total);
          }
          total_moved_smoke += moved_smoke;
          total_moved_fuel += moved_fuel;
          total_moved_air += moved_air;
        }
        
        if (source_cell->vy > 0) {
          int32_t moved_fuel = (source_cell->fuel * y_portion) / div;
          int32_t moved_air = (source_cell->air * y_portion) / div;
          int32_t moved_smoke = (source_cell->smoke * y_portion) / div;
          int32_t moved_total = moved_fuel + moved_air + moved_smoke;
          if (moved_total > 0 && y < FIRE_GRID_HEIGHT - 1) {
            pixel* top_cell=&grid[y+1][x];
            velocity_transfer(*source_cell, *top_cell, moved_fuel, moved_air, moved_smoke, moved_total);
          }
          total_moved_smoke += moved_smoke;
          total_moved_fuel += moved_fuel;
          total_moved_air += moved_air;
        }

        if (source_cell->vy < 0) {
          int32_t moved_fuel = (source_cell->fuel * y_portion) / div;
          int32_t moved_air = (source_cell->air * y_portion) / div;
          int32_t moved_smoke = (source_cell->smoke * y_portion) / div;
          int32_t moved_total = moved_fuel + moved_air + moved_smoke;
          if (moved_total > 0 && y > 0) {
            pixel* bottom_cell=&grid[y-1][x];
            velocity_transfer(*source_cell, *bottom_cell, moved_fuel, moved_air, moved_smoke, moved_total);
          }
          total_moved_smoke += moved_smoke;
          total_moved_fuel += moved_fuel;
          total_moved_air += moved_air;
        }

/*
        if (v_total_constrain > 25000 && source_cell->fuel > 0) {
          uint32_t fuel2 = (source_cell->fuel - 500) / 15;
          uint8_t h = _max(_min(fuel2, 200), 0) >> 4;
          uint8_t s = 255 - (_max(_min(fuel2-255, 255), 32) >> 2);
          uint8_t v = _max(_min(fuel2, 255), 0);

          drawXY(leds, x, y, h, s, v);  
        }

 */


        if ( v_total_constrain > 25000 && (source_cell->fuel-500 > 14) ) {
          uint32_t fuel2 = (source_cell->fuel - 500) / 15;
          int h = _max(_min(fuel2, 255), 0)/20;
          int s = 255 - _max(_min(fuel2-255, 255), 32)/4;
          int v = _max(_min(fuel2, 255), 0);
          drawXY(leds, x, y, h, s, v); 
        }

        source_cell->smoke -= total_moved_smoke; //looked kind of neat without this line
        source_cell->fuel -= total_moved_fuel;
        source_cell->air -= total_moved_air;
        
      }
    }

    void calculate_density(int x, int y, int density_cnt) {

      pixel* source_cell=&grid[y][x];

      
      //total number of particles in cell
      int32_t total_particles = source_cell->fuel + source_cell->air + source_cell->smoke;
      //particle imbalance (number of particles above/below our base threshold/pressure)
      int32_t pressure = (total_particles - 10000)/density_cnt;


      if (pressure > 0 && total_particles > 0) {
        //float ratio=(pressure+0.f)/total_particles;
        int32_t fuel_pressure = ( pressure * source_cell->fuel )/total_particles;
        int32_t air_pressure = ( pressure * source_cell->air )/total_particles;
        int32_t smoke_pressure = ( pressure * source_cell->smoke )/total_particles;

        source_cell->fuel -= fuel_pressure;
        source_cell->air -= air_pressure;
        source_cell->smoke -= smoke_pressure;

        //number of particles to push to neighboring cells
        int32_t moved_fuel = fuel_pressure / 6;
        int32_t moved_air = air_pressure / 6;
        int32_t moved_smoke = smoke_pressure / 6;
        int32_t moved_total = moved_fuel + moved_air + moved_smoke;
        

          //right cell
          if (x < FIRE_GRID_WIDTH - 1) {
            pixel* right_cell=&grid[y][x+1];
            velocity_transfer(*source_cell, *right_cell, moved_fuel, moved_air, moved_smoke, moved_total);
          }

          //left cell
          if (x > 0) {
            pixel* left_cell=&grid[y][x-1];
            velocity_transfer(*source_cell, *left_cell, moved_fuel, moved_air, moved_smoke, moved_total);
          }

          //top cell
          if (y < FIRE_GRID_HEIGHT - 1) {
            pixel* top_cell=&grid[y+1][x];
            velocity_transfer(*source_cell, *top_cell, moved_fuel, moved_air, moved_smoke, moved_total);
          }

          //bottom cell
          if (y > 0) {
            pixel* bottom_cell=&grid[y-1][x];
            velocity_transfer(*source_cell, *bottom_cell, moved_fuel, moved_air, moved_smoke, moved_total);
          }
        
      }

      if ( pressure < 0) {
        //number of particles to grab from adjacent cells
        pressure /= -8;

        uint8_t off_grid_cnt = 4;
        //right cell
        if (x < FIRE_GRID_WIDTH - 1) {
          pixel* right_cell=&grid[y][x+1];
          pull_particles(*source_cell, *right_cell, pressure, -1, 0);
          off_grid_cnt--;
        }
        
        //left cell
        if (x > 0) {
          pixel* left_cell=&grid[y][x-1];
          pull_particles(*source_cell, *left_cell, pressure, 1, 0);
          off_grid_cnt--;
        } 

        //top cell
        if (y < FIRE_GRID_HEIGHT - 1) {
          pixel* top_cell=&grid[y+1][x];
          pull_particles(*source_cell, *top_cell, pressure, 0, -1);
          off_grid_cnt--;
        } 

        //bottom cell
        if (y > 0) {
          pixel* bottom_cell=&grid[y-1][x];
          pull_particles(*source_cell, *bottom_cell, pressure, 0, 1);
          off_grid_cnt--;
        } 

        if (off_grid_cnt > 0) {
          pull_air(*source_cell, pressure*off_grid_cnt);
        }
              
      }
      
    }


};



LIGHT_SKETCHES::REGISTER<FIRE2> fire2("fire2");