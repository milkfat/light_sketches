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

    #define NUM_JELLIES 1
    #define NUM_JELLY_SEGMENTS 10
    #define NUM_TENTACLES NUM_JELLY_SEGMENTS
    #define NUM_TENTACLE_SEGMENTS 12
    struct tentacle_segment {
      int32_t x = -20*256;
      int32_t y = -150*256;
      int32_t z = 0;
      float vx = 0;
      float vy = 0;
      float vz = 0;
    };


    //POINT points[NUM_POINTS];

  public:

    void next_effect() {
    }

    void setup() {
      rotation_alpha = 0;
      rotation_beta = 90;
      rotation_gamma = 0;
      //jellyfish
      for (int i = 0; i < NUM_JELLIES; i++) {
        jellies[i].on_screen = false;
        jellies[i].live_until = 0;
      }

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
      LED_black();

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
      uint8_t ax = 0;
      uint8_t ay = 0;
      uint32_t steps = 0;
      uint8_t step = 0;
      uint8_t step2 = 32;
      uint8_t speed = 2;
      int32_t age = millis();
      int32_t live_until = millis();
      bool on_screen = false;
      tentacle_segment tentacles[NUM_TENTACLES][NUM_TENTACLE_SEGMENTS];
    };

    JELLY jellies[NUM_JELLIES];
    
    void handle_jellies() {
      for (int i = 0; i < NUM_JELLIES; i++) {
        handle_jelly(jellies[i]);
      }
    }

    void handle_jelly(JELLY& jelly) {

      //calculate step size
      int this_step = (jelly.speed * (loop_time-jelly.age))/16;
      
      //update jelly step
      jelly.steps++;
      jelly.step += this_step;  
      jelly.step2 += this_step;   

      //double speed to give the jelly some "snap" on its down stroke
      if (jelly.step > 32 && jelly.step < 164) {
        jelly.step+=this_step;
      }
      if (jelly.step2 > 32 && jelly.step2 < 164) {
        jelly.step2+=this_step;
      }
      

      //jelly velocity is based on a sine wave (same as the animation)
      int jelly_velocity =  (sin8(jelly.step-32)-64);
      //limit velocity to positive numbers (no going backwards)
      jelly_velocity = _max(jelly_velocity,0)/2;
      //adjust velocity for animation speed
      jelly_velocity = (jelly_velocity*jelly.speed)/4;
      //adjust velocity for framerate
      jelly_velocity = ( jelly_velocity * (loop_time-jelly.age) ) /16;

      jelly.age = millis(); 

      //reset the jelly if it goes off the screen
      if (!jelly.on_screen && loop_time > jelly.live_until) {
        
        jelly.live_until = millis()+10000; //live at least 10 seconds
        jelly.steps = 0;
        jelly.step = 0;
        jelly.step2 = 32;

        //reset to a position that is just off the side of the screen
        long p[3];
        p[0] = MATRIX_WIDTH*256;
        p[1] = random(MATRIX_HEIGHT*160) + MATRIX_HEIGHT*32;
        p[2] = random(400);
        p[2] = (p[2]*p[2])/400;
        p[2] = -p[2];
        p[2] *= 256;
        matrix.reverse_perspective(p); //translate screen coordinates to 3D coordinates
        p[0] += 70*48+256*16; //adjust for the size of the jelly (push it off the screen)
        jelly.x = p[0]; 
        jelly.y = p[1];
        jelly.z = p[2];
        jelly.az = 8+random(30);
        jelly.ax = random(20)-10;
        
        //random side of the screen
        if (random(2)) {
          jelly.x *= -1;
          jelly.az *= -1;
        }

        //random z direction
        if (random(2)) {
          jelly.ax *= -1;
        }

        //random speed
        jelly.speed = random(2,6);

        //jelly.x = 0;        //debug
        // jelly.y = -50*256;  //debug
        // jelly.z = 100*256;  //debug
        // jelly.az = 0;       //debug
        // jelly.speed = 2;    //debug

        //reset tentacles
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

      }//end jelly reset

      jelly.on_screen = false;

      long jelly_lines[NUM_JELLY_SEGMENTS/2][5][2];
      
      long ring_points[NUM_JELLY_SEGMENTS][2];

      //a jellyfish consists of a segment rotated around an axis (the center of the jellyfish)
      //each segment is made up of three points
      long jelly_points[3][3] = {
        {256,75*84,0},
        {70*48,50*84,0},
        {100*48,0,0}
      };

      //animate midpoint
      long jp1x = jelly_points[1][0] + sin8(jelly.step2)*12;
      long jp1y = jelly_points[1][1] + cos8(jelly.step2)*8;

      //animate tips
      long jp2x = jelly_points[2][0] + sin8(jelly.step)*18;
      long jp2y = jelly_points[2][1] + cos8(jelly.step)*12;

      //process each segment of the jellyfish, like slices of pizza
      for (int i = 0; i < NUM_JELLY_SEGMENTS; i++) {
        uint8_t ang = (i*256/NUM_JELLY_SEGMENTS)+NUM_JELLY_SEGMENTS + ( (fmix32(i) % 16) - 8);
        uint8_t s = sin8(ang);
        uint8_t c = cos8(ang);

        int y_r = ( (fmix32(i) % 256) - 128 );
        int y_r2 = ( (fmix32(i) % 512) - 256 );
        int x_r = ( (fmix32(i+NUM_JELLY_SEGMENTS) % 256) - 128 );
        int x_r2 = ( (fmix32(i+NUM_JELLY_SEGMENTS) % 512) - 256 );

        long p0[3];
        long p1[3];
        long p2[3];

        //build the jellyfish by rotating around y-axis:
        //rotate x coord
        p0[0] = ( jelly_points[0][0]*( c - 128 ) - jelly_points[0][2]*( s - 128 )  )/128;
        p1[0] = ( (jp1x+x_r)*( c - 128 ) - jelly_points[1][2]*( s - 128 )  )/128;
        p2[0] = ( (jp2x+x_r2)*( c - 128 ) - jelly_points[2][2]*( s - 128 )  )/128;
        
        //don't rotate y coords
        p0[1] = jelly_points[0][1];
        p1[1] = jp1y + y_r;
        p2[1] = jp2y + y_r2;

        //rotate z coords
        p0[2] = ( jelly_points[0][0]*( s - 128 ) + jelly_points[0][2]*( c - 128 )  )/128;
        p1[2] = ( (jp1x+x_r)*( s - 128 ) + jelly_points[1][2]*( c - 128 )  )/128;
        p2[2] = ( (jp2x+x_r2)*( s - 128 ) + jelly_points[2][2]*( c - 128 )  )/128;

        //rotate the jellyfish to particular orientation
        matrix.rotate_y(p0,jelly.ay);
        matrix.rotate_y(p1,jelly.ay);
        matrix.rotate_y(p2,jelly.ay);

        //rotate the jellyfish to particular orientation
        matrix.rotate_z(p0,jelly.az);
        matrix.rotate_z(p1,jelly.az);
        matrix.rotate_z(p2,jelly.az);

        //rotate the jellyfish to particular orientation
        matrix.rotate_x(p0,jelly.ax);
        matrix.rotate_x(p1,jelly.ax);
        matrix.rotate_x(p2,jelly.ax);

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

        //update the tentacles

        jelly.tentacles[i][0].x = p1[0];
        jelly.tentacles[i][0].y = p1[1];
        jelly.tentacles[i][0].z = p1[2];

        jelly.tentacles[i][1].x = p2[0];
        jelly.tentacles[i][1].y = p2[1];
        jelly.tentacles[i][1].z = p2[2];

        //rotate the jellyfish as part of our global 3d matrix
        matrix.rotate(p0);
        matrix.rotate(p1);
        matrix.rotate(p2);

        //some sort of screen scaling
        matrix.scale_z(p0);
        matrix.scale_z(p1);
        matrix.scale_z(p2);

        //map our 3d coordinates to screen coordinates
        matrix.perspective(p0);
        matrix.perspective(p1);
        matrix.perspective(p2);

        if (!jelly.on_screen) {
          if (p0[0] >= 0 && p0[0] < MATRIX_WIDTH*256 && p0[1] >= 0 && p0[1] < MATRIX_HEIGHT*256) {
            jelly.on_screen = true;
          }

          if (p1[0] >= 0 && p1[0] < MATRIX_WIDTH*256 && p1[1] >= 0 && p1[1] < MATRIX_HEIGHT*256) {
            jelly.on_screen = true;
          }

          if (p2[0] >= 0 && p2[0] < MATRIX_WIDTH*256 && p2[1] >= 0 && p2[1] < MATRIX_HEIGHT*256) {
            jelly.on_screen = true;
          }
        }
        
        if (i < 5) {
          jelly_lines[i][2][0] = p0[0];
          jelly_lines[i][2][1] = p0[1];
          jelly_lines[i][1][0] = p1[0];
          jelly_lines[i][1][1] = p1[1];
          jelly_lines[i][0][0] = p2[0];
          jelly_lines[i][0][1] = p2[1];
         } else {
          jelly_lines[i-5][3][0] = p1[0];
          jelly_lines[i-5][3][1] = p1[1];
          jelly_lines[i-5][4][0] = p2[0];
          jelly_lines[i-5][4][1] = p2[1];
         }


        //add the outer point of each segment to an array
        //we use this array to draw a circle around the jelly at the end
        ring_points[i][0] = p2[0];
        ring_points[i][1] = p2[1];
        
      }

        int bri = jelly.z/700+150;
        bri = _max(_min(bri,255),0);
     for (int i = 0; i < NUM_JELLY_SEGMENTS/2; i++) {
        //draw each segment
        matt_curve8(jelly_lines[i], 5,212,80,bri,false,false,true,255,128);
     }

      //draw a circle tying the jelly's segments together
      matt_curve8(ring_points,NUM_JELLY_SEGMENTS,212,80,bri,false,true,true,255,128);
      

      //a unit vector representing the jelly's velocity
      long v[3] = {0,256,0};

      //rotate the unit vector to figure out how far the jelly should move in each direction
      matrix.rotate_z(v,jelly.az);
      matrix.rotate_x(v,jelly.ax);

      //update the jelly's position
      jelly.x += 2*(jelly_velocity*v[0])/256;
      jelly.y += 2*(jelly_velocity*v[1])/256;
      jelly.z += 2*(jelly_velocity*v[2])/256;

      if (jelly.steps % 16 == 0) {
        jelly.ay += 1;
      }

      handle_tentacles(jelly);

    

    } //handle_jelly()


    /*
    Calculating bending force:
    1) Start with points a,b,c
    2) Calculate d   [midpoint of a,c]
    3) Calculate e   [midpoint of b,d]
    4) b change = d - e
    5) a and c change = e - d
                         a (tentacles[i][j-1])
                        /|
                       / |
                      /  |
                     /   |
                    /    |
(tentacles[i][j])  b--e--d
                    \    |
                     \   |
                      \  |
                       \ |
                        \|
                         c (tentacles[i][j-1])

     */

    void handle_tentacles(JELLY& jelly) {
      tentacle_segment (*tentacles)[NUM_TENTACLE_SEGMENTS] = jelly.tentacles;
      for (int i = 0; i < NUM_JELLY_SEGMENTS; i++) {
        long tentacle_points[NUM_TENTACLE_SEGMENTS][2];
        for (int j = 0; j < NUM_TENTACLE_SEGMENTS; j++) {

          //apply physics to all but the first point (which is affixed to the jelly)
          //points pull against eachother
          if (j > 0) {
                        
            tentacle_segment* a = &tentacles[i][j-1]; //previous point
            tentacle_segment* b = &tentacles[i][j];   //this point

            // BENDING FORCE
            if (false && j < NUM_TENTACLE_SEGMENTS-1) {

              tentacle_segment temp0;
              tentacle_segment temp1;
              tentacle_segment* c = &tentacles[i][j+1]; //next point
              tentacle_segment* d = &temp0;
              tentacle_segment* e = &temp1;
              
              //a,c midpoint
              d->x = (a->x + c->x) / 2;
              d->y = (a->y + c->y) / 2;
              d->z = (a->z + c->z) / 2;
              
              //b,d midpoint -- this is our target for point b (or j)
              e->x = (b->x + d->x) / 2;
              e->y = (b->y + d->y) / 2;
              e->z = (b->z + d->z) / 2;


              int32_t dx = (d->x - e->x);
              int32_t dy = (d->y - e->y);
              int32_t dz = (d->z - e->z);

              int32_t acx = a->x - c->x;
              int32_t acy = a->x - c->x;
              int32_t acz = a->x - c->x;

              int32_t ac = sqrt( acx*acx + acy*acy + acz*acz);

              //255 = no bending force, 0 = max bending force
              ac = ac/6;
              ac = _min(_max(ac,0),255);
              //0 = no bending force, 255 = max bending force
              ac = 255 - ac;

              dx = (dx*ac)/255;
              dy = (dy*ac)/255;
              dz = (dz*ac)/255;
              
              if (j > 1) {
                b->x += dx;
                b->y += dy;
                b->z += dz;
                b->vx += dx/2;
                b->vy += dy/2;
                b->vz += dz/2;
              }

              dx *= -1;
              dy *= -1;
              dz *= -1;

              if (j > 2) {
                a->x += dx;
                a->y += dy;
                a->z += dz;
                a->vx += dx/2;
                a->vy += dy/2;
                a->vz += dz/2;
              }

              c->x += dx;
              c->y += dy;
              c->z += dz;
              c->vx += dx/2;
              c->vy += dy/2;
              c->vz += dz/2;
            
            }






            // TENSION/COMPRESSION FORCE
            int32_t dx = (a->x - b->x);
            int32_t dy = (a->y - b->y);
            int32_t dz = (a->z - b->z);

            int32_t d2 = dx*dx + dy*dy + dz*dz;

            //segments are fixed in length, any longer/shorter causes push/pull action
            if ( j > 1 && d2 != ((4*256) * (4*256)) ) {
              //distance between points
              int32_t d = sqrt(d2);
              if (d == 0) {
                d = 1;
              }
              //overage distance
              int32_t od = (d - 4*256);
              od/=16;
              //points pull equally on one-another, except for the first point
              

              //overage distance along each axis
              dx = (od*dx)/d;
              dy = (od*dy)/d;
              dz = (od*dz)/d;

                            
              //if our current speed is less than the overage distance, then speed up
              
              b->x += dx/2;
              b->y += dy/2;
              b->z += dz/2;
              b->vx += dx/2;
              b->vy += dy/2;
              b->vz += dz/2;
              

              //accelerate the other point along each axis
              if (j > 2) {
                
                //overage distance along each axis
                dx *= -1;
                dy *= -1;
                dz *= -1;
                
                //if our current speed is less than the overage distance, then speed up
                a->x += dx;
                a->y += dy;
                a->z += dz;
                a->vx += dx/2;
                a->vy += dy/2;
                a->vz += dz/2;
              
                
              }

              //gravity
              //b->vy -= 4;

              //drag
              b->vx *= .95;
              b->vy *= .95;
              b->vz *= .95;


            }

            //update position
            b->x += b->vx;
            b->y += b->vy;
            b->z += b->vz;

            
          }

          long p[3];
          p[0] = tentacles[i][j].x;
          p[1] = tentacles[i][j].y;
          p[2] = tentacles[i][j].z;

          //rotate with global matrix
          matrix.rotate(p);

          //some sort of screen scaling
          matrix.scale_z(p);

          //map our 3d coordinates to screen coordinates
          matrix.perspective(p);

          if (!jelly.on_screen) {
          if (p[0] >= 0 && p[0] < MATRIX_WIDTH*256 && p[1] >= 0 && p[1] < MATRIX_HEIGHT*256) {
            jelly.on_screen = true;
          }

        }
          if (j > 0) {
            tentacle_points[j-1][0] = p[0];
            tentacle_points[j-1][1] = p[1];
          }
        
        }

        
        int bri = jelly.z/700+150;
        bri = _max(_min(bri,255),0);
        matt_curve8(tentacle_points,NUM_TENTACLE_SEGMENTS-1,212,48,bri,false,false,true,255,255);
        //matt_curve8(tentacle_points,NUM_TENTACLE_SEGMENTS-1,96,80,160,false,false,true,255,255);

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
          bubbles[i].y += bubbles[i].r/8+16;
          bubbles[i].y += random(-48,48);
          bubbles[i].x += random(-48,48);
        }
        
      }

      for(int i = 0; i < NUM_BUBBLES; i++) {
        draw_circle_fine(bubbles[i].x, bubbles[i].y, bubbles[i].r/2+128, 0, 0, 96-(256-bubbles[i].r)/64, -1, 64);
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

        fish.add_speed *= .999f; //slow down
        int speed = fish.speed + fish.add_speed/1000; //calculate current speed

        if (fish.age > fish.new_target_age) {
          fish.new_target();
          fish.new_target_age = millis() + (15 + random(30))*1000;
        }
        
        long a_thing = (fish.x - fish.target_x)/256;
        long b_thing = (fish.z - fish.target_z)/256;
        long c_length = sqrt(a_thing*a_thing + b_thing*b_thing); //this is the distance between the target and the fish's y-axis

        //target angle around z-axis (the angle between y-axis and target)
        uint8_t target_az = -(atan2(c_length - fish.x/256, (fish.target_y - fish.y)/256)*255)/(2*PI)+64;
        //target angle around y-axis
        uint8_t target_ay = -(atan2(fish.target_x/256 - fish.x/256, (fish.target_z - fish.z)/256)*255)/(2*PI)+64;

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
          uint8_t sin_ay = sin8(fish.ay);
          uint8_t cos_ay = cos8(fish.ay);
          p[0] = ( x*( cos_ay - 128 ) - z*( sin_ay - 128 )  )/128;
          //rotate z
          p[2] = ( x*( sin_ay - 128 ) + z*( cos_ay - 128 )  )/128;
          

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
        uint8_t sin_az = sin8(fish.az);
        uint8_t cos_az = cos8(fish.az);
        long x = ( fish_points[i][0]*64*( cos_az - 128 ) - fish_points[i][1]*48*( sin_az - 128 )  )/128;
        //rotate y
        p[1] = ( fish_points[i][0]*64*( sin_az - 128 )  + fish_points[i][1]*48*( cos_az - 128 )  )/128;


        //rotate around y-axis:
        //rotate x
        uint8_t sin_ay = sin8(fish.ay);
        uint8_t cos_ay = cos8(fish.ay);
        p[0] = ( x*( cos_ay - 128 ) - z*( sin_ay - 128 )  )/128;
        //rotate z
        p[2] = ( x*( sin_ay - 128 ) + z*( cos_ay - 128 )  )/128;

        //translate fish to position        
        p[0] += fish.x;
        p[1] += fish.y;
        p[2] += fish.z;
        long p0[3];
        matrix.rotate(p, p0);

        matrix.scale_z(p0);
        
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
        
        //update the grass points
        for (int j = 0; j < NUM_POINTS; j++) {
          //use FastLED's 16-bit simplex function to generate waves for the grass
          uint16_t grass_noise = inoise16(y*2, my_points[i][j][1]*4+y*(128+speed), i << 16) >> 7;
          //update this point's X coordinate
          my_points[i][j][0] = MATRIX_WIDTH*(_min(_max(grass_noise,0),512)-256)/2;
          //grass moves less at ground level (moves more at the tips)
          my_points[i][j][0] = (my_points[i][j][0] * _min(j,NUM_POINTS)) / NUM_POINTS;
          //set the X coordinate for the entire blade of grass
          my_points[i][j][0] += offset;
        }

          //draw curve
          //swap X and Y axis if our display width is greater than the height
          uint8_t b = (i*140)/NUM_THINGS+5;
          #if MATRIX_WIDTH > MATRIX_HEIGHT
          matt_curve8(leds, my_points[i], NUM_POINTS, 96, default_saturation, b, true, false, true, b, 255);
          #else
          matt_curve8(leds, my_points[i], NUM_POINTS, 96, default_saturation, b, false, false, true, b, 255);
          #endif

          
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
            bri = (bri*3)/4;
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