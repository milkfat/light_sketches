#ifndef LIGHTS_BALLS_SQUISHY_H
#define LIGHTS_BALLS_SQUISHY_H

//BALLS SQUISHY

#define SQUISHY_MATRIX_WIDTH (MATRIX_WIDTH)
#define SQUISHY_MATRIX_HEIGHT (MATRIX_HEIGHT)

class BALLS_SQUISHY: public LIGHT_SKETCH {
  public:
    BALLS_SQUISHY () {setup();}
    ~BALLS_SQUISHY () {}

    static int gravity;
    static float gravity_x;
    static float gravity_y;
    static unsigned long debug_time2;

  private:
    uint8_t current_variation = 0;
    int pulse[5] = {255, 255, 255, 255, 255};
    int current_pulse = 0;
    bool draw_filled = false;
    
    enum squishy_types {
      SQUISHY_DEFAULT,
      SQUISHY_GRAVITY,
      NUMBER_OF_SQUISHY_TYPES
    };



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
        float r = 768;
        float vr = 768;
        float m = 768;
        bool sleeping = false;
        uint8_t cnt = 0;
        int dist[5] = {0, 0, 0, 0, 0};

        void update() {


          cnt = 0;
          vx -= gravity_x;
          vy -= gravity_y;
          distx = vx;
          disty = vy;
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

      return earliest_collision;
    }

    //check for collisions with walls
    void check_wall_collisions(const int& i) {
      //check for collisions with walls

      if (balls[i].y + balls[i].r > (SQUISHY_MATRIX_HEIGHT-1) * 256.f) {
        float dt = ( balls[i].y + balls[i].r - (SQUISHY_MATRIX_HEIGHT * 256.f) ) / (balls[i].disty);
        if (dt != 0) {
          add_to_buffer(i, 0, dt, 't');
        }
      }
      
      if (balls[i].y - balls[i].r < -0.f * 256.f) {
        float dt = ( balls[i].y - balls[i].r + 1.f * 256.f) / (balls[i].disty);
        if (dt != 0) {
          add_to_buffer(i, 0, dt, 'b');
        }
      }
      
      if (balls[i].x - balls[i].r < -0.f * 256.f) {
        float dt = ( balls[i].x - balls[i].r  + 1.f * 256.f) / (balls[i].distx);
        if (dt != 0) {
          add_to_buffer(i, 0, dt, 'l');
        }
      }
      
      if (balls[i].x + balls[i].r > (SQUISHY_MATRIX_WIDTH-1) * 256.f) {
        float dt = ( balls[i].x + balls[i].r - (SQUISHY_MATRIX_WIDTH * 256.f) ) / (balls[i].distx);
        if (dt != 0) {
          add_to_buffer(i, 0, dt, 'r');
        }
      }

    }

