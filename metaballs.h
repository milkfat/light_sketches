#ifndef LIGHTS_METABALLS_H
#define LIGHTS_METABALLS_H

//METABALLS
#define NUM_METABALLS 100
//#define METABALLS_QUARTER_RES

#ifdef METABALLS_QUARTER_RES
#define METABALL_MATRIX_HEIGHT (MATRIX_HEIGHT/2)
#define METABALL_MATRIX_WIDTH (MATRIX_WIDTH/2)
#define METABALL_HEIGHTMAP_WIDTH (HEIGHTMAP_WIDTH/2)
#define METABALL_HEIGHTMAP_HEIGHT (HEIGHTMAP_HEIGHT/2)
#else
#define METABALL_MATRIX_HEIGHT MATRIX_HEIGHT
#define METABALL_MATRIX_WIDTH MATRIX_WIDTH
#define METABALL_HEIGHTMAP_WIDTH HEIGHTMAP_WIDTH
#define METABALL_HEIGHTMAP_HEIGHT HEIGHTMAP_HEIGHT
#endif

class METABALLS: public LIGHT_SKETCH {
  public:
        METABALLS () {setup();}
    ~METABALLS () {}
        private:

  private:
    CRGB temp_led[NUM_LEDS+1];
    uint8_t current_variation = 0;
    int launch_speed = 0;
    int16_t height_map[HEIGHTMAP_HEIGHT][HEIGHTMAP_WIDTH];

    
    uint8_t num_balls = NUM_METABALLS;
    
    struct BALL {
      int32_t x = 0;
      int32_t y = 0;
      int vx = 0;
      int vy = 0;
      uint16_t r = 1;
      uint32_t r2 = 1;
      int32_t m = 1;
    };

    BALL balls[NUM_METABALLS];
    
    void lava_lamp_setup() {
      for (int i = 0; i < num_balls; i++) {
        balls[i].x = METABALL_MATRIX_WIDTH*128;
        balls[i].y = random(10*255, (METABALL_MATRIX_HEIGHT-11)*255);
        balls[i].vx = random(3);
        if (random(2) == 0) {
          balls[i].vx += 1;
          balls[i].vx *= -1;
        }
        balls[i].vy = random(20);
        if (random(2) == 0) {
          balls[i].vy += 5;
          balls[i].vy *= -1;
        }
        balls[i].r = random(15*256,20*256); 
        if (i > 15) {
          balls[i].r = 4*256;
        }
        balls[i].r2 = balls[i].r*balls[i].r;   
        //balls[i].r = 30*256-1;        
        balls[i].m = PI_m(balls[i].r2);
      }
      num_balls = 15;
    }
    uint32_t frame_time = millis();

  public:
    void next_effect() {
      current_variation++;
      current_variation %= 2;
      
      if (current_variation == 0) {
        lava_lamp_setup();
      }

      if (current_variation == 1) {
        num_balls = 25;
      }
      
    }

    void setup() {
      height_map_ptr = &height_map;
      lava_lamp_setup();
      
      long td = 0;
      long tv = 0;
      while (td < METABALL_MATRIX_HEIGHT*256L) {
        td += tv;
        tv += 16;
      }
      launch_speed = tv;
      
    }

    void reset() {

    }

