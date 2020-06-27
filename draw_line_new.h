#ifndef LIGHTS_DRAW_LINE_NEW_H
#define LIGHTS_DRAW_LINE_NEW_H

#include "draw_line.h"

static void draw_line_fine_new(PERSPECTIVE& screen_object, VECTOR3 a, VECTOR3 b, CRGB rgb, VECTOR3 a_ratio = VECTOR3(255,0,0), VECTOR3 b_ratio = VECTOR3(0,255,0)) {
  
  // LINE_PIXEL pixel_buffer[(uint)sqrt((abs(x1_led-x2_led)+1)*(abs(x1_led-x2_led)+1) + (abs(y1_led-y2_led+1)+1)*(abs(y1_led-y2_led)+1))*2];
  // uint16_t current_pixel = 0;
  

  //add one pixel to compensate for rounding errors between -1 and 0
  a+=256;
  b+=256;

  //avoid vertical and horizontal lines by fudging a bit
  if (a.x == b.x ) {
    b.x++;
  }
  if (a.y == b.y ) {
    b.y++;
  }
  if (a.z == b.z ) {
    b.z++;
  }
  
  if ( abs(a.x - b.x) > abs(a.y - b.y) ) {
    //calculate horizontally
    //flip coordinates if necessary to calculate always in the positive direction
    if (a.x > b.x) {
      VECTOR3 temp = a;
      a = b;
      b = temp;
      temp = a_ratio;
      a_ratio = b_ratio;
      b_ratio = temp;
    }
  } else {
    //calculate vertically
    //flip coordinates if necessary to calculate always in the positive direction
    if (a.y > b.y) {
      VECTOR3 temp = a;
      a = b;
      b = temp;
      temp = a_ratio;
      a_ratio = b_ratio;
      b_ratio = temp;
    }
  }

  //extend ends to an LED integer
  int x1_led = (a.x+128)/256;
  int x1_r = ((x1_led+1)*256) - (a.x+128);
  int x2_led = (b.x-128+256)/256; //ceil
  int x2_r = ((b.x-128) - ((x2_led-1)*256));


  int y1_led = (a.y+128)/256;
  int y1_r = ((y1_led+1)*256) - (a.y+128);
  int y2_led = (b.y-128+256)/256; //ceil
  int y2_r = ((b.y-128) - ((y2_led-1)*256));
  
  
  VECTOR3 ratio_dist = b_ratio-a_ratio;
  int32_t x_dist = b.x - a.x;
  int32_t y_dist = b.y - a.y;
  int32_t z_dist = b.z - a.z;
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

  if (abs(a.x - b.x) > abs(a.y - b.y)) {
    //calculate horizontally

    //calculate starting coordinates;
    VECTOR3 ratio_step = (ratio_dist*256)/x_dist;
    VECTOR3 ratio_start = a_ratio + ((VECTOR3(x1_led*256,x1_led*256,x1_led*256) - a_ratio)*ratio_step)/256;
    float y_step = (y_dist*256.f)/x_dist;
    float y_start = a.y + ((x1_led*256 - a.x)*y_step)/256.f;
    float z_step= (z_dist*256.f)/x_dist;
    float z_start = a.z + ((x1_led*256 - a.x)*z_step)/256.f;
    // int32_t y_step = (y_dist*256)/x_dist;
    // int32_t y_start = y1 + ((x1_led*256 - x1)*y_step)/256;
    // int32_t z_step= (z_dist*256)/x_dist;
    // int32_t z_start = z1 + ((x1_led*256 - x1)*z_step)/256;


    //record the off-screen portion to the y_buffer
    if (x1_led != x_low) {
      int y_start_led = (y_start-y_step)/256;
      int start_led = _max(_min(y_low-1,y_start_led-1),0);
      int end_led = _min(_max(y_low-1,y_start_led-1),MATRIX_HEIGHT-1);
      for (int y = start_led; y <= end_led; y++) {
        y_buffer_min = _min(y_buffer_min, y);
        y_buffer_max = _max(y_buffer_max, y);
        (*y_buffer)[y][0].x = -1;
      }
    }

    //draw the line
    for (int i = (x1_led); i <= (x2_led); i++) {
      
        int y = y_start;
        int z = z_start;
        int Ly = y/256;
        int Hy = Ly+1;
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
            

        //record stuff in our x and y buffers for other functions to use
        if (i >= 0 && i < MATRIX_WIDTH) {
            x_buffer_min = _min(i,x_buffer_min);
            x_buffer_max = _max(i,x_buffer_max);
            if (Ly == x_buffer[i][0].y) {
                x_buffer[i][0].alpha = _max(x_buffer[i][0].alpha, b);
            } else if (Ly < x_buffer[i][0].y) {
                x_buffer[i][0].y = Ly;
                x_buffer[i][0].alpha = b;
            }
            if (Hy == x_buffer[i][1].y) {
                x_buffer[i][1].alpha = _max(x_buffer[i][1].alpha, b2);
            } else if (Hy > x_buffer[i][1].y) {
                x_buffer[i][1].y = Hy;
                x_buffer[i][1].alpha = b2;
            }
        }

        if (Hy >= 0 && Hy < MATRIX_HEIGHT) {
            y_buffer_min = _min(y_buffer_min, Hy);
            y_buffer_max = _max(y_buffer_max, Hy);
            if (i == (*y_buffer)[Hy][0].x) {
                (*y_buffer)[Hy][0].alpha = _max((*y_buffer)[Hy][0].alpha, b2);
            } else if (i < (*y_buffer)[Hy][0].x) {
                (*y_buffer)[Hy][0].x = i;
                (*y_buffer)[Hy][0].alpha = b2;
            }
            if (i == (*y_buffer)[Hy][1].x) {
                (*y_buffer)[Hy][1].alpha = _max((*y_buffer)[Hy][1].alpha, b2);
            } else if (i > (*y_buffer)[Hy][1].x) {
                (*y_buffer)[Hy][1].x = i;
                (*y_buffer)[Hy][1].alpha = b2;
            }
        }

        if (Ly >= 0 && Ly < MATRIX_HEIGHT) {
            y_buffer_min = _min(y_buffer_min, Ly);
            y_buffer_max = _max(y_buffer_max, Ly);
            if (i == (*y_buffer)[Ly][0].x) {
                (*y_buffer)[Ly][0].alpha = _max((*y_buffer)[Ly][0].alpha, b);
            } else if (i < (*y_buffer)[Ly][0].x) {
                (*y_buffer)[Ly][0].x = i;
                (*y_buffer)[Ly][0].alpha = b;
            }
            if (i == (*y_buffer)[Ly][1].x) {
                (*y_buffer)[Ly][1].alpha = _max((*y_buffer)[Ly][1].alpha, b);
            } else if (i > (*y_buffer)[Ly][1].x) {
                (*y_buffer)[Ly][1].x = i;
                (*y_buffer)[Ly][1].alpha = b;
            }
        }
        
      
        i++;
        y_start += y_step;
        z_start += z_step;
    }


    //record the off-screen portion to the y_buffer
    if (x2_led != x_high) {
      int y_start_led = (y_start-y_step)/256;
      int start_led = _max(_min(y_high-1,y_start_led-1),0);
      int end_led = _min(_max(y_high-1,y_start_led-1),MATRIX_HEIGHT-1);
      for (int y = start_led; y <= end_led; y++) {
        y_buffer_min = _min(y_buffer_min, y);
        y_buffer_max = _max(y_buffer_max, y);
        (*y_buffer)[y][1].x = MATRIX_WIDTH;
      }
    }
  } else {

    //calculate vertically

    float x_step = (x_dist*256.f)/y_dist;
    float x_start = a.x + ((y1_led*256-a.y)*x_step)/256.f;
    float z_step  = (z_dist*256.f)/y_dist;
    float z_start = a.z + ((y1_led*256-a.y)*z_step)/256.f;
    // int32_t x_step = (x_dist*256)/y_dist;
    // int32_t x_start = x1 + ((y1_led*256-y1)*x_step)/256;
    // int32_t z_step  = (z_dist*256)/y_dist;
    // int32_t z_start = z1 + ((y1_led*256-y1)*z_step)/256;


    //record the off-screen portion to the y_buffer
    if (y1_led != y_low) {
      int x_start_led = (x_start-x_step)/256;
      int start_led = _max(_min(x_low-1,x_start_led-1),0);
      int end_led = _min(_max(x_low-1,x_start_led-1),MATRIX_WIDTH-1);
      for (int x = start_led; x <= end_led; x++) {
        x_buffer_min = _min(x_buffer_min, x);
        x_buffer_max = _max(x_buffer_max, x);
        x_buffer[x][0].y = -1;
      }
    }

    for (int i = (y1_led); i <= (y2_led); i++) {

      int x = x_start;
      int z = z_start;
      int Lx = x/256;
      int Hx = Lx+1;
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


      //record stuff in our x and y buffers for other functions to use
      if (i >= 0 && i < MATRIX_HEIGHT) {
        y_buffer_min = _min(i,y_buffer_min);
        y_buffer_max = _max(i,y_buffer_max);
        if (Lx == (*y_buffer)[i][0].x) {
            (*y_buffer)[i][0].alpha = _max((*y_buffer)[i][0].alpha, b);
        } else if (Lx < (*y_buffer)[i][0].x) {
            (*y_buffer)[i][0].x = Lx;
            (*y_buffer)[i][0].alpha = b;
        }
        if (Hx == (*y_buffer)[i][1].x) {
            (*y_buffer)[i][1].alpha = _max((*y_buffer)[i][1].alpha, b2);
        } else if (Hx > (*y_buffer)[i][1].x) {
            (*y_buffer)[i][1].x = Hx;
            (*y_buffer)[i][1].alpha = b2;
        }
      }

      if (Hx >= 0 && Hx < MATRIX_WIDTH) {
        x_buffer_min = _min(x_buffer_min, Hx);
        x_buffer_max = _max(x_buffer_max, Hx);
        if (i == x_buffer[Hx][0].y) {
            x_buffer[Hx][0].alpha = _max(x_buffer[Hx][0].alpha, b2);
        } else if (i < x_buffer[Hx][0].y) {
            x_buffer[Hx][0].y = i;
            x_buffer[Hx][0].alpha = b2;
        }
        if (i == x_buffer[Hx][1].y) {
            x_buffer[Hx][1].alpha = _max(x_buffer[Hx][1].alpha, b2);
        } else if (i > x_buffer[Hx][1].y) {
            x_buffer[Hx][1].y = i;
            x_buffer[Hx][1].alpha = b2;
        }
      }

      if (Lx >= 0 && Lx < MATRIX_WIDTH) {
        x_buffer_min = _min(x_buffer_min, Lx);
        x_buffer_max = _max(x_buffer_max, Lx);
        if (i == x_buffer[Lx][0].y) {
            x_buffer[Lx][0].alpha = _max(x_buffer[Lx][0].alpha, b);
        } else if (i < x_buffer[Lx][0].y) {
            x_buffer[Lx][0].y = i;
            x_buffer[Lx][0].alpha = b;
        }

        if (i == x_buffer[Lx][1].y) {
            x_buffer[Lx][1].alpha = _max(x_buffer[Lx][1].alpha, b);
        } else if (i > x_buffer[Lx][1].y) {
            x_buffer[Lx][1].y = i;
            x_buffer[Lx][1].alpha = b;
        }
      }

      //add the pixel we subtracted to compensate for rounding errors between -1 and 0
      i++;
      x_start += x_step;
      z_start += z_step;
    }

    //record the off-screen portion to the y_buffer
    if (y2_led != y_high) {
      int x_start_led = (x_start-x_step)/256;
      int start_led = _max(_min(x_high-1,x_start_led-1),0);
      int end_led = _min(_max(x_high-1,x_start_led-1),MATRIX_WIDTH-1);
      for (int x = start_led; x <= end_led; x++) {
        x_buffer_min = _min(x_buffer_min, x);
        x_buffer_max = _max(x_buffer_max, x);
        x_buffer[x][1].y = MATRIX_HEIGHT;

      }

    }
    
  }
  
} //draw_line_fine()


#endif