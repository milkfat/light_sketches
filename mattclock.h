//#define CLOCK_HORIZONTAL
#define SEGMENT_LENGTH 20
#define SEGMENT_SPACING 4



class MATTCLOCK: public LIGHT_SKETCH {
  public:
        MATTCLOCK () {setup();}
    ~MATTCLOCK () {}
  private:
    bool clock_scribble = true;
    bool clock_digital = false;
    bool digit_rotate = true;
    bool clock_3d = true;
    int x_shift = -10;
    int y_shift = 10;
    int update_count = 0; //keep track of how many digits were updated in a cycle
    int erase_delay = 0;
    int draw_delay = 0;

    
    //uint8_t SEGMENT_LENGTH = 5;
    //uint8_t SEGMENT_SPACING = 1;
    
    enum clock_types {
      DIGITAL_CLOCK,
      HAND_DRAWN_CLOCK,
      TEXT_CLOCK,
      ANALOG_CLOCK,
      NUMBER_OF_CLOCK_TYPES
    };

    //0 = digital clock
    //1 = text clock
    //2 = analog clock
    //3 = hand-drawn digital clock
    uint8_t current_effect = DIGITAL_CLOCK+(NUMBER_OF_CLOCK_TYPES-1);

    char timebuffer[15];
    struct tm ti;
    struct tm * timeinfo = &ti;

    int old_seconds = 0;
    int seconds;
    uint32_t tick_time = 0;

    //define the (x,y) start and (x,y) end coordinates of each segment in a 7-segment digit
    uint8_t segments[7][2][2]
      {
        {{SEGMENT_SPACING,0},  {SEGMENT_LENGTH,0}},  //top
        //{{1,0},  {5,0}},  //top
        {{0,SEGMENT_SPACING},  {0,SEGMENT_LENGTH}},  //top left
        //{{0,1},  {0,5}},  //top left
        {{SEGMENT_LENGTH+SEGMENT_SPACING,SEGMENT_SPACING},  {SEGMENT_LENGTH+SEGMENT_SPACING,SEGMENT_LENGTH}},  //top right
        //{{6,1},  {6,5}},  //top right
        {{SEGMENT_SPACING,SEGMENT_LENGTH+SEGMENT_SPACING},  {SEGMENT_LENGTH,SEGMENT_LENGTH+SEGMENT_SPACING}},  //middle
        //{{1,6},  {5,6}},  //middle
        {{0,SEGMENT_LENGTH+SEGMENT_SPACING*2},  {0,SEGMENT_LENGTH*2+SEGMENT_SPACING}}, //bottom left
        //{{0,7},  {0,11}}, //bottom left
        {{SEGMENT_LENGTH+SEGMENT_SPACING,SEGMENT_LENGTH+SEGMENT_SPACING*2},  {SEGMENT_LENGTH+SEGMENT_SPACING,SEGMENT_LENGTH*2+SEGMENT_SPACING}}, //bottom right
        //{{6,7},  {6,11}}, //bottom right
        {{SEGMENT_SPACING,SEGMENT_LENGTH*2+SEGMENT_SPACING*2}, {SEGMENT_LENGTH,SEGMENT_LENGTH*2+SEGMENT_SPACING*2}}  //bottom
        //{{1,12}, {5,12}}  //bottom
      };

    //define which segments are illuminated for each number 0-9
    bool numbers[10][7]
      {
        {1,1,1,0,1,1,1}, //0
        {0,0,1,0,0,1,0}, //1
        {1,0,1,1,1,0,1}, //2 000000000.00
        {1,0,1,1,0,1,1}, //3 0000000.0000
        {0,1,1,1,0,1,0}, //4 00000.000000
        {1,1,0,1,0,1,1}, //5 0000.0000000
        {1,1,0,1,1,1,1}, //6 000.00...000
        {1,0,1,0,0,1,0}, //7 000.. 0000.0
        {1,1,1,1,1,1,1}, //8 0000.0000.00
        {1,1,1,1,0,1,1}  //9 000000..0000
      };

