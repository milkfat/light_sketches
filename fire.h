//FIRE

//this class uses malloc for the grid, because it gets HUGE with large displays
//128x32 uses ~213KB
//should probaby optimize that somehow
#define FIRE_GRID_WIDTH (MATRIX_WIDTH/2)
#define FIRE_GRID_HEIGHT (MATRIX_HEIGHT)

#define FIRE_GRID_MIN -60000
#define FIRE_GRID_MAX 60000

class FIRE2: public LIGHT_SKETCH {
    public:
      FIRE2 () {setup();}
      ~FIRE2 () {
      for (int x = 0; x < FIRE_GRID_WIDTH; x++) {
        //delete grid_temp[x];
      }
    }
    private:
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



    typedef cint18 pixel_data;
    class pixel {
      public:
        pixel_data smoke = 0;
        pixel_data fuel = 0;
        pixel_data air = 0; //10000 = base density
        pixel_data vx = 0; //10000 = max velocity? (all particles move out)
        pixel_data vy = 0;

      public:
        int32_t get_smoke() {
          return smoke;
        }
        int32_t get_fuel() {
          return fuel;
        }
        int32_t get_air() {
          return air;
        }
        int32_t get_vx() {
          return vx;
        }
        int32_t get_vy() {
          return vy;
        }

        void set_smoke(int32_t val) {
          smoke = val;
        }
        void set_fuel(int32_t val) {
          fuel = val;
        }
        void set_air(int32_t val) {
          air = val;
        }
        void set_vx(int32_t val) {
          vx = val;
        }
        void set_vy(int32_t val) {
          vy = val;
        }


    };

    struct pixel_temp {
        int32_t smoke = 0;
        int32_t fuel = 0;
        int32_t air = 0;
        int32_t vx = 0;
        int32_t vy = 0;
        int32_t add_smoke = 0;
        int32_t add_fuel = 0;
        int32_t add_air = 0;
        int32_t add_vx = 0;
        int32_t add_vy = 0;
        int32_t remove_smoke = 0;
        int32_t remove_fuel = 0;
        int32_t remove_air = 0;

        void clear() {
          smoke = 0;
          fuel = 0;
          air = 0;
          vx = 0;
          vy = 0;
          add_smoke = 0;
          add_fuel = 0;
          add_air = 0;
          add_vx = 0;
          add_vy = 0;
          remove_smoke = 0;
          remove_fuel = 0;
          remove_air = 0;
        }
    };

    //pixel * grid[FIRE_GRID_WIDTH];
    pixel grid[FIRE_GRID_WIDTH][FIRE_GRID_HEIGHT];
    #define GRID_BUFFER_SIZE 3
    pixel_temp grid_temp[FIRE_GRID_WIDTH][GRID_BUFFER_SIZE]; //buffer for calculations


    uint8_t order[4] = {0,1,2,3};

    void shuffle_order() {
      for (int i = 0; i < 4; i++) {
        uint8_t r = random(4);
        uint8_t temp = order[r];
        order[r] = order[i];
        order[i] = temp;
      }
    }

    void velocity_transfer(pixel_temp &source_cell, pixel_temp &target_cell, int32_t &moved_fuel, int32_t &moved_air, int32_t &moved_smoke, int32_t &moved_total ) { 
      int32_t existing_total = target_cell.add_fuel + target_cell.add_air + target_cell.add_smoke;
      if (moved_total + existing_total == 0) {
        existing_total++; //avoid divide by zero
      }
      int32_t total = moved_total + existing_total;
      target_cell.add_smoke += moved_smoke;
      target_cell.add_fuel += moved_fuel;
      target_cell.add_air += moved_air;
      target_cell.add_vx = ((source_cell.vx * moved_total) + (target_cell.add_vx * existing_total))/total;
      target_cell.add_vy = ((source_cell.vy * moved_total) + (target_cell.add_vy * existing_total))/total;
      
    }

