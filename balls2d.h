#ifndef LIGHTS_BALLS_2D_H
#define LIGHTS_BALLS_2D_H
//#define DEBUG_CHECK_ORDER

//BALLS 2D


class BALLS2D: public LIGHT_SKETCH {
  public:
    BALLS2D () {setup();}
    ~BALLS2D () {}

    static int default_gravity;
    static int gravity;
    static float gravity_x;
    static float gravity_y;
    #define BALLS2D_RADIUS 1024

  private:
    int _dampen_ball = 95;
    int _dampen_wall = 95;
    int _friction_wall = 95;
    float dampen_ball;
    float dampen_wall;
    float friction_wall;
    int energy_transfer = 100;
    Z_BUF _z_buffer;
    uint8_t current_variation = 0;
    int pulse[5] = {255, 255, 255, 255, 255};
    int current_pulse = 0;

    #define BALLS2D_GRID 120
    #define BALLS2D_GRID_MAX 10
    static uint frame_divisor;
    static int16_t collision_grid[BALLS2D_GRID][BALLS2D_GRID][BALLS2D_GRID_MAX];
    static uint16_t collision_grid_cnt[BALLS2D_GRID][BALLS2D_GRID];

#define NUM_BALLS2D 500

  private:
    int16_t first_collision = -1;
    int16_t last_collision = -1;

    class BALL {

      public:
        float x;
        float y;
        float vx = 5;
        float vy = 0;
        float distx;
        float disty;
        uint8_t cnt = 0;
        float radius = BALLS2D_RADIUS;

        int j; // ball 2
        float dt; //time of collision 0-1; 1 being earliest; 0 being latest
        float d; //distance between balls (square root)
        char flag; //type of collision 'B' = ball, 't' = top wall, 'l' = left wall, 'b' = bottom wall, 'r' = right wall
        bool unresolved = false; //does this collision need to be resolved?

        uint8_t grid_x;
        uint8_t grid_y;
        int16_t prev_collision = -1;
        int16_t next_collision = -1;

        void grid_position() {
          grid_x = (x*BALLS2D_GRID)/(MATRIX_WIDTH*256);
          grid_y = (y*BALLS2D_GRID)/(MATRIX_HEIGHT*256);
          grid_x = _min(_max(grid_x, 0), BALLS2D_GRID-1);
          grid_y = _min(_max(grid_y, 0), BALLS2D_GRID-1);
        }

        void add_to_grid(int ball) {
          int pos = collision_grid_cnt[grid_x][grid_y]++;
          collision_grid[grid_x][grid_y][pos] = ball;
        }

        void grid_update(int ball) {
          uint8_t old_grid_x = grid_x;
          uint8_t old_grid_y = grid_y;
          grid_position();
          //remove ball from old grid position
          //add to new position
          if (old_grid_x != grid_x || old_grid_y != grid_y) {
            bool move = false;
            for (int i = 0; i < BALLS2D_GRID_MAX; i++) {
              if (move) {
                collision_grid[old_grid_x][old_grid_y][i-1] = collision_grid[old_grid_x][old_grid_y][i];
              } else if (collision_grid[old_grid_x][old_grid_y][i] == ball) {
                move = true;
              }
            }
            collision_grid_cnt[old_grid_x][old_grid_y]--;
            add_to_grid(ball);
          }
        }

// 8/1 = 8
// 4/1.5 + 8/1.5 = 8
// 2/2   + 4/2   + 6/2 = 6
// 2/2.5 + 4/2.5 + 6/2.5 + 8/2.5 = 8
// 2/3   + 4/3   + 6/3   + 8/3   + 10/3 = 10
// 2/3.5 + 4/3.5 + 6/3.5 + 8/3.5 + 10/3.5 + 12/3.5 = 12
// 4 + 12 = 16    (4+6)/2 = 5  (10+12)/2 = 16 
// 2 + 8 = 10  (2+2)/4 = 1 (4+4)/4 = 2 (8+6)/4 = 3.5 (14+8)/4 = 5.5
// (2+2)/4.8 + (4+4)/4.8 + (8+6)/4.8 + (14+8)/4.8 = 10
// (2+3.2)/4 + (5.2+3.2)/4 + (8.4+3.2)/4 + (11.6+3.2)/4 = 10
// (2+2)/7 + (4+2)/7 + (6+2)/7 + (8+2)/7 + (10+2)/7 + (12+2)/7 + (14+2)/7 = 10
// 1 + 2 + 3.5 + 5.5 = 12

