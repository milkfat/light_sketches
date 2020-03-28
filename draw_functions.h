#ifndef LIGHTS_DRAW_FUNCTIONS_H
#define LIGHTS_DRAW_FUNCTIONS_H

#include "led_screen.h"
#include "vector3.h"
#include "vector2.h"
#include "scale.h"
#include "gamma.h"
#include "math_helpers.h"
#include "draw_variables.h"

static inline __attribute__ ((always_inline)) void drawXY_fine(PERSPECTIVE& screen_object, const int32_t& xpos, const int32_t& ypos, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255) {
  
  CRGB rgb = CHSV(hue,sat,255);
  color_add_scaled_linear(screen_object.screen_buffer[screen_object.XY(xpos/256,ypos/256)], rgb, val);

}


static inline __attribute__ ((always_inline)) void drawXY(PERSPECTIVE& screen_object, const int& x, const int& y, const uint8_t& hue, const uint8_t& sat, const uint8_t& val) {
  
  CRGB rgb = CHSV(hue,sat,255);
  color_add_scaled_linear(screen_object.screen_buffer[screen_object.XY(x,y)], rgb, val);
  
}

static inline __attribute__ ((always_inline)) void drawXY_fineRGB(PERSPECTIVE& screen_object, const int32_t& xpos, const int32_t& ypos, const uint8_t& r, const uint8_t& g, const uint8_t& b) {

  color_add_linear8(screen_object.screen_buffer[screen_object.XY(xpos / 256,ypos / 256)], CRGB(r,g,b));

}

static inline __attribute__ ((always_inline)) void drawXY_RGB(PERSPECTIVE& screen_object, const int& x, const int& y, const uint8_t& r, const uint8_t& g, const uint8_t& b) {
  
  color_add_linear8(screen_object.screen_buffer[screen_object.XY(x,y)], CRGB(r,g,b));
  
}

static inline __attribute__ ((always_inline)) void drawXY_blend_gamma(PERSPECTIVE& screen_object, const uint16_t& led, const CRGB& rgb, const uint8_t& brightness = 255) {
  
  //treat RGB values as gamma 2.2
  //must be decoded, added, then re-encoded

  screen_object.screen_buffer[led].r = gamma16_encode( ( gamma16_decode(rgb.r)*brightness + gamma16_decode(screen_object.screen_buffer[led].r)*(255-brightness) ) >> 8);
  screen_object.screen_buffer[led].g = gamma16_encode( ( gamma16_decode(rgb.g)*brightness + gamma16_decode(screen_object.screen_buffer[led].g)*(255-brightness) ) >> 8);
  screen_object.screen_buffer[led].b = gamma16_encode( ( gamma16_decode(rgb.b)*brightness + gamma16_decode(screen_object.screen_buffer[led].b)*(255-brightness) ) >> 8);

  //nblend(screen_object.screen_buffer[screen_object.XY(x,y)], rgb, brightness);

}

static inline __attribute__ ((always_inline)) void drawXY_blend_gamma( PERSPECTIVE& screen_object, const int& x, const int& y, const int& z, const CRGB& rgb_in, const uint8_t& brightness = 255, const bool& ignore_z = true) {
  
  //treat RGB values as gamma 2.2
  //must be decoded, added, then re-encoded
  uint32_t led = XY(x,y);
  if (led >= 0 && led < NUM_LEDS-1) {

    int z_depth = z/16;
    if (z_buffer == nullptr || z_depth >= (*z_buffer)[x][y]) {

      if (z_buffer != nullptr && z_depth > (*z_buffer)[x][y]) {
        (*z_buffer)[x][y] = z_depth; 
      }
      
      //uint8_t bri = _clamp8(100 - z/768);
      uint32_t bri = _max(led_screen.camera_scaler - z,0)/256;
      bri /= 2;
      bri = _min(bri,255);
      bri = (bri*bri)>>8;
      bri = 255-bri;
      
      CRGB rgb = rgb_in;
      color_scale(rgb, bri);
      
      drawXY_blend_gamma(screen_object, led, rgb, brightness);

      //screen_object.screen_buffer[screen_object.XY(x,y)] = rgb;
    }

  }

}

