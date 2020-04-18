#ifndef LIGHTS_SQUARES_H
#define LIGHTS_SQUARES_H

class SQUARES: public LIGHT_SKETCH {

    #define SQUARE_X_OFFSET ((-MATRIX_WIDTH*256)/2+4*256)
    #define SQUARE_Y_OFFSET ((-MATRIX_HEIGHT*256)/2+4*256)

    struct SQUARE {
        VECTOR3 a = VECTOR3(-4*256,-4*256,0);
        VECTOR3 b = VECTOR3(4*256,-4*256,0);
        VECTOR3 c = VECTOR3(-4*256,4*256,0);
        VECTOR3 d = VECTOR3(4*256,4*256,0);
        //VECTOR3 na = VECTOR3(0,0,255);
        VECTOR3 na = VECTOR3(-64,-64,255);
        VECTOR3 nb = VECTOR3(64,-64,255);
        VECTOR3 nc = VECTOR3(-64,64,255);
        VECTOR3 nd = VECTOR3(64,64,255);

        void rotate__y(uint8_t ang) {
            rotate_y(a,ang);
            rotate_y(b,ang);
            rotate_y(c,ang);
            rotate_y(d,ang);
            rotate_y(na,ang);
            rotate_y(nb,ang);
            rotate_y(nc,ang);
            rotate_y(nd,ang);
        }

        void rotate() {
            a+=VECTOR3(SQUARE_X_OFFSET,SQUARE_Y_OFFSET,led_screen.camera_position.z-led_screen.screen_distance);
            b+=VECTOR3(SQUARE_X_OFFSET,SQUARE_Y_OFFSET,led_screen.camera_position.z-led_screen.screen_distance);
            c+=VECTOR3(SQUARE_X_OFFSET,SQUARE_Y_OFFSET,led_screen.camera_position.z-led_screen.screen_distance);
            d+=VECTOR3(SQUARE_X_OFFSET,SQUARE_Y_OFFSET,led_screen.camera_position.z-led_screen.screen_distance);

            led_screen.matrix.rotate(a);
            led_screen.matrix.rotate(b);
            led_screen.matrix.rotate(c);
            led_screen.matrix.rotate(d);
            led_screen.matrix.rotate(na);
            led_screen.matrix.rotate(nb);
            led_screen.matrix.rotate(nc);
            led_screen.matrix.rotate(nd);
        }

        void perspective() {
            led_screen.perspective(a);
            led_screen.perspective(b);
            led_screen.perspective(c);
            led_screen.perspective(d);
        }

        void draw_me() {
            draw_triangle_fine(b,d,c,nb,nd,nc);
            draw_triangle_fine(a,b,c,na,nb,nc);
            na = -na;
            nb = -nb;
            nc = -nc;
            nd = -nd;
            draw_triangle_fine(b,c,d,nb,nc,nd);
            draw_triangle_fine(a,c,b,na,nc,nb);
            
            //draw_triangle_fine(b,d,c,nb,nd,nc);
            //draw_triangle_fine(a,b,c,na,nb,nc);
            //draw_triangle_fine(b,c,d,nb2,nc2,nd2);
            //draw_triangle_fine(a,c,b,na2,nc2,nb2);
        }

        void draw() {
            rotate();
            perspective();
            draw_me();
        }

        //overload +=
        inline SQUARE& operator+= (const VECTOR3& rhs) {
            a+=rhs;
            b+=rhs;
            c+=rhs;
            d+=rhs;
            return *this;
        }

    };

    struct GRID_SQUARE {
        uint16_t ang;
        uint16_t spd;
    };
    
    GRID_SQUARE grid_squares[MATRIX_WIDTH/8][MATRIX_HEIGHT/8];
    Z_BUF _z_buffer;
    Y_BUF2 _y_buffer2;

    #define NUM_SQUARES_EFFECTS 4
    uint8_t current_effect = 0;

  public:
    SQUARES () {setup();}
    ~SQUARES () {}

    void next_effect() {
        current_effect++;
        current_effect%=NUM_SQUARES_EFFECTS;
        led_screen.reset_camera();
        switch (current_effect) {
            case 0:
                break;
            case 1:
                led_screen.screen_distance = 55*256;
                break;
            case 2:
                led_screen.camera_position = VECTOR3(-1500,33614,2012);
                led_screen.screen_distance = 33792;
                led_screen.rotation_beta = 149;
                break;
            case 3:
                led_screen.camera_position = VECTOR3(-24253,20044,-41845);
                led_screen.screen_distance = 13312;
                led_screen.rotation_alpha = 25;
                led_screen.rotation_beta = 114;
                led_screen.rotation_gamma = 1;
            default:
                break;
        }

    }

    void reset() {setup();}
    
    void setup() {
        for (uint8_t x = 0; x < MATRIX_WIDTH/8; x++) {
            for (uint8_t y = 0; y < MATRIX_HEIGHT/8; y++) {
                grid_squares[x][y].ang = random(65536);
                grid_squares[x][y].spd = x*256+random(384);
            }
        }
        z_buffer = &_z_buffer;
        y_buffer2 = &_y_buffer2;
    }

    void draw_square(const VECTOR3& a, const uint8_t& ang) {
        SQUARE square;
        square.rotate__y(ang);
        square+=a;
        square.draw();
    }

    void loop() {
        LED_show();
        LED_black();


        for (uint8_t x = 0; x < MATRIX_WIDTH/8; x++) {
            for (uint8_t y = 0; y < MATRIX_HEIGHT/8; y++) {
                draw_square(VECTOR3(x*8*256,y*8*256,0), grid_squares[x][y].ang/256);
                grid_squares[x][y].ang+=easeSwing(grid_squares[x][y].spd)*2;
                grid_squares[x][y].spd++;
            }
        }
        


    }
};   


LIGHT_SKETCHES::REGISTER<SQUARES> squares("squares");

#endif