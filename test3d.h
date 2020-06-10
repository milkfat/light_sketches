#ifndef LIGHTS_TEST3D_H
#define LIGHTS_TEST3D_H
//3D

#include "phos.h"

class TEST3D: public LIGHT_SKETCH {
  public:
    TEST3D () {setup();}
    ~TEST3D () {}
  private:
    int skipped_frames = 0;

    uint16_t next_firework_frame = 0;
    
    Z_BUF _z_buffer;
    //0 = test object
    //1 = grid
    //2 = snow
    //3 = spiral
    //4 = box
    //5 = tunnel
    enum effects {
      TEST_OBJECT,
      GRID,
      SNOW,
      SPIRAL,
      BOX,
      TUNNEL,
      NUM_EFFECTS
    };
    
    enum grid_types {
      GRID_RED,
      GRID_PHOS,
      GRID_NOISE,
      NUM_GRID_TYPES
    };


    float compress(float value) {
      //return FC.compress(value);
      return value;
    }

    float decompress(float value) {
      //return FC.decompress(value);
      return value;
    }

    uint8_t grid_type = GRID_RED;

    uint8_t current_variation = BOX;

    uint16_t frame_count = 2100;

    bool do_not_update = 0;

    int32_t square[4][3] = {
      { -3 * 256, -4 * 256, 0},
      { -3 * 256, 4 * 256, 0},
      {3 * 256, 4 * 256, 0},
      {3 * 256, -4 * 256, 0}
    };

    int32_t spiral[12][3] = {
      { -8 * 256L, 0, 0},
      {0, 8 * 256L, -8 * 256L},
      {8 * 256L, 16 * 256L, 0},
      {0, 24 * 256L, 8 * 256L},
      { -8 * 256L, 32 * 256L, 0},
      {0, 40 * 256L, -8 * 256L},
      {8 * 256L, 48 * 256L, 0},
      {0, 56 * 256L, 8 * 256L},
      { -8 * 256L, 64 * 256L, 0},
      {0, 72 * 256L, -8 * 256L},
      {8 * 256L, 80 * 256L, 0},
      {0, 88 * 256L, 8 * 256L}
    };
    int32_t square_curvetest[4][3] = {
      { -18 * 256L, -18 * 256L, 0},
      { 18 * 256L, -18 * 256L, 0},
      { 18 * 256L, 18 * 256L, 0},
      { -18 * 256L, 18 * 256L, 0}
    };


    int32_t linex[2][3] = {
      {0, 0, 0},
      {8 * 256, 0, 0}
    };

    int32_t liney[2][3] = {
      {0, 0, 0},
      {0, 8 * 256, 0}
    };


    int32_t linez[2][3] = {
      {0, 0, 0},
      {0, 0, 8 * 256}
    };
    //
    int32_t square2[4][3];

    void rotate(int32_t in[3], int32_t out[3]) {
      led_screen.matrix.rotate(in, out);
    }

    void rotate(VECTOR3& p) {
      led_screen.matrix.rotate(p);
    }


    struct PARTICLE {
        int16_t x;  //positional coordinates
        int16_t y;
        int16_t z;
        int16_t vx; //velocities
        int16_t vy;
        int16_t vz;
    };

  private:

  #define NUM_TEST_3D_PARTICLES 2000
  PARTICLE particles[NUM_TEST_3D_PARTICLES];
  



    int32_t update_time = millis();

  public:
    void next_effect() {
      if (current_variation == GRID) {
        grid_type++;
        if (grid_type == NUM_GRID_TYPES) {
          grid_type = 0;
          current_variation++;
        }
      } else {
        current_variation++;
      }
      current_variation %= NUM_EFFECTS;

      display_text = "";
      if (current_variation == TUNNEL) {
        display_text = " HELLO";
      }

      //reset rotation
      reset();

      //snow
      if (current_variation == SNOW) {
        for (int i = 0; i < NUM_TEST_3D_PARTICLES; i++) {
          PARTICLE * cp = &particles[i];
          cp->x = random(-30 * 256L, 30 * 256L);
          cp->y = random(-148 * 256L, 148 * 256L);
          cp->z = random(-148 * 256L, 148 * 256L);
          cp->vx = random(-255, 256);
          cp->vy = -255;
          cp->vz = random(-255, 256);
        }
      }


    }


