#ifndef LIGHTS_UNTITLED_H
#define LIGHTS_UNTITLED_H

#define NUMBER_OF_CELLS 100
#define QUEUE_SIZE 4000

#define BORDER_BUFFER 48

class UNTITLED: public LIGHT_SKETCH {

    #define NUMBER_OF_THINGS (NUMBER_OF_CELLS)

    struct thing {
        VECTOR3 pos;
        VECTOR3 spd;
        CRGB rgb;
    };

    thing things[NUMBER_OF_THINGS+1];
    
    uint8_t queue[QUEUE_SIZE][2];
    int16_t queue_read = 0;
    int16_t queue_write = 0;

    struct PIXEL_DATA {
        int8_t t = -1; //which thing has the shortest distance
        uint32_t dist = UINT32_MAX; //distance of t from this pixel
    };

    PIXEL_DATA grid[MATRIX_HEIGHT][MATRIX_WIDTH];

  public:
    UNTITLED () {setup();}
    ~UNTITLED () {}

    void next_effect() {}

    void reset() {
        setup();
    }
    void setup() {
        for (int i = 0; i < NUMBER_OF_CELLS; i++) {
            things[i].pos.y = random(-BORDER_BUFFER*256,(MATRIX_HEIGHT+BORDER_BUFFER)*256);
            things[i].pos.x = random(-BORDER_BUFFER*256,(MATRIX_WIDTH+BORDER_BUFFER)*256);
            things[i].spd.y = random(-13,14);
            things[i].spd.x = random(-13,14);
            things[i].rgb = CHSV(random(256), 255, random(32,256));
        }
  
    }
    

