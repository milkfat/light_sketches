#ifndef LIGHTS_CURVY_H
#define LIGHTS_CURVY_H
#include "triangle.h"

//CURVY

#if MATRIX_WIDTH > MATRIX_HEIGHT
#define GRID_WIDTH MATRIX_HEIGHT
#define GRID_HEIGHT MATRIX_WIDTH
#else
#define GRID_WIDTH MATRIX_WIDTH
#define GRID_HEIGHT MATRIX_HEIGHT
#endif


#define NUM_FISH 30
#define NUM_JELLIES 1

class CURVY: public LIGHT_SKETCH {
  public:
    CURVY () {setup();}
    ~CURVY () {}
#define NUM_POINTS 10
#define NUM_THINGS 10


  private:

    Z_BUF _z_buffer;
    Y_BUF2 _y_buffer2;

    #define NUM_CURVY_EFFECTS 2
    uint8_t current_effect = 0;


    uint8_t temp_led[NUM_LEDS+1];

    #define NUM_BUBBLES 20
    struct BUBBLE {
      int x=0;
      int y=MATRIX_HEIGHT*256;
      int z=0;
      int r=1;
    };

    BUBBLE bubbles[20];

    int32_t my_points[NUM_THINGS][NUM_POINTS][2];
    uint32_t loop_time = millis();
    #define FISH_POINTS 10
    VECTOR3 fish_points[FISH_POINTS] =
    {
      //{-32,0}, //tail center
      {-72,32,0}, //tail tip bottom
      {-16,0,0}, //tail meets body
      {32,-32,0}, //body bottom
      {64,0,0}, //nose
      {32,32,0}, //body top
      {-16,0,0}, //tail meets body
      {-64,-32,0}, //tail tip bottom
      {24, 0, -12}, //body left
      {24, 0, 12}, //body right
      {-40,0, 0} //tail center
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
      int32_t target_x = 0;
      int32_t target_y = 0;
      int32_t target_z = 0;
      int32_t age = millis();
      int32_t new_target_age = 0;
      uint8_t hue = 0;
      uint8_t sat = 0;

      void new_target() {
          target_z = random(-100*256, 300*256);
          int low_x = 0;
          int low_y = 0;
          int high_x = MATRIX_WIDTH*256;
          int high_y = MATRIX_HEIGHT*256;
          //std::cout << low_y << " " << high_y << " " << target_z << " = ";
          led_screen.reverse_perspective(low_x,low_y,target_z);
          led_screen.reverse_perspective(high_x,high_y,target_z);

          //std::cout << low_y << " " << high_y << " " << target_z << " result: ";
          target_x = random(high_x-low_x) + low_x;
          //target_x = 0;
          target_y = random(high_y-low_y) + low_y;
          //std::cout << target_y << "\n";
          //target_z = 0;
          add_speed = random(20000)+10000;
      }
    };
    FISH fishies[NUM_FISH];

    //some jellyfish tentacles

    #define NUM_JELLY_SEGMENTS 10
    #define NUM_TENTACLES NUM_JELLY_SEGMENTS
    #define NUM_TENTACLE_SEGMENTS 20
    struct tentacle_segment {
      VECTOR3 pos = VECTOR3(-20*256,-150*256,0);
      float vx = 0;
      float vy = 0;
      float vz = 0;
    };


    //VECTOR3 points[NUM_POINTS];

  public:

    void next_effect() {
      current_effect++;
      current_effect%=NUM_CURVY_EFFECTS;
    }