    #define GRID_SIZE 20
    CHSV grid_hsv[GRID_SIZE*2+1][GRID_SIZE*2+1];

    void grid_hsv_v(const int& x, const int& y, int32_t& var, int& cnt) {
      if (x >= 0 && x < GRID_SIZE*2+1 && y >= 0 && y < GRID_SIZE*2+1) {
        var += grid_hsv[x][y].v;
        cnt++;
      }
    }

    PHOS phos;

    void setup() {
      z_buffer = &_z_buffer;
      //particles = (PARTICLE*) malloc (NUM_TEST_3D_PARTICLES * sizeof(PARTICLE));
      for (int i = 0; i < NUM_TEST_3D_PARTICLES; i++) {
        particles[i].x = random(-30 * 256L, 30 * 256L);
        particles[i].y = -127*256;
        particles[i].z = random(-148 * 256L, 148 * 256L);
        particles[i].vx = random(-255, 256);
        particles[i].vy = -255;
        particles[i].vz = random(-255, 256);
      }

      reset();

      phos.setup();

      control_variables.add(led_screen.camera_position.z, "Camera Z:", 0, 1024*256);
      control_variables.add(led_screen.screen_distance, "Screen Z:", 0, 256*256);
      control_variables.add(led_screen.light_falloff, "Light Distance:", 1, 16);

    }

    void reset() {
      led_screen.rotation_alpha = 0;
      led_screen.rotation_beta = 90;
      led_screen.rotation_gamma = 0;
    }