    void pull_particles(pixel_temp &source_cell, pixel_temp &target_cell, int32_t &pressure, int x_dir, int y_dir) {

      //find the starting total of the target cell
      int32_t target_cell_total = target_cell.fuel + target_cell.air + target_cell.smoke;

      //find the cumulative total of the target cell
      int32_t target_cell_total2 = target_cell_total + target_cell.remove_fuel + target_cell.remove_air + target_cell.remove_smoke;
      if ( target_cell_total2 > pressure && target_cell_total != 0 ) {
        int32_t smoke = ( pressure * target_cell.smoke ) / target_cell_total2;
        int32_t fuel = ( pressure * target_cell.fuel ) / target_cell_total2;
        int32_t air = ( pressure * target_cell.air ) / target_cell_total2;
        
        //record the removed particles
        target_cell.remove_smoke += smoke;
        target_cell.remove_fuel += fuel;
        target_cell.remove_air += air;
        
        //find the total number of moved particles
        int32_t moved_total = fuel+air+smoke;

        //add the moved particles to the source cell, calculate velocities
        int32_t existing_total = source_cell.add_fuel + source_cell.add_air + source_cell.add_smoke;
        int32_t total = moved_total + existing_total;
        if (total != 0) {
          source_cell.add_fuel += fuel;
          source_cell.add_air += air;
          source_cell.add_smoke += smoke;
          source_cell.add_vx = ((source_cell.add_vx * existing_total) + (target_cell.vx * moved_total))/total;
          source_cell.add_vy = ((source_cell.add_vy * existing_total) + (target_cell.vy * moved_total))/total;

          //add a bit of velocity to the target cell (sucking effect?)
          target_cell.vx += moved_total * x_dir;
          target_cell.vy += moved_total * y_dir;
        }
        
      }
    }

    void pull_air(pixel_temp &source_cell, int32_t pressure) {
      //pull air from off the grid
      int32_t existing_total = source_cell.add_fuel + source_cell.add_air + source_cell.add_smoke;
      int32_t total = pressure + existing_total;
      if (total != 0) {
        source_cell.add_vx = (source_cell.add_vx * existing_total)/total;
        source_cell.add_vy = (source_cell.add_vy * existing_total)/total;
      }
      source_cell.add_air += pressure;
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
        grid_calcs = 3;
        velocity_calcs = 2;
        heat_acceleration = 1;
        draw_fuel = 1;
        draw_flame = 0;
        buffer_mod = 0;
        draw_air = 0;
        draw_smoke = 0;
        smoke_divisor = 1;
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

      
      for (int i = 0; i < NUM_GAS; i++) {
        gas[i].y = -1;
      }

      //initialize our grid
      for (int x = 0; x < FIRE_GRID_WIDTH; x++) {
        //grid_temp[x] = (pixel *) malloc (FIRE_GRID_HEIGHT * sizeof(pixel));
        for (int y = 0; y < FIRE_GRID_HEIGHT; y++) {
          grid[x][y].set_smoke(0);
          grid[x][y].set_fuel(0);
          grid[x][y].set_air(10000); //10000 = base density
          grid[x][y].set_vx(0); //10000 = max velocity? (all particles move out)
          grid[x][y].set_vy(0);
        }
      }

      for (int x = 0; x < FIRE_GRID_WIDTH; x++) {
        //grid_temp[x] = (pixel *) malloc (FIRE_GRID_HEIGHT * sizeof(pixel));
        for (int y = 0; y < 3; y++) {
          clear_buffer(x, y);
        }
      }

      //initialize the first effect
      next_effect();

    }

    void reset() {

    }
    private:
    
    void fill_buffer(int x, int y) {
      pixel * cell = &grid[x][y];
      pixel_temp * buffer_cell = &grid_temp[x][y%GRID_BUFFER_SIZE];
      if (y >= 0 && y < FIRE_GRID_HEIGHT) {
        buffer_cell->air = cell->get_air();
        buffer_cell->fuel = cell->get_fuel();
        buffer_cell->smoke = cell->get_smoke();
        buffer_cell->vx = cell->get_vx();
        buffer_cell->vy = cell->get_vy();
      }
    }


    void copy_buffer(int x, int y) {
      pixel * cell = &grid[x][y];
      pixel_temp * buffer_cell = &grid_temp[x][y%GRID_BUFFER_SIZE];
      if (y >= 0 && y < FIRE_GRID_HEIGHT) {
        cell->set_air(buffer_cell->air);
        cell->set_fuel(buffer_cell->fuel);
        cell->set_smoke(buffer_cell->smoke);
        cell->set_vx(buffer_cell->vx);
        cell->set_vy(buffer_cell->vy);
      }
    }

    void clear_buffer(int x, int y) {
      grid_temp[x][y%GRID_BUFFER_SIZE].clear();
    }


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
            
