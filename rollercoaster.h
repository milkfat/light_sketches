#ifndef LIGHTS_ROLLER_COASTER_H
#define LIGHTS_ROLLER_COASTER_H

#include "text.h"
#include "draw_line_fine_new2.h"

//ROLLER_COASTER
#define NUM_ROLLER_COASTER_EFFECTS 2
#define NUM_ROLLER_COASTER_CUBES 200
class ROLLER_COASTER: public LIGHT_SKETCH {

  public:
    ROLLER_COASTER () {setup();}
    ~ROLLER_COASTER () {
        
#ifdef ENABLE_MULTITHREAD
        worker_thread_stop();
#endif
    }

  private:
    Z_BUF _z_buffer;
    uint8_t current_effect = 0;

    struct CUBE {
        VECTOR3 p;
        VECTOR3 r;
        bool active = 0;
        int32_t age = 0;
    };
    
    struct LANDSCAPE {
        VECTOR3 p;
        VECTOR3 n;
        bool active = 0;
        int32_t age = 0;
    };

    int current_cube = 0;
    CUBE cubes[NUM_ROLLER_COASTER_CUBES];
    int active_tie = 0;

    int tick_count = 0;
    int frame_count = 0;
    bool do_draw = 0;
    uint32_t frame_time;

    VECTOR3 target_camera_offset;
    uint16_t target_camera_rotate_x;
    uint16_t target_camera_rotate_y;
    uint16_t target_camera_rotate_z;

    int velocity = 380;
    int velocity_z_fraction = 256;
    int grade = 0;
    int distance_traveled = 0;

    uint32_t random_seed;


  public:
    void reset() {
        random_seed = random(0,UINT32_MAX);
        frame_time = millis();
    }

    void setup() {
#ifdef ENABLE_MULTITHREAD
        shape_to_fill.stop = false;
        shape_to_fill.ready = false;
        shape_to_fill.state = 1;
#endif
        reset();
        led_screen.camera_position.z /= 10;
        led_screen.screen_distance = (225*256) * 2;
        led_screen.light_falloff = 10;
        control_variables.add(led_screen.camera_position.z, "Camera Z", 0, 256*256*256);
        control_variables.add(led_screen.screen_distance, "Screen Z", 0, 256*256*50);
        control_variables.add(led_screen.light_falloff, "Light Distance:", 1, 16);   
    }

    void next_effect() {
        current_effect++;
        current_effect%=NUM_ROLLER_COASTER_EFFECTS;
    }

    #define NUM_LANDSCAPE 3840

    int landscape_distance = 800;
    int landscape_segment_width = 50;
    int landscape_segment_height = 50;
    int landscape_width = 24;
    int landscape_variation = 4;

    int current_landscape = 0;
    LANDSCAPE landscape_points[NUM_LANDSCAPE];
    int landscape_traveled = 0;
    int change_rate = 50;
    int track_y = 0;
    int track_x = 0;
    int x_samples = 0;
    int pre_load = 2000;

    void handle_track() {
        generate_track();
        update_track();
    }
    
    void handle_landscape() {
        generate_landscape();
        update_landscape();
    }

    void loop() {

        while ( frame_time < millis() ) {
            frame_time += 16;
            if (pre_load) {
                pre_load--;
                frame_time -= 16;
            }

            distance_traveled += (velocity*velocity_z_fraction)/256;
            landscape_traveled += (velocity*velocity_z_fraction)/256;

#ifdef ENABLE_MULTITHREAD
            std::thread th1(&ROLLER_COASTER::handle_track,this);
            handle_landscape();
            th1.join();
#endif
            //generate_track();
            //generate_landscape();
            //update_landscape();
            //update_track();
            handle_track();
            handle_landscape();
            update_velocity();
        }

        LED_show();
        LED_black();

        update_camera();

        draw();
        
        frame_count++;

    }

