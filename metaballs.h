//METABALLS

class METABALLS: public LIGHT_SKETCH {
  public:
        METABALLS () {setup();}
    ~METABALLS () {}
        private:

  private:
    uint8_t current_variation = 0;
    int launch_speed = 0;

    #define NUM_METABALLS 30
    
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
        balls[i].x = random(2*255, (MATRIX_WIDTH-3)*255);
        balls[i].y = random(10*255, (MATRIX_HEIGHT-11)*255);
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
        balls[i].r = random(8*255,13*255)*2;        
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
      while (td < MATRIX_HEIGHT*256L) {
        td += tv;
        tv += 32;
      }
      launch_speed = tv;
      
    }

    void reset() {

    }

    void loop() {
      int dt = millis() - frame_time;
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
      for (int x = 0; x < HEIGHTMAP_WIDTH; x++) {
        for (int y = 0; y < HEIGHTMAP_HEIGHT; y++) {
          height_map[x][y] = 0;
        }
      }
      
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
  
          if(balls[i].x > (MATRIX_WIDTH+2)*255L - balls[i].r/4) {
            //balls[i].vx = -abs(balls[i].vx);
            balls[i].vx -= (balls[i].x - (MATRIX_WIDTH*255L - balls[i].r/4)) / 50;
          }
          
          if(balls[i].x < -2*255 + balls[i].r/4) {
            //balls[i].vx = abs(balls[i].vx);
            balls[i].vx += ((0 + balls[i].r/4) - balls[i].x) / 50;
          }
          
          balls[i].y += balls[i].vy;
          
          if(balls[i].y > (MATRIX_HEIGHT+2)*255L - balls[i].r/4) {
            //balls[i].vy = -abs(balls[i].vy);
            balls[i].vy -= (balls[i].y - (MATRIX_HEIGHT*255L - balls[i].r/4)) / 50;
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
          balls[i].vy -= (32*dt)/16;

          //spray balls up
          if (balls[i].y < -5*255 || balls[i].x < -5*255 || balls[i].x > (MATRIX_WIDTH+5)*255L) {
            balls[i].y = -2*255;
            balls[i].x = 8*127;
            balls[i].vy = random(launch_speed/2, launch_speed);
            balls[i].vx = random(100) - 50;
            balls[i].r = random(7*255,8*255);
          }
          
        }

        //draw all balls on the temporary canvas (apparently this is called an isosurface)
        //find the distance of all LEDs within the bounding square of each ball 
        for (int x = (balls[i].x - balls[i].r)/255; x < (balls[i].x + balls[i].r)/255+1; x++) {
          for (int y = (balls[i].y - balls[i].r)/255; y < (balls[i].y + balls[i].r)/255+1; y++) {
            
            if (x >= 0 && x < HEIGHTMAP_WIDTH && y >= 0 && y < HEIGHTMAP_HEIGHT) {
              int d = sq(x*255L - balls[i].x)+sq(y*255L - balls[i].y);
              //make sure the distance is within our radius
              if (d <= sq(balls[i].r)) {
                //led radius
                d = sqrt(d);

                int d2;
                
                //add threshold information to canvas using inverse square
                if ( x < MATRIX_WIDTH && y < MATRIX_HEIGHT ) {
                  //convert to byte 0-255
                  d2 = (d*255)/balls[i].r;
                  //inverse (farther away is dimmer), this is linear
                  d2 = 255-d2;
                  //square
                  d2 = (d2*d2)/255;
                  //add to temporary canvas
                  temp_canvas[XY(x, y)].r = _max(_min(temp_canvas[XY(x, y)].r + d2, 255), 0);
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
      for (int x = 0; x < HEIGHTMAP_WIDTH; x++) {
        for (int y = 0; y < HEIGHTMAP_HEIGHT; y++) {
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
      for (int x = 0; x < MATRIX_WIDTH; x++) {
        for (int y = 0; y < MATRIX_HEIGHT; y++) {
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
      
      
    }
};

LIGHT_SKETCHES::REGISTER<METABALLS> metaballs("metaballs");