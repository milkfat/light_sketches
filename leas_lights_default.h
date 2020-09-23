#ifndef LEAS_LIGHTS_DEFAUL_SKETCH_H
#define LEAS_LIGHTS_DEFAUL_SKETCH_H

#include "EEPROM.h"

#define NUM_LEAS_LIGHTS_DEFAULT_EFFECTS 3

void _lld_re_c_handle();
void _lld_re_d_handle();
void _lld_button_handle();

class LEAS_LIGHTS_DEFAULT: public LIGHT_SKETCH {

  public:
    LEAS_LIGHTS_DEFAULT () {setup(); minimum_frame_delay = 4;}
    ~LEAS_LIGHTS_DEFAULT () {disableInterrupts(); minimum_frame_delay = DEFAULT_MINIMUM_FRAME_DELAY;}

    volatile uint8_t re_buff[32];
    volatile uint8_t re_buff_r = 0;
    volatile uint8_t re_buff_w = 0;

  private:
    int current_effect = 0;
    uint32_t start_led = 0;
    uint32_t end_led = NUM_LEDS;

    const byte re_c_pin = 21;
    const byte re_d_pin = 19;

    const byte button_pin = 18;

    uint8_t re_hue = 0;
    uint8_t re_sat = 0;
    uint8_t re_val = 0;
    uint8_t re_val_mem = 20;
    uint8_t re_function = 0;

    uint8_t hue_change_spd = 10;

    uint32_t EEPROM_time = 0;
    uint8_t hue_last = 0;
    uint8_t sat_last = 180;
    uint8_t val_last = 200;
    #define EEPROM_SIZE 4
    uint16_t EEPROM_countdown_to_write = 0;


    enum re_functions {
        RE_BRIGHTNESS,
        RE_HUE,
        RE_SATURATION,
        RE_START_LED,
        RE_END_LED
    };

    

  public:
    volatile uint32_t button_cnt = 0;
    volatile uint8_t button_status = 0;
    volatile uint32_t button_time = 0;
    bool button_new = 0;

    void button_handle() {
        uint8_t b = !digitalRead(button_pin);
        uint32_t t = micros();
        uint32_t e = t - button_time;
        if (b != button_status && e > 1023) {
            button_status = b;
            button_time = t;
        }
    
    }

    void re_c_handle() {
        re_buff[re_buff_w++] = digitalRead(re_c_pin);
        re_buff_w%=32;
    }

    void re_d_handle() {
        re_buff[re_buff_w++] = digitalRead(re_d_pin)+2;
        re_buff_w%=32;
    }

  private:
    void enableInterrupts() {
        attachInterrupt(digitalPinToInterrupt(re_c_pin), _lld_re_c_handle, CHANGE);
        attachInterrupt(digitalPinToInterrupt(re_d_pin), _lld_re_d_handle, CHANGE);
        attachInterrupt(digitalPinToInterrupt(button_pin), _lld_button_handle, CHANGE);
    }

    void disableInterrupts() {
        detachInterrupt(digitalPinToInterrupt(re_c_pin));
        detachInterrupt(digitalPinToInterrupt(re_d_pin));
        detachInterrupt(digitalPinToInterrupt(button_pin));
    }


    struct LED_ACCUM {
        uint16_t r = 0;
        uint16_t g = 0;
        uint16_t b = 0;
    };

    LED_ACCUM led_accum[NUM_LEDS];

  public:
    
    void reset() {
    }

    void setup() {

        control_variables.add(re_hue, "Hue", 0, 255);
        control_variables.add(re_sat, "Saturation", 0, 255);
        control_variables.add(re_val, "Brightness", 0, 255);
        control_variables.add(start_led, "Start LED", 0, NUM_LEDS);
        control_variables.add(end_led, "End LED", 0, NUM_LEDS);
        
        for (int i = 0; i < NUM_LEDS; i++) {
            led_accum[i].r = 0;
            led_accum[i].g = 0;
            led_accum[i].b = 0;  
        }
        
        EEPROM.begin(EEPROM_SIZE);
        re_hue = EEPROM.read(0);
        re_sat = EEPROM.read(1);
        re_val = EEPROM.read(2);
        re_val_mem = EEPROM.read(3);

        hue_last = re_hue;
        sat_last = re_sat;
        val_last = re_val;
        
        pinMode(re_c_pin, INPUT_PULLUP);
        pinMode(re_d_pin, INPUT_PULLUP);
        pinMode(button_pin, INPUT_PULLUP);

        enableInterrupts();
  
    }

    void next_effect() {
        current_effect++;
        current_effect%=NUM_LEAS_LIGHTS_DEFAULT_EFFECTS;
    }


    void loop() {
        LED_show();
        handle_buttons();

        static uint8_t hue_change_cnt = 0;
        if (current_effect) {
          if (hue_change_cnt >= hue_change_spd) {
            re_hue++;
            hue_change_cnt = 0;
          }
          hue_change_cnt++;
        }

        handle_lights();
        handle_rf_codes();
    }

