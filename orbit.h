#ifndef LIGHTS_ORBIT_H
#define LIGHTS_ORBIT_H

//ORBIT

class ORBIT: public LIGHT_SKETCH {
  public:
        ORBIT () {setup();}
    ~ORBIT () {}
  private:
    unsigned long time0 = 0;
    uint8_t angle_x=0;
    uint8_t angle_y=0;
    uint8_t angle_z=0;

  public:
    void reset() {

    }

    void setup() {

    }

    void next_effect() {

    }

    uint32_t loop_time = 0;

    uint16_t cnt_x=0;
    uint16_t cnt_y=0;
    uint16_t cnt_z=0;

    int16_t amount = 0;
    int16_t light = 0;
    int16_t fade = 0;
    uint16_t count = 0;
    
    void loop() {
        count+=8;
        int cnt = 0;
        //4500, 10, 500
        while ( cnt < 1 + (4500*count)/65535 ) {
            cnt++;
            VECTOR3 v(0,6740,0);
            matrix.rotate_x(v,angle_x);
            matrix.rotate_y(v,angle_y);
            matrix.rotate_z(v,angle_z);
            //v.y += (v.y * sin8(count/4))/64;
            matrix.rotate(v);
            matrix.scale_z(v);
            matrix.perspective(v);

            color_add_linear16(leds[XY(v[0]/256, v[1]/256)].r, (65535 - (65279*count)/65535)/32);
            leds[XY(v[0]/256, v[1]/256)].g = leds[XY(v[0]/256, v[1]/256)].r;
            leds[XY(v[0]/256, v[1]/256)].b = leds[XY(v[0]/256, v[1]/256)].r;

            cnt_x++;
            cnt_y++;
            cnt_z++;

            if (random(2)==0) {
                angle_x+=1;
            }
            if (random(3)==0) {
                angle_y+=1;
            }
            if (random(4)==0) {
                angle_z+=1;
            }
            // angle_x+=1;
            // if (cnt_z == 37) {
            //     cnt_z = 0;
            //     angle_z+=1;
            // }
            // if (cnt_y == 17) {
            //     cnt_y = 0;
            //     angle_y+=1;
            // }

            if (cnt%(2+(1000*count)/65535)==0) {
                for (int i = 0; i < NUM_LEDS; i++) {
                    leds[i].r -= 4;
                    leds[i].g -= 4;
                    leds[i].b -= 4;
                }
            }
        }

        time0=millis();

        LED_show();
        LED_black();

    }
};

LIGHT_SKETCHES::REGISTER<ORBIT> orbit("orbit");

#endif