    void draw() {
        //draw landscape
        for (int i0 = current_landscape-landscape_width*2+NUM_LANDSCAPE*2; i0 > current_landscape-landscape_width*2 + NUM_LANDSCAPE; i0 -= landscape_width) {
            int i = i0 % NUM_LANDSCAPE;
            VECTOR3 points_a[landscape_width];
            VECTOR3 points_b[landscape_width];
            VECTOR3 normals_a[landscape_width];
            VECTOR3 normals_b[landscape_width];
            bool tests_a[landscape_width];
            bool tests_b[landscape_width];

            if ( landscape_points[i%NUM_LANDSCAPE].active && landscape_points[(i+landscape_width)%NUM_LANDSCAPE].active ) {

                CRGB rgb = CRGB(24,72,12);

                for (int j = 0; j < landscape_width; j++) {
                    points_a[j] = landscape_points[(i+j)%NUM_LANDSCAPE].p;
                    normals_a[j] = landscape_points[(i+j)%NUM_LANDSCAPE].n;
                    points_b[j] = landscape_points[(i+j+landscape_width)%NUM_LANDSCAPE].p;
                    normals_b[j] = landscape_points[(i+j+landscape_width)%NUM_LANDSCAPE].n;
                }

                for (int j = 0; j < landscape_width; j++) {
                    led_screen.matrix.rotate_camera(points_a[j]);
                    led_screen.matrix.rotate_camera(points_b[j]);
                    tests_a[j] = led_screen.perspective(points_a[j]);
                    tests_b[j] = led_screen.perspective(points_b[j]);
                }

                VECTOR3 norm2 = VECTOR3(0,256,0);

                for (int j = 0; j < landscape_width-1; j++) {
                    if (tests_a[j] || tests_a[j+1] || tests_b[j]) {
                        draw_triangle_fine(points_a[j], points_a[j+1], points_b[j], normals_a[j], normals_a[j+1], normals_b[j], rgb);
                    }

                    if (tests_a[j+1] || tests_b[j+1] || tests_b[j]) {
                        draw_triangle_fine(points_a[j+1], points_b[j+1], points_b[j], normals_a[j+1], normals_b[j+1], normals_b[j], rgb);
                    }
                }
                
            }
            
        }
        


        //draw track ties
        for (int i = 0; i < NUM_ROLLER_COASTER_CUBES; i++) {
            if (cubes[i].active) {
                draw_cube(cubes[i].p, VECTOR3(8*256, 1*256/10, 1*256), cubes[i].r, CHSV(24,96,196), false, true);
                if (i%10 == 0) {
                    VECTOR3 support = cubes[i].p;
                    support.y -= 100*256;
                    draw_cube(support, VECTOR3(1*256/2, 100*256, 1*256/2), VECTOR3(0,0,0), CHSV(24,96,196), false, true);
                }
            }
        }

        handle_text();

        //draw rails (after everything else)
        for (int i = 0; i < NUM_ROLLER_COASTER_CUBES; i++) {
            if (cubes[i].active) {

                if (i+1 != current_cube) {

                    CRGB rgb = CRGB(64,64,64);

                    //right rail
                    {
                        VECTOR3 a0 = VECTOR3(7*256, 1*256, 0);
                        VECTOR3 b0 = VECTOR3(6*256, 1*256, 0);
                        VECTOR3 a1 = VECTOR3(7*256, 1*256, 0);
                        VECTOR3 b1 = VECTOR3(6*256, 1*256, 0);

                        rotate16(a0,cubes[i].r);
                        rotate16(b0,cubes[i].r);
                        rotate16(a1,cubes[(i+1)%NUM_ROLLER_COASTER_CUBES].r);
                        rotate16(b1,cubes[(i+1)%NUM_ROLLER_COASTER_CUBES].r);

                        a0 += cubes[i].p;
                        b0 += cubes[i].p;
                        a1 += cubes[(i+1)%NUM_ROLLER_COASTER_CUBES].p;
                        b1 += cubes[(i+1)%NUM_ROLLER_COASTER_CUBES].p;

                        VECTOR3 norm = normal(a0, b1, b0);

                        led_screen.matrix.rotate_camera(a0);
                        led_screen.matrix.rotate_camera(b0);
                        led_screen.matrix.rotate_camera(a1);
                        led_screen.matrix.rotate_camera(b1);
                        rotate_x(norm,26);
                        rotate_y(norm,40);

                        bool test = false;

                        test = led_screen.perspective(a0) || test;
                        test = led_screen.perspective(b0) || test;
                        test = led_screen.perspective(a1) || test;
                        test = led_screen.perspective(b1) || test;

                        if (test) {

                            y_buffer->reset();
                            reset_x_buffer();
                            

                            draw_line_fine_new2(led_screen, a0, b0);
                            draw_line_fine_new2(led_screen, a1, b1);
                            draw_line_fine_new2(led_screen, a0, a1);
                            draw_line_fine_new2(led_screen, b0, b1);

                            uint8_t bri = _min(_max((norm.z*7)/8,0)+32,255);
                            color_scale(rgb, z_brightness(led_screen, a0.z));
                            color_scale(rgb, bri);
                            fill_shape_no_antialias(_max(_max(_max(a0.z,b0.z),a1.z),b1.z)+256, rgb);
                        }

                    }

                    //left rail
                    
                    {
                        VECTOR3 a0 = VECTOR3(-7*256, 1*256, 0);
                        VECTOR3 b0 = VECTOR3(-6*256, 1*256, 0);
                        VECTOR3 a1 = VECTOR3(-7*256, 1*256, 0);
                        VECTOR3 b1 = VECTOR3(-6*256, 1*256, 0);

                        rotate16(a0,cubes[i].r);
                        rotate16(b0,cubes[i].r);
                        rotate16(a1,cubes[(i+1)%NUM_ROLLER_COASTER_CUBES].r);
                        rotate16(b1,cubes[(i+1)%NUM_ROLLER_COASTER_CUBES].r);

                        a0 += cubes[i].p;
                        b0 += cubes[i].p;
                        a1 += cubes[(i+1)%NUM_ROLLER_COASTER_CUBES].p;
                        b1 += cubes[(i+1)%NUM_ROLLER_COASTER_CUBES].p;

                        VECTOR3 norm = normal(a0, b1, b0);

                        led_screen.matrix.rotate_camera(a0);
                        led_screen.matrix.rotate_camera(b0);
                        led_screen.matrix.rotate_camera(a1);
                        led_screen.matrix.rotate_camera(b1);
                        rotate_x(norm,56);
                        rotate_y(norm,10);

                        bool test = false;

                        test = led_screen.perspective(a0) || test;
                        test = led_screen.perspective(b0) || test;
                        test = led_screen.perspective(a1) || test;
                        test = led_screen.perspective(b1) || test;

                        if (test) {

                            y_buffer->reset();
                            reset_x_buffer();

                            draw_line_fine_new2(led_screen, a0, b0);
                            draw_line_fine_new2(led_screen, a1, b1);
                            draw_line_fine_new2(led_screen, a0, a1);
                            draw_line_fine_new2(led_screen, b0, b1);

                            fill_shape_no_antialias(_max(_max(_max(a0.z,b0.z),a1.z),b1.z)+256, rgb);
                        }
                    }

                }
            }
        }
    }//draw()

