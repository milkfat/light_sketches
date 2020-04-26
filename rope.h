#ifndef LIGHTS_ROPE_PHYSICS_H
#define LIGHTS_ROPE_PHYSICS_H

//ROPE


class ROPE_PHYSICS: public LIGHT_SKETCH {
    #define NUM_JOINTS 17
    #define ROPE_SEGMENT_LENGTH (3*256)
    #define NUM_ROPE_EFFECTS 1
    #define NUM_ROPES 7
    #define NUM_BALLS 21
    #define BALLS_PER_ROW 3
    #define MAX_BALL_RADIUS (7*256)

  public:
    ROPE_PHYSICS () {setup();}
    ~ROPE_PHYSICS () {}
  private:
    int current_effect = 0;
    CRGB rgb = CRGB(255,255,255);
    Z_BUF _z_buffer;

    struct JOINT {
        bool s = false; //static
        VECTOR3 p; //position
        VECTOR3 np; //next position
        VECTOR3 v; //velocity
    };

    struct ROPE {
        JOINT joints[NUM_JOINTS];
        CRGB rgb;
    };


    ROPE ropes[NUM_ROPES];

    struct BALL {
        VECTOR3 sp; //static position
        VECTOR3 offset;
        VECTOR3 ov; //offset velocity
        VECTOR3 p; //position
        VECTOR3 v; //velocity
        uint16_t r; //radius
    };

    BALL balls[NUM_BALLS];

    void reset_rope(int j) {
        JOINT * rope = ropes[j].joints;
        int r = random(8*256)-4*256;
        int r2 = random(5*256);
        for (int i = 0; i < NUM_JOINTS; i++) {
            rope[i].s = false;
            rope[i].p = VECTOR3(MATRIX_WIDTH*256/2-((NUM_JOINTS-1)*ROPE_SEGMENT_LENGTH)/2 + i*ROPE_SEGMENT_LENGTH,(MATRIX_HEIGHT+2)*256,0);
            rope[i].p.x += r;
            rope[i].p.y += r2;
            rope[i].p.z = 0;
            rope[i].np = rope[i].p;
            //rope[i].b = VECTOR3((MATRIX_WIDTH/2)*256,(MATRIX_HEIGHT-20-10*i)*256,0);
            rope[i].v = VECTOR3(0,0,0);
        }
        ropes[j].rgb = CHSV(random(256),255,255);
    }

    void reset_ball(int i) {
        balls[i].r =  MAX_BALL_RADIUS/2 + random(MAX_BALL_RADIUS/2);
        //balls[i].r = 4*256;
        balls[i].p.x = -(MAX_BALL_RADIUS+256);
        balls[i].p.y = MAX_BALL_RADIUS+512+((i/BALLS_PER_ROW)*(MATRIX_HEIGHT)*256)/(NUM_BALLS/BALLS_PER_ROW);
        balls[i].v.x = (fmix32(i/BALLS_PER_ROW)&0xF)+15;
        if ((i/BALLS_PER_ROW)%2) {
             balls[i].p.x += MATRIX_WIDTH*256+MAX_BALL_RADIUS*2+512;
             balls[i].v.x = -balls[i].v.x;
        }
        balls[i].v.y = 0;
        balls[i].v.z = 0;
        balls[i].p.z = 0;

        balls[i].sp = balls[i].p;
        balls[i].offset = VECTOR3(0,0,0);
        balls[i].ov = VECTOR3(0,0,0);

    }

  public:
    void reset() {
        setup();
    }

    void setup() {
        z_buffer = &_z_buffer;
        for (int i = 0; i < NUM_BALLS; i++) {
            reset_ball(i);
            balls[i].p.x = -MAX_BALL_RADIUS + (i%BALLS_PER_ROW) * ((MATRIX_WIDTH*256+MAX_BALL_RADIUS*2+MAX_BALL_RADIUS/2)/(BALLS_PER_ROW));
            balls[i].sp = balls[i].p;
            
        }
        for (int j = 0; j < NUM_ROPES; j++) {
            reset_rope(j);
        }
    }

    void next_effect() {
        for (int j = 0; j < NUM_ROPES; j++) {
            JOINT * rope = ropes[j].joints;
            rope[0].s = !rope[0].s;
            rope[NUM_JOINTS-1].s = !rope[NUM_JOINTS-1].s;
            current_effect++;
            current_effect %= NUM_ROPE_EFFECTS;
        }
    }
    void loop() {
        LED_show();
        LED_black();

        switch (current_effect) {
            case 0:
                handle_rope();
                break;
            default:
                break;

        }
        

    }

