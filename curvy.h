//CURVY

#if MATRIX_WIDTH > MATRIX_HEIGHT
#define GRID_WIDTH MATRIX_HEIGHT
#define GRID_HEIGHT MATRIX_WIDTH
#else
#define GRID_WIDTH MATRIX_WIDTH
#define GRID_HEIGHT MATRIX_HEIGHT
#endif

class CURVY: public LIGHT_SKETCH {
  public:
    CURVY () {setup();}
    ~CURVY () {}
#define NUM_POINTS 10
#define NUM_THINGS 10


  private:

    #define NUM_BUBBLES 20
    struct BUBBLE {
      int x=0;
      int y=MATRIX_HEIGHT*256;
      int z=0;
      int r=1;
    };

    BUBBLE bubbles[20];

    long my_points[NUM_THINGS][NUM_POINTS][2];
    uint32_t loop_time = millis();
    #define FISH_POINTS 7
    int8_t fish_points[FISH_POINTS][2] =
    {
      //{-32,0}, //tail center
      {-64,32}, //tail tip bottom
      {-16,0}, //tail meets body
      {32,-32}, //body top
      {64,0}, //nose
      {32,32}, //body top
      {-16,0}, //tail meets body
      {-64,-32}, //tail tip bottom
      //{-32,0} //tail center
    };

    struct FISH {
      int32_t x = 0;
      int32_t y = -100*256;
      int32_t z = 100*256;
      uint8_t ay = 0;
      uint8_t az = 0;
      int16_t wiggle = 0;
      int16_t speed = 0;
      int16_t add_speed = 0;
      long target_x = 0;
      long target_y = 0;
      long target_z = 0;
      long age = millis();
      long new_target_age = 0;
      uint8_t hue = 0;
      uint8_t sat = 0;

      void new_target() {
          target_x = random(64*256) - 32*256;
          //target_x = 0;
          target_y = random(500*256) - 250*256;
          target_z = random(400*256) - 300*256;
          //target_z = 0;
          add_speed = random(20000)+10000;
      }
    };
    #define NUM_FISH 25
    FISH fishies[NUM_FISH];

    //some jellyfish tentacles
    #define NUM_TENTACLE_SEGMENTS 10
    struct tentacle_segment {
      int32_t x = -20*256;
      int32_t y = -150*256;
      int32_t z = 0;
      int16_t vx = 0;
      int16_t vy = 0;
      int16_t vz = 0;
    };
    #define NUM_TENTACLES 10

    //POINT points[NUM_POINTS];

  public:

    void next_effect() {
    }

    void setup() {

      //bubbles
      bubble_time = millis();
      for (int i = 0; i < NUM_BUBBLES; i++) {
        bubbles[i].x =(MATRIX_WIDTH/3)*256 + random((MATRIX_WIDTH/6)*256);
        bubbles[i].y = random(0, MATRIX_HEIGHT*256);
        bubbles[i].z = 0;
        bubbles[i].r = random(256);
      }

      //grass
      for (int i = 0; i < NUM_THINGS; i++) {
          int h = (random(64)+120);
          for (int j = 0; j < NUM_POINTS; j++) {
              my_points[i][j][1] = j * (MATRIX_HEIGHT*h)/NUM_POINTS;
          }
      }

      //fish
      for (int i = 0; i < NUM_FISH; i++) {
        fishies[i].x = random(32*256) - 16*256;
        fishies[i].y = random(400*256) - 200*256;
        fishies[i].z = random(200*256) - 100*256;
        fishies[i].wiggle = i*1024;
        fishies[i].az = random(256);
        fishies[i].ay = random(256);
        fishies[i].speed = random(10)+3;
        fishies[i].age = millis();
        fishies[i].hue = random(256);
        fishies[i].sat = random(128,256);
      }

    }

    void reset() {
      setup();
      for (int i = 0; i < NUM_LEDS; i++)
      {
        leds[i] = CRGB::Black;
      }

    }
    
    void loop() {


      if (millis()-16 > loop_time) {

        loop_time = millis();

        
        LED_show();
        LED_black();

        handle_bubbles();

        handle_jellies();

        handle_fish();

        draw_grass();

        draw_water();

      }

    } //loop()





