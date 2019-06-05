
//BALLS 2D


class BALLS2D: public LIGHT_SKETCH {
  public:
    BALLS2D () {setup();}
    ~BALLS2D () {}

    static int gravity;
    static float gravity_x;
    static float gravity_y;
    static float frame_delta;
    static unsigned long debug_time2;

  private:
    uint8_t current_variation = 0;
    int pulse[5] = {255, 255, 255, 255, 255};
    int current_pulse = 0;



#define NUM_BALLS 20

  private:
    class BALL {

      public:
        uint8_t h;
        int x_led;
        int y_led;
        float x;
        float y;
        float vx = 5;
        float vy = 0;
        float distx;
        float disty;
        float r = 256;
        float m = 1;
        bool sleeping = false;
        uint8_t cnt = 0;
        int dist[5] = {0, 0, 0, 0, 0};

        void update() {


          cnt = 0;
          vx -= gravity_x;
          vy -= gravity_y;
          distx = vx * frame_delta;
          disty = vy * frame_delta;
          x += distx;
          y += disty;
          x_led = x / 256;
          y_led = y / 256;
        }

    };

    BALL balls[NUM_BALLS];

    //create a buffer to store unresolved collisions
    class COLLISION {
      public:
        int i; //ball 1
        int j; // ball 2
        float dt; //time of collision 0-1; 1 being earliest; 0 being latest
        float d; //distance between balls (square root)
        char flag; //type of collision 'B' = ball, 't' = top wall, 'l' = left wall, 'b' = bottom wall, 'r' = right wall
        bool unresolved = false; //does this collision need to be resolved?
    };
#define COLLISION_BUFFER_SIZE 6
    COLLISION collision_buffer[(NUM_BALLS * COLLISION_BUFFER_SIZE)];
    int max_buffer_position = -1;
    int collision_buffer_position = 0;

    void add_to_buffer(int ii, int ij, float idt, char iflag, float id = 0) {

      //find the first available buffer position
      for (int i = 0; i < (NUM_BALLS * COLLISION_BUFFER_SIZE); i++) {
        if (collision_buffer[i].unresolved == false) {
          collision_buffer_position = i;
          if (i > max_buffer_position) {
            max_buffer_position = i;
          }
          break;
        }
      }

      collision_buffer[collision_buffer_position].i = ii;
      collision_buffer[collision_buffer_position].j = ij;
      collision_buffer[collision_buffer_position].dt = idt;
      collision_buffer[collision_buffer_position].flag = iflag;
      collision_buffer[collision_buffer_position].d = id;
      collision_buffer[collision_buffer_position].unresolved = true;



    }

    //find the next unresolved collision and void any collisions involving either of the two balls
    int next_collision() {
      int i;
      int j;
      float earliest_dt = 0;
      int earliest_collision = -1;
      int my_cnt = 0;

      for (int k = 0; k <= (max_buffer_position); k++) {
        if (collision_buffer[k].unresolved) {
          my_cnt++;
        }
        if (collision_buffer[k].unresolved && ( fabsf(collision_buffer[k].dt) > earliest_dt ) ) {
          earliest_dt = fabsf(collision_buffer[k].dt);
          earliest_collision = k;
          i = collision_buffer[k].i;
          j = collision_buffer[k].j;
        }
      }

      if (earliest_collision == -1) {
        return earliest_collision;
      }

      //      DEBUG
      //      if (earliest_collision != -1) {
      //        for (int m = 0;m < NUM_LEDS;m++) {
      //              debug_canvas[m] = leds[m];
      //            }
      //            blendXY(leds, balls[i].x, balls[i].y, 0, 0, 255);
      //            if (collision_buffer[earliest_collision].flag == 'B') {
      //              blendXY(leds, balls[j].x, balls[j].y, 0, 0, 255);
      //            }
      //            LED_show();
      //            for (int m = 0;m < NUM_LEDS;m++) {
      //              leds[m] = debug_canvas[m];
      //            }
      //      }

      //if we find a collision, void any collisions involving either of the two balls
      for (int k = 0; k <= (max_buffer_position); k++) {
        //void any collisions with the first ball
        if (collision_buffer[k].i == i || (collision_buffer[k].flag == 'B' && collision_buffer[k].j == i) ) {
          collision_buffer[k].unresolved = false;

        }
        //void any collisions with the second ball, if this is a two-ball collision
        if (collision_buffer[earliest_collision].flag == 'B' && ( collision_buffer[k].i == j || (collision_buffer[k].flag == 'B' && collision_buffer[k].j == j ) ) ) {
          collision_buffer[k].unresolved = false;

        }
      }

      return earliest_collision;
    }