    uint8_t wire_character[10][6][2] = 
      {
        {
          //zero
          {128,240},
          {32,200},
          {64,32},
          {196,64},
          {158,220},
          {80,246}
        },
        {
          //one
          {160,190},
          {212,240},
          {212,220},
          {212,100},
          {212,50},
          {212,24}
        },
        {
          //two
          {32,192},
          {128,240},
          {220,128},
          {32,24},
          {48,60},
          {220,32}
        },
        {
          //three
          {32,192},
          {192,240},
          {128,128},
          {128,128},
          {192,16},
          {32,64}
        },
        {
          //four
          {192,0},
          {192,230},
          {192,255},
          {32,128},
          {48,128},
          {255,128}
        },
        {
          //five
          {212,255},
          {32,255},
          {32,128},
          {128,164},
          {212,48},
          {32, 48}
        },
        {
          //six
          {192,255},
          {64,96},
          {128,32},
          {192,64},
          {128,128},
          {64,64}
        },
        {
          //seven
          {32,232},
          {128,210},
          {232,230},
          {232,230},
          {100,110},
          {32,16}
        },
        {
          //eight
          {240,232},
          {32,192},
          {230,64},
          {148,16},
          {68,64},
          {236,240}
        },
        {
          //nine
          {200,230},
          {24,210},
          {60,160},
          {198,210},
          {198,238},
          {160,32}
        }
      };
    
    //does our clock draw characters in the vertical or horizontal direction
    #ifdef CLOCK_HORIZONTAL
    int cx = 1;
    int cy = 0;
    #else
    int cx = 0;
    int cy = 1;
    #endif

    //current cursor position  
    int cposx = 0;
    int cposy = 0;
    
    //current character position
    int dpos = 0;

    struct digit {
      //last number
      uint8_t number_from = 0;
      //current number
      uint8_t number_to = 0;

      //any segment that has changed states will be matched up with a non-changing segment (to animate the merging/spawning of segments)
      //store the matching segments here
      uint8_t segments_move[7] = {0,0,0,0,0,0,0};

      //millis() of last change
      uint32_t update_time;
      //time at which to erase the old digit
      uint32_t erase_time;
      //time at which to draw the new digit
      uint32_t draw_time;
    };

    digit digits[6];

    //update a digit
    //figure out which segments will move and merge with the old segments
    void update_digit(const uint8_t& dig, const uint8_t& number) {
      digit * d = &digits[dig];
      

      
      if (d->number_to != number) {

        //calculate our delay for erasing the old digit
        int erase_delay2 = update_count * erase_delay;
        if (erase_delay < 0) {
          //negative delays indicate reverse order
          erase_delay2 = (5 - dig) * -erase_delay;
        }
        //calculate our delay for drawing the new digit
        int draw_delay2 = update_count * draw_delay;
        if (draw_delay < 0) {
          //negative delays indicate reverse order
          draw_delay2 = (5 - dig) * -draw_delay;
        }

        uint32_t new_update_time = millis();
      
        //do not update the number if the previous number is currently drawing
        if (new_update_time < d->draw_time || new_update_time > d->draw_time+500) {
          d->number_from = d->number_to;
          d->number_to = number;
        }
        
        //do not update times until the previous update is complete
        if (new_update_time > d->draw_time + 500) {
          d->update_time = new_update_time;
          d->erase_time = new_update_time + erase_delay2;
          d->draw_time = new_update_time + draw_delay2;
        }

        update_count++;
        

        //figure out segment movement
        for (int i = 0; i < 7; i++) {
          if (numbers[d->number_from][i] != numbers[d->number_to][i]) {
            //old segment goes away (find new segment with which to merge)
            if (numbers[d->number_from][i] == 1) {
              uint8_t min_dist = 10;
              for (int j = 0; j < 7; j++) {
                if (numbers[d->number_to][j] == 1) {
                  uint8_t dist = abs(i - j)+2;
                  if (numbers[d->number_from][j] == 0) {
                    dist -= 2;
                  }
                  if (dist < min_dist) {
                    min_dist = dist;
                    d->segments_move[i] = j;
                  }
                }
              }
            } else { //d->number_to == 1
              //new segment comes into existence (find old segment from which to spawn)
              uint8_t min_dist = 10;
              for (int j = 0; j < 7; j++) {
                if (numbers[d->number_from][j] == 1) {
                  uint8_t dist = abs(i - j)+2;
                  if (numbers[d->number_to][j] == 0) {
                    dist -= 2;
                  }
                  if (dist < min_dist) {
                    min_dist = dist;
                    d->segments_move[i] = j;
                  }
                }
              }
            }
          }
        }
        draw_digit(*d);
      } else {
        draw_digit(*d);
      }
    }

