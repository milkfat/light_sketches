#ifndef LIGHTS_DRAW_LINE_H
#define LIGHTS_DRAW_LINE_H


//DRAW LINE FINE


static void draw_line_fine_base(PERSPECTIVE& screen_object, const VECTOR3& a, const VECTOR3& b, const CRGB& rgb, const uint8_t& val = 255, const uint8_t& val2 = 255, const bool& trim = false, const bool& ignore_z = true, const bool& wide_fill = true, const bool& additive = false) {
  
  int32_t z_depth = a.z;

  //add one pixel to compensate for rounding errors between -1 and 0
  int32_t x1 = a.x+256;
  int32_t y1 = a.y+256;
  int32_t x2 = b.x+256;
  int32_t y2 = b.y+256;
  
  //avoid vertical and horizontal lines by fudging a bit
  if (x1 == x2 ) {
    x1++;
  }

  if (y1 == y2 ) {
    y2++;
  }
  bool flip = false;
  if ( abs(x1 - x2) > abs(y1 - y2) ) {
    //calculate horizontally
    //flip coordinates if necessary to calculate always in the positive direction
    if (x1 > x2) {
      int tempx = x1;
      int tempy = y1;
      x1 = x2;
      y1 = y2;
      x2 = tempx;
      y2 = tempy;
      flip = true;
    }
  } else {
    //calculate vertically
    //flip coordinates if necessary to calculate always in the positive direction
    if (y1 > y2) {
      int tempx = x1;
      int tempy = y1;
      x1 = x2;
      y1 = y2;
      x2 = tempx;
      y2 = tempy;
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
  
    x1_led = (x1+128)>>8;
    x1_r = ((x1_led+1)<<8) - (x1+128);
    x2_led = (x2-128+255)/256; //ceil
    x2_r = ((x2-128) - ((x2_led-1)<<8));


    y1_led = (y1+128)>>8;
    y1_r = ((y1_led+1)<<8) - (y1+128);
    y2_led = (y2-128+255)/256; //ceil
    y2_r = ((y2-128) - ((y2_led-1)<<8));
  
  } else {
    
    x1_led = x1>>8;
    x1_r = ((x1_led+1)<<8) - (x1);
    x2_led = (x2+255)/256; //ceil
    x2_r = ((x2) - ((x2_led-1)<<8));


    y1_led = y1>>8;
    y1_r = ((y1_led+1)<<8) - (y1);
    y2_led = (y2+255)/256; //ceil
    y2_r = ((y2) - ((y2_led-1)<<8));
  
  }

  int32_t x_dist = x2 - x1;
  int32_t y_dist = y2 - y1;
  float x_step = (x_dist*256.f)/y_dist;
  float y_step = (y_dist*256.f)/x_dist;
  if (abs(x1 - x2) > abs(y1 - y2)) {
    //calculate horizontally
    
    float y_start = y1 + ((x1_led*256 - x1)*y_step)/256.f;
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
        int Hy = (y+255)/256; //ceil
        int Ly = y/256;
        uint progress = abs(y - Hy*256);
        uint8_t b = ease8InOutApprox(_min( _max(progress, 0), 255));
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
          drawXYZ2(screen_object, i, Hy, z_depth, rgb, ((b2*v1)>>8) + ((b2*v2)>>8), ignore_z );
          drawXYZ2(screen_object, i, Ly, z_depth, rgb, ((b *v1)>>8) + ((b *v2)>>8), ignore_z );
        }

        //record stuff in our x and y buffers for other functions to use
        if (i >= 0 && i < MATRIX_WIDTH) {

          if (!wide_fill) {
            int temp = Ly;
            Ly = Hy;
            Hy = temp;
          }

          if (Hy <= x_buffer[i][0]) {
            x_buffer[i][0] = _min(x_buffer[i][0], Hy);

            y_buffer_min = _min(y_buffer_min,Hy);

            if (Hy >= 0 && Hy < MATRIX_HEIGHT) {
              y_buffer[Hy][0] = _min(y_buffer[Hy][0], i);
              y_buffer[Hy][1] = _max(y_buffer[Hy][1], i);
            }

          }

          if (Ly >= x_buffer[i][1]) {
            x_buffer[i][1] = _max(x_buffer[i][1], Ly);

            y_buffer_max = _max(y_buffer_max,Ly);

            if (Ly >= 0 && Ly < MATRIX_HEIGHT) {
              y_buffer[Ly][0] = _min(y_buffer[Ly][0], i);
              y_buffer[Ly][1] = _max(y_buffer[Ly][1], i);

            }

          }

        } else {

          if (Ly >= 0 && Ly < MATRIX_HEIGHT) {
            y_buffer[Ly][0] = _min(y_buffer[Ly][0], i);
            y_buffer[Ly][1] = _max(y_buffer[Ly][1], i);

          } 
          if (Hy >= 0 && Hy < MATRIX_HEIGHT) {
            y_buffer[Hy][0] = _min(y_buffer[Hy][0], i);
            y_buffer[Hy][1] = _max(y_buffer[Hy][1], i);
          }
         
        }
        //drawXYZ(screen_object,i, Hy, z_depth, hue, sat, (b2*v1)/256 + (b2*v2)/256 );
        //drawXYZ(screen_object,i, Ly, z_depth, hue, sat, (b *v1)/256 + (b *v2)/256 );
      }
      i++;
      y_start += y_step;
    }

    
  } else {
    //calculate vertically

    float x_start = x1 + ((y1_led*256-y1)*x_step)/256.f;
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
      int Hx = (x+255)/256; //ceil
      int Lx = x/256;
      uint progress = abs(x - Hx*256);
      uint8_t b = ease8InOutApprox(_min( _max(progress, 0), 255));
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
        drawXYZ2(screen_object, Hx, i, z_depth, rgb, ((b2*v1)>>8) + ((b2*v2)>>8), ignore_z );
        drawXYZ2(screen_object, Lx, i, z_depth, rgb, ((b *v1)>>8) + ((b *v2)>>8), ignore_z );
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

      if (Lx >= 0 && Lx < MATRIX_WIDTH) {
          x_buffer[Lx][0] = _min(x_buffer[Lx][0], i);
          x_buffer[Lx][1] = _max(x_buffer[Lx][1], i);
      }
      if (Hx >= 0 && Hx < MATRIX_WIDTH) {
          x_buffer[Hx][0] = _min(x_buffer[Hx][0], i);
          x_buffer[Hx][1] = _max(x_buffer[Hx][1], i);
      }
      
      //drawXYZ(screen_object,  ceil(x_start/256L), i, z_depth, h, s, (b2*v1)/256 + (b2*v2)/256 );
      //drawXYZ(screen_object, floor(x_start/256L), i, z_depth, h, s, (b *v1)/256 + (b *v2)/256 );


      //add the pixel we subtracted to compensate for rounding errors between -1 and 0
      i++;
      x_start += x_step;
    }
    
  }

} //draw_line_fine()




