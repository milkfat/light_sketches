#ifndef LIGHTS_ROPE_PHYSICS_H
#define LIGHTS_ROPE_PHYSICS_H

//ROPE


class ROPE_PHYSICS: public LIGHT_SKETCH {
    #define MAX_NUM_JOINTS 30
    #define ROPE_SEGMENT_LENGTH (3*256)
    #define NUM_ROPE_EFFECTS 1
    #define MAX_NUM_ROPES 50
    #define BALLS_PER_COLUMN (MATRIX_HEIGHT/10)
    #define BALLS_PER_ROW (MATRIX_WIDTH/10)
    #define NUM_BALLS (BALLS_PER_COLUMN * BALLS_PER_ROW)
    #define MAX_BALL_RADIUS (4*256)
    
    bool draw_balls = 1;
    uint8_t num_joints = 10;
    uint8_t num_ropes = 10;

  public:
    ROPE_PHYSICS () {setup();}
    ~ROPE_PHYSICS () {}
  private:
    int current_effect = 0;
    CRGB rgb = CRGB(255,255,255);
    Z_BUF _z_buffer;
    uint8_t gravity = 10;

    struct JOINT {
        bool s = false; //static
        VECTOR2 p; //position
        VECTOR2 np; //next position
        VECTOR2_16 v; //velocity
        uint8_t friction_cnt = 0;
    };

    struct ROPE {
        JOINT joints[MAX_NUM_JOINTS];
        CRGB rgb;
    };


    ROPE ropes[MAX_NUM_ROPES];

    struct BALL {
        VECTOR2 sp; //static position
        VECTOR2_16 offset;
        VECTOR2_16 ov; //offset velocity
        VECTOR2 p; //position
        VECTOR2_16 v; //velocity
        uint16_t r; //radius
        CRGB rgb;
    };

    BALL balls[NUM_BALLS];

    void reset_rope(int j) {
        JOINT * rope = ropes[j].joints;

        //const uint32_t x_variance = MATRIX_WIDTH*256 - (num_joints * ROPE_SEGMENT_LENGTH);
        const uint32_t x_variance = MATRIX_WIDTH*256;
        int r = random(x_variance*2)-x_variance;
        int r2 = random(10*256);
        for (int i = 0; i < num_joints; i++) {
            rope[i].s = false;
            rope[i].p = VECTOR2(MATRIX_WIDTH*256/2-((num_joints-1)*ROPE_SEGMENT_LENGTH)/2 + i*ROPE_SEGMENT_LENGTH,(MATRIX_HEIGHT+2)*256);
            rope[i].p.x += r;
            rope[i].p.y += r2;
            rope[i].np = rope[i].p;
            //rope[i].b = VECTOR2((MATRIX_WIDTH/2)*256,(MATRIX_HEIGHT-20-10*i)*256,0);
            rope[i].v = VECTOR2(0,0);
        }
        ropes[j].rgb = CHSV(random(256),0,255);
    }

    void reset_ball(int i) {
        balls[i].r =  MAX_BALL_RADIUS/2 + random(MAX_BALL_RADIUS/2);
        //balls[i].r = 4*256;
        balls[i].p.x = -(MAX_BALL_RADIUS+256);
        balls[i].p.y = MAX_BALL_RADIUS+512+((i/BALLS_PER_ROW)*(MATRIX_HEIGHT)*306)/(NUM_BALLS/BALLS_PER_ROW);
        balls[i].v.x = (fmix32(i/BALLS_PER_ROW)&0xF)+15;
        if ((i/BALLS_PER_ROW)%2) {
             balls[i].p.x += MATRIX_WIDTH*256+MAX_BALL_RADIUS*2+512;
             balls[i].v.x = -balls[i].v.x;
        }
        balls[i].v.y = 0;

        balls[i].sp = balls[i].p;
        balls[i].offset = VECTOR2(0,0);
        balls[i].ov = VECTOR2(0,0);

    }

  public:
    void reset() {
        for (int i = 0; i < NUM_BALLS; i++) {
            reset_ball(i);
            balls[i].p.x = -MAX_BALL_RADIUS + (i%BALLS_PER_ROW) * ((MATRIX_WIDTH*256+MAX_BALL_RADIUS*2+MAX_BALL_RADIUS/2)/(BALLS_PER_ROW));
            balls[i].sp = balls[i].p;
            
        }
        for (int j = 0; j < MAX_NUM_ROPES; j++) {
            reset_rope(j);
        }
    }

    void setup() {
        control_variables.add(num_ropes,"Number of Ropes",1,MAX_NUM_ROPES);
        control_variables.add(num_joints,"Number of Joints",1,MAX_NUM_JOINTS);
        control_variables.add(draw_balls,"Draw Balls");
        control_variables.add(gravity,"Gravity",1,50);
        reset();
    }

