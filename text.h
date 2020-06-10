#ifndef LIGHTS_TEXT_H
#define LIGHTS_TEXT_H

  //TEXT STUFF

#include "font.h"


bool text_filter = 0;
uint8_t text_effect = 0;
bool text_rotate = 0;
#define NUM_TEXT_EFFECTS 5
uint32_t text_shake_time = 0;
uint8_t current_font = 0;
int ystp = 0;
int xstp = 0;

#if MATRIX_WIDTH >= MATRIX_HEIGHT
int horizontal_text=1;
#else
int horizontal_text=0;
#endif

int text_color = -1;
uint8_t text_saturation = 255;
uint8_t text_brightness = 96;
uint8_t font_scaler = 48;
bool scrolling_text_enabled = 0;
int drawing_enabled = 0;

uint8_t font_sizes[8] = {0,15,16,17,7,7,5,7};
uint8_t font_y_adjust[8] = {0,3,2,2,1,1,1,0};

void add_character(char c) {
    display_text.push_back(c);
}
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
// static inline uint8_t bitRead(const uint8_t& this_byte, const uint8_t& this_bit) {
//     return (this_byte & (1 << (this_bit))) ? 1 : 0;
// }

int32_t cube_ang = 0;
int32_t cube_ang2 = 0;
int32_t cube_ang3 = 0;

