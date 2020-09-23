#ifndef LIGHTS_TETRIS_H
#define LIGHTS_TETRIS_H

#include "text.h"
#include "triangle.h"

//PARTICLES

class TETRIS: public LIGHT_SKETCH {
  public:
    TETRIS () {
        setup();
    }
    ~TETRIS () {}
  private:
    #define NUM_TETRIS_EFFECTS 1
    #define TETRIS_WIDTH 10
    #define TETRIS_HEIGHT 24
    #define MAX_BLOCK_HEIGHT (MATRIX_HEIGHT/26)
    #define MAX_BLOCK_WIDTH (MATRIX_WIDTH/12)
    #define TETRIS_SIZE (_min(MAX_BLOCK_HEIGHT,MAX_BLOCK_WIDTH)*256)
    int current_effect = 0;
    int border_width = 0;
    Z_BUF _z_buffer;
    int score = 0;
    bool button_rotate_status = false;
    bool button_rotate_old = false;
    bool button_left_status = false;
    bool button_left_old = false;
    bool button_right_status = false;
    bool button_right_old = false;
    bool button_down_status = false;
    bool button_down_old = false;
    uint16_t tetris_spacing = 128;
    int16_t gravity = 1000;
    uint16_t cube_repel = 50;
    int rows_cleared = 0;
    int previous_level = 0;


    VECTOR3 grav = VECTOR3(0,0,-TETRIS_SIZE*50);

    enum tetris_states {
        TETRIS_WAITING,
        TETRIS_READY,
        TETRIS_ACTIVE,
        TETRIS_PAUSED,
        TETRIS_GAME_OVER
    };

    int tetris_state = TETRIS_WAITING;
    uint32_t game_over_time = 0;
    uint32_t wait_time = millis() + 3000;

    enum grid_states {
        GRID_EMPTY,
        GRID_FILLED,
        GRID_SHRINKING,
        GRID_BRIGHTENING,
        GRID_GAME_OVER,
        GRID_DEAD
    };

    struct GRID {
        uint8_t state = 0;
        int32_t param = 0;
        VECTOR3 coord;
        VECTOR3 offset;
        VECTOR3 velocity;
        uint8_t hue;
        uint8_t sat;
        uint8_t bri;
    };

    enum CUBE_STATES {
    CUBE_NORMAL,
    CUBE_GRAVITY_WELL,
    CUBE_FALL,
    CUBE_TEST
    };
    
    struct CUBE {
        VECTOR3 p;
        VECTOR3 v;
        VECTOR3_16 r;
        VECTOR3_16 rv;
        uint8_t hue;
        bool active = false;
        uint8_t state = CUBE_NORMAL;
    };

    #define NUM_TETRIS_CUBES 500

    CUBE cubes[NUM_TETRIS_CUBES];

    int current_cube = 0;

    #define NUM_BORDER_CUBES ((TETRIS_WIDTH+TETRIS_HEIGHT+2)*2)
    CUBE border_cubes[NUM_BORDER_CUBES];
    int32_t border_y = -1;

    GRID grid[TETRIS_WIDTH][TETRIS_HEIGHT];

    VECTOR3 tetris_size;

    VECTOR3 shapes[7][4];

    struct PIECE {
        VECTOR3 shp[4]; //shape
        VECTOR3 pos; //position
        uint32_t t = 0;
        uint8_t hue = 0;
        CRGB rgb;
        uint32_t cnt = 0;

        void move_right() {
            pos.x++;
        }

        void move_left() {
            pos.x--;
        }

        void rotate_left() {
            for (int i = 0; i < 4; i++) {
                int temp_x = shp[i].x;
                shp[i].x = shp[i].y;
                shp[i].y = -temp_x;
            }
        }

        void rotate_right() {
            for (int i = 0; i < 4; i++) {
                int temp_x = shp[i].x;
                shp[i].x = -shp[i].y;
                shp[i].y = temp_x;
            }
        }

        bool collision_grid(GRID grid[TETRIS_WIDTH][TETRIS_HEIGHT]) {
            for (int i = 0; i < 4; i++) {
                int x = shp[i].x+pos.x;
                int y = shp[i].y+pos.y;
                if(grid[x][y].state == GRID_FILLED) {
                    return true;
                }     
            }
            return false;
        }

        bool collision_wall_left() {
            for (int i = 0; i < 4; i++) {
                int x = shp[i].x+pos.x;
                if(x < 0) {
                    return true;
                }     
            }
            return false;
        }

        bool collision_wall_right() {
            for (int i = 0; i < 4; i++) {
                int x = shp[i].x+pos.x;
                if(x > TETRIS_WIDTH-1) {
                    return true;
                }     
            }
            return false;
        }



        bool collision_floor() {
            for (int i = 0; i < 4; i++) {
                int y = shp[i].y+pos.y;
                if(y < 0) {
                    return true;
                }     
            }
            return false;
        }

    };

    PIECE piece;

    void reset_cell(int x, int y) {
        grid[x][y].state = GRID_EMPTY;
        grid[x][y].offset = VECTOR3(0,0,0);
        grid[x][y].velocity = VECTOR3(0,0,0);
        grid[x][y].sat = 200;
        grid[x][y].bri = 255;
        grid[x][y].param = 0;
    }

    void new_piece() {
        uint8_t s = random(7);
        s = 0; //debug
        piece.shp[0] = shapes[s][0];
        piece.shp[1] = shapes[s][1];
        piece.shp[2] = shapes[s][2];
        piece.shp[3] = shapes[s][3];
        piece.pos.x = TETRIS_WIDTH/2;
        piece.pos.y = TETRIS_HEIGHT-4;
        piece.hue = random(255);
        piece.rgb = CHSV(piece.hue,200,255);
        piece.t = millis()+500;
        piece.cnt++;
    }

    bool rotate_left = false;