    struct JELLY {
      int32_t x = -20*256;
      int32_t y = 300*256;
      int32_t z = 0*256;
      uint8_t az = -5;
      uint8_t step = 0;
      uint8_t speed = 2;
      int32_t age = millis();
      tentacle_segment tentacles[NUM_TENTACLES][NUM_TENTACLE_SEGMENTS];
    };

    #define NUM_JELLIES 2
    JELLY jellies[NUM_JELLIES];
    
    void handle_jellies() {
      for (int i = 0; i < NUM_JELLIES; i++) {
        handle_jelly(jellies[i]);
      }
    }

    bool draw_jelly;
    bool update_jelly;
    void handle_jelly(JELLY& jelly) {

      draw_jelly = false;
      update_jelly = true;
      while (!draw_jelly) {

        if (loop_time - 16 > jelly.age) {
          jelly.age += 16;
          if (loop_time - 16 <= jelly.age) {
            draw_jelly = true;
          }
        } else {
          draw_jelly = true;
          update_jelly = false;
        }

        if (update_jelly) {
          jelly.step += jelly.speed;
        }

        int jelly_speed = sin8(jelly.step-48)-64;
        jelly_speed = _max(jelly_speed,0)/2;
        jelly_speed = (jelly_speed*jelly.speed)/2;

        uint8_t jelly_step2 = jelly.step+64;


        if (jelly.y > 250*256 || jelly.x > 100*256 || jelly.x < -100*256) {
          
          jelly.x = 50*256;
          jelly.y = random(200*256) - 100*256;
          jelly.az = 5+random(40);
          
          if (random(2)) {
            jelly.x *= -1;
            jelly.az *= -1;
          }

          jelly.speed = random(1,6);

          for (int i = 0; i < NUM_TENTACLES; i++) {
            for (int j = 0; j < NUM_TENTACLE_SEGMENTS; j++) {
              jelly.tentacles[i][j].x = jelly.x;
              jelly.tentacles[i][j].y = jelly.y;
              jelly.tentacles[i][j].z = jelly.z;
              jelly.tentacles[i][j].vx = 0;
              jelly.tentacles[i][j].vy = 0;
              jelly.tentacles[i][j].vz = 0;
            }
          }

        }
        
        #define NUM_JELLY_SEGMENTS 10
        long ring_points[NUM_JELLY_SEGMENTS][2];

        //a jellyfish consists of a segment rotated around an axis (the center of the jellyfish)
        //each segment is made up of three points
        long jelly_points[3][3] = {
          {2*256,100*42,0},
          {70*24,70*42,0},
          {100*24,0,0}
        };

        //animate midpoint
        long jp1x = jelly_points[1][0] + sin8(jelly_step2)*8;
        long jp1y = jelly_points[1][1] + cos8(jelly_step2)*8;

        //animate tips
        long jp2x = jelly_points[2][0] + sin8(jelly.step)*12;
        long jp2y = jelly_points[2][1] + cos8(jelly.step)*12;

        //process each segment of the jellyfish
        for (int i = 0; i < NUM_JELLY_SEGMENTS; i++) {
          uint8_t ang = i*(256/NUM_JELLY_SEGMENTS)+NUM_JELLY_SEGMENTS;
          uint8_t s = sin8(ang);
          uint8_t c = cos8(ang);

          long p0[3];
          long p1[3];
          long p2[3];

          //build the jellyfish by rotating around y-axis:
          //rotate x coord
          p0[0] = ( jelly_points[0][0]*( c - 128 ) - jelly_points[0][2]*( s - 128 )  )/128;
          p1[0] = ( jp1x*( c - 128 ) - jelly_points[1][2]*( s - 128 )  )/128;
          p2[0] = ( jp2x*( c - 128 ) - jelly_points[2][2]*( s - 128 )  )/128;
          
          //don't rotate y coords
          p0[1] = jelly_points[0][1];
          p1[1] = jp1y;
          p2[1] = jp2y;

          //rotate z coords
          p0[2] = ( jelly_points[0][0]*( s - 128 ) + jelly_points[0][2]*( c - 128 )  )/128;
          p1[2] = ( jp1x*( s - 128 ) + jelly_points[1][2]*( c - 128 )  )/128;
          p2[2] = ( jp2x*( s - 128 ) + jelly_points[2][2]*( c - 128 )  )/128;

          //rotate the jellyfish to particular orientation
          matrix.rotate_z(p0,jelly.az);
          matrix.rotate_z(p1,jelly.az);
          matrix.rotate_z(p2,jelly.az);

          //translate to coordinates
          p0[0] += jelly.x;
          p0[1] += jelly.y;
          p0[2] += jelly.z;

          p1[0] += jelly.x;
          p1[1] += jelly.y;
          p1[2] += jelly.z;

          p2[0] += jelly.x;
          p2[1] += jelly.y;
          p2[2] += jelly.z;

          if (update_jelly) {
            //update the tentacles
            jelly.tentacles[i][0].x = p2[0];
            jelly.tentacles[i][0].y = p2[1];
            jelly.tentacles[i][0].z = p2[2];
          }

          //rotate the jellyfish as part of our global 3d matrix
          matrix.rotate(p0);
          matrix.rotate(p1);
          matrix.rotate(p2);

          //some sort of screen scaling
          matrix.z_scale(p0);
          matrix.z_scale(p1);
          matrix.z_scale(p2);

          //map our 3d coordinates to screen coordinates
          matrix.perspective(p0);
          matrix.perspective(p1);
          matrix.perspective(p2);

          long points[3][2] {
            {p0[0],p0[1]},
            {p1[0],p1[1]},
            {p2[0],p2[1]}
          };

          if (draw_jelly) {
            //draw each segment
            matt_curve8(points, 3,212,128,64,false,false,true,255,128);
          }

          //add the outer point of each segment to an array
          //we use this array to draw a circle around the jelly at the end
          ring_points[i][0] = p2[0];
          ring_points[i][1] = p2[1];
          
        }

        if (draw_jelly) {
          //draw a circle tying the jelly's segments together
          matt_curve8(ring_points,NUM_JELLY_SEGMENTS,212,128,64,false,true,true,255,128);
        }

        //a unit vector representing the jelly's speed
        long speed[3] = {0,256,0};

        //rotate the unit vector to figure out how far the jelly should move in each direction
        matrix.rotate_z(speed,jelly.az);

        if (update_jelly) {
          //update the jelly's position
          jelly.x += (jelly_speed*speed[0])/256;
          jelly.y += (jelly_speed*speed[1])/256;
          jelly.z += (jelly_speed*speed[2])/256;
        }

        handle_tentacles(jelly.tentacles);

      }

    } //handle_jelly()


    