    //check for collisions with walls
    void check_wall_collisions(int i) {
      //check for collisions with walls

      if (balls[i].y + balls[i].r > (MATRIX_HEIGHT - 1.f) * 256.f) {
        float dt = ( balls[i].y + balls[i].r - ((MATRIX_HEIGHT - 1.f) * 256.f) ) / (balls[i].disty);
        if (dt != 0) {
          add_to_buffer(i, 0, dt, 't');
        }
      }
      if (balls[i].y - balls[i].r < -1.f * 256.f) {
        float dt = ( balls[i].y - balls[i].r + 1.f * 256.f) / (balls[i].disty);
        if (dt != 0) {
          add_to_buffer(i, 0, dt, 'b');
        }
      }
      if (balls[i].x - balls[i].r < -1.f * 256.f) {
        float dt = ( balls[i].x - balls[i].r  + 1.f * 256.f) / (balls[i].distx);
        if (dt != 0) {
          add_to_buffer(i, 0, dt, 'l');
        }
      }
      if (balls[i].x + balls[i].r > MATRIX_WIDTH * 256.f) {
        float dt = ( balls[i].x + balls[i].r - (MATRIX_WIDTH * 256.f) ) / (balls[i].distx);
        if (dt != 0) {
          add_to_buffer(i, 0, dt, 'r');
        }
      }

    }

    //check for collisions with balls
    void check_ball_collisions(int i, int j) {
      //if ( abs(balls[i].x_led - balls[j].x_led) < 3 && abs(balls[i].y_led - balls[j].y_led) < 3 ) {

      float dx = balls[j].x - balls[i].x;
      float dy = balls[j].y - balls[i].y;
      float dd = (dx * dx) + (dy * dy);

      //proceed with collision if detected
      if (dd < ( (balls[i].r + balls[j].r) * (balls[i].r + balls[j].r) - 1.f) ) {
        //do some maths to calculate the collision
        float d = sqrt(dd);

        //figure out how far the balls have overlapped
        float vp1 = (balls[i].distx) * dx / d + (balls[i].disty) * dy / d;
        float vp2 = (balls[j].distx) * dx / d + (balls[j].disty) * dy / d;
        if (vp1 != vp2) {
          float dt = (balls[i].r + balls[j].r - d) / (vp1 - vp2);
          add_to_buffer(i, j, dt, 'B', d);
        }
      }

      //}
    }



    unsigned long update_time = 0;

  public:
    void next_effect() {
      current_variation++;
      current_variation %= 3;
    }



    void setup() {
      rotation_alpha = 0;
      rotation_beta = 90;
      rotation_gamma = 0;
      for ( int i = 0; i < NUM_BALLS; i++) {
        balls[i].h = random(256);
        balls[i].x = i % 2 * 2 * 256 + 2 * 256;
        balls[i].y = 120 * 255L - (i / 2) * 5 * 255;
        balls[i].vx = random(1, 200) - 100;
        balls[i].vy = random(1, 200) - 100;
        balls[i].r = 917;
      }
    }

    void reset() {
      rotation_alpha = 0;
      rotation_beta = 90;
      rotation_gamma = 0;
      for (int i = 0; i < NUM_BALLS; i++) {
        if (balls[i].y < 3 * 256) {
          balls[i].vy += 1000;
          balls[i].sleeping = false;
        }
      }
    }