    void draw_3d(const int& x0, const int& y0, const int& z0, const int& x1, const int& y1, const int& z1, const uint8_t& hue = 0, const uint8_t& sat = 255, const uint8_t& val = 255) {

      int32_t p[3];
      
      p[0] = x0;
      p[1] = y0;
      p[2] = z0;

      int32_t p0[3];
      
      matrix.rotate(p,p0);

      p[0] = x1;
      p[1] = y1;
      p[2] = z1;

      int32_t p1[3];

      matrix.rotate(p,p1);

      //translate vectors to coordinates
      p0[2] += -180 * 256 + (200 * 256 * debug_scaler) / 256;

      p1[2] += -180 * 256 + (200 * 256 * debug_scaler) / 256;

      matrix.perspective(p0);
      matrix.perspective(p1);
      
      draw_line_fine(leds, p0[0], p0[1], p1[0], p1[1], hue, sat, val, -10000, val);
    } //draw_3d()


    //draw a segment
    void draw_s(const uint8_t& i, int d = 0, const uint8_t& part = 0) {
      
      uint8_t draw_part = 0;

      uint8_t bump = sin8(d-64);
      
      int x = cposx*256;
      int y = (MATRIX_HEIGHT-1-cposy)*256;
      int z = bump*5;

      int x0 = segments[i][0][0]*256;
      int y0 = segments[i][0][1]*256;
      int z0 = z;

      int x1 = segments[i][1][0]*256;
      int y1 = segments[i][1][1]*256;
      int z1 = z;

      if (clock_3d) {

        if (digit_rotate) {
          d = ease8InOutApprox(d);
          int d1 = d/2 + 64;

          int adjust = (SEGMENT_LENGTH+SEGMENT_SPACING*2)*128;


          //figure out our screen orientation
          //this allows us to change digits when perpendicular to the screen
          //TODO: need to optimize this, it's super inefficient to do this for every segment
          int x0b_asdf = SEGMENT_SPACING*256-adjust;
          int x0b = (x0b_asdf*sin16(d1*256))/32768;
          x0b += adjust;
          int z0b = (x0b_asdf*cos16(d1*256))/32768;
          
          int x1b_asdf = (SEGMENT_LENGTH+SEGMENT_SPACING)*256 - adjust;
          int x1b = (x1b_asdf*sin16(d1*256))/32768;
          x1b += adjust;
          int z1b = (x1b_asdf*cos16(d1*256))/32768;

          int32_t p[3];

          p[0] = x+x0b;
          p[1] = y-y0;
          p[2] = z+z0b;

          int32_t p0[3];

          matrix.rotate(p, p0);

          p[0] = x+x1b;
          p[1] = y-y1;
          p[2] = z+z1b;

          int32_t p1[3];

          matrix.rotate(p, p1);

          //translate vectors to coordinates
          p0[2] += -180 * 256 + (200 * 256 * debug_scaler) / 256;
          p1[2] += -180 * 256 + (200 * 256 * debug_scaler) / 256;
        
          matrix.perspective(p0);
          matrix.perspective(p1);

          draw_part = 1;
          if (p0[0] > p1[0]) {
            draw_part = 2;
            d1+=128;
          }
          //END SCREEN ORIENTATION STUFF

        
          int x0_asdf = x0-adjust;
          x0 = (x0_asdf*sin16(d1*256))/32768;
          x0 += adjust;
          z0 = (x0_asdf*cos16(d1*256))/32768;
          
          int x1_asdf = x1-adjust;
          x1 = (x1_asdf*sin16(d1*256))/32768;
          x1 += adjust;
          z1 = (x1_asdf*cos16(d1*256))/32768;
        
        } //DIGIT_ROTATE


        if (part == 0 || draw_part == part) {
          draw_3d(x+x0, y-y0, z+z0, x+x1, y-y1, z+z1);
        }

      } else {
        draw_line_fine(leds, x+x0, y-y0, x+x1, y-y1, 0, 255, 64, -10000, 64);
      }

    }

  
    //draw an interpolated segment between two segments based on a delta
    void draw_si(const uint8_t& a, const uint8_t& b, const int& d) {

      uint8_t bump = sin8(d-64);

      int x = cposx*256;
      int y = (MATRIX_HEIGHT-1-cposy)*256;
      int z = bump*5;
      
      int x0d = segments[b][0][0] - segments[a][0][0];
      int x0 = segments[a][0][0]*256 + x0d*d;
      
      int y0d = segments[b][0][1] - segments[a][0][1];
      int y0 = segments[a][0][1]*256L + y0d*d;
      
      int x1d = segments[b][1][0]  - segments[a][1][0];
      int x1 = segments[a][1][0]*256 + x1d*d;
      
      int y1d = segments[b][1][1] - segments[a][1][1];
      int y1 = segments[a][1][1]*256L + y1d*d;
      
      if (clock_3d) {

        draw_3d(x+x0, y-y0, z, x+x1, y-y1, z);

      } else {
        draw_line_fine(leds, x+x0, y-y0, x+x1, y-y1, 0, 255, 64, -10000, 64);
      }

    }
    