    void handle_rope() {
        if (button2_down) {
            for (int j = 0; j < NUM_ROPES; j++) {
                JOINT * rope = ropes[j].joints;
                rope[NUM_JOINTS/2].v.x -= 100;
            }
        }
        
        //move balls
        for (int i = 0; i < NUM_BALLS; i++) {
            balls[i].sp.x += balls[i].v.x;
            if (balls[i].sp.x < -(MAX_BALL_RADIUS+512) || balls[i].sp.x > MATRIX_WIDTH*256+MAX_BALL_RADIUS+512) {
                reset_ball(i);
            }
            balls[i].offset += balls[i].ov;
            balls[i].ov -= balls[i].offset/50;
            balls[i].ov *= 85;
            balls[i].ov /= 100;
            balls[i].p = balls[i].sp+balls[i].offset;
        }

        //apply gravity and find next position

        for (int j = 0; j < NUM_ROPES; j++) {
            JOINT * rope = ropes[j].joints;
            bool off_screen = true;
            for (int i = 0; i < NUM_JOINTS; i++) {
                if (!rope[i].s) {
                    rope[i].v.y -= 10;
                    rope[i].np = rope[i].p + rope[i].v;
                }
                if (rope[i].p.y >= 0) {
                    off_screen = false;
                }
            }
            if (off_screen) {
                reset_rope(j);
            }
        }

        //solve physics
        int cnt = 4;
        while (cnt--) {
            
            for (int k = 0; k < NUM_ROPES; k++) {
                JOINT * rope = ropes[k].joints;
                for (int i = 1; i < NUM_JOINTS; i++) {
                    if (rope[i].s && rope[i-1].s) break;
                    for (int j = 0; j < NUM_BALLS; j++) {
                        if (
                            rope[i].p.x > balls[j].p.x-(balls[j].r+128)
                            && rope[i].p.x < balls[j].p.x+(balls[j].r+128)
                            && rope[i].p.y > balls[j].p.y-(balls[j].r+128)
                            && rope[i].p.y < balls[j].p.y+(balls[j].r+128)
                            ) {
                            VECTOR3 c = rope[i].np - balls[j].p;
                            uint32_t l = sqrt(c.x*c.x+c.y*c.y+c.z*c.z); //distance between joint and ball
                            if (l < (balls[j].r+128)) {
                                if (l == 0) break;
                                VECTOR3 c0 = c;
                                c0 *= (balls[j].r+128);
                                c0 /= l; //target relative coordinate
                                VECTOR3 adjust = c0 - c; //amount we need to adjust to hit target
                                VECTOR3 adjust2;
                                adjust2.x = sgn(adjust.x)*sqrt16(_max(abs(adjust.x),1));
                                adjust2.y = sgn(adjust.y)*sqrt16(_max(abs(adjust.y),1));
                                adjust2.z = sgn(adjust.z)*sqrt16(_max(abs(adjust.z),1));
                                rope[i].np += adjust-adjust2;
                                balls[j].offset -= adjust2;
                                //blendXY(led_screen, balls[j].p.x, balls[j].p.y);

                            }

                        }
                    }
                    VECTOR3 c = rope[i].np - rope[i-1].np; //current relative coordinate
                    uint32_t l = sqrt(c.x*c.x+c.y*c.y+c.z*c.z); //length of current segment
                    if (l == 0) break;
                    VECTOR3 c0 = c;
                    c0 *= ROPE_SEGMENT_LENGTH;
                    c0 /= l; //target relative coordinate
                    VECTOR3 adjust = c0 - c; //amount we need to adjust to hit target
                    if (!rope[i].s && !rope[i-1].s) {
                        rope[i].np += adjust/2;
                        rope[i-1].np -= adjust/2;
                    } else if (!rope[i].s) {
                        rope[i].np += adjust;
                    } else if (!rope[i-1].s) {
                        rope[i-1].np -= adjust;
                    }
                }
            }
        }

        //final pass

        for (int j = 0; j < NUM_ROPES; j++) {
            JOINT * rope = ropes[j].joints;
            for (int i = 0; i < NUM_JOINTS; i++) {
                rope[i].v = rope[i].np - rope[i].p;
                rope[i].p = rope[i].np;
            }
        }


        for (int j = 0; j < NUM_ROPES; j++) {
            JOINT * rope = ropes[j].joints;
            for (int i = 0; i < NUM_JOINTS-1; i++) {
                draw_line_fine(led_screen, rope[i].p, rope[i+1].p, ropes[j].rgb, -10000, 255, 255, true);
            }
        }

        for (int i = 0; i < NUM_BALLS; i++) {
            reset_x_buffer();
            reset_y_buffer();
            draw_circle_fine(balls[i].p.x, balls[i].p.y, balls[i].r, 0, 255, 16);
            CRGB fart = CHSV(0, 255, 16);
            fart = gamma8_encode(gamma8_encode(fart));
            //fart = gamma8_encode(fart);
            fill_shape(256, fart);
        }
    }


};

LIGHT_SKETCHES::REGISTER<ROPE_PHYSICS> rope_physics("rope_physics");

#endif