    void handle_tentacles(tentacle_segment tentacles[NUM_TENTACLE_SEGMENTS][NUM_TENTACLE_SEGMENTS]) {
      for (int i = 0; i < NUM_JELLY_SEGMENTS; i++) {
        long tentacle_points[NUM_TENTACLE_SEGMENTS][2];
        for (int j = 0; j < NUM_TENTACLE_SEGMENTS; j++) {

          //apply physics to all but the first point (which is affixed to the jelly)
          //points pull against eachother
          if (j > 0 && update_jelly) {
            
            int32_t dx = (tentacles[i][j-1].x - tentacles[i][j].x);
            int32_t dy = (tentacles[i][j-1].y - tentacles[i][j].y);
            int32_t dz = (tentacles[i][j-1].z - tentacles[i][j].z);

            int32_t d2 = dx*dx + dy*dy + dz*dz;

            if ( d2 > ((3*256) * (3*256)) ) {
              //distance between points
              int32_t d = sqrt(d2);
              //overage distance
              int32_t od = (d - 3*256);
              //points pull equally on one-another, except for the first point
              if (j > 1) {
                od/=2;
              }

              //overage distance along each axis
              dx = (od*dx)/d;
              dy = (od*dy)/d;
              dz = (od*dz)/d;
              
              
              //if our current speed is less than the overage distance, then speed up
              if ( dx != 0 && tentacles[i][j].vx / dx < 1 ) {
                tentacles[i][j].vx += (dx-tentacles[i][j].vx)/2;
              }
              if ( dy != 0 && tentacles[i][j].vy / dy < 1 ) {
                tentacles[i][j].vy += (dy-tentacles[i][j].vy)/2;
              }
              if ( dz != 0 && tentacles[i][j].vz / dz < 1 ) {
                tentacles[i][j].vz += (dz-tentacles[i][j].vz)/2;
              }
              

              //accelerate the other point along each axis
              if (j > 1) {
                //overage distance along each axis
                dx *= -1;
                dy *= -1;
                dz *= -1;
                
                //if our current speed is less than the overage distance, then speed up
                if ( dx != 0 && tentacles[i][j-1].vx / dx < 1 ) {
                  tentacles[i][j-1].vx += (dx-tentacles[i][j-1].vx)/2;
                }
                if ( dy != 0 && tentacles[i][j-1].vy / dy < 1 ) {
                  tentacles[i][j-1].vy += (dy-tentacles[i][j-1].vy)/2;
                }
                if ( dz != 0 && tentacles[i][j-1].vz / dz < 1 ) {
                  tentacles[i][j-1].vz += (dz-tentacles[i][j-1].vz)/2;
                }
                
              }


            } 

            //update position
            tentacles[i][j].x += tentacles[i][j].vx;
            tentacles[i][j].y += tentacles[i][j].vy;
            tentacles[i][j].z += tentacles[i][j].vz;
            
            
          }

          if (draw_jelly) {
            long p[3];
            p[0] = tentacles[i][j].x;
            p[1] = tentacles[i][j].y;
            p[2] = tentacles[i][j].z;

            //rotate with global matrix
            matrix.rotate(p);

            //some sort of screen scaling
            matrix.z_scale(p);

            //map our 3d coordinates to screen coordinates
            matrix.perspective(p);

            tentacle_points[j][0] = p[0];
            tentacle_points[j][1] = p[1];
          }

        }

        if (draw_jelly) {
          matt_curve8(tentacle_points,NUM_TENTACLE_SEGMENTS,96,64,64,false,false,true,255,128);
        }

      }
    }