        void gravity() {
          vx -= gravity_x;
          vy -= gravity_y;
        }

        void update(int ball) {
          cnt = 0;
          distx = vx/frame_divisor;
          disty = vy/frame_divisor;
          x += distx;
          y += disty;
          grid_update(ball);
        }

    };

    BALL balls[NUM_BALLS2D];

    #ifdef DEBUG_CHECK_ORDER
    int16_t debug_order[NUM_BALLS2D];
    void check_order() {

          {
            int i = first_collision;
            int p = 0;
            int prev = -1;
            while (i != -1) {
              if (balls[i].prev_collision != prev) {

              }
              prev = i;
              debug_order[p] = debug_order[p];
              i = balls[i].next_collision; 
              p++;
            }
          }

          for (int i = 0; i < NUM_BALLS2D; i++) {
              debug_order[i] = -2;
          }

          {
            int i = first_collision;
            int p = 0;
            while (i != -1) {
              debug_order[p] = i;
              i = balls[i].next_collision; 
              p++;
            }
            debug_order[p] = -1;
          }
    }
    #endif
    void sort_collisions(int i) {
    #ifdef DEBUG_CHECK_ORDER
    check_order();
      //remove the ball from the list if necessary
      int debug_prev_collision = balls[i].prev_collision;
      int debug_next_collision = balls[i].next_collision;
    #endif

      if (balls[i].prev_collision != -1) {
        balls[balls[i].prev_collision].next_collision = balls[i].next_collision;
        if (balls[i].next_collision == -1) {
          last_collision = balls[i].prev_collision;
        }
      }
      if (balls[i].next_collision != -1) {
        balls[balls[i].next_collision].prev_collision = balls[i].prev_collision;
        if (balls[i].prev_collision == -1) {
          first_collision = balls[i].next_collision;
        }
      }

    #ifdef DEBUG_CHECK_ORDER
    check_order();
    #endif

      int p = first_collision;
      int pp = -1;
      if(p == i) return;
      if (p == -1) {
        //this is the first entry
    #ifdef DEBUG_CHECK_ORDER
    check_order();
    #endif
        balls[i].prev_collision = -1;
        balls[i].next_collision = -1;
        first_collision = i;
        last_collision = i;
        return;
    #ifdef DEBUG_CHECK_ORDER
    check_order();
    #endif
      } else {
        //compare collision times
        while ( balls[p].dt > balls[i].dt ) {

          if (balls[p].next_collision == -1) {
            //insert at the end of the list
            balls[p].next_collision = i;
            balls[i].prev_collision = p;
            balls[i].next_collision = -1;
            last_collision = i;

    #ifdef DEBUG_CHECK_ORDER
    check_order();
    #endif
            return;
          }
          //move down the list
          pp = p;
          p = balls[p].next_collision;
        }

    #ifdef DEBUG_CHECK_ORDER
    check_order();
    #endif
        
        //insert into the list
        balls[i].next_collision = p;
        balls[i].prev_collision = pp;
        balls[p].prev_collision = i;
        if (pp != -1) {
          balls[pp].next_collision = i;
        } else {
          first_collision = i;
        }
        

    #ifdef DEBUG_CHECK_ORDER
    check_order();
    #endif
      }

    }