    void loop() {
      int dt = millis() - frame_time;

      if (dt > 15) {
        frame_time = millis();
        LED_show();
        LED_black();
        reset_heightmap();

        
        int max_touch = -1;
        for (uint8_t i = 0; i < NUM_POINTERS; i++) {
          if (pointers[i].down && pointers[i].y > max_touch) {
            max_touch = pointers[i].y;
          }
        }
        
        if (max_touch > -1) {
          long td = 0;
          long tv = 0;
          while (td < max_touch*256L) {
            td += tv;
            tv += 32;
          }
          launch_speed = tv;
          
        }

        //reset our height map
        for (int x = 0; x < METABALL_HEIGHTMAP_WIDTH; x++) {
          for (int y = 0; y < METABALL_HEIGHTMAP_HEIGHT; y++) {
            height_map[y][x] = 0;
          }
        }
        
        //calculate ball positions/collisions
        for (int i = 0; i < num_balls; i++) {
          
          if (current_variation == 0) {
            
            //collide with other balls
            for (int j = i; j < num_balls; j++) {
              
              //x and y distances between balls
              int16_t xd = balls[i].x - balls[j].x;
              int16_t yd = balls[i].y - balls[j].y;
    
              //distance between balls
              int16_t d = xd*xd + yd*yd;
              
              //combined radius
              int16_t ir_temp = balls[i].r/2;
              int16_t jr_temp = balls[j].r/2;
              int16_t d2 = ir_temp*ir_temp + jr_temp*jr_temp;
              
              //check to see if distance is less than radius
              if( d < d2 && d2 != 0 ) {
                d = sqrt16(d);
                d2 = sqrt16(d2);
                
                //closeness of balls as a byte (0-255)
                //0 = farthest
                //255 = closest
                int16_t closeness = ((d2 - d)*255)/(d2);
                
                //x fraction as a byte (0-255)
                //0 = 0%
                //255 = 100%
                if (abs(xd)+abs(yd) != 0) {
                  int16_t ax = (xd*255)/(abs(xd)+abs(yd));
                  //y fraction as a byte (0-255)
                  int16_t ay = (yd*255)/(abs(xd)+abs(yd));
      
                  //apply acceleration force to balls
                  //balls[i].vx += (closeness*ax)/(255*32);
                  //balls[i].vy += (closeness*ay)/(255*32);
                  //balls[j].vx -= (closeness*ax)/(255*32);
                  //balls[j].vy -= (closeness*ay)/(255*32);
                } 
                
              }
            }
          
    
    
            //update ball positions and collide with walls
            balls[i].x += balls[i].vx;
    
            if(balls[i].x > (METABALL_MATRIX_WIDTH+2)*255L - balls[i].r/4) {
              //balls[i].vx = -abs(balls[i].vx);
              balls[i].vx -= (balls[i].x - (METABALL_MATRIX_WIDTH*255L - balls[i].r/4)) / 50;
            }
            
            if(balls[i].x < -2*255 + balls[i].r/4) {
              //balls[i].vx = abs(balls[i].vx);
              balls[i].vx += ((0 + balls[i].r/4) - balls[i].x) / 50;
            }
            
            balls[i].y += balls[i].vy;
            
            if(balls[i].y > (METABALL_MATRIX_HEIGHT+2)*255L - balls[i].r/4) {
              //balls[i].vy = -abs(balls[i].vy);
              balls[i].vy -= (balls[i].y - (METABALL_MATRIX_HEIGHT*255L - balls[i].r/4)) / 50;
            }
            
            if(balls[i].y < -2*255 + balls[i].r/4) {
              //balls[i].vy = abs(balls[i].vy);
              balls[i].vy += ((0 + balls[i].r/4) - balls[i].y) / 50;
            }
            
          } else if (current_variation == 1) {
            
            //update ball positions
            balls[i].x += (balls[i].vx*dt)/16;
            balls[i].y += (balls[i].vy*dt)/16;
            
            //gravity
            balls[i].vy -= (dt);

            //spray balls up
            if (balls[i].y < -5*255 || balls[i].x < -5*255 || balls[i].x > (METABALL_MATRIX_WIDTH+5)*255L) {
              balls[i].y = -2*255;
              balls[i].x = METABALL_MATRIX_WIDTH*128;
              balls[i].vy = random(launch_speed/2, launch_speed);
              balls[i].vx = random(200) - 100;
              balls[i].r = random(7*255,18*255);
              balls[i].r2 = balls[i].r*balls[i].r;
            }
            
          }
        }




        //draw balls
        for (int i = 0; i < num_balls; i++) {
          //draw all balls on the temporary canvas (apparently this is called an isosurface)
          //find the distance of all LEDs within the bounding square of each ball

          uint32_t y_min = _max((balls[i].y - balls[i].r)/256,0);
          uint32_t y_max = _min((balls[i].y + balls[i].r)/256+1,METABALL_MATRIX_HEIGHT+1);
          uint32_t x_min = _max((balls[i].x - balls[i].r)/256,0);
          uint32_t x_max = _min((balls[i].x + balls[i].r)/256+1,METABALL_MATRIX_WIDTH+1);

          for (uint32_t y = y_min; y < y_max; y++) {
            for (uint32_t x = x_min; x < x_max; x++) {
              if (x >= 0 && x < METABALL_HEIGHTMAP_WIDTH && y >= 0 && y < METABALL_HEIGHTMAP_HEIGHT) {

                //bring d into range of uint16_t so that we can use FastLED's sqrt16() function
                int16_t x_temp = ((x*8) - balls[i].x/32);
                int16_t y_temp = ((y*8) - balls[i].y/32);
                uint16_t d = x_temp*x_temp+y_temp*y_temp;
                
                //make sure the distance is within our radius
                if (d <= balls[i].r2/(32*32)) {
                  
                  //led radius
                  d = sqrt16(d);
                  d*=32; 

                  uint16_t d2;
                  
                  //add threshold information to canvas using inverse square
                  if ( x < METABALL_MATRIX_WIDTH && y < METABALL_MATRIX_HEIGHT ) {
                    //convert to byte 0-255
                    d2 = (d<<8)/balls[i].r;
                    //inverse (farther away is dimmer), this is linear
                    d2 = 255-d2;
                    //square
                    d2 = (d2*d2)>>8;
                    //add to temporary canvas
                    int led = XY(x, y);
                    temp_led[led].r = _min(temp_led[led].r + d2,255);
                    
                    //temp_led[XY(x, y)].r = _min(temp_led[XY(x, y)].r + d2,255); //slower?
                  }

                  //add heights to heightmap using cosine
                  height_map[y][x] += cos8( (d*128)/balls[i].r );
                  
                }
              
              }
              
            }
          }


          
        }





        
        static uint8_t light_x = 0;
        static uint8_t light_y = 0;
        
        light_x++;
        light_y++;
        

        
        
        //scale our heightmap
        for (int y = 0; y < METABALL_HEIGHTMAP_HEIGHT; y++) {
          for (int x = 0; x < METABALL_HEIGHTMAP_WIDTH; x++) {
            //use an approximate logarithmic scale
            if (height_map[y][x] > 255) {
              int scale = (height_map[y][x]*100) / 256;
              int val = height_map[y][x] - 255;
              height_map[y][x] = 255 + (val*100)/scale;  
            }
            //multiply scale
            height_map[y][x] *= 32;  
          }
        }
      
        //move heightmap to LEDs
        height_map_to_LED(96*64+1, 0, -130);

        

      //uint32_t debug_time2 = micros();
        
        //add opacity values to LEDs (for soft edges)
        uint16_t led = 0;
        for (int y = 0; y < METABALL_MATRIX_HEIGHT; y++) {
          for (int x = 0; x < METABALL_MATRIX_WIDTH; x++) {
            uint8_t opacity = 0;
            if (temp_led[led].r > 96) {
              opacity = _max(_min((temp_led[led].r - 96)*6, 255), 0);
              //led_mask2[led] = opacity;
              height_map[y+1][x+1] = (height_map[y+1][x+1]*opacity)/255;
              color_scale(leds[led], opacity);
            } else {
              leds[led] = CRGB::Black;
              height_map[y+1][x+1] = 0;
              //led_mask2[led]=0;
            }
            temp_led[led] = CRGB::Black;
            led++;
          }    
        }
        
      //debug_micros1 += micros() - debug_time2;
        
        
      }
    }
};

LIGHT_SKETCHES::REGISTER<METABALLS> metaballs("metaballs");

#endif