static inline __attribute__ ((always_inline)) bool drawXYZ(PERSPECTIVE& screen_object, const int32_t& x, const int32_t& y, const int32_t& z, CRGB rgb, const bool& gamma = false) {
  bool on_screen = false;
  if (y >= 0 && y < screen_object.screen_height && x >= 0 && x < screen_object.screen_width) {
    on_screen = true;
    if (z_buffer == nullptr || z/16 > (*z_buffer)[x][y]) {

      if (z_buffer != nullptr) {
        (*z_buffer)[x][y] = z/16; 
      }


      //uint8_t bri = _clamp8(100 - z/768);
     // std::cout << screen_object.camera_scaler << " " << z << " ";
      uint32_t bri = _max(screen_object.camera_scaler - z,0)/256;
      bri = (bri*bri)/512;
      bri = 255-_min(bri,255);
      //std::cout << bri << "\n";

      
      color_scale(rgb, bri);
      

      if (gamma) {
        screen_object.screen_buffer[screen_object.XY(x,y)] = gamma8_decode(rgb);
        return on_screen;
      }

      screen_object.screen_buffer[screen_object.XY(x,y)] = rgb;

    }

  }
  return on_screen;
}

static inline __attribute__ ((always_inline)) void drawXYZ2(PERSPECTIVE& screen_object, const int32_t& x, const int32_t& y, const int32_t& z, const CRGB& rgb, const uint8_t& brightness = 255, const bool& ignore_z = true) {
  
  drawXY_blend_gamma(screen_object, x, y, z, rgb, brightness, ignore_z);

}


static inline __attribute__ ((always_inline)) bool blendXY(PERSPECTIVE& screen_object, const int32_t& xpos, const int32_t& ypos, CRGB& rgb, const bool& subtractive = false) {
  bool on_screen = false;
  //find the base x and y positions
  //add 2 pixels before division to avoid rounding errors at 0 (between -1 and 0)
  int x = xpos+512;
  int y = ypos+512;
  x /= 256;
  y /= 256;
  x -= 2;
  y -= 2;

  int xval = (xpos + 512) % 256; //amount of light right
  int yval = (ypos + 512) % 256; //amount of light bottom
  xval = ease8InOutApprox(xval);
  yval = ease8InOutApprox(yval);
  int x2val = 255 - xval; //amount of light left
  int y2val = 255 - yval; //amount of light top
  
  uint32_t led = screen_object.XY(x,y);

  if (led < NUM_LEDS-1) {
    on_screen = true;
    if (!subtractive) {
        color_add_scaled_linear(screen_object.screen_buffer[led], rgb, (x2val*y2val*1L)/(255L)); //top left

        if (x < screen_object.screen_width-1) {
          color_add_scaled_linear(screen_object.screen_buffer[screen_object.XY(x+1,y)], rgb, (xval*y2val*1L)/(255L)); //top right
        }
        
        if (x < screen_object.screen_width-1 && y < screen_object.screen_height-1) {
          color_add_scaled_linear(screen_object.screen_buffer[screen_object.XY(x+1,y+1)], rgb, (xval*yval*1L)/(255L)); //bottom right
        }

        if (y < screen_object.screen_height-1) {
          color_add_scaled_linear(screen_object.screen_buffer[screen_object.XY(x,y+1)], rgb, (x2val*yval*1L)/(255L)); //bottom left
        }
      } else {
        color_sub_scaled_linear(screen_object.screen_buffer[led], rgb, (x2val*y2val*1L)/(255L)); //top left

        if (x < screen_object.screen_width-1) {
          color_sub_scaled_linear(screen_object.screen_buffer[screen_object.XY(x+1,y)], rgb, (xval*y2val*1L)/(255L)); //top right
        }
        
        if (x < screen_object.screen_width-1 && y < screen_object.screen_height-1) {
          color_sub_scaled_linear(screen_object.screen_buffer[screen_object.XY(x+1,y+1)], rgb, (xval*yval*1L)/(255L)); //bottom right
        }

        if (y < screen_object.screen_height-1) {
          color_sub_scaled_linear(screen_object.screen_buffer[screen_object.XY(x,y+1)], rgb, (x2val*yval*1L)/(255L)); //bottom left
        }
      }
  }

  return on_screen;
  }