    void add_to_buffer(const int& ii, const int& ij, const float& idt, const char& iflag, const float& id = 0) {
      if (idt > balls[ii].dt) {
        if (iflag != 'B') {
          balls[ii].j = ij;
          balls[ii].dt = idt;
          balls[ii].flag = iflag;
          balls[ii].d = id;
          balls[ii].unresolved = true;
          sort_collisions(ii);

        } else if (idt > balls[ij].dt) {
          balls[ii].j = ij;
          balls[ii].dt = idt;
          balls[ii].flag = iflag;
          balls[ii].d = id;
          balls[ii].unresolved = true;
          sort_collisions(ii);

          balls[ij].j = ii;
          balls[ij].dt = idt;
          balls[ij].flag = iflag;
          balls[ij].d = id;
          balls[ij].unresolved = true;
        }
      }

    }

    //find the next unresolved collision and void any collisions involving either of the two balls
    int next_collision() {
      int i = first_collision;

      //look for the first unresolved collision
      while (i != -1 && !balls[i].unresolved) {
        balls[i].prev_collision = -1;
        balls[i].next_collision = -1;
        i = balls[i].next_collision;
      }

      if (i == -1) {
        return i;
      }

      //make sure the other ball contains the same collision information
      if (balls[i].flag == 'B' && !(balls[balls[i].j].flag == 'B' && balls[balls[i].j].j == i)) {
        balls[i].flag = 'x';
      }


    #ifdef DEBUG_CHECK_ORDER
    check_order();
    #endif
      //mark the ball(s) as resolved
      balls[i].unresolved = false;
      first_collision = balls[i].next_collision;
      if(first_collision != -1) {
        balls[first_collision].prev_collision = -1;
      }
      balls[i].next_collision = -1;
      balls[i].prev_collision = -1;
      if (balls[i].flag == 'B') {
        balls[balls[i].j].unresolved = false;
      }
    #ifdef DEBUG_CHECK_ORDER
    check_order();
    #endif
      return i;
    }

    //check for collisions with walls
    void check_wall_collisions(int i) {
      //check for collisions with walls

      //top
      if (balls[i].grid_y == BALLS2D_GRID-1 && balls[i].y + balls[i].radius > (MATRIX_HEIGHT - 1) * 256.f) {
        float dt = ( balls[i].y + balls[i].radius - (MATRIX_HEIGHT-1) * 256.f ) / balls[i].disty;
        if (dt != 0) {
          add_to_buffer(i, 0, dt, 't');
        }
      }
      //bottom
      if (balls[i].grid_y == 0 && balls[i].y - balls[i].radius < 0) {
        float dt = ( balls[i].y - balls[i].radius) / balls[i].disty;
        if (dt != 0) {
          add_to_buffer(i, 0, dt, 'b');
        }
      }
      //left
      if (balls[i].grid_x == 0 &&  balls[i].x - balls[i].radius < 0) {
        float dt = ( balls[i].x - balls[i].radius) / (balls[i].distx);
        if (dt != 0) {
          add_to_buffer(i, 0, dt, 'l');
        }
      }
      //right
      if (balls[i].grid_x == BALLS2D_GRID-1 && balls[i].x + balls[i].radius > (MATRIX_WIDTH-1) * 256.f) {
        float dt = ( balls[i].x + balls[i].radius - ((MATRIX_WIDTH-1) * 256.f) ) / (balls[i].distx);
        if (dt != 0) {
          add_to_buffer(i, 0, dt, 'r');
        }
      }

    }

    bool check_ball_collisions_bool(int i) {
      int grid_x_min = _max(balls[i].grid_x-1, 0);
      int grid_x_max = _min(balls[i].grid_x+1, BALLS2D_GRID-1);
      int grid_y_min = _max(balls[i].grid_y-1, 0);
      int grid_y_max = _min(balls[i].grid_y+1, BALLS2D_GRID-1);

      for (int x = grid_x_min; x <= grid_x_max; x++) {
        for (int y = grid_y_min; y <= grid_y_max; y++) {
          uint cnt = collision_grid_cnt[x][y];
          for (int p = 0; p < cnt; p++) {
            int j = collision_grid[x][y][p];
            if (j < i) {
              if ( check_ball_collision_bool(i, j) ) {
                return true;
              }
            }
          }
        }
      }
      return false;
    }

