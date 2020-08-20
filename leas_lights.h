#ifndef LEAS_LIGHTS_SKETCH_H
#define LEAS_LIGHTS_SKETCH_H

//RAYTRACE
#define NUM_LEAS_LIGHTS_EFFECTS 2
#define NUM_LEAS_LEDS 127

#define MAX_NUM_LEAS_CHASING_LIGHTS 31
#define NUM_LEAS_WALLS 6

class LEAS_LIGHTS: public LIGHT_SKETCH {

  public:
    LEAS_LIGHTS () {setup();}
    ~LEAS_LIGHTS () {}

  private:
    int current_effect = 0;
    int chasing_lights[MAX_NUM_LEAS_CHASING_LIGHTS];
    int chase_speed = 8;
    int hue = 0;
    uint8_t hue_old = 0;
    bool blending_method = 0;
    uint32_t walls[NUM_LEAS_WALLS][2];
    CRGB wall_colors[NUM_LEAS_WALLS];
    uint8_t fade_rate = 245;
    uint8_t hue_step = 48;
    uint8_t hue_step_old = 48;
    uint8_t hue_change_speed = 0;
    uint8_t num_chasing_lights = 7;
    uint8_t num_chasing_lights_old = 7;
    bool invert_gradient = 0;
    uint8_t sparkle = 0;

    void check_hue_change() {
        for (int i = 0; i < NUM_LEAS_WALLS; i++) {
            wall_colors[i] = CHSV(i*hue_step+hue, 255, 255);
        }
    }


  public:
    void setup_chasing_lights() {
      for (int i = 0; i < num_chasing_lights; i++) {
          chasing_lights[i] = (i*256*NUM_LEAS_LEDS)/num_chasing_lights;
      }
      LED_black();
    }

    void reset() {
    }

    void setup() {
        
        walls[0][0] = 0;
        walls[0][1] = 34;

        walls[1][0] = 34;
        walls[1][1] = 35+25;

        walls[2][0] = 35+25;
        walls[2][1] = 35+25+7;

        walls[3][0] = 35+25+7;
        walls[3][1] = 35+24+7+4;

        walls[4][0] = 35+24+7+4;
        walls[4][1] = 35+24+7+4+27;

        walls[5][0] = 35+24+7+4+27+1;
        walls[5][1] = 35+24+7+4+27+29;
        
        reset();
        control_variables.add(chase_speed, "Chase Speed", -256, 256);
        control_variables.add(hue, "Hue", 0, 255);
        control_variables.add(fade_rate, "Fade Rate", 0, 255);
        control_variables.add(hue_step, "Hue Step Size", 0, 255);
        control_variables.add(hue_change_speed, "Auto Hue Change", 0, 255);
        control_variables.add(num_chasing_lights, "Number of Chasing Lights", 0, MAX_NUM_LEAS_CHASING_LIGHTS);
        control_variables.add(blending_method, "Blending Method");
        control_variables.add(invert_gradient, "Invert Gradient");
        control_variables.add(sparkle, "Sparkle", 0, 255);

        setup_chasing_lights();

        check_hue_change();

    }

    void next_effect() {
        current_effect++;
        current_effect%=NUM_LEAS_LIGHTS_EFFECTS;
    }


    void loop() {
        hue += hue_change_speed;
        
        if (num_chasing_lights != num_chasing_lights_old) {
          setup_chasing_lights();
          num_chasing_lights_old = num_chasing_lights;
        }
        if (hue_step != hue_step_old || hue != hue_old) {
          check_hue_change();
          hue_old = hue;
          hue_step_old = hue_step;
        }

        if (sparkle) {
          for (int i = 0; i < 127; i++) {
            if (random(256) < sparkle) {
              leds[i] = CRGB::White;
            }
          }
        }

        LED_show();

        switch (current_effect)
        {
            case 0:
                chasing_effect();
                break;
            case 1:
                gradient_effect();
                break;
            default:
                break;
        }
    }