    void handle_rf_codes() {
        static uint32_t power_toggle_time = 0;
        static uint32_t next_effect_time = 0;
        while ( rf_code_available() ) {
          //Serial.println("GOT CODE");
            int mod = 5;
            RF_CODE * rf = rf_code_get();

            switch (rf->code) {
                case 9560813:
                  re_hue = 160;
                  re_sat = 255;
                  //Serial.println("B");
                  break;
                case 9560814:
                  re_hue = 96;
                  re_sat = 255;
                  //Serial.println("G");
                  break;
                case 9560815:
                  re_hue = 0;
                  re_sat = 255;
                  //Serial.println("R");
                  break;
                case 9560816:
                  re_hue-=5;
                  //Serial.println("DEL_DIY");
                  break;
                case 9560817:
                  re_hue = 235;
                  re_sat = 64;
                  //Serial.println("W");
                  break;
                case 9560818:
                  brightness_down(mod);
                  //Serial.println("DIY-");
                  break;
                case 9560819:
                  re_hue+=5;
                  //Serial.println("ADD_DIY");
                  break;
                case 9560820:
                  saturation_down();
                  //Serial.println("MODE-");
                  break;
                case 9560821:
                  brightness_up(mod);
                  //Serial.println("DIY+");
                  break;
                case 9560822:
                  hue_change_spd++;
                  hue_change_spd = _min(hue_change_spd, 254);
                  //Serial.println("SPEED+");
                  break;
                case 9560823:
                  if (millis() > next_effect_time) {
                    next_effect_time = millis()+500;
                    next_effect();
                  }
                  //Serial.println("MENU_AUTO");
                  break;
                case 9560824:
                  hue_change_spd--;
                  hue_change_spd = _max(hue_change_spd, 1);
                  //Serial.println("SPEED-");
                  break;
                case 9560826:
                  saturation_up();
                  //Serial.println("MODE+");
                  break;
                case 9560830:
                  if (millis() > power_toggle_time) {
                    power_toggle_time = millis()+500;
                    power_toggle();
                  }
                  //Serial.println("POWER");
                  break;
                default:
                  break;
            }

        }

    }

    void handle_lights() {
      
        if ( re_hue != hue_last || re_sat != sat_last || re_val != val_last ) {
            hue_last = re_hue;
            sat_last = re_sat;
            val_last = re_val;
            EEPROM_countdown_to_write = 1000;
        }

        if (EEPROM_countdown_to_write > 0) {
            EEPROM_countdown_to_write--;
            if (EEPROM_countdown_to_write == 0) {
              EEPROM.write(0, re_hue);
              EEPROM.write(1, re_sat);
              EEPROM.write(2, re_val);
              EEPROM.write(3, re_val_mem);
              EEPROM.commit();
            }
        }

        LED_black();
        
        for (int i = start_led; i < end_led; i++) {
          
            CRGB rgb = CHSV(re_hue, re_sat, 255);

            if (current_effect == 2) {
              rgb = CHSV(re_hue+i*2, re_sat, 255);
            }

            uint16_t bri16 = gamma16_decode(re_val);
        
            uint16_t bri16_r = (rgb.r*bri16)/255;
            uint16_t bri16_g = (rgb.g*bri16)/255;
            uint16_t bri16_b = (rgb.b*bri16)/255;
            
            uint8_t bri_r = bri16_r/256;
            uint8_t bri_g = bri16_g/256;
            uint8_t bri_b = bri16_b/256;
            
            led_accum[i].r += bri16_r & 0b11000000;
            if (led_accum[i].r > 255) {
            led_accum[i].r -= 256;
            if (bri_r < 255) {
                bri_r++;
            }
            }
        
            led_accum[i].g += bri16_g & 0b11000000;
            if (led_accum[i].g > 255) {
            led_accum[i].g -= 256;
            if (bri_g < 255) {
                bri_g++;
            }
            }
        
            led_accum[i].b += bri16_b & 0b11000000;
            if (led_accum[i].b > 255) {
            led_accum[i].b -= 256;
            if (bri_b < 255) {
                bri_b++;
            }
            }
        
            leds[i].r = bri_r;
            leds[i].g = bri_g;
            leds[i].b = bri_b;
        }
    
    }