static inline __attribute__ ((always_inline)) bool blendXY(PERSPECTIVE& screen_object, const int32_t& xpos, const int32_t& ypos, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255) {
  CRGB rgb;
  hsv2rgb_rainbow(CHSV(hue,sat,val),rgb);
  return blendXY(screen_object, xpos, ypos, rgb);
}


static inline __attribute__ ((always_inline)) bool blendXY(PERSPECTIVE& screen_object, const VECTOR3& point, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255) {
  CRGB rgb;
  hsv2rgb_rainbow(CHSV(hue,sat,val),rgb);
  return blendXY(screen_object, point.x, point.y, rgb);
}


static inline __attribute__ ((always_inline)) bool blendXY(PERSPECTIVE& screen_object, const VECTOR3& point, CRGB rgb) {
  return blendXY(screen_object, point.x, point.y, rgb);
}

static inline __attribute__ ((always_inline)) bool blendXY(PERSPECTIVE& screen_object, const VECTOR2& point, CRGB rgb) {
  return blendXY(screen_object, point.x, point.y, rgb);
}


struct alpha_pixel {
    uint16_t r = 0;
    uint16_t g = 0;
    uint16_t b = 0;
    uint16_t a = 0;
    uint16_t cnt = 0;
};

static inline __attribute__ ((always_inline)) void blendXY_RGBA(alpha_pixel ap[], const int32_t& xpos, const int32_t& ypos, const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a) {
  
  //find the base x and y positions
  //add 2 pixels before division to avoid rounding errors at 0 (between -1 and 0)
  int x = xpos+512;
  int y = ypos+512;
  x /= 256;
  y /= 256;
  x -= 2;
  y -= 2;

  int xval = (xpos + 512) % 256; //amount of light right
  int yval = (ypos + 512) % 256; //amount of light bottom
  xval = ease8InOutApprox(xval);
  yval = ease8InOutApprox(yval);
  // int x2val = 255 - xval; //amount of light left
  // int y2val = 255 - yval; //amount of light top
  
  uint8_t l1 = 255; //top left
  uint8_t l2 = 255; //top right
  uint8_t l3 = 255; //bottom right
  uint8_t l4 = 255; //bottom left

  // uint8_t l1a = (l1*a)/255;
  // uint8_t l2a = (l2*a)/255;
  // uint8_t l3a = (l3*a)/255;
  // uint8_t l4a = (l4*a)/255;
  
  ap[XY(x,y)].r += (r*l1)/255;
  ap[XY(x,y)].g += (g*l1)/255;
  ap[XY(x,y)].b += (b*l1)/255;
  ap[XY(x,y)].a += (a*l1)/255;
  ap[XY(x,y)].cnt++;
  if (x < MATRIX_WIDTH-1) {
    ap[XY(x+1,y)].r += (r*l2)/255;
    ap[XY(x+1,y)].g += (g*l2)/255;
    ap[XY(x+1,y)].b += (b*l2)/255;
    ap[XY(x+1,y)].a += (a*l2)/255;
    ap[XY(x+1,y)].cnt++;
  }
  
  if (x < MATRIX_WIDTH-1 && y < MATRIX_HEIGHT-1) {
    ap[XY(x+1,y+1)].r += (r*l3)/255;
    ap[XY(x+1,y+1)].g += (g*l3)/255;
    ap[XY(x+1,y+1)].b += (b*l3)/255;
    ap[XY(x+1,y+1)].a += (a*l3)/255;
    ap[XY(x+1,y+1)].cnt++;
  }
  if (y < MATRIX_HEIGHT-1) {
    ap[XY(x,y+1)].r += (r*l4)/255;
    ap[XY(x,y+1)].g += (g*l4)/255;
    ap[XY(x,y+1)].b += (b*l4)/255;
    ap[XY(x,y+1)].a += (a*l4)/255;
    ap[XY(x,y+1)].cnt++;
  }

//  leds[XY(x,y)] += CHSV(hue,sat,(x2val*y2val*1L*val)/(255L*255L));
//  leds[XY(x+1,y)] += CHSV(hue,sat,(xval*y2val*1L*val)/(255L*255L));
//  leds[XY(x,y+1)] += CHSV(hue,sat,(x2val*yval*1L*val)/(255L*255L));
//  leds[XY(x+1,y+1)] += CHSV(hue,sat,(xval*yval*1L*val)/(255L*255L));
}


