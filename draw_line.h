#ifndef LIGHTS_DRAW_LINE_H
#define LIGHTS_DRAW_LINE_H


//DRAW LINE FINE

uint8_t line_sharpen = 1;
static void draw_line_fine_base(PERSPECTIVE& screen_object, VECTOR3 a, VECTOR3 b, CRGB rgb, uint8_t val = 255, uint8_t val2 = 255, bool trim = false, bool ignore_z = true, bool wide_fill = true, bool additive = false) {
  
  //add one pixel to compensate for rounding errors between -1 and 0
  int32_t x1 = a.x+256;
  int32_t y1 = a.y+256;
  int32_t z1 = a.z+256;
  int32_t x2 = b.x+256;
  int32_t y2 = b.y+256;
  int32_t z2 = b.z+256;

  //avoid vertical and horizontal lines by fudging a bit
  if (x1 == x2 ) {
    x1++;
  }
  if (y1 == y2 ) {
    y2++;
  }
  if (z1 == z2 ) {
    z2++;
  }
  bool flip = false;
  if ( abs(x1 - x2) > abs(y1 - y2) ) {
    //calculate horizontally
    //flip coordinates if necessary to calculate always in the positive direction
    if (x1 > x2) {
      int tempx = x1;
      int tempy = y1;
      int tempz = z1;
      x1 = x2;
      y1 = y2;
      z1 = z2;
      x2 = tempx;
      y2 = tempy;
      z2 = tempz;
      flip = true;
    }
  } else {
    //calculate vertically
    //flip coordinates if necessary to calculate always in the positive direction
    if (y1 > y2) {
      int tempx = x1;
      int tempy = y1;
      int tempz = z1;
      x1 = x2;
      y1 = y2;
      z1 = z2;
      x2 = tempx;
      y2 = tempy;
      z2 = tempz;
      flip = true;
    }
  }

  //extend ends to an LED integer
  int x1_led;
  int x1_r;
  int y1_led;
  int y1_r;
  int x2_led;
  int x2_r;
  int y2_led;
  int y2_r;
  
  if (trim) {
  
    x1_led = (x1+128)/256;
    x1_r = ((x1_led+1)*256) - (x1+128);
    x2_led = (x2-128+256)/256; //ceil
    x2_r = ((x2-128) - ((x2_led-1)*256));


    y1_led = (y1+128)/256;
    y1_r = ((y1_led+1)*256) - (y1+128);
    y2_led = (y2-128+256)/256; //ceil
    y2_r = ((y2-128) - ((y2_led-1)*256));
  
  } else {
    
    x1_led = x1/256;
    x1_r = ((x1_led+1)*256) - (x1);
    x2_led = (x2+255)/256; //ceil
    x2_r = ((x2) - ((x2_led-1)*256));


    y1_led = y1/256;
    y1_r = ((y1_led+1)*256) - (y1);
    y2_led = (y2+255)/256; //ceil
    y2_r = ((y2) - ((y2_led-1)*256));
  
  }

  int32_t x_dist = x2 - x1;
  int32_t y_dist = y2 - y1;
  int32_t z_dist = z2 - z1;
  //save the coordinate extremes
  int32_t x_low = x1_led;
  int32_t y_low = y1_led;
  int32_t x_high = x2_led;
  int32_t y_high = y2_led;
  //clip coordinates to screen
  x1_led = _max(x1_led,0);
  y1_led = _max(y1_led,0);
  x2_led = _min(x2_led,MATRIX_WIDTH+1);
  y2_led = _min(y2_led,MATRIX_HEIGHT+1);

  if (abs(x1 - x2) > abs(y1 - y2)) {
    //calculate horizontally


    //calculate starting coordinates;
    float y_step = (y_dist*256.f)/x_dist;
    float y_start = y1 + ((x1_led*256 - x1)*y_step)/256.f;
    float z_step= (z_dist*256.f)/x_dist;
    float z_start = z1 + ((x1_led*256 - x1)*z_step)/256.f;
    // int32_t y_step = (y_dist*256)/x_dist;
    // int32_t y_start = y1 + ((x1_led*256 - x1)*y_step)/256;
    // int32_t z_step= (z_dist*256)/x_dist;
    // int32_t z_start = z1 + ((x1_led*256 - x1)*z_step)/256;


    //record the off-screen portion to the y_buffer
    if (x1_led != x_low) {
      int y_start_led = (y_start-y_step)/256;
      for (int y = _max(_min(y_low,y_start_led),0); y <= _min(_max(y_low,y_start_led),MATRIX_HEIGHT-1); y++) {
        y_buffer_min = _min(y_buffer_min, y);
        y_buffer_max = _max(y_buffer_max, y);
        y_buffer[y][0] = 0;
      }
    }

    //draw the line
    for (int i = (x1_led); i <= (x2_led); i++) {
      //if ((i != x1_led && i != x2_led) || !trim) {
      if (true) {
        uint8_t v1 = val;
        int pos = i - x1_led;
        if (flip) {
          pos = x2_led - i;
        }
        uint8_t v2;
        if (x2_led-x1_led != 0) {
          v2 = ((val2-val)*(pos))/(x2_led-x1_led);
        } else {
          v2 = (val2-val)*pos;
        }
        int y = y_start;
        int z = z_start;
        int Hy = (y+255)/256; //ceil
        int Ly = y/256;
        uint progress = abs(y - Hy*256);
        uint8_t b = _min( _max(progress, 0), 255);
        uint8_t s = line_sharpen;
        while (s--) {
          b = ease8InOutApprox(b);
        }
        uint8_t b2 = 255-b;
        if (i == x1_led) {
          b = (b*ease8InOutApprox(_min(x1_r,255)))/255;
          b2 = (b2*ease8InOutApprox(_min(x1_r,255)))/255;
        }
        if (i == x2_led) {
          b = (b*ease8InOutApprox(_min(x2_r,255)))/255;
          b2 = (b2*ease8InOutApprox(_min(x2_r,255)))/255;
        }

        //subtract the pixel we added to compensate for rounding errors between -1 and 0
        Hy--;
        Ly--;
        i--;

        if (additive) {
          CRGB rgb0 = rgb;
          color_scale(rgb0, ((b2*v1)>>8) + ((b2*v2)>>8));
          CRGB rgb1 = rgb;
          color_scale(rgb1, ((b *v1)>>8) + ((b *v2)>>8));
          color_add_gamma8(screen_object.screen_buffer[XY(i, Hy)], rgb0);
          color_add_gamma8(screen_object.screen_buffer[XY(i, Ly)], rgb1);
        } else {
          drawXYZ2(screen_object, i, Hy, z, rgb, ((b2*v1)>>8) + ((b2*v2)>>8), ignore_z );
          drawXYZ2(screen_object, i, Ly, z, rgb, ((b *v1)>>8) + ((b *v2)>>8), ignore_z );
        }

        if (!wide_fill) {
          int temp = Ly;
          Ly = Hy;
          Hy = temp;
        }

        //record stuff in our x and y buffers for other functions to use
        if (i >= 0 && i < MATRIX_WIDTH) {
          x_buffer_min = _min(i,x_buffer_min);
          x_buffer_max = _max(i,x_buffer_max);
          x_buffer[i][0] = _min(x_buffer[i][0], Hy);
          x_buffer[i][1] = _max(x_buffer[i][1], Ly);
        }

        if (Hy >= 0 && Hy < MATRIX_HEIGHT) {
          y_buffer_min = _min(y_buffer_min, Hy);
          y_buffer_max = _max(y_buffer_max, Hy);
          y_buffer[Hy][0] = _min(y_buffer[Hy][0], i+1);
          y_buffer[Hy][1] = _max(y_buffer[Hy][1], i-1);
        }

        if (Ly >= 0 && Ly < MATRIX_HEIGHT) {
          y_buffer_min = _min(y_buffer_min, Ly);
          y_buffer_max = _max(y_buffer_max, Ly);
          y_buffer[Ly][0] = _min(y_buffer[Ly][0], i+1);
          y_buffer[Ly][1] = _max(y_buffer[Ly][1], i-1);
        }
        
      }
      i++;
      y_start += y_step;
      z_start += z_step;
    }


    //record the off-screen portion to the y_buffer
    if (x2_led != x_high) {
      int y_start_led = (y_start-y_step)/256;
      for (int y = _max(_min(y_high,y_start_led),0); y <= _min(_max(y_high,y_start_led),MATRIX_HEIGHT-1); y++) {
        y_buffer_min = _min(y_buffer_min, y);
        y_buffer_max = _max(y_buffer_max, y);
        y_buffer[y][1] = MATRIX_WIDTH-1;
      }
    }
    
  } else {
    //calculate vertically

    float x_step = (x_dist*256.f)/y_dist;
    float x_start = x1 + ((y1_led*256-y1)*x_step)/256.f;
    float z_step  = (z_dist*256.f)/y_dist;
    float z_start = z1 + ((y1_led*256-y1)*z_step)/256.f;
    // int32_t x_step = (x_dist*256)/y_dist;
    // int32_t x_start = x1 + ((y1_led*256-y1)*x_step)/256;
    // int32_t z_step  = (z_dist*256)/y_dist;
    // int32_t z_start = z1 + ((y1_led*256-y1)*z_step)/256;


    //record the off-screen portion to the y_buffer
    if (y1_led != y_low) {
      int x_start_led = (x_start-x_step)/256;
      for (int x = _max(_min(x_low,x_start_led),0); x <= _min(_max(x_low,x_start_led),MATRIX_WIDTH-1); x++) {
        x_buffer_min = _min(x_buffer_min, x);
        x_buffer_max = _max(x_buffer_max, x);
        x_buffer[x][0] = 0;
      }
    }


    for (int i = (y1_led); i <= (y2_led); i++) {
      uint8_t v1 = val;
      int pos = i - y1_led;
      if (flip) {
        pos = y2_led - i;
      }
      uint8_t v2;
      if (y2_led-y1_led != 0) {
        v2 = ((val2-val)*(pos))/(y2_led-y1_led);
      } else {
        v2 = (val2-val)*pos;
      }
      int x = x_start;
      int z = z_start;
      int Hx = (x+255)/256; //ceil
      int Lx = x/256;
      uint progress = abs(x - Hx*256);
      uint8_t b = _min( _max(progress, 0), 255);
      uint8_t s = line_sharpen;
      while (s--) {
        b = ease8InOutApprox(b);
      }
      uint8_t b2 = 255-b;
      if (i == y1_led) {
        b  = (b*ease8InOutApprox(_min(y1_r,255)))/255;
        b2 = (b2*ease8InOutApprox(_min(y1_r,255)))/255;
      }
      if (i == y2_led) {
        b  = (b*ease8InOutApprox(_min(y2_r,255)))/255;
        b2 = (b2*ease8InOutApprox(_min(y2_r,255)))/255;
      }


      //subtract the pixel we added to compensate for rounding errors between -1 and 0
      Hx--;
      Lx--;
      i--;

      if (additive) {
        CRGB rgb0 = rgb;
        color_scale(rgb0, ((b2*v1)>>8) + ((b2*v2)>>8));
        CRGB rgb1 = rgb;
        color_scale(rgb1, ((b *v1)>>8) + ((b *v2)>>8));
        color_add_gamma8(screen_object.screen_buffer[XY(Hx, i)], rgb0);
        color_add_gamma8(screen_object.screen_buffer[XY(Lx, i)], rgb1);
      } else {
        drawXYZ2(screen_object, Hx, i, z, rgb, ((b2*v1)>>8) + ((b2*v2)>>8), ignore_z );
        drawXYZ2(screen_object, Lx, i, z, rgb, ((b *v1)>>8) + ((b *v2)>>8), ignore_z );
      }
      
      
      if (!wide_fill) {
        int temp = Lx;
        Lx = Hx;
        Hx = temp;
      }

      //record stuff in our x and y buffers for other functions to use
      if (i >= 0 && i < MATRIX_HEIGHT) {
        y_buffer_min = _min(i,y_buffer_min);
        y_buffer_max = _max(i,y_buffer_max);
        y_buffer[i][0] = _min(y_buffer[i][0], Hx);
        y_buffer[i][1] = _max(y_buffer[i][1], Lx);
      }

      if (Hx >= 0 && Hx < MATRIX_WIDTH) {
        x_buffer_min = _min(x_buffer_min, Hx);
        x_buffer_max = _max(x_buffer_max, Hx);
        x_buffer[Hx][0] = _min(x_buffer[Hx][0], i+1);
        x_buffer[Hx][1] = _max(x_buffer[Hx][1], i-1);
      }

      if (Lx >= 0 && Lx < MATRIX_WIDTH) {
        x_buffer_min = _min(x_buffer_min, Lx);
        x_buffer_max = _max(x_buffer_max, Lx);
        x_buffer[Lx][0] = _min(x_buffer[Lx][0], i+1);
        x_buffer[Lx][1] = _max(x_buffer[Lx][1], i-1);
      }
      
      //drawXYZ(screen_object,  ceil(x_start/256L), i, z_depth, h, s, (b2*v1)/256 + (b2*v2)/256 );
      //drawXYZ(screen_object, floor(x_start/256L), i, z_depth, h, s, (b *v1)/256 + (b *v2)/256 );


      //add the pixel we subtracted to compensate for rounding errors between -1 and 0
      i++;
      x_start += x_step;
      z_start += z_step;
    }

    //record the off-screen portion to the y_buffer
    if (y2_led != y_high) {
      int x_start_led = (x_start-x_step)/256;
      for (int x = _max(_min(x_high,x_start_led),0); x <= _min(_max(x_high,x_start_led),MATRIX_WIDTH-1); x++) {
        x_buffer_min = _min(x_buffer_min, x);
        x_buffer_max = _max(x_buffer_max, x);
        x_buffer[x][1] = MATRIX_HEIGHT-1;
      }
    }
    
  }

} //draw_line_fine()