static inline __attribute__ ((always_inline)) void draw_line_fine(PERSPECTIVE& screen_object, const int32_t& x1, const int32_t& y1, const int32_t& x2, const int32_t& y2, CRGB& rgb, const int& z_depth = -10000, const uint8_t& val = 255, const uint8_t& val2 = 255, const bool& trim = false, const bool& ignore_z = true, const bool wide_fill = true, const bool additive = false) {
  VECTOR3 a(x1,y1,z_depth);
  VECTOR3 b(x2,y2,z_depth);
  draw_line_fine_base(screen_object, a, b, rgb, val, val2, trim, ignore_z, wide_fill, additive);
}


static inline __attribute__ ((always_inline)) void draw_line_fine(PERSPECTIVE& screen_object, const int32_t& x1, const int32_t& y1, const int32_t& x2, const int32_t& y2, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255, const int& z_depth = -10000, const uint8_t& val2 = 255, const bool& trim = false) {
  CRGB rgb = CHSV(hue,sat,255);
  draw_line_fine(screen_object, x1, y1, x2, y2, rgb, z_depth, val, val2, trim);
}

static inline __attribute__ ((always_inline)) void draw_line_fine(PERSPECTIVE& screen_object, const VECTOR3& a, const VECTOR3& b, CRGB& rgb, const int& z_depth = -10000, const uint8_t& val = 255, const uint8_t& val2 = 255, const bool& trim = false, const bool& ignore_z = true, const bool& wide_fill = true) {
  draw_line_fine(screen_object, a.x, a.y, b.x, b.y, rgb, z_depth, val, val2, trim, ignore_z, wide_fill);
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
      //drawXY(led_screen,x1,y1,0,0,255);
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
      //drawXY(led_screen,x1,y1,0,0,255);
      y1++;
      err += ax_dist;
      if (err >= ay_dist) {
        x1 += step;
        err -= ay_dist;
      }
    }
  }


}