    //check for collisions with balls
    void check_ball_collisions(const int& i, const int& j) {
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


    void wall_collide (const int& i, const float& d, const int& x, const int& y) {
      float i_angle = atan2((balls[i].x+x) - balls[i].x, (balls[i].y+y) - balls[i].y);

      //convert angle to 0-1
      i_angle = (i_angle*113)/355;

      //convert angle to -64 - 191
      i_angle *= -128;
      i_angle += 64;

      //convert angle to 0 - 255
      uint8_t i_angle8 = i_angle;

      //convert angle to segment 0-15
      uint8_t i_a = i_angle8 / 16;

      uint8_t remainder = i_angle8 % 16;

      
      {


        //distance between the two balls

        //total radius of two balls
        float tr = balls[i].r;

        //distance of overlap (compression)
        float c = tr - d;

        //ratio of overlap (compression)
        float cr = c / tr;


        //calculate a number of angle segments to make a smooth curve
        for (int a = -3; a <= 3; a++) {
          
          //find the distance of this segment from our target segment (0 = target segment)
          uint8_t segment_distance = _max(abs(0 - a),0);

          //find the weight for compression of this segment (6 = maximum weight)
          uint8_t segment_weight = 0;
          switch (segment_distance) {
            case 0:
              segment_weight = 0;
            case 1:
              segment_weight = 16;
              break;
            case 2:
              segment_weight = 64;
              break;
            case 3:
              segment_weight = 128;
              break;
          }

          //convert to squish amount 0-255 (target segment is squished the most)
          uint8_t distance_squish = _clamp8(cr*255 - segment_weight);

          //calculate a "squish factor" based on the ratio of two balls' radii
          

          //find the absolute angle segment that we are working with
          uint8_t i_a2 = i_a + a;

          //normalize segment to 0-15
          i_a2 *= 16;
          i_a2 /= 16;

          //apply squish using the compression ratio and squish amount
          circle_angles[i][i_a2] = _max(_min(circle_angles[i][i_a2], 255 - distance_squish),0);
          float area = (balls[i].r*balls[i].r*355)/113;
          area += (area*cr)/24;
          float new_radius = sqrt((area*113)/355);
          balls[i].vr += new_radius-balls[i].r;

        }
        
        float vxdi = (cr*cr)*25;
        float vydi = (cr*cr)*25;

        
        balls[i].vx += vxdi*-x;
        balls[i].vy += vydi*-y;
        
        //balls[i].vx *= .9999;
        //balls[i].vy *= .9999;

      }
    } //wall_collide()


    void draw_balls() {

      for (int i = 0; i < NUM_BALLS; i++) {
        uint8_t hue = balls[i].h;
        if (balls[i].sleeping) {
          hue = 96;
        }

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
              //int mag = sqrt( sq(balls[i].x - (SQUISHY_MATRIX_WIDTH*256L/2L)) + sq(balls[i].y - (SQUISHY_MATRIX_HEIGHT*256L)/2L) );

              draw_x += ((balls[i].x / 256L - SQUISHY_MATRIX_WIDTH / 2) / (balls[i].dist[p])) * ((10000 * pulse_dist) / 80);
              draw_y += ((balls[i].y / 256L - SQUISHY_MATRIX_HEIGHT / 2) / (balls[i].dist[p])) * ((10000 * pulse_dist) / 80);

            }
            if (current_variation == 2 && pulse_dist < 20) {
              pulse_dist = 20 - pulse_dist;
              pulse_dist = (pulse_dist * 255) / 20;
              pulse_dist = ease8InOutApprox(pulse_dist);
              if (pulse_dist > 230) {
                sat = _max(sat - (pulse_dist - 230) * 10, 0);
              }
              pulse_dist = (pulse_dist * 20) / 255;
              //int mag = sqrt( sq(balls[i].x - (SQUISHY_MATRIX_WIDTH*256L/2L)) + sq(balls[i].y - (SQUISHY_MATRIX_HEIGHT*256L)/2L) );

              int mod_x = ((balls[i].x / 256L - SQUISHY_MATRIX_WIDTH / 2) / (balls[i].dist[p])) * ((1500 * pulse_dist) / 20);
              int mod_y = ((balls[i].y / 256L - SQUISHY_MATRIX_HEIGHT / 2) / (balls[i].dist[p])) * ((1500 * pulse_dist) / 20);
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
        if (draw_filled) {
          draw_circle_fine(draw_x, draw_y, balls[i].vr, hue, sat, 255, i, 16, i*32);
          draw_circle_fine(draw_x, draw_y, balls[i].vr, 0, 0, 0, i, 16, i*32);
          CRGB rgb = CHSV(hue,sat,255);
          y_buffer_fill(leds, rgb, i*32+16);
          reset_y_buffer();
          reset_x_buffer();
        } else {
          draw_circle_fine(draw_x, draw_y, balls[i].vr, hue, sat, 255, i, 16, i*32);
        }
        
        //dma_display.drawPixelRGB36(draw_x/256, draw_y/256, 255, 0, 0);
      }

    } //draw_balls();



    void process_collisions() {

      uint16_t total_cnt = 0;
      int iterations = 0;

      while (true) {
          


        int k = next_collision();

        


        //if no collision: break out of while loop
        if (k == -1) {
          break;
        }

        total_cnt++;
        //if there was a collision: calculate it and check for more
        int i = collision_buffer[k].i;
        int j = collision_buffer[k].j;
        char flag = collision_buffer[k].flag;
        collision_buffer[k].unresolved = false;



        if (flag == 'B') {

          balls[i].cnt++;
          balls[j].cnt++;
          //wake balls if either is sleeping
          balls[i].sleeping = false;
          balls[j].sleeping = false;
          //process the earliest ball collision
          
          float i_angle = atan2(balls[j].x - balls[i].x, balls[j].y - balls[i].y);
          float j_angle = atan2(balls[i].x - balls[j].x, balls[i].y - balls[j].y);

          //convert angle to 0-1 (113/255 == pi)
          i_angle = (i_angle*113)/355;
          j_angle = (j_angle*113)/355;

          //convert angle to -64 - 191
          i_angle *= -128;
          j_angle *= -128;
          i_angle += 64;
          j_angle += 64;

          //convert angle to 0 - 255
          uint8_t i_angle8 = i_angle;
          uint8_t j_angle8 = j_angle;

          //convert angle to segment 0-15
          uint8_t i_a = i_angle8 / 16;
          uint8_t j_a = j_angle8 / 16;

          //find the remainder if the angle isn't an exact match
          uint8_t i_ar = i_angle8 % 16;
          uint8_t j_ar = j_angle8 % 16;

          


          
          {


            float dx = balls[j].x - balls[i].x;
            float dy = balls[j].y - balls[i].y;
            //float dd = (dx * dx) + (dy * dy);
            //float d = sqrt(dd);

            //distance between the two balls
            float& d = collision_buffer[k].d;

            //total radius of two balls
            float tr = balls[i].r+balls[j].r;

            //distance of overlap (compression)
            float c = tr - d;

            //ratio of overlap (compression)
            float cr = c / tr;

            //find the ratio between ball sizes minimum = 4, equal = 32, max = 255
            uint8_t i_ratio = (balls[i].r*32)/balls[j].r;
            uint8_t j_ratio = (balls[j].r*32)/balls[i].r;

            i_ratio = sqrt(i_ratio*256);
            j_ratio = sqrt(j_ratio*256);

            i_ratio = _max(i_ratio, 8);
            i_ratio = _max(i_ratio, 8);

            //calculate a number of angle segments to make a smooth curve
            for (int a = -3; a <= 3; a++) {
              
              //find the distance of this segment from our target segment (0 = target segment)
              uint8_t segment_distance = abs(0 - a);

              //find the weight for compression of this segment (6 = maximum weight)
              uint8_t segment_weight = 0;
              switch (segment_distance) {
                case 0:
                  segment_weight = 0;
                case 1:
                  segment_weight = 4;
                  break;
                case 2:
                  segment_weight = 32;
                  break;
                case 3:
                  segment_weight = 128;
                  break;
              }

              //convert to squish amount 0-255 (target segment is squished the most)
              uint8_t distance_squish = _clamp8(cr*255 - segment_weight)*1.4f;
              
          
              //find the absolute angle segment that we are working with
              uint8_t i_a2 = i_a + a;
              uint8_t j_a2 = j_a + a;

              //normalize segment to 0-15
              i_a2 *= 16;
              i_a2 /= 16;
              j_a2 *= 16;
              j_a2 /= 16;

              //apply squish using the compression ratio and squish amount
              circle_angles[i][i_a2] = _max(_min(circle_angles[i][i_a2], 255 - distance_squish),0);
              circle_angles[j][j_a2] = _max(_min(circle_angles[j][j_a2], 255 - distance_squish),0);
              {
              float area = (balls[i].r*balls[i].r*355)/113;
              area += (area*cr)/16;
              float new_radius = sqrt((area*113)/355);
              balls[i].vr += new_radius-balls[i].r;
              }
              {
              float area = (balls[j].r*balls[j].r*355)/113;
              area += (area*cr)/16;
              float new_radius = sqrt((area*113)/355);
              balls[j].vr += new_radius-balls[j].r;
              }

            }
            
            //ball mass (kinda)
            float mi = (balls[i].m/256.f)*(balls[i].m/256.f);
            float mj = (balls[j].m/256.f)*(balls[j].m/256.f);

            //use mass to cause the balls to repel (bounce off) eachother
            float vdi = (((mi-mj)/(mi+mj)) * (mi-mj)) + mj;
            float vdj = (((mj-mi)/(mj+mi)) * (mj-mi)) + mi;

            


            //ratio of x and y components
            float rxi = (balls[i].x - balls[j].x) / (abs(dx)+abs(dy));
            float ryi = (balls[i].y - balls[j].y) / (abs(dx)+abs(dy));
            float rxj = (balls[j].x - balls[i].x) / (abs(dx)+abs(dy));
            float ryj = (balls[j].y - balls[i].y) / (abs(dx)+abs(dy));

            float vxdi = vdi * rxi * (cr*cr)*2;
            float vydi = vdi * ryi * (cr*cr)*2;
            float vxdj = vdj * rxj * (cr*cr)*2;
            float vydj = vdj * ryj * (cr*cr)*2;

            
            balls[i].vx += vxdi;
            balls[i].vy += vydi;
            balls[j].vx += vxdj;
            balls[j].vy += vydj;

            balls[i].vx *= .9999;
            balls[i].vy *= .9999;
            balls[j].vx *= .9999;
            balls[j].vy *= .9999;
            
            balls[i].distx = balls[i].vx;
            balls[i].disty = balls[i].vy;
            balls[j].distx = balls[j].vx;
            balls[j].disty = balls[j].vy;





          }
          
        }

        if (flag == 't') {
          balls[i].cnt++;
          float d = (SQUISHY_MATRIX_HEIGHT-1 ) * 256.f - balls[i].y;
          wall_collide(i, d, 0, 1);

        }
        if (flag == 'b') {
          balls[i].cnt++;
          float d = balls[i].y;
          wall_collide(i, d, 0, -1);

        }
        if (flag == 'l') {
          balls[i].cnt++;
          float d = balls[i].x;
          wall_collide(i, d, -1, 0);

        }
        if (flag == 'r') {
          balls[i].cnt++;
          float d = ( ((SQUISHY_MATRIX_WIDTH-1) * 256.f) - balls[i].x );
          wall_collide(i, d, 1, 0);

        }
        
        iterations++;
      }

    } //process_collisions()


    unsigned long frame_time = 0;

  public:
    void next_effect() {
      current_variation++;
      current_variation %= 2;
    }



    void setup() {
      frame_time = millis();
      for ( int i = 0; i < NUM_BALLS; i++) {
        balls[i].h = random(256);
        //balls[i].h = 128;
        //balls[i].x = (((i+1)*SQUISHY_MATRIX_WIDTH)/(NUM_BALLS+2))*256;
        balls[i].x = (((i/2+1)*SQUISHY_MATRIX_WIDTH)/(NUM_BALLS/2+2))*256;
        //balls[i].y = random(1,7)*(SQUISHY_MATRIX_HEIGHT/8)*256;
        balls[i].y = ((i+1)*SQUISHY_MATRIX_HEIGHT*256)/(NUM_BALLS+2);
        
        //balls[i].r = random(512,2048);
        balls[i].r = random(950,1736);
        //balls[i].r = 1024;
        balls[i].m = balls[i].r;
        balls[i].vx = random(1, 200) - 100;
        balls[i].vy = random(1, 200) - 100;
        //balls[i].vx = 0;
        //balls[i].vy = 10;
      }
    }


    void reset() {
      for (int i = 0; i < NUM_BALLS; i++) {
        if (balls[i].y < 3 * 256) {
          balls[i].vy += 1000;
          balls[i].sleeping = false;
        }
      }
    }


    void loop() {
//    CRGB rgb = CRGB(32,32,32);
//    draw_line_fine(leds,(MATRIX_WIDTH-4)*256,0,(MATRIX_WIDTH-4)*256,(MATRIX_HEIGHT-1)*256,rgb);
      if (button1_click) {
        draw_filled = !draw_filled;
        reset_y_buffer();
        reset_x_buffer();
        button1_click = false;
      }

      if (current_variation == 0) {
        gravity = 20;
        gravity_x = 0;
        gravity_y = 0;
      } else {
        gravity = 0;
        gravity_x = 0;
        gravity_y = 3;
      }

     
      //base our values on ~60FPS
      if (millis() - 10 > frame_time) {

        while (millis() - 10 > frame_time) {
          frame_time += 10; //update the time for the next frame


          for (int i = 0; i < NUM_BALLS; i++) {
            balls[i].vr = balls[i].r;
            balls[i].update();
          }

          if (button2_down) {
            for (int i = 0; i < NUM_BALLS; i++) {
                balls[i].vx *= .99f;
                balls[i].vy *= .99f;
            }
          }

          
          
          //calculate collisions
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
          reset_circle_angles();

          //process collisions 
          process_collisions();   
          
        }

        
        //LED_black();
        //draw balls to LED buffer
        draw_balls();



        
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

        
        LED_show();
        LED_black();
      }
    }

};

int BALLS_SQUISHY::gravity = 0;
float BALLS_SQUISHY::gravity_x = 0;
float BALLS_SQUISHY::gravity_y = 0;
unsigned long BALLS_SQUISHY::debug_time2 = millis();

LIGHT_SKETCHES::REGISTER<BALLS_SQUISHY> balls_squishy("balls_squishy");
//END BALLS 2D

#endif