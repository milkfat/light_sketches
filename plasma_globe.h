#ifndef LIGHTS_PLASMA_GLOBE_H
#define LIGHTS_PLASMA_GLOBE_H

#define NUM_PLASMA_THINGS 15
#define NUM_PLASMA_SEGMENTS 4

#define PLASMA_WIDTH MATRIX_WIDTH
#define PLASMA_HEIGHT MATRIX_HEIGHT

class PLASMA_GLOBE: public LIGHT_SKETCH {
  public:

    PLASMA_GLOBE () {setup();}
    ~PLASMA_GLOBE () {}


  private:

    CRGB rgb = CRGB(84,84,156);
    CRGB rgb2 = CRGB(72,32,64);
    
    uint32_t global_cnt = 0;

    struct PLASMA_NODE {
        int32_t x; //current position including random noise
        int32_t y;
        int32_t ox; //original position without noise
        int32_t oy;
        int32_t tx; //target position (original position always moves toward this)
        int32_t ty;
        uint8_t t; //current time
    };

    struct PLASMA_THING {
        PLASMA_NODE nodes[NUM_PLASMA_SEGMENTS];
        uint32_t p;
        uint32_t r; //random value
        int n = 1;
    };

    PLASMA_THING things[NUM_PLASMA_THINGS];

  public:

    void setup() {
      control_variables.add(rgb, "Color");
      control_variables.add(rgb2, "Color 2");
      for (int i = 0; i < NUM_PLASMA_THINGS; i++) {
          things[i].p = random(PLASMA_WIDTH*256*2 + PLASMA_HEIGHT*256*2);
          things[i].r = random(UINT32_MAX);
          things[i].n = 1;
      }
    }

    void reset() {

    }

    void next_effect() {

    }