static inline __attribute__ ((always_inline)) void draw_line_fine(PERSPECTIVE& screen_object, const int32_t& x1, const int32_t& y1, const int32_t& x2, const int32_t& y2, const CRGB& rgb = CRGB(0,0,255), const int& z_depth = -10000, const uint8_t& val = 255, const uint8_t& val2 = 255, const bool& trim = false, const bool& ignore_z = true, const bool wide_fill = true, const bool additive = false) {
  VECTOR3 a(x1,y1,z_depth);
  VECTOR3 b(x2,y2,z_depth);
  draw_line_fine_base(screen_object, a, b, rgb, val, val2, trim, ignore_z, wide_fill, additive);
}


static inline __attribute__ ((always_inline)) void draw_line_fine_hsv(PERSPECTIVE& screen_object, const int32_t& x1, const int32_t& y1, const int32_t& x2, const int32_t& y2, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255, const int& z_depth = -10000, const uint8_t& val2 = 255, const bool& trim = false) {
  CRGB rgb = CHSV(hue,sat,255);
  draw_line_fine(screen_object, x1, y1, x2, y2, rgb, z_depth, val, val2, trim,false);
}

static inline __attribute__ ((always_inline)) void draw_line_fine(PERSPECTIVE& screen_object, const VECTOR3& a, const VECTOR3& b, CRGB& rgb, const int& z_depth = -10000, const uint8_t& val = 255, const uint8_t& val2 = 255, const bool& trim = false, const bool& ignore_z = true, const bool& wide_fill = true, const bool additive = false) {
  draw_line_fine_base(screen_object, a, b, rgb, val, val2, trim, ignore_z, wide_fill,additive);
}

