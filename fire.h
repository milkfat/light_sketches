#ifndef LIGHTS_FIRE_H
#define LIGHTS_FIRE_H

//FIRE

#define FIRE_GRID_BORDER 1
#define FIRE_GRID_WIDTH (MATRIX_WIDTH+FIRE_GRID_BORDER*2)
#define FIRE_GRID_HEIGHT (MATRIX_HEIGHT/2+FIRE_GRID_BORDER*2)

#define FIRE_GRID_VELOCITY_MAX  1000000
#define FIRE_GRID_VELOCITY_MIN -1000000
#define FIRE_GRID_MAX  1000000
#define FIRE_GRID_MIN -1000000

#define CELL_DIVISOR 1

typedef int32_t fire_cell_t; //int16_t
typedef int32_t fire_vel_t; //cint18
typedef int32_t fire_calc_t; //int32_t
class FIRE2: public LIGHT_SKETCH {
    public:
      FIRE2 () {setup();}
      ~FIRE2 () {}

    private:

    #define MAX_NUM_GAS 100
    uint8_t num_gas = 10;

    uint32_t order[FIRE_GRID_WIDTH*FIRE_GRID_HEIGHT];
    uint8_t current_variation = 1;
    uint8_t grid_calcs = 1;
    uint8_t velocity_calcs = 1;
    uint8_t density_calcs = 1;
    uint8_t heat_acceleration = 1;
    uint32_t fuel_injection = 8000;
    uint32_t air_injection = 15000;
    bool draw_flame = 0;
    bool draw_fuel = 1;
    bool draw_air = 0;
    bool draw_smoke = 0;
    uint8_t buffer_mod = 0;

    bool flame_button = false;
    bool flame_on = false;

    struct cell_info {
      fire_calc_t moved_air = 0;
      fire_calc_t moved_fuel = 0;
      fire_calc_t moved_smoke = 0;
      fire_calc_t moved_total = 0;
      fire_calc_t pressure = 0;
      fire_calc_t x_dir = 0;
      fire_calc_t y_dir = 0;
      fire_calc_t vx = 0;
      fire_calc_t vy = 0;
      fire_calc_t pvx = 0;
      fire_calc_t pvy = 0;

    };

    struct pixel_full;

    struct pixel {
        fire_cell_t smoke;
        fire_cell_t fuel;
        fire_cell_t air;
        fire_vel_t vx;
        fire_vel_t vy;
    };

    struct pixel_full {
        int32_t smoke;
        int32_t fuel;
        int32_t air;
        int32_t vx;
        int32_t vy;
    };


    pixel_full * left_cell;
    pixel_full * right_cell;
    pixel_full * top_cell;
    pixel_full * bottom_cell;

    pixel grid[FIRE_GRID_HEIGHT][FIRE_GRID_WIDTH];

    inline void velocity_transfer(const pixel_full& source_cell, pixel_full& target_cell, const cell_info& ci) { 
      fire_calc_t existing_total = target_cell.fuel + target_cell.air + target_cell.smoke;
      if (ci.moved_total + existing_total == 0) {
        return; //avoid divide by zero
      }
      fire_calc_t total = ci.moved_total + existing_total;
   
      target_cell.smoke += ci.moved_smoke;
      target_cell.fuel += ci.moved_fuel;
      target_cell.air += ci.moved_air;
      float existing_ratio = existing_total/(float)total;
      float moved_ratio = ci.moved_total/(float)total;
      target_cell.vx = source_cell.vx * moved_ratio + target_cell.vx * existing_ratio;
      target_cell.vy = source_cell.vy * moved_ratio + target_cell.vy * existing_ratio;
     
          target_cell.vx += ci.x_dir * ci.pressure/4;
          target_cell.vy += ci.y_dir * ci.pressure/4;
    }

    void pull_air(pixel_full& source_cell, const cell_info& ci) {
      //pull air from off the grid
      fire_calc_t existing_total = source_cell.fuel + source_cell.air + source_cell.smoke;
      fire_calc_t total = ci.pressure + existing_total;
      if (total != 0) {
        float ratio = existing_total/(float)total;
        source_cell.vx = source_cell.vx * ratio;
        source_cell.vy = source_cell.vy * ratio;
      }
      source_cell.air += ci.pressure;
    }

    struct GAS {
      int32_t x = -1;
      int32_t y = -1;
      int32_t vx = 50;
      int32_t vy = 100;
      int32_t fuel = 10;
      int32_t air = 5;
    };