  public:
    void reset() {
        
        score = 0;
        rows_cleared = 0;
        previous_level = 0;

        new_piece();

        for (int x = 0; x < TETRIS_WIDTH; x++) {
            int x_coord = border_width*256+TETRIS_SIZE/2+(TETRIS_SIZE)*x;
            for (int y = 0; y < TETRIS_HEIGHT; y++) {
                int y_coord = border_width*256+TETRIS_SIZE/2+(TETRIS_SIZE)*y;
                grid[x][y].coord.x = x_coord-TETRIS_SIZE*TETRIS_WIDTH/2-128;
                grid[x][y].coord.y = y_coord-TETRIS_SIZE*TETRIS_HEIGHT/2-128;
                grid[x][y].coord.z = led_screen.camera_position.z-led_screen.screen_distance-TETRIS_SIZE/2;
                reset_cell(x,y);
            }
        }

        for (int i = 0; i < NUM_TETRIS_CUBES; i++) {
            cubes[i].active = 0;
        }


        //create border cubes
        {
            int x = 0;
            int y = 0;
            int c = 0;
            while (y < TETRIS_HEIGHT+1) {
                border_cubes[c++].p = grid[0][0].coord + VECTOR3((x-1)*TETRIS_SIZE, (y-1)*TETRIS_SIZE, 0);
                y++;
            }
            while ( x < TETRIS_WIDTH+1 ) {
                border_cubes[c++].p = grid[0][0].coord + VECTOR3((x-1)*TETRIS_SIZE, (y-1)*TETRIS_SIZE, 0);
                x++;
            }
            while (y > 0) {
                border_cubes[c++].p = grid[0][0].coord + VECTOR3((x-1)*TETRIS_SIZE, (y-1)*TETRIS_SIZE, 0);
                y--;
            }
            while (x > 0) {
                border_cubes[c++].p = grid[0][0].coord + VECTOR3((x-1)*TETRIS_SIZE, (y-1)*TETRIS_SIZE, 0);
                x--;
            }
        }

        ///*

        static int cnt = 0;
        current_cube = 1;
        cubes[current_cube].active = true;
        cubes[current_cube].state = CUBE_TEST;
        cubes[current_cube].p = VECTOR3(0, 0, TETRIS_SIZE*10);
        cubes[current_cube].v = VECTOR3(0, 0, 0);
        //cubes[current_cube].r = VECTOR3(8000, 0, 0);
        cubes[current_cube].rv = VECTOR3(20, 30, 40);

        switch (cnt) {
            case 0:
                cubes[current_cube].p.x -= (TETRIS_SIZE*2+TETRIS_SIZE/6);
                cubes[current_cube].v.x = TETRIS_SIZE/60;
                break;
            case 1:
                cubes[current_cube].p.x += (TETRIS_SIZE*2+TETRIS_SIZE/6);
                cubes[current_cube].v.x = -TETRIS_SIZE/60;
                break;
            case 2:
                cubes[current_cube].p.y -= (TETRIS_SIZE*2+TETRIS_SIZE/6);
                cubes[current_cube].v.y = TETRIS_SIZE/60;
                break;
            case 3:
                cubes[current_cube].p.y += (TETRIS_SIZE*2+TETRIS_SIZE/6);
                cubes[current_cube].v.y = -TETRIS_SIZE/60;
                break;
            case 4:
                cubes[current_cube].p.z -= (TETRIS_SIZE*2+TETRIS_SIZE/6);
                cubes[current_cube].v.z = TETRIS_SIZE/60;
                break;
            case 5:
                cubes[current_cube].p.z += (TETRIS_SIZE*2+TETRIS_SIZE/6);
                cubes[current_cube].v.z = -TETRIS_SIZE/60;
                break;
        }
        
        current_cube = 0;
        cubes[current_cube].active = true;
        cubes[current_cube].state = CUBE_TEST;
        cubes[current_cube].p = VECTOR3((TETRIS_SIZE*3)/4, (TETRIS_SIZE*3)/4, -(TETRIS_SIZE*5)/6+TETRIS_SIZE*10);
        cubes[current_cube].p = VECTOR3(0, 0, TETRIS_SIZE*10);
        cubes[current_cube].v = VECTOR3(0, 0, 0);
        cubes[current_cube].r = VECTOR3(0, 0, 0);
        cubes[current_cube].rv = VECTOR3(-45, -25, -15);
        current_cube=2;
        cnt++;
        cnt%=6;

        //*/

    }

    void setup() {
        light_rotation_x = 24;
        light_rotation_y = 10;
        control_variables.add(button_rotate_status,"Rotate", 1, 38);
        control_variables.add(button_left_status,"Left", 1, 37);
        control_variables.add(button_right_status,"Right", 1, 39);
        control_variables.add(button_down_status,"Down", 1, 40);
        control_variables.add(led_screen.camera_position.z, "Camera Z:", 0, 1024*256);
        control_variables.add(led_screen.screen_distance, "Screen Z:", 0, 1024*256);
        control_variables.add(tetris_spacing, "Spacing:", 0, 1024);
        control_variables.add(led_screen.light_falloff, "Light Distance:", 1, 16);
        control_variables.add(gravity, "Gravity:", -30000, 30000);
        control_variables.add(cube_repel, "Cube Repel:", 1, 200);
        
        led_screen.camera_position.z=200*256;
        led_screen.screen_distance=566*256;

        tetris_size = VECTOR3(TETRIS_SIZE/2-tetris_spacing,TETRIS_SIZE/2-tetris_spacing,TETRIS_SIZE/2-tetris_spacing);

        //straight
        shapes[0][0] = VECTOR3(0,-1,0);
        shapes[0][1] = VECTOR3(0,0,0);
        shapes[0][2] = VECTOR3(0,1,0);
        shapes[0][3] = VECTOR3(0,2,0);

        //J
        shapes[1][0] = VECTOR3(-1,-1,0);
        shapes[1][1] = VECTOR3(0,-1,0);
        shapes[1][2] = VECTOR3(0,0,0);
        shapes[1][3] = VECTOR3(0,1,0);


        //L
        shapes[2][0] = VECTOR3(1,-1,0);
        shapes[2][1] = VECTOR3(0,-1,0);
        shapes[2][2] = VECTOR3(0,0,0);
        shapes[2][3] = VECTOR3(0,1,0);


        //S
        shapes[3][0] = VECTOR3(-1,0,0);
        shapes[3][1] = VECTOR3(0,0,0);
        shapes[3][2] = VECTOR3(0,1,0);
        shapes[3][3] = VECTOR3(1,1,0);

        //Z
        shapes[4][0] = VECTOR3(1,0,0);
        shapes[4][1] = VECTOR3(0,0,0);
        shapes[4][2] = VECTOR3(0,1,0);
        shapes[4][3] = VECTOR3(-1,1,0);

        //box
        shapes[5][0] = VECTOR3(-1,0,0);
        shapes[5][1] = VECTOR3(0,0,0);
        shapes[5][2] = VECTOR3(-1,1,0);
        shapes[5][3] = VECTOR3(0,1,0);

        //w
        shapes[6][0] = VECTOR3(-1,0,0);
        shapes[6][1] = VECTOR3(0,0,0);
        shapes[6][2] = VECTOR3(1,0,0);
        shapes[6][3] = VECTOR3(0,1,0);

        led_screen.light_falloff = 11;

        border_width = (MATRIX_WIDTH%TETRIS_WIDTH)/2;

        reset();
    }

