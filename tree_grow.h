#ifndef LIGHTS_TREE_H
#define LIGHTS_TREE_H


//TREE
#define NUM_TREE_EFFECTS 2
#define NUM_TREE_SEGMENTS 1000
class TREE: public LIGHT_SKETCH {
  public:
    TREE () {setup();}
    ~TREE () {}
  private:
    Z_BUF _z_buffer;
    uint8_t current_effect = 0;
    //uint8_t debug_slider = 0;
    int grow_speed = 16;

    struct TREE_SEGMENT {
        VECTOR3 start;
        VECTOR3 end;
        VECTOR3 velocity;
        //VECTOR3 end_debug;
        //VECTOR3 velocity_debug;
        int32_t length;
        bool active = 0;
        uint32_t next_branch_left;
        uint32_t next_branch_right;
        int distance = 1;
        bool stop_growing = 0;
        uint16_t parent = 0;
    };

    TREE_SEGMENT tree_segments[NUM_TREE_SEGMENTS];
    int current_segment = 1;

  public:
    void reset() {
        for (int i = 0; i < NUM_TREE_SEGMENTS; i++) {
            tree_segments[i].start = VECTOR3(MATRIX_WIDTH*128, 0, 0);
            tree_segments[i].start = VECTOR3(0, 0, 0);
            tree_segments[i].velocity = VECTOR3(0, 256, 0);
            //tree_segments[i].velocity_debug = VECTOR3(0, 256, 0);
            tree_segments[i].length = 0;
            tree_segments[i].active = 0;
            tree_segments[i].stop_growing = 0;
            tree_segments[i].next_branch_left = random(1*60,5*60);
            tree_segments[i].next_branch_right = random(1*60,5*60);
        }
        tree_segments[0].active = 1;
        current_segment = 1;
    }

    void setup() {
        reset();
        led_screen.screen_distance = 201472;
        led_screen.camera_position.z = 159744;
        led_screen.light_falloff = 12;
        control_variables.add(led_screen.camera_position.z, "Camera Z", 0, 256*256);
        control_variables.add(led_screen.screen_distance, "Screen Z", 0, 256*256*8);
        control_variables.add(led_screen.light_falloff, "Light Distance:", 1, 16);
        control_variables.add(grow_speed, "Grow Rate:", 1, 256);
        //control_variables.add(debug_slider, "Debug Slider:", 0, 256);
        
    }

    void next_effect() {
        current_effect++;
        current_effect%=NUM_TREE_EFFECTS;
    }

    void loop() {
        LED_show();
        //z_buffer->reset();
        LED_black();

        create_tree();

        draw_tree();

        draw_cube(VECTOR3(0,0,0), VECTOR3(10*256,10*256,10*256));
        draw_cube(VECTOR3(100*256,0,0), VECTOR3(10*256,10*256,10*256), VECTOR3(0,0,0), CHSV(160,255,255));
        draw_cube(VECTOR3(-100*256,0,0), VECTOR3(10*256,10*256,10*256), VECTOR3(0,0,0), CHSV(0,255,255));
        draw_cube(VECTOR3(0,0,-100*256), VECTOR3(10*256,10*256,10*256), VECTOR3(0,0,0), CHSV(96,255,255));
        draw_cube(VECTOR3(0,0,100*256), VECTOR3(10*256,10*256,10*256), VECTOR3(0,0,0), CHSV(48,255,255));

        
    }

    void create_tree() {

        int cnt = grow_speed;
        while (tree_segments[0].length < MATRIX_HEIGHT*128 && cnt--) {
            bool stop = true;
            for (int i = 0; i < NUM_TREE_SEGMENTS; i++) {
                if (tree_segments[i].active && !tree_segments[i].stop_growing) {
                    stop = false;
                    if ( i == 0) {
                        tree_segments[i].length += MATRIX_HEIGHT/10;
                    } else {
                        tree_segments[i].length += MATRIX_HEIGHT/20;
                    }

                    tree_segments[i].end = tree_segments[i].start;
                    tree_segments[i].end += (tree_segments[i].velocity*tree_segments[i].length)/256;

                    //tree_segments[i].end_debug = tree_segments[i].start;
                    //tree_segments[i].end_debug += (tree_segments[i].velocity_debug*tree_segments[i].length)/256;
                    
                    tree_segments[i].next_branch_left--;
                    tree_segments[i].next_branch_right--;

                    if (tree_segments[i].next_branch_left == 0) {
                        new_branch_left(i);
                    }

                    if (tree_segments[i].next_branch_right == 0) {
                        new_branch_right(i);
                    }

                    if (i != 0 && random(5000/tree_segments[i].distance) == 0) {
                        tree_segments[i].stop_growing = true;
                    }

                }
            }
            if (stop) break;
        }

    }