void handle_text() {
  //if the screen has been updated then redraw the text
  if (z_buffer != nullptr && update_since_text == 1) {

    z_buffer->reset();
    static uint32_t text_time = 0;
    uint32_t time_elapsed = millis() - text_time;
    text_time = millis();
    cube_ang+=time_elapsed*(512/16);
    cube_ang2 = cube_ang;
    cube_ang3 = cube_ang;

    

    if (drawing_enabled && temp_canvas != nullptr) {


      //add our temp canvas to LEDS
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] += temp_canvas[i];
        temp_canvas[i] = CRGB::Black;
      }

      //debug, draw line
      //leds[XY(round(cursor_position_x/256.0), round(cursor_position_y/256.0))].b = 255;
      //long derpx = -2*256; //(4*256L)+201;
      //long derpy = -2*256; //50*256L+45;
      //long derpy = 50*256L;
      //draw_line_fine(led_screen, derpx, derpy,cursor_position_x, cursor_position_y);
      for (int i = 0; i < NUM_POINTERS; i++) {
        if (pointers[i].down) {
          drawXY(led_screen,pointers[i].x, pointers[i].y, 96, 255, 255);
        }
      }
      //draw_line(leds, derpx/255L, derpy/255L+20,cursor_position_x/255L, cursor_position_y/255L+20);
    }

    update_since_text = 0;



    
    static uint8_t angle = 0;
    angle++;
    int angle_sin = sin8(angle)-127;
    int angle_cos = cos8(angle)-127;


    
    //SHAKING TEXT
    
    uint8_t scroll_reset = 1;
    static int8_t offsets[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    static uint32_t text_animation[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    static uint32_t text_animation_speed[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    static uint8_t shaking = 0;
    static uint8_t shake_step = 0;
    
    if (millis() - 600 < text_shake_time) {
      shake_step += 1;
      shake_step %= 2;
      shaking = 1;
      uint8_t max_offset = 2;
      int time_since_start = millis() - text_shake_time;
      if ( (shake_step%2)==0 ) { //shake every other frame
        for (int i = 0;i < 16; i++) {
           if (time_since_start > 100+( (i/2)*50 ) ) {
            max_offset = 1;
            if (time_since_start > 300+( (i/2)*50 ) ) {
              max_offset = 0;
            }
          }
          //move in a random direction
          if (random(2) == 0) {
            offsets[i] += 1;
          } else {
            offsets[i] -= 1;
          }
          //if we move out-of-bounds, then back up
          if (offsets[i] > max_offset) {
            offsets[i] -=1 ;
          } else if (offsets[i] < -max_offset) {
            offsets[i] += 1;
          }
        }
      }
    } else if (shaking == 1) {
      shaking = 0;
      for (int i = 0;i<16;i++) {
        offsets[i]=0;
      }
    }


  
    //RENDER THE TEXT
    int current_height = 0; //keep track of the vertical position of our current character
    int current_width = 0; //keep track of the horizontal position of our current character

    //RESET THE MASK
    if (text_mask == 1) {
      for (int i = 0; i < NUM_LEDS; i++) {
        led_mask[i] = 255;
      }
    }

    
    for (int i = _max( display_text.length(), old_display_text.length() ); i < 16; i++) {
        text_animation[i] = 0;
        text_animation_speed[i] = 1500;
    }
    for (int i = 0; i < _max( display_text.length(), old_display_text.length() ); i++) {

      //keep track of this character's lookup position for the shaking offset
      uint8_t offset_pos = i%8*2;

      //decide the hue of this character
      uint8_t hue = (i*32)%255;
      uint8_t sat = 255;
      bool persist = false;
      if (text_color >= 0) {
        hue = text_color;
      }
  
      //ADAFRUIT FONTS
      if (current_font >= 1) {
        uint8_t letter = display_text[i]; //letter position, minus 20 for non-used characters
        letter -= 32;
        uint16_t letter_location = font_glyphs[current_font-1][letter][0]; ///< Pointer into GFXfont->bitmap
        uint8_t letter_width = font_glyphs[current_font-1][letter][1];    ///< Bitmap dimensions in pixels
        uint8_t letter_height = font_glyphs[current_font-1][letter][2];   ///< Bitmap dimensions in pixels
        //uint8_t letter_xAdvance = font_glyphs[current_font-1][letter][3]; ///< Distance to advance cursor (x axis)
        //int8_t letter_xOffset = font_glyphs[current_font-1][letter][4];  ///< X dist from cursor pos to UL corner
        //int8_t letter_yOffset = font_glyphs[current_font-1][letter][5];  ///< Y dist from cursor pos to UL corner
        //int8_t center_offset = (MATRIX_WIDTH-letter_width)/2;
        //float scaler = MATRIX_WIDTH/letter_width;
        for (int y = 0; y < letter_height; y++) {
          //int ypos = 0-round(y*font_scaler)+ystp-current_height;
          int ypos = 0-(y*(font_scaler+16))/64;
          if (!horizontal_text && (ypos+ystp-current_height < -10 || ypos+ystp-current_height > MATRIX_HEIGHT+letter_height)) {
            break; //stop drawing if we go off the bottom of the screen
          }
          scroll_reset = 0;
          if (ypos < MATRIX_HEIGHT) { //draw line if it is on the screen
            for (int x = 0;x < letter_width; x++) {
              uint8_t pixel_bit = bitRead(font_bitmaps[current_font-1][letter_location+(y*letter_width+x)/8],7-(y*letter_width+x)%8);
              if (pixel_bit) {
                long u;
                long v;
                if (text_rotate == 1) {
                  u = ( (angle_cos) * (x*256L-(256*letter_width)/2) )/127L - ( (angle_sin) * (ypos*256L+(256*letter_height)/2) )/127L + (256*letter_width)/2;
                  v = ( (angle_sin) * (x*256L-(256*letter_width)/2) )/127L + ( (angle_cos) * (ypos*256L+(256*letter_height)/2) )/127L - (256*letter_height)/2;
                } else {
                  u = x*256L;
                  v = ypos*256L;
                }
                u += xstp*256L+current_width*256L*horizontal_text;
                v += ystp*256L-current_height*256L*!horizontal_text;
                if (text_mask == 0) {
                  if (text_filter == 1) {
                    blendXY(led_screen, u+offsets[offset_pos]*256, v+offsets[offset_pos+1]*256, hue, text_saturation, text_brightness );
                  } else {
                    drawXY_fine(led_screen, u+offsets[offset_pos]*256, v+offsets[offset_pos+1]*256, hue, text_saturation, 96  );
                  }
                } else {
                  int led_num = XY(x+offsets[offset_pos], ypos+ystp-current_height+offsets[offset_pos+1]);
                  led_mask[led_num] = 0;
                }
                
                //blendXY(led_screen,(x+offsets[pos])*(36+255), (ypos+offsets[pos+1])*(36+255), hue, 255, bitRead(font8x8_basic[letter][y],x)*96  );
                
              }
            }
        }
        }
        //advance the cursor for the next letter
        current_height += (letter_height*(font_scaler+16))/64+1;
        current_width += (letter_width*(font_scaler+16))/64+1;
      }
  
  
      //SOME FONT I FOUND
      
      if (current_font == 0) {
        uint8_t letter = display_text[i];
        uint8_t old_letter = old_display_text[i];
        if (text_animation[i] < 200000) {
            text_animation[i]+=text_animation_speed[i];
            text_animation_speed[i]+=75;
        }
        for (int y = 0; y < 8; y++) {
          int32_t ypos = 0-y;
          if (ypos+ystp-8*i < -10 || ypos+ystp-8*i > MATRIX_HEIGHT+8) {
            break; //stop drawing if we go off the bottom of the screen
          }
          scroll_reset = 0;
          for (int x = 0; x < 8; x++) {
            uint8_t pixel_bits[2][2] = {{0,0},{1,0}};
            if (display_text.length() > i) {
              pixel_bits[0][1] = bitRead(font8x8_basic[letter][y],x);
            }
            uint8_t amount = 1;
            if (display_text[i] != old_display_text[i] && old_display_text.length() > i) {
              pixel_bits[1][1] = bitRead(font8x8_basic[old_letter][y],x);
              amount++;
              persist=true;
              //text_animation[i] = 0;
              //text_animation_speed[i] = 6000;
            }
            //int u = cos(-angle) * x * (1.0 / scale) + sin(-angle) * y * (1.0 / scale);
            //int v = -sin(-angle) * x * (1.0 / scale) + cos(-angle) * y * (1.0 / scale);
            for (int j = 0; j < amount; j++) {
              uint8_t is_old = pixel_bits[j][0];
              uint8_t pixel_bit = pixel_bits[j][1];
              if (pixel_bit) {
                int32_t u;
                int32_t v;
                if (text_rotate == 1) {
                  u = ( (angle_cos) * (x*256L-(256*7)/2) )/127L - ( (angle_sin) * (ypos*256L+(256*7)/2) )/127L + (256*7)/2;
                  v = ( (angle_sin) * (x*256L-(256*7)/2) )/127L + ( (angle_cos) * (ypos*256L+(256*7)/2) )/127L - (256*7)/2;
                } else {
                  u = x*256L;
                  v = ypos*256L;
                }
                v += (ystp-8*i)*256L;
                if (text_filter == 1) {
                  

                  int32_t u2 = u+offsets[offset_pos]*256;
                  int32_t v2 = v+offsets[offset_pos+1]*256;

                  int32_t u_diff = 0;
                  int32_t v_diff = 0;
                  if (text_effect == 0) {
                    //do nothing
                    
                  } if (text_effect == 1) {
                    //mask (text appears beneath the mask)
                    
                  } else if (text_effect == 2 && height_map_ptr != nullptr) {
                    
                    //DISPLACEMENT
                    
                    //horizontal pixel difference
                    u_diff = (*height_map_ptr)[u2/256+2][v2/256+1] - (*height_map_ptr)[u2/256][v2/256+1]; //between -128*32 and 127*32
                    u_diff/=8;
                    //vertical pixel difference
                    v_diff = (*height_map_ptr)[u2/256+1][v2/256+2] - (*height_map_ptr)[u2/256+1][v2/256]; //between -128*32 and 127*32
                    v_diff/=8;
                    
                  } else if (text_effect == 3 && height_map_ptr != nullptr) {
    
                    //LIGHT BENDING?
                    u_diff = (*height_map_ptr)[u2/256+2][v2/256+1] - (*height_map_ptr)[u2/256][v2/256+1]; //between -128*32 and 127*32
                    if (u_diff > 0) {
                      //u_diff = 127*32 - abs(u_diff);
                      //u_diff = sq(u_diff)/(128L*32);
                    } else if (u_diff < 0) {
                      //u_diff = 127*32 - abs(u_diff);
                      //u_diff = -sq(u_diff)/(128L*32);
                    }
                    u_diff /= 24;
                    //vertical pixel difference
                    v_diff = (*height_map_ptr)[u2/256+1][v2/256+2] - (*height_map_ptr)[u2/256+1][v2/256]; //between -128*32 and 127*32
                    if (v_diff > 0) {
                      //v_diff = 127*32 - abs(v_diff);
                      //v_diff = sq(v_diff)/(128L*32);
                    } else if (v_diff < 0) {
                      //v_diff = 127*32 - abs(v_diff);
                      //v_diff = -sq(v_diff)/(128L*32);
                    }
                    v_diff /= 24;
                    
                  }
                  

                  
                  if (!text_mask) {
                    //filtered text is added to the image
        
                    if (text_effect) {

                      if (text_effect != 4 && led_mask2 != nullptr) {
                        
                        //normal text drawn outside of mask area
                        blendXY(led_screen, u2, v2, hue, text_saturation, (text_brightness*(255-led_mask2[XY(u2/256,v2/256)]))/256L );
    
                        if (text_effect > 1) {
                        
                          //refractory effects drawn inside the mask area
                          blendXY(led_screen, u2+u_diff, v2+v_diff, hue, text_saturation, (text_brightness*(led_mask2[XY(u2/256,v2/256)]))/256L );
                        }

                      } else {
                        //text is drawn to temporary canvas to be used for later effects
                        CRGB * old_screen_buffer = led_screen.screen_buffer;
                        if (temp_canvas != nullptr) {
                          led_screen.screen_buffer = temp_canvas;
                        }
                        blendXY(led_screen, u2+u_diff, v2+u_diff, hue, text_saturation, text_brightness  );
                        led_screen.screen_buffer = old_screen_buffer;
                      }
                      
                    } else {
                      //normal text, no effects, drawn to the image
                      blendXY(led_screen, u2, v2, hue, text_saturation, text_brightness );
                      
                    }
                    
                  } else {
                    
                    //text is used as a mask for the rest of the image
                    //use the temp_canvas as a temporary buffer to build the mask  
                      CRGB * old_screen_buffer = led_screen.screen_buffer;
                      if (temp_canvas != nullptr) {
                        led_screen.screen_buffer = temp_canvas;             
                      }
                      blendXY(led_screen, u2+u_diff, v2+u_diff, hue, 0, 255  );
                      led_screen.screen_buffer = old_screen_buffer;  
                  }
                    
                    
                } else {
                  
                  //no filtering, coarse text
                  if (text_mask) {
                    //text is used as a mask for the rest of the image
                    int led_num = XY(x+offsets[offset_pos], ypos+ystp-8*i+offsets[offset_pos+1]);
                    led_mask[led_num] = 0;
                  } else {                  
                  //normal text drawn to the image
                  //drawXY_fine(led_screen, u+offsets[offset_pos]*256, v+offsets[offset_pos+1]*256, hue, text_saturation, text_brightness  );
                  //draw some cubes... this is a fucking mess!
                  int32_t z = (led_screen.camera_position.z-65*256)-text_animation[i] + (fmix32((i+1)*(y*8+x))%(400*256));
                      z = _max(z,35*256);
                      z = 0;
                      if (z < led_screen.camera_position.z-5*256) {;
                          VECTOR3 p((u+offsets[offset_pos]*256)*7-7*3*256,(v+offsets[offset_pos+1]*256-22000)*7,0);
                          cube_ang3 = 0;
                          if ( cube_ang2 % (65536*4) < 65536 ) {
                            //cube_ang3 = (sin8(cube_ang2/256)-127)/3;
                            cube_ang3 = cube_ang2/256;
                          }
                          rotate_y(p,cube_ang3); //rotates the cube as part of a letter (around the letter's y-axis)
                          p.z += z;
                          if (p.z < led_screen.camera_position.z-512) {
                            draw_cube( p, VECTOR3(512,512,512), VECTOR3_8(0,cube_ang3,0), CHSV(hue,sat,255), persist);
                          }

                      }

                      cube_ang2-=256;

                      
                      //std::cout << u2 << "," << v2 << "\n";
                }
                
                }
                
                //blendXY(led_screen, (x+offsets[pos])*(36+255), (ypos+offsets[pos+1])*(36+255), hue, 255, bitRead(font8x8_basic[letter][y],x)*96  );
                
                
                
              }
            }
          }
        }
        current_height+=8;
      }
  
  
      
    }

    if (text_effect == 4 && temp_canvas != nullptr && led_mask2 != nullptr && height_map_ptr != nullptr) {

      //attempt at a refractory effect that uses the temp_canvas as the source
      for (int x = 0; x < MATRIX_WIDTH; x++) {
        for (int y = 0; y < MATRIX_HEIGHT; y++) {
    
          //horizontal pixel difference
          int u_diff = (*height_map_ptr)[x+2][y+1] - (*height_map_ptr)[x][y+1]; //between -128*32 and 127*32
          u_diff /= 64; //-32 to 32
          //vertical pixel difference
          int v_diff = (*height_map_ptr)[x+1][y+2] - (*height_map_ptr)[x+1][y]; //between -128*32 and 127*32
          v_diff /= 64; //-32 to 32
          
          int u = x + u_diff;
          int v = y + v_diff;

          int light_total = 0;
          int light_count = 0;
          for (int u2 = u-1; u2 <= u+1; u2++) {
            for (int v2 = v-1; v2 <= v+1; v2++) {              

              if (u2 >= 0 && u2 < MATRIX_WIDTH && v2 >= 0 && v2 < MATRIX_HEIGHT) {
                light_total += temp_canvas[XY(u2,v2)].getAverageLight();
                light_count++;
              }

            }
          }

          //draw non-masked area
          CRGB rgb = temp_canvas[XY(x,y)];
          rgb.nscale8(255-led_mask2[XY(x,y)]);
          leds[XY(x,y)] += rgb;
          
          if (light_count > 0) {
            //refractory effects drawn inside the mask area
            CHSV hsv;
            hsv.h = text_color;
            hsv.s = text_saturation;
            hsv.v = ( _min((light_total*2)/light_count,255) * (led_mask2[XY(x,y)]) )/256L;
            leds[XY(x,y)] += hsv;
          }
        }
      }

      //clear the temp canvas
      for (int i = 0; i < NUM_LEDS; i++) {
        temp_canvas[i] = CRGB::Black;
      }
      
    }


    if (text_mask && text_filter && temp_canvas != nullptr) {
      //copy filtered text mask from temp canvas
      for (int i = 0; i < NUM_LEDS; i++) {
          led_mask[i] = 255-temp_canvas[i].getAverageLight();
          temp_canvas[i] = CRGB::Black;
        }
    }
    
    
    //SCROLL TEXT
    static unsigned int ystp_time = 0;
    if (millis() > ystp_time) {
      if (scrolling_text_enabled == 1) {
        ystp_time = millis()+48;
        ystp++;
        xstp++;
        if (scroll_reset == 1) {
          ystp = 0;
          xstp = 0;
        }
      } else {
        ystp = MATRIX_HEIGHT-1;
        xstp = 0;
      }
    }
   
  
    //END SCROLLING TEXT
  }
  draw_cubes();
  old_display_text=display_text;
} //handle_text()

void spherize(int32_t& u, int32_t& v) {
//SPHERE RENDERING

    //increased density at edges
    //decreased density in center
    //u = cos8(128-(u/MATRIX_WIDTH)/2)*MATRIX_WIDTH;
    //v = cos8(128-(v/MATRIX_HEIGHT)/2)*MATRIX_HEIGHT;
    u = ((cos16(32768-(u*128)/MATRIX_WIDTH)+32768)*MATRIX_WIDTH)/256;
    v = ((cos16(32768-(v*128)/MATRIX_WIDTH)+32768)*MATRIX_HEIGHT)/256;

    //now we need to adjust our square coordinates to fit inside a circle

    //translate our coordinates to be centered around 0,0
    u -= MATRIX_WIDTH*128;
    v -= MATRIX_HEIGHT*128;
    
    //calculate coordinate amplitude 
    //float: 0 - 1 (0 being center, 1 being edge of screen)
    // float x_val = abs(u)/(float)(MATRIX_WIDTH*128);
    // float y_val = abs(v)/(float)(MATRIX_HEIGHT*128);
    //fixed: 0 - 256 (0 being center, 256 being edge of screen)
    uint16_t x_val = (abs(u)*256)/(MATRIX_WIDTH*128);
    uint16_t y_val = (abs(v)*256)/(MATRIX_HEIGHT*128);

    //square each coordinate amplitude
    //this will tell us how much we need to adjust the other coordinate
    //this is based on a 45 degree unit vector = 1/sqrt(2) = 1/1.414 = .707
    //1-.707 = .293 (or 75/256)
    // float sqx = x_val*x_val*(1-.707f);
    // float sqy = y_val*y_val*(1-.707f);
    uint16_t sqx = (x_val*x_val*300)/65536;
    uint16_t sqy = (y_val*y_val*300)/65536;

    //use those values to adjust each coordinate
    //u -= u*sqy;
    //v -= v*sqx;
    u -= (u*sqy)/1024;
    v -= (v*sqx)/1024;

    //translate our coordinates back to screen space
    u += MATRIX_WIDTH*128;
    v += MATRIX_HEIGHT*128;
}


uint8_t draw_character(uint8_t font, uint8_t chr, int32_t chr_x, int32_t chr_y, CRGB rgb = CRGB(255,0,0), uint16_t scale = 256, const bool& subtractive = 0, uint8_t sharpen = 1) {
  //ADAFRUIT FONTS
    
        uint16_t letter = chr; //letter position, minus 20 for non-used characters
        letter -= 32;
        uint16_t letter_location = font_glyphs[font-1][letter][0]; ///< Pointer into GFXfont->bitmap
        uint8_t letter_width = font_glyphs[font-1][letter][1];    ///< Bitmap dimensions in pixels
        uint8_t letter_height = font_glyphs[font-1][letter][2];   ///< Bitmap dimensions in pixels
        uint8_t letter_xAdvance = font_glyphs[font-1][letter][3]; ///< Distance to advance cursor (x axis)
        int8_t letter_xOffset = font_glyphs[font-1][letter][4];  ///< X dist from cursor pos to UL corner
        int8_t letter_yOffset = font_glyphs[font-1][letter][5];  ///< Y dist from cursor pos to UL corner
        //int8_t center_offset = (MATRIX_WIDTH-letter_width)/2;
        for (int y = 0; y < letter_height; y++) {
            for (int x = 0;x < letter_width; x++) {
              uint8_t pixel_bit = bitRead(font_bitmaps[font-1][letter_location+(y*letter_width+x)/8],7-(y*letter_width+x)%8);
              if (pixel_bit) {
                int32_t u = (letter_xOffset+x)*(scale); //fixed width
                //long u = (x)*(scale);
                int32_t v = (-letter_yOffset-y)*(scale);
                u += chr_x;
                v += chr_y;

                if (u >= 0  && u < MATRIX_WIDTH*256 && v >= 0 && v < MATRIX_HEIGHT*256) {

                  //spherize(u,v);

                  //draw them on the screen
                  blendXY(led_screen, u, v, rgb, subtractive, sharpen);
                  //drawXY_fine(led_screen, u, v, 0, 0, 255 );
                }
                
                
              }
            }
        }
        return letter_xAdvance;
        //return letter_width+1;
}

//find the highest and lowest pixels for a line of characters
void hl_char_in_line(const char * chr, uint8_t font, int& highest, int& lowest) {
  highest = 0;
  lowest = 0;
  while (true) {
    char c = *chr++;
    if (c == '\0' || c == '\n') {
      break;
    }
    int letter_height = font_glyphs[font-1][c-32][2];   ///< Bitmap dimensions in pixels
    int letter_yOffset = font_glyphs[font-1][c-32][5];  ///< Y dist from cursor pos to UL corner  
    highest = _max(-letter_yOffset, highest);
    lowest = _min(-letter_yOffset-letter_height, lowest);
  }
}

int draw_characters(uint8_t font, const char * chr, int32_t chr_x, int32_t chr_y, const CRGB& rgb = CRGB(255,0,0), uint16_t scale = 256, bool subtractive = 0, uint8_t sharpen = 1) {
  int x_offset = 0;
  int y_offset = 0;
  int highest = 0;
  int lowest = 0;

  hl_char_in_line(&chr[0], font, highest, lowest);

  y_offset -= highest*scale;

  int pos = 0;
  while (true) {
    if (chr[pos] == '\0') {
      break;
    }
    if (chr[pos] == '\n') {
      x_offset = 0;
      y_offset += lowest*scale;
      pos++;
      hl_char_in_line(&chr[pos], font, highest, lowest);
      y_offset -= highest*scale;
      y_offset -= 1*256;
      continue;
    }
    x_offset += draw_character(font, chr[pos], chr_x+x_offset, chr_y+y_offset, rgb, scale, subtractive, sharpen)*scale;
    pos++;
  }
  return x_offset;
}


int draw_characters(uint8_t font, std::string chr, int32_t chr_x, int32_t chr_y, const CRGB& rgb = CRGB(255,0,0), uint16_t scale = 256, bool subtractive = false, uint8_t sharpen = 1) {
    return draw_characters(font, chr.c_str(), chr_x, chr_y, rgb, scale, subtractive, sharpen);
}

#endif