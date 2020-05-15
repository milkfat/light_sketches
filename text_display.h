#ifndef LIGHTS_TEXT_DISPLAY_NOISE_H
#define LIGHTS_TEXT_DISPLAY_NOISE_H

//PARTICLES

class TEXT_DISPLAY: public LIGHT_SKETCH {
  public:
    TEXT_DISPLAY () {setup();}
    ~TEXT_DISPLAY () {}
  private:
    #define NUM_TEXT_DISPLAY_EFFECTS 3
    int current_effect = 0;

    #define NUM_FONTS 7
    uint8_t current_font = 1;
    uint16_t font_scale = 256;
    char text[500] = "TOPGUN\n6:45pm";
    uint8_t threshold = 0;
    uint8_t sharpen_amount = 0;
    

  public:
    void reset() {
        current_font = 1;
    }

    void setup() {
        control_variables.add(sharpen_amount, "Sharpen", 0, 9);
        control_variables.add(threshold, "Threshold", 1, 255);
        control_variables.add(current_font, "Font", 1, NUM_FONTS);
        control_variables.add(font_scale, "Scale", 1, 256);
        control_variables.add(text, "Text");
        
    }

    void next_effect() {
        current_effect++;
        current_effect %= NUM_TEXT_DISPLAY_EFFECTS;
    }

    void loop() {
        LED_show();

        switch (current_effect) {
            case 0:
                handle_text();
                break;
            case 1:
                break;
            case 2:
                break;
            default:
                break;

        }
    }

    void handle_text() {
        LED_black();
        draw_characters(current_font, text, 0*256, (MATRIX_HEIGHT-1)*256, CRGB(255,0,0), font_scale, false, sharpen_amount);
        for (int i = 0; i < NUM_LEDS; i++) {
            if (leds[i].r < threshold) {
                leds[i].r = 0;
            }
        }
    }

};

LIGHT_SKETCHES::REGISTER<TEXT_DISPLAY> text_display("text_display");

#endif