    GAS gas[MAX_NUM_GAS];

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
        grid_calcs = 1;
        velocity_calcs = 1;
        heat_acceleration = 1;
        draw_fuel = 1;
        draw_flame = 0;
        buffer_mod = 1;
        draw_air = 1;
        draw_smoke = 1;
      } else if (current_variation == 2) {
        grid_calcs = 1;
        velocity_calcs = 1;
        heat_acceleration = 1;
        draw_fuel = 1;
        draw_flame = 0;
        buffer_mod = 0;
        draw_air = 0;
        draw_smoke = 0;
      } else if (current_variation == 1) {
        grid_calcs = 1;
        velocity_calcs = 1;
        heat_acceleration = 1;
        draw_fuel = 1;
        draw_flame = 1;
        buffer_mod = 0;
        draw_air = 0;
        draw_smoke = 1;
      } else if (current_variation == 0) {
        grid_calcs = 2;
        velocity_calcs = 2;
        heat_acceleration = 1;
        draw_fuel = 1;
        draw_flame = 0;
        buffer_mod = 0;
        draw_air = 0;
        draw_smoke = 0;
      }
    }

    void setup() {
      // for (int i = 0; i < FIRE_GRID_HEIGHT; i++) {
      //   yorder[i] = i;
      // }
      // for (int i = 0; i < FIRE_GRID_WIDTH; i++) {
      //   xorder[i] = i;
      // }
      //order = reinterpret_cast<uint16_t*>(height_map[0]);
      control_variables.add(flame_button,"Flame Thrower!", 1);
      control_variables.add(draw_flame,"Draw Flame", 0);
      control_variables.add(draw_fuel,"Draw Fuel", 0);
      control_variables.add(draw_smoke,"Draw Smoke", 0);
      control_variables.add(draw_air,"Draw Air", 0);
      control_variables.add(fuel_injection,"Amount of Fuel", 0, 1000000);
      control_variables.add(air_injection,"Amount of Air", 0, 1000000);
      control_variables.add(num_gas,"Number of Gas Jets", 0, 100);
      for (int i = 0; i < FIRE_GRID_WIDTH*FIRE_GRID_HEIGHT; i++) {
        order[i] = i;
      }
      
      for (int i = 0; i < MAX_NUM_GAS; i++) {
        gas[i].y = -1;
      }

      reset();


      //initialize the first effect
      next_effect();

    }

    void reset() {
      
      //initialize our grid
      for (int y = 0; y < FIRE_GRID_HEIGHT; y++) {
        for (int x = 0; x < FIRE_GRID_WIDTH; x++) {
        //grid_temp[x] = (pixel *) malloc (FIRE_GRID_HEIGHT * sizeof(pixel));
          grid[y][x].smoke=0;
          grid[y][x].fuel=0;
          grid[y][x].air=10000/CELL_DIVISOR; //10000 = base density
          grid[y][x].vx=0; //10000 = max velocity? (all particles move out)
          grid[y][x].vy=0;
        }
      }

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

        if(random(100) == 0) {
          int rh = random(FIRE_GRID_HEIGHT);
          int rw = random(FIRE_GRID_WIDTH);
          grid[rh][rw].fuel = 2000000;
          grid[rh][rw].air = 2000000;
        }
       
        //display and blackout

        LED_show();
        LED_black();
        // for (int i = 0; i < NUM_LEDS; i++) {
        //   leds[i].r /= 2;
        //   leds[i].g /= 2;
        //   leds[i].b /= 2;
        // }

        static unsigned long gas_time = millis();
        static uint16_t gas_pos = 0;
        static int gas_delay = 50;


        if (current_variation == 1) {
          //add roman candle fireball
          if(millis() - gas_delay > gas_time) {
            gas_time = millis();
            gas_delay = random(400);
            gas[gas_pos].vx = random(100, 5000);
            gas[gas_pos].vy = 0;
            gas[gas_pos].x = 0;
            gas[gas_pos].y = random(50000);
            if (random(2)) {
              gas[gas_pos].x = FIRE_GRID_WIDTH*10000-1;
              gas[gas_pos].vx *= -1;
            }
            gas[gas_pos].fuel = 10;
            gas[gas_pos].air = 15;
            gas_pos++;
            if(gas_pos > num_gas - 1) {
              gas_pos = 0;
            }
          }
        
        
          //update roman candle fireballs
          for (int i = 0; i < num_gas; i++) {
            int j = 0;
            while (j < 1) {
              if (gas[i].x != -1) {
                //gas[i].x += gas[i].vx;
                gas[i].x = inoise8(millis()/2,0,i*10000)*FIRE_GRID_WIDTH*2 + (i*FIRE_GRID_WIDTH*256*5)/6;
                gas[i].x %= FIRE_GRID_WIDTH*256;
                //gas[i].y = inoise8(0,millis()/2,i*10000)*800 - 90000;
                gas[i].y = 0;
                int32_t x = gas[i].x/256;
                int32_t y = gas[i].y/256;

                gas[i].vx = random(50000);
                gas[i].vy = random(500000);
                if (random(2)) {
                  gas[i].vx *= -1;
                }
                //y = 30;
                //gas[i].vx *= .99f;
                if(x < 0 || x > (FIRE_GRID_WIDTH-1) || y < 0 || y > (FIRE_GRID_HEIGHT-1))
                {  
                  //do nothing
                }
                else
                {
                  grid[y][x].vy = gas[i].vy/CELL_DIVISOR;
                  grid[y][x].vx = gas[i].vx/CELL_DIVISOR;
                  //grid[y][x].vy = 400;
                  if (i%2)
                  {
                    grid[y][x].fuel = _min(grid[y][x].fuel*CELL_DIVISOR + random(fuel_injection), FIRE_GRID_MAX)/CELL_DIVISOR;
                  } 
                  else
                  {
                    grid[y][x].air = _min(grid[y][x].air*CELL_DIVISOR + random(air_injection), FIRE_GRID_MAX)/CELL_DIVISOR;
                  }
                }
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
            //shuffle_order();
            
            pixel_full source_cell;
            pixel_full left_cell_;
            pixel_full right_cell_;
            pixel_full top_cell_;
            pixel_full bottom_cell_;
            for (int i = 0; i < FIRE_GRID_WIDTH*FIRE_GRID_HEIGHT; i++) {
              //uint16_t y = order[i%(FIRE_GRID_WIDTH*FIRE_GRID_HEIGHT)] / FIRE_GRID_WIDTH;
              //uint16_t x = order[i%(FIRE_GRID_WIDTH*FIRE_GRID_HEIGHT)] % FIRE_GRID_WIDTH;
               static uint16_t y = 0;
               static uint16_t x = 0;

              x+=random(9)+1;
              if (x >= FIRE_GRID_WIDTH) {
                x-=FIRE_GRID_WIDTH;
                y++;
                if ( y >= FIRE_GRID_HEIGHT) {
                  y-=FIRE_GRID_HEIGHT;
                }
              }


              cell_d(source_cell, grid[y][x]);

              left_cell = (x > 0) ? cell_d(left_cell_, grid[y][x-1]) : nullptr;
              right_cell = (x < FIRE_GRID_WIDTH-1) ? cell_d(right_cell_, grid[y][x+1]) : nullptr;
              top_cell = (y < FIRE_GRID_HEIGHT-1) ? cell_d(top_cell_, grid[y+1][x]) : nullptr;
              bottom_cell = (y > 0) ? cell_d(bottom_cell_, grid[y-1][x]) : nullptr;

              calculate_density(source_cell);
              calculate_velocity(source_cell);

              cell_c(grid[y][x], &source_cell);
              cell_c(grid[y][x-1], left_cell);
              cell_c(grid[y][x+1], right_cell);
              cell_c(grid[y+1][x], top_cell);
              cell_c(grid[y-1][x], bottom_cell);
              
            }
             
             
  
            
            

          }
          //END CALCULATE VELOCITIES
          

          }  
      
        //END CALCULATE GRID
        
     //debug_micros1 += micros() - debug_time2;
      }
    }//LOOP

  static inline pixel_full * cell_d(pixel_full& pf, pixel& p) {
    pf.fuel = p.fuel*CELL_DIVISOR;
    pf.air = p.air*CELL_DIVISOR;
    pf.smoke = p.smoke*CELL_DIVISOR;
    pf.vx = p.vx*CELL_DIVISOR;
    pf.vy = p.vy*CELL_DIVISOR;
    return &pf;
  }

  static inline void cell_c(pixel& p, pixel_full * pf) {
    if (!pf) return;
    p.fuel = pf->fuel/CELL_DIVISOR;
    p.air = pf->air/CELL_DIVISOR;
    p.smoke = pf->smoke/CELL_DIVISOR;
    p.vx = pf->vx/CELL_DIVISOR;
    p.vy = pf->vy/CELL_DIVISOR;
  }

  void shuffle_order() {
    for (int j = 0; j < FIRE_GRID_WIDTH*FIRE_GRID_HEIGHT/8; j++) {
      uint32_t rand = random(FIRE_GRID_WIDTH*FIRE_GRID_HEIGHT);
      uint32_t temp = order[j];
      order[j] = order[rand];
      order[rand] = temp;
    }
  }

    void process_grid_effects(const int& grid_cnt) {

      uint16_t r;
      uint16_t g;
      uint16_t b;
      uint32_t burned_fuel; 
      fire_calc_t air;
      fire_calc_t fuel;
      fire_calc_t smoke;
      pixel * cell;
      //burn fuel
      for (int y = 0; y < FIRE_GRID_HEIGHT; y++) {
        for (int x = 0; x < FIRE_GRID_WIDTH; x++) {
          cell = &grid[y][x];
          air = cell->air*CELL_DIVISOR;
          fuel = cell->fuel*CELL_DIVISOR;
          smoke = _max(cell->smoke, 0)*CELL_DIVISOR;
          cell->vy += cell->smoke/8;
          cell->vx -= ( cell->vx * cell->air ) / (_max( cell->air + cell->smoke + cell->fuel, 1)*2);
          cell->vy -= ( cell->vy * cell->air ) / (_max( cell->air + cell->smoke + cell->fuel, 1)*2);
          //add drag
          //smoke to air

          // if (smoke > 199) {
          //   fire_calc_t smoke_dissipate = smoke/200;
          //   smoke -= smoke_dissipate;
          //   air += smoke_dissipate;
          // }
          //burn fuel
          r = 0;
          g = 0;
          b = 0;
          burned_fuel = 0;
          if (fuel >= 10) {

            if ( air >= 10) {
              burned_fuel = _min(fuel/10, air/10);
              cell->fuel -= burned_fuel/CELL_DIVISOR;
              cell->air -= burned_fuel/CELL_DIVISOR;
              cell->smoke += (burned_fuel/CELL_DIVISOR)*2;
              // if (heat_acceleration == 1) {
              //   vy = vy + (fuel*fuel);
              // }
              //draw flame
              if (!(x >= FIRE_GRID_BORDER && x < FIRE_GRID_WIDTH-FIRE_GRID_BORDER && y >= FIRE_GRID_BORDER && y < FIRE_GRID_HEIGHT-FIRE_GRID_BORDER)) continue;

              if(draw_flame == 1) {
                //draw flame (burning fuel and air)
                r += _min(burned_fuel<<1,255);
                g += _min(burned_fuel>>2,255);
                b += _min(burned_fuel>>3,255);
              }

              
            }

            if (!(x >= FIRE_GRID_BORDER && x < FIRE_GRID_WIDTH-FIRE_GRID_BORDER && y >= FIRE_GRID_BORDER && y < FIRE_GRID_HEIGHT-FIRE_GRID_BORDER)) continue;

            if(draw_fuel == 1) {
                uint16_t fuel2 = fuel*5;
                r += _min(fuel2>>5,255);
                g += _min(fuel2>>8,255);
                b += _min(fuel2>>9,255);     
            }

          }

          if (!(x >= FIRE_GRID_BORDER && x < FIRE_GRID_WIDTH-FIRE_GRID_BORDER && y >= FIRE_GRID_BORDER && y < FIRE_GRID_HEIGHT-FIRE_GRID_BORDER)) continue;

         
          if (draw_smoke && smoke >> 9) {
            b += _min(smoke>>9,255);
            r += _min(smoke>>11,255);
            g += _min(smoke>>11,255);
          }

          if (draw_air) {
            g = _min(air>>5,255);
          }

          if(r | g | b) {
            r = _min(r,255);
            g = _min(g,255);
            b = _min(b,255);
            leds[XY(x-FIRE_GRID_BORDER, y-FIRE_GRID_BORDER)] = CRGB(gamma8_encode(r), gamma8_encode(g), gamma8_encode(b));
          }

          
        }
      }

      //////////////////////////////////////
      //flamethrower effect
      if (true || current_variation == 2 /*&& grid_cnt == grid_cnt*/) {
        

        static FLAMETHROWER_CYCLE * current_flame = get_current_flame();
        if ((button2_down || flame_button) && !flame_on) {
          flame_on = true;
          current_flame->fuel_on = true;
          current_flame->active = true;
        }

        if (!(button2_down || flame_button) && flame_on) {
          flame_on = false;
          current_flame->fuel_on = false;
          current_flame = get_current_flame();
        }
        
        if (flame_on)
        {
          grid[10][FIRE_GRID_WIDTH/2].vy = (INT16_MAX*3)/4;
          //grid[10][FIRE_GRID_WIDTH/2].vx = 0;
          int asdf = grid[10][FIRE_GRID_WIDTH/2].air+grid[10][FIRE_GRID_WIDTH/2].smoke;
          //grid[10][FIRE_GRID_WIDTH/2].fuel = random(INT16_MAX/2);
          grid[10][FIRE_GRID_WIDTH/2].fuel += asdf;
          grid[10][FIRE_GRID_WIDTH/2].air = 0;
          grid[10][FIRE_GRID_WIDTH/2].smoke = 0;
          //grid[10][FIRE_GRID_WIDTH/2].smoke = 30000;
          //grid[9][FIRE_GRID_WIDTH/2].air = 0;
          //grid[9][FIRE_GRID_WIDTH/2].fuel = 0;
          //fgrid[9][FIRE_GRID_WIDTH/2].smoke = 0;
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
          grid[i-14][x].vx += ((i)*random(50)-(i)*25);
          int r = random((i+1)*200, (i+1)*400);
          r = (r*r)/((i+1)*400);
          grid[i-14][x].fuel += (r*2);
          //grid[x][i-5].air += random(0,i*75);
        }
      }
    }




    

    inline void calculate_velocity(pixel_full& source_cell) {
      
      //transfer particles between cells based on velocity (10000 max velocity, all particles move)
      fire_calc_t total_moved_fuel = 0;
      fire_calc_t total_moved_air = 0;
      fire_calc_t total_moved_smoke = 0;
      uint32_t v_total = abs(source_cell.vx)+abs(source_cell.vy);
      
      if (v_total != 0) {
        cell_info ci;
        uint32_t v_total_constrain = _min(v_total, 10000);
        
        if (source_cell.vx != 0) {
          int32_t x_portion = (abs(source_cell.vx)*(float)v_total_constrain) / v_total;
          ci.moved_fuel = (source_cell.fuel*x_portion)/20000;
          ci.moved_air = (source_cell.air*x_portion)/20000;
          ci.moved_smoke = (source_cell.smoke*x_portion)/20000;
          ci.moved_total = ci.moved_fuel + ci.moved_air + ci.moved_smoke;
          if (ci.moved_total > 0) {
            if (source_cell.vx > 0) {   
              if (right_cell) {
                velocity_transfer(source_cell, *right_cell, ci);
              }
            } else {
              if (left_cell) {
                velocity_transfer(source_cell, *left_cell, ci);
              }
            }
            total_moved_smoke += ci.moved_smoke;
            total_moved_fuel += ci.moved_fuel;
            total_moved_air += ci.moved_air;
          }
        }

        if (source_cell.vy != 0) {
          int32_t y_portion = (abs(source_cell.vy)*(float)v_total_constrain) / v_total;
          ci.moved_fuel = (source_cell.fuel*y_portion)/20000;
          ci.moved_air = (source_cell.air*y_portion)/20000;
          ci.moved_smoke = (source_cell.smoke*y_portion)/20000;
          ci.moved_total = ci.moved_fuel + ci.moved_air + ci.moved_smoke;
          if (ci.moved_total > 0) {
            if (source_cell.vy > 0) {
              if (top_cell) {
                velocity_transfer(source_cell, *top_cell, ci);
              }
            } else {
              if (bottom_cell) {
                velocity_transfer(source_cell, *bottom_cell, ci);
              }
            }
            total_moved_smoke += ci.moved_smoke;
            total_moved_fuel += ci.moved_fuel;
            total_moved_air += ci.moved_air;
          }
        }

        source_cell.smoke -= total_moved_smoke; //looked kind of neat without this line
        source_cell.fuel -= total_moved_fuel;
        source_cell.air -= total_moved_air;
        
      }
    }

    inline void calculate_density(pixel_full& source_cell) {
      
      cell_info ci;

      //total number of particles in cell
      fire_calc_t total_particles = source_cell.fuel + source_cell.air + source_cell.smoke;

      //total number of particles in neighboring cells
      fire_calc_t neighboring_pressure = 0;
      uint8_t neighbor_count = 4;
        if (right_cell) {
            neighboring_pressure += right_cell->fuel;
            neighboring_pressure += right_cell->air;
            neighboring_pressure += right_cell->smoke;
        } else {
            neighboring_pressure += 10000;
        }
        
        if (top_cell) {
            neighboring_pressure += top_cell->fuel;
            neighboring_pressure += top_cell->air;
            neighboring_pressure += top_cell->smoke;
        } else {
            neighboring_pressure += 10000;
        }

        if (bottom_cell) {
            neighboring_pressure += bottom_cell->fuel;
            neighboring_pressure += bottom_cell->air;
            neighboring_pressure += bottom_cell->smoke;
        } else {
            neighboring_pressure += 10000;
        }

        if (left_cell) {
            neighboring_pressure += left_cell->fuel;
            neighboring_pressure += left_cell->air;
            neighboring_pressure += left_cell->smoke;
        } else {
            neighboring_pressure += 10000;
        }

        neighboring_pressure/=neighbor_count;
      //particle imbalance (number of particles above/below our base threshold/pressure)
      ci.pressure = (total_particles - neighboring_pressure);


      if (ci.pressure > 0) {
        float pt = ci.pressure/(float)total_particles;
        fire_calc_t fuel_pressure = source_cell.fuel * pt;
        fire_calc_t air_pressure = source_cell.air * pt;
        fire_calc_t smoke_pressure = source_cell.smoke * pt;

        //number of particles to push to neighboring cells
        ci.moved_fuel = fuel_pressure / 5;
        ci.moved_air = air_pressure / 5;
        ci.moved_smoke = smoke_pressure / 5;
        ci.moved_total = ci.moved_fuel + ci.moved_air + ci.moved_smoke;

        source_cell.fuel -= ci.moved_fuel*5;
        source_cell.air -= ci.moved_air*5;
        source_cell.smoke -= ci.moved_smoke*5;

        source_cell.fuel += ci.moved_fuel;
        source_cell.air += ci.moved_air;
        source_cell.smoke += ci.moved_smoke;
        

          //right cell
          if (right_cell) {
            ci.x_dir = 1;
            ci.y_dir = 0;
            velocity_transfer(source_cell, *right_cell, ci);
          } else {
            source_cell.fuel += ci.moved_fuel;
            source_cell.air += ci.moved_air;
            source_cell.smoke += ci.moved_smoke;
          }

          //left cell
          if (left_cell) {
            ci.x_dir = -1;
            ci.y_dir = 0;
            velocity_transfer(source_cell, *left_cell, ci);
          } else {
            source_cell.fuel += ci.moved_fuel;
            source_cell.air += ci.moved_air;
            source_cell.smoke += ci.moved_smoke;
          }

          //top cell
          if (top_cell) {
            ci.x_dir = 0;
            ci.y_dir = 1;
            velocity_transfer(source_cell, *top_cell, ci);
          }

          //bottom cell
          if (bottom_cell) {
            ci.x_dir = 0;
            ci.y_dir = -1;
            velocity_transfer(source_cell, *bottom_cell, ci);
          } else {
            source_cell.fuel += ci.moved_fuel;
            source_cell.air += ci.moved_air;
            source_cell.smoke += ci.moved_smoke;
          }
        
      }
      if (ci.pressure < 0 && (ci.pressure+total_particles)){
          ci.pressure = -ci.pressure;
          if (!top_cell) {
            source_cell.vy -= (ci.pressure*ci.pressure)/(ci.pressure+total_particles);
            //source_cell.vy -= ci.pressure;
            source_cell.air += ci.pressure;
            return;
          }
          if (!bottom_cell) {
            source_cell.vy += (ci.pressure*ci.pressure)/(ci.pressure+total_particles);
            //source_cell.vy += ci.pressure;
            source_cell.air += ci.pressure;
            return;
          }
          if (!left_cell) {
            source_cell.vx += (ci.pressure*ci.pressure)/(ci.pressure+total_particles);
            //source_cell.vx += ci.pressure;
            source_cell.air += ci.pressure;
            return;
          }
          if (!right_cell) {
            source_cell.vx -= (ci.pressure*ci.pressure)/(ci.pressure+total_particles);
            //source_cell.vx -= ci.pressure;
            source_cell.air += ci.pressure;
            return;
          }
      }
      
    }




};



LIGHT_SKETCHES::REGISTER<FIRE2> fire2("fire2");

#endif