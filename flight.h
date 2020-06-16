#ifndef LIGHTS_FLIGHT_H
#define LIGHTS_FLIGHT_H

//PARTICLES

class FLIGHT: public LIGHT_SKETCH {
  public:
    FLIGHT () {setup();}
    ~FLIGHT () {}
  private:
    #define NUM_FLIGHT_EFFECTS 1
    int current_effect = 0;
    Z_BUF _z_buffer;
    uint32_t default_size = 15*256;

  public:
    void reset() {
    }

    void setup() {
        z_buffer = &_z_buffer;
        led_screen.light_falloff = 10;
        led_screen.camera_position.z = 29865;
        led_screen.screen_distance = 126794;
        control_variables.add(led_screen.camera_position.z, "Camera Z:", 0, 2048*256);
        control_variables.add(led_screen.screen_distance, "Screen Z:", 0, 2048*256);
        control_variables.add(default_size, "cube length", 1, 150*256);
    }

    void next_effect() {
        current_effect++;
        current_effect %= NUM_FLIGHT_EFFECTS;
    }
    void loop() {
        LED_show();
        LED_black();

        switch (current_effect) {
            case 0:
                handle_flight();
                break;
            default:
                break;

        }
        

    }

    void handle_flight() {
        static uint16_t r = 0;
        if(!button2_down) 
        {
        r+=48/8;
        }
        VECTOR3 rv = VECTOR3(0,0,r);
        static int z_pos = 0;
        if(!button2_down) 
        {
        z_pos+=1024/8;
        }
        static uint8_t base_hue = 0;
        if (z_pos > 100*256) {
            z_pos -= 100*256;
            base_hue+=32;
        }
        uint8_t hue = base_hue;
        int z = z_pos/10;
        static uint16_t alpha = 0;
        static uint16_t beta = 0;
        if(!button2_down) 
        {
        alpha+=170/8;
        beta+=110/8;
        }
        led_screen.rotation_alpha = 0 + sin16(alpha)/6000.f;
        led_screen.rotation_beta = 90 + cos16(beta)/6000.f;

        for (int j = -1; j < 13; j+=1) {
            hue+=32;
            int this_many = 1;
            uint32_t size = default_size;
            static uint32_t val = 0;
            val++;
            //uint8_t x_pos = inoise8(val,0,0);
            for (int i = 0; i < this_many; i++) {
                    VECTOR3 a = VECTOR3(5*256+i*10*256,0,-j*10*256+z);
                    VECTOR3 b = VECTOR3(-5*256-i*10*256,0,-j*10*256+z);
                    VECTOR3 c = VECTOR3(0,5*256+i*10*256,-j*10*256+z+2*256);
                    VECTOR3 d = VECTOR3(0,-5*256-i*10*256,-j*10*256+z+2*256);
                    rotate16(a,rv);
                    rotate16(b,rv);
                    rotate16(c,rv);
                    rotate16(d,rv);
                    a*=10;
                    b*=10;
                    c*=10;
                    d*=10;
                    draw_cube(a, VECTOR3(10*256,default_size,10*256), rv, CHSV(hue,192,255), false, true);
                    draw_cube(b, VECTOR3(10*256,default_size,10*256), rv, CHSV(hue,192,255), false, true);

                    draw_cube(c, VECTOR3(default_size,10*256,10*256), rv, CHSV(hue,192,255), false, true);
                    draw_cube(d, VECTOR3(default_size,10*256,10*256), rv, CHSV(hue,192,255), false, true);

            }
        }
    }

    void handle_plus() {
        static uint16_t r = 0;
        r+=64;
        VECTOR3 rv = VECTOR3(r,r,r);
        VECTOR3 p0 = VECTOR3(-30*256,0,0);
        VECTOR3 p1 = VECTOR3(0,-30*256,0);
        VECTOR3 p2 = VECTOR3(0,0,-30*256);
        rotate16(p0, rv);
        rotate16(p1, rv);
        rotate16(p2, rv);
        draw_cube(p0, VECTOR3(20*256,10*256,10*256), rv, CHSV(0,0,255), false, true);
        draw_cube(p1, VECTOR3(10*256,20*256,10*256), rv, CHSV(0,0,255), false, true);
        draw_cube(p2, VECTOR3(10*256,10*256,20*256), rv, CHSV(0,0,255), false, true);
        draw_cube(-p0, VECTOR3(20*256,10*256,10*256), rv, CHSV(0,0,255), false, true);
        draw_cube(-p1, VECTOR3(10*256,20*256,10*256), rv, CHSV(0,0,255), false, true);
        draw_cube(-p2, VECTOR3(10*256,10*256,20*256), rv, CHSV(0,0,255), false, true);
    }


};

LIGHT_SKETCHES::REGISTER<FLIGHT> flight("flight");

#endif