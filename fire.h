//FIRE

//this class uses malloc for the grid, because it gets HUGE with large displays
//128x32 uses ~213KB
//should probaby optimize that somehow
#define FIRE_GRID_WIDTH (MATRIX_WIDTH)
#define FIRE_GRID_HEIGHT (MATRIX_HEIGHT)

class FIRE2: public LIGHT_SKETCH {
    public:
      FIRE2 () {setup();}
      ~FIRE2 () {
      for (int x = 0; x < FIRE_GRID_WIDTH; x++) {
        //delete grid[x];
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
    

    struct pixel {
        int32_t smoke = 0;
        int32_t fuel = 0;
        int32_t air = 10000; //10000 = base density
        int32_t vx = 0; //10000 = max velocity? (all particles move out)
        int32_t vy = 0;

    };

    struct pixel_temp {
        int32_t add_smoke = 0;
        int32_t add_fuel = 0;
        int32_t add_air = 0;
        int32_t add_vx = 0;
        int32_t add_vy = 0;
        int32_t remove_smoke = 0;
        int32_t remove_fuel = 0;
        int32_t remove_air = 0;
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

    void velocity_transfer(pixel &source_cell, pixel &target_cell, pixel_temp &source_buffer, pixel_temp &target_buffer, int32_t &moved_fuel, int32_t &moved_air, int32_t &moved_smoke, int32_t &moved_total ) { 
      int32_t existing_total = target_buffer.add_fuel + target_buffer.add_air + target_buffer.add_smoke;
      if (moved_total + existing_total == 0) {
        existing_total++; //avoid divide by zero
      }
      int32_t moved_vx_portion = (source_cell.vx * moved_total)/(moved_total + existing_total);
      int32_t existing_vx_portion = (target_buffer.add_vx * existing_total)/(moved_total + existing_total);
      int32_t moved_vy_portion = (source_cell.vy * moved_total)/(moved_total + existing_total);
      int32_t existing_vy_portion = (target_buffer.add_vy * existing_total)/(moved_total + existing_total);
      target_buffer.add_smoke += moved_smoke;
      target_buffer.add_fuel += moved_fuel;
      target_buffer.add_air += moved_air;
      target_buffer.add_vx = existing_vx_portion + moved_vx_portion;
      target_buffer.add_vy = existing_vy_portion + moved_vy_portion;
      
    }

    void pull_particles(pixel &source_cell, pixel &target_cell, pixel_temp &source_buffer, pixel_temp &target_buffer, int32_t &pressure, int x_dir, int y_dir) {

      //find the starting total of the target cell
      int32_t target_cell_total = target_cell.fuel + target_cell.air + target_cell.smoke;

      //find the cumulative total of the target cell
      int32_t target_cell_total2 = target_cell_total + target_buffer.remove_fuel + target_buffer.remove_air + target_buffer.remove_smoke;
      if ( target_cell_total2 > pressure && target_cell_total != 0 ) {
        int32_t smoke = ( pressure * target_cell.smoke ) / target_cell_total2;
        int32_t fuel = ( pressure * target_cell.fuel ) / target_cell_total2;
        int32_t air = ( pressure * target_cell.air ) / target_cell_total2;
        
        //record the removed particles
        target_buffer.remove_smoke += smoke;
        target_buffer.remove_fuel += fuel;
        target_buffer.remove_air += air;
        
        //find the total number of moved particles
        int32_t moved_total = fuel+air+smoke;

        //add the moved particles to the source cell, calculate velocities
        int32_t existing_total = source_buffer.add_fuel + source_buffer.add_air + source_buffer.add_smoke;
        if (moved_total + existing_total != 0) {
          int32_t moved_vx_portion = (target_cell.vx * moved_total)/(moved_total + existing_total);
          int32_t existing_vx_portion = (source_buffer.add_vx * existing_total)/(moved_total + existing_total);
          int32_t moved_vy_portion = (target_cell.vy * moved_total)/(moved_total + existing_total);
          int32_t existing_vy_portion = (source_buffer.add_vy * existing_total)/(moved_total + existing_total);
          source_buffer.add_fuel += fuel;
          source_buffer.add_air += air;
          source_buffer.add_smoke += smoke;
          source_buffer.add_vx = existing_vx_portion + moved_vx_portion;
          source_buffer.add_vy = existing_vy_portion + moved_vy_portion;

          //add a bit of velocity to the target cell (sucking effect?)
          target_cell.vx += moved_total * x_dir;
          target_cell.vy += moved_total * y_dir;
        }
        
      }
    }

    void pull_air(pixel &source_cell, pixel_temp &source_buffer, int32_t pressure) {
      //pull air from off the grid
      int32_t existing_total = source_buffer.add_fuel + source_buffer.add_air + source_buffer.add_smoke;
      if (pressure+existing_total != 0) {
        int32_t moved_vx_portion = 0;
        int32_t existing_vx_portion = (source_buffer.add_vx * existing_total)/(pressure + existing_total);
        int32_t moved_vy_portion = 0;
        int32_t existing_vy_portion = (source_buffer.add_vy * existing_total)/(pressure + existing_total);
        source_buffer.add_vx = existing_vx_portion + moved_vx_portion;
        source_buffer.add_vy = existing_vy_portion + moved_vy_portion;
      }
      source_buffer.add_air += pressure;
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
        grid_calcs = 2;
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
        //grid[x] = (pixel *) malloc (FIRE_GRID_HEIGHT * sizeof(pixel));
        for (int y = 0; y < FIRE_GRID_HEIGHT; y++) {
          grid[x][y].smoke = 0;
          grid[x][y].fuel = 0;
          grid[x][y].air = 10000; //10000 = base density
          grid[x][y].vx = 0; //10000 = max velocity? (all particles move out)
          grid[x][y].vy = 0;
        }
      }

      for (int x = 0; x < FIRE_GRID_WIDTH; x++) {
        //grid[x] = (pixel *) malloc (FIRE_GRID_HEIGHT * sizeof(pixel));
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
    void clear_buffer(int x, int y) {
      int ty = y%GRID_BUFFER_SIZE;
      grid_temp[x][ty].add_smoke = 0;
      grid_temp[x][ty].add_fuel = 0;
      grid_temp[x][ty].add_air = 0;
      grid_temp[x][ty].add_vx = 0;
      grid_temp[x][ty].add_vy = 0;
      grid_temp[x][ty].remove_smoke = 0;
      grid_temp[x][ty].remove_fuel = 0;
      grid_temp[x][ty].remove_air = 0;
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
                grid[gas[i].x/10000][gas[i].y/10000].fuel = _min(grid[gas[i].x/10000][gas[i].y/10000].fuel + gas[i].fuel*_max(_min(gas[i].vy,1600)-300,0), 60000);
                grid[gas[i].x/10000][gas[i].y/10000].air = _min(grid[gas[i].x/10000][gas[i].y/10000].air + gas[i].air*_max(_min(gas[i].vy,1600)-150,0), 60000);
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
//            grid[rx][y].vy = _min(grid[rx][y].vy + sq(120-y), 60000);
//            grid[rx][y].fuel = _min(grid[rx][y].fuel + rf, 60000);
//            grid[rx][y].air = _min(grid[rx][y].air + rf2, 60000);
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
              for (int y = 0; y < FIRE_GRID_HEIGHT+GRID_BUFFER_SIZE; y++) {
                
                for (int x = 0; x < FIRE_GRID_WIDTH; x++) {  
                  if (y > GRID_BUFFER_SIZE-2) {
                    resolve_velocity(x,y-(GRID_BUFFER_SIZE-1));
                  }
                }

                for (int x = 0; x < FIRE_GRID_WIDTH; x++) {
                  if (y < FIRE_GRID_HEIGHT) {
                    clear_buffer(x,y-(GRID_BUFFER_SIZE-1));
                    calculate_velocity(x,y);
                  }
                }

              }
            } else {
              //calculate in reverse starting at FIRE_GRID_HEIGHT, FIRE_GRID_WIDTH
              for (int y = FIRE_GRID_HEIGHT-1; y >= 0-(GRID_BUFFER_SIZE-1); y--) {
                
                for (int x = FIRE_GRID_WIDTH-1; x >= 0; x--) {
                  
                  if (y < (FIRE_GRID_HEIGHT-1) - (GRID_BUFFER_SIZE-2)) {
                    resolve_velocity(x,y+(GRID_BUFFER_SIZE-1));
                  }

                  if (y >= 0) {
                    clear_buffer(x,y+(GRID_BUFFER_SIZE-1));
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
              for (int y = 0; y < FIRE_GRID_HEIGHT+GRID_BUFFER_SIZE; y++) {
                
                for (int x = 0; x < FIRE_GRID_WIDTH; x++) {
                  
                  if (y > GRID_BUFFER_SIZE-2) {
                    resolve_density(x,y-(GRID_BUFFER_SIZE-1));
                  }

                  if (y < FIRE_GRID_HEIGHT) {
                    clear_buffer(x,y-(GRID_BUFFER_SIZE-1-buffer_mod));
                    calculate_density(x,y,density_cnt);
                  }
                }

              }


            } else {
              //std::cout << "reverse\n";

              //calculate in reverse starting at FIRE_GRID_HEIGHT, FIRE_GRID_WIDTH
              for (int y = FIRE_GRID_HEIGHT-1; y >= 0-(GRID_BUFFER_SIZE-1); y--) {
                
                for (int x = FIRE_GRID_WIDTH-1; x >= 0; x--) {
                  
                  if (y < (FIRE_GRID_HEIGHT-1) - (GRID_BUFFER_SIZE-2)) {
                    resolve_density(x,y+(GRID_BUFFER_SIZE-1));
                  }

                  if (y >= 0) {
                    clear_buffer(x,y+(GRID_BUFFER_SIZE-1-buffer_mod));
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
          //add drag
          if (grid[x][y].vx > 5 ){
            grid[x][y].vx *= .99f; //caused wierd lines
          }
            if (grid[x][y].vy > 5 ){
            grid[x][y].vy *= .95f; //caused wierd lines
          }
          //smoke to air
          int32_t smoke_dissipate = grid[x][y].smoke/20;
          grid[x][y].smoke -= smoke_dissipate;
          grid[x][y].air += smoke_dissipate;
          
          //burn fuel
          if (grid[x][y].fuel > 0 && grid[x][y].air > 0) {
            int32_t burned_fuel = _min((int32_t)grid[x][y].fuel, grid[x][y].air/2);
            grid[x][y].fuel = grid[x][y].fuel - burned_fuel;
            grid[x][y].air = grid[x][y].air - burned_fuel*2;
            grid[x][y].smoke += burned_fuel;
            if (heat_acceleration == 1 && grid[x][y].vy < burned_fuel*30) {
              grid[x][y].vy = _min(grid[x][y].vy + (burned_fuel*burned_fuel)/100, 30000);
            }
            //draw flame
            if(draw_flame == 1 && burned_fuel > 9) {
              drawXY(leds, x, y, _max(_min((burned_fuel*2)/6, 255), 0)/10, 255 - _max(_min(burned_fuel/2-255, 255), 32)/4, _max(_min(burned_fuel/2, 255), 0));
              //drawXY(leds, x*2+1, y*2, _max(_min((burned_fuel*2)/6, 255), 0)/10, 255 - _max(_min(burned_fuel/2-255, 255), 32)/4, _max(_min(burned_fuel/2, 255), 0));
              //drawXY(leds, x*2, y*2+1, _max(_min((burned_fuel*2)/6, 255), 0)/10, 255 - _max(_min(burned_fuel/2-255, 255), 32)/4, _max(_min(burned_fuel/2, 255), 0));
              //drawXY(leds, x*2+1, y*2+1, _max(_min((burned_fuel*2)/6, 255), 0)/10, 255 - _max(_min(burned_fuel/2-255, 255), 32)/4, _max(_min(burned_fuel/2, 255), 0));
            } else {
              //grid[x][y].vy *= .9f;
            }
            if(draw_fuel == 1 && grid[x][y].fuel > 500) {
              drawXY(leds, x, y, _max(_min((grid[x][y].fuel-500)/15, 255), 0)/20, 255 - _max(_min((grid[x][y].fuel-500)/15-255, 255), 32)/4, _max(_min((grid[x][y].fuel-500)/15, 255), 0)); 
              //drawXY(leds, x*2+1, y*2, _max(_min((grid[x][y].fuel-500)/15, 255), 0)/20, 255 - _max(_min((grid[x][y].fuel-500)/15-255, 255), 32)/4, _max(_min((grid[x][y].fuel-500)/15, 255), 0)); 
              //drawXY(leds, x*2, y*2+1, _max(_min((grid[x][y].fuel-500)/15, 255), 0)/20, 255 - _max(_min((grid[x][y].fuel-500)/15-255, 255), 32)/4, _max(_min((grid[x][y].fuel-500)/15, 255), 0)); 
              //drawXY(leds, x*2+1, y*2+1, _max(_min((grid[x][y].fuel-500)/15, 255), 0)/20, 255 - _max(_min((grid[x][y].fuel-500)/15-255, 255), 32)/4, _max(_min((grid[x][y].fuel-500)/15, 255), 0)); 
            }
          }
            //draw air and smoke for debugging
            if (draw_air) {
              drawXY(leds, x, y, 96, 255, _min(_max((grid[x][y].air)/100,0),255));
            }
            if (draw_smoke) {
              //drawXY(leds, x, y, 160, 255, _min(_max((grid[x][y].smoke)/100,0),255));
            }
          //constrain to avoid huge stuff/overflow
          grid[x][y].fuel = _max(_min((int32_t)grid[x][y].fuel, 60000), -60000);
          grid[x][y].air = _max(_min((int32_t)grid[x][y].air, 60000), -60000);
          grid[x][y].smoke = _max(_min((int32_t)grid[x][y].smoke, 60000), -60000);
          grid[x][y].vx = _max(_min((int32_t)grid[x][y].vx, 60000), -60000);
          grid[x][y].vy = _max(_min((int32_t)grid[x][y].vy, 60000), -60000);
        }
      }

      //flamethrower effect
      if (current_variation == 2 /*&& grid_cnt == grid_cnt*/) {
        //flame throwy effect
        static int flame_cycle = 0;
        static uint8_t y_from = 0;
        static uint8_t y_to = 100;
        if(flame_cycle < 255) {
          y_from = 0;
          y_to = _min(ease8Out(flame_cycle)*100/255,100);
          flame_cycle+=8;
        } else if (flame_cycle > (620-255) && flame_cycle < 620 ) {
          y_from = _min(ease8Out(flame_cycle-(620-255))*100/255,100);
          y_to = 100;
          flame_cycle+=8;
        } else {
          flame_cycle++;
          if (flame_cycle > 720) {
            flame_cycle = 0;
          }
        }
        for (int i = y_from; i < y_to; i++) {
          int x = (FIRE_GRID_WIDTH-1)/2+random(0,2);
          grid[x][i].vy += (100)*random(200);
          grid[x][i].vx += (i)*random(200)-(i)*100;
          
          int r = random((i+1)*00, (i+15)*90);
          r = (r*r)/((i+1)*100);
          if (flame_cycle < 255 && y_to - i < 20) {
            r*=10;
          }
          grid[x][i].fuel += r*2;
          int r2 = random(0,i*175);
          
          //if (flame_cycle > 275 && flame_cycle < 511) {
          //  r2 = (r2*3)/2;
          //}
          grid[x][i].air += r2;
        }
      }


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






    void calculate_velocity(int x, int y) {
      int ty = y%GRID_BUFFER_SIZE;

      //transfer particles between cells based on velocity (10000 max velocity, all particles move)
      int32_t total = grid[x][y].fuel + grid[x][y].air + grid[x][y].smoke;
      int32_t total_moved_fuel = 0;
      int32_t total_moved_air = 0;
      int32_t total_moved_smoke = 0;
      int32_t v_total = abs(grid[x][y].vx)+abs(grid[x][y].vy);
      
      if (v_total != 0) {
        int32_t v_total_constrain = _min(v_total, 10000);
        int32_t x_portion = (abs(grid[x][y].vx)*v_total_constrain) / v_total;
        int32_t y_portion = (abs(grid[x][y].vy)*v_total_constrain) / v_total;
      
        
        shuffle_order();
        for (int i = 0; i < 4; i++) {

          if (order[i] == 0 && grid[x][y].vx > 0) {
            int32_t moved_fuel = (grid[x][y].fuel * x_portion) / 10000;
            int32_t moved_air = (grid[x][y].air * x_portion) / 10000;
            int32_t moved_smoke = (grid[x][y].smoke * x_portion) / 10000;
            int32_t moved_total = moved_fuel + moved_air + moved_smoke;
            
            if (moved_total > 0 && x < FIRE_GRID_WIDTH - 1) {
              velocity_transfer(grid[x][y], grid[x+1][y], grid_temp[x][ty], grid_temp[x+1][ty], moved_fuel, moved_air, moved_smoke, moved_total);
            }
            
            total_moved_smoke += moved_smoke;
            total_moved_fuel += moved_fuel;
            total_moved_air += moved_air;  
          }

          if (order[i] == 1 && grid[x][y].vx < 0) {
            int32_t moved_fuel = (grid[x][y].fuel * x_portion) / 10000;
            int32_t moved_air = (grid[x][y].air * x_portion) / 10000;
            int32_t moved_smoke = (grid[x][y].smoke * x_portion) / 10000;
            int32_t moved_total = moved_fuel + moved_air + moved_smoke;
            if (moved_total > 0 && x > 0) {
              velocity_transfer(grid[x][y], grid[x-1][y], grid_temp[x][ty], grid_temp[x-1][ty], moved_fuel, moved_air, moved_smoke, moved_total);
            }
            total_moved_smoke += moved_smoke;
            total_moved_fuel += moved_fuel;
            total_moved_air += moved_air;
          }
          
          if (order[i] == 2 && grid[x][y].vy > 0) {
            int32_t moved_fuel = (grid[x][y].fuel * y_portion) / 10000;
            int32_t moved_air = (grid[x][y].air * y_portion) / 10000;
            int32_t moved_smoke = (grid[x][y].smoke * y_portion) / 10000;
            int32_t moved_total = moved_fuel + moved_air + moved_smoke;
            if (moved_total > 0 && y < FIRE_GRID_HEIGHT - 1) {
              velocity_transfer(grid[x][y], grid[x][y+1], grid_temp[x][ty], grid_temp[x][(y+1)%GRID_BUFFER_SIZE], moved_fuel, moved_air, moved_smoke, moved_total);
            }
            total_moved_smoke += moved_smoke;
            total_moved_fuel += moved_fuel;
            total_moved_air += moved_air;
          }

          if (order[i] == 3 && grid[x][y].vy < 0) {
            int32_t moved_fuel = (grid[x][y].fuel * y_portion) / 10000;
            int32_t moved_air = (grid[x][y].air * y_portion) / 10000;
            int32_t moved_smoke = (grid[x][y].smoke * y_portion) / 10000;
            int32_t moved_total = moved_fuel + moved_air + moved_smoke;
            if (moved_total > 0 && y > 0) {
              velocity_transfer(grid[x][y], grid[x][y-1], grid_temp[x][ty], grid_temp[x][(y-1)%GRID_BUFFER_SIZE], moved_fuel, moved_air, moved_smoke, moved_total);
            }
            total_moved_smoke += moved_smoke;
            total_moved_fuel += moved_fuel;
            total_moved_air += moved_air;
          }
        }
      
          grid[x][y].smoke -= total_moved_smoke; //looked kind of neat without this line
          grid[x][y].fuel -= total_moved_fuel;
          grid[x][y].air -= total_moved_air;
      }
    }







    void resolve_velocity(int x, int y) {
      int ty = y%GRID_BUFFER_SIZE;
      int32_t total_existing = grid[x][y].fuel + grid[x][y].air + grid[x][y].smoke;
      int32_t total_new = grid_temp[x][ty].add_fuel + grid_temp[x][ty].add_air + grid_temp[x][ty].add_smoke;
      if (total_new + total_existing == 0) {
        total_existing++;
      }
      grid[x][y].smoke += grid_temp[x][ty].add_smoke;
      grid_temp[x][ty].add_smoke = 0;
      grid[x][y].air += grid_temp[x][ty].add_air;
      grid_temp[x][ty].add_air = 0;
      grid[x][y].fuel += grid_temp[x][ty].add_fuel;
      grid_temp[x][ty].add_fuel = 0;
      int32_t new_vx_portion = (grid_temp[x][ty].add_vx * total_new)/(total_new + total_existing);
      int32_t existing_vx_portion = (grid[x][y].vx * total_existing)/(total_new + total_existing);
      int32_t new_vy_portion = (grid_temp[x][ty].add_vy * total_new)/(total_new + total_existing);
      int32_t existing_vy_portion = (grid[x][y].vy * total_existing)/(total_new + total_existing);
      grid[x][y].vx = new_vx_portion + existing_vx_portion;
      grid_temp[x][ty].add_vx = 0;
      grid[x][y].vy = new_vy_portion + existing_vy_portion;
      grid_temp[x][ty].add_vy = 0;
      
    }






    void calculate_density(int x, int y, int density_cnt) {
      int ty = y%GRID_BUFFER_SIZE;
      //total number of particles in cell
      int32_t total_particles = grid[x][y].fuel + grid[x][y].air + grid[x][y].smoke;
      //particle imbalance (number of particles above/below our base threshold/pressure)
      int32_t pressure = (total_particles - 10000)/density_cnt;

      if (pressure > 0) {
        int32_t fuel_pressure = ( pressure * grid[x][y].fuel ) / total_particles;
        int32_t air_pressure = ( pressure * grid[x][y].air ) / total_particles;
        int32_t smoke_pressure = ( pressure * grid[x][y].smoke ) / total_particles;


        //remove particles from this cell
        grid_temp[x][ty].remove_fuel += fuel_pressure;
        grid_temp[x][ty].remove_air += air_pressure;
        grid_temp[x][ty].remove_smoke += smoke_pressure;

        //number of particles to push to neighboring cells
        int32_t moved_total = pressure /= 4;
        int32_t moved_fuel = fuel_pressure /= 4;
        int32_t moved_air = air_pressure /= 4;
        int32_t moved_smoke = smoke_pressure /= 4;

        shuffle_order();
        for (int i = 0; i < 4; i++) {
          //right cell
          if (order[i] == 0 && x < FIRE_GRID_WIDTH - 1) {
              velocity_transfer(grid[x][y], grid[x+1][y], grid_temp[x][y], grid_temp[x+1][ty], moved_fuel, moved_air, moved_smoke, moved_total); 
          }

          //left cell
          if (order[i] == 1 && x > 0) {
              velocity_transfer(grid[x][y], grid[x-1][y], grid_temp[x][y], grid_temp[x-1][ty], moved_fuel, moved_air, moved_smoke, moved_total);
          }

          //top cell
          if (order[i] == 2 && y < FIRE_GRID_HEIGHT - 1) {
              velocity_transfer(grid[x][y], grid[x][y+1], grid_temp[x][y], grid_temp[x][(y+1)%GRID_BUFFER_SIZE], moved_fuel, moved_air, moved_smoke, moved_total);
          }

          //bottom cell
          if (order[i] == 3 && y > 0) {
              velocity_transfer(grid[x][y], grid[x][y-1], grid_temp[x][y], grid_temp[x][(y-1)%GRID_BUFFER_SIZE], moved_fuel, moved_air, moved_smoke, moved_total);
          }
        }
      }

      if ( pressure < 0 ) {
        //number of particles to grab from adjacent cells
        pressure /= -4;

        shuffle_order();
        for (int i = 0; i < 4; i++) {

          //right cell
          if (order[i] == 0) {
            if (x < FIRE_GRID_WIDTH - 1) {
              pull_particles(grid[x][y], grid[x+1][y], grid_temp[x][ty], grid_temp[x+1][ty], pressure, -1, 0);
            } else {
              //pull air from off the grid
              pull_air(grid[x][y], grid_temp[x][ty], pressure);
            }
          }
          
          //left cell
          if (order[i] == 1) {
            if (x > 0) {
              pull_particles(grid[x][y], grid[x-1][y], grid_temp[x][ty], grid_temp[x-1][ty], pressure, 1, 0);
            } else {
              //pull air from off the grid
              pull_air(grid[x][y], grid_temp[x][ty], pressure);
            }
          }

          //top cell
          if (order[i] == 2) {
            if (y < FIRE_GRID_HEIGHT - 1) {
              pull_particles(grid[x][y], grid[x][y+1], grid_temp[x][ty], grid_temp[x][(y+1)%GRID_BUFFER_SIZE], pressure, 0, -1);
            } else {
              //pull air from off the grid
              pull_air(grid[x][y], grid_temp[x][ty], pressure);
            }
          }

          //bottom cell
          if (order[i] == 3) {
            if (y > 0) {
              pull_particles(grid[x][y], grid[x][y-1], grid_temp[x][ty], grid_temp[x][(y-1)%GRID_BUFFER_SIZE], pressure, 0, 1);
            } else {
              //pull air from off the grid
              pull_air(grid[x][y], grid_temp[x][ty], pressure);
            }
          }

        }
      
      }
      
      

    }






    void resolve_density(int x, int y) {
      int ty = y%GRID_BUFFER_SIZE;
      grid[x][y].fuel = _max(grid[x][y].fuel - grid_temp[x][ty].remove_fuel,0);
      grid_temp[x][ty].remove_fuel = 0;
      grid[x][y].air = _max(grid[x][y].air - grid_temp[x][ty].remove_air,0);
      grid_temp[x][ty].remove_air = 0;
      grid[x][y].smoke = _max(grid[x][y].smoke - grid_temp[x][ty].remove_smoke,0);
      grid_temp[x][ty].remove_smoke = 0;
      int32_t total_existing = grid[x][y].fuel + grid[x][y].air + grid[x][y].smoke;
      int32_t total_new = grid_temp[x][ty].add_fuel + grid_temp[x][ty].add_air + grid_temp[x][ty].add_smoke;
      if (total_new + total_existing != 0) {
        grid[x][y].smoke += grid_temp[x][ty].add_smoke;
        grid_temp[x][ty].add_smoke = 0;
        grid[x][y].air += grid_temp[x][ty].add_air;
        grid_temp[x][ty].add_air = 0;
        grid[x][y].fuel += grid_temp[x][ty].add_fuel;
        grid_temp[x][ty].add_fuel = 0;
        int32_t new_vx_portion = (grid_temp[x][ty].add_vx * total_new)/(total_new + total_existing);
        int32_t existing_vx_portion = (grid[x][y].vx * total_existing)/(total_new + total_existing);
        int32_t new_vy_portion = (grid_temp[x][ty].add_vy * total_new)/(total_new + total_existing);
        int32_t existing_vy_portion = (grid[x][y].vy * total_existing)/(total_new + total_existing);
        grid[x][y].vx = _min(_max(new_vx_portion + existing_vx_portion,-60000),60000);
        grid_temp[x][ty].add_vx = 0;
        grid[x][y].vy = _min(_max(new_vy_portion + existing_vy_portion,-60000),60000);
        grid_temp[x][ty].add_vy = 0;
      }
    }

};



LIGHT_SKETCHES::REGISTER<FIRE2> fire2("fire2");