void y_buffer_fill(PERSPECTIVE& screen_object, const CRGB& rgb, const int32_t& z_depth) {

        int32_t x_min = MATRIX_WIDTH;
        int32_t x_max = -1;

        int32_t x_cnt = 0;
        int32_t x_min_avg = 0;
        int32_t x_max_avg = 0;
        
        for (int i = _max(y_buffer_min, 0); i <= _min(y_buffer_max,screen_object.screen_height-1); i++) {

          x_min_avg += _max(y_buffer[i][0], 0);
          x_max_avg += _min(y_buffer[i][1], MATRIX_WIDTH-1);
          x_cnt++;
          
          if (y_buffer[i][0] < x_min) {
            x_min = y_buffer[i][0];
          }
          if (y_buffer[i][1] > x_max) {
            x_max = y_buffer[i][1];
          }
        }

        int32_t y_min = MATRIX_HEIGHT;
        int32_t y_max = -1;

        int32_t y_cnt = 0;
        int32_t y_min_avg = 0;
        int32_t y_max_avg = 0;

        for (int i = _max(x_min, 0); i <= _min(x_max,screen_object.screen_width-1); i++) {

          y_min_avg += _max(x_buffer[i][0],0);
          y_max_avg += _min(x_buffer[i][1],screen_object.screen_height-1);
          y_cnt++;

          if (x_buffer[i][0] < y_min) {
            y_min = x_buffer[i][0];
          }
          if (x_buffer[i][1] > y_max) {
            y_max = x_buffer[i][1];
          }
        }


        int32_t x_center = x_min;
        int32_t y_center = y_min;

        if (x_cnt != 0) {
          x_center = (x_min_avg + x_max_avg + x_cnt) / (x_cnt*2);
        }

        if (y_cnt != 0) {
          y_center = (y_min_avg + y_max_avg + y_cnt) / (y_cnt*2);
        }

        //drawXYZ(led_screen, x_center, y_center, 100000, CRGB(255,255,255));

  for (int y = _max(y_buffer_min, 0); y <= _min(y_buffer_max,screen_object.screen_height-1); y++) {
    if (y_buffer[y][0] <= y_buffer[y][1]) {
      
        int32_t x_min2 = y_buffer[y][0];
        int32_t x_max2 = y_buffer[y][1];
        
        
        int32_t x_dist = _max(x_center-x_min2, x_max2-x_center);

      for (int x = y_buffer[y][0]; x <= y_buffer[y][1]; x++) {

        int32_t y_min2 = x_buffer[x][0];
        int32_t y_max2 = x_buffer[x][1];

        int32_t x_pos = (x - x_min2);
        int32_t y_pos = (y - y_min2);

        int32_t y_dist = _max(y_center-y_min2, y_max2-y_center);

        uint8_t x_amount = 255;

        if (x_dist != 0) {
          x_amount = _clamp8(255-(abs(x_dist-x_pos)*255)/x_dist);
          x_amount = gamma8_encode(x_amount);
        }
        uint8_t y_amount = 255;
        if (y_dist != 0) {
          y_amount = _clamp8(255-(abs(y_dist-y_pos)*255)/y_dist);
          y_amount = gamma8_encode(y_amount);
        }
        drawXY_blend_gamma(screen_object, x, y, z_depth, rgb,  (x_amount*y_amount)/255  );
        //drawXYZ(screen_object.screen_buffer, x, y, z_depth, rgb);
      }
    }
  }
}















#endif