    void gradient_effect() {
      LED_black();
      static int32_t cnt = 0;
      cnt+=8;
      for (int w = 0; w < NUM_LEAS_WALLS; w++) {
        
        int dist = walls[w][1] - walls[w][0];

        CRGB color0 = wall_colors[w];
        CRGB color1 = wall_colors[(w+1)%NUM_LEAS_WALLS];
        

        for ( int i = 0; i <= dist/2; i++) {

        uint8_t bri0 = inoise8(cnt-(i*16*15)/(dist/2), w*5000, 0);
        uint8_t bri1 = inoise8(cnt-(i*16*15)/(dist/2), (w+1)%NUM_LEAS_WALLS*5000, 0);

        bri0 = _min(_max((((int)bri0)-64)*2,0),255);
        bri1 = _min(_max((((int)bri1)-64)*2,0),255);

        // bri0 = 255-bri0;
        // bri1 = 255-bri1;

          if (invert_gradient) {
            // leds[walls[w][0] + i].r = (255*i*2)/dist;
            // leds[walls[w][1] - i].r = (255*i*2)/dist;
            leds[walls[w][0] + i].r = (color0.r*i*2)/dist;
            leds[walls[w][0] + i].g = (color0.g*i*2)/dist;
            leds[walls[w][0] + i].b = (color0.b*i*2)/dist;
            leds[walls[w][1] - i].r = (color1.r*i*2)/dist;
            leds[walls[w][1] - i].g = (color1.g*i*2)/dist;
            leds[walls[w][1] - i].b = (color1.b*i*2)/dist;
          } else {
            // leds[walls[w][0] + i].r = 255 - (255*i*2)/dist;
            // leds[walls[w][1] - i].r = 255 - (255*i*2)/dist;
            leds[walls[w][0] + i].r = (bri0*(color0.r - (color0.r*i*2)/dist))/255;
            leds[walls[w][0] + i].g = (bri0*(color0.g - (color0.g*i*2)/dist))/255;
            leds[walls[w][0] + i].b = (bri0*(color0.b - (color0.b*i*2)/dist))/255;
            leds[walls[w][1] - i].r = (bri1*(color1.r - (color1.r*i*2)/dist))/255;
            leds[walls[w][1] - i].g = (bri1*(color1.g - (color1.g*i*2)/dist))/255;
            leds[walls[w][1] - i].b = (bri1*(color1.b - (color1.b*i*2)/dist))/255;
          }
        }

      }
    }


    void chasing_effect() {

        for (int i = 0; i < NUM_LEDS; i++) {
          leds[i].r = ((uint16_t)leds[i].r*fade_rate)/255;
          leds[i].g = ((uint16_t)leds[i].g*fade_rate)/255;
          leds[i].b = ((uint16_t)leds[i].b*fade_rate)/255;
        }

        for (int i = 0; i < num_chasing_lights; i++) {
          int led0 = chasing_lights[i]/256;
          int led1 = chasing_lights[i]/256+1;
          led1 %= NUM_LEAS_LEDS;
          uint8_t a = chasing_lights[i] % 256;
          uint16_t level1;
          uint16_t level0;
          if ( a > 127) {
            level0 = gamma16_decode(((256-a)*187)/128);
            level1 = 65535 - level0;
          } else {
            level1 = gamma16_decode((a*187)/128);
            level0 = 65535 - level1;
          }

          // uint16_t level1 = a*256;
          // uint16_t level0 = 65535 - level1;
          if (blending_method) {
            CRGB rgb0 = CHSV(hue+i*hue_step,255,255);
            CRGB rgb1 = CHSV(hue+i*hue_step,255,255);
            rgb0.r = (rgb0.r * level0)/65535;
            rgb0.g = (rgb0.g * level0)/65535;
            rgb0.b = (rgb0.b * level0)/65535;
            rgb1.r = (rgb1.r * level1)/65535;
            rgb1.g = (rgb1.g * level1)/65535;
            rgb1.b = (rgb1.b * level1)/65535;
            CRGB _rgb0 = gamma8_encode(rgb0);
            CRGB _rgb1 = gamma8_encode(rgb1);
            leds[led0].r = _max(leds[led0].r, _rgb0.r);
            leds[led0].g = _max(leds[led0].g, _rgb0.g);
            leds[led0].b = _max(leds[led0].b, _rgb0.b);
            leds[led1].r = _max(leds[led1].r, _rgb1.r);
            leds[led1].g = _max(leds[led1].g, _rgb1.g);
            leds[led1].b = _max(leds[led1].b, _rgb1.b);
          } else {
            uint16_t l0 = gamma16_encode(level0);
            uint16_t l1 = gamma16_encode(level1);
            leds[led0].r = _max(leds[led0].r, l0);
            leds[led0].g = _max(leds[led0].g, l0);
            leds[led0].b = _max(leds[led0].b, l0);
            leds[led1].r = _max(leds[led1].r, l1);
            leds[led1].g = _max(leds[led1].g, l1);
            leds[led1].b = _max(leds[led1].b, l1);
          }

          chasing_lights[i] += chase_speed;
          chasing_lights[i] += NUM_LEAS_LEDS*256;
          chasing_lights[i] %= NUM_LEAS_LEDS*256;
        }
    }



};

LIGHT_SKETCHES::REGISTER<LEAS_LIGHTS> leas_lights("Lea's Lights (Fancy)");

#endif