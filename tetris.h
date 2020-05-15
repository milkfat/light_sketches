#ifndef LIGHTS_TETRIS_H
#define LIGHTS_TETRIS_H

#include "text.h"

//PARTICLES

class TETRIS: public LIGHT_SKETCH {
  public:
    TETRIS () {
        setup();
        control_variables.add(button_rotate_status,"Rotate", 1, 38);
        control_variables.add(button_left_status,"Left", 1, 37);
        control_variables.add(button_right_status,"Right", 1, 39);
        control_variables.add(button_down_status,"Down", 1, 40);
    }
    ~TETRIS () {}
  private:
    #define NUM_TETRIS_EFFECTS 1
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

    enum grid_states {
        GRID_EMPTY,
        GRID_FILLED,
        GRID_SHRINKING,
        GRID_BRIGHTENING,
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

    #define TETRIS_WIDTH 10
    #define TETRIS_HEIGHT (MATRIX_HEIGHT/(MATRIX_WIDTH/TETRIS_WIDTH))
    #define TETRIS_SIZE ((MATRIX_WIDTH/TETRIS_WIDTH)*256)
    GRID grid[TETRIS_WIDTH][TETRIS_HEIGHT];

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
                int y = shp[i].y+pos.y;
                if(x < 0) {
                    return true;
                }     
            }
            return false;
        }

        bool collision_wall_right() {
            for (int i = 0; i < 4; i++) {
                int x = shp[i].x+pos.x;
                int y = shp[i].y+pos.y;
                if(x > TETRIS_WIDTH-1) {
                    return true;
                }     
            }
            return false;
        }



        bool collision_floor() {
            for (int i = 0; i < 4; i++) {
                int x = shp[i].x+pos.x;
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
        grid[x][y].sat = 255;
        grid[x][y].bri = 255;
        grid[x][y].param = 0;
    }

    void new_piece() {
        uint8_t s = random(7);
        //s = 0;
        piece.shp[0] = shapes[s][0];
        piece.shp[1] = shapes[s][1];
        piece.shp[2] = shapes[s][2];
        piece.shp[3] = shapes[s][3];
        piece.pos.x = TETRIS_WIDTH/2;
        piece.pos.y = TETRIS_HEIGHT-4;
        piece.hue = random(255);
        piece.rgb = CHSV(piece.hue,255,255);
        piece.t = millis()+500;
        piece.cnt++;
    }

    bool rotate_left = false;

  public:
    void reset() {
        setup();
    }

    void setup() {
        score = 0;

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

        new_piece();

        z_buffer = &_z_buffer;
        led_screen.light_falloff = 10;

        border_width = (MATRIX_WIDTH%TETRIS_WIDTH)/2;

        for (int x = 0; x < TETRIS_WIDTH; x++) {
            int x_coord = border_width*256+TETRIS_SIZE/2+(TETRIS_SIZE)*x;
            for (int y = 0; y < TETRIS_HEIGHT; y++) {
                int y_coord = border_width*256+TETRIS_SIZE/2+(TETRIS_SIZE)*y;
                grid[x][y].coord.x = x_coord-MATRIX_WIDTH*256/2-128;
                grid[x][y].coord.y = y_coord-MATRIX_HEIGHT*256/2-128;
                grid[x][y].coord.z = led_screen.camera_position.z-led_screen.screen_distance-TETRIS_SIZE/2;
                reset_cell(x,y);
            }
        }


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

        char buffer [50];
        int n = sprintf (buffer, "%d", score);
        for (int i = 0; i < n; i++) {
            draw_character(5, buffer[i], border_width*256+2*256+i*4*256, (MATRIX_HEIGHT-10)*256, CRGB(255,255,255), 255, 0);
        }

        LED_show();
        LED_black();
        draw_border();

        switch (current_effect) {
            case 0:
                handle_tetris();
                break;
            default:
                break;

        }
        

    }

    void handle_tetris() {

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
                    for (int x = 0; x < TETRIS_WIDTH; x++) {
                        grid[x][y].state = GRID_BRIGHTENING;
                        grid[x][y].param = 155+abs(most_recent_cell-x)*(100-abs(most_recent_cell-x));
                    }
                }

                //handle brightening
                //if done, then mark for next effect
                for (int x = 0; x < TETRIS_WIDTH; x++) {
                    if (grid[x][y].state == GRID_BRIGHTENING){
                        VECTOR3 size = VECTOR3(TETRIS_SIZE/2-128,TETRIS_SIZE/2-128,TETRIS_SIZE/2-128);
                        grid[x][y].sat = _min(grid[x][y].param, 255);
                        draw_cube(grid[x][y].coord, size, VECTOR3(0,0,0), CHSV(grid[x][y].hue,grid[x][y].sat,grid[x][y].bri));
                        grid[x][y].param -= 100;
                        if (grid[x][y].param <= 0) {
                            grid[x][y].state = GRID_SHRINKING;
                            grid[x][y].param = 255;
                        }
                    }
                }

                //handle shrinking
                //if done, mark the cell as dead
                for (int x = 0; x < TETRIS_WIDTH; x++) {
                    if (grid[x][y].state == GRID_SHRINKING){
                        VECTOR3 size = VECTOR3(TETRIS_SIZE/2-128,TETRIS_SIZE/2-128,TETRIS_SIZE/2-128);
                        size *= _min(grid[x][y].param, 255);
                        size /= 255;
                        draw_cube(grid[x][y].coord, size, VECTOR3(0,0,0), CHSV(grid[x][y].hue,grid[x][y].sat,grid[x][y].bri));
                        grid[x][y].param -= 20;
                        if (grid[x][y].param <= 0) {
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
                            grid[x][y2-1].offset.y += TETRIS_SIZE;
                            grid[x][y2].state = GRID_EMPTY;
                        }
                    }
                }

            }

            //draw existing squares on the grid
            for (int y = 0; y < TETRIS_HEIGHT; y++) {
                for (int x = 0; x < TETRIS_WIDTH; x++) {

                    if(grid[x][y].offset.y > 0) {
                        grid[x][y].velocity.y -= 20;
                        grid[x][y].offset.y += grid[x][y].velocity.y;
                        if(grid[x][y].offset.y < 0) {
                            grid[x][y].offset.y = 0;
                            grid[x][y].velocity.y = 0;
                        }
                    }

                    if(grid[x][y].state == GRID_FILLED) {
                        draw_cube(grid[x][y].coord + grid[x][y].offset, VECTOR3(TETRIS_SIZE/2-128,TETRIS_SIZE/2-128,TETRIS_SIZE/2-128), VECTOR3(0,0,0), CHSV(grid[x][y].hue,grid[x][y].sat,grid[x][y].bri));
                    }
                    
                }
            }

            //make the piece fall
            if (button_down_status || millis() > piece.t) {
                piece.pos.y--;
                piece.t = millis()+500;
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
                    break;
                }
                
            }

            bool game_over = 0;

            //draw the piece
            //record to the grid if there was a collision
            //if a cell is already filled then GAME OVER!
            for (int i = 0; i < 4; i++) {
                int x = piece.shp[i].x+piece.pos.x;
                int y = piece.shp[i].y+piece.pos.y;
                GRID* square = &grid[x][y];
                draw_cube(square->coord, VECTOR3(TETRIS_SIZE/2-128,TETRIS_SIZE/2-128,TETRIS_SIZE/2-128), VECTOR3(0,0,0), CHSV(piece.hue,255,255));
                if(place) {
                    if (square->state == GRID_FILLED) game_over = 1;
                    reset_cell(x,y);
                    square->state = GRID_FILLED;
                    square->hue = piece.hue;
                    square->param = piece.cnt;
                }
            }

            if (game_over) {
                //game is over,destroy all blocks
                for (int y = 0; y < TETRIS_HEIGHT; y++) {
                    for (int x = 0; x < TETRIS_WIDTH; x++) {
                        if (grid[x][y].state == GRID_FILLED) {
                            grid[x][y].param = (TETRIS_HEIGHT-y)*100;
                            grid[x][y].state = GRID_BRIGHTENING;
                        }
                    }
                }
                score = 0;
            }

            //generate a new piece
            if (place) {
                new_piece();
            }


    }

    void draw_border() {
        for (int i = 0; i < border_width; i++) {
            for (int y = 0; y < MATRIX_HEIGHT; y++) {
                leds[XY(i,y)].r = 96;
                leds[XY(MATRIX_WIDTH-1-i,y)].r = 96;
            }
            for (int x = 0; x < MATRIX_WIDTH; x++) {
                leds[XY(x,i)].r = 96;
            }
        }
    }

};

LIGHT_SKETCHES::REGISTER<TETRIS> tetris("tetris");

#endif