    void loop() {
      static unsigned long frame_time = millis();
      //base our values on ~60FPS
      frame_delta = (millis() - frame_time) / 16.0f;

      //limit our frame_delta to avoid huge jumps through time (like when switching sketches)
      if (frame_delta > 1) {
        frame_delta = 1;
      }


      if (effect_beat == 1) {
        effect_beat = 0;
        if (current_variation == 0) {
          reset();
        }
        if (current_variation == 1) {
          //kind of 3d pulsing effect
          for (int i = 0; i < NUM_BALLS; i++) {
            balls[i].dist[current_pulse] = sqrt(sq(balls[i].x / 256L - MATRIX_WIDTH / 2) + sq(balls[i].y / 256L - MATRIX_HEIGHT / 2));
            balls[i].dist[current_pulse] += 80;
            balls[i].dist[current_pulse] += random(20);
          }
          pulse[current_pulse] = 0;
        }
        if (current_variation == 2) {
          //slow wave pulsing effect
          for (int i = 0; i < NUM_BALLS; i++) {
            balls[i].dist[current_pulse] = sqrt(sq(balls[i].x / 256L - MATRIX_WIDTH / 2) + sq(balls[i].y / 256L - MATRIX_HEIGHT / 2));
            //balls[i].dist[current_pulse] += 80;
            //balls[i].dist[current_pulse] += random(20);
          }
          pulse[current_pulse] = -20;
        }
        current_pulse++;
        current_pulse %= 5;

      }


      if (current_variation == 0) {
        gravity = 20;
      } else {
        gravity = 0;
        gravity_x = 0;
        gravity_y = 0;
      }

      if (current_variation == 0) {


        //construct ZXY rotation matrix

        //find our angles in radians
        //float alpha = ((rotation_alpha-180.0) * PI) / 180.0; //Z
        float beta = (rotation_beta * PI) / 180.f; //X;
        float gamma = (rotation_gamma * PI) / 180.f; //Y

        //store sin/cos in variables
        //float cZ = 1.0*cos( alpha );
        float cX = 1.f * cos( beta );
        //float cY = 1.0*cos( gamma );
        //float sZ = 1.0*sin( alpha );
        float sX = 1.f * sin( beta );
        float sY = 1.f * sin( gamma );

        //create our matrix
        //float m11 = 1.0 * cZ * cY - 1.0 * sZ * sX * sY;
        //float m12 = 1.0 * - cX * sZ;
        //float m13 = 1.0 * cY * sZ * sX + cZ * sY;

        //float m21 = 1.0 * cY * sZ + 1.0 * cZ * sX * sY;
        //float m22 = 1.0 * cZ * cX;
        //float m23 = 1.0 * sZ * sY - 1.0 * cZ * cY * sX;

        float m31 = 1.f * - cX * sY;
        float m32 = 1.f * sX;
        //float m33 = 1.0 * cX * cY;

        /*
          X' m11,    m12,    m13,
          Y' m21,    m22,    m23,
          Z' m31,    m32,    m33
        */

        //Bottom of phone (phone laying flat on table, gravity towards bottom of phone);
        //float bx = 0;
        //float by = 1;
        //float bz = 0;

        gravity_y = 1.f * m32 * gravity * frame_delta * frame_delta;
        gravity_x = 1.f * m31 * gravity * frame_delta * frame_delta;

        /*
          //Around Z-axis: //back to front of phone
          x = x*cos(alpha) - y*sin(alpha);
          y = x*sin(alpha) + y*cos(alpha);
          z = z;
          //Around X-axis: //left to right of phone
          x = x
          y = y*cos(beta) - z*sin(beta);
          z = y*sin(beta) + z*cos(beta);
          //Around Y-axis: //bottom to top of phone
          x = x*cos(gamma) + z*sin(gamma);
          y = y;
          z = z*cos(gamma) - x*sin(gamma);
        */

        //Side of phone (phone laying flat on table, gravity towards right of phone);
        //float lx = 1;
        //float ly = 0;
        //float lz = 0;


      }

      if (frame_delta > 0.01) {
        uint16_t total_cnt = 0;
        uint32_t milliseconds = millis();
        uint32_t microseconds = micros();

        frame_time = millis(); //update the time for the next frame


        for (int i = 0; i < NUM_BALLS; i++) {
          balls[i].update();
        }

        int iterations = 0;
        //calculate collisions
#ifdef DEBUG
        static int max_k = 0;
#endif
        while (true) {


          int k = next_collision();

          if (k == -1) {
            max_buffer_position = 0;


            for (int i = 0; i < (NUM_BALLS * COLLISION_BUFFER_SIZE); i++) {
              collision_buffer[i].unresolved = false;
            }

            //find all existing collisions
            for (int i = 0; i < NUM_BALLS; i++) {

              //check for collisions with walls
              check_wall_collisions(i);

              //check for collisions with other balls
              for (int j = i + 1; j < NUM_BALLS; j++) {
                check_ball_collisions(i, j);
              }

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

          total_cnt++;
          //if there was a collision: calculate it and check for more
          int i = collision_buffer[k].i;
          int j = collision_buffer[k].j;
          char flag = collision_buffer[k].flag;



          if (flag == 'B') {
#ifdef DEBUG2
            Serial.print("#");
            Serial.print(total_cnt);
            Serial.print("BALL");
            Serial.print(i);
            Serial.print(",");
            Serial.print(j);
            Serial.print(" oldiy:");
            Serial.print(balls[i].y);
            Serial.print(" oldix:");
            Serial.print(balls[i].x);
            Serial.print(" oldjy:");
            Serial.print(balls[j].y);
            Serial.print(" oldjx:");
            Serial.print(balls[j].x);
            Serial.print(" oldidisty:");
            Serial.print(balls[i].disty);
            Serial.print(" oldidistx:");
            Serial.print(balls[i].distx);
            Serial.print(" oldjdisty:");
            Serial.print(balls[j].disty);
            Serial.print(" oldjdistx:");
            Serial.print(balls[j].distx);
            Serial.print(" oldivy:");
            Serial.print(balls[i].vy);
            Serial.print(" oldivx:");
            Serial.print(balls[i].vx);
            Serial.print(" oldjvy:");
            Serial.print(balls[j].vy);
            Serial.print(" oldjvx:");
            Serial.print(balls[j].vx);
#endif
            balls[i].cnt++;
            balls[j].cnt++;
            //wake balls if either is sleeping
            balls[i].sleeping = false;
            balls[j].sleeping = false;
            //process the earliest ball collision
            {


              float dx = balls[j].x - balls[i].x;
              float dy = balls[j].y - balls[i].y;
              //float dd = (dx * dx) + (dy * dy);
              //float d = sqrt(dd);
              float& d = collision_buffer[k].d;
              //figure out how far the balls have overlapped

              float ax = dx / d;
              float ay = dy / d;
              float vp1 = (balls[i].distx) * ax + (balls[i].disty) * ay;
              float vp2 = (balls[j].distx) * ax + (balls[j].disty) * ay;
              //float dt = (balls[i].r + balls[j].r - d) / (vp1 - vp2);
              float& dt = collision_buffer[k].dt;
              //back the two balls up so there is no overlap


              balls[i].x -= balls[i].distx * dt;
              balls[i].y -= balls[i].disty * dt;
              balls[j].x -= balls[j].distx * dt;
              balls[j].y -= balls[j].disty * dt;


#ifdef DEBUG2
              Serial.print(" d:");
              Serial.print(d);
              //Serial.print(" d_buffer:");
              //Serial.print(d2);
              Serial.print(" ax:");
              Serial.print(ax);
              Serial.print(" ay:");
              Serial.print(ay);
#endif
              float va1 = (balls[i].vx * ax + balls[i].vy * ay);
              float vb1 = (-balls[i].vx * ay + balls[i].vy * ax);
              float va2 = (balls[j].vx * ax + balls[j].vy * ay);
              float vb2 = (-balls[j].vx * ay + balls[j].vy * ax);
              float ed = 0.95; //dampen multiple collisions greatly
              //float ed = 1.0;
              float vaP1 = va1 + (1.0f + ed) * (va2 - va1) / (1.0f + balls[i].m / balls[j].m);
              float vaP2 = va2 + (1.0f + ed) * (va1 - va2) / (1.0f + balls[j].m / balls[i].m);
#ifdef DEBUG2
              Serial.print(" va1:");
              Serial.print(va1);
              Serial.print(" vb1:");
              Serial.print(vb1);
              Serial.print(" va2:");
              Serial.print(va2);
              Serial.print(" vb2:");
              Serial.print(vb2);
              Serial.print(" vaP1:");
              Serial.print(vaP1);
              Serial.print(" vaP2:");
              Serial.print(vaP2);
#endif

              float oldvxi = balls[i].vx;
              float oldvyi = balls[i].vy;
              float oldvxj = balls[j].vx;
              float oldvyj = balls[j].vy;


              balls[i].vx = vaP1 * ax - vb1 * ay;
              balls[i].vy = vaP1 * ay + vb1 * ax;
              balls[j].vx = vaP2 * ax - vb2 * ay;
              balls[j].vy = vaP2 * ay + vb2 * ax;


              balls[i].distx = balls[i].vx * frame_delta;
              balls[i].disty = balls[i].vy * frame_delta;
              balls[j].distx = balls[j].vx * frame_delta;
              balls[j].disty = balls[j].vy * frame_delta;


              //readjust the previous overlap using the post-collision vectors
              balls[i].x += balls[i].distx * dt;
              balls[i].y += balls[i].disty * dt;
              balls[j].x += balls[j].distx * dt;
              balls[j].y += balls[j].disty * dt;

#ifdef DEBUG2
              Serial.print(" dt:");
              Serial.print(dt);
              //Serial.print(" dt_buffer:");
              //Serial.print(dt2);
              Serial.print(" ivy:");
              Serial.print(balls[i].vy);
              Serial.print(" ivx:");
              Serial.print(balls[i].vx);
              Serial.print(" jvy:");
              Serial.print(balls[j].vy);
              Serial.print(" jvx:");
              Serial.print(balls[j].vx);
              Serial.print(" idisty:");
              Serial.print(balls[i].disty);
              Serial.print(" idistx:");
              Serial.print(balls[i].distx);
              Serial.print(" jdisty:");
              Serial.print(balls[j].disty);
              Serial.print(" jdistx:");
              Serial.print(balls[j].distx);
              Serial.print(" iy:");
              Serial.print(balls[i].y);
              Serial.print(" ix: ");
              Serial.print(balls[i].x);
              Serial.print(" jy:");
              Serial.print(balls[j].y);
              Serial.print(" jx: ");
              Serial.println(balls[j].x);
              //            if(d != d2) {
              //              Serial.println("HEEEEEYYYYYY!!!");
              //              Serial.println("HEEEEEYYYYYY!!!");
              //              Serial.println("HEEEEEYYYYYY!!!");
              //              Serial.println("HEEEEEYYYYYY!!!");
              //              Serial.println("HEEEEEYYYYYY!!!");
              //              Serial.println("HEEEEEYYYYYY!!!");
              //            }
#endif

            }
          }

          if (flag == 't') {
            balls[i].cnt++;
#ifdef DEBUG2
            Serial.print("#");
            Serial.print(total_cnt);
            Serial.print("WALL_T");
            Serial.print(i);
            Serial.print(" oldy:");
            Serial.print(balls[i].y);
            Serial.print(" oldx:");
            Serial.print(balls[i].x);
            Serial.print(" olddisty:");
            Serial.print(balls[i].disty);
            Serial.print(" olddistx:");
            Serial.print(balls[i].distx);
            Serial.print(" oldvy:");
            Serial.print(balls[i].vy);
            Serial.print(" oldvx:");
            Serial.print(balls[i].vx);
#endif
            float dt = ( balls[i].y + balls[i].r - ((MATRIX_HEIGHT - 1.f) * 256.f) ) / (balls[i].disty);
            balls[i].y -= fabsf(balls[i].disty * dt);
            balls[i].vy = -fabsf(balls[i].vy);
            balls[i].vy *= .95f;
            balls[i].disty = balls[i].vy * frame_delta;
            balls[i].y += -fabsf(balls[i].disty * dt);
#ifdef DEBUG2
            Serial.print(" dt:");
            Serial.print(dt);
            Serial.print(" vy:");
            Serial.print(balls[i].vy);
            Serial.print(" vx:");
            Serial.print(balls[i].vx);
            Serial.print(" disty:");
            Serial.print(balls[i].disty);
            Serial.print(" distx:");
            Serial.print(balls[i].distx);
            Serial.print(" y:");
            Serial.print(balls[i].y);
            Serial.print(" x: ");
            Serial.println(balls[i].x);
#endif

          }
          if (flag == 'b') {
            balls[i].cnt++;
#ifdef DEBUG2
            Serial.print("#");
            Serial.print(total_cnt);
            Serial.print("WALL_B");
            Serial.print(i);
            Serial.print(" oldy:");
            Serial.print(balls[i].y);
            Serial.print(" oldx:");
            Serial.print(balls[i].x);
            Serial.print(" olddisty:");
            Serial.print(balls[i].disty);
            Serial.print(" olddistx:");
            Serial.print(balls[i].distx);
            Serial.print(" oldvy:");
            Serial.print(balls[i].vy);
            Serial.print(" oldvx:");
            Serial.print(balls[i].vx);
#endif
            float dt = ( balls[i].y - balls[i].r  + 1.f * 256.f) / (balls[i].disty);
            balls[i].y += fabsf(balls[i].disty * dt);
            balls[i].vy = fabsf(balls[i].vy);
            balls[i].vy *= .95f;
            balls[i].disty = balls[i].vy * frame_delta;
            //if (current_variation == 1) {
            //  balls[i].vy = 1000;
            //}
            balls[i].y += fabsf(balls[i].disty * dt);
#ifdef DEBUG2
            Serial.print(" dt:");
            Serial.print(dt);
            Serial.print(" vy:");
            Serial.print(balls[i].vy);
            Serial.print(" vx:");
            Serial.print(balls[i].vx);
            Serial.print(" disty:");
            Serial.print(balls[i].disty);
            Serial.print(" distx:");
            Serial.print(balls[i].distx);
            Serial.print(" y:");
            Serial.print(balls[i].y);
            Serial.print(" x: ");
            Serial.println(balls[i].x);
#endif

          }
          if (flag == 'l') {
            balls[i].cnt++;
#ifdef DEBUG2
            Serial.print("#");
            Serial.print(total_cnt);
            Serial.print("WALL_L");
            Serial.print(i);
            Serial.print(" oldy:");
            Serial.print(balls[i].y);
            Serial.print(" oldx:");
            Serial.print(balls[i].x);
            Serial.print(" olddisty:");
            Serial.print(balls[i].disty);
            Serial.print(" olddistx:");
            Serial.print(balls[i].distx);
            Serial.print(" oldvy:");
            Serial.print(balls[i].vy);
            Serial.print(" oldvx:");
            Serial.print(balls[i].vx);
#endif
            float dt = ( balls[i].x - balls[i].r  + 1.f * 256.f) / (balls[i].distx);
            balls[i].x += fabsf(balls[i].distx * dt);
            balls[i].vx = fabsf(balls[i].vx);
            balls[i].vx *= .95f;
            balls[i].distx = balls[i].vx * frame_delta;
            balls[i].x += fabsf(balls[i].distx * dt);
#ifdef DEBUG2
            Serial.print(" dt:");
            Serial.print(dt);
            Serial.print(" vy:");
            Serial.print(balls[i].vy);
            Serial.print(" vx:");
            Serial.print(balls[i].vx);
            Serial.print(" disty:");
            Serial.print(balls[i].disty);
            Serial.print(" distx:");
            Serial.print(balls[i].distx);
            Serial.print(" y:");
            Serial.print(balls[i].y);
            Serial.print(" x: ");
            Serial.println(balls[i].x);
#endif

          }
          if (flag == 'r') {
            balls[i].cnt++;
#ifdef DEBUG2
            Serial.print("#");
            Serial.print(total_cnt);
            Serial.print("WALL_R");
            Serial.print(i);
            Serial.print(" oldy:");
            Serial.print(balls[i].y);
            Serial.print(" oldx:");
            Serial.print(balls[i].x);
            Serial.print(" olddisty:");
            Serial.print(balls[i].disty);
            Serial.print(" olddistx:");
            Serial.print(balls[i].distx);
            Serial.print(" oldvy:");
            Serial.print(balls[i].vy);
            Serial.print(" oldvx:");
            Serial.print(balls[i].vx);
#endif
            float dt = ( balls[i].x + balls[i].r - (MATRIX_WIDTH * 256.f) ) / (balls[i].distx);
            balls[i].x -= fabsf(balls[i].distx * dt);
            balls[i].vx = -fabsf(balls[i].vx);
            balls[i].vx *= .95f;
            balls[i].distx = balls[i].vx * frame_delta;
            balls[i].x += -fabsf(balls[i].distx * dt);
#ifdef DEBUG2
            Serial.print(" dt:");
            Serial.print(dt);
            Serial.print(" vy:");
            Serial.print(balls[i].vy);
            Serial.print(" vx:");
            Serial.print(balls[i].vx);
            Serial.print(" disty:");
            Serial.print(balls[i].disty);
            Serial.print(" distx:");
            Serial.print(balls[i].distx);
            Serial.print(" y:");
            Serial.print(balls[i].y);
            Serial.print(" x: ");
            Serial.println(balls[i].x);
#endif


          }

          //recheck collisions for the recalculated balls
          if (flag == 'B') {
            for (int k = 0; k < NUM_BALLS; k++) {
              if (k != i && k != j) {
                check_ball_collisions(i, k);
                check_ball_collisions(j, k);
              }
            }
            check_wall_collisions(i);
            check_wall_collisions(j);
          } else {
            for (int k = 0; k < NUM_BALLS; k++) {
              if (k != i) {
                check_ball_collisions(i, k);
              }
            }
            check_wall_collisions(i);
          }


          iterations++;
        }
        microseconds = micros() - microseconds;
        LED_black();
        //write balls to LED buffer
        for (int i = 0; i < NUM_BALLS; i++) {
          uint8_t hue = balls[i].h;
          if (balls[i].sleeping) {
            hue = 96;
          }
#ifdef DEBUG
          if (millis() > 2000 && millis() - 2000 > debug_time2) {
            Serial.print("Ball: ");
            Serial.print(i);
            Serial.print(" x: ");
            Serial.print(balls[i].x);
            Serial.print(" y: ");
            Serial.print(balls[i].y);
            Serial.print(" vx: ");
            Serial.print(balls[i].vx);
            Serial.print(" vy: ");
            Serial.print(balls[i].vy);
            Serial.print(" distx: ");
            Serial.print(balls[i].distx);
            Serial.print(" disty: ");
            Serial.print(balls[i].disty);
            Serial.print(" collisions: ");
            Serial.println(balls[i].cnt);
          }
#endif
          uint8_t sat = 255;
          float draw_x = balls[i].x;
          float draw_y = balls[i].y;
          for (int p = 0; p < 5; p++) {
            if (pulse[p] < 255) {
              int pulse_dist = abs(balls[i].dist[p] - pulse[p]);
              if (current_variation == 1 && pulse_dist < 80) {
                pulse_dist = 80 - pulse_dist;
                pulse_dist = (pulse_dist * 255) / 80;
                if (pulse_dist > 128) {
                  //ease out only
                  pulse_dist = ease8InOutApprox(pulse_dist);
                }
                if (pulse_dist > 230) {
                  sat = _max(sat - (pulse_dist - 230) * 10, 0);
                }
                pulse_dist = (pulse_dist * 80) / 255;
                //int mag = sqrt( sq(balls[i].x - (MATRIX_WIDTH*256L/2L)) + sq(balls[i].y - (MATRIX_HEIGHT*256L)/2L) );

                draw_x += ((balls[i].x / 256L - MATRIX_WIDTH / 2) / (balls[i].dist[p])) * ((10000 * pulse_dist) / 80);
                draw_y += ((balls[i].y / 256L - MATRIX_HEIGHT / 2) / (balls[i].dist[p])) * ((10000 * pulse_dist) / 80);

              }
              if (current_variation == 2 && pulse_dist < 20) {
                pulse_dist = 20 - pulse_dist;
                pulse_dist = (pulse_dist * 255) / 20;
                pulse_dist = ease8InOutApprox(pulse_dist);
                if (pulse_dist > 230) {
                  sat = _max(sat - (pulse_dist - 230) * 10, 0);
                }
                pulse_dist = (pulse_dist * 20) / 255;
                //int mag = sqrt( sq(balls[i].x - (MATRIX_WIDTH*256L/2L)) + sq(balls[i].y - (MATRIX_HEIGHT*256L)/2L) );

                int mod_x = ((balls[i].x / 256L - MATRIX_WIDTH / 2) / (balls[i].dist[p])) * ((1500 * pulse_dist) / 20);
                int mod_y = ((balls[i].y / 256L - MATRIX_HEIGHT / 2) / (balls[i].dist[p])) * ((1500 * pulse_dist) / 20);
                if (balls[i].dist[p] < 20) {
                  mod_x = (mod_x * balls[i].dist[p]) / 20;
                  mod_y = (mod_y * balls[i].dist[p]) / 20;
                }
                draw_x += mod_x;
                draw_y += mod_y;

              }
            }
          }
          //blendXY(leds, draw_x, draw_y, hue, sat, 255);
          draw_circle_fine(draw_x, draw_y, balls[i].r, hue, sat, 255);
        }
#ifdef DEBUG
        if (millis() - 2000 > debug_time2) {
          debug_time2 = millis();
          Serial.print(" Total collisions: ");
          Serial.println(total_cnt);
          Serial.print(" Total microseconds: ");
          Serial.println(microseconds);
          Serial.print(" Microseconds per collision: ");
          Serial.println(microseconds / (1.f * total_cnt));
          Serial.print(" Max buffered collisions: ");
          Serial.println(max_k);
          max_k = 0;
        }
#endif

        //update LEDS
        LED_show();
        for (int p = 0; p < 5; p ++) {
          if (pulse[p] < 255) {
            if (current_variation == 1) {
              if (pulse[p] < 128) {
                pulse[p] += 10;
              } else {
                pulse[p] += 5;
              }
            }
            if (current_variation == 2) {
              pulse[p]++;
            }
          }
        }


      }
    }

};

int BALLS2D::gravity = 0;
float BALLS2D::gravity_x = 0;
float BALLS2D::gravity_y = 0;
float BALLS2D::frame_delta = 0;
unsigned long BALLS2D::debug_time2 = millis();


LIGHT_SKETCHES::REGISTER<BALLS2D> balls2d("balls2d");

//END BALLS 2D