static void draw_line_ybuffer(const int32_t& x1i, const int32_t& y1i, const int32_t& x2i, const int32_t& y2i) {
  int32_t x1 = (x1i+128)/256;
  int32_t y1 = (y1i+128)/256;
  int32_t x2 = (x2i+128)/256;
  int32_t y2 = (y2i+128)/256;
  int32_t x_dist = x2-x1;
  int32_t y_dist = y2-y1;
  int32_t ax_dist = abs(x_dist);
  int32_t ay_dist = abs(y_dist);
  uint32_t err = 0;

  if (ax_dist > ay_dist) {
    //draw horizontally
    if (x1 > x2) {
      swap_coords(x1,y1,x2,y2,y_dist);
    }
    int32_t step = sgn(y_dist);
    while (x1 <= x2) {
      if (y1 >= 0 && y1 < MATRIX_HEIGHT) {
        y_buffer_min = _min(y1,y_buffer_min);
        y_buffer_max = _max(y1,y_buffer_max);
        y_buffer[y1][0] = _min(y_buffer[y1][0], x1);
        y_buffer[y1][1] = _max(y_buffer[y1][1], x1);
      }
      x1++;;
      err += ay_dist;
      if (err >= ax_dist) {
        y1 += step;
        err -= ax_dist;
      }
    }
  } else {
    //draw vertically
    if (y1 > y2) {
      swap_coords(x1,y1,x2,y2,x_dist);
    }
    int32_t step = sgn(x_dist);
    while (y1 <= y2) {
      if (y1 >= 0 && y1 < MATRIX_HEIGHT) {
        y_buffer_min = _min(y1,y_buffer_min);
        y_buffer_max = _max(y1,y_buffer_max);
        y_buffer[y1][0] = _min(y_buffer[y1][0], x1);
        y_buffer[y1][1] = _max(y_buffer[y1][1], x1);
      }
      y1++;
      err += ax_dist;
      if (err >= ay_dist) {
        x1 += step;
        err -= ay_dist;
      }
    }
  }


}


