#ifndef LIGHTS_TUNNEL_H
#define LIGHTS_TUNNEL_H

#include "text.h"

//TUNNEL
#define NUM_TUNNEL_EFFECTS 2
class TUNNEL: public LIGHT_SKETCH {
  public:
    TUNNEL () {setup();}
    ~TUNNEL () {}
  private:
    Z_BUF _z_buffer;
    Y_BUF2 _y_buffer2;
    uint8_t current_effect = 0;


    struct TRIANGLE {
        VECTOR3 a;
        VECTOR3 b;
        VECTOR3 c;
        VECTOR3 norm;
        CRGB rgb_a;
        CRGB rgb_b;
        CRGB rgb_c;
        bool active = 0;
    };

    #define NUM_TRIANGLES 120
    uint16_t current_triangle = 0;
    TRIANGLE triangles[NUM_TRIANGLES];

    void add_triangle(VECTOR3& a, VECTOR3& b, VECTOR3& c, const VECTOR3& norm, CRGB rgb_a, CRGB rgb_b, CRGB rgb_c) {
        triangles[current_triangle].a = a;
        triangles[current_triangle].b = b;
        triangles[current_triangle].c = c;
        //triangles[current_triangle].norm = normal(a,b,c);
        triangles[current_triangle].norm = norm;
        triangles[current_triangle].active = true;
        triangles[current_triangle].rgb_a = rgb_a;
        triangles[current_triangle].rgb_b = rgb_b;
        triangles[current_triangle].rgb_c = rgb_c;
        current_triangle++;
        current_triangle%=NUM_TRIANGLES;
    }

  public:
    void reset() {
    }

    void setup() {
        z_buffer = &_z_buffer;
        y_buffer2 = &_y_buffer2;
        reset();
        led_screen.screen_distance = 225*256;
        control_variables.add(led_screen.camera_position.z, "Camera Z", 0, 256*256);
        control_variables.add(led_screen.screen_distance, "Screen Z", 0, 256*256*8);
        
    }