    void loop() {

      // debug
      // static int asdf = 0;
      // if (asdf == 0) {
      //   asdf++;
      //   int32_t test_num = 1;

      //   for (int i = 0; i < 32; i++ ) {
      //     PARTICLE p;
      //     int32_t boo = test_num;
      //     p.set_vx(boo);
      //     int32_t raw_vx = p.get_vx_raw();
      //     std::cout << "\n\n\noriginal boo: " << boo << " vx: " << p.vx << " get_vx(): " << p.get_vx() << " /p\n";
      //     raw_vx /= 2;
      //     boo /= 2;
      //     p.set_vx_raw(raw_vx);
      //     std::cout << "updated  boo: " << boo << " vx: " << p.vx << " get_vx(): " << p.get_vx() << " /p\n";
      //     test_num*= 2;
      //   }

      // }
      
      if (effect_beat == 1) {
        effect_beat = 0;
        //reset();
      }


        static uint8_t cube_step = 0;
        uint16_t cube_size = 80*sin8(cube_step)+30*256;

     //uint32_t debug_time2 = micros();
        switch (current_variation)
        {
          case TEST_OBJECT:
            handle_test_object();
            break;
          case GRID:
            handle_grid();
            break;
          case SNOW:
            handle_snow();
            break;
          case SPIRAL:
            handle_spiral();
            break;
          case BOX:
            draw_cube(VECTOR3(0,cube_size,0), VECTOR3(15*256,15*256,15*256) );
            draw_cube(VECTOR3(0,-cube_size,0), VECTOR3(15*256,15*256,15*256) );
            draw_cube(VECTOR3(cube_size,0,0), VECTOR3(15*256,15*256,15*256) );
            draw_cube(VECTOR3(-cube_size,0,0), VECTOR3(15*256,15*256,15*256) );
            draw_cube(VECTOR3(0,0,cube_size), VECTOR3(15*256,15*256,15*256) );
            draw_cube(VECTOR3(0,0,-cube_size), VECTOR3(15*256,15*256,15*256) );
            cube_step +=4;
            led_screen.rotation_alpha += 1;
            led_screen.rotation_beta += .77;
            led_screen.rotation_gamma += .68;
            //handle_cube();
            break;
          case TUNNEL:
            handle_tunnel();
            break;
        }
     //debug_micros1 += micros() - debug_time2;

        //update the display
        if (!do_not_update) {
          skipped_frames--;
          //update LEDS
          LED_show();
          //clear LED buffer
          LED_black();
        }

    } //loop();



void handle_test_object() {

          //3D TEST OBJECT
          for (int i = 0; i < 4; i++) {
            rotate(square[i], square2[i]);
          }

          for (int i = 0; i < 4; i++) {
            draw_line_fine_hsv(led_screen, square2[i][0] + 3 * 256, square2[i][1] + 50 * 256, square2[(i + 1) % 4][0] + 3 * 256, square2[(i + 1) % 4][1] + 50 * 256);
          }

          int32_t r0[3];
          int32_t r1[3];
          rotate(linex[0], r0);
          rotate(linex[1], r1);

          draw_line_fine_hsv(led_screen, r0[0] + 3 * 256, r0[1] + 50 * 256, r1[0] + 3 * 256, r1[1] + 50 * 256, 160);


          rotate(liney[0], r0);
          rotate(liney[1], r1);

          draw_line_fine_hsv(led_screen, r0[0] + 3 * 256, r0[1] + 50 * 256, r1[0] + 3 * 256, r1[1] + 50 * 256, 48);


          rotate(linez[0], r0);
          rotate(linez[1], r1);

          draw_line_fine_hsv(led_screen, r0[0] + 3 * 256, r0[1] + 50 * 256, r1[0] + 3 * 256, r1[1] + 50 * 256, 96);

          led_screen.rotation_alpha += 2;
          led_screen.rotation_beta += .154;
          led_screen.rotation_gamma += .136;

} //handle_test_object()




void handle_grid() {


          //GRID

/*
          //DISSOLVE EFFECT
          static bool reset = 1;
          static uint8_t d = 1;
          if (reset) {
            reset = 0;
            d++;
            d%=2;
            for (int x = 0; x < GRID_SIZE*2+1;x++) {
              for (int y = 0; y < GRID_SIZE*2+1; y++) {
                if (!(x == 20 && y == 20)) {
                  grid_hsv[x][y].h=0;
                  grid_hsv[x][y].s=255;
                  grid_hsv[x][y].v=255-(d*255);
                } else {
                  grid_hsv[x][y].v=d*255;
                }
              }
            }
          }
          reset = 1;
          for (int x = 0; x < GRID_SIZE*2+1;x++) {
              for (int y = 0; y < GRID_SIZE*2+1; y++) {
                if (grid_hsv[x][y].v != d*255) {
                  reset = 0;
                  uint8_t up = 0;
                  uint8_t down = 0;
                  uint8_t left = 0;
                  uint8_t right = 0;
                  
                  if (x > 0) {
                    left = 1;
                  }
                  if (x < GRID_SIZE*2) {
                    right = 1;
                  }
                  if (y > 0) {
                    down = 1;
                  }
                  if (y < GRID_SIZE*2) {
                    up = 1;
                  }

                  if
                  (
                    (
                      grid_hsv[x][y-down].v == d*255 ||
                      grid_hsv[x][y+up].v == d*255 ||
                      grid_hsv[x+right][y].v == d*255 ||
                      grid_hsv[x-left][y].v == d*255 ||
                      grid_hsv[x+right][y-down].v == d*255 ||
                      grid_hsv[x+right][y+up].v == d*255 ||
                      grid_hsv[x-left][y-down].v == d*255 ||
                      grid_hsv[x-left][y+up].v == d*255
                    )
                    && random(60) == 0
                  ) {
                    grid_hsv[x][y].v = d*255;
                  }
                
                }
              }
            }
          //END DISSOLVE EFFECT
*/
/*
          //CRAWLER EFFECT
          #define NUM_CRAWLERS 9
          static uint8_t crawlers[NUM_CRAWLERS][3] = 
          {
            {1,6,130},
            {11,2,0},
            {20,6,48},
            {17,17,210},
            {7,14,160},
            {3,19,96},
            {3,19,24},
            {3,19,70},
            {3,19,185}
          };

          for (int i = 0;i < NUM_CRAWLERS; i++) {
            //place crawlers on the grid
            grid_hsv[crawlers[i][0]][crawlers[i][1]].h = crawlers[i][2];
            grid_hsv[crawlers[i][0]][crawlers[i][1]].v = 255;
            grid_hsv[crawlers[i][0]][crawlers[i][1]].s = 255;

            //move the crawlers
            if (random(2)) {
              if (random(2)) {
                if (crawlers[i][0] < GRID_SIZE*2) {
                  crawlers[i][0] += 1;
                }
              } else {
                if (crawlers[i][0] > 0) {
                  crawlers[i][0] -= 1;
                }
              }
            } else {
              if (random(2)) {
                if (crawlers[i][1] < GRID_SIZE*2) {
                  crawlers[i][1] += 1;
                }
              } else {
                if (crawlers[i][1] > 0) {
                  crawlers[i][1] -= 1;
                }
              }
            }
          }

          for (int x = 0; x < GRID_SIZE*2+1;x++) {
            for (int y = 0; y < GRID_SIZE*2+1; y++) {
              if (grid_hsv[x][y].v > 2) {
                grid_hsv[x][y].v-=(uint8_t)2;
              } else {
                grid_hsv[x][y].v=0;
              }
            }
          }
          //END CRAWLER EFFECT
*/
          uint8_t grid_z = 0;
          if (grid_type == GRID_PHOS) {
            phos.loop();
            static int stp = 0;
            grid_z = 5;
            stp++;
            for (int x = 0; x < GRID_SIZE*2+1; x++) {
              for (int y = 0; y < GRID_SIZE*2+1; y++) {
                grid_hsv[x][y].h = 96;
                grid_hsv[x][y].s = 255;
                grid_hsv[x][y].v = _min(_max(phos.getVal(x,y)*255,0),255);
              }
            }
          } else {
          
            static int stp = 0;
            grid_z = 25;
            stp++;
            for (int x = 0; x < GRID_SIZE*2+1; x++) {
              for (int y = 0; y < GRID_SIZE*2+1; y++) {
                int tmp_hue = 0;
                int tmp_val = 255;
                if (grid_type == GRID_NOISE) {
                  tmp_hue = inoise8(x*15+stp*5,y*15+stp*5,stp*5);
                  tmp_val = tmp_hue;
                }
                grid_hsv[x][y].h = tmp_hue;
                grid_hsv[x][y].s = 255;
                grid_hsv[x][y].v = tmp_val;
              }
            }
          }

          static float step0 = 0;
          static float step1 = 0;


          int32_t p0[3];
          int32_t p1[3];
          int32_t pt[3]; //temporary storage


          //horizontal lines
          for (int i = 0; i < GRID_SIZE+1; i++) {
            uint8_t x = i*2;

            int32_t stepi = step0+20;
            uint8_t hue = 0;
            uint8_t sat = 255;
            int8_t dist = 41 - ((stepi+i) % 42);
            uint8_t val = 0;
            if (dist > 0) {
              val = dist*6;
            }
            for (int j = 0; j < GRID_SIZE+0; j++) {
              uint8_t y = j*2+1;

              hue = grid_hsv[x][y].h;
              sat = grid_hsv[x][y].s;
              val = grid_hsv[x][y].v;

              pt[0] = -(MATRIX_HEIGHT * 256L) / 2 - (MATRIX_HEIGHT * 256L) / (GRID_SIZE-1) + (j*MATRIX_HEIGHT * 256L) / (GRID_SIZE-1);
              pt[1] = (i * MATRIX_HEIGHT * 256L) / (GRID_SIZE-1) - (MATRIX_HEIGHT * 256L) / 2;
              pt[2] = 0;
              if (grid_type > 0) {
                int cnt = 0;
                grid_hsv_v(x  ,y-2,pt[2],cnt);
                grid_hsv_v(x  ,y  ,pt[2],cnt);
                grid_hsv_v(x+1,y-1,pt[2],cnt);
                grid_hsv_v(x-1,y-1,pt[2],cnt);
                pt[2] -= 128*cnt;
                pt[2] *= grid_z;
              }
              rotate(pt, p0);

              //pt[0] *= -1;
              pt[0] += (MATRIX_HEIGHT * 256L) / (GRID_SIZE-1);
              pt[2] = 0;
              if (grid_type > 0) {
                int cnt = 0;
                grid_hsv_v(x  ,y+2,pt[2],cnt);
                grid_hsv_v(x  ,y  ,pt[2],cnt);
                grid_hsv_v(x+1,y+1,pt[2],cnt);
                grid_hsv_v(x-1,y+1,pt[2],cnt);
                pt[2] -= 128*cnt;
                pt[2] *= grid_z;
              }
              rotate(pt, p1);

              //translate vectors to coordinates
              scale_z(p0[2]);
              scale_z(p1[2]);

              //correct 3d perspective

              led_screen.perspective(p0);
              led_screen.perspective(p1);



              //draw_line_fine2(leds, p0[0], p0[1], p1[0], p1[1], hue, sat, val);
              draw_line_fine_hsv(led_screen, p0[0], p0[1], p1[0], p1[1], hue, sat, val, -10000, val, true);

            }

          }
          
          
          //vertical lines
          for (int i = 0; i < GRID_SIZE+1; i++) {
            uint8_t y = i*2;
            int32_t stepi = step0;
            uint8_t hue = 0;
            uint8_t sat = 255;
            int8_t dist = 41 - ((stepi+i) % 42);
            uint8_t val = 0;
            if (dist > 0) {
              val = dist*6;
            }

            for (int j = 0; j < GRID_SIZE+0; j++) {
              uint8_t x = j*2+1;

              hue = grid_hsv[x][y].h;
              sat = grid_hsv[x][y].s;
              val = grid_hsv[x][y].v;
              
              pt[0] = (-MATRIX_HEIGHT * 256L) / (GRID_SIZE-1) + (i * MATRIX_HEIGHT * 256L) / (GRID_SIZE-1) - (MATRIX_HEIGHT * 256L) / 2;
              pt[1] = -(MATRIX_HEIGHT * 256L) / 2 + (j * MATRIX_HEIGHT * 256L) / (GRID_SIZE-1);
              pt[2] = 0;
              if (grid_type > 0) {
                int cnt = 0;
                grid_hsv_v(x-2,y  ,pt[2],cnt);
                grid_hsv_v(x  ,y  ,pt[2],cnt);
                grid_hsv_v(x-1,y+1,pt[2],cnt);
                grid_hsv_v(x-1,y-1,pt[2],cnt);
                pt[2] -= 128*cnt;
                pt[2] *= grid_z;
              }
              rotate(pt, p0);

              //pt[1] *= -1;
              pt[1] += (MATRIX_HEIGHT * 256L) / (GRID_SIZE-1);
              pt[2] = 0;
              if (grid_type > 0) {
                int cnt = 0;
                grid_hsv_v(x+2,y  ,pt[2],cnt);
                grid_hsv_v(x  ,y  ,pt[2],cnt);
                grid_hsv_v(x+1,y+1,pt[2],cnt);
                grid_hsv_v(x+1,y-1,pt[2],cnt);
                pt[2] -= 128*cnt;
                pt[2] *= grid_z;
              }
              rotate(pt, p1);

              //translate vectors to coordinates
              scale_z(p0[2]);
              scale_z(p1[2]);


              //correct 3d perspective
 
              
              //              p0[0] = (-150*256L * (p0[0] - 0))       / (-150*256L + p0[2]) + 0;
              //              p0[1] = (-150*256L * (p0[1] - 0)) / (-150*256L + p0[2]) + 0;
              //              p1[0] = (-150*256L * (p1[0] - 0))       / (-150*256L + p1[2]) + 0;
              //              p1[1] = (-150*256L * (p1[1] - 0)) / (-150*256L + p1[2]) + 0;
              if ( led_screen.perspective(p0) && led_screen.perspective(p1) ) {
                draw_line_fine_hsv(led_screen, p0[0], p0[1], p1[0], p1[1], hue, sat, val, -10000, val);
              }
            }
          }


          static uint32_t grid_time = micros();
          uint32_t current_time = micros();
          uint32_t elapsed_time = current_time - grid_time;
          grid_time = current_time;
          
          static uint32_t step_time = millis();
          if (millis() - 16 > step_time) {
            step0+=.9f;
            step1+=.9f;
            step_time = millis();
          }


          float rotation_dividy_thing = 1;
          if (grid_type > 0) {
            rotation_dividy_thing = .2;
          }
          led_screen.rotation_alpha += (elapsed_time/20000.f)*rotation_dividy_thing;
          if (led_screen.rotation_alpha > 360) {
            led_screen.rotation_alpha -= 360;
          }
          led_screen.rotation_beta += (elapsed_time/18000.f)*rotation_dividy_thing;
          if (led_screen.rotation_beta > 360) {
            led_screen.rotation_beta -= 360;
          }
          led_screen.rotation_gamma += (elapsed_time/17000.f)*rotation_dividy_thing;
          if (led_screen.rotation_gamma > 360) {
            led_screen.rotation_gamma -= 360;
          }

} //handle_grid()



void handle_snow() {

          //SNOW
          for (int i = 0; i < NUM_TEST_3D_PARTICLES; i++) {
            PARTICLE * cp = &particles[i];
            cp->x += cp->vx;
            cp->y += cp->vy;
            cp->z += cp->vz;
            if (cp->y < -127 * 256L) {
              cp->x = random(-100 * 256L, 100 * 256L);
              cp->y = 32767;
              cp->z = random(-148 * 256L, 148 * 256L);
              cp->vx = random(-128, 128);
              cp->vz = random(-255, 256);
            }

            int32_t p[3] = {
              cp->x,
              cp->y,
              cp->z
            };
            int32_t p0[3];

            rotate(p, p0);

            //translate vectors to coordinates
            scale_z(p0[2]);

            //correct 3d perspective
              
            if (led_screen.perspective(p0)) {
              blendXY(led_screen, p0[0], p0[1], 0, 0, _max(_min((p0[2] + 148 * 256L) / 256L, 255), 0));
            }

          }
        
} //handle_snow()


void handle_spiral() {

          //SPIRAL
          int32_t coords[12][2];
          for (int i = 0; i < 12; i++) {


            int32_t p[3];
            static float taco = 1;
            static float taco_dir = .0000;
            taco += taco_dir;
            if (taco > 2) {
              taco_dir = -fabs(taco_dir);
            }
            if (taco < 1) {
              taco_dir = fabs(taco_dir);
            }
            p[0] = spiral[i][0]*2;
            p[1] = spiral[i][1]*taco;
            p[2] = spiral[i][2]*2;
            //int32_t* p = square_curvetest[i];
            int32_t p0[3];

            rotate(p, p0);

            //translate vectors to coordinates
            scale_z(p0[2]);

            //correct 3d perspective

            
            led_screen.perspective(p0);

            coords[i][0] = p0[0];
            coords[i][1] = p0[1];

            //blendXY(led_screen, p0[0], p0[1], 0, 0, _max(_min((150*256L-p0[2])/256L,255),0));


          }
          matt_curve8(coords, 12, default_color, default_saturation, 255, false, false, true);
          led_screen.rotation_alpha+=.2;
          led_screen.rotation_beta+=.3;

} //handle_spiral();


void handle_tunnel() {
  static uint16_t stp = 0; //rotation
  //static uint8_t cnt = 0;
  static uint8_t stp2 = 0; //bright light
  //static uint8_t stp3 = 0; //circle width
  static uint16_t stp4 = 0; //tunnel speed
  #define TUNNEL_DETAIL 32
  for (int i = 0; i < TUNNEL_DETAIL+1; i++) {
    //wavy
    //int32_t v0[3] = {0, 20*256L-2*256L*i, -150*256L};
    //int32_t v1[3] = {-3*256L+6*sin8(stp+i*20), 17*256L-4*256L*i, 210*256L};
    static int32_t old_v1[3];
    //tunnel thing
    //float val = (36.f * sin8(stp3)) / 256.f;
    int32_t c = static_cast<int32_t>(cos16(stp + i * (65536L/TUNNEL_DETAIL)));
    int32_t s = static_cast<int32_t>(sin16(stp + i * (65536L/TUNNEL_DETAIL)));
    int32_t v0[3] = {c/4, s/4, -10000 * 256L};
    int32_t v1[3] = {c, s, 120 * 256L};
    int32_t p0[3];
    int32_t p1[3];

    rotate(v0, p0);
    rotate(v1, p1);

    //translate vectors to coordinates
    scale_z(p0);
    scale_z(p1);

    //correct 3d perspective
    
    led_screen.perspective(p0);
    led_screen.perspective(p1);
    uint8_t hue = i * (256/TUNNEL_DETAIL);

    if (i!=TUNNEL_DETAIL) {
      draw_line_fine_hsv(led_screen, p0[0], p0[1], p1[0], p1[1], hue, 255, 0, -10000, 128, true);
      if ((i+stp2)%8 == 0 ) {
        draw_line_fine_hsv(led_screen, p0[0], p0[1], p1[0], p1[1], hue, 255, 16, -10000, 255, true);
      }
    }
    //draw_line_fine_hsv(CRGB crgb_object[], int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t hue = 0, uint8_t sat = 255, uint8_t val = 255, int z_depth = -10000, uint8_t val2 = 255)

    #define NUM_CIRCLES_TEST3D 4
    if ( i > 0 ) {
      for (int j = 0; j < NUM_CIRCLES_TEST3D; j++) {
        v1[2] = (stp4 - j * 65536)*5 - 130*256*9;
        old_v1[2] = (stp4 - j * 65536)*5 - 130*256*9;
        rotate(old_v1, p0);
        rotate(v1, p1);
        
        scale_z(p0);
        scale_z(p1);
        
        led_screen.perspective(p0);
        led_screen.perspective(p1);

        int thing = (255 - (255 / NUM_CIRCLES_TEST3D)) + (stp4/256) / NUM_CIRCLES_TEST3D - j * (255 / NUM_CIRCLES_TEST3D);
        thing = (thing*thing)/256L;

        draw_line_fine_hsv(
            led_screen, 
            p0[0], 
            p0[1], 
            p1[0], 
            p1[1], 
            i * (256/TUNNEL_DETAIL), 
            255, 
            thing,
            -10000, 
            thing , 
            true);
      }
    }

    old_v1[0] = v1[0];
    old_v1[1] = v1[1];
    old_v1[2] = v1[2];


  }

  stp  = millis()*3;       //rotation
  stp2 = millis()/64;    //bright light
  //stp3 = millis()/96; //circle width
  stp4 = millis()*128;   //tunnel speed

} //handle_tunnel();






}; //TEST3D





LIGHT_SKETCHES::REGISTER<TEST3D> test3d("test3d");
//3D

#endif