    //draw seven-segment digit
    void draw_ss(bool n[]) {
      
      if (clock_scribble) {
        draw_scribble(digits[dpos]);
      }

      if (clock_digital) {
        //step through each segment
        for (int i = 0; i < 7; i++) {
          //check to see if the segment should be drawn
          if (n[i]) {
            //draw the segment
            draw_s(i);
          }
        
        }
      }

      dpos++;
      //cposy += 14*cy;
      cposy += (SEGMENT_LENGTH*2+SEGMENT_SPACING*4)*cy;
      //cposx += 8*cx;
      cposx += (SEGMENT_LENGTH+SEGMENT_SPACING*3)*cx;
    }

    void animate_digit(digit &d, const int& td) {
      int tracker[7][7] = {0};
        for (uint8_t i = 0; i < 7; i++) {
          if (numbers[d.number_from][i] == 1 && numbers[d.number_to][i] == 1) {
            //draw a basic digit segment
            draw_s(i, td);
          } else if (numbers[d.number_from][i] == 1) {
            //this segment was removed; animate it.
            if (digit_rotate) {
              draw_s(i, td, 1);
            } else {
              if (tracker[i][d.segments_move[i]] == 0) {
                draw_si(i, d.segments_move[i], td);
                tracker[i][d.segments_move[i]] = 1;
              }
            }
            
          } else if (numbers[d.number_to][i] == 1) {
            //this segment is new; animate it.
            if (digit_rotate) {
              draw_s(i, td, 2);
            } else {     
              if (tracker[d.segments_move[i]][i] == 0) {
                draw_si(d.segments_move[i], i, td);
                tracker[d.segments_move[i]][i] = 1;
              }
            }
            
          }
        }
    }

    //draw digit object, including animation when transitioning
    void draw_digit(digit &d) { 
      
      int td = millis() - d.update_time;
      int ed = millis() - d.erase_time;
      int dd = millis() - d.draw_time;
      if (ed < 0) {
        draw_digit(d.number_from + '0');
      } else if (dd < 500) {
        dd /= 2;
        
        if (clock_scribble) {
          draw_scribble(digits[dpos]);
        }
        
        if (clock_digital) {
          animate_digit(d, dd);
        }

        dpos++;
        //cposy += 14*cy;
        cposy += (SEGMENT_LENGTH*2+SEGMENT_SPACING*4)*cy;
        //cposx += 8*cx;
        cposx += (SEGMENT_LENGTH+SEGMENT_SPACING*3)*cx;
      } else {
        draw_digit(d.number_to + '0');
      }
    }