    void handle_buttons() {
        //handle_log();
        uint8_t button_down_new = 0;
        static uint32_t button_down_time = 0;
        static uint32_t button_down_duration = 0;
        static uint32_t button_down_duration_old = 0;
        
        uint8_t button_up_new = 0;
        static uint32_t button_up_time = 0;
        static uint32_t button_up_duration = 0;
        static uint32_t button_up_duration_old = 0;
        
        static uint8_t old_button_status = 0;
        uint8_t current_button_status = button_status; //take a snapshot to avoid race condition with interrupt
        
        if (current_button_status)
        {
        if (!old_button_status) 
        {
            button_down_new = 1;
            button_down_time = millis();
            button_down_duration = 0;
            //Serial.println("NEW!");
        } 
        else
        {
            button_down_duration = millis() - button_down_time;
        }
        }
        else if (!current_button_status)
        {
        if (old_button_status)
        {
            button_up_new = 1;
            button_up_time = millis();
            button_up_duration = 0;
        }
        else
        {
            button_up_duration = millis() - button_up_time;
        }
        }
        

        static uint8_t button_short_count = 0;
        static uint8_t button_long_count = 0;
        uint8_t button_short_clicks = 0;
        uint8_t button_long_clicks = 0;

        if (button_up_new)
        {
        if (button_down_duration < 500) 
        {
            button_short_count++;
        }
        else
        {
            button_long_count++;
        }
        }

        if (button_up_duration_old < 500 && button_up_duration >= 500) {
          button_short_clicks = button_short_count;
          button_long_clicks = button_long_count;
          button_short_count = 0;
          button_long_count = 0;
        }

        
        old_button_status = current_button_status;
        button_down_duration_old = button_down_duration;
        button_up_duration_old = button_up_duration;

        
        handle_input();

        if (button_short_clicks == 1 && button_long_clicks == 0) {
            re_function = RE_BRIGHTNESS;
            power_toggle();
        }
        
        if (button_short_clicks == 2 && button_long_clicks == 0) {
          re_function = RE_SATURATION;
        }
        
        if (button_short_clicks == 3 && button_long_clicks == 0) {
          re_function = RE_HUE;
        }
        
        if (button_short_clicks == 4 && button_long_clicks == 0) {
          re_function = RE_START_LED;
        }
        
        if (button_short_clicks == 5 && button_long_clicks == 0) {
          re_function = RE_END_LED;
        }
        
    }

    void power_toggle() {
        if (re_val > 0) {
            re_val_mem = re_val;
            re_val = 0;
            //Serial.println("OFF!");
        } else {
            re_val=re_val_mem;
            //Serial.println("ON!");
        }
    }

    void brightness_up(int mod = 5) {
      if (re_val >= 15) {
        mod = 5;
      }
      if (re_val+mod > 255) {
        re_val=255;
      } else {
        re_val+=mod;
      }
    }
    
    void saturation_up() {
        if ((int)re_sat+5 > 255) {
            re_sat=255;
        } else {
            re_sat+=5;
        }
    }

    void re_up() {
      int mod = 5;
      switch (re_function) {
        case RE_BRIGHTNESS:
          brightness_up(mod);
          break;
        case RE_HUE:
          if (re_hue >= 15) {
            mod = 5;
          }
          if (re_hue+mod > 255) {
            re_hue=255;
          } else {
            re_hue+=mod;
          }
          break;
        case RE_SATURATION:
          if (re_sat >= 15) {
            mod = 5;
          }
          if (re_sat+mod > 255) {
            re_sat=255;
          } else {
            re_sat+=mod;
          }
          break;
        case RE_START_LED:
          if (start_led < end_led) start_led++;
          break;
        case RE_END_LED:
          if (end_led < NUM_LEDS) end_led++;
          break;
      }
    }

    void brightness_down(int mod = 5) {
        if (re_val > 15) {
            mod = 5;
        }
        if ((int)re_val-mod < 0) {
            re_val=0;
        } else {
            re_val-=mod;
        }
    }

    void saturation_down() {
        if ((int)re_sat-5 < 0) {
            re_sat=0;
        } else {
            re_sat-=5;
        }
    }

    void re_down() {
      int mod = 5;
      switch (re_function) {
        case RE_BRIGHTNESS:
          brightness_down(mod);
          break;
        case RE_HUE:
          if (re_hue > 15) {
            mod = 5;
          }
          if ((int)re_hue-mod < 0) {
            re_hue=0;
          } else {
            re_hue-=mod;
          }
          break;
        case RE_SATURATION:
          if (re_sat > 15) {
            mod = 5;
          }
          if ((int)re_sat-mod < 0) {
            re_sat=0;
          } else {
            re_sat-=mod;
          }
          break;
        case RE_START_LED:
          if (start_led > 0) start_led--;
          break;
        case RE_END_LED:
          if (end_led > start_led) end_led--;
          break;
      }
    }


    void handle_input() {
      
        static uint8_t c = 0;
        static uint8_t d = 0; 
        static uint8_t max_cnt = 0;
        uint8_t cnt = 0;
        while (re_buff_r != re_buff_w) {
          cnt++;
          if (cnt > max_cnt) {
            max_cnt = cnt;
          }
          uint8_t val = re_buff[re_buff_r++];
          re_buff_r%=32;
          switch (val) {
            case 0: //c == 0
              c = 0;
              break;
            case 1: //c == 1
              c = 1;
              if (d == 1) {
                re_down();
                c = 0;
                d = 0;
              }
              break;
            case 2: //d == 0
              d = 0;
              break;
            case 3: //d == 1
              d = 1;
              if (c == 1) {
                re_up();
                c = 0;
                d = 0;
              }
              break;
          }
        }
    }



};

LIGHT_SKETCHES::REGISTER<LEAS_LIGHTS_DEFAULT> leas_lights_default("Lea's Lights (Standard)");


void _lld_re_c_handle() {
  leas_lights_default.sketch->re_c_handle();
};
void _lld_re_d_handle() {
  leas_lights_default.sketch->re_d_handle();
};
void _lld_button_handle() {
  leas_lights_default.sketch->button_handle();
};
#endif