    void check_ball_collisions(int i) {
      int grid_x_min = _max(balls[i].grid_x-1, 0);
      int grid_x_max = _min(balls[i].grid_x+1, BALLS2D_GRID-1);
      int grid_y_min = _max(balls[i].grid_y-1, 0);
      int grid_y_max = _min(balls[i].grid_y+1, BALLS2D_GRID-1);

      for (int x = grid_x_min; x <= grid_x_max; x++) {
        for (int y = grid_y_min; y <= grid_y_max; y++) {
          uint cnt = collision_grid_cnt[x][y];
          for (int p = 0; p < cnt; p++) {
            int j = collision_grid[x][y][p];
            if (j != i) {
              check_ball_collision(i, j);
            }
          }
        }
      }
    }

    //check for collisions with balls
    void check_ball_collision(const int& i, const int& j) {
      if ( abs(balls[i].x - balls[j].x) > balls[i].radius+balls[j].radius || abs(balls[i].y - balls[j].y) > balls[i].radius+balls[j].radius  )
        return;

      float dx = balls[j].x - balls[i].x;
      float dy = balls[j].y - balls[i].y;
      float dd = (dx * dx) + (dy * dy);

      //proceed with collision if detected
      if (dd < ( (balls[i].radius + balls[j].radius) * (balls[i].radius + balls[j].radius)) ) {
        //do some maths to calculate the collision
        float d = sqrt(dd);

        //figure out how far the balls have overlapped
        float vp1 = (balls[i].distx) * dx / d + (balls[i].disty) * dy / d;
        float vp2 = (balls[j].distx) * dx / d + (balls[j].disty) * dy / d;
        if (vp1 != vp2) {
          float dt = (balls[i].radius + balls[j].radius - d) / (vp1 - vp2);
          add_to_buffer(i, j, dt, 'B', d);
          // if (current_variation == 1 && dt < 0) {
          //   uint8_t hue = random(256);
          //   CRGB rgb = CHSV(hue, 255, 255);
          //   CRGB rgb2 = CHSV(hue, 128, 255);
          //   draw_circle_fine(balls[i].x, balls[i].y, balls[i].radius, rgb, -1, 32);
          //   draw_line_fine(led_screen, balls[i].x, balls[i].y, balls[i].x-balls[i].distx, balls[i].y-balls[i].disty, rgb);
          //   draw_circle_fine(balls[j].x, balls[j].y, balls[j].radius, rgb2, -1, 32);
          //   draw_line_fine(led_screen, balls[j].x, balls[j].y, balls[j].x-balls[j].distx, balls[j].y-balls[j].disty, rgb);
          // }
        }
      }

      //}
    }

    bool check_ball_collision_bool(const int& i, const int& j) {
      if ( abs(balls[i].x - balls[j].x) > balls[i].radius+balls[j].radius || abs(balls[i].y - balls[j].y) > balls[i].radius+balls[j].radius  )
        return false;

      float dx = balls[j].x - balls[i].x;
      float dy = balls[j].y - balls[i].y;
      float dd = (dx * dx) + (dy * dy);

      //proceed with collision if detected
      if (dd < ( (balls[i].radius + balls[j].radius) * (balls[i].radius + balls[j].radius)) ) {
        return true;
      }
      return false;
    }

    void create_ball_column() {

      int current_x = BALLS2D_RADIUS;
      int current_y = BALLS2D_RADIUS;
      int x_offset = random(MATRIX_WIDTH*256);
      for ( int i = 0; i < NUM_BALLS2D; i++) {
        balls[i].radius = random(BALLS2D_RADIUS*7/8,(BALLS2D_RADIUS*9)/8);
        balls[i].x = current_x;
        balls[i].x += x_offset;
        balls[i].x = ((uint)balls[i].x)%(MATRIX_WIDTH*256);

        balls[i].y = current_y;
        current_y += random(BALLS2D_RADIUS*2,BALLS2D_RADIUS*3);
        if (current_y > MATRIX_HEIGHT*256 - BALLS2D_RADIUS) {
          current_y = BALLS2D_RADIUS;
          current_x += random(BALLS2D_RADIUS*2,BALLS2D_RADIUS*3);
          current_x %= MATRIX_WIDTH*256;
        }
        balls[i].vx = random(1, 200) - 100;
        balls[i].vy = random(1, 200) - 100;
        balls[i].grid_position();
        balls[i].add_to_grid(i);

      }
    }