    //draw digits (and colons) on the screen
    void draw_digit(const char& dig) {
      uint8_t val = 255;

      switch (dig)
      {
        case '0':
          //don't draw the first character if it is a zero
          if (dpos == 0) {
            dpos++;
            //cposy += 14*cy;
            cposy += (SEGMENT_LENGTH*2+SEGMENT_SPACING*4)*cy;
            //cposx += 8*cx;
            cposx += (SEGMENT_LENGTH+SEGMENT_SPACING*3)*cx;
          } else {
            draw_ss(numbers[0]);
          }
          break;
        case '1':
            draw_ss(numbers[1]);
          break;
        case '2':
            draw_ss(numbers[2]);
          break;
        case '3':
            draw_ss(numbers[3]);
          break;
        case '4':
            draw_ss(numbers[4]);
          break;
        case '5':
            draw_ss(numbers[5]);
          break;
        case '6':
            draw_ss(numbers[6]);
          break;
        case '7':
            draw_ss(numbers[7]);
          break;
        case '8':
            draw_ss(numbers[8]);
          break;
        case '9':
            draw_ss(numbers[9]);
          break;
        case ':':
          if (dpos > 3 && millis() - 500 > tick_time) {
            val = 0;
          }
          if (!clock_3d) {
            //draw a stupid colon on the stupid screen in the stupidest way possible
            if (SEGMENT_LENGTH > 4) {
              leds[XY(cposx + 0 + (SEGMENT_LENGTH-2)*cy, MATRIX_HEIGHT - 1 - cposy - 0 - (SEGMENT_LENGTH-1)*cx)] = 0x202020;
            }
            //leds[XY(cposx + 0 + 3*cy, MATRIX_HEIGHT - 1 - cposy - 1 - 4*cx)].r = 255;
            leds[XY(cposx + 0 + (SEGMENT_LENGTH-2)*cy, MATRIX_HEIGHT - 1 - cposy - 1 - (SEGMENT_LENGTH-1)*cx)] = 0x202020;
            if (SEGMENT_LENGTH > 4) {
              leds[XY(cposx + 1 + (SEGMENT_LENGTH-2)*cy, MATRIX_HEIGHT - 1 - cposy - 0 - (SEGMENT_LENGTH-1)*cx)] = 0x202020;
              leds[XY(cposx + 1 + (SEGMENT_LENGTH-2)*cy, MATRIX_HEIGHT - 1 - cposy - 1 - (SEGMENT_LENGTH-1)*cx)] = 0x202020;
            }
            //leds[XY(cposx + 0 + 3*cy, MATRIX_HEIGHT - 1 - cposy - 3 - 4*cx)].r = 255;
            leds[XY(cposx + 0 + (SEGMENT_LENGTH-2)*cy, MATRIX_HEIGHT - 1 - cposy - 3 - (SEGMENT_LENGTH-1)*cx)] = 0x202020;
            if (SEGMENT_LENGTH > 4) {
              leds[XY(cposx + 0 + (SEGMENT_LENGTH-2)*cy, MATRIX_HEIGHT - 1 - cposy - 4 - (SEGMENT_LENGTH-1)*cx)] = 0x202020;
              leds[XY(cposx + 1 + (SEGMENT_LENGTH-2)*cy, MATRIX_HEIGHT - 1 - cposy - 3 - (SEGMENT_LENGTH-1)*cx)] = 0x202020;
              leds[XY(cposx + 1 + (SEGMENT_LENGTH-2)*cy, MATRIX_HEIGHT - 1 - cposy - 4 - (SEGMENT_LENGTH-1)*cx)] = 0x202020;
            }
            //cposy += 6*cy;
            cposy += ((SEGMENT_LENGTH*2+SEGMENT_SPACING*2)/2)*cy;
            //cposx += 4*cx;
            cposx += (SEGMENT_LENGTH-SEGMENT_SPACING)*cx;
          } else {
            //draw a 3d colon
            int32_t p[3];
      
            int x = (cposx+SEGMENT_SPACING+2)*256;
            int y = (MATRIX_HEIGHT-1-cposy)*256;

            draw_3d(x, y-(SEGMENT_SPACING*2)*256, 0, x+2*256, y-(SEGMENT_SPACING*2)*256, 0, 0, 255, val);

            draw_3d(x+(SEGMENT_LENGTH-10)*256, y-(SEGMENT_SPACING*2)*256, 0, x+(SEGMENT_LENGTH-8)*256, y-(SEGMENT_SPACING*2)*256, 0, 0, 255, val);
            
            //cposy += 6*cy;
            cposy += (SEGMENT_LENGTH+SEGMENT_SPACING)*cy;
            //cposx += 4*cx;
            cposx += (SEGMENT_LENGTH-SEGMENT_SPACING)*cx;

          }
          break;
        default:
          break;
        
      }
      
    }

