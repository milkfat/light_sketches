#ifndef LIGHTS_UNTITLED_H
#define LIGHTS_UNTITLED_H

#define NUMBER_OF_CELLS 100
#define QUEUE_SIZE 1000

#define BORDER_BUFFER 48
#define LIGHTNING_SPEED 4

class UNTITLED: public LIGHT_SKETCH {

    #define NUMBER_OF_THINGS (NUMBER_OF_CELLS)

    //int16_t height_map[HEIGHTMAP_HEIGHT][HEIGHTMAP_WIDTH];

    struct thing {
        VECTOR2 pos; //the absolute position on the screen, calculated from virtual position
        VECTOR2 pos2; //the virtual position, this is the variable to be adjusted by spd
        VECTOR2_8s spd;
        CRGB rgb;
        uint32_t max_distance;
        uint32_t new_max_distance;
        bool on_screen = false;
    };

    struct particle {
        VECTOR2 pos;
        VECTOR2 spd;
        uint8_t age=0;
    };

    #define NUM_SPARKS 255
    particle particles[NUM_SPARKS];
    uint8_t current_particle;
    bool lightning_var = 1; //1 = timer based lightning
    uint8_t lightning_cnt = 0; //how many lightning bolts have spawned
    uint8_t lightning_continue = 0; //the number frames to continue processing lightning
    bool button_status = 0;
    bool button_down = 0;
    bool do_lightning = 0;

    uint16_t add_particle(VECTOR2 pos, VECTOR2 spd) {
        uint16_t cp = current_particle;
        particles[cp].pos = pos;
        particles[cp].spd = spd;
        particles[cp].age = 255;
        current_particle++;
        current_particle %= NUM_SPARKS;
        return cp;

    }

    thing things[NUMBER_OF_THINGS+1];

    
    uint8_t queue[QUEUE_SIZE][2];
    int16_t queue_read = 0;
    int16_t queue_write = 0;

    struct PIXEL_DATA {

        uint16_t dist = UINT16_MAX; //distance of t from this pixel
        int8_t t:8; //which thing has the shortest distance
        uint8_t age:8;
        uint8_t edge_value:8;
        uint8_t edge_distance:3;
        uint8_t edge_cnt:3;
        bool corner:1;
        bool edge:1;

        inline void set_dist(uint32_t d) {
            dist = d>>6;
        }

        inline uint32_t get_dist() {
            return dist<<6;
        }
    };

    PIXEL_DATA grid[MATRIX_HEIGHT][MATRIX_WIDTH];

  public:
    UNTITLED () {
        setup();
        control_variables.add(lightning_var,"Lightning");
        control_variables.add(button_status,"Thor!", 1);
    }
    ~UNTITLED () {}

    void next_effect() {}

    void reset() {
        setup();
    }
    void setup() {
        //height_map_ptr = &height_map;
        for (int y = 0; y < MATRIX_HEIGHT; y++) {  
            for (int x = 0; x < MATRIX_WIDTH; x++) {
                grid[y][x].t = -1;
                grid[y][x].age = 0;
                grid[y][x].edge_distance = 0;
                grid[y][x].edge_value = 0;
                grid[y][x].edge_cnt = 0;
                grid[y][x].corner = false;
                grid[y][x].edge = false;
            }
        }
        for (int i = 0; i < NUMBER_OF_CELLS; i++) {
            things[i].pos2.y = random(-BORDER_BUFFER*256,(MATRIX_HEIGHT+BORDER_BUFFER)*256);
            things[i].pos2.x = random(-BORDER_BUFFER*256,(MATRIX_WIDTH+BORDER_BUFFER)*256);
            things[i].spd.y = random(-14,15);
            things[i].spd.x = random(-14,15);
            things[i].rgb = CHSV(random(256), 255, random(32,256));
           // things[i].rgb = CHSV(0, 0, random(2)*255); //black and white
        }
  
    }
    

