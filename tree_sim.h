#ifndef LIGHTS_TREE_SIM_H
#define LIGHTS_TREE_SIM_H

class TREE_SIM: public LIGHT_SKETCH {public:
    TREE_SIM () {setup();}
    ~TREE_SIM () {}
  
    VECTOR3 tree_coords[500];
    uint8_t tree_radians[500];
    CRGB tree_leds[500];
    
  
  public:
        
    void next_effect() {}


    void reset() {}


    void setup() {
        for (int i = 0; i < 500; i++) {
            tree_coords[i] = VECTOR3(0, -150*256, 30*256);
            tree_leds[i] = CRGB(8, 8, 8);
            rotate_y(tree_coords[i], i*17);
            tree_radians[i] = i*17;
            tree_coords[i].y = (i*65535)/499;
            std::cout << tree_coords[i].x << " " << tree_coords[i].y << " " << tree_coords[i].z << "\n";
        }
         
    }
    

    void loop() {

        // //RED
        // static uint8_t pos = 0;
        // pos+=1;
        // for (int i = 0; i < 500; i++) {
        //     uint8_t twist = tree_coords[i].y/128;
        //     twist += pos;

        //     uint8_t twist_dist = 128 - abs(abs(tree_radians[i] - twist) - 128);
        //     if (  twist_dist < 48 ) {
        //         uint8_t inverse_twist_dist = ((47-twist_dist)*255)/47;
        //         tree_leds[i].r = inverse_twist_dist;
        //     } else {
        //         tree_leds[i].r = 8;
        //     }
        // }
        static uint8_t pos_array[400] = {0};
        static uint16_t pos = 0;
        pos-=911;
        for (int i = 398; i >= 0; i--) {
            pos_array[i+1]=pos_array[i];
        }
        pos_array[0] = pos/256;

        for (int i = 0; i < 500; i++) {
             uint8_t dist = 128 - abs(abs(tree_radians[i] - pos_array[(tree_coords[i].y*60)/65535]) - 128);
             if (dist < 48) {
                uint8_t inverse_dist = ((47-dist)*255)/47;
                tree_leds[i].r = inverse_dist;
             } else {
                tree_leds[i].r = 8;
            }
        }
        
        for (int i = 0; i < 500; i++) {
            VECTOR3 p = tree_coords[i];
            p.y-=150*256;
            MATRIX::rotate(p);
            if (p.z >= 0) {
                scale_z(p);
                led_screen.perspective(p);
                blendXY(led_screen,p,tree_leds[i]);
            }
        }

        LED_show();
        LED_black();
        
    }

};   


//LIGHT_SKETCHES::REGISTER<TREE_SIM> tree_sim("tree sim");

#endif