    uint32_t bubble_time = millis();;
    void handle_bubbles() {
      while (bubble_time < loop_time) {

        bubble_time += 16;

        for(int i = 0; i < NUM_BUBBLES; i++) {
          if (bubbles[i].y > (MATRIX_HEIGHT+1)*256) {
            bubbles[i].x =(MATRIX_WIDTH/3)*256 + random((MATRIX_WIDTH/6)*256);
            bubbles[i].y = 0;
            bubbles[i].z = 0;
            bubbles[i].r = random(256);
          }
          bubbles[i].y += bubbles[i].r/8+64;
          bubbles[i].y += random(-32,33);
          bubbles[i].x += random(-32,33);
        }
        
      }

      for(int i = 0; i < NUM_BUBBLES; i++) {
        draw_circle_fine(bubbles[i].x, bubbles[i].y, bubbles[i].r/2+128, 0, 0, 128-(256-bubbles[i].r)/64, -1, 64);
      }
    }//handle_bubbles()





    void handle_fish() {
      for (int i = 0; i < NUM_FISH; i++) {
        update_fish(fishies[i]);
        draw_fish(fishies[i]);
      }
    }//handle_fish()





    void update_fish(FISH& fish) {

      //update fish independent of framerate
      while (loop_time > fish.age) {
        fish.age += 16;

        fish.add_speed *= .999; //slow down
        int speed = fish.speed + fish.add_speed/1000; //calculate current speed

        if (fish.age > fish.new_target_age) {
          fish.new_target();
          fish.new_target_age = millis() + (15 + random(30))*1000;
        }
        
        long a_thing = fish.x/256 - fish.target_x/256;
        long b_thing = fish.z/256 - fish.target_z/256;
        long c_length = sqrt(a_thing*a_thing + b_thing*b_thing); //this is the distance between the target and the fish's y-axis

        //target angle around z-axis (the angle between y-axis and target)
        uint8_t target_az = -(atan2(c_length - fish.x/256, fish.target_y/256 - fish.y/256)*255)/(2*PI)+64;
        //target angle around y-axis
        uint8_t target_ay = -(atan2(fish.target_x/256 - fish.x/256, fish.target_z/256 - fish.z/256)*255)/(2*PI)+64;

        //find the difference between the angle of our fish and the angle pointing to the target
        int8_t y_diff = target_ay - fish.ay;
        int8_t z_diff = target_az - fish.az;

        //take a fraction of that angle to change on each update (slowly turning toward the target)
        if (fish.age%32 < 16) {
          fish.ay += (y_diff/(40-speed/5));
          fish.az += (z_diff/(40-speed/5));
        }
        //iterate our wiggle
        fish.wiggle += speed/10 + 1;

        //update fish's position by rotating a unit vector {1,0,0} and using the resulting x,y,z components as speed
        {
          long p[3];

          long z = 0;

          //rotate around z-axis:
          //rotate x
          long x = ( 16*( cos8(fish.az) - 128 ) )/128;
          //rotate y
          p[1] = ( 16*( sin8(fish.az) - 128 ) )/128;


          //rotate around y-axis:
          //rotate x
          p[0] = ( x*( cos8(fish.ay) - 128 ) - z*( sin8(fish.ay) - 128 )  )/128;
          //rotate z
          p[2] = ( x*( sin8(fish.ay) - 128 ) + z*( cos8(fish.ay) - 128 )  )/128;
          

          fish.x += p[0]*speed;
          fish.y += p[1]*speed;
          fish.z += p[2]*speed;
          
        }


      }
    } //update_fish()