    void update_timebuffer() {

      getLocalTime(timeinfo);
      
      strftime(timebuffer, sizeof(timebuffer), "%I:%M:%S %p", timeinfo);
      
    }

    void draw_digital_clock() {
      update_count = 0;
        for (int i = 0; i < sizeof(timebuffer); i++) {
          if (timebuffer[i] == '\0') {
            break;
          }
          //draw the characters from our buffer
          if ( timebuffer[i] >= '0' && timebuffer[i] <= '9' ) {
            update_digit(dpos, timebuffer[i] - '0'); //delay between digits when multiple are updated (rolling effect)
          } else {
            draw_digit(timebuffer[i]);
          }
        }
    }

  
  public:



    void next_effect() {
      if (current_effect == TEXT_CLOCK) {
        display_text = "";
      }
      current_effect++;
      current_effect %= NUMBER_OF_CLOCK_TYPES;
      if (current_effect == DIGITAL_CLOCK) {
        //seven-segment digital clock
        clock_scribble = false;
        clock_digital = true;
        digit_rotate = true;
        erase_delay = -175;
        draw_delay = -175;
      } else if (current_effect == HAND_DRAWN_CLOCK) {
        //hand-drawn digital clock
        clock_scribble = true;
        clock_digital = false;
        erase_delay = 100;
        draw_delay = 400;
      }
    }

    void setup() {
      for (int i = 0; i < 6; i++) {
        digits[i].number_from = 0;
        digits[i].number_to = 0;
        digits[i].update_time = millis();
        digits[i].erase_time = millis();
        digits[i].draw_time = millis();
      }
      next_effect();
    }

    void reset() {

    }

    uint32_t loop_time = millis();
    void loop() {
      if (millis() - 16 > loop_time) {
        loop_time = millis();
        static uint16_t bd = 1;
        static uint16_t gd = 1;
        static uint16_t ad = 1;
        rotation_beta = 90 + (sin16(bd)*15.f)/32768; 
        rotation_gamma = 0 + (sin16(gd)*20.f)/32768;
        rotation_alpha = 0 + (sin16(ad)*20.f)/32768;
        bd += 32;
        gd += 42;
        ad += 52;

        

        cposx = x_shift;
        cposy = y_shift;
        dpos = 0;
        
        LED_show();
        LED_black();

        
        //get the current time
        update_timebuffer();

        //read the current time and update tick_time if necessary
        seconds = timeinfo->tm_sec;
        
        if (seconds != old_seconds) {
          tick_time = millis();
          old_seconds = seconds;
        }

        //run the active sketch
        if (current_effect == DIGITAL_CLOCK || current_effect == HAND_DRAWN_CLOCK) {
          draw_digital_clock();
        }

        if (current_effect == TEXT_CLOCK) {
          display_text = timebuffer;
        }

        if (current_effect == ANALOG_CLOCK) {
          draw_analog_clock();
        }
      }

    }//loop