        //     if (grid[x][y].air > max_air) {
        //       max_air = grid[x][y].air;
        //     }
        //     if (grid[x][y].fuel > max_fuel) {
        //       max_fuel = grid[x][y].fuel;
        //     }
        //     if (grid[x][y].smoke > max_smoke) {
        //       max_smoke = grid[x][y].smoke;
        //     }
        //     if (grid[x][y].vx > max_vx) {
        //       max_vx = grid[x][y].vx;
        //     }
        //     if (grid[x][y].vy > max_vy) {
        //       max_vy = grid[x][y].vy;
        //     }
        //     if (grid[x][y].air < min_air) {
        //       min_air = grid[x][y].air;
        //     }
        //     if (grid[x][y].fuel < min_fuel) {
        //       min_fuel = grid[x][y].fuel;
        //     }
        //     if (grid[x][y].smoke < min_smoke) {
        //       min_smoke = grid[x][y].smoke;
        //     }
        //     if (grid[x][y].vx < min_vx) {
        //       min_vx = grid[x][y].vx;
        //     }
        //     if (grid[x][y].vy < min_vy) {
        //       min_vy = grid[x][y].vy;
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
        LED_black();

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
                grid[gas[i].x/10000][gas[i].y/10000].vy = gas[i].vy;
                grid[gas[i].x/10000][gas[i].y/10000].fuel = _min(grid[gas[i].x/10000][gas[i].y/10000].fuel + gas[i].fuel*_max(_min(gas[i].vy,1600)-300,0), FIRE_GRID_MAX);
                grid[gas[i].x/10000][gas[i].y/10000].air = _min(grid[gas[i].x/10000][gas[i].y/10000].air + gas[i].air*_max(_min(gas[i].vy,1600)-150,0), FIRE_GRID_MAX);
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

        

        //CALCULATE GRID
        //burn fuel, move particles
        static uint8_t loop_cnt = 0;
        loop_cnt++;