    void draw_fish(FISH& fish) {

      long points[FISH_POINTS][2];
      int32_t detail_z = 0;
      int32_t on_screen = false;
      for (int i = 0; i < FISH_POINTS; i++) {
        long p[3];

        //scale z
        long z = (inoise8(fish_points[i][0]*2+fish.wiggle*8, 0, 0)-128)*16;

        //rotate around z-axis:
        //rotate x
        long x = ( fish_points[i][0]*64*( cos8(fish.az) - 128 ) - fish_points[i][1]*48*( sin8(fish.az) - 128 )  )/128;
        //rotate y
        p[1] = ( fish_points[i][0]*64*( sin8(fish.az) - 128 )  + fish_points[i][1]*48*( cos8(fish.az) - 128 )  )/128;


        //rotate around y-axis:
        //rotate x
        p[0] = ( x*( cos8(fish.ay) - 128 ) - z*( sin8(fish.ay) - 128 )  )/128;
        //rotate z
        p[2] = ( x*( sin8(fish.ay) - 128 ) + z*( cos8(fish.ay) - 128 )  )/128;

        //translate fish to position        
        p[0] += fish.x;
        p[1] += fish.y;
        p[2] += fish.z;
        long p0[3];
        matrix.rotate(p, p0);

        p0[2] += -180 * 256 + (200 * 256 * debug_scaler) / 256;
        
        detail_z = p0[2];

        matrix.perspective(p0);

        points[i][0] = p0[0];
        points[i][1] = p0[1];

        //check to make sure at least one point is located on the screen
        if (!on_screen && p0[0] > 0 && p0[0] < MATRIX_WIDTH*256 && p0[1] > 0 && p0[1] < MATRIX_HEIGHT*256) {
          on_screen = true;
        }

      }

      //draw the fish if it is on the screen
      if (on_screen) {
        uint8_t detail = 255;
        //lower the level of detail when farther away
        if (detail_z > -150*256) {
          detail = 128;
        }
        int bri = detail_z/512+255;
        bri = _max(_min(bri,255),0);
        matt_curve8(points,FISH_POINTS,fish.hue,fish.sat,bri,false,false,true,255,detail);
      }
            // //fish debug, lines between fish and target
            // long v0[3] = {fish.x,fish.y,fish.z};
            // long v1[3] = {fish.target_x, fish.target_y, fish.target_z};
            // long p0[3];
            // long p1[3];

            // matrix.rotate(v0, p0);
            // matrix.rotate(v1, p1);

            // p0[2] += -180 * 256 + (200 * 256 * debug_scaler) / 256;
            // p1[2] += -180 * 256 + (200 * 256 * debug_scaler) / 256;

            // //correct 3d perspective
            
            // matrix.perspective(p0);
            // matrix.perspective(p1);

            // draw_line_fine(leds, p0[0], p0[1], p1[0], p1[1], 255, 255, 255, -10000, 255, true);

    } //draw_fish()