    void setup() {
      z_buffer = &_z_buffer;
      y_buffer2 = &_y_buffer2;
      led_screen.light_falloff = 9;
      led_screen.rotation_alpha = 0;
      led_screen.rotation_beta = 90;
      led_screen.rotation_gamma = 0;
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
        fishies[i].sat = random(0,256);
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
        //LED_black();
        z_buffer->reset();

        for (uint16_t i = 0; i < NUM_LEDS; i++) {
          leds[i].r = 0;
          leds[i].g = 4;
          leds[i].b = 5;
        }

        
        
        handle_jellies();
        
        //reset_y_buffer2(y_buffer2);
        
        handle_fish();
        
        handle_bubbles();

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

        while (jellies[i].age < loop_time) {
          jellies[i].age+=16;
          handle_jelly(jellies[i]);
        }

        draw_jelly(jellies[i]);
      }
    }

    void handle_jelly(JELLY& jelly) {

      //calculate step size
      int this_step = jelly.speed;
      
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
      int jelly_velocity =  (sin8(jelly.step-64)-64);
      //limit velocity to positive numbers (no going backwards)
      jelly_velocity = _max(jelly_velocity,0)/2;
      //adjust velocity for animation speed
      jelly_velocity = (jelly_velocity*jelly.speed)/4;
      //adjust velocity for framerate
      jelly_velocity = jelly_velocity;


      //reset the jelly if it goes off the screen
      if (!jelly.on_screen && loop_time > jelly.live_until) {
        
        jelly.live_until = millis()+10000; //live at least 10 seconds
        jelly.steps = 0;
        jelly.step = 0;
        jelly.step2 = 32;

        //reset to a position that is just off the side of the screen
        int32_t p[3];
        p[0] = MATRIX_WIDTH*256;
        p[1] = random(MATRIX_HEIGHT*160) + MATRIX_HEIGHT*32;
        p[2] = random(002);
        p[2] = (p[2]*p[2])/400;
        p[2] = -p[2];
        p[2] *= 256;
        led_screen.reverse_perspective(p); //translate screen coordinates to 3D coordinates
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
        jelly.speed = random(2,4);

        //jelly.x = 0;        //debug
        // jelly.y = -50*256;  //debug
        // jelly.z = 100*256;  //debug
        // jelly.az = 0;       //debug
        // jelly.speed = 2;    //debug

        //reset tentacles
        for (int i = 0; i < NUM_TENTACLES; i++) {
          for (int j = 0; j < NUM_TENTACLE_SEGMENTS; j++) {
            jelly.tentacles[i][j].pos.x = jelly.x;
            jelly.tentacles[i][j].pos.y = jelly.y;
            jelly.tentacles[i][j].pos.z = jelly.z;
            jelly.tentacles[i][j].vx = 0;
            jelly.tentacles[i][j].vy = 0;
            jelly.tentacles[i][j].vz = 0;
          }
        }

      }//end jelly reset


      //a unit vector representing the jelly's velocity
      int32_t v[3] = {0,256,0};

      //rotate the unit vector to figure out how far the jelly should move in each direction
      rotate_z(v,jelly.az);
      rotate_x(v,jelly.ax);

      //update the jelly's position
      jelly.x += 2*(jelly_velocity*v[0])/256;
      jelly.y += 2*(jelly_velocity*v[1])/256;
      jelly.z += 2*(jelly_velocity*v[2])/256;

      if (jelly.steps % 16 == 0) {
        jelly.ay += 1;
      }


      handle_tentacles(jelly);

    

    } //handle_jelly()