    void draw_tree() {

        // CRGB rgb_debug = CRGB(0,255,255);
        for (int i = 0; i < NUM_TREE_SEGMENTS; i++) {
            if (tree_segments[i].active) {
                VECTOR3 a;
                VECTOR3 b;
                led_screen.matrix.rotate_camera(tree_segments[i].start, a);
                led_screen.matrix.rotate_camera(tree_segments[i].end, b);
                led_screen.perspective(a);
                led_screen.perspective(b);
                VECTOR3 a2 = a;
                VECTOR3 width = VECTOR3( (tree_segments[i].length*256)/50,0,0);
                if (i > 0) {
                    VECTOR3 parent_velocity = tree_segments[tree_segments[i].parent].velocity;
                    led_screen.matrix.rotate(parent_velocity);
                    width = parent_velocity*(tree_segments[i].length/50);
                }
                width /= 256;
                a -= width;
                a2 += width;
                VECTOR3 norm = normal(a,b,a2);
                norm.unit();
                uint8_t bri = abs(norm.z);
                CRGB rgb = CRGB(bri,bri,bri);
                draw_line_fine_new(led_screen, a, b);
                draw_line_fine_new(led_screen, a2, b);
                draw_line_fine_new(led_screen, a2, a);
                fill_shape_z(0, rgb);
                y_buffer->reset();
                reset_x_buffer();
                //VECTOR3 b_debug;
                //led_screen.matrix.rotate(tree_segments[i].end_debug, b_debug);
                //led_screen.perspective(b_debug);
                //draw_line_fine(led_screen, a, b_debug, rgb_debug);
            }
        }

    }

    void new_branch(int i, int dir) {
        if (current_segment < NUM_TREE_SEGMENTS) {
            tree_segments[current_segment].distance = tree_segments[i].distance + 1;
            tree_segments[current_segment].parent = i;
            //tree_segments[current_segment].angle.x += (8+random(32))/tree_segments[i].distance*dir;


            VECTOR3 velocity = tree_segments[i].velocity;
            uint8_t angle_y = (atan2(-velocity.x, -velocity.z)*256)/(2*PI);
            rotate_y(velocity, angle_y);
            uint8_t angle_x = (atan2(-velocity.y, -velocity.z)*256)/(2*PI);
            rotate_x(velocity, angle_x);

            VECTOR3 velocity2 = VECTOR3(0,0,-256);
            rotate_x(velocity2, (16+random(32)) + tree_segments[i].distance*dir);
            rotate_z(velocity2, random(256));

            rotate_x(velocity2, -angle_x);
            rotate_y(velocity2, -angle_y);

            // VECTOR3 velocity3 = velocity2;
            // uint8_t angle_y2 = (atan2(-velocity3.x, -velocity3.z)*256)/(2*PI);
            // rotate_y(velocity3, angle_y2);
            // uint8_t angle_x2 = (atan2(-velocity3.y, -velocity3.z)*256)/(2*PI);
            // rotate_x(velocity3, angle_x2);

            tree_segments[current_segment].velocity = velocity2;
            //tree_segments[current_segment].velocity_debug = velocity3;
            tree_segments[current_segment].start = tree_segments[i].end;
            tree_segments[current_segment].length = 0;
            tree_segments[current_segment].active = 1;
            tree_segments[current_segment].distance = tree_segments[i].distance + 1;
            tree_segments[current_segment].next_branch_left = random((tree_segments[i].distance)*60,(10+tree_segments[i].distance)*60);
            tree_segments[current_segment].next_branch_right = random((tree_segments[i].distance)*60,(10+tree_segments[i].distance)*60);
            current_segment++;
        }
    }

    void new_branch_left(int i) {
        new_branch(i, -1);
        tree_segments[i].next_branch_left = random((tree_segments[i].distance)*60,(2+tree_segments[i].distance)*60);
    }

    void new_branch_right(int i) {
        new_branch(i, 1);
        tree_segments[i].next_branch_right = random((tree_segments[i].distance)*60,(2+tree_segments[i].distance)*60);
    }
    

};

LIGHT_SKETCHES::REGISTER<TREE> tree("Tree");

#endif