    void next_effect() {
        current_effect++;
        current_effect %= NUM_TETRIS_EFFECTS;
    }
    void loop() {
        rotate_left = false;
        if (button_rotate_status) {
            if (!button_rotate_old) {
                rotate_left = true;
            }
            button_rotate_old = true;
        } else {
            button_rotate_old = false;
        }
        
        {
        char buffer [50];
        int n = sprintf (buffer, "%d", score);
        VECTOR3 p = grid[0][TETRIS_HEIGHT-1].coord;
        led_screen.perspective(p);
        draw_characters(5, buffer, p.x, p.y, CRGB(255,255,255), MATRIX_HEIGHT*2, 0);
        }

        {
        char buffer [50];
        int n = sprintf (buffer, "level %d", rows_cleared/10);
        VECTOR3 p = grid[0][TETRIS_HEIGHT-3].coord;
        led_screen.perspective(p);
        draw_characters(5, buffer, p.x, p.y, CRGB(255,255,255), MATRIX_HEIGHT, 0);  
        }
        
        handle_text();
        LED_show();
        LED_black();


        switch (current_effect) {
            case 0:
                draw_border();
                handle_tetris();
                handle_cubes();
                break;
            default:
                break;

        }


        

    }

    //draw all of our non-grid cubes
    void handle_cubes() {
        for(int i = 0; i < NUM_TETRIS_CUBES; i++) {
            if (cubes[i].active) {
                if (cubes[i].p.z > led_screen.camera_position.z) {
                    cubes[i].active = 0;
                    continue;
                }
                
                switch (cubes[i].state) {
                    case CUBE_FALL:
                        cubes[i].p += cubes[i].v;
                        cubes[i].r += cubes[i].rv;
                        cubes[i].v.y -= TETRIS_SIZE/1600;
                        break;
                    case CUBE_GRAVITY_WELL:
                        break;
                    case CUBE_TEST:
                        cubes[i].p += cubes[i].v;
                        cubes[i].r += cubes[i].rv;
                        break;
                }
            }
        }
        for(int i = 0; i < NUM_TETRIS_CUBES; i++) {
            if (cubes[i].active) {
                
                switch (cubes[i].state) {
                    case CUBE_FALL:
                        cube_fall(i);
                        break;
                    case CUBE_GRAVITY_WELL:
                        cube_gravity_well(i);
                        break;
                    case CUBE_TEST:
                        cube_test(i);
                        break;
                    default:
                        cubes[i].active = false;
                }
            }
        }
    }

    void cube_fall(int i) {
        if ((cubes[i].p.y < border_cubes[0].p.y - TETRIS_SIZE*10)) {
            cubes[i].active = false;
            return;
        }
        uint8_t sat = 255;
        for (int j = i+1; j < NUM_TETRIS_CUBES; j++) {
            if (cubes[j].active) {
                if ( abs(cubes[i].p.x-cubes[j].p.x) < (TETRIS_SIZE*1.73) && abs(cubes[i].p.y-cubes[j].p.y) < (TETRIS_SIZE*1.73) && abs(cubes[i].p.z-cubes[j].p.z) < (TETRIS_SIZE*1.73) ) {
                    VECTOR3 adjust;
                    if (check_cube_collision(i, cubes[j], adjust)) {
                        //sat = 0;
                        cubes[i].p += adjust/2;
                        cubes[i].v += adjust/2;
                        cubes[j].p -= adjust/2;
                        cubes[j].v -= adjust/2;
                    }
                }
            }
        }

        for (int j = 0; j < NUM_BORDER_CUBES; j++) {
            if ( abs(cubes[i].p.x-border_cubes[j].p.x) < TETRIS_SIZE && abs(cubes[i].p.y-border_cubes[j].p.y) < TETRIS_SIZE && abs(cubes[i].p.z-border_cubes[j].p.z) < TETRIS_SIZE ) {
                VECTOR3 adjust;
                    if (check_cube_collision(i, border_cubes[j], adjust)) {
                        //sat = 0;
                        cubes[i].p += adjust;
                        cubes[i].v += adjust;
                    }
            }
        }

        draw_cube(cubes[i].p, tetris_size, cubes[i].r, CHSV(cubes[i].hue,sat,255), false, true);
    }

    void cube_gravity_well(int i) {
        cubes[i].p += cubes[i].v;
        cubes[i].r += cubes[i].rv;
        cubes[i].v *= 98;
        cubes[i].v /= 100;
        if (gravity != 0) cubes[i].v += (grav - cubes[i].p) / gravity;
        for (int j = i+1; j < NUM_TETRIS_CUBES; j++) {
            if (cubes[j].active) {
                if ( abs(cubes[i].p.x-cubes[j].p.x) < TETRIS_SIZE*2 && abs(cubes[i].p.y-cubes[j].p.y) < TETRIS_SIZE*2 && abs(cubes[i].p.z-cubes[j].p.z) < TETRIS_SIZE*2 ) {
                    int x_dist = cubes[i].p.x-cubes[j].p.x;
                    x_dist*=2;
                    int x_adjust = sgn(x_dist) * (TETRIS_SIZE*2 - abs(x_dist)) / cube_repel;
                    cubes[i].v.x += x_adjust;
                    cubes[j].v.x -= x_adjust;
                    int y_dist = cubes[i].p.y-cubes[j].p.y;
                    y_dist*=2;
                    int y_adjust = sgn(y_dist) * (TETRIS_SIZE*2 - abs(y_dist)) / cube_repel;
                    cubes[i].v.y += y_adjust;
                    cubes[j].v.y -= y_adjust;
                    int z_dist = cubes[i].p.z-cubes[j].p.z;
                    z_dist*=2;
                    int z_adjust = sgn(z_dist) * (TETRIS_SIZE*2 - abs(z_dist)) / cube_repel;
                    cubes[i].v.z += z_adjust;
                    cubes[j].v.z -= z_adjust;
                }
            }
        }

        draw_cube(cubes[i].p, tetris_size, cubes[i].r, CHSV(cubes[i].hue,255,255), false, true);
    }

