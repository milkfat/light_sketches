#ifndef LIGHTS_TREE_SIM_H
#define LIGHTS_TREE_SIM_H

class TREE_SIM: public LIGHT_SKETCH {public:
    TREE_SIM () {setup();}
    ~TREE_SIM () {}
  
    CRGB tree_leds[NUM_LEDS];
    uint8_t current_effect = 0;
    CRGB rgb0 = CRGB(0, 255, 0);
    CRGB rgb1 = CRGB(0, 0, 255);


    class SPIRAL {
      public:
        uint16_t vertical_speed = 125;
        uint8_t pos_array[400] = {0};
        uint16_t pos = 0;
        int offset = 0;
        int spd = 0;
        int target_spd = 0;
        uint32_t new_spd_time = 0;

        void update() {
            if (millis() > new_spd_time) {
                new_spd_time = millis() + random(3000, 12000);
                target_spd = random(400, 900);
                if (random(2)) {
                    target_spd *= -1;
                }
            }
            spd += (target_spd - spd)/20;
            pos+=spd;
            for (int i = 398; i >= 0; i--) {
                pos_array[i+1]=pos_array[i];
            }
            pos_array[0] = pos/256;
        }
    };

    #define NUM_SPIRALS 2
    SPIRAL spirals[NUM_SPIRALS];

  public:
        
    void next_effect() {
        current_effect++;
        current_effect%=2;

        switch (current_effect) {
            case 0:
                rgb0 = CRGB(0,255,0);
                rgb1 = CRGB(0,0,255);
                break;
            case 1:
                rgb0 = CRGB(0, 128, 255);
                break;
            default:
                break;
        }
    }


    void reset() {}


    void setup() {
        for (int i = 0; i < NUM_SPIRALS; i++) {
            spirals[i].offset = (i*255)/NUM_SPIRALS;
        }

        control_variables.add(rgb0, "Color");
        control_variables.add(rgb1, "Color 2");
         
    }

    

    void loop() {

        if (current_effect == 0) {
            int cnt = 0;
            spirals[0].update();
            while (cnt < NUM_SPIRALS) {
                SPIRAL * spiral = &spirals[0];
                for (int i = 0; i < NUM_LEDS; i++) {
                    int y = 65535-tree_coords[i].y;
                    uint8_t my_pos = spiral->pos_array[(y*spiral->vertical_speed)/65535];
                    uint8_t adjacent_pos;
                    if(tree_coords[i].y > 65536/2) {
                        adjacent_pos = spiral->pos_array[(y*spiral->vertical_speed)/65535 - 1];
                    } else {
                        adjacent_pos = spiral->pos_array[(y*spiral->vertical_speed)/65535 + 1];
                    }
                    my_pos += (cnt*255)/NUM_SPIRALS;
                    adjacent_pos += (cnt*255)/NUM_SPIRALS;
                    int target_dist = 52;
                    //int target_dist = 128 - abs(abs(adjacent_pos - my_pos) - 128);
                    //target_dist *= 24;
                    //target_dist = _max(target_dist, 48);
                    uint8_t dist = 128 - abs(abs(tree_radians[i] - my_pos) - 128);
                    if (dist < target_dist) {
                        uint8_t inverse_dist = (((target_dist-1)-dist)*255)/(target_dist-1);
                        switch (cnt) {
                            case 0:
                                leds[i].r = (rgb0.r*inverse_dist)/256;
                                leds[i].g = (rgb0.g*inverse_dist)/256;
                                leds[i].b = (rgb0.b*inverse_dist)/256;
                                break;
                            case 1:
                                leds[i].r = (rgb1.r*inverse_dist)/256;
                                leds[i].g = (rgb1.g*inverse_dist)/256;
                                leds[i].b = (rgb1.b*inverse_dist)/256;
                                break;
                            case 2:
                                leds[i].b = inverse_dist;
                                break;
                        }
                    }
                }

                cnt++;
            }
        }

        if (current_effect == 1) {
            static uint32_t cnt = 0;
            cnt++;
            static uint16_t grid[MATRIX_HEIGHT+1][MATRIX_WIDTH];

            //generate light at the bottom of the screen
            for (int x = 0; x < MATRIX_WIDTH; x++) {

                    int amt = _max(inoise8(x*5, x*5000, cnt*4)-128, 0);
                    amt = (amt*amt)/256;
                    amt*=3;
                    grid[0][x] += amt;
            }

            //light moves toward the top
            for (int y = MATRIX_HEIGHT-1; y >= 0; y--) {
                for (int x = 0; x < MATRIX_WIDTH; x++) {
                    int val = grid[y][x];
                    if (val > 0) {
                        int iy = MATRIX_HEIGHT+5 - y;
                        iy = (iy*iy)/MATRIX_HEIGHT;
                        int amount = _max((val * (iy))/(MATRIX_HEIGHT), 1);
                        grid[y][x] = _max(grid[y][x]-amount, 0);
                        grid[y+1][x] += amount;
                    }
                }
            }

            for (int y = 0; y < MATRIX_HEIGHT; y++) {
                for (int x = 0; x < MATRIX_WIDTH; x++) {
                    leds[XY(x,y)].r = (_min(grid[y][x],255)*rgb0.r)/255;
                    leds[XY(x,y)].g = (_min(grid[y][x],255)*rgb0.g)/255;
                    leds[XY(x,y)].b = (_min(grid[y][x],255)*rgb0.b)/255;
                }
            }
        }

        LED_show();
        LED_black();
        
    }

};   


LIGHT_SKETCHES::REGISTER<TREE_SIM> tree_sim("tree sim");

#endif