static void draw_line_ybuffer(Y_BUF y_buffer2[MATRIX_HEIGHT][2], VECTOR3 a, VECTOR3 a_rgb, VECTOR3 b, VECTOR3 b_rgb) {

  a += 128;
  b += 128;
  a /= 256;
  b /= 256;
  VECTOR3 dist = b-a;
  VECTOR3 a_dist = abs(dist);

  VECTOR3 dist_rgb = b_rgb-a_rgb;
  VECTOR3 a_dist_rgb = abs(dist_rgb);

  VECTOR3 err(0,0,0);
  VECTOR3 step(0,0,0);

  VECTOR3 err_rgb(0,0,0);
  VECTOR3 step_rgb(0,0,0);


  if (a_dist.x > a_dist.y) {
    //draw horizontally
    if (a.x > b.x) {
      swap_coords(a,a_rgb,b,b_rgb,dist);
      dist_rgb.x = -dist_rgb.x;
      dist_rgb.y = -dist_rgb.y;
      dist_rgb.z = -dist_rgb.z;
    }
    step.y = sgn(dist.y);
    step.z = sgn(dist.z);
    step_rgb.x = sgn(dist_rgb.x);
    step_rgb.y = sgn(dist_rgb.y);
    step_rgb.z = sgn(dist_rgb.z);
    while (a.x <= b.x) {
      if (a.y >= 0 && a.y < MATRIX_HEIGHT) {
        y_buffer_min = _min(a.y,y_buffer_min);
        y_buffer_max = _max(a.y,y_buffer_max);
        //TODO: take into account a.z when a.x is equal, if necessary
        if (a.x < y_buffer2[a.y][0].position.x) {
          y_buffer2[a.y][0].position = a;
          y_buffer2[a.y][0].ratio = a_rgb;
        }
        if (a.x > y_buffer2[a.y][1].position.x) {
          y_buffer2[a.y][1].position = a;
          y_buffer2[a.y][1].ratio = a_rgb;
        }
      }
      //drawXY(led_screen,a.x,a.y,0,0,255);
      a.x++;
      
      iterate(a, step, a_dist, err, a_dist.x);
      
      iterate(a_rgb, step_rgb, a_dist_rgb, err_rgb, a_dist.x);

    }
  } else {
    //draw vertically
    if (a.y > b.y) {
      swap_coords(a,a_rgb,b,b_rgb,dist);
      dist_rgb.x = -dist_rgb.x;
      dist_rgb.y = -dist_rgb.y;
      dist_rgb.z = -dist_rgb.z;
    }
    step.x = sgn(dist.x);
    step.z = sgn(dist.z);
    step_rgb.x = sgn(dist_rgb.x);
    step_rgb.y = sgn(dist_rgb.y);
    step_rgb.z = sgn(dist_rgb.z);
    while (a.y <= b.y) {
      if (a.y >= 0 && a.y < MATRIX_HEIGHT) {
        y_buffer_min = _min(a.y,y_buffer_min);
        y_buffer_max = _max(a.y,y_buffer_max);
        if (a.x < y_buffer2[a.y][0].position.x) {
          y_buffer2[a.y][0].position = a;
          y_buffer2[a.y][0].ratio = a_rgb;
        }
        if (a.x > y_buffer2[a.y][1].position.x) {
          y_buffer2[a.y][1].position = a;
          y_buffer2[a.y][1].ratio = a_rgb;
        }
      }
      //drawXY(led_screen,a.x,a.y,0,0,255);

      a.y++;

      iterate(a, step, a_dist, err, a_dist.y);

      iterate(a_rgb, step_rgb, a_dist_rgb, err_rgb, a_dist.y);


    }
  }


}


static inline __attribute__ ((always_inline)) void draw_line_ybuffer(VECTOR3& a, VECTOR3& b) {
  draw_line_ybuffer(a.x, a.y, b.x, b.y);
}





#endif