static void draw_line_ybuffer_base(VECTOR3 a, VECTOR3 a_rgb, VECTOR3 b, VECTOR3 b_rgb, bool fine=false) {

  a.y += 128;
  b.y += 128;

    if (!y_buffer2) return;
    if (a.y > b.y) {
        VECTOR3 temp = a;
        VECTOR3 temp_rgb = a_rgb;
        a = b;
        a_rgb = b_rgb;
        b = temp;
        b_rgb = temp_rgb;
    }
    VECTOR3 dist = b-a;
    VECTOR3 dist_rgb = b_rgb-a_rgb;

    if (dist.y != 0) {
      VECTOR3 c;
      c.y = (a.y+255)/256;
      c.y*=256;
      c.y = _max(c.y,0);
      int32_t y_end = b.y/256;
      y_end*=256;
      while (c.y <= _min(y_end,MATRIX_HEIGHT*256)) {
        if (c.y >= 0 && c.y < MATRIX_HEIGHT*256) {
          int32_t y_travel = c.y - a.y;
          y_travel = _max(_min(y_travel,dist.y),0);
          c.x = a.x + (dist.x * y_travel) / dist.y;
          c.z = a.z + (dist.z * y_travel) / dist.y;
          VECTOR3 c_rgb = a_rgb + (dist_rgb * y_travel) / dist.y;
          y_buffer_min = _min(c.y/256,y_buffer_min);
          y_buffer_max = _max(c.y/256,y_buffer_max);
          if (fine) {
            if (c.x < (*y_buffer2)[c.y/256][0].position.x) {
              (*y_buffer2)[c.y/256][0].position = c;
              (*y_buffer2)[c.y/256][0].ratio = c_rgb;
            }
            if (c.x > (*y_buffer2)[c.y/256][1].position.x) {
              (*y_buffer2)[c.y/256][1].position = c;
              (*y_buffer2)[c.y/256][1].ratio = c_rgb;
            }
          } else {
            VECTOR3 c2 = c/256;
            if (c2.x < (*y_buffer2)[c2.y][0].position.x) {
              (*y_buffer2)[c2.y][0].position = c2;
              (*y_buffer2)[c2.y][0].ratio = c_rgb;
            }
            if (c2.x > (*y_buffer2)[c2.y][1].position.x) {
              (*y_buffer2)[c2.y][1].position = c2;
              (*y_buffer2)[c2.y][1].ratio = c_rgb;
            }
          }
        }
        c.y+=256;
        

      }
    }
  


}