    void loop() {
        global_cnt++;
        for (int i = 0; i < NUM_PLASMA_THINGS; i++) {
            uint32_t cnt = global_cnt+things[i].r;
            int32_t x;
            int32_t y;
            int cnt2 = 2;
            while (cnt2--) {
                if (random(50) == 0) {
                    things[i].p = random(PLASMA_WIDTH*256*2 + PLASMA_HEIGHT*256*2);
                    things[i].r = random(UINT32_MAX);
                    things[i].n = 1;
                }
                things[i].p += (inoise8(cnt*4, i*2000, i*100)-128)/3;
                things[i].p += PLASMA_WIDTH*256*2 + PLASMA_HEIGHT*256*2;
                things[i].p %= PLASMA_WIDTH*256*2 + PLASMA_HEIGHT*256*2;
                if (things[i].p < PLASMA_HEIGHT*256) {
                    x = 0;
                    y = things[i].p;
                } else if (things[i].p < PLASMA_HEIGHT*256+PLASMA_WIDTH*256) {
                    x = things[i].p - PLASMA_HEIGHT*256;
                    y = PLASMA_HEIGHT*256;
                } else if (things[i].p < PLASMA_HEIGHT*256+PLASMA_WIDTH*256+PLASMA_HEIGHT*256) {
                    x = PLASMA_WIDTH*256;
                    y = PLASMA_HEIGHT*256 - (things[i].p - (PLASMA_HEIGHT*256 + PLASMA_WIDTH*256));
                } else {
                    x = PLASMA_WIDTH*256 - (things[i].p - (PLASMA_HEIGHT*256+PLASMA_WIDTH*256+PLASMA_HEIGHT*256));
                    y = 0;
                }
            }

            #define PLASMA_GLOBE_INNER_DIAMETER 6

            VECTOR3 unit_vec = VECTOR3((x-(PLASMA_WIDTH*256)/2)/256,(y-(PLASMA_HEIGHT*256)/2)/256,0).unit();
            VECTOR3 unit_vec2 = unit_vec;
            rotate_x(unit_vec2, inoise8(cnt, i*100, -i*100)/2-64);
            rotate_y(unit_vec2, inoise8(-i*100, cnt, i*100)/2-64);
            rotate_y(unit_vec2, inoise8(i*100, -i*100,cnt)/2-64);
            //inner sphere is static:
            int32_t origin_x = (PLASMA_WIDTH*256)/2 + unit_vec2.x*PLASMA_GLOBE_INNER_DIAMETER;
            int32_t origin_y = (PLASMA_HEIGHT*256)/2 + unit_vec2.y*PLASMA_GLOBE_INNER_DIAMETER;
            //or inner sphere includes some motion:
            //int32_t origin_x = (PLASMA_WIDTH*256)/2 + (inoise8(cnt, 0, 0)-128)*32 + unit_vec2.x*PLASMA_GLOBE_INNER_DIAMETER;
            //int32_t origin_y = (PLASMA_HEIGHT*256)/2 + (inoise8(0, cnt, 0)-128)*192 + unit_vec2.y*PLASMA_GLOBE_INNER_DIAMETER;

            int32_t x_dist = (x - origin_x)/(NUM_PLASMA_SEGMENTS+1);
            int32_t y_dist = (y - origin_y)/(NUM_PLASMA_SEGMENTS+1);


            #define PLASMA_GLOBE_SEGMENT_SPEED 2
            for (int j = 0; j < NUM_PLASMA_SEGMENTS; j++) {
                things[i].nodes[j].tx = origin_x + x_dist*(j+1);
                things[i].nodes[j].ty = origin_y + y_dist*(j+1);
                if (things[i].n) {
                    things[i].nodes[j].ox = things[i].nodes[j].tx;
                    things[i].nodes[j].oy = things[i].nodes[j].ty;
                } else {
                    things[i].nodes[j].ox += (things[i].nodes[j].tx-things[i].nodes[j].ox)/15;
                    things[i].nodes[j].oy += (things[i].nodes[j].ty-things[i].nodes[j].oy)/15;
                }
                //things[i].nodes[j].x = things[i].nodes[j].ox + ((inoise8(cnt*PLASMA_GLOBE_SEGMENT_SPEED,j*700,i*1400)-128)*_max(abs(x_dist),abs(y_dist)))/256;
                //things[i].nodes[j].y = things[i].nodes[j].oy + ((inoise8(cnt*PLASMA_GLOBE_SEGMENT_SPEED,j*400,i*131)-128)*_max(abs(x_dist),abs(y_dist)))/256;
                things[i].nodes[j].ox += (inoise8(cnt*PLASMA_GLOBE_SEGMENT_SPEED,j*700,i*1400)-128)*_max(abs(x_dist),abs(y_dist))/2048;
                things[i].nodes[j].oy += (inoise8(cnt*PLASMA_GLOBE_SEGMENT_SPEED,j*400,i*131)-128)*_max(abs(x_dist),abs(y_dist))/2048;
                things[i].nodes[j].x = things[i].nodes[j].ox;
                things[i].nodes[j].y = things[i].nodes[j].oy;
                things[i].nodes[j].x = _min(_max(0, things[i].nodes[j].x), (PLASMA_WIDTH-1)*256); 
                things[i].nodes[j].y = _min(_max(0, things[i].nodes[j].y), (PLASMA_HEIGHT-1)*256); 
            }

            if (things[i].n) {
                things[i].n = 0;
            }


            int32_t coords[NUM_PLASMA_SEGMENTS+2][2];
            uint32_t current_coord = 0;

            //draw first (inner) segment
            //draw_line_fine(led_screen, origin_x, origin_y, things[i].nodes[0].x,things[i].nodes[0].y, rgb, -10000, 255, 255, true, true, true, true);
            coords[current_coord][0] = origin_x;
            coords[current_coord][1] =  origin_y;
            current_coord++;

            //draw middle segments
            for (int j = 0; j < NUM_PLASMA_SEGMENTS; j++) {
                //draw_line_fine(led_screen, things[i].nodes[j].x,things[i].nodes[j].y, things[i].nodes[j+1].x,things[i].nodes[j+1].y, rgb, -10000, 255, 255, true, true, true, true);
                coords[current_coord][0] = things[i].nodes[j].x;
                coords[current_coord][1] =  things[i].nodes[j].y;
                current_coord++;
            }

            //draw final (outer) segment
            //draw_line_fine(led_screen, things[i].nodes[NUM_PLASMA_SEGMENTS-1].x,things[i].nodes[NUM_PLASMA_SEGMENTS-1].y, x,y, rgb, -10000, 255, 255, true, true, true, true);
            coords[current_coord][0] = x;
            coords[current_coord][1] = y;

            current_coord++;

            if (x == 0 || x == MATRIX_WIDTH*256) {
                coords[current_coord][0] = x+x_dist;
                coords[current_coord][1] = y;
            } else if (y == 0 || y == MATRIX_HEIGHT*256) {
                coords[current_coord][0] = x;
                coords[current_coord][1] = y+y_dist;
            } 

            matt_curve8(led_screen, coords, NUM_PLASMA_SEGMENTS+2, rgb, false, false, true, 255, 32, true);

            // int32_t tail_coords[4][2];

            // tail_coords[3][0] = coords[NUM_PLASMA_SEGMENTS-1][0];
            // tail_coords[3][1] = coords[NUM_PLASMA_SEGMENTS-1][1];
            // tail_coords[2][0] = coords[NUM_PLASMA_SEGMENTS][0];
            // tail_coords[2][1] = coords[NUM_PLASMA_SEGMENTS][1];
            // tail_coords[1][0] = coords[NUM_PLASMA_SEGMENTS+1][0];
            // tail_coords[1][1] = coords[NUM_PLASMA_SEGMENTS+1][1];
            // tail_coords[0][0] = coords[NUM_PLASMA_SEGMENTS+2][0];
            // tail_coords[0][1] = coords[NUM_PLASMA_SEGMENTS+2][1];

            // for (int j = 0; j < 4; j++) {
            //     if (x == 0 || x == MATRIX_WIDTH*256) {
            //         tail_coords[1][0] = coords[NUM_PLASMA_SEGMENTS+1][0];
            //         tail_coords[1][1] = coords[NUM_PLASMA_SEGMENTS+1][1]+random(-512,512);
            //     } else if (y == 0 || y == MATRIX_HEIGHT*256) {
            //         tail_coords[1][0] = coords[NUM_PLASMA_SEGMENTS+1][0]+random(-512,512);
            //         tail_coords[1][1] = coords[NUM_PLASMA_SEGMENTS+1][1]+y_dist;
            //     } 
                
            //     matt_curve8(led_screen, tail_coords, 4, nblend(rgb,rgb2,_min((256*(j+1))/3,255)), false, false, true, 160+(35*(3-j))/3, 32, true);
            // }

            //draw fuzzy electricity around center sphere
            for (int j = 0; j < 4; j++) {
                VECTOR3 vec = unit_vec;
                rotate_z(vec, random(-50, 50));
                uint8_t dist = random(PLASMA_GLOBE_INNER_DIAMETER);
                dist = (dist*dist)/(PLASMA_GLOBE_INNER_DIAMETER+1);
                dist = _max(1, dist);
                draw_line_fine(led_screen, origin_x, origin_y, origin_x-random(_min(0,vec.x*dist),_max(0,vec.x*dist)),origin_y-random(_min(0,vec.y*dist),_max(0,vec.y*dist)), rgb2, -10000, 255, 255, true, true, true, true);
            }
        }

        LED_show();
        LED_black();
    }//loop

};




LIGHT_SKETCHES::REGISTER<PLASMA_GLOBE> plasma_globe("plasma globe");

#endif