  private:
    void draw_analog_clock() {

        //the absolute position of the clock on the screen
        int clock_x = (MATRIX_WIDTH*256) / 2;
        int clock_y = MATRIX_HEIGHT*256 - 16*256;


        //draw the hour markers around the outer edge
        for (int i = 0; i < 12; i++) {
          
          int l = 14*256;
          if (i%3 == 0) {
            l = 13*256;
          }
          int l2 = 15*256;

          int a = (i*65536)/12;

          int x = (l*sin16(a))/32768;
          int y = (l*cos16(a))/32768;
          int x2 = (l2*sin16(a))/32768;
          int y2 = (l2*cos16(a))/32768;


          draw_line_fine(leds, clock_x+x, clock_y+y, clock_x+x2, clock_y+y2,0,0,12,-10000,12);
        }


        //draw the hands

        int second_angle = (seconds*65536)/60;

        if (millis() - tick_time < 50) {

          second_angle += 65536/180;
        }
        
        int second_length = 14*256;
        int second_x = ( second_length*(sin16(second_angle)) ) /32768;
        int second_y = ( second_length*(cos16(second_angle)) ) /32768;

        int minutes = timeinfo->tm_min;
        int minute_angle = (minutes*65536 + second_angle )/60;
        int minute_length = 10*256;
        int minute_x = ( minute_length*(sin16(minute_angle)) ) /32768;
        int minute_y = ( minute_length*(cos16(minute_angle)) ) /32768;

        int hours = timeinfo->tm_hour;
        int hour_angle = (hours*65536 + minute_angle )/12;
        int hour_length = 6*256;
        int hour_x = ( hour_length*(sin16(hour_angle)) ) /32768;
        int hour_y = ( hour_length*(cos16(hour_angle)) ) /32768;


        draw_line_fine(leds, clock_x, clock_y, clock_x+second_x, clock_y+second_y,0,0,32,-10000,32);
        draw_line_fine(leds, clock_x, clock_y, clock_x+minute_x, clock_y+minute_y,160,0,64,-10000,64);
        draw_line_fine(leds, clock_x, clock_y, clock_x+hour_x, clock_y+hour_y,90,0,64,-10000,64);

    } //draw analog clock



    void draw_scribble(digit &d) {


      #define SCRIBBLE_SPEED 600
      #define X_SCALE 36
      #define Y_SCALE 54
      
      uint32_t digit_time = d.update_time;
      uint32_t erase_time = d.erase_time;
      uint32_t draw_time = d.draw_time;

      int td = millis() - digit_time;
      int ed = millis() - erase_time;
      int dd = millis() - draw_time;

      uint8_t old_digit = d.number_from;
      uint8_t current_digit = d.number_to;
      
      
      uint8_t val = 255; //default brightness

      uint8_t percentage = _max(_min(((dd-200)*255)/SCRIBBLE_SPEED,255),0);

      if (ed < 100) {
        percentage = 255;
        val = _max(_min(100-ed,100)*64/100,0);
        current_digit = old_digit;
      }

      int32_t wire_digit[6][2];

      if ( current_digit >= 0 && current_digit <= 9 ) {
        
        for (int i = 0; i < 6; i++) {

          int32_t p[3];



          p[0] = wire_character[current_digit][i][0]*X_SCALE;
          p[1] = (255-wire_character[current_digit][i][1])*Y_SCALE;
/*
          //animate from the old digit
          if (td < SCRIBBLE_SPEED) {
            td = _max(_min(td,SCRIBBLE_SPEED),0);
            
            p[0] = (p[0]*td)/SCRIBBLE_SPEED;
            p[1] = (p[1]*td)/SCRIBBLE_SPEED;


            p[0] += ((wire_character[old_digit][i][0]*X_SCALE)*(SCRIBBLE_SPEED-td))/SCRIBBLE_SPEED;
            p[1] += (((255-wire_character[old_digit][i][1])*Y_SCALE)*(SCRIBBLE_SPEED-td))/SCRIBBLE_SPEED;
          }
*/        
          int x = cposx*256;
          int y = (MATRIX_HEIGHT-1-cposy)*256;

          p[0] = x+p[0];
          p[1] = y-p[1];
          p[2] = 0;

          int32_t p0[3];

          matrix.rotate(p, p0);

          //translate vectors to coordinates
          p0[2] += -180 * 256 + (200 * 256 * debug_scaler) / 256;

          //correct 3d perspective
          matrix.perspective(p0);

          wire_digit[i][0] = p0[0];
          wire_digit[i][1] = p0[1];

        }
      }  
      
      matt_curve8(wire_digit, 6, default_color, default_saturation, val, false, false, true, percentage);
    }


};




LIGHT_SKETCHES::REGISTER<MATTCLOCK> mattclock("mattclock");