    void update_camera() {
        //interpolate camera position
        VECTOR3 a_offset = VECTOR3(cubes[active_tie].p.x, cubes[active_tie].p.y, 0);
        VECTOR3 b_offset = VECTOR3(cubes[(active_tie+1)%NUM_ROLLER_COASTER_CUBES].p.x, cubes[(active_tie+1)%NUM_ROLLER_COASTER_CUBES].p.y, 0);
        int a_ratio = abs(cubes[(active_tie+1)%NUM_ROLLER_COASTER_CUBES].p.z - 20*256);
        int b_ratio = abs(cubes[active_tie].p.z - 20*256);
        a_offset *= a_ratio;
        b_offset *= b_ratio;
        target_camera_offset = (a_offset+b_offset)/(a_ratio+b_ratio);

        //interpolate camera rotation
        VECTOR3 a_rotation = cubes[(active_tie+1)%NUM_ROLLER_COASTER_CUBES].r;
        VECTOR3 b_rotation = cubes[(active_tie+2)%NUM_ROLLER_COASTER_CUBES].r;
        a_rotation*=a_ratio;
        b_rotation*=b_ratio;
        VECTOR3 rotation = (a_rotation+b_rotation)/(a_ratio+b_ratio);

        VECTOR3 v = VECTOR3(0,0,256);
        rotate16_y(v, rotation.y);
        velocity_z_fraction = v.z;

        VECTOR3 camera_mod = VECTOR3(0, 30*256, 0);
        rotate16_z(camera_mod, rotation.z);
        target_camera_offset += camera_mod;

        

        led_screen.camera_offset = target_camera_offset;
        led_screen.camera_rotate_x = rotation.x*3/4;
        led_screen.camera_rotate_y = rotation.y*3/4;
        led_screen.camera_rotate_z = rotation.z/2;
    }//update_camera()
    
