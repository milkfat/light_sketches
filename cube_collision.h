#ifndef LIGHTS_CUBE_COLLISION_H
#define LIGHTS_CUBE_COLLISION_H


class CUBE_COLLISION: public LIGHT_SKETCH {
  public:
    CUBE_COLLISION () {setup();}
    ~CUBE_COLLISION () {}

  private:
    struct TEST_CUBE {
        VECTOR3 p;
        VECTOR3 s;
        VECTOR3 r;
        CHSV hsv;
    };

    TEST_CUBE cube1;
    TEST_CUBE cube2;

    Z_BUF _zbuffer;

    bool button_up_status = false;
    bool button_left_status = false;
    bool button_right_status = false;
    bool button_down_status = false;
    bool button_forward_status = false;
    bool button_backward_status = false;
    bool button_rotate_z0_status = false;
    bool button_rotate_z1_status = false;
    bool button_rotate_x0_status = false;
    bool button_rotate_x1_status = false;
    bool button_rotate_y0_status = false;
    bool button_rotate_y1_status = false;
    bool cube_select = false;

  public:

    void setup ()
    {
        cube1.p = VECTOR3(10*512,0,0);
        cube1.s = VECTOR3(10*256,10*256,10*256);
        cube1.r = VECTOR3(0,0,0);
        cube1.hsv = CHSV(0,96,255);
        cube2.p = VECTOR3(10*-512,0,0);
        cube2.s = VECTOR3(10*256,10*256,10*256);
        cube2.r = VECTOR3(0,0,0);
        cube2.hsv = CHSV(96,96,255);
        led_screen.camera_position.z = 100*256;
        led_screen.screen_distance = 1024*256;
        control_variables.add(led_screen.camera_position.z, "Camera Z", 0, 1024*256);
        control_variables.add(led_screen.screen_distance, "Screen Z", 0, 1024*256);
        //control_variables.add(led_screen.light_falloff, "Light Distance:", 1, 16);
        control_variables.add(button_up_status,"Up", 1, 38);
        control_variables.add(button_left_status,"Left", 1, 37);
        control_variables.add(button_right_status,"Right", 1, 39);
        control_variables.add(button_down_status,"Down", 1, 40);
        //control_variables.add(button_forward_status,"Forward", 1, 87);
        //control_variables.add(button_backward_status,"Backward", 1, 83);
        control_variables.add(button_rotate_z0_status,"Rotate Z", 1, 65);
        control_variables.add(button_rotate_z1_status,"Rotate Z", 1, 68);
        control_variables.add(button_rotate_x0_status,"Rotate X", 1, 81);
        control_variables.add(button_rotate_x1_status,"Rotate X", 1, 69);
        control_variables.add(button_rotate_y0_status,"Rotate Y", 1, 90);
        control_variables.add(button_rotate_y1_status,"Rotate Y", 1, 67);
        control_variables.add(cube_select,"Cube Select", 0, 70);
    }

    void reset ()
    {

    }

    void next_effect()
    {

    }

    void loop()
    {
        if ( button_up_status ) {
            if ( cube_select ) {
                cube1.p.y+=40;
            } else {
                cube2.p.y+=40;
            }
        }
        if ( button_down_status ) {
            if ( cube_select ) {
                cube1.p.y-=40;
            } else {
                cube2.p.y-=40;
            }
        }
        if ( button_left_status ) {
            if ( cube_select ) {
                cube1.p.x-=40;
            } else {
                cube2.p.x-=40;
            }
        }
        if ( button_right_status ) {
            if ( cube_select ) {
                cube1.p.x+=40;
            } else {
                cube2.p.x+=40;
            }
        }
        if ( button_backward_status ) {
            if ( cube_select ) {
                cube1.p.z-=40;
            } else {
                cube2.p.z-=40;
            }
        }
        if ( button_forward_status ) {
            if ( cube_select ) {
                cube1.p.z+=40;
            } else {
                cube2.p.z+=40;
            }
        }
        if ( button_rotate_z0_status ) {
            if ( cube_select ) {
                cube1.r.z-=4;
            } else {
                cube2.r.z-=4;
            }
        }
        if ( button_rotate_z1_status ) {
            if ( cube_select ) {
                cube1.r.z+=4;
            } else {
                cube2.r.z+=4;
            }
        }
        if ( button_rotate_x0_status ) {
            if ( cube_select ) {
                cube1.r.x-=4;
            } else {
                cube2.r.x-=4;
            }
        }
        if ( button_rotate_x1_status ) {
            if ( cube_select ) {
                cube1.r.x+=4;
            } else {
                cube2.r.x+=4;
            }
        }
        if ( button_rotate_y0_status ) {
            if ( cube_select ) {
                cube1.r.y-=4;
            } else {
                cube2.r.y-=4;
            }
        }
        if ( button_rotate_y1_status ) {
            if ( cube_select ) {
                cube1.r.y+=4;
            } else {
                cube2.r.y+=4;
            }
        }
        if (abs(cube1.p.x - cube2.p.x) < 20*256 && abs(cube1.p.y - cube2.p.y) < 20*256 && abs(cube1.p.z - cube2.p.z) < 20*256) {
            //collision
            draw_cube(cube1.p, cube1.s, cube1.r);
            draw_cube(cube2.p, cube2.s, cube2.r);
        } else {
            draw_cube(cube1.p, cube1.s, cube1.r, cube1.hsv);
            draw_cube(cube2.p, cube2.s, cube2.r, cube2.hsv);
        }
        LED_show();
        LED_black();
    }

};


LIGHT_SKETCHES::REGISTER<CUBE_COLLISION> cube_collision("cube_collision");
//END NEON

#endif