    void loop() {
        if (button_status && !button_down) {
            button_down = 1;
            do_lightning = 1;
        }
        if (!button_status && button_down) {
            button_down = 0;
        }

        LED_show();
        LED_black();

        //update our things
        for (uint8_t i = 0; i < NUMBER_OF_CELLS; i++) {
            if (things[i].on_screen) {
                things[i].max_distance = (things[i].max_distance*2+things[i].new_max_distance)/3;
            } else {
                things[i].max_distance = UINT32_MAX;
            }
            things[i].on_screen = false;
            things[i].new_max_distance = 0;
            things[i].pos2.x += things[i].spd.x;
            if (things[i].pos2.x < -BORDER_BUFFER*256) {
                things[i].pos2.x += (BORDER_BUFFER*2-1)*256+MATRIX_WIDTH*256;
            }
            if (things[i].pos2.x > (MATRIX_WIDTH+BORDER_BUFFER)*256) {
                things[i].pos2.x -= (BORDER_BUFFER*2-1)*256+MATRIX_WIDTH*256;
            }
            things[i].pos2.y += things[i].spd.y;
            if (things[i].pos2.y < -BORDER_BUFFER*256) {
                things[i].pos2.y += (BORDER_BUFFER*2-1)*256+MATRIX_HEIGHT*256;
            }
            if (things[i].pos2.y > (MATRIX_HEIGHT+BORDER_BUFFER)*256) {
                things[i].pos2.y -= (BORDER_BUFFER*2-1)*256+MATRIX_HEIGHT*256;
            }
            things[i].pos.x = things[i].pos2.x;
            things[i].pos.y = things[i].pos2.y;
            //accelerate things away from the screen to avoid popping
            if (things[i].pos2.x < 0) {
                uint32_t overflow = abs(things[i].pos2.x);
                things[i].pos.x -= (overflow*overflow)/1024;
            } else if (things[i].pos2.x > MATRIX_WIDTH*256) {
                uint32_t overflow = things[i].pos2.x-MATRIX_WIDTH*256;
                things[i].pos.x += (overflow*overflow)/1024;
            }

            if (things[i].pos2.y < 0) {
                uint32_t overflow = abs(things[i].pos2.y);
                things[i].pos.y -= (overflow*overflow)/1024;
            } else if (things[i].pos2.y > MATRIX_HEIGHT*256) {
                uint32_t overflow = things[i].pos2.y-MATRIX_HEIGHT*256;
                things[i].pos.y += (overflow*overflow)/1024;
            }

            // for (int j = i+1; j < NUMBER_OF_CELLS; j++) {
            //     int dist_x = things[i].pos.x - things[j].pos.x;
            //     int dist_y = things[i].pos.y - things[j].pos.y;
            //     int mag_x = 5*256 - abs(dist_x);
            //     mag_x = (mag_x * mag_x)/(5*256);
            //     int mag_y = 5*256 - abs(dist_y);
            //     //mag_y = (mag_y * mag_y)/(5*256);
            //     if (abs(dist_x) < 5*256 && abs(dist_y) < 5*256) {
            //         things[i].pos.x += (sgn(dist_x)*mag_x*18)/(5*256);
            //         things[i].pos.y += (sgn(dist_y)*mag_y*18)/(5*256);
            //         things[j].pos.x += -(sgn(dist_x)*mag_x*18)/(5*256);
            //         things[j].pos.y += -(sgn(dist_y)*mag_y*18)/(5*256);
            //     }
            // }

        }

        //draw each thing to a grid of the 9 closest LEDs
        for (uint8_t i = 0; i < NUMBER_OF_THINGS; i++) {
            int32_t x = things[i].pos.x;
            int32_t y = things[i].pos.y;
            int32_t x2 = _max(_min(x/256, MATRIX_WIDTH-1),0);
            int32_t y2 = _max(_min(y/256, MATRIX_HEIGHT-1),0);

            //draw to a 3x3 grid
            for (int32_t y3 = y2-1; y3 <= y2+1; y3++) {
                for (int32_t x3 = x2-1; x3 <= x2+1; x3++) {
                    if (y3 >= 0 && y3 < MATRIX_HEIGHT && x3 >= 0 && x3 < MATRIX_WIDTH) {
                        uint32_t a = _min(abs(x - x3*256),UINT16_MAX*2)/8; //limit to avoid overflow
                        uint32_t b = _min(abs(y - y3*256),UINT16_MAX*2)/8;
                        //int c = sqrt(a*a + b*b);
                        uint32_t c = a*a + b*b;
                        //the closest thing is assigned to each pixel
                        //if two things are the exact same distance from a pixel, then we pick the first one (from the thing array) 
                        if (c < grid[y3][x3].get_dist() || (c == grid[y3][x3].get_dist() && i > grid[y3][x3].t)) {
                            //record thing and distance
                            grid[y3][x3].t = i;
                            grid[y3][x3].set_dist(c);
                            //add this pixel to our queue for reprocessing
                            queue[queue_write][0] = x3;
                            queue[queue_write][1] = y3;
                            queue_write++;
                            queue_write%=QUEUE_SIZE;
                        }
                    }
                }
            }
        }


        //go through the LED matrix and fill it in (drawing outward from each recent pixel)
        //we will step through the matrix multiple times until each pixel is illuminated
        
        while (queue_read != queue_write) {
            int y = queue[queue_read][1];
            int x = queue[queue_read][0];
            queue_read++;
            queue_read%=QUEUE_SIZE;

            //an array of adjacent pixel coordinates (above, below, left, right)
            uint16_t l[4][2];
            l[0][0] = x-1;
            l[0][1] = y;
            l[1][0] = x+1;
            l[1][1] = y;
            l[2][0] = x;
            l[2][1] = y-1;
            l[3][0] = x;
            l[3][1] = y+1;

            int this_t = grid[y][x].t;
            
            grid[y][x].edge = 0;
            //check each adjacent pixel for the distance of the current pixel's thing
            for (int i = 0; i < 4; i++) {
                int x2 = l[i][0];
                int y2 = l[i][1];
                //is the pixel on the grid?
                //is the thing different than the current pixel's thing?
                if (y2 >= 0 && y2 < MATRIX_HEIGHT && x2 >= 0 && x2 < MATRIX_WIDTH && this_t != grid[y2][x2].t) {
                    
                    //calculate the distance between thing and pixel
                    uint32_t a = _min(abs(things[this_t].pos.x - x2*256),UINT16_MAX*2)/8;
                    uint32_t b = _min(abs(things[this_t].pos.y - y2*256),UINT16_MAX*2)/8;
                    uint32_t c = a*a + b*b;
                    //are we closer?
                    if (c < grid[y2][x2].get_dist()) {
                        //record information to the grid
                        grid[y2][x2].t = this_t;
                        grid[y2][x2].set_dist(c);
                        //add this pixel to the queue to be processed
                        queue[queue_write][0] = x2;
                        queue[queue_write][1] = y2;
                        queue_write++;
                        queue_write%=QUEUE_SIZE;
                        grid[y2][x2].set_dist(c);
                        leds[XY(x2,y2)] = things[this_t].rgb;
                    } else {
                        grid[y][x].edge = 1;
                        grid[y2][x2].edge = 1;
                    }
                }
            }


        }




        //edges/anti-aliasing
        //step through each pixel and find edges (does the adjacent pixel contain a different thing)
        bool do_debug = 1;
        for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
            for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {

                //shading
                CRGB rgb = gamma8_encode(get_shade(x,y));
                leds[XY(x,y)] = rgb;
                things[grid[y][x].t].on_screen = true;

                if (!grid[y][x].edge) {continue;}
                //array of adjacent pixel coordinates
                uint16_t l[4][2];
                l[0][0] = x+1;
                l[0][1] = y;
                l[1][0] = x;
                l[1][1] = y+1;
                l[2][0] = x-1;
                l[2][1] = y;
                l[3][0] = x;
                l[3][1] = y-1;

                //step through each adjacent pixel to see if we need to do any blending 
                //we will do our blending:
                    //1) imagine a line between our two things
                    //2) imagine a line between our two pixels
                    //3) find the coordinate M (midpoint) where our two lines intersect
                    //4) find the distance between M and our pixel (0-255)
                    //5) use that distance as our blending value (which pixel is closest to the line)
                    //  0 = 100% this pixel
                    //  255 = 100% adjacent pixel
                int cnt = 0;
                int16_t first_thing;

                grid[y][x].edge_distance = 0;
                for (int i = 0; i < 4; i++) {
                    int x2 = l[i][0];
                    int y2 = l[i][1];
                    //is the current pixel on the screen
                    if (y2 >= 0 && y2 < MATRIX_HEIGHT && x2 >= 0 && x2 < MATRIX_WIDTH && grid[y][x].t != grid[y2][x2].t) {
                        if (cnt == 0) {
                            first_thing = grid[y2][x2].t;
                        } else if (grid[y2][x2].t != first_thing) {
                            grid[y][x].corner = true;
                        }
                        grid[y][x].edge_distance = 1;

                        static uint32_t lightning_led = random(NUM_LEDS);
                        if(lightning_var) {
                            //spawn lightning on a timed basis
                            static uint32_t lightning_time = 0;
                            if (millis() > lightning_time && XY(x,y) > lightning_led) {
                                spawn_lightning(x,y);
                                lightning_time = millis() + random(10000);
                                lightning_led = random(NUM_LEDS);
                            }
                        }

                        if(do_lightning) {
                            //spawn lightning on demand
                            if (XY(x,y) > lightning_led) {
                                spawn_lightning(x,y);
                                do_lightning = 0;
                                lightning_led = random(NUM_LEDS);
                            }
                        }



                        //calculate the midpoint between the two things
                        int x_diff = things[grid[y2][x2].t].pos.x-things[grid[y][x].t].pos.x;
                        int y_diff = things[grid[y2][x2].t].pos.y-things[grid[y][x].t].pos.y;
                        int x_mid = (things[grid[y2][x2].t].pos.x+things[grid[y][x].t].pos.x)/2;
                        int y_mid = (things[grid[y2][x2].t].pos.y+things[grid[y][x].t].pos.y)/2;

                        //calculate the slope of the line between the two things
                        // float slope = 0;
                        // if (x_diff != 0) {
                        //     slope = (1.f*y_diff)/(1.f*x_diff);
                        // }
                        //calculate the perpendicular slope
                        //float neg_slope = 0;
                        int neg_slope_x = 1;
                        int neg_slope_y = 1;
                        if (y_diff != 0) {
                            neg_slope_y = y_diff;
                        }
                        if (x_diff != 0) {
                            neg_slope_x = -x_diff;
                        }
                        //calculate the intercept
                        int intercept = y_mid - (x_mid * neg_slope_x)/neg_slope_y;
                        //now we can calculate the absolute coordinates at which the line intersects with each side of our pixel
                        #define OFFSET 128
                        int y_actual = ((x*256-OFFSET)*neg_slope_x)/neg_slope_y+intercept;
                        int x_actual = (((y*256-OFFSET)-intercept)*neg_slope_y)/neg_slope_x;
                        int y2_actual = (((x+1)*256-OFFSET)*neg_slope_x)/neg_slope_y+intercept;
                        int x2_actual = ((((y+1)*256-OFFSET)-intercept)*neg_slope_y)/neg_slope_x;
 
                        
                        //record the max distance of the cell boundary from the cell
                        if (abs(neg_slope_x) < abs(neg_slope_y)) {
                            //drawXY_fine(led_screen, x*256, y_actual, 0, 255, 255);
                            int y_actual_no_offset = ((x*256)*neg_slope_x)/neg_slope_y+intercept;
                            int x_diff2 = x*256 - things[grid[y][x].t].pos.x;
                            x_diff2 = _min(abs(x_diff2),UINT16_MAX*2)/8;
                            int y_diff2 = y_actual_no_offset - things[grid[y][x].t].pos.y;
                            y_diff2 = _min(abs(y_diff2),UINT16_MAX*2)/8;
                            int max_ = (x_diff2)*(x_diff2) + (y_diff2)*(y_diff2);
                            things[grid[y][x].t].new_max_distance = _max(things[grid[y][x].t].new_max_distance, max_);
                        } else {
                            //drawXY_fine(led_screen, x_actual, y*256, 160, 255, 255);
                            int x_actual_no_offset = (((y*256)-intercept)*neg_slope_y)/neg_slope_x; 
                            int x_diff2 = x_actual_no_offset - things[grid[y][x].t].pos.x;
                            x_diff2 = _min(abs(x_diff2),UINT16_MAX*2)/8;
                            int y_diff2 = y*256 - things[grid[y][x].t].pos.y;
                            y_diff2 = _min(abs(y_diff2),UINT16_MAX*2)/8;
                            int max_ = (x_diff2)*(x_diff2) + (y_diff2)*(y_diff2);
                            things[grid[y][x].t].new_max_distance = _max(things[grid[y][x].t].new_max_distance, max_);
                        }
                        //drawXY_fine(led_screen, x2_actual_no_offset, y2_actual_no_offset, 96, 255, 255);

                        //LED_show();
                        //things[grid[y2][x2].t].new_max_distance = _max(things[grid[y2][x2].t].new_max_distance, max_);
                        //things[grid[y][x].t].new_max_distance = 100000;
                        //things[grid[y2][x2].t].new_max_distance = 100000;
                        //find the relative intersect position along each side
                        //0 = one end ("left")
                        //1-255 = somewhere in the middle
                        //256 = other end ("right")
                        int y_a = y_actual - (y*256-OFFSET);
                        int x_a = x_actual - (x*256-OFFSET);
                        int y_b = y2_actual - (y*256-OFFSET);
                        int x_b = x2_actual - (x*256-OFFSET);

                        //test to make sure the intersect is within the pixel boundaries
                        int ya_test = y_a >= 0 && y_a <= 256;
                        int yb_test = y_b >= 0 && y_b <= 256;
                        int xa_test = x_a >= 0 && x_a <= 256;
                        int xb_test = x_b >= 0 && x_b <= 256;

                        //get the two potential color values

                        //shading
                        CRGB rgb0 = get_shade(x,y);

                        CRGB rgb1 = get_shade(x2,y2);

                        CRGB rgb_new = rgb0;
                        
                        int b = 0;
                        grid[y][x].edge_value = 0;

                        if (ya_test && yb_test) {
                            //split top-bottom
                            int b0 = y_a;
                            int b1 = y_b;
                            b0 = _max(_min(b0,255),0);
                            b1 = _max(_min(b1,255),0);
                            b = (b0+b1)/2;
                            if (things[grid[y][x].t].pos.y > things[grid[y2][x2].t].pos.y) {
                                b = 255-b;
                            }
                            rgb_new = (nblend(rgb1, rgb0, b));
                        } else

                        if (xa_test && xb_test) {
                            //split left-right
                            int b0 = x_a;
                            int b1 = x_b;
                            b0 = _max(_min(b0,255),0);
                            b1 = _max(_min(b1,255),0);
                            b = (b0+b1)/2;
                            if (things[grid[y][x].t].pos.x > things[grid[y2][x2].t].pos.x) {
                                b = 255-b;
                            }
                            rgb_new = (nblend(rgb1, rgb0, b));
                        } else
                        if (yb_test && xa_test) {
                            //lower right corner
                            int b0 = y_b;
                            int b1 = 256-x_a;
                            b0 = _max(_min(b0,255),0);
                            b1 = _max(_min(b1,255),0);
                            b = (b0*b1)/2;
                            b /= 256;
                            if (things[grid[y][x].t].pos.x < things[grid[y2][x2].t].pos.x) {
                                b = 255-b;
                            }
                            rgb_new = (nblend(rgb1, rgb0, b));
                            //leds[XY(x,y)] = CRGB::Blue;

                        } else
                        if (ya_test && xb_test) {
                            //upper left corner
                            int b0 = 256-y_a;
                            int b1 = x_b;
                            b0 = _max(_min(b0,255),0);
                            b1 = _max(_min(b1,255),0);
                            b = (b0*b1)/2;
                            b /= 256;
                            if (things[grid[y][x].t].pos.x > things[grid[y2][x2].t].pos.x) {
                                b = 255-b;
                            }
                            rgb_new = (nblend(rgb1, rgb0, b));
                            //leds[XY(x,y)] = CRGB::Green;
                        } else
                        if (ya_test && xa_test) {
                            //lower left corner
                            int b0 = y_a;
                            int b1 = x_a;
                            b0 = _max(_min(b0,255),0);
                            b1 = _max(_min(b1,255),0);
                            b = (b0*b1)/2;
                            b /= 256;
                            if (things[grid[y][x].t].pos.x > things[grid[y2][x2].t].pos.x) {
                                b = 255-b;
                            }
                            rgb_new = (nblend(rgb1, rgb0, b));
                            //leds[XY(x,y)] = CRGB::Red;


                        } else
                        if (yb_test && xb_test) {
                            //upper right corner
                            int b0 = 256-y_b;
                            int b1 = 256-x_b;
                            b0 = _max(_min(b0,255),0);
                            b1 = _max(_min(b1,255),0);
                            b = (b0*b1)/2;
                            b /= 256;
                            if (things[grid[y][x].t].pos.x < things[grid[y2][x2].t].pos.x) {
                                b = 255-b;
                            }
                            rgb_new = (nblend(rgb1, rgb0, b));
                            //leds[XY(x,y)] = CRGB::White;

                        } else {
                            //rare cases ... glitches?
                            if (false && y==y2) {
                                if (x_actual < (x*256-OFFSET)) {
                                    if (things[grid[y][x].t].pos.x >= things[grid[y2][x2].t].pos.x) {
                                        rgb_new = (rgb0);
                                    } else {
                                        rgb_new = (rgb1);
                                    }
                                } else if (x_actual >= ((x+1)*256-OFFSET)) {
                                    if (things[grid[y][x].t].pos.x >= things[grid[y2][x2].t].pos.x) {
                                        rgb_new = (rgb1);
                                    } else {
                                        rgb_new = (rgb0);
                                    }
                                }

                            }
                            if (x==x2) {
                                if (y_actual <= (y*256-OFFSET)) {
                                    if (things[grid[y][x].t].pos.y >= things[grid[y2][x2].t].pos.y) {
                                        rgb_new = (rgb0);
                                    } else {
                                        rgb_new = (rgb1);
                                    }
                                } else if (y_actual >= ((y+1)*256-OFFSET)) {
                                    if (things[grid[y][x].t].pos.y >= things[grid[y2][x2].t].pos.y) {
                                        rgb_new = rgb1;
                                    } else {
                                        rgb_new = rgb0;
                                    }
                                }
                            }
                        
                        }

                        //get the color of each pixel in linear color space
                        



                        cnt++;
                        CRGB rgb_old = gamma8_decode(leds[XY(x,y)]);
                        leds[XY(x,y)] = gamma8_encode(nblend(rgb_old, rgb_new, 255/cnt));

                        if (lightning_continue) {
                            //figure out some lightning shading stuff
                            grid[y][x].edge_value = _min(_max(abs((127-b))*2, 0), 255);
                            grid[y][x].edge_value = 255-grid[y][x].edge_value;
                            grid[y][x].edge_value = _min(grid[y][x].edge_value*4, 255);
                        }
                    }
                }



            }
        }