    void loop() {
        LED_show();
        LED_black();

        //update our things
        for (int i = 0; i < NUMBER_OF_CELLS; i++) {
            things[i].pos.x += things[i].spd.x;
            if (things[i].pos.x < -BORDER_BUFFER*256) {
                things[i].pos.x += (BORDER_BUFFER*2-1)*256+MATRIX_WIDTH*256;
            }
            if (things[i].pos.x > (MATRIX_WIDTH+BORDER_BUFFER)*256) {
                things[i].pos.x -= (BORDER_BUFFER*2-1)*256+MATRIX_WIDTH*256;
            }
            things[i].pos.y += things[i].spd.y;
            if (things[i].pos.y < -BORDER_BUFFER*256) {
                things[i].pos.y += (BORDER_BUFFER*2-1)*256+MATRIX_HEIGHT*256;
            }
            if (things[i].pos.y > (MATRIX_HEIGHT+BORDER_BUFFER)*256) {
                things[i].pos.y -= (BORDER_BUFFER*2-1)*256+MATRIX_HEIGHT*256;
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
        for (int i = 0; i < NUMBER_OF_THINGS; i++) {
            int32_t x = things[i].pos.x;
            int32_t y = things[i].pos.y;
            int32_t x2 = _max(_min(x/256, MATRIX_WIDTH-1),0);
            int32_t y2 = _max(_min(y/256, MATRIX_HEIGHT-1),0);

            //draw to a 3x3 grid
            for (int32_t y3 = y2-1; y3 <= y2+1; y3++) {
                for (int32_t x3 = x2-1; x3 <= x2+1; x3++) {
                    if (y3 >= 0 && y3 < MATRIX_HEIGHT && x3 >= 0 && x3 < MATRIX_WIDTH) {
                        uint32_t a = _min(abs(x - x3*256),UINT16_MAX/4); //limit to avoid overflow
                        uint32_t b = _min(abs(y - y3*256),UINT16_MAX/4);
                        //int c = sqrt(a*a + b*b);
                        uint32_t c = a*a + b*b;
                        //the closest thing is assigned to each pixel
                        //if two things are the exact same distance from a pixel, then we pick the first one (from the thing array) 
                        if (c < grid[y3][x3].dist || (c == grid[y3][x3].dist && i > grid[y3][x3].t)) {
                            //record thing and distance
                            grid[y3][x3].t = i;
                            grid[y3][x3].dist = c;
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
            
            //check each adjacent pixel for the distance of the current pixel's thing
            for (int i = 0; i < 4; i++) {
                int x2 = l[i][0];
                int y2 = l[i][1];
                //is the pixel on the grid?
                //is the thing different than the current pixel's thing?
                if (y2 >= 0 && y2 < MATRIX_HEIGHT && x2 >= 0 && x2 < MATRIX_WIDTH && this_t != grid[y2][x2].t) {
                    //calculate the distance between thing and pixel
                    uint32_t a = _min(abs(things[this_t].pos.x - x2*256),UINT16_MAX/4);
                    uint32_t b = _min(abs(things[this_t].pos.y - y2*256),UINT16_MAX/4);
                    uint32_t c = a*a + b*b;
                    //are we closer?
                    if (c < grid[y2][x2].dist) {
                        //record information to the grid
                        grid[y2][x2].t = this_t;
                        grid[y2][x2].dist = c;
                        //add this pixel to the queue to be processed
                        queue[queue_write][0] = x2;
                        queue[queue_write][1] = y2;
                        queue_write++;
                        queue_write%=QUEUE_SIZE;
                    }
                }
            }

        }


        //draw colors to leds
        for (int y = 0; y < MATRIX_HEIGHT; y++) {
            for (int x = 0; x < MATRIX_WIDTH; x++) {
                leds[XY(x,y)] = things[grid[y][x].t].rgb;
                //shading
                uint32_t c = grid[y][x].dist/8192;
                leds[XY(x,y)].r -= (uint8_t)_min(c/32,(uint8_t)leds[XY(x,y)].r);
                leds[XY(x,y)].g -= (uint8_t)_min(c/32,(uint8_t)leds[XY(x,y)].g);
                leds[XY(x,y)].b -= (uint8_t)_min(c/32,(uint8_t)leds[XY(x,y)].b);
            }
        }

        //edges/anti-aliasing
        //step through each pixel and find edges (does the adjacent pixel contain a different thing)
        for (int y = 0; y < MATRIX_HEIGHT; y++) {
            for (int x = 0; x < MATRIX_WIDTH; x++) {
                //array of adjacent pixel coordinates
                uint16_t l[4][2];
                l[0][0] = x-1;
                l[0][1] = y;
                l[1][0] = x;
                l[1][1] = y-1;

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
                for (int i = 0; i < 2; i++) {
                    int x2 = l[i][0];
                    int y2 = l[i][1];
                    //is the current pixel on the screen
                    if (y2 >= 0 && y2 < MATRIX_HEIGHT && x2 >= 0 && x2 < MATRIX_WIDTH && grid[y][x].t != grid[y2][x2].t) {
                        
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
                        float neg_slope = 0;
                        if (y_diff != 0) {
                            neg_slope = -(1.f*x_diff)/(1.f*y_diff);
                        }
                        //calculate the intercept
                        int intercept = y_mid - (x_mid * neg_slope);
                        //now we can calculate coordinates of the midpoint
                        int y_actual = (x*256)*neg_slope+intercept;
                        int x_actual = (y*256-intercept)/neg_slope;
                        //get the color of each pixel in linear color space
                        CRGB rgb0 = gamma8_decode(leds[XY(x,y)]);
                        CRGB rgb1 = gamma8_decode(leds[XY(x2,y2)]);
                        if (y == y2) {
                            //our two pixels are aligned horizontally
                            //we'll use the x-coordinate for blending
                            if (abs(x_diff) < abs(y_diff)) {
                                continue;
                            }

                            int x_dist = abs((x*256)-x_actual);
                            int x2_dist = abs((x2*256)-x_actual);
                            int x3 = x;
                            int y3 = y;
                            int x3_dist = x_dist;


                            uint8_t b = _min(x3_dist, 255);
                            leds[XY(x3,y3)]   = gamma8_encode(nblend(rgb1, rgb0, b));
                            //leds[XY(x,y)].r = 255;
                            //break;
                        } else { //(x == x2)
                            if (abs(y_diff) < abs(x_diff)) {
                                continue;
                            }

                            int y_dist = abs((y*256)-y_actual);
                            int y2_dist = abs((y2*256)-y_actual);
                            int x3 = x;
                            int y3 = y;
                            int y3_dist = y_dist;

                            if ( y2_dist < y_dist) {
                                CRGB temp_rgb = rgb0;
                                rgb0 = rgb1;
                                rgb1 = temp_rgb;
                                x3 = x2;
                                y3 = y2;
                                y3_dist = y2_dist;
                            }

                            // if ( (x_diff > 0) != (y_diff > 0) ) {
                            //     CRGB temp_rgb = rgb0;
                            //     rgb0 = rgb1;
                            //     rgb1 = temp_rgb;
                            //     x3 = x2;
                            //     y3 = y2;
                            // }

                            uint8_t b = _min(y3_dist, 255);
                            //our two pixels are aligned vertically
                            //we'll use the y-coordinate for blending
                            leds[XY(x,y)]   = gamma8_encode(nblend(rgb1, rgb0, b));
                            //leds[XY(x,y)].g = 255;
                            //break;
                           
                        }

                        //leds[XY(x,y)].g += 255;
                    }
                }

            }
        }



        //reset the grid
        for (int y = 0; y < MATRIX_HEIGHT; y++) {
            for (int x = 0; x < MATRIX_WIDTH; x++) {
                grid[y][x].dist = UINT32_MAX;
                grid[y][x].t = -1;
            }
        }


       
    }

};   


LIGHT_SKETCHES::REGISTER<UNTITLED> untitled("untitled");

#endif