    void next_effect() {
        current_effect++;
        current_effect%=NUM_TUNNEL_EFFECTS;
    }
    void loop() {
        LED_show();
        z_buffer->reset();
        LED_black();
        do_triangle_thing();
        
    }
    #define TUNNEL_SPEED (256*8)
    #define TUNNEL_CIRCLE_DETAIL 8
    #define TUNNEL_DEPTH_DETAIL 48
    #define TUNNEL_START_DISTANCE 90
    void do_triangle_thing() {
        static int32_t cnt = 0;
        if (!current_effect) {
            cnt+=TUNNEL_SPEED;
        }
        static int32_t cnt2 = 0;
        cnt2+=10;

        if (cnt == TUNNEL_DEPTH_DETAIL*256) {
        static int8_t rotation_speed = 8;
        static int8_t rotation_speed_start = 8;
        static int8_t rotation_speed_end = -8;
        static uint16_t rotation_speed_stp = 0;
        static uint8_t rotation_speed_stp_size = 10;
        static uint16_t tunnel_radius = 35;
        static uint16_t last_tunnel_radius = 35;
        static uint8_t frq = 4;
        rotation_speed = rotation_speed_start + ((rotation_speed_end - rotation_speed_start)*ease8InOutQuad(rotation_speed_stp))/255;
        rotation_speed_stp+=rotation_speed_stp_size;
        if (rotation_speed_stp > 255) {
            rotation_speed_stp%=256;
            rotation_speed_start = rotation_speed_end;
            rotation_speed_end = random(-20,20);
            rotation_speed_stp_size = random(5,25);
        }
        // last_tunnel_radius = tunnel_radius;
        // tunnel_radius = 25 + inoise8(cnt2,0,0)/4;
        static uint8_t ang = 0;
        static uint8_t hue = 0;
            cnt = 0;
            hue++;
            int8_t skip = -1;
            for (int i = 0; i < 256; i+=TUNNEL_CIRCLE_DETAIL) {
                skip++;
                skip%=frq;
                if (skip) {
                    continue;
                }
                VECTOR3 a;
                VECTOR3 b;
                VECTOR3 c;
                VECTOR3 c2;
                VECTOR3 d;
                a.x = (cos8(i+ang)-128)*last_tunnel_radius;
                a.y = (sin8(i+ang)-128)*last_tunnel_radius;
                a.z = -TUNNEL_START_DISTANCE*256;
                b.x = (cos8(i+ang+TUNNEL_CIRCLE_DETAIL)-128)*last_tunnel_radius;
                b.y = (sin8(i+ang+TUNNEL_CIRCLE_DETAIL)-128)*last_tunnel_radius;
                b.z = -TUNNEL_START_DISTANCE*256;
                c.x = (cos8(i+ang+rotation_speed)-128)*tunnel_radius;
                c.y = (sin8(i+ang+rotation_speed)-128)*tunnel_radius;
                c.z = -(TUNNEL_START_DISTANCE+TUNNEL_DEPTH_DETAIL)*256;
                c2.x = (cos8(i+ang)-128)*tunnel_radius;
                c2.y = (sin8(i+ang)-128)*tunnel_radius;
                c2.z = -(TUNNEL_START_DISTANCE+TUNNEL_DEPTH_DETAIL)*256;
                d.x = (cos8(i+ang+rotation_speed+TUNNEL_CIRCLE_DETAIL)-128)*tunnel_radius;
                d.y = (sin8(i+ang+rotation_speed+TUNNEL_CIRCLE_DETAIL)-128)*tunnel_radius;
                d.z = -(TUNNEL_START_DISTANCE+TUNNEL_DEPTH_DETAIL)*256;
                VECTOR3 n = (normal(a,d,c2) + VECTOR3(0,0,255))/2;
                add_triangle(a,d,c,n,CHSV(hue+i,255,255),CHSV(hue+i+TUNNEL_CIRCLE_DETAIL,255,255),CHSV(hue+i,255,255));
                add_triangle(a,b,d,n,CHSV(hue+i,255,255),CHSV(hue+i+TUNNEL_CIRCLE_DETAIL,255,255),CHSV(hue+i+TUNNEL_CIRCLE_DETAIL,255,255));
                //add_triangle(a,d,c,VECTOR3(0,0,255),CHSV(hue+i,255,255),CHSV(hue+i+TUNNEL_CIRCLE_DETAIL,255,255),CHSV(hue+i,255,255));
                //add_triangle(a,b,d,VECTOR3(0,0,255),CHSV(hue+i,255,255),CHSV(hue+i+TUNNEL_CIRCLE_DETAIL,255,255),CHSV(hue+i+TUNNEL_CIRCLE_DETAIL,255,255));
            }
            ang += rotation_speed;
        }

        for(int i = NUM_TRIANGLES-1; i >= 0; i--) {
            if (triangles[i].active) {
                if (!current_effect) {
                    triangles[i].a.z += TUNNEL_SPEED;
                    triangles[i].b.z += TUNNEL_SPEED;
                    triangles[i].c.z += TUNNEL_SPEED;
                }
                 triangles[i].a.z = _min(triangles[i].a.z, led_screen.camera_position.z-45*256);
                 triangles[i].b.z = _min(triangles[i].b.z, led_screen.camera_position.z-45*256);
                 triangles[i].c.z = _min(triangles[i].c.z, led_screen.camera_position.z-45*256);
               
                VECTOR3 pa;
                VECTOR3 pb;
                VECTOR3 pc;
                VECTOR3 pn;
                

                //rotate with our global matrix
                led_screen.matrix.rotate_camera(triangles[i].a, pa);
                led_screen.matrix.rotate_camera(triangles[i].b, pb);
                led_screen.matrix.rotate_camera(triangles[i].c, pc);
                led_screen.matrix.rotate(triangles[i].norm, pn);
                //correct 3d perspective
                led_screen.perspective(pa);
                led_screen.perspective(pb);
                led_screen.perspective(pc);
                if (pa.z >= led_screen.camera_position.z-50*256 && pb.z >= led_screen.camera_position.z-50*256 && pc.z >= led_screen.camera_position.z-50*256) {
                    triangles[i].active = false;
                    continue;
                }

                 //bool test = draw_triangle_rgb( pa,pb,pc,pn,pn,pn,triangles[i].rgb_a,triangles[i].rgb_b,triangles[i].rgb_c );
                // bool test2 = draw_triangle_rgb( pa,pc,pb,pn,pn,pn,triangles[i].rgb_a,triangles[i].rgb_b,triangles[i].rgb_c );
                bool test = draw_triangle_fine( pa,pb,pc,pn,pn,pn,triangles[i].rgb_a );
                // draw_line_fine(led_screen, pa, pb, triangles[i].rgb_a);
                // draw_line_fine(led_screen, pb, pc, triangles[i].rgb_a);
                // draw_line_fine(led_screen, pc, pb, triangles[i].rgb_a);
                //bool test2 = draw_triangle( pa,pc,pb,pn,pn,pn,triangles[i].rgb_a );
                if (!current_effect && !(test)) {
                     triangles[i].active = false;
                }

            }
        }
    }

};

LIGHT_SKETCHES::REGISTER<TUNNEL> tunnel("Tunnel");

#endif