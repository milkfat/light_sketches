#ifndef LIGHTS_TEXT_DISPLAY_NOISE_H
#define LIGHTS_TEXT_DISPLAY_NOISE_H
#include "text.h"
//PARTICLES

class TEXT_DISPLAY: public LIGHT_SKETCH {
  public:
    TEXT_DISPLAY () {setup();}
    ~TEXT_DISPLAY () {}
  private:
    #define NUM_TEXT_DISPLAY_EFFECTS 3
    int current_effect = 0;

    #define NUM_FONTS 7
    uint8_t current_font = 3;
    uint16_t font_scale = 387;
    //char text[500] = "TOPGUN\n6:45pm";
    char text[500] = "\n\n\n\n\n\n\n\n\nHAPPY BIRTHDAY SARAH!!!";
    uint8_t threshold = 0;
    uint8_t sharpen_amount = 0;
    int32_t x_start = 0;
    uint8_t scroll_speed = 185;
    uint8_t color_speed = 252;
    CRGB rgb = CRGB(255,255,255);
    bool enable_text_sparkle = 1;
    bool enable_background_sparkle = 1;
    

  public:
    void reset() {
        current_font = 1;
    }

    void setup() {
        control_variables.add(sharpen_amount, "Sharpen", 0, 9);
        control_variables.add(threshold, "Threshold", 1, 255);
        control_variables.add(current_font, "Font", 1, NUM_FONTS);
        control_variables.add(font_scale, "Scale", 1, 1024);
        control_variables.add(scroll_speed, "Scroll Speed", 0, 255);
        control_variables.add(color_speed, "Color Speed", 0, 255);
        control_variables.add(rgb, "Text Color");
        control_variables.add(enable_text_sparkle, "Text Sparkle!");
        control_variables.add(enable_background_sparkle, "Background Sparkle!");
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
        int x_end;
        x_end = draw_characters(current_font, text, x_start, (MATRIX_HEIGHT-1)*256, rgb, font_scale, false, sharpen_amount);
        
        if (x_start + x_end < 0) {
            x_start = MATRIX_WIDTH*256;
        } else {
            x_start -= scroll_speed;
        }
        static uint8_t color_rotate;
        if (color_speed) {
            color_rotate+=color_speed;
        }
        for (int x = 0; x < MATRIX_WIDTH; x++) {
            for (int y = 0; y < MATRIX_HEIGHT; y++) {
                int i = XY(x,y);
                uint8_t b = (leds[i].r+leds[i].g+leds[i].b)/3;
                if (color_speed) {
                    leds[i] = CHSV(x + color_rotate, 255, 255);
                    leds[i].r = (leds[i].r*b)/255;
                    leds[i].g = (leds[i].g*b)/255;
                    leds[i].b = (leds[i].b*b)/255;
                }
                if (enable_text_sparkle && b && !random(100)) {
                    leds[i] = CRGB::White;
                }
                if (leds[i].r < threshold) {
                    leds[i].r = 0;
                }

                if (enable_background_sparkle && !random(NUM_LEDS/3)) {
                    leds[i] = CRGB::White;
                }
            }
        }
    }

};

LIGHT_SKETCHES::REGISTER<TEXT_DISPLAY> text_display("text_display");

#endif