    void create_ball_ball() {
      int offset = 0;
      int angle = 0;
      for ( int i = 0; i < NUM_BALLS2D; i++ ) {

        while ( true ) {
          int x = ((cos8(angle)-(int)128)*offset)/128;
          int y = ((sin8(angle)-(int)128)*offset)/128;
          balls[i].x = x + MATRIX_WIDTH*128;
          balls[i].y = y + MATRIX_HEIGHT*128;
          balls[i].grid_position();
          balls[i].add_to_grid(i);

          if ( !check_ball_collisions_bool(i) ) {
            break;
          }

          angle++;
          if (angle > 255) {
            offset+=BALLS2D_RADIUS*3;
            angle = 0;
          }

        }


        balls[i].vx = 15*256;
        balls[i].vy = 5*256;

      }
    }



    unsigned long update_time = 0;

  public:
    void next_effect() {
      current_variation++;
      current_variation %= 4;
    }



    void setup() {
      control_variables.add(default_gravity, "Gravity:", -200, 200);
      control_variables.add(_dampen_ball, "Ball dampening:", 0, 100);
      control_variables.add(_dampen_wall, "Wall dampening:", 0, 100);
      control_variables.add(_friction_wall, "Wall friction:", 0, 100);
      control_variables.add(energy_transfer, "Energy transfer:", 0, 100);
      reset();
      
    }

    void reset() {
      led_screen.rotation_alpha = 0;
      led_screen.rotation_beta = 90;
      led_screen.rotation_gamma = 0;
      // for (int i = 0; i < NUM_BALLS2D; i++) {
      //   if (balls[i].y < 8000) {
      //     balls[i].vy += 2000;
      //   }
      // }

      //reset the grid
      for (int x = 0; x < BALLS2D_GRID; x++) {
        for (int y = 0; y < BALLS2D_GRID; y++) {
          collision_grid_cnt[x][y] = 0;
        }
      }

      //invalidate any existing collisions (in case reset() is being called asynchronously)
      for ( int i = 0; i < NUM_BALLS2D; i++) {
        balls[i].dt = 0.5f;
        balls[i].flag = 'x';
      }

      //create_ball_column();
      create_ball_ball();

    }