        if (lightning_continue) {
            lightning_continue--;
            //calculate lightning in the forward direction
            for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
                for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {

                    //lightning
                    if(grid[y][x].age) {
                        int amount = (grid[y][x].age * grid[y][x].edge_value)/255;
                        leds[XY(x,y)].r = qadd8(leds[XY(x,y)].r,amount);
                        leds[XY(x,y)].g = qadd8(leds[XY(x,y)].g,amount);
                        leds[XY(x,y)].b = qadd8(leds[XY(x,y)].b,amount);
                        grid[y][x].age = (grid[y][x].age*6)/7;
                        lightning_continue = 2;
                    }

                    if (!grid[y][x].edge) {continue;}
                    //array of adjacent pixel coordinates
                    uint16_t l[4][2];
                    l[0][0] = x+1;
                    l[0][1] = y;
                    l[1][0] = x;
                    l[1][1] = y+1;
                    l[2][0] = x-1;
                    l[2][1] = y;
                    l[3][0] = x;
                    l[3][1] = y-1;

                    for (int i = 0; i < 4; i++) {
                        int x2 = l[i][0];
                        int y2 = l[i][1];
                        if (y2 >= 0 && y2 < MATRIX_HEIGHT && x2 >= 0 && x2 < MATRIX_WIDTH && grid[y][x].edge_distance && grid[y2][x2].edge_distance) {
                            if (grid[y][x].age == 0 && grid[y2][x2].age > 150 && grid[y][x].edge_cnt < grid[y2][x2].edge_cnt && grid[y2][x2].edge_distance < LIGHTNING_SPEED) {
                                grid[y][x].age = 255;
                                grid[y][x].edge_distance = grid[y2][x2].edge_distance+1;
                                grid[y][x].edge_cnt = grid[y2][x2].edge_cnt;
                                if(grid[y][x].corner) {
                                    add_particle(VECTOR2(x*256,y*256), VECTOR2(random(-500,500), random(-250,250)));
                                }
                            }
                            if (grid[y2][x2].age == 0 && grid[y][x].age > 150 && grid[y][x].edge_cnt > grid[y2][x2].edge_cnt  && grid[y][x].edge_distance < LIGHTNING_SPEED) {
                                grid[y2][x2].age = 255;
                                grid[y2][x2].edge_distance = grid[y][x].edge_distance+1;
                                grid[y2][x2].edge_cnt = grid[y][x].edge_cnt;
                                if(grid[y2][x2].corner) {
                                    add_particle(VECTOR2(x2*256,y2*256), VECTOR2(random(-500,500), random(-250,250)));
                                }
                            }
                        }
                    }

                }
            }