   void generate_track() {
       //generate track
        if (distance_traveled > 16*179) {
            distance_traveled -= 16*179;
            static int tick_count_prev = 0;
            static int tick_count_now = 0;
            static int tick_count_next = 0;
            tick_count_prev = tick_count_now;
            tick_count_now = tick_count_next;
            tick_count_next += 16 * inoise8(tick_count, tick_count, random_seed);
            tick_count++;
            cubes[current_cube].p.x = 0;
            cubes[current_cube].p.y = -15*256;
            cubes[current_cube].p.z = -(landscape_distance+landscape_segment_height)*256;
            cubes[current_cube].p.x += (inoise16(tick_count_now,0,random_seed)-32768)*3;
            cubes[current_cube].p.y += (inoise16(0,tick_count_now,random_seed)-32768)*3;
            cubes[current_cube].r = VECTOR3(0,0,0);

            track_y = _min(track_y, cubes[current_cube].p.y);
            track_x += cubes[current_cube].p.x;
            x_samples++;


            int prev_x = (inoise16(tick_count_prev,0,random_seed)-32768)*3;
            int prev_y = -15*256 + (inoise16(0,tick_count_prev,random_seed)-32768)*3;

            int next_x = (inoise16(tick_count_next,0,random_seed)-32768)*3;
            int next_y = -15*256 + (inoise16(0,tick_count_next,random_seed)-32768)*3;


            cubes[current_cube].r.x = (atan2(next_y - prev_y, (-600*256+16*179) - (-600*256-16*179))*65536)/(2*PI);
            cubes[current_cube].r.y = (atan2(next_x - prev_x, (-600*256+16*179) - (-600*256-16*179))*65536)/(2*PI);
            cubes[current_cube].r.z = (cubes[(NUM_ROLLER_COASTER_CUBES + current_cube - 1)%NUM_ROLLER_COASTER_CUBES].r.y - cubes[current_cube].r.y)*2;



            cubes[current_cube].active = true;
            current_cube++;
            if (current_cube >= NUM_ROLLER_COASTER_CUBES) {
                current_cube = 0;
            }
        }
    }
    #define LANDSCAPE_SCALE 20
    #define LANDSCAPE_HEIGHT 256
    void generate_landscape() {
        //generate landscape
        if (landscape_traveled > landscape_segment_height*256) {
            landscape_traveled -= landscape_segment_height*256;
            int x = track_x/x_samples;
            int inner_y = -30;
            int lx = (-landscape_segment_width*landscape_width*256)/2+(landscape_segment_width*256)/2;
            static int noise_y = 0;
            noise_y++;
            for (int i = 0; i < landscape_width; i++) {
                int ly = ((inoise8(i*LANDSCAPE_SCALE, noise_y*LANDSCAPE_SCALE, 0)-128)*LANDSCAPE_HEIGHT)/256; //random height

                VECTOR3 a = VECTOR3(i*LANDSCAPE_SCALE, inoise8(i*LANDSCAPE_SCALE, noise_y*LANDSCAPE_SCALE, 0)-128, 0);
                VECTOR3 b = VECTOR3(i*LANDSCAPE_SCALE, inoise8(i*LANDSCAPE_SCALE, (noise_y-1)*LANDSCAPE_SCALE, 0)-128, landscape_segment_height*256);
                VECTOR3 c = VECTOR3((i-1)*LANDSCAPE_SCALE, inoise8((i-1)*LANDSCAPE_SCALE, noise_y*LANDSCAPE_SCALE, 0)-128, 0);

                landscape_points[current_landscape].n = normal(a,c,b);

                if ( track_y < (50+ly+inner_y)*256 && abs(lx-x) < 240*256) {
                    int dist = 240*256 - abs(lx-x);
                    dist /= 256;
                    int offset = (track_y - (50+ly+inner_y)*256);
                    offset = (offset * dist) / 240;
                    inner_y += offset / 256;
                }

                landscape_points[current_landscape].p = VECTOR3(random(landscape_variation)+lx + x,(ly+inner_y)*256,-landscape_distance*256);
                landscape_points[current_landscape].active = true;
                current_landscape++;
                current_landscape %= NUM_LANDSCAPE;
                lx+=landscape_segment_width*256;
            }
            track_y = INT32_MAX;
            track_x = 0;
            x_samples = 0;
        }
   }

   void update_landscape() {
       //update landscape
        for ( int i = 0; i < NUM_LANDSCAPE; i++) {
            landscape_points[i].p.z += (velocity*velocity_z_fraction)/256;
            if ( landscape_points[i].p.z > 250*256 ) {
                landscape_points[i].active = false;
            }
        }

   }

   void update_track() {
        //update track
        for (int i = 0; i < NUM_ROLLER_COASTER_CUBES; i++) {
            if (cubes[i].active) {

                //tie
                
                bool z_test = cubes[i].p.z > 20*256;
                cubes[i].p.z += (velocity*velocity_z_fraction)/256;

                if (!z_test && cubes[i].p.z > 20*256) {
                    active_tie = i;
                } 

                if (cubes[i].p.z > 40*256) {
                    cubes[i].active = false;
                }

            }
        }
   }

   void update_velocity() {

        //update velocity
        VECTOR3 grade_v = VECTOR3(0,0,256);
        rotate16(grade_v, cubes[active_tie].r);

        grade = grade_v.y;
        //led_screen.camera_offset.x = cubes[i].p.x;
        if (tick_count % 3 == 0) {
            velocity *= 499;
            velocity /= 500;
        }

        velocity += grade/45;
        velocity = _max(_min(velocity, 1300), 400);

   }

};

LIGHT_SKETCHES::REGISTER<ROLLER_COASTER> roller_coaster("Roller Coaster");

#endif