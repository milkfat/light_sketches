#ifndef LIGHTS_DRAW_CIRCLE_H
#define LIGHTS_DRAW_CIRCLE_H


uint8_t circle_angles[20][17];

static inline __attribute__ ((always_inline)) void reset_circle_angles() {
  for (int i = 0; i < 20; i++) {
    for (int j = 0; j < 16; j++) {
      circle_angles[i][j] = 255;
    }
  }
}

static inline __attribute__ ((always_inline)) void draw_circle_fine(const int32_t& x, const int32_t& y, const int32_t& r, const CRGB& rgb = CRGB(255,0,0), const int& ballnum = -1, const uint8_t& step_size = 16, const int32_t& z_depth = 0) {
  
  if(step_size == 255) {
    blendXY(led_screen, x, y, rgb);
    return;
  }

  int t0 = 0;
  int32_t xl2;
  int32_t yl2;
  //int32_t coords[16][2];
  //int32_t coord_pos = -1;
  while (t0 <= 256) {
    uint8_t t = t0;
    int32_t radius = r;
    if(ballnum > -1) {
      radius = r*circle_angles[ballnum][t/step_size]/256;
    }
    int32_t xl = ((cos8(t)-128)*radius)/128;
    int32_t yl = ((sin8(t)-128)*radius)/128;
    xl += x;
    yl += y;
    if (t0 > 0) {
      draw_line_fine(led_screen, xl, yl, xl2, yl2, rgb, z_depth, 255, 255, true);
      //coords[coord_pos][0] = xl2;
      //coords[coord_pos][1] = yl2;
    }
    xl2 = xl;
    yl2 = yl;
    //coord_pos++;
    t0+=step_size;
  }
  //matt_curve8(coords, 16, hue, sat, val, false, true, false,255,64);

  //blendXY(led_screen, x, y, hue, sat, val);
}

static inline __attribute__ ((always_inline)) void draw_circle_fine_hsv(const int32_t& x, const int32_t& y, const int32_t& r, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255, const int& ballnum = -1, const uint8_t& step_size = 16, const int32_t& z_depth = 0) {
  CRGB rgb = CHSV(hue,sat,val); 
  draw_circle_fine(x, y, r, rgb, ballnum, step_size, z_depth);
}  




#endif