void draw_jelly(JELLY& jelly) {

      jelly.on_screen = false;
      VECTOR3 jelly_lines[NUM_JELLY_SEGMENTS/2][5];
      
      VECTOR3 ring_points[NUM_JELLY_SEGMENTS];

      //a jellyfish consists of a segment rotated around an axis (the center of the jellyfish)
      //each segment is made up of three points
      int32_t jelly_points[3][3] = {
        {256,75*84,0},
        {70*48,50*84,0},
        {100*48,0,0}
      };

      //animate midpoint
      int32_t jp1x = jelly_points[1][0] + sin8(jelly.step2)*12;
      int32_t jp1y = jelly_points[1][1] + cos8(jelly.step2)*8;

      //animate tips
      int32_t jp2x = jelly_points[2][0] + sin8(jelly.step)*18;
      int32_t jp2y = jelly_points[2][1] + cos8(jelly.step)*18;

      //process each segment of the jellyfish, like slices of pizza
      for (int i = 0; i < NUM_JELLY_SEGMENTS; i++) {
        uint8_t ang = (i*256/NUM_JELLY_SEGMENTS)+NUM_JELLY_SEGMENTS + ( (fmix32(i) % 16) - 8);
        uint8_t s = sin8(ang);
        uint8_t c = cos8(ang);

        int y_r = ( (fmix32(i) % 256) - 128 );
        int y_r2 = ( (fmix32(i) % 512) - 256 );
        int x_r = ( (fmix32(i+NUM_JELLY_SEGMENTS) % 256) - 128 );
        int x_r2 = ( (fmix32(i+NUM_JELLY_SEGMENTS) % 512) - 256 );

        int32_t p0[3];
        int32_t p1[3];
        int32_t p2[3];

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
        rotate_y(p0,jelly.ay);
        rotate_y(p1,jelly.ay);
        rotate_y(p2,jelly.ay);

        //rotate the jellyfish to particular orientation
        rotate_z(p0,jelly.az);
        rotate_z(p1,jelly.az);
        rotate_z(p2,jelly.az);

        //rotate the jellyfish to particular orientation
        rotate_x(p0,jelly.ax);
        rotate_x(p1,jelly.ax);
        rotate_x(p2,jelly.ax);

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

        jelly.tentacles[i][0].pos.x = p1[0];
        jelly.tentacles[i][0].pos.y = p1[1];
        jelly.tentacles[i][0].pos.z = p1[2];

        jelly.tentacles[i][1].pos.x = p2[0];
        jelly.tentacles[i][1].pos.y = p2[1];
        jelly.tentacles[i][1].pos.z = p2[2];

        //rotate the jellyfish as part of our global 3d matrix
        led_screen.matrix.rotate(p0);
        led_screen.matrix.rotate(p1);
        led_screen.matrix.rotate(p2);

        //map our 3d coordinates to screen coordinates
        led_screen.perspective(p0);
        led_screen.perspective(p1);
        led_screen.perspective(p2);

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
          jelly_lines[i][2].x = p0[0];
          jelly_lines[i][2].y = p0[1];
          jelly_lines[i][2].z = p0[2];
          jelly_lines[i][1].x = p1[0];
          jelly_lines[i][1].y = p1[1];
          jelly_lines[i][1].z = p1[2];
          jelly_lines[i][0].x = p2[0];
          jelly_lines[i][0].y = p2[1];
          jelly_lines[i][0].z = p2[2];
         } else {
          jelly_lines[i-5][3].x = p1[0];
          jelly_lines[i-5][3].y = p1[1];
          jelly_lines[i-5][3].z = p1[2];
          jelly_lines[i-5][4].x = p2[0];
          jelly_lines[i-5][4].y = p2[1];
          jelly_lines[i-5][4].z = p2[2];
         }


        //add the outer point of each segment to an array
        //we use this array to draw a circle around the jelly at the end
        ring_points[i].x = p2[0];
        ring_points[i].y = p2[1];
        ring_points[i].z = p2[2];
        
      }

        // int bri = jelly.z/1000+150;
        // bri = _max(_min(bri,255),0);
        CRGB rgb = CHSV(212,80,150);
     for (int i = 0; i < NUM_JELLY_SEGMENTS/2; i++) {
        //draw each segment
        matt_curve8(led_screen, jelly_lines[i], 5,rgb,false,false,true,255,128);
     }

      //draw a circle tying the jelly's segments together
      matt_curve8(led_screen,ring_points,NUM_JELLY_SEGMENTS,rgb,false,true,true,255,128);
      
      draw_tentacles(jelly);
}

    /*
    Calculating bending force:
    1) Start with points a,b,c
    2) Calculate d   [midpoint of a,c]
    3) Calculate e   [midpoint of b,d]
    4) change in b = d - e
    5) change in a and c = e - d
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
        for (int j = 0; j < NUM_TENTACLE_SEGMENTS; j++) {

          //apply physics to all but the first point (which is affixed to the jelly)
          //points pull against eachother
          if (j > 0) {
                        
            tentacle_segment* a = &tentacles[i][j-1]; //previous point
            tentacle_segment* b = &tentacles[i][j];   //this point

            // BENDING FORCE -- not used at the moment
            if (j < NUM_TENTACLE_SEGMENTS-1) {

              tentacle_segment temp0;
              tentacle_segment temp1;
              tentacle_segment* c = &tentacles[i][j+1]; //next point
              tentacle_segment* d = &temp0;
              tentacle_segment* e = &temp1;
              
              //a,c midpoint
              d->pos.x = (a->pos.x + c->pos.x) / 2;
              d->pos.y = (a->pos.y + c->pos.y) / 2;
              d->pos.z = (a->pos.z + c->pos.z) / 2;
              
              //b,d midpoint -- this is our target for point b (or j)
              e->pos.x = (b->pos.x + d->pos.x) / 2;
              e->pos.y = (b->pos.y + d->pos.y) / 2;
              e->pos.z = (b->pos.z + d->pos.z) / 2;


              int32_t dx = (d->pos.x - e->pos.x);
              int32_t dy = (d->pos.y - e->pos.y);
              int32_t dz = (d->pos.z - e->pos.z);

              int32_t acx = a->pos.x - c->pos.x;
              int32_t acy = a->pos.y - c->pos.y;
              int32_t acz = a->pos.z - c->pos.z;

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
                b->pos.x += dx;
                b->pos.y += dy;
                b->pos.z += dz;
                b->vx += dx/2;
                b->vy += dy/2;
                b->vz += dz/2;
              }

              dx = -dx;
              dy = -dy;
              dz = -dz;

              if (j > 2) {
                a->pos.x += dx;
                a->pos.y += dy;
                a->pos.z += dz;
                a->vx += dx/2;
                a->vy += dy/2;
                a->vz += dz/2;
              }

              c->pos.x += dx;
              c->pos.y += dy;
              c->pos.z += dz;
              c->vx += dx/2;
              c->vy += dy/2;
              c->vz += dz/2;
            
            }






            // TENSION/COMPRESSION FORCE
            int32_t dx = (a->pos.x - b->pos.x);
            int32_t dy = (a->pos.y - b->pos.y);
            int32_t dz = (a->pos.z - b->pos.z);

            int32_t d2 = dx*dx + dy*dy + dz*dz;

            //segments are fixed in length, any longer/shorter causes push/pull action
            if ( j > 1 && d2 != ((2*300) * (2*300)) ) {
              //distance between points
              int32_t d = sqrt(d2);
              if (d == 0) {
                d = 1;
              }
              //overage distance
              int32_t od = (d - 2*300);
              od/=16;
              //points pull equally on one-another, except for the first point
              

              //overage distance along each axis
              dx = (od*dx)/d;
              dy = (od*dy)/d;
              dz = (od*dz)/d;

                            
              //if our current speed is less than the overage distance, then speed up
              
              b->pos.x += dx/2;
              b->pos.y += dy/2;
              b->pos.z += dz/2;
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
                a->pos.x += dx;
                a->pos.y += dy;
                a->pos.z += dz;
                a->vx += dx/2;
                a->vy += dy/2;
                a->vz += dz/2;
              
                
              }

              //gravity
              //b->vy -= 4;

              //drag
              b->vx = (b->vx*98)/100;
              b->vy = (b->vy*98)/100;
              b->vz = (b->vz*98)/100;


            }

            //update position
            b->pos.x += b->vx;
            b->pos.y += b->vy;
            b->pos.z += b->vz;

            
          }

        }
      }
    }
    
    inline bool dt(VECTOR3& a, VECTOR3& b, VECTOR3& c, VECTOR3& norm_a, VECTOR3& norm_b, VECTOR3& norm_c, const CRGB& rgb = CRGB(255,255,255)) {
      return (current_effect) ? draw_triangle_fine(a,b,c,norm_a,norm_b,norm_c,rgb) : draw_triangle(a,b,c,norm_a,norm_b,norm_c,rgb);
        
    }

    void draw_tentacles(JELLY& jelly) {
      tentacle_segment (*tentacles)[NUM_TENTACLE_SEGMENTS] = jelly.tentacles;
      for (int i = 0; i < NUM_JELLY_SEGMENTS; i++) {
        VECTOR3 tentacle_points[NUM_TENTACLE_SEGMENTS];
        for (int j = 0; j < NUM_TENTACLE_SEGMENTS; j++) {

          VECTOR3 p;
          p.x = tentacles[i][j].pos.x;
          p.y = tentacles[i][j].pos.y;
          p.z = tentacles[i][j].pos.z;

          //rotate with global matrix
          led_screen.matrix.rotate(p);

          //map our 3d coordinates to screen coordinates
          led_screen.perspective(p);

          if (!jelly.on_screen) {
          if (p.x >= 0 && p.x < MATRIX_WIDTH*256 && p.y >= 0 && p.y < MATRIX_HEIGHT*256) {
            jelly.on_screen = true;
          }

        }
          if (j > 0) {
            tentacle_points[j-1].x = p.x;
            tentacle_points[j-1].y = p.y;
            tentacle_points[j-1].z = p.z;
          }
        
        }

        
        // int bri = jelly.z/1000+150;
        // bri = _max(_min(bri,255),0);
        CRGB rgb = CHSV(212,48,150);
        matt_curve8(led_screen,tentacle_points,NUM_TENTACLE_SEGMENTS-1,rgb,false,false,true,255,255);
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
        draw_circle_fine_hsv(bubbles[i].x, bubbles[i].y, bubbles[i].r/2+128, 0, 0, 40-(256-bubbles[i].r)/32, -1, 64);
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
        
        int32_t a_thing = (fish.x - fish.target_x)/256;
        int32_t b_thing = (fish.z - fish.target_z)/256;
        int32_t c_length = sqrt(a_thing*a_thing + b_thing*b_thing); //this is the distance between the target and the fish's y-axis

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
          int32_t p[3];

          int32_t z = 0;

          //rotate around z-axis:
          //rotate x
          int32_t x = ( 16*( cos8(fish.az) - 128 ) )/128;
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

      VECTOR3 points_3d[FISH_POINTS];
      VECTOR3 points_2d[FISH_POINTS];
      int32_t detail_z = 0;
      int32_t on_screen = false;
      for (int i = 0; i < FISH_POINTS; i++) {
        VECTOR3* p = &points_3d[i];

        //scale z to add wiggly swimming motion
        p->x = fish_points[i].x*64;
        p->y = fish_points[i].y*48;
        p->z = fish_points[i].z*64 + (inoise8(fish_points[i].x*2+fish.wiggle*8, 0, 0)-128)*16;
        //rotate around z-axis:
        rotate_z(*p, fish.az);

        //rotate around y-axis:
        rotate_y(*p, fish.ay);

        //translate fish to position        
        p->x += fish.x;
        p->y += fish.y;
        p->z += fish.z;
        p->z = -p->z;
        led_screen.matrix.rotate(*p);

        //scale_z(*p);
        
        detail_z = p->z;

        led_screen.perspective(*p,points_2d[i]);

        //check to make sure at least one point is located on the screen
        if (!on_screen && points_2d[i].x > 0 && points_2d[i].x < MATRIX_WIDTH*256 && points_2d[i].y > 0 && points_2d[i].y < MATRIX_HEIGHT*256) {
          on_screen = true;
        }

      }

      //draw the fish if it is on the screen
      if (on_screen) {

        uint8_t a = 0; //tail bottom
        uint8_t b = 1; //body meets tail
        uint8_t c = 2; //body bottom
        uint8_t d = 3; //nose
        uint8_t e = 4; //body top
                       //body meets tail
        uint8_t g = 6; //tail top
        uint8_t h = 7; //body left
        uint8_t i = 8; //body right
        uint8_t j = 9; //tail center

        uint8_t detail = 255;
        //lower the level of detail when farther away
        if (detail_z > -150*256) {
          detail = 128;
        }
        // int bri = detail_z/512+255;
        // bri = _max(_min(bri,255),0);
        uint8_t bri = 222;

        //draw_triangle(points_2d[a],points_2d[b],points_2d[j],fish.hue,fish.sat,bri);
        //draw_triangle(points_2d[b],points_2d[g],points_2d[j],fish.hue,fish.sat,bri);
        VECTOR3 right(0,0,-255);
        rotate_z(right, fish.az);
        rotate_y(right, fish.ay);
        led_screen.matrix.rotate(right);
        rotate_x(right,-24);
        VECTOR3 left(0,0,255);
        rotate_z(left, fish.az);
        rotate_y(left, fish.ay);
        led_screen.matrix.rotate(left);
        rotate_x(left,-24);
        VECTOR3 up(0,-255,0);
        rotate_z(up, fish.az);
        rotate_y(up, fish.ay);
        led_screen.matrix.rotate(up);
        rotate_x(up,-24);
        VECTOR3 down(0,255,0);
        rotate_z(down, fish.az);
        rotate_y(down, fish.ay);
        led_screen.matrix.rotate(down);
        rotate_x(down,-24);
        VECTOR3 front(-255,0,0);
        rotate_z(front, fish.az);
        rotate_y(front, fish.ay);
        led_screen.matrix.rotate(front);
        rotate_x(front,-24);
        VECTOR3 back(255,0,0);
        rotate_z(back, fish.az);
        rotate_y(back, fish.ay);
        led_screen.matrix.rotate(back);
        rotate_x(right,-24);

        CRGB rgb = CHSV(fish.hue,fish.sat,bri);

        VECTOR3 tnorm = normal(points_2d[a],points_2d[b],points_2d[j]);
        VECTOR3 tnorm2 = normal(points_2d[b],points_2d[g],points_2d[j]);
        VECTOR3 atnorm = tnorm*-1;
        VECTOR3 atnorm2 = tnorm2*-1;

        //if (points_2d[a].z > -200*256) {

          dt( points_2d[a],points_2d[b],points_2d[j],tnorm,right,right,rgb );
          dt( points_2d[b],points_2d[g],points_2d[j],right,tnorm2,right,rgb );
          dt( points_2d[b],points_2d[a],points_2d[j],left,atnorm,left,rgb );
          dt( points_2d[g],points_2d[b],points_2d[j],atnorm2,left,left,rgb );
          
          dt(points_2d[c],points_2d[b],points_2d[i],down,back,right,rgb);
          dt(points_2d[d],points_2d[c],points_2d[i],front,down,right,rgb);
          dt(points_2d[e],points_2d[d],points_2d[i],up,front,right,rgb);
          dt(points_2d[b],points_2d[e],points_2d[i],back,up,right,rgb);

          dt(points_2d[b],points_2d[c],points_2d[h],back,down,left,rgb);
          dt(points_2d[c],points_2d[d],points_2d[h],down,front,left,rgb);
          dt(points_2d[d],points_2d[e],points_2d[h],front,up,left,rgb);
          dt(points_2d[e],points_2d[b],points_2d[h],up,back,left,rgb);

        // } else {

        //   draw_triangle_flat(points_2d[a],points_2d[b],points_2d[j],fish.hue,fish.sat,bri,true);
        //   draw_triangle_flat(points_2d[b],points_2d[g],points_2d[j],fish.hue,fish.sat,bri,true);

        //   draw_triangle_flat(points_2d[b],points_2d[c],points_2d[h],fish.hue,fish.sat,bri,false);
        //   draw_triangle_flat(points_2d[c],points_2d[d],points_2d[h],fish.hue,fish.sat,bri,false);
        //   draw_triangle_flat(points_2d[d],points_2d[e],points_2d[h],fish.hue,fish.sat,bri,false);
        //   draw_triangle_flat(points_2d[e],points_2d[b],points_2d[h],fish.hue,fish.sat,bri,false);

        //   draw_triangle_flat(points_2d[c],points_2d[b],points_2d[i],fish.hue,fish.sat,bri,false);
        //   draw_triangle_flat(points_2d[d],points_2d[c],points_2d[i],fish.hue,fish.sat,bri,false);
        //   draw_triangle_flat(points_2d[e],points_2d[d],points_2d[i],fish.hue,fish.sat,bri,false);
        //   draw_triangle_flat(points_2d[b],points_2d[e],points_2d[i],fish.hue,fish.sat,bri,false);
        
        // }

        //matt_curve8(points,FISH_POINTS,fish.hue,fish.sat,bri,false,false,true,255,detail);
      }
            // //fish debug, lines between fish and target
            // int32_t v0[3] = {fish.x,fish.y,fish.z};
            // int32_t v1[3] = {fish.target_x, fish.target_y, fish.target_z};
            // int32_t p0[3];
            // int32_t p1[3];

            // matrix.rotate(v0, p0);
            // matrix.rotate(v1, p1);

            // p0[2] += -180 * 256 + (200 * 256 * debug_scaler) / 256;
            // p1[2] += -180 * 256 + (200 * 256 * debug_scaler) / 256;

            // //correct 3d perspective
            
            // led_screen.perspective(p0);
            // led_screen.perspective(p1);

            // draw_line_fine(led_screen, p0[0], p0[1], p1[0], p1[1], 255, 255, 255, -10000, 255, true);

    } //void draw_fish(FISH& fish)





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
          uint8_t b = 96-i*8;
          #if MATRIX_WIDTH > MATRIX_HEIGHT
          matt_curve8(led_screen, my_points[i], NUM_POINTS, 96, default_saturation, b, true, false, true, b, 255);
          #else
          matt_curve8(led_screen, my_points[i], NUM_POINTS, 96, default_saturation, b, false, false, true, b, 255);
          #endif

          
      }

    } //void draw_grass()



    //buffer our water effect, we won't update the whole screen on every frame (too slow)
    //CRGB water_canvas[NUM_LEDS];
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

      for (uint16_t y = y_start; y < y_start + MATRIX_HEIGHT/2; y++) {
        if ( y%2 == screen_section/2 ) {
          for (uint16_t x = 0; x < MATRIX_WIDTH; x++) {
            uint8_t bri = inoise8((x<<2)+z,y<<2,z);
            //uint8_t caustics = inoise8(((x-MATRIX_WIDTH/2)*((y+16)))/6,(y*(y+8))/6,z*2);

            //CRGB rgb = CHSV(142, 255, 255);
            temp_led[XY(x,y)] = bri;
          }
        }
      }
//static uint32_t ttt[NUM_LEDS+1][6];
      for (uint16_t i = 0; i < NUM_LEDS; i++) {
          //nblend(leds[i], temp_led[i], 127);
          color_blend_linear16(leds[i], 0, gamma16_decode(210), 65535, gamma16_decode(temp_led[i])>>5);
          //color_blend_linear16(leds[i], 0, (gamma16_decode(temp_led[i])*8)/10, gamma16_decode(temp_led[i]), 2048);
          // ttt[i][0] = temp_led[i];
          // ttt[i][1] = (gamma16_decode(temp_led[i])*2048 + gamma16_decode(0)*(65535-2048) ) >> 16;
          // ttt[i][2] = leds[i].g;
          // ttt[i][3] = leds[i].b;
          // ttt[i][4] = (gamma16_decode(temp_led[i])*2048 + gamma16_decode(0)*(65535-2048) );
          // ttt[i][5] = gamma16_decode(temp_led[i]);
          //leds[i] += temp_led[i];
      }

    }



};


LIGHT_SKETCHES::REGISTER<CURVY> curvy("curvy");
//END CURVY

#endif