    void loop() {
      dampen_ball = (1.f*_dampen_ball)/100.f;
      dampen_wall = (1.f*_dampen_wall)/100.f;
      friction_wall = (1.f*_friction_wall)/100.f;



      gravity = default_gravity;
      
      switch (current_variation) {
        case 0:
          gravity_y = gravity;
          gravity_x = 0;
          break;
        case 1:
          gravity_y = 0;
          gravity_x = gravity;
          break;
        case 2:
          gravity_y = -gravity;
          gravity_x = 0;
          break;
        case 3:
          gravity_y = 0;
          gravity_x = -gravity;
          break;
        default:
          gravity_y = gravity;
          gravity_x = 0;
      }

      //apply gravity acceleration
      for (int i = 0; i < NUM_BALLS2D; i++) {
        balls[i].gravity();
      }

      uint calc_cnt = 0;
      frame_divisor = 32;
      uint32_t total_cnt = 0;
      uint32_t microseconds = micros();
      uint32_t collisions = 0;

      while (calc_cnt < frame_divisor) {
        calc_cnt++;

        //reset balls and update positions
        first_collision = -1;
        last_collision = -1;

        for (int i = 0; i < NUM_BALLS2D; i++) {
          balls[i].prev_collision = -1;
          balls[i].next_collision = -1;
          balls[i].dt = 0;
          balls[i].update(i);
        }

        //calculate collisions
#ifdef DEBUG
        static int max_k = 0;
#endif
          //int checks = 20;
        while (true) {

          //make sure there are no lingering collisions
          int k = next_collision();
          //reset all balls
          if (k == -1) {

            first_collision = -1;
            last_collision = -1;
            for (int i = 0; i < NUM_BALLS2D; i++) {
              balls[i].unresolved = false;
              balls[i].dt = 0;
              balls[i].prev_collision = -1;
              balls[i].next_collision = -1;
            }

            
            for (int i = 0; i < NUM_BALLS2D; i++) {
              //check for collisions with walls
              check_wall_collisions(i);
              check_ball_collisions(i);
            }
            k = next_collision();
          }

#ifdef DEBUG
          if (k > max_k) {
            max_k = k;
          }
#endif


          //if no collision: break out of while loop
          if (k == -1) {
            break;
          }
          collisions++;

          
          

          total_cnt++;
          //if there was a collision: calculate it and check for more
          int i = k;
          int j = balls[k].j;
          char flag = balls[k].flag;



          if (flag == 'B') {
            //process the earliest ball collision
            {
              BALL debug_ball_i_before = balls[i];
              BALL debug_ball_j_before = balls[j];

              float dx = balls[j].x - balls[i].x;
              float dy = balls[j].y - balls[i].y;
              // float dd = (dx * dx) + (dy * dy);
              // float d = sqrt(dd);
              float d = balls[i].d;
              //figure out how far the balls have overlapped
  
              float ax = dx / d;
              float ay = dy / d;
              //  float vp1 = (balls[i].distx) * ax + (balls[i].disty) * ay;
              //  float vp2 = (balls[j].distx) * ax + (balls[j].disty) * ay;
              // float dt = (balls[i].radius + balls[j].radius - d) / (vp1 - vp2);
              float dt = balls[i].dt;
              //back the two balls up so there is no overlap


              balls[i].x -= balls[i].distx * dt;
              balls[i].y -= balls[i].disty * dt;
              balls[j].x -= balls[j].distx * dt;
              balls[j].y -= balls[j].disty * dt;


              float va1 = (balls[i].vx * ax + balls[i].vy * ay);
              float vb1 = (-balls[i].vx * ay + balls[i].vy * ax);
              float va2 = (balls[j].vx * ax + balls[j].vy * ay);
              float vb2 = (-balls[j].vx * ay + balls[j].vy * ax);
              float vaP1 = va1 + (1.0f + dampen_ball) * (va2 - va1) / 2;
              float vaP2 = va2 + (1.0f + dampen_ball) * (va1 - va2) / 2;


              balls[i].vx = ((vaP1 * ax - vb1 * ay)*energy_transfer + balls[i].vx*(100-energy_transfer))/100;
              balls[i].vy = ((vaP1 * ay + vb1 * ax)*energy_transfer + balls[i].vy*(100-energy_transfer))/100;
              balls[j].vx = ((vaP2 * ax - vb2 * ay)*energy_transfer + balls[j].vx*(100-energy_transfer))/100;
              balls[j].vy = ((vaP2 * ay + vb2 * ax)*energy_transfer + balls[j].vy*(100-energy_transfer))/100;



              balls[i].distx = balls[i].vx/frame_divisor;
              balls[i].disty = balls[i].vy/frame_divisor;
              balls[j].distx = balls[j].vx/frame_divisor;
              balls[j].disty = balls[j].vy/frame_divisor;


              //readjust the previous overlap using the post-collision vectors
              balls[i].x += balls[i].distx * dt;
              balls[i].y += balls[i].disty * dt;
              balls[j].x += balls[j].distx * dt;
              balls[j].y += balls[j].disty * dt;

              BALL debug_ball_i_after = balls[i];
              BALL debug_ball_j_after = balls[j];

              if (abs(debug_ball_i_before.y - debug_ball_i_after.y) > (MATRIX_HEIGHT*256)/4) {
                int debug = 1;
              }


            }
          } else if (flag == 't') {

            float dt = ( balls[i].y + balls[i].radius - ((MATRIX_HEIGHT-1) * 256.f) ) / (balls[i].disty);
            balls[i].y -= fabsf(balls[i].disty * dt);
            balls[i].vy = -fabsf(balls[i].vy);
            balls[i].vy *= dampen_wall;
            balls[i].vx *= friction_wall;
            balls[i].distx *= friction_wall;
            balls[i].disty = balls[i].vy/frame_divisor;
            balls[i].y += -fabsf(balls[i].disty * dt);


          } else if (flag == 'b') {

            float dt = ( balls[i].y - balls[i].radius) / (balls[i].disty);
            balls[i].y += fabsf(balls[i].disty * dt);
            balls[i].vy = fabsf(balls[i].vy);
            balls[i].vy *= dampen_wall;
            balls[i].vx *= friction_wall;
            balls[i].distx *= friction_wall;
            balls[i].disty = balls[i].vy/frame_divisor;
            balls[i].y += fabsf(balls[i].disty * dt);


          } else if (flag == 'l') {

            float dt = ( balls[i].x - balls[i].radius) / (balls[i].distx);
            balls[i].x += fabsf(balls[i].distx * dt);
            balls[i].vx = fabsf(balls[i].vx);
            balls[i].vx *= dampen_wall;
            balls[i].vy *= friction_wall;
            balls[i].disty *= friction_wall;
            balls[i].distx = balls[i].vx/frame_divisor;
            balls[i].x += fabsf(balls[i].distx * dt);


          } else if (flag == 'r') {
            BALL debug_ball_before = balls[i];

            float dt = ( balls[i].x + balls[i].radius - (MATRIX_WIDTH-1) * 256.f ) / (balls[i].distx);
            balls[i].x -= fabsf(balls[i].distx * dt);
            balls[i].vx = -fabsf(balls[i].vx);
            balls[i].vx *= dampen_wall;
            balls[i].vy *= friction_wall;
            balls[i].disty *= friction_wall;
            balls[i].distx = balls[i].vx/frame_divisor;
            balls[i].x += -fabsf(balls[i].distx * dt);
            
            BALL debug_ball_after = balls[i];
            if (abs(debug_ball_before.y - debug_ball_after.y) > (MATRIX_HEIGHT*256)/4) {
              int debug = 1;
            }
          }

          //recheck collisions for the recalculated balls
          balls[i].dt = 0;
          balls[i].unresolved = false;
          balls[i].grid_update(i);
          if (flag == 'B') {
            balls[j].dt = 0;
            balls[j].unresolved = false;
            balls[j].grid_update(j);

            check_ball_collisions(i);
            check_ball_collisions(j);
            check_wall_collisions(i);
            check_wall_collisions(j);

          } else {

            check_ball_collisions(i);
            check_wall_collisions(i);
          }

        }
        
      }
      
      //std::cout << "collision count: " << total_cnt << "\n";
      microseconds = micros() - microseconds;
      //std::cout << "microseconds: " << microseconds << " collisions: " << collisions << " tpc: " << microseconds/_max(collisions,1) << "\n";

      //write balls to LED buffer
      CRGB rgb = CHSV(96, 200, 255);
      for (int i = 0; i < NUM_BALLS2D; i++) {
        //blendXY(led_screen, balls[i].x, balls[i].y, 0, 0, 255);
        draw_circle_fine(balls[i].x, balls[i].y, balls[i].radius, rgb, -1, 32);
         fill_shape(0, rgb);
         y_buffer->reset();
         reset_x_buffer();
      }

      //update LEDS
      LED_show();
      LED_black();

    }

};

int BALLS2D::gravity = 0;
int BALLS2D::default_gravity = 80;
float BALLS2D::gravity_x = 0;
float BALLS2D::gravity_y = 0;
uint BALLS2D::frame_divisor = 1;

int16_t BALLS2D::collision_grid[BALLS2D_GRID][BALLS2D_GRID][BALLS2D_GRID_MAX] = {-1};
uint16_t BALLS2D::collision_grid_cnt[BALLS2D_GRID][BALLS2D_GRID] = {0};


LIGHT_SKETCHES::REGISTER<BALLS2D> balls2d("balls2d");

//END BALLS 2D

#endif