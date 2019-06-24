//METABALLS
#define NUM_METABALLS 30
#define METABALLS_QUARTER_RES

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
    uint8_t current_variation = 0;
    int launch_speed = 0;

    
    uint8_t num_balls = NUM_METABALLS;
    
    struct BALL {
      int32_t x = 0;
      int32_t y = 0;
      int vx = 0;
      int vy = 0;
      int r = 1;
      int32_t m = 1;
    };

    BALL balls[NUM_METABALLS];
    
    void lava_lamp_setup() {
      num_balls = 15;
      for (int i = 0; i < num_balls; i++) {
        balls[i].x = random(2*255, (METABALL_MATRIX_WIDTH-3)*255);
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
        balls[i].r = random(8*256,13*256);   
        //balls[i].r = 30*256-1;        
        balls[i].m = PI_m(sq(balls[i].r));
      }
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
        num_balls = 15;
      }
      
    }

    void setup() {
      lava_lamp_setup();
      
      long td = 0;
      long tv = 0;
      while (td < METABALL_MATRIX_HEIGHT*256L) {
        td += tv;
        tv += 32;
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
            height_map[x][y] = 0;
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
              int32_t d = sq(xd) + sq(yd);
              //combined radius
              int32_t d2 = sq(balls[i].r/2) + sq(balls[j].r/2);
              
              //check to see if distance is less than radius
              if( d < d2 && d2 != 0 ) {
                d = sqrt(d);
                d2 = sqrt(d2);
                
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
            balls[i].vy -= (8*dt)/16;

            //spray balls up
            if (balls[i].y < -5*255 || balls[i].x < -5*255 || balls[i].x > (METABALL_MATRIX_WIDTH+5)*255L) {
              balls[i].y = -2*255;
              balls[i].x = METABALL_MATRIX_WIDTH*128;
              balls[i].vy = random(launch_speed/4, launch_speed/2);
              balls[i].vx = random(100) - 50;
              balls[i].r = random(7*255,8*255);
            }
            
          }
        }




        //draw balls
        for (int i = 0; i < num_balls; i++) {
          //draw all balls on the temporary canvas (apparently this is called an isosurface)
          //find the distance of all LEDs within the bounding square of each ball

          
          
          for (int x = _max((balls[i].x - balls[i].r)/256,0); x < _min((balls[i].x + balls[i].r)/256+1,METABALL_MATRIX_WIDTH+1); x++) {
            for (int y = _max((balls[i].y - balls[i].r)/256,0); y < _min((balls[i].y + balls[i].r)/256+1,METABALL_MATRIX_HEIGHT+1); y++) {
              if (x >= 0 && x < METABALL_HEIGHTMAP_WIDTH && y >= 0 && y < METABALL_HEIGHTMAP_HEIGHT) {
                int d = sq(x*256L - balls[i].x)+sq(y*256L - balls[i].y);
                //make sure the distance is within our radius
                if (d <= sq(balls[i].r)) {
                  //led radius
                  d = sqrt(d);

                  int d2;
                  
                  //add threshold information to canvas using inverse square
                  if ( x < METABALL_MATRIX_WIDTH && y < METABALL_MATRIX_HEIGHT ) {
                    //convert to byte 0-255
                    d2 = (d*256)/balls[i].r;
                    //inverse (farther away is dimmer), this is linear
                    d2 = 255-d2;
                    //square
                    d2 = (d2*d2)/256;
                    //add to temporary canvas
                    int led = XY(x, y);
                    int t = temp_canvas[led].r + d2;
                    if (t <= 255) {
                      temp_canvas[led].r = t;
                    } else {
                      temp_canvas[led].r = 255;
                    }
                    //temp_canvas[XY(x, y)].r = _min(temp_canvas[XY(x, y)].r + d2,255); //slower?
                  }

                  //add heights to heightmap using cosine
                  d2 = (d*128)/balls[i].r;
                  d2 = cos8(d2);
                  height_map[x][y] += d2;
                  
                }
              
              }
              
            }
          }


          
        }





        /*
        //move our isosurface to the LEDs
        //using a threshold value
        //using some of the shading from the isosurface for antialiasing
        for(int i = 0; i < NUM_LEDS; i++) {
          if(temp_canvas[i].r > 96) {
            //leds[i].r = _max(_min((temp_canvas[i].r - 128)*2, 255), 0);
            leds[i] = CHSV(0,255, _max(_min((temp_canvas[i].r - 96)*3, 255), 0) );
          }
          
          temp_canvas[i] = CRGB::Black;
        }
        */
        static uint8_t light_x = 0;
        static uint8_t light_y = 0;
        
        light_x++;
        light_y++;
        

        
        
        //scale our heightmap
        for (int x = 0; x < METABALL_HEIGHTMAP_WIDTH; x++) {
          for (int y = 0; y < METABALL_HEIGHTMAP_HEIGHT; y++) {
            //use an approximate logarithmic scale
            if (height_map[x][y] > 255) {
              int scale = (height_map[x][y]*100) / 256;
              int val = height_map[x][y] - 255;
              height_map[x][y] = 255 + (val*100)/scale;  
            }
            //multiply scale
            height_map[x][y] *= 32;  
          }
        }
      
        //move heightmap to LEDs
        height_map_to_LED(96*64+1, 0, -130);

        
        
        //add opacity values to LEDs (for soft edges)
        for (int x = 0; x < METABALL_MATRIX_WIDTH; x++) {
          for (int y = 0; y < METABALL_MATRIX_HEIGHT; y++) {
            int i = XY(x,y);
            uint8_t opacity = 0;
            if (temp_canvas[i].r > 96) {
              opacity = _max(_min((temp_canvas[i].r - 96)*6, 255), 0);
              led_mask2[i] = opacity;
              height_map[x+1][y+1] = (height_map[x+1][y+1]*opacity)/255;
              leds[i].nscale8(opacity);
            } else {
              leds[i] = CRGB::Black;
              height_map[x+1][y+1] = 0;
              led_mask2[i]=0;
            }
            temp_canvas[i] = CRGB::Black;
          }    
        }

      //uint32_t debug_time2 = micros();
        #ifdef METABALLS_QUARTER_RES
        for (int y = MATRIX_HEIGHT-1; y >= 0; y--) {
          for (int x = MATRIX_WIDTH-1; x >= 0; x--) {
            
            int led = XY(x,y);


            //
            //  c      d
            //   3  4
            //  a1  2  b
            //

            uint16_t red = 0; 
            uint16_t green = 0;
            uint16_t blue = 0;


            int a = XY(x>>1,y>>1);
            red += leds[a].r;
            green += leds[a].g;
            blue += leds[a].b;
            uint8_t total = 1;

            if (x%2) {
              
              int b = XY((x+1)>>1,y>>1);
              red += leds[b].r;
              green += leds[b].g;
              blue += leds[b].b;
              total++;
            }

            if (y%2) {

              int c = XY(x>>1,(y+1)>>1);
              red += leds[c].r;
              green += leds[c].g;
              blue += leds[c].b;
              total++;

              if (x%2) {

                int d = XY((x+1)>>1,(y+1)>>1);
                red += leds[d].r;
                green += leds[d].g;
                blue += leds[d].b;
                total++;

              }
            }

            leds[led].r = red/total;
            leds[led].g = green/total;
            leds[led].b = blue/total;

          }
        }
        /* 
        //move leds to fill the grid
        for (int y = METABALL_MATRIX_HEIGHT-0; y >=0; y--) {
          for (int x = METABALL_MATRIX_WIDTH-1; x >= 0; x--) {
            int red = 0;
            int green = 0;
            int blue = 0;

            int led = XY(x*2,y*2);

            int a = XY(x,y);

            red += leds[a].r;
            green += leds[a].g;
            blue += leds[a].b;
  
            leds[led].r = red;
            leds[led].g = green;
            leds[led].b = blue;
          }
        }

        
        //fill in vertical and horizontal
        for (int y = 0; y <= MATRIX_HEIGHT-1; y++) {
          for (int x = 0; x <= MATRIX_HEIGHT-1; x++) {
            if (!(x%2 == y%2)) {
              int red = 0;
              int green = 0;
              int blue = 0;

              int led = XY(x,y);

              int a = -1;
              int b = -1;

              if (y%2) {
                a = XY(x,y+1);
                b = XY(x,y-1);  
              }

              if (x%2) {
                a = XY(x+1,y);
                b = XY(x-1,y);
              }

              red = leds[a].r;
              green = leds[a].g;
              blue = leds[a].b;

              red += leds[b].r;
              green += leds[b].g;
              blue += leds[b].b;
              
              leds[led].r = red>>1;
              leds[led].g = green>>1;
              leds[led].b = blue>>1;
            }
          }
        }

        //fill in the rest
        for (int y = 1; y <= MATRIX_HEIGHT-2; y+=2) {
          for (int x = 1; x <= MATRIX_HEIGHT-2; x+=2) {
            int red = 0;
            int green = 0;
            int blue = 0;

            int led = XY(x,y);

            int a = XY(x+1,y);
            int b = XY(x-1,y);
            int c = XY(x,y+1);
            int d = XY(x,y-1);

            red += leds[a].r;
            green += leds[a].g;
            blue += leds[a].b;

            red += leds[b].r;
            green += leds[b].g;
            blue += leds[b].b;

            red += leds[c].r;
            green += leds[c].g;
            blue += leds[c].b;

            red += leds[d].r;
            green += leds[d].g;
            blue += leds[d].b;

            leds[led].r = red>>2;
            leds[led].g = green>>2;
            leds[led].b = blue>>2;
          }
        }
        */
        #endif
      //debug_micros1 += micros() - debug_time2;
        
        
      }
    }
};

LIGHT_SKETCHES::REGISTER<METABALLS> metaballs("metaballs");