        int grid_cnt = 0;
        bool r0 = random(2);
        bool r1 = r0;
        //running the calculation multiple times can raise the maximum velocity of effects, but is less smooth/more jumpy
        while ( grid_cnt < grid_calcs ) {
          grid_cnt++;

          

          process_grid_effects(grid_cnt);

          //CALCULATE VELOCITIES
          int velocity_count = 0;
          while (velocity_count < 1) {
            velocity_count++;

            //alternate the direction of calculation between forward and reverse to remove bias
            //if ((grid_cnt+loop_cnt)%2 == 1) {
            if ((grid_cnt%2) == r0) {
              //push particles based on velocity
              for (int y = -1; y < FIRE_GRID_HEIGHT+GRID_BUFFER_SIZE; y++) {
                
                for (int x = 0; x < FIRE_GRID_WIDTH; x++) {  
                  if (y > GRID_BUFFER_SIZE-2) {
                    resolve_velocity(x,y-(GRID_BUFFER_SIZE-1));
                  }

                  if (y < 0) {
                    clear_buffer(x,y+1);
                    fill_buffer(x,y+1);
                  }

                  else if (y < FIRE_GRID_HEIGHT) {
                    copy_buffer(x,y-(GRID_BUFFER_SIZE-1));
                    clear_buffer(x,y+1);
                    fill_buffer(x,y+1);
                    calculate_velocity(x,y);
                  }

                }

              }
            } else {
              //calculate in reverse starting at FIRE_GRID_HEIGHT, FIRE_GRID_WIDTH
              for (int y = FIRE_GRID_HEIGHT; y >= 0-(GRID_BUFFER_SIZE-1); y--) {
                
                for (int x = FIRE_GRID_WIDTH-1; x >= 0; x--) {
                  
                  if (y < (FIRE_GRID_HEIGHT-1) - (GRID_BUFFER_SIZE-2)) {
                    resolve_velocity(x,y+(GRID_BUFFER_SIZE-1));
                  }

                  if (y > FIRE_GRID_HEIGHT-1) {
                    clear_buffer(x,y-1);
                    fill_buffer(x,y-1);
                  }

                  else if (y >= 0) {
                    copy_buffer(x,y+(GRID_BUFFER_SIZE-1));
                    clear_buffer(x,y+(GRID_BUFFER_SIZE-1));
                    fill_buffer(x,y-1);
                    calculate_velocity(x,y);
                  }
                }
              }
            }
          }
          //END CALCULATE VELOCITIES
          

          //CALCULATE DENSITIES
          int density_cnt = 0;
          while(density_cnt < 1) {
            
            density_cnt++;
            
            //move particles based on density/pressure
           
            //alternate the direction of calculation between forward and reverse to remove bias
            //if ((grid_cnt+loop_cnt)%2 == 1) {
              if ((grid_cnt%2) == r1) {
              //std::cout << "forward\n";
              //calculate starting at 0,0
              for (int y = -1; y < FIRE_GRID_HEIGHT+GRID_BUFFER_SIZE; y++) {
                
                for (int x = 0; x < FIRE_GRID_WIDTH; x++) {
                  
                  if (y > GRID_BUFFER_SIZE-2) {
                    resolve_density(x,y-(GRID_BUFFER_SIZE-1));
                  }

                  if (y < 0) {
                    clear_buffer(x,y+1);
                    fill_buffer(x,y+1);
                  }

                  else if (y < FIRE_GRID_HEIGHT) {
                    copy_buffer(x,y-(GRID_BUFFER_SIZE-1-buffer_mod));
                    clear_buffer(x,y-(GRID_BUFFER_SIZE-1-buffer_mod));
                    fill_buffer(x,y+1);
                    calculate_density(x,y,density_cnt);
                  }
                }

              }


            } else {
              //std::cout << "reverse\n";

              //calculate in reverse starting at FIRE_GRID_HEIGHT, FIRE_GRID_WIDTH
              for (int y = FIRE_GRID_HEIGHT; y >= 0-(GRID_BUFFER_SIZE-1); y--) {
                
                for (int x = FIRE_GRID_WIDTH-1; x >= 0; x--) {
                  
                  if (y < (FIRE_GRID_HEIGHT-1) - (GRID_BUFFER_SIZE-2)) {
                    resolve_density(x,y+(GRID_BUFFER_SIZE-1));
                  }

                  if (y > FIRE_GRID_HEIGHT-1) {
                    clear_buffer(x,y-1);
                    fill_buffer(x,y-1);
                  } else

                  if (y >= 0) {
                    copy_buffer(x,y+(GRID_BUFFER_SIZE-1-buffer_mod));
                    clear_buffer(x,y+(GRID_BUFFER_SIZE-1-buffer_mod));
                    fill_buffer(x,y-1);
                    calculate_density(x,y,density_cnt);
                  }
                }
              }


            }
    
          }

        }  
        //END CALCULATE GRID
        
      }
    }//LOOP










    void process_grid_effects(int grid_cnt) {

      //burn fuel
      for (int x = 0; x < FIRE_GRID_WIDTH; x++) {
        for (int y = 0; y < FIRE_GRID_HEIGHT; y++) {
          pixel * cell = &grid[x][y];
          int32_t vx = cell->vx;
          int32_t vy = cell->vy;
          int32_t air = cell->air;
          int32_t fuel = cell->fuel;
          int32_t smoke = cell->smoke;
          //add drag
          if (vx > 5 ){
            vx *= .99f; //caused wierd lines
          }
            if (vy > 5 ){
            vy *= .99f; //caused wierd lines
          }
          //smoke to air
          int32_t smoke_dissipate = smoke/20;
          smoke -= smoke_dissipate;
          air += smoke_dissipate;
          
          //burn fuel
          if (fuel > 0 && air > 0) {
            int32_t burned_fuel = _min(fuel, air/2);
            fuel = fuel - burned_fuel;
            air = air - burned_fuel*2;
            smoke += burned_fuel;
            if (heat_acceleration == 1 && vy < burned_fuel*30) {
              vy = _min(vy + (burned_fuel*burned_fuel)/100, 30000);
            }
            //draw flame
            if(draw_flame == 1 && burned_fuel > 9) {
              drawXY(leds, x, y, _max(_min((burned_fuel*2)/6, 255), 0)/10, 255 - _max(_min(burned_fuel/2-255, 255), 32)/4, _max(_min(burned_fuel/2, 255), 0));
              //drawXY(leds, x*2+1, y*2, _max(_min((burned_fuel*2)/6, 255), 0)/10, 255 - _max(_min(burned_fuel/2-255, 255), 32)/4, _max(_min(burned_fuel/2, 255), 0));
              //drawXY(leds, x*2, y*2+1, _max(_min((burned_fuel*2)/6, 255), 0)/10, 255 - _max(_min(burned_fuel/2-255, 255), 32)/4, _max(_min(burned_fuel/2, 255), 0));
              //drawXY(leds, x*2+1, y*2+1, _max(_min((burned_fuel*2)/6, 255), 0)/10, 255 - _max(_min(burned_fuel/2-255, 255), 32)/4, _max(_min(burned_fuel/2, 255), 0));
            } else {
              //cell->vy *= .9f;
            }
            if(draw_fuel == 1 && fuel > 500) {
              drawXY(leds, x, y, _max(_min((fuel-500)/15, 255), 0)/20, 255 - _max(_min((fuel-500)/15-255, 255), 32)/4, _max(_min((fuel-500)/15, 255), 0)); 
              //drawXY(leds, x*2+1, y*2, _max(_min((cell->fuel-500)/15, 255), 0)/20, 255 - _max(_min((cell->fuel-500)/15-255, 255), 32)/4, _max(_min((cell->fuel-500)/15, 255), 0)); 
              //drawXY(leds, x*2, y*2+1, _max(_min((cell->fuel-500)/15, 255), 0)/20, 255 - _max(_min((cell->fuel-500)/15-255, 255), 32)/4, _max(_min((cell->fuel-500)/15, 255), 0)); 
              //drawXY(leds, x*2+1, y*2+1, _max(_min((cell->fuel-500)/15, 255), 0)/20, 255 - _max(_min((cell->fuel-500)/15-255, 255), 32)/4, _max(_min((cell->fuel-500)/15, 255), 0)); 
            }
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
          cell->vx = _max(_min(vx, FIRE_GRID_MAX), FIRE_GRID_MIN);
          cell->vy = _max(_min(vy, FIRE_GRID_MAX), FIRE_GRID_MIN);
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


        #define FLAMETHROWER_SPEED 6
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
              int x = (FIRE_GRID_WIDTH-1)/2+random(0,2);
              grid[x][i].vy += (100)*random(200);
              grid[x][i].vx += (i)*random(200)-(i)*100;
              
              int r = random((i+1)*00, (i+15)*100);
              r = (r*r)/((i+15)*100);
              if (y_end < 100 && y_end - i < 20) {
                if (flame->fuel_on) {
                  r*=10;
                } else {
                  r*=5;
                }
              }
              grid[x][i].fuel += r*2;
              int r2 = random(0,i*175);
              
              grid[x][i].air += r2;
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
          grid[x][i-14].vx += (i)*random(50)-(i)*25;
          int r = random((i+1)*200, (i+1)*400);
          r = (r*r)/((i+1)*400);
          grid[x][i-14].fuel += r*2;
          //grid[x][i-5].air += random(0,i*75);
        }
      }
    }






    void calculate_velocity(int x, int y2) {
      int ty = y2%GRID_BUFFER_SIZE;
      int y = ty;

      pixel_temp* source_cell=&grid_temp[x][y];

      pixel_temp* right_cell=&grid_temp[x+1][y];
      pixel_temp* left_cell=&grid_temp[x-1][y];
      pixel_temp* top_cell=&grid_temp[x][(y+1)%GRID_BUFFER_SIZE];
      pixel_temp* bottom_cell=&grid_temp[x][(y+2)%GRID_BUFFER_SIZE];

      //transfer particles between cells based on velocity (10000 max velocity, all particles move)
      int32_t total = source_cell->fuel + source_cell->air + source_cell->smoke;
      int32_t total_moved_fuel = 0;
      int32_t total_moved_air = 0;
      int32_t total_moved_smoke = 0;
      int32_t v_total = abs(source_cell->vx)+abs(source_cell->vy);
      
      if (v_total != 0) {
        int32_t v_total_constrain = _min(v_total, 10000);
        int32_t x_portion = (abs(source_cell->vx)*v_total_constrain) / v_total;
        int32_t y_portion = (abs(source_cell->vy)*v_total_constrain) / v_total;
      
      
        if (source_cell->vx > 0) {
          int32_t moved_fuel = (source_cell->fuel * x_portion) / 10000;
          int32_t moved_air = (source_cell->air * x_portion) / 10000;
          int32_t moved_smoke = (source_cell->smoke * x_portion) / 10000;
          int32_t moved_total = moved_fuel + moved_air + moved_smoke;
          if (moved_total > 0 && x < FIRE_GRID_WIDTH - 1) {
            velocity_transfer(*source_cell, *right_cell, moved_fuel, moved_air, moved_smoke, moved_total);
          }
          
          total_moved_smoke += moved_smoke;
          total_moved_fuel += moved_fuel;
          total_moved_air += moved_air;  
        }

        if (source_cell->vx < 0) {
          int32_t moved_fuel = (source_cell->fuel * x_portion) / 10000;
          int32_t moved_air = (source_cell->air * x_portion) / 10000;
          int32_t moved_smoke = (source_cell->smoke * x_portion) / 10000;
          int32_t moved_total = moved_fuel + moved_air + moved_smoke;
          if (moved_total > 0 && x > 0) {
            velocity_transfer(*source_cell, *left_cell, moved_fuel, moved_air, moved_smoke, moved_total);
          }
          total_moved_smoke += moved_smoke;
          total_moved_fuel += moved_fuel;
          total_moved_air += moved_air;
        }
        
        if (source_cell->vy > 0) {
          int32_t moved_fuel = (source_cell->fuel * y_portion) / 10000;
          int32_t moved_air = (source_cell->air * y_portion) / 10000;
          int32_t moved_smoke = (source_cell->smoke * y_portion) / 10000;
          int32_t moved_total = moved_fuel + moved_air + moved_smoke;
          if (moved_total > 0 && y2 < FIRE_GRID_HEIGHT - 1) {
            velocity_transfer(*source_cell, *top_cell, moved_fuel, moved_air, moved_smoke, moved_total);
          }
          total_moved_smoke += moved_smoke;
          total_moved_fuel += moved_fuel;
          total_moved_air += moved_air;
        }

        if (source_cell->vy < 0) {
          int32_t moved_fuel = (source_cell->fuel * y_portion) / 10000;
          int32_t moved_air = (source_cell->air * y_portion) / 10000;
          int32_t moved_smoke = (source_cell->smoke * y_portion) / 10000;
          int32_t moved_total = moved_fuel + moved_air + moved_smoke;
          if (moved_total > 0 && y2 > 0) {
            velocity_transfer(*source_cell, *bottom_cell, moved_fuel, moved_air, moved_smoke, moved_total);
          }
          total_moved_smoke += moved_smoke;
          total_moved_fuel += moved_fuel;
          total_moved_air += moved_air;
        }
      
    
        source_cell->smoke -= total_moved_smoke; //looked kind of neat without this line
        source_cell->fuel -= total_moved_fuel;
        source_cell->air -= total_moved_air;
      }
    }







    void resolve_velocity(int x, int y) {

      pixel_temp * cell = &grid_temp[x][y%GRID_BUFFER_SIZE];

      int32_t total_existing = cell->fuel + cell->air + cell->smoke;
      int32_t total_new = cell->add_fuel + cell->add_air + cell->add_smoke;
      if (total_new + total_existing == 0) {
        total_existing++;
      }
      cell->smoke += cell->add_smoke;
      cell->add_smoke = 0;
      cell->air += cell->add_air;
      cell->add_air = 0;
      cell->fuel += cell->add_fuel;
      cell->add_fuel = 0;
      int32_t new_vx_portion = (cell->add_vx * total_new)/(total_new + total_existing);
      int32_t existing_vx_portion = (cell->vx * total_existing)/(total_new + total_existing);
      int32_t new_vy_portion = (cell->add_vy * total_new)/(total_new + total_existing);
      int32_t existing_vy_portion = (cell->vy * total_existing)/(total_new + total_existing);
      cell->vx = new_vx_portion + existing_vx_portion;
      cell->add_vx = 0;
      cell->vy = new_vy_portion + existing_vy_portion;
      cell->add_vy = 0;
      
    }






    void calculate_density(int x, int y, int density_cnt) {
      int ty = y%GRID_BUFFER_SIZE;

      pixel_temp* source_cell=&grid_temp[x][ty];

      pixel_temp* right_cell=&grid_temp[x+1][ty];
      pixel_temp* left_cell=&grid_temp[x-1][ty];
      pixel_temp* top_cell=&grid_temp[x][(y+1)%GRID_BUFFER_SIZE];
      pixel_temp* bottom_cell=&grid_temp[x][(y+2)%GRID_BUFFER_SIZE];
      
      //total number of particles in cell
      int32_t total_particles = source_cell->fuel + source_cell->air + source_cell->smoke;
      //particle imbalance (number of particles above/below our base threshold/pressure)
      int32_t pressure = (total_particles - 10000)/density_cnt;


      if (pressure > 0) {
        int32_t fuel_pressure = ( pressure * source_cell->fuel ) / total_particles;
        int32_t air_pressure = ( pressure * source_cell->air ) / total_particles;
        int32_t smoke_pressure = ( pressure * source_cell->smoke ) / total_particles;


        //remove particles from this cell
        source_cell->remove_fuel += fuel_pressure;
        source_cell->remove_air += air_pressure;
        source_cell->remove_smoke += smoke_pressure;

        //number of particles to push to neighboring cells
        int32_t moved_total = pressure /= 4;
        int32_t moved_fuel = fuel_pressure /= 4;
        int32_t moved_air = air_pressure /= 4;
        int32_t moved_smoke = smoke_pressure /= 4;

          //right cell
          if (x < FIRE_GRID_WIDTH - 1) {
            velocity_transfer(*source_cell, *right_cell, moved_fuel, moved_air, moved_smoke, moved_total);
          }

          //left cell
          if (x > 0) {
            velocity_transfer(*source_cell, *left_cell, moved_fuel, moved_air, moved_smoke, moved_total);
          }

          //top cell
          if (y < FIRE_GRID_HEIGHT - 1) {
            velocity_transfer(*source_cell, *top_cell, moved_fuel, moved_air, moved_smoke, moved_total);
          }

          //bottom cell
          if (y > 0) {
            velocity_transfer(*source_cell, *top_cell, moved_fuel, moved_air, moved_smoke, moved_total);
          }
        
      }

      if ( pressure < 0 ) {
        //number of particles to grab from adjacent cells
        pressure /= -4;

        uint8_t off_grid_cnt = 4;
        //right cell
        if (x < FIRE_GRID_WIDTH - 1) {
          pull_particles(*source_cell, *right_cell, pressure, -1, 0);
          off_grid_cnt--;
        }
        
        //left cell
        if (x > 0) {
          pull_particles(*source_cell, *left_cell, pressure, 1, 0);
          off_grid_cnt--;
        } 

        //top cell
        if (y < FIRE_GRID_HEIGHT - 1) {
          pull_particles(*source_cell, *top_cell, pressure, 0, -1);
          off_grid_cnt--;
        } 

        //bottom cell
        if (y > 0) {
          pull_particles(*source_cell, *bottom_cell, pressure, 0, 1);
          off_grid_cnt--;
        } 

        if (off_grid_cnt > 0) {
          pull_air(*source_cell, pressure*off_grid_cnt);
        }
        
      
      }
      
      

    }






    void resolve_density(int x, int y2) {
      pixel_temp * cell = &grid_temp[x][y2%GRID_BUFFER_SIZE];
      cell->fuel = _max(cell->fuel - cell->remove_fuel,0);
      cell->remove_fuel = 0;
      cell->air = _max(cell->air - cell->remove_air,0);
      cell->remove_air = 0;
      cell->smoke = _max(cell->smoke - cell->remove_smoke,0);
      cell->remove_smoke = 0;
      int32_t total_existing = cell->fuel + cell->air + cell->smoke;
      int32_t total_new = cell->add_fuel + cell->add_air + cell->add_smoke;
      if (total_new + total_existing != 0) {
        cell->smoke += cell->add_smoke;
        cell->add_smoke = 0;
        cell->air += cell->add_air;
        cell->add_air = 0;
        cell->fuel += cell->add_fuel;
        cell->add_fuel = 0;
        int32_t new_vx_portion = (cell->add_vx * total_new)/(total_new + total_existing);
        int32_t existing_vx_portion = (cell->vx * total_existing)/(total_new + total_existing);
        int32_t new_vy_portion = (cell->add_vy * total_new)/(total_new + total_existing);
        int32_t existing_vy_portion = (cell->vy * total_existing)/(total_new + total_existing);
        cell->vx = new_vx_portion + existing_vx_portion;
        cell->add_vx = 0;
        cell->vy = new_vy_portion + existing_vy_portion;
        cell->add_vy = 0;
      }
    }

};



LIGHT_SKETCHES::REGISTER<FIRE2> fire2("fire2");