    void cube_test(int i) {
        uint8_t sat = 255;
        // if(i == 0) {
        //     cubes[1].r.x -= 200;
        //     cubes[1].r.y -= 300;
        //     cubes[1].r.z -= 100;
        // }
        for (int j = 0; j < NUM_TETRIS_CUBES; j++) {
            if ( j == i ) continue;
            if (cubes[j].active) {
                if ( abs(cubes[i].p.x-cubes[j].p.x) <= (TETRIS_SIZE*3)/2 && abs(cubes[i].p.y-cubes[j].p.y) <= (TETRIS_SIZE*3)/2 && abs(cubes[i].p.z-cubes[j].p.z) <= (TETRIS_SIZE*3)/2 ) {
                    VECTOR3 adjust;
                    if (check_cube_collision(i, cubes[j], adjust)) {
                        sat = 0;
                        cubes[i].p += adjust/2;
                        cubes[i].v += adjust/2;
                        cubes[j].p -= adjust/2;
                        cubes[j].v -= adjust/2;
                    }
                    
                }
            }


        }

        draw_cube(cubes[i].p, tetris_size, cubes[i].r, CHSV(160,sat,255), false, true, 64);
    }

    bool check_cube_collision (int i, CUBE& j, VECTOR3& adjust) {
        //define cube at 0,0,0
        VECTOR3 points[] = {
            VECTOR3(TETRIS_SIZE/2,TETRIS_SIZE/2,TETRIS_SIZE/2), //top right front
            VECTOR3(TETRIS_SIZE/2,TETRIS_SIZE/2,-TETRIS_SIZE/2), //top right back
            VECTOR3(-TETRIS_SIZE/2,TETRIS_SIZE/2,-TETRIS_SIZE/2), //top left  back
            VECTOR3(-TETRIS_SIZE/2,TETRIS_SIZE/2,TETRIS_SIZE/2), //top left  front
            
            VECTOR3(TETRIS_SIZE/2,-TETRIS_SIZE/2,TETRIS_SIZE/2), //bottom right front
            VECTOR3(TETRIS_SIZE/2,-TETRIS_SIZE/2,-TETRIS_SIZE/2), //bottom right back
            VECTOR3(-TETRIS_SIZE/2,-TETRIS_SIZE/2,-TETRIS_SIZE/2), //bottom left  back
            VECTOR3(-TETRIS_SIZE/2,-TETRIS_SIZE/2,TETRIS_SIZE/2)  //bottom left  front
        };
        VECTOR3 collision_point = VECTOR3(0,0,0);
        int num_collision_points = 0;
        uint32_t min_dist = UINT32_MAX;
        int min_point = -1;
        uint32_t min_dist2 = UINT32_MAX;
        int min_point2 = -1;
        for (int k = 0; k < 8; k++) {
            //apply cube's own rotation
            VECTOR3 p = points[k];
            rotate16(p, j.r);
            //translate position relative to other cube's position
            p += j.p - cubes[i].p;
            //apply inverse of other cube's rotation
            reverse_rotate16(p, cubes[i].r);
            //check if point is inside the other cube
            if ( abs(p.x) <= TETRIS_SIZE/2 && abs(p.y) <= TETRIS_SIZE/2 && abs(p.z) <= TETRIS_SIZE/2 ) {
                collision_point += p;
                num_collision_points++;
            }

            //attempt to identify the two points nearest to the other cube (nearest line segment)
            uint32_t dist = p.x*p.x + p.y*p.y + p.z*p.z;
            if(dist < min_dist) {
                min_dist2 = min_dist;
                min_point2 = min_point;
                min_dist = dist;
                min_point = k;
            } else if (dist < min_dist2) {
                min_dist2 = dist;
                min_point2 = k;
            }
            

        }

        //attempt to identify the nearest line segment on the other cube
        VECTOR3 q = j.p - cubes[i].p;
        reverse_rotate16(q, cubes[i].r);
        uint32_t qmin_dist = UINT32_MAX;
        int qmin_point = -1;
        uint32_t qmin_dist2 = UINT32_MAX;
        int qmin_point2 = -1;
        for (int k = 0; k < 8; k++) {
            int32_t dist = (points[k].x-q.x)*(points[k].x-q.x) + (points[k].y-q.y)*(points[k].y-q.y) + (points[k].z-q.z)*(points[k].z-q.z);
            if(dist < qmin_dist) {
                qmin_dist2 = qmin_dist;
                qmin_point2 = qmin_point;
                qmin_dist = dist;
                qmin_point = k;
            } else if (dist < qmin_dist2) {
                qmin_dist2 = dist;
                qmin_point2 = k;
            }
        }


        //draw the first line segment
        VECTOR3 p0 = points[min_point];
        rotate16(p0, j.r);
        p0 += j.p - cubes[i].p;
        reverse_rotate16(p0, cubes[i].r);

        VECTOR3 p1 = points[min_point2];
        rotate16(p1, j.r);
        p1 += j.p - cubes[i].p;
        reverse_rotate16(p1, cubes[i].r);

        //draw the second line segment
        VECTOR3 q0 = points[qmin_point];
        
        VECTOR3 q1 = points[qmin_point2];
       
        /*
        led_screen.perspective(q1);
        CRGB rgb2 = CRGB(255,0,0);
        draw_line_fine(led_screen, q0, q1, rgb2, led_screen.camera_position.z, 255, 255, false, false);
*/
        //translate one line segment to the other (so the segments meet on one end)
        //this gives us a triangle representing a plane parallel to both lines
        //finding the surface normal of the triangle gives us a vector perpendicular to both lines
        VECTOR3 p2 = q1 - (q0-p0);
        VECTOR3 norm = normal(p2, p0, p1);


        //now we need to find on our plane the point common to both lines
        //this is the shortest distance between the two lines
        //I gotta find a fuckin' better way to do this
        VECTOR3 back = norm; //we want to find the angles to make our surface normal parallel to the z-axis
        uint8_t angle_z = (atan2(-back.x, -back.y)*256)/(2*PI);
        rotate_z(back,angle_z);
        uint8_t angle_x = (atan2(-back.y, -back.z)*256)/(2*PI);
        rotate_x(back,angle_x);

        VECTOR3 pp0 = p0;
        VECTOR3 pp1 = p1;
        VECTOR3 qq0 = q0;
        VECTOR3 qq1 = q1;

        pp1 -= pp0;
        qq0 -= pp0;
        qq1 -= pp0;

        rotate_z(pp1,angle_z);
        rotate_x(pp1,angle_x);
        rotate_z(qq0,angle_z);
        rotate_x(qq0,angle_x);
        rotate_z(qq1,angle_z);
        rotate_x(qq1,angle_x);

        pp1 += pp0;
        qq0 += pp0;
        qq1 += pp0;

        led_screen.matrix.rotate(pp0);
        led_screen.matrix.rotate(pp1);
        led_screen.matrix.rotate(qq0);
        led_screen.matrix.rotate(qq1);

        //now our lines are on the X/Y plane
        //we need to figure out where they intersect

        //rise_a * x + y_intercept_a = rise_b * x + y_intercept_b
        //rise_a * x - rise_b * x = y_intercept_b - y_intercept_a
        //(rise_a - rise_b) * x = y_intercept_b - y_intercept_a
        //x = (y_intercept_b - y_intercept_a) / (rise_a - rise_b)

        // if (pd1.x != pd0.x && qd1.x != qd0.x)
        // {

            float rise_a = (pp1.y-pp0.y) / (1.f*pp1.x-pp0.x);
            float rise_b = (qq1.y-qq0.y) / (1.f*qq1.x-qq0.x);

            float y_intercept_a = pp0.y - (rise_a * pp0.x);
            float y_intercept_b = qq0.y - (rise_b * qq0.x);

            float common_x = (y_intercept_b - y_intercept_a) / (rise_a - rise_b);
            float common_y = rise_a * common_x + y_intercept_a;

            float p_location = (common_x-pp0.x)/((float)pp1.x-pp0.x);
            float q_location = (common_x-qq0.x)/((float)qq1.x-qq0.x);


            if (p_location >= 0 && p_location <= 1 && q_location >= 0 && q_location <= 1 ) {
                p0.x = (p1.x-p0.x) * p_location + p0.x;
                p0.y = (p1.y-p0.y) * p_location + p0.y;
                p0.z = (p1.z-p0.z) * p_location + p0.z;
                q0.x = (q1.x-q0.x) * q_location + q0.x;
                q0.y = (q1.y-q0.y) * q_location + q0.y;
                q0.z = (q1.z-q0.z) * q_location + q0.z;


                if ( abs(p0.x) <= TETRIS_SIZE/2 && abs(p0.y) <= TETRIS_SIZE/2 && abs(p0.z) <= TETRIS_SIZE/2 ) {
                    // cubes[i].p += (p0-q0)/2;
                    // cubes[i].v += (p0-q0)/4;
                    // j.p += (q0-p0)/2;
                    // j.v += (q0-p0)/4;
                    
                    collision_point += p0;
                    num_collision_points++;

                    rotate16(p0, cubes[i].r);
                    p0 += cubes[i].p;
                    led_screen.matrix.rotate(p0);
                    led_screen.perspective(p0);
                    rotate16(q0, cubes[i].r);
                    q0 += cubes[i].p;
                    led_screen.matrix.rotate(q0);
                    led_screen.perspective(q0);


                    CRGB rgb6 = CRGB(0,255,0);
                    draw_circle_fine(p0.x, p0.y, 3*256, rgb6, -1, 32);
                    CRGB rgb7 = CRGB(0,0,255);
                    draw_circle_fine(q0.x, q0.y, 3*256, rgb7, -1, 32);
                }
            }

/*

            VECTOR3 derp = VECTOR3(common_x, common_y, pd0.z);

            derp.x += (MATRIX_WIDTH*256)/3;
            derp.y += (MATRIX_HEIGHT*256)/3;

            CRGB rgb6 = CRGB(255,255,255);
            draw_circle_fine(derp.x, derp.y, 3*256, rgb6, -1, 32);

        //}

        pd0.x += (MATRIX_WIDTH*256)/3;
        pd0.y += (MATRIX_HEIGHT*256)/3;
        pd1.x += (MATRIX_WIDTH*256)/3;
        pd1.y += (MATRIX_HEIGHT*256)/3;
        qd0.x += (MATRIX_WIDTH*256)/3;
        qd0.y += (MATRIX_HEIGHT*256)/3;
        qd1.x += (MATRIX_WIDTH*256)/3;
        qd1.y += (MATRIX_HEIGHT*256)/3;

        CRGB rgb5 = CRGB(255,255,255);
        draw_line_fine(led_screen, pd0, pd1, rgb5, led_screen.camera_position.z, 255, 255, false, false);
        draw_line_fine(led_screen, qd0, qd1, rgb5, led_screen.camera_position.z, 255, 255, false, false);



        back*=20;
        back+=pd0;
        led_screen.matrix.rotate(back);
        led_screen.perspective(back);
        CRGB rgb4 = CRGB(255,255,255);
        draw_line_fine(led_screen, p0, back, rgb4, led_screen.camera_position.z, 255, 255, false, false);

        norm *= TETRIS_SIZE/2;
        norm /= 256;
        VECTOR3 norm2 = -norm;
        norm += pd0;
        norm2 += pd0;

        led_screen.matrix.rotate(norm);
        led_screen.perspective(norm);
        led_screen.matrix.rotate(norm2);
        led_screen.perspective(norm2);

        CRGB rgb3 = CRGB(0,255,0);
        draw_line_fine(led_screen, p0, norm,  rgb3, led_screen.camera_position.z, 255, 255, false, false);
        draw_line_fine(led_screen, p0, norm2, rgb3, led_screen.camera_position.z, 255, 255, false, false);
*/





        if (num_collision_points) {
            collision_point /= num_collision_points;
            VECTOR3 angle = VECTOR3(0,0,0);
            if (abs(collision_point.x) >= abs(collision_point.y) && abs(collision_point.x) >= abs(collision_point.z)) {
                //x is closest
                int mag = ((TETRIS_SIZE/2 - abs(collision_point.x)) * 1024 ) / (TETRIS_SIZE/2);
                int mag2 = sqrt(sq(TETRIS_SIZE/2 - abs(collision_point.y)) + sq(TETRIS_SIZE/2 - abs(collision_point.z)));
                const int mag2_max = sqrt(sq(TETRIS_SIZE/2)*2);

                angle = VECTOR3(0,(sgn(collision_point.x)*collision_point.z*mag)/(TETRIS_SIZE/2),(sgn(collision_point.x)*collision_point.y*mag)/(TETRIS_SIZE/2));
                angle *= mag2;
                angle /= mag2_max;

                mag2 = mag2_max - mag2;
                adjust = VECTOR3(sgn(collision_point.x) * (abs(collision_point.x) - TETRIS_SIZE/2), 0, 0);
                // adjust *= mag2;
                // adjust /= mag2_max;
            }
            else if (abs(collision_point.y) >= abs(collision_point.x) && abs(collision_point.y) >= abs(collision_point.z)) {
                //y is closest
                int mag = ((TETRIS_SIZE/2 - abs(collision_point.y)) * 1024 ) / (TETRIS_SIZE/2);
                int mag2 = sqrt(sq(TETRIS_SIZE/2 - abs(collision_point.x)) + sq(TETRIS_SIZE/2 - abs(collision_point.z)));
                const int mag2_max = sqrt(sq(TETRIS_SIZE/2)*2);

                angle = VECTOR3((sgn(collision_point.y)*collision_point.z*mag)/(TETRIS_SIZE/2),0,-(sgn(collision_point.y)*collision_point.x*mag)/(TETRIS_SIZE/2));
                angle *= mag2;
                angle /= mag2_max;

                mag2 = mag2_max - mag2;
                adjust = VECTOR3(0, sgn(collision_point.y) * (abs(collision_point.y) - TETRIS_SIZE/2), 0);
                // adjust *= mag2;
                // adjust /= mag2_max;
            } else {
                //z is closest
                int mag = ((TETRIS_SIZE/2 - abs(collision_point.z)) * 1024 ) / (TETRIS_SIZE/2);
                int mag2 = sqrt(sq(TETRIS_SIZE/2 - abs(collision_point.x)) + sq(TETRIS_SIZE/2 - abs(collision_point.y)));
                const int mag2_max = sqrt(sq(TETRIS_SIZE/2)*2);
                
                angle = VECTOR3(-(sgn(collision_point.z)*collision_point.y*mag)/(TETRIS_SIZE/2),-(sgn(collision_point.z)*collision_point.x*mag)/(TETRIS_SIZE/2),0);
                angle *= mag2;
                angle /= mag2_max;

                mag2 = mag2_max - mag2;
                adjust = VECTOR3(0, 0, sgn(collision_point.z) * (abs(collision_point.z) - TETRIS_SIZE/2));
                // adjust *= mag2;
                // adjust /= mag2_max;
            }
            //out = collision_point;
            //int hyp = sqrt(sq(collision_point.x-cubes[i].p.x) + sq(collision_point.y-cubes[i].p.y) + sq(collision_point.z-cubes[i].p.z));
            rotate16(adjust, cubes[i].r);
            //rotate16(angle, cubes[i].r);
            //cubes[i].r += angle;
            //cubes[i].rv += angle;
            return true;
        }
        return false;

    }