            //calculate lightning in the reverse direction
            for (int y = MATRIX_HEIGHT-1; y >= 0; y--) {
                for (int x = MATRIX_WIDTH-1; x >= 0; x--) {
                    if (!grid[y][x].edge) {continue;}
                    //array of adjacent pixel coordinates
                    uint16_t l[4][2];
                    l[0][0] = x+1;
                    l[0][1] = y;
                    l[1][0] = x;
                    l[1][1] = y+1;
                    l[2][0] = x-1;
                    l[2][1] = y;
                    l[3][0] = x;
                    l[3][1] = y-1;

                    for (int i = 0; i < 4; i++) {
                        int x2 = l[i][0];
                        int y2 = l[i][1];
                        if (y2 >= 0 && y2 < MATRIX_HEIGHT && x2 >= 0 && x2 < MATRIX_WIDTH && grid[y][x].edge_distance && grid[y2][x2].edge_distance) {
                            if (grid[y][x].age == 0 && grid[y2][x2].age > 150 && grid[y][x].edge_cnt < grid[y2][x2].edge_cnt && grid[y2][x2].edge_distance < LIGHTNING_SPEED) {
                                grid[y][x].age = 255;
                                grid[y][x].edge_distance = grid[y2][x2].edge_distance+1;
                                grid[y][x].edge_cnt = grid[y2][x2].edge_cnt;
                                if(grid[y][x].corner) {
                                    add_particle(VECTOR2(x*256,y*256), VECTOR2(random(-500,500), random(-250,250)));
                                }
                            }
                            if (grid[y2][x2].age == 0 && grid[y][x].age > 150 && grid[y][x].edge_cnt > grid[y2][x2].edge_cnt  && grid[y][x].edge_distance < LIGHTNING_SPEED) {
                                grid[y2][x2].age = 255;
                                grid[y2][x2].edge_distance = grid[y][x].edge_distance+1;
                                grid[y2][x2].edge_cnt = grid[y][x].edge_cnt;
                                if(grid[y2][x2].corner) {
                                    add_particle(VECTOR2(x2*256,y2*256), VECTOR2(random(-500,500), random(-250,250)));
                                }
                            }
                        }
                    }

                }
            }

        }

        // for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
        //     for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
        //         height_map[y][x] = grid[y][x].dist*3;
        //     }
        // }

        //LED_black();
        //move heightmap to LEDs
        //height_map_to_LED(-128*256, 100, 100, -30, -20);

        //handle particles
        for (uint8_t i = 0; i < NUM_SPARKS; i++) {
            if (particles[i].age) {
                if (particles[i].pos.x < -10*256 || particles[i].pos.x > MATRIX_WIDTH*256+10*256) {
                    particles[i].age = 0;
                    continue;
                }
                uint8_t b = 128;
                if (particles[i].age < 64) {
                    b = particles[i].age*2;
                }
                VECTOR2 spd = particles[i].spd;
                spd /= 3;
                particles[i].pos += spd;
                blendXY(led_screen, particles[i].pos, CRGB(b,b,b));
                particles[i].pos += spd;
                blendXY(led_screen, particles[i].pos, CRGB(b,b,b));
                particles[i].pos += spd;
                blendXY(led_screen, particles[i].pos, CRGB(b,b,b));
                particles[i].age--;
                particles[i].spd.y -= 20;
                //hit the ground
                if(particles[i].pos.y < 0 && particles[i].spd.y < 0) {
                    int new_particle = 0;
                    if (particles[i].spd.y < -512 ) {
                        new_particle = abs(particles[i].spd.y);
                    }
                    particles[i].spd.y = (-particles[i].spd.y*random(4,6))/12;
                    particles[i].spd.x *= .8f;

                    if (new_particle) {
                        VECTOR2 spd = particles[i].spd;
                        //particles[i].spd.y *= .7f;
                        //spd.y *= .75f;
                        particles[i].spd.y -= random(particles[i].spd.y/4);
                        spd.y -= random(spd.y/4);
                        particles[i].spd.x = -random(10,new_particle/2);
                        spd.x = random(10,new_particle/2);
                        particles[add_particle(particles[i].pos, spd)].age = particles[i].age;
                    }

                }
                //fade quickly if resting on the ground
                if (abs(particles[i].spd.y) < 10 && particles[i].pos.y < 256) {
                    particles[i].age = (particles[i].age*7)/10;
                }
            }
        }

        //reset the grid
        for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
            for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
                grid[y][x].set_dist(UINT32_MAX);
                grid[y][x].t = -1;
                grid[y][x].corner = false;
                // if (grid[y][x].edge) {
                //     leds[XY(x,y)] = CRGB::White;
                // }
            }
        }


       
    }

    CRGB get_shade(uint16_t x, uint16_t y) {

        CRGB rgb = things[grid[y][x].t].rgb;
        int32_t max_dist = (things[grid[y][x].t].max_distance)/200;
        int32_t c = 0;
        if (max_dist != 0) {
            c = grid[y][x].get_dist()/max_dist;
        }
        c = _min(_max(255-c,32),255);
        rgb.r = (rgb.r*c)/255;
        rgb.g = (rgb.g*c)/255;
        rgb.b = (rgb.b*c)/255;
        return rgb;
        
    }

    void spawn_lightning(uint16_t x, uint16_t y) {
        grid[y][x].age=255;
        if (lightning_cnt < 7) {
            lightning_cnt++;
        } else {
            lightning_cnt = 0;
        }
        if (lightning_cnt == 0) {
            for (int y2 = 0; y2 < MATRIX_HEIGHT; y2++) {
                for (int x2 = 0; x2 < MATRIX_WIDTH; x2++) {
                    grid[y2][x2].edge_cnt = 0;
                }
            }
            lightning_cnt = 1;
        }
        grid[y][x].edge_cnt=lightning_cnt;
        lightning_continue = 20;
    }

};   


LIGHT_SKETCHES::REGISTER<UNTITLED> untitled("untitled");

#endif