    void next_effect() {
        for (int j = 0; j < num_ropes; j++) {
            JOINT * rope = ropes[j].joints;
            rope[0].s = !rope[0].s;
            rope[num_joints-1].s = !rope[num_joints-1].s;
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
            for (int j = 0; j < num_ropes; j++) {
                JOINT * rope = ropes[j].joints;
                rope[num_joints/2].v.x -= 100;
            }
        }
        
        //move balls
        for (int i = 0; i < NUM_BALLS; i++) {
            balls[i].rgb = gamma8_encode(CHSV(0,255,32));
            balls[i].sp.x += balls[i].v.x;
            if (balls[i].sp.x < -(MAX_BALL_RADIUS+512) || balls[i].sp.x > MATRIX_WIDTH*256+MAX_BALL_RADIUS+512) {
                reset_ball(i);
            }
            balls[i].offset += balls[i].ov;
            balls[i].ov -= balls[i].offset/240;
            balls[i].ov *= 90;
            balls[i].ov /= 100;
            balls[i].p = balls[i].sp+balls[i].offset;
        }

        //apply gravity and find next position

        for (int j = 0; j < num_ropes; j++) {
            JOINT * rope = ropes[j].joints;
            bool off_screen = true;
            for (int i = 0; i < num_joints; i++) {
                if (!rope[i].s) {
                    rope[i].v.y -= gravity;
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
            
            for (int k = 0; k < num_ropes; k++) {
                JOINT * rope = ropes[k].joints;
                for (int i = 1; i < num_joints; i++) {
                    if (rope[i].s && rope[i-1].s) break;
                    for (int j = 0; j < NUM_BALLS; j++) {
                        //check if we are above this row of balls
                        if ( rope[i].np.y > balls[j].p.y+(MAX_BALL_RADIUS+128) ) {
                            j+=BALLS_PER_ROW-1;
                            continue;
                        }
                        //check if we are below this row of balls
                        if ( rope[i].np.y < balls[j].p.y-(MAX_BALL_RADIUS+128) ) {
                            break;
                        }
                        //check if we are within the bounding square of this ball
                        if (
                            rope[i].np.x > balls[j].p.x-(balls[j].r+128)
                            && rope[i].np.x < balls[j].p.x+(balls[j].r+128)
                            && rope[i].np.y > balls[j].p.y-(balls[j].r+128)
                            && rope[i].np.y < balls[j].p.y+(balls[j].r+128)
                            ) {

                            VECTOR2 c = rope[i].np - balls[j].p; //relative vector between joint and ball
                            uint32_t l = sqrt(c.x*c.x+c.y*c.y); //distance between joint and ball
                            if (l < (balls[j].r+128)) {
                                if (l == 0) break;
                                VECTOR2 c0 = c;
                                c0 *= (balls[j].r+128);
                                c0 /= l; //relative coordinates of target
                                VECTOR2 adjust = c0 - c; //amount we need to adjust to hit target
                                VECTOR2 adjust2;
                                adjust2.x = sgn(adjust.x)*sqrt16(_max(abs(adjust.x),1));
                                adjust2.y = sgn(adjust.y)*sqrt16(_max(abs(adjust.y),1));
                                adjust2 /= 2;
                                rope[i].np += adjust-adjust2;
                                rope[i].friction_cnt+=1;
                                balls[j].offset -= adjust2;
                                //blendXY(led_screen, balls[j].p.x, balls[j].p.y);

                            }

                        }
                    }
                    VECTOR2 c = rope[i].np - rope[i-1].np; //current relative coordinate
                    uint32_t l = sqrt(c.x*c.x+c.y*c.y); //length of current segment
                    if (l == 0) break;
                    VECTOR2 c0 = c;
                    c0 *= ROPE_SEGMENT_LENGTH;
                    c0 /= l; //target relative coordinate
                    VECTOR2 adjust = c0 - c; //amount we need to adjust to hit target
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

        for (int j = 0; j < num_ropes; j++) {
            JOINT * rope = ropes[j].joints;
            for (int i = 0; i < num_joints; i++) {
                rope[i].v = rope[i].np - rope[i].p;
                rope[i].p = rope[i].np;
                rope[i].v -= (rope[i].v*rope[i].friction_cnt)/50;
                rope[i].friction_cnt = 0;
                if (i < num_joints-1) {
                    uint8_t v = (ROPE_SEGMENT_LENGTH*255)/(sqrt(rope[i].v.x*rope[i].v.x+rope[i].v.y*rope[i].v.y)+ROPE_SEGMENT_LENGTH);
                    draw_line_fine(led_screen, rope[i].p, rope[i+1].np, ropes[j].rgb, -10000, v, v, true);
                }
            }
        }
        if (draw_balls) {
            for (int i = 0; i < NUM_BALLS; i++) {
                reset_x_buffer();
                y_buffer->reset();
                draw_circle_fine(balls[i].p.x, balls[i].p.y, balls[i].r, balls[i].rgb, -1, 32);
                //fart = gamma8_encode(gamma8_encode(fart));
                //fart = gamma8_encode(fart);
                fill_shape(256, balls[i].rgb);
            }
        }
    }


};

LIGHT_SKETCHES::REGISTER<ROPE_PHYSICS> rope_physics("rope_physics");

#endif