    void handle_tetris() {


            if (tetris_state == TETRIS_WAITING) {
                if (millis() > wait_time) {
                    tetris_state = TETRIS_READY;
                }
                char buffer[] = "GAME OVER";
                VECTOR3 p = grid[0][0].coord;
                led_screen.perspective(p);
                draw_characters(5, buffer, p.x, (MATRIX_HEIGHT*256*3)/4, CRGB(255,255,255), MATRIX_HEIGHT*2, 0);
            }

            if (tetris_state == TETRIS_GAME_OVER) {
                tetris_state = TETRIS_WAITING;
                wait_time = millis()+3000;
            }

            if (tetris_state == TETRIS_READY) {

                if (button_down_status || button_rotate_status || button_left_status || button_right_status) {
                    score = 0;
                    rows_cleared = 0;
                    tetris_state = TETRIS_ACTIVE;
                } else {
                    char buffer[] = "PRESS ANY KEY TO START";
                    VECTOR3 p = grid[0][0].coord;
                    led_screen.perspective(p);
                    draw_characters(5, buffer, p.x, (MATRIX_HEIGHT*256*2)/3, CRGB(255,255,255), MATRIX_HEIGHT, 0);
                }

            }

            //update grid
            int filled_rows = 0;
            for (int y = 0; y < TETRIS_HEIGHT; y++) {
                bool row_filled = true;

                //check if row is filled
                int max_piece = -1;
                int most_recent_cell = -1;
                for (int x = 0; x < TETRIS_WIDTH; x++) {
                    if(grid[x][y].state != GRID_FILLED) {
                        row_filled = false;
                        break;
                    }
                    if (grid[x][y].param > max_piece) {
                        max_piece = grid[x][y].param;
                        most_recent_cell = x;
                    }
                }

                //if a row is filled, mark it for the next effect
                if (row_filled) {
                    score+=100+100*filled_rows;
                    filled_rows++;
                    rows_cleared++;
                    for (int x = 0; x < TETRIS_WIDTH; x++) {
                        grid[x][y].state = GRID_BRIGHTENING;
                        grid[x][y].param = 155+abs(most_recent_cell-x)*(100-abs(most_recent_cell-x));
                    }
                }

                //handle game over
                //if done, then mark for next effect
                for (int x = 0; x < TETRIS_WIDTH; x++) {
                    if (grid[x][y].state == GRID_GAME_OVER){
                        //grid[x][y].state = GRID_SHRINKING;
                        grid[x][y].state = GRID_EMPTY;
                        grid[x][y].param = 255;
                        cubes[current_cube].p = grid[x][y].coord;
                        cubes[current_cube].v = VECTOR3(random(10)-5,0,random(10)-5);
                        cubes[current_cube].r = VECTOR3(0,0,0);
                        cubes[current_cube].rv = VECTOR3(random(0,200)-100,random(0,200)-100,random(0,200)-100);
                        cubes[current_cube].state = CUBE_FALL;


                        cubes[current_cube].hue = grid[x][y].hue; 
                        cubes[current_cube].active = 1;
                        current_cube++;
                        current_cube%=NUM_TETRIS_CUBES;
                    }  
                }

                //handle brightening
                //if done, then mark for next effect
                for (int x = 0; x < TETRIS_WIDTH; x++) {
                    if (grid[x][y].state == GRID_BRIGHTENING){
                        //grid[x][y].sat = _min(grid[x][y].param, 255);
                        //draw_cube(grid[x][y].coord, tetris_size, VECTOR3(0,0,0), CHSV(grid[x][y].hue,grid[x][y].sat,grid[x][y].bri));
                        grid[x][y].param -= 100;
                        if (grid[x][y].param <= 0) {
                            //grid[x][y].state = GRID_SHRINKING;
                            grid[x][y].state = GRID_DEAD;
                            grid[x][y].param = 255;
                            cubes[current_cube].p = grid[x][y].coord;
                            cubes[current_cube].v = VECTOR3(random(TETRIS_SIZE*2)-TETRIS_SIZE, random(TETRIS_SIZE*2)-TETRIS_SIZE, -random(TETRIS_SIZE));
                            cubes[current_cube].r = VECTOR3(0,0,0);
                            cubes[current_cube].rv = VECTOR3(random(512)-256,random(512)-256,random(512)-256);
                            cubes[current_cube].state = CUBE_GRAVITY_WELL;
                            cubes[current_cube].hue = grid[x][y].hue; 
                            cubes[current_cube].active = 1;
                            current_cube++;
                            current_cube%=NUM_TETRIS_CUBES;
                        } else {
                            draw_cube(grid[x][y].coord, tetris_size, VECTOR3(0,0,0), CHSV(grid[x][y].hue,255,255), false, true);
                        }
                        
                    }

                }

                //handle shrinking
                //if done, mark the cell as dead
                for (int x = 0; x < TETRIS_WIDTH; x++) {
                    if (grid[x][y].state == GRID_SHRINKING){
                        VECTOR3 size = tetris_size;
                        size *= _min(grid[x][y].param, 255);
                        size /= 255;
                        grid[x][y].param -= 20;
                        if (size.x > 0) {
                            draw_cube(grid[x][y].coord, tetris_size, VECTOR3(0,0,0), CHSV(grid[x][y].hue,grid[x][y].sat,grid[x][y].bri), false, true);
                        } else {
                            grid[x][y].state = GRID_DEAD;
                        }
                    }
                }

                //if a cell is dead, then move the remaining column down
                for (int x = 0; x < TETRIS_WIDTH; x++) {
                    if (grid[x][y].state == GRID_DEAD) {
                        for (int y2 = y+1; y2 < TETRIS_HEIGHT; y2++) {
                            grid[x][y2-1].state = grid[x][y2].state;
                            grid[x][y2-1].hue = grid[x][y2].hue;
                            grid[x][y2-1].sat = grid[x][y2].sat;
                            grid[x][y2-1].bri = grid[x][y2].bri;
                            grid[x][y2-1].offset.y = grid[x][y2].offset.y+TETRIS_SIZE;
                            grid[x][y2].state = GRID_EMPTY;
                        }
                    }
                }

            }

            //draw existing squares on the grid
            for (int y = 0; y < TETRIS_HEIGHT; y++) {
                for (int x = 0; x < TETRIS_WIDTH; x++) {

                    if(grid[x][y].offset.y > 0) {
                        grid[x][y].velocity.y -= TETRIS_SIZE/40;
                        grid[x][y].offset.y += grid[x][y].velocity.y;
                        if(grid[x][y].offset.y < 0) {
                            grid[x][y].offset.y = 0;
                            grid[x][y].velocity.y = 0;
                        }
                    }

                    if(grid[x][y].state == GRID_FILLED) {
                        draw_cube(grid[x][y].coord + grid[x][y].offset, tetris_size, VECTOR3(0,0,0), CHSV(grid[x][y].hue,grid[x][y].sat,grid[x][y].bri), false, true);
                    }
                    
                }
            }

            //handle the active piece
            if (tetris_state == TETRIS_ACTIVE) {

                //make the piece fall
                if (button_down_status || millis() > piece.t) {
                    piece.pos.y--;
                    int t = 1500;
                    for (int i = 0; i < rows_cleared/10; i++) {
                        t*=9;
                        t/=10;
                    }
                    piece.t = millis()+t;
                }


                //rotate the piece
                if (rotate_left) {
                    piece.rotate_left();

                    //check for obstructions and move up to two spaces if possible
                    if (piece.collision_wall_left() || piece.collision_grid(grid) ) {  
                        piece.move_right();
                        if (piece.collision_wall_left() || piece.collision_grid(grid) ) {
                            piece.move_right();
                            if (piece.collision_wall_left() || piece.collision_grid(grid)) { 
                                //obstructed, revert (do nothing)
                                piece.move_left();
                                piece.move_left();
                                piece.rotate_right();
                            }
                        }
                    }

                    if (piece.collision_wall_right() || piece.collision_grid(grid) ) {  
                        piece.move_left();
                        if (piece.collision_wall_right() || piece.collision_grid(grid) ) {  
                            piece.move_left();
                            if (piece.collision_wall_right() || piece.collision_grid(grid)) { 
                                piece.move_right();
                                piece.move_right();
                                piece.rotate_right();
                            }
                        }
                    }

                    if (piece.collision_floor()) {
                        piece.rotate_right();
                    }

                }


                //move the active piece one position to the right
                static uint32_t button_right_timer = 0;
                if (button_right_status && millis() > button_right_timer) {
                    button_right_timer = millis() + 120;
                    piece.move_right();
                    if (piece.collision_wall_right() || piece.collision_grid(grid)) {
                        piece.move_left();   
                    }

                } else if (!button_right_status) {
                    button_right_timer = 0;
                }

                //move the active piece one position to the left
                static uint32_t button_left_timer = 0;
                if (button_left_status && millis() > button_left_timer) {
                    button_left_timer = millis() + 120;
                    piece.move_left();
                    if (piece.collision_wall_left() || piece.collision_grid(grid)) {
                        piece.move_right();
                    }
                } else if (!button_left_status) {
                    button_left_timer = 0;
                }
                
                //check for collisions with ground and other squares
                //revert the piece to the last position if there is a collision
                bool place = 0;
                for (int i = 0; i < 4; i++) {
                    if(piece.collision_floor() || piece.collision_grid(grid)) {
                        place = 1;
                        piece.pos.y++; //revert
                        //border_y = border_cubes[0].p.y;
                        break;
                    }
                    
                }

                bool game_over = 0;


                //draw the piece
                //record to the grid if there was a collision
                //if a cell is already filled then GAME OVER!
                int min_collision_distance = 1000;
                int min_y = 1000;
                for (int i = 0; i < 4; i++) {
                    int collision_distance = 0;
                    int x = piece.shp[i].x+piece.pos.x;
                    int y = piece.shp[i].y+piece.pos.y;
                    if (y < min_y) {
                        min_y = y;
                    }
                    GRID* square = &grid[x][y];
                    draw_cube(square->coord, tetris_size, VECTOR3(0,0,0), CHSV(piece.hue,200,255), false, true);
                    if(place) {
                        if (square->state == GRID_FILLED) {
                            game_over = 1;
                            tetris_state = TETRIS_GAME_OVER;
                        }
                        reset_cell(x,y);
                        square->state = GRID_FILLED;
                        square->hue = piece.hue;
                        square->param = piece.cnt;
                    }

                    for (int y2 = y; y2 >= 0; y2--) {
                        if (y2 == 0 || grid[x][y2-1].state == GRID_FILLED) {
                            break;
                        }
                        collision_distance++;
                    }

                    if (collision_distance < min_collision_distance) {
                        min_collision_distance = collision_distance;
                    }

                }

                //draw a "ghost" piece to indicate where the piece will land
                for (int i = 0; i < 4; i++) {
                    int x = piece.shp[i].x+piece.pos.x;
                    int y = piece.shp[i].y+piece.pos.y-min_collision_distance;
                    if (y < min_y) {
                        draw_cube(grid[x][y].coord, VECTOR3(TETRIS_SIZE/2-tetris_spacing,TETRIS_SIZE/2-tetris_spacing,TETRIS_SIZE/2-tetris_spacing), VECTOR3(0,0,0), CHSV(piece.hue,128,255), false, true, 12, true);
                    }
                }
            

                if (game_over) {
                    //game is over,destroy all blocks
                    for (int y = 0; y < TETRIS_HEIGHT; y++) {
                        for (int x = 0; x < TETRIS_WIDTH; x++) {
                            if (grid[x][y].state == GRID_FILLED) {
                                grid[x][y].param = (TETRIS_HEIGHT-y)*100;
                                grid[x][y].state = GRID_GAME_OVER;
                            }
                        }
                    }
                }

                //generate a new piece
                if (place) {
                    new_piece();
                }
            }


    }

