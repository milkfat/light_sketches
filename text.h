
  //TEXT STUFF

#include "font.h"


bool text_filter = 0;
uint8_t text_effect = 0;
bool text_rotate = 0;
#define NUM_TEXT_EFFECTS 5
uint32_t text_shake_time = 0;
uint8_t current_font = 0;
int ystp = 0;

int text_color = -1;
uint8_t text_saturation = 255;
uint8_t text_brightness = 96;
uint8_t font_scaler = 48;
bool scrolling_text_enabled = 0;

void add_character(char c) {
    display_text.push_back(c);
}

static inline uint8_t bitRead(const uint8_t& this_byte, const uint8_t& this_bit) {
    return (this_byte & (1 << (this_bit))) ? 1 : 0;
}

void handle_text() {
  //if the screen has been updated then redraw the text
  if (update_since_text == 1) {

    reset_z_buffer();
    cube_ang+=512;
    cube_ang2 = cube_ang;
    cube_ang3 = cube_ang;

    //add canvas to LEDS
    // for (int i = 0; i < NUM_LEDS; i++) {
    //     leds[i] += canvas[i];
    //   }

    //add our temp canvas to LEDS
    // for (int i = 0; i < NUM_LEDS; i++) {
    //     leds[i] += temp_canvas[i];
    //     temp_canvas[i] = CRGB::Black;
    //   }

    // if (drawing_enabled) {
    //   //debug, draw line
    //   //leds[XY(round(cursor_position_x/256.0), round(cursor_position_y/256.0))].b = 255;
    //   long derpx = -2*256; //(4*256L)+201;
    //   long derpy = -2*256; //50*256L+45;
    //   //long derpy = 50*256L;
    //   draw_line_fine(leds, derpx, derpy,cursor_position_x, cursor_position_y);
    //   for (int i = 0; i < NUM_POINTERS; i++) {
    //     if (pointers[i].down) {
    //       drawXY(leds,pointers[i].x, pointers[i].y, 96);
    //     }
    //   }
    //   //draw_line(leds, derpx/255L, derpy/255L+20,cursor_position_x/255L, cursor_position_y/255L+20);
    // }

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

    //RESET THE MASK
    if (text_mask == 1) {
      for (int i = 0; i < NUM_LEDS; i++) {
        led_mask[i] = 255;
      }
    }

    
    for (int i = display_text.length(); i < 16; i++) {
        text_animation[i] = 0;
        text_animation_speed[i] = 1000;
    }
    for (int i = 0; i < display_text.length(); i++) {

      //keep track of this character's lookup position for the shaking offset
      uint8_t offset_pos = i%8*2;

      //decide the hue of this character
      uint8_t hue = (i*32)%255;
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
        uint8_t letter_xAdvance = font_glyphs[current_font-1][letter][3]; ///< Distance to advance cursor (x axis)
        int8_t letter_xOffset = font_glyphs[current_font-1][letter][4];  ///< X dist from cursor pos to UL corner
        int8_t letter_yOffset = font_glyphs[current_font-1][letter][5];  ///< Y dist from cursor pos to UL corner
        int8_t center_offset = (MATRIX_WIDTH-letter_width)/2;
        //float scaler = MATRIX_WIDTH/letter_width;
        for (int y = 0; y < letter_height; y++) {
          //int ypos = 0-round(y*font_scaler)+ystp-current_height;
          int ypos = 0-(y*(font_scaler+16))/64;
          if (ypos+ystp-current_height < -10 || ypos+ystp-current_height > MATRIX_HEIGHT+letter_height) {
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
                v += (ystp-current_height)*256L;
                if (text_mask == 0) {
                  if (text_filter == 1) {
                    blendXY(leds, u+offsets[offset_pos]*256, v+offsets[offset_pos+1]*256, hue, text_saturation, text_brightness );
                  } else {
                    drawXY_fine(leds, u+offsets[offset_pos]*256, v+offsets[offset_pos+1]*256, hue, text_saturation, 96  );
                  }
                } else {
                  int led_num = XY(x+offsets[offset_pos], ypos+ystp-current_height+offsets[offset_pos+1]);
                  led_mask[led_num] = 0;
                }
                
                //blendXY(leds,(x+offsets[pos])*(36+255), (ypos+offsets[pos+1])*(36+255), hue, 255, bitRead(font8x8_basic[letter][y],x)*96  );
                
              }
            }
        }
        }
        //advance the cursor for the next letter
        current_height += (letter_height*(font_scaler+16))/64+1;
      }
  
  
      //SOME FONT I FOUND
      
      if (current_font == 0) {
        uint8_t letter = display_text[i];
        for (int y = 0; y < 8; y++) {
          int32_t ypos = 0-y;
          if (ypos+ystp-8*i < -10 || ypos+ystp-8*i > MATRIX_HEIGHT+8) {
            break; //stop drawing if we go off the bottom of the screen
          }
          scroll_reset = 0;
          for (int x = 0; x < 8; x++) {
            uint8_t pixel_bit = bitRead(font8x8_basic[letter][y],x);
            
            //int u = cos(-angle) * x * (1.0 / scale) + sin(-angle) * y * (1.0 / scale);
            //int v = -sin(-angle) * x * (1.0 / scale) + cos(-angle) * y * (1.0 / scale);
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
                  
                } else if (text_effect == 2) {
                  
                  //DISPLACEMENT
                  
                  //horizontal pixel difference
                  u_diff = height_map[u2/256+2][v2/256+1] - height_map[u2/256][v2/256+1]; //between -128*32 and 127*32
                  u_diff/=8;
                  //vertical pixel difference
                  v_diff = height_map[u2/256+1][v2/256+2] - height_map[u2/256+1][v2/256]; //between -128*32 and 127*32
                  v_diff/=8;
                  
                } else if (text_effect == 3) {
  
                  //LIGHT BENDING?
                  u_diff = height_map[u2/256+2][v2/256+1] - height_map[u2/256][v2/256+1]; //between -128*32 and 127*32
                  if (u_diff > 0) {
                    //u_diff = 127*32 - abs(u_diff);
                    //u_diff = sq(u_diff)/(128L*32);
                  } else if (u_diff < 0) {
                    //u_diff = 127*32 - abs(u_diff);
                    //u_diff = -sq(u_diff)/(128L*32);
                  }
                  u_diff /= 24;
                  //vertical pixel difference
                  v_diff = height_map[u2/256+1][v2/256+2] - height_map[u2/256+1][v2/256]; //between -128*32 and 127*32
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

                    if (text_effect != 4) {
                      
                      //normal text drawn outside of mask area
                      blendXY(leds, u2, v2, hue, text_saturation, (text_brightness*(255-led_mask2[XY(u2/256,v2/256)]))/256L );
  
                      if (text_effect > 1) {
                      
                        //refractory effects drawn inside the mask area
                        blendXY(leds, u2+u_diff, v2+v_diff, hue, text_saturation, (text_brightness*(led_mask2[XY(u2/256,v2/256)]))/256L );
                      }

                    } else {
                      //text is drawn to temporary canvas to be used for later effects
                      blendXY(temp_canvas, u2+u_diff, v2+u_diff, hue, text_saturation, text_brightness  );
                    }
                    
                  } else {
                    //normal text, no effects, drawn to the image
                    blendXY(leds, u2, v2, hue, text_saturation, text_brightness );
                    
                  }
                  
                } else {
                  
                  //text is used as a mask for the rest of the image
                  //use the temp_canvas as a temporary buffer to build the mask                 
                  blendXY(temp_canvas, u2+u_diff, v2+u_diff, hue, 0, 255  );
                }
                  
                  
              } else {
                
                //no filtering, coarse text
                if (text_mask) {
                  //text is used as a mask for the rest of the image
                  int led_num = XY(x+offsets[offset_pos], ypos+ystp-8*i+offsets[offset_pos+1]);
                  led_mask[led_num] = 0;
                } else {                  
                 //normal text drawn to the image
                 //drawXY_fine(leds, u+offsets[offset_pos]*256, v+offsets[offset_pos+1]*256, hue, text_saturation, text_brightness  );
                 int32_t z = (camera_scaler-65)*256-text_animation[i] + (fmix32((i+1)*(y*8+x))%(400*256));
                    z = _max(z,35*256);
                    if (z < (camera_scaler-5)*256) {;
                        VECTOR3 p((u+offsets[offset_pos]*256)*7-7*3*256,(v+offsets[offset_pos+1]*256-42000)*7,0);
                        cube_ang3 = (sin8(cube_ang2/256)-127)/4;
                        //cube_ang3 = cube_ang2/256;
                        matrix.rotate_y(p,cube_ang3); //rotates the cube as part of a letter (around the letter's y-axis)
                        p.z += z;
                        
                        draw_cube(p, 512, 512, 512, 96, 192, 255);

                    }

                    cube_ang2-=256;

                    if (text_animation[i] < 200000) {
                        text_animation[i]+=text_animation_speed[i]/16;
                        text_animation_speed[i]+=4;
                    }
                    //std::cout << u2 << "," << v2 << "\n";
               }
               
              }
              
              //blendXY(leds, (x+offsets[pos])*(36+255), (ypos+offsets[pos+1])*(36+255), hue, 255, bitRead(font8x8_basic[letter][y],x)*96  );
              
              
              
            }
          }
        }
        current_height+=8;
      }
  
  
      
    }

    if (text_effect == 4) {

      //attempt at a refractory effect that uses the temp_canvas as the source
      for (int x = 0; x < MATRIX_WIDTH; x++) {
        for (int y = 0; y < MATRIX_HEIGHT; y++) {
    
          //horizontal pixel difference
          int u_diff = height_map[x+2][y+1] - height_map[x][y+1]; //between -128*32 and 127*32
          u_diff /= 64; //-32 to 32
          //vertical pixel difference
          int v_diff = height_map[x+1][y+2] - height_map[x+1][y]; //between -128*32 and 127*32
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


    if (text_mask && text_filter) {
      //copy filtered text mask from temp canvas
      for (int i = 0; i < NUM_LEDS; i++) {
          led_mask[i] = 255-temp_canvas[i].getAverageLight();
          temp_canvas[i] = CRGB::Black;
        }
    }
    
    
    //SCROLL TEXT
    static unsigned int ystp_time = 0;
    if (millis() - 48 > ystp_time) {
      if (scrolling_text_enabled == 1) {
        ystp_time = millis();
        ystp++;
        if (scroll_reset == 1) {
          ystp = 0;
        }
      } else {
        ystp = MATRIX_HEIGHT-1;
      }
    }
   
  
    //END SCROLLING TEXT
  }
}