static inline void draw_line_ybuffer(VECTOR3& a, const VECTOR3& a_rgb, VECTOR3& b, const VECTOR3& b_rgb) {
  draw_line_ybuffer_base(a, a_rgb, b, b_rgb, false);
}


static inline void draw_line_ybuffer_fine(VECTOR3& a, const VECTOR3& a_rgb, VECTOR3& b, const VECTOR3& b_rgb) {
  draw_line_ybuffer_base(a, a_rgb, b, b_rgb, true);
}


static inline __attribute__ ((always_inline)) void draw_line_ybuffer(VECTOR3& a, VECTOR3& b) {
  draw_line_ybuffer(a.x, a.y, b.x, b.y);
}

/*
//different method of drawing... really buggy and didn't seem much faster/slower
static void draw_line_ybuffer_alt(VECTOR3 a, VECTOR3 a_rgb, VECTOR3 b, VECTOR3 b_rgb) {
  if (!y_buffer2) return;
  a += 128;
  b += 128;

  //if points a and b are equal in height then record and return
  if (a.y/256 == b.y/256) {
      a.y/=256;
      b.y/=256;
      if ( !(a.y >= 0 && a.y < MATRIX_HEIGHT) ) return;
      y_buffer_min = _min(a.y,y_buffer_min);
      y_buffer_max = _max(a.y,y_buffer_max);
      if (a.x < (*y_buffer2)[a.y][0].position.x) {
        (*y_buffer2)[a.y][0].position = a;
        (*y_buffer2)[a.y][0].ratio = a_rgb;
      }
      if (a.x > (*y_buffer2)[a.y][1].position.x) {
        (*y_buffer2)[a.y][1].position = a;
        (*y_buffer2)[a.y][1].ratio = a_rgb;
      }
      if (b.x < (*y_buffer2)[a.y][0].position.x) {
        (*y_buffer2)[a.y][0].position = b;
        (*y_buffer2)[a.y][0].ratio = b_rgb;
      }
      if (b.x > (*y_buffer2)[a.y][1].position.x) {
        (*y_buffer2)[a.y][1].position = b;
        (*y_buffer2)[a.y][1].ratio = b_rgb;
      }
      return;
  }
  
  VECTOR3 dist_fine = b-a;
  VECTOR3 dist_rgb = b_rgb-a_rgb;

  VECTOR3 dist_accum(0,0,0);
  VECTOR3 dist_step(0,0,0);

  VECTOR3 rgb_accum(0,0,0);
  VECTOR3 rgb_step(0,0,0);

  //draw vertically

  //always draw in the positive direction
  if (a.y > b.y) {
    swap_coords(a,a_rgb,b,b_rgb,dist_fine);
    dist_rgb.x = -dist_rgb.x;
    dist_rgb.y = -dist_rgb.y;
    dist_rgb.z = -dist_rgb.z;
  }

  //check that we're on screen
  if (b.y < 0 || a.y > (MATRIX_HEIGHT-1)*256) {
    return;
  }

  //trim our line to the bottom of the screen
  //this is to avoid drawing off the screen (super long lines)
  //and to avoid rounding errors with large coordinates
  if (a.y < 0) {
    uint y_axis_dist = 0 - a.y;
    uint y_dist = b.y - a.y;
    float d = (float)y_axis_dist/y_dist;
    VECTOR3 a_offset = dist_fine;
    VECTOR3 rgb_offset = dist_rgb;
    a_offset.x*=d;
    a_offset.y*=d;
    a_offset.z*=d;
    rgb_offset.x*=d;
    rgb_offset.y*=d;
    rgb_offset.z*=d;
    a+=a_offset;
    a_rgb+=rgb_offset;
    dist_fine = b-a;
    dist_rgb = b_rgb-a_rgb;
  }

  //trim our line to the top of the screen
  if (b.y > (MATRIX_HEIGHT-1)*256) {
    uint y_axis_dist = b.y - (MATRIX_HEIGHT-1)*256;
    uint y_dist = b.y - a.y;
    float d = (float)y_axis_dist/y_dist;
    VECTOR3 b_offset = dist_fine;
    VECTOR3 rgb_offset = dist_rgb;
    b_offset.x*=d;
    b_offset.y*=d;
    b_offset.z*=d;
    rgb_offset.x*=d;
    rgb_offset.y*=d;
    rgb_offset.z*=d;
    b-=b_offset;
    b_rgb-=rgb_offset;
    dist_fine = b-a;
    dist_rgb = b_rgb-a_rgb;
  }

  //set up some accumulator and step variables
  dist_accum = a;
  a/=256;
  b/=256;
  dist_step = dist_fine;
  rgb_accum = a_rgb*256;
  rgb_step = (dist_rgb*256);
  if (dist_fine.y != 0) {
    dist_step=(dist_step*256)/dist_fine.y;
    rgb_step=(rgb_step*256)/dist_fine.y;
  }
  
  //draw our line
  while (a.y <= b.y && a.y < MATRIX_HEIGHT) { //stop drawing when we go off the top of the screen
    if (a.y >= 0) {
      y_buffer_min = _min(a.y,y_buffer_min);
      y_buffer_max = _max(a.y,y_buffer_max);
      if (a.x < (*y_buffer2)[a.y][0].position.x) {
        (*y_buffer2)[a.y][0].position = a;
        (*y_buffer2)[a.y][0].ratio = a_rgb;
      }
      if (a.x > (*y_buffer2)[a.y][1].position.x) {
        (*y_buffer2)[a.y][1].position = a;
        (*y_buffer2)[a.y][1].ratio = a_rgb;
      }
    }
    if (a.y == b.y) break;
    //iterate our accumulators
    dist_accum+=dist_step;
    a = dist_accum/256;
    rgb_accum+=rgb_step;
    a_rgb=rgb_accum/256;
  }
  


}
*/

#endif