    void draw_grass() {

      int y = -loop_time/4;
      int max_y;
      
      for (int i = 0; i < NUM_THINGS; i++) {
        int offset = fmix32(i)%((MATRIX_WIDTH+3)*256);
        int speed = fmix32(i)%(64);

        max_y = my_points[i][NUM_POINTS-1][1]/256;
        
        for (int j = 0; j < NUM_POINTS; j++) {
          my_points[i][j][0] = MATRIX_WIDTH*(_min(_max(inoise16(y*2, my_points[i][j][1]*4+y*(128+speed), i * 65536)/128,0),512)-256)/2;
          my_points[i][j][0] = (my_points[i][j][0] * _min(j,NUM_POINTS)) / NUM_POINTS;
          my_points[i][j][0] += offset;
        }

          //draw curve
          //swap X and Y axis if our display width is greater than the height
          uint8_t b = (i*255)/NUM_THINGS;
          #if MATRIX_WIDTH > MATRIX_HEIGHT
          matt_curve8(temp_canvas, my_points[i], NUM_POINTS, 96, default_saturation, 255, true, false, true, 255, 255);
          #else
          matt_curve8(temp_canvas, my_points[i], NUM_POINTS, 96, default_saturation, 255, false, false, true, 255, 255);
          #endif

          CRGB full_rgb = CHSV(96, default_saturation, 255);
          int full_total = full_rgb.r + full_rgb.g + full_rgb.b;

          int led = 0;
          for (int y = 0; y < max_y+1; y++) {
            for(int x = 0; x < MATRIX_WIDTH; x++) {
              int temp_total = temp_canvas[led].r + temp_canvas[led].g + temp_canvas[led].b;
              
              if (temp_total > 0) {
                CRGB rgb = CHSV(96,default_saturation,b);
                nblend(leds[led], rgb, temp_total*255/full_total);
                temp_canvas[led].r = 0;
                temp_canvas[led].g = 0;
                temp_canvas[led].b = 0;
              }

              led++;
            }
          }

      }

    }



    //buffer our water effect, we won't update the whole screen on every frame (too slow)
    CRGB water_canvas[NUM_LEDS];
    void draw_water() {

      int z = loop_time/8;

      //partial update, screen is fully updated every four frames
      //0,1 = even lines
      //2,3 = odd lines
      //0,2 = bottom half
      //1,3 = top half
      static int screen_section = 0;
      screen_section++;
      screen_section %=4;
      int y_start = (screen_section%2) * MATRIX_HEIGHT/2;

      for (int y = y_start; y < y_start + MATRIX_HEIGHT/2; y++) {
        if ( y%2 == screen_section/2 ) {
          for (int x = 0; x < MATRIX_WIDTH; x++) {
            uint8_t light = inoise8(x*4+z,y*4,z)/2;
            uint8_t caustics = inoise8(((x-MATRIX_WIDTH/2)*((y+16)))/6,(y*(y+8))/6,z*2);
            uint8_t sat = 255;
            uint8_t bri = light;
            
            //convert to 0-127-0
            if (caustics > 127) {
              caustics -= 128;
              caustics = 127-caustics;
            }
            caustics *= 2;
            caustics = (caustics*caustics)/256;
            caustics = _max(caustics-y,0);
            caustics /= 8;
            bri = _min(light+caustics,255);
            int i = XY(x,y);
            water_canvas[i] = CHSV(142, 255, bri);
          }
        }
      }
      
      for (int i = 0; i < NUM_LEDS; i++) {
          nblend(leds[i], water_canvas[i], 127);
      }

    }



};


LIGHT_SKETCHES::REGISTER<CURVY> curvy("curvy");
//END CURVY