    void draw_border() {
        static int gravity_animation = 0;
        static int animation = 0;
        animation+=8;
        
        if (gravity_animation == 1) {
            gravity = (gravity*96)/100;
            if (gravity == 1) {
                gravity_animation = 2;
            }
        }
        if (gravity_animation > 1) {
            gravity_animation++;
            gravity = -1000;
            cube_repel = 10;
            if(gravity_animation == 100) {
                gravity_animation = 0;
                gravity = 1000;
                cube_repel = 50;
                for (int i = 0; i < NUM_TETRIS_CUBES; i++) {
                    cubes[i].active = false;
                }
            }
        }

        for (int i = 0; i < NUM_BORDER_CUBES; i++) {

            CHSV hsv = CHSV(255,0,128);
            VECTOR3 tetris_size_mod = VECTOR3(0,0,0);
            uint8_t alpha = 255;
            int y = (border_cubes[i].p.y-grid[0][0].coord.y)/TETRIS_SIZE;

            //row gauge
            if(y >= 0 && y < 9) {
                tetris_size_mod = tetris_size/4;
                alpha = 64;
                if (y > (animation*10)/256) {
                    hsv.v = 255;
                    alpha = 192;
                }
            }

            //highlight rows cleared
            if (y < rows_cleared % 10 && y >= 0) {
                hsv.v = 255;
                alpha = 192;
            }

            static uint8_t level_change = 128;
            static uint8_t level_change_cnt = 4;

            //trigger animation every 10 rows
            if (y == 9) {
                if (previous_level != rows_cleared/10) {
                    previous_level = rows_cleared/10;
                    level_change = 128;
                    level_change_cnt = 4;
                    animation = 0;
                    gravity_animation = 1;
                }
                hsv.h = 96;
                hsv.v = 127 + level_change;
                hsv.s = 127 + level_change;
                if (level_change) {
                    level_change-=8;
                } else if (level_change_cnt) {
                    level_change_cnt--;
                    level_change = 128;
                }
            }

            //draw the cube
            draw_cube(border_cubes[i].p, tetris_size - tetris_size_mod, VECTOR3(0,0,0), hsv, false, true, alpha);

            //some animation stuff
            if (border_y == border_cubes[i].p.y) {
                border_cubes[i].p.z -= 1;
                border_cubes[i].v.z = -TETRIS_SIZE/10;
            }

            if (border_cubes[i].p.z < grid[0][0].coord.z) {
                border_cubes[i].p += border_cubes[i].v;
                border_cubes[i].v.z += TETRIS_SIZE/50;
            } else {
                border_cubes[i].p.z = grid[0][0].coord.z;
            }

        }

        if (border_y >= border_cubes[0].p.y) {
            border_y += TETRIS_SIZE;
        }
        if (border_y > border_cubes[TETRIS_HEIGHT+1].p.y) {
            border_y = grid[0][0].coord.y - 1;
        }
        
    }

};

LIGHT_SKETCHES::REGISTER<TETRIS> tetris("tetris");

#endif