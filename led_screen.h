#ifndef LIGHTS_LED_SCREEN_H
#define LIGHTS_LED_SCREEN_H

#include "perspective.h"
#include "draw_variables.h"
#include "scale.h"
#include "math_helpers.h"
#include "gamma.h"
#include "draw_functions.h"


PERSPECTIVE led_screen = PERSPECTIVE(MATRIX_WIDTH, MATRIX_HEIGHT);

#ifndef LIGHTS_ADJUST
#define LIGHTS_ADJUST
static inline __attribute__ ((always_inline)) int adjust (const int& p) {
    int po = p;
    
    //adjust for the ceiling beams in my home
    //if (po > 52) {
    //  po+=2;
    //}
    //if (po > 79) {
    //  po+=3;
    //}
    
    //adjust for window lights 149-199 being in reverse order
    //if ( po > (149) ) {
    //  po = 349 - po;
    //}

    return po;
  }
#endif

//return LED position from X,Y coordinates
//return NUM_LEDS-1 (our safety "invisible" pixel) if coordinates are off-screen
#ifndef XY_DEFINED
static inline __attribute__ ((always_inline)) uint32_t XY(const int& x, const int& y) {
    return led_screen.XY(x, y);
}
#endif


static inline __attribute__ ((always_inline)) void LED_show() {
  if (text_mask == 1 && led_mask != nullptr) {
    for (int i = 0;i < NUM_LEDS;i++) {
      if (invert_mask == 0) {
        leds[i].nscale8(255-led_mask[i]);
      }
      if (invert_mask == 1) {
        leds[i].nscale8(led_mask[i]);
      }
    }
  }
  led_screen.update();
  scale_update();
  #ifdef __INC_FASTSPI_LED2_H 
  //FastLED.show();
  update_matrix();
  #else
  update_matrix();
  #endif
  //FastLEDshowESP32();
  update_since_text = 1;
}




static inline __attribute__ ((always_inline)) void reset_y_buffer() {
  for (int i = 0; i < MATRIX_HEIGHT; i++) {
    y_buffer[i][0] = MATRIX_WIDTH + 1;
    y_buffer[i][1] = -1;

    //y_buffer2[i][0].position.x = INT32_MAX;
    //y_buffer2[i][1].position.x = INT32_MIN;

  }

  y_buffer_max = 0;
  y_buffer_min = MATRIX_HEIGHT-1;
  
}

static inline __attribute__ ((always_inline)) void reset_y_buffer2() {
  if (!y_buffer2) return;
  for (int i = 0; i < MATRIX_HEIGHT; i++) {
    y_buffer[i][0] = MATRIX_WIDTH + 1;
    y_buffer[i][1] = -1;

    (*y_buffer2)[i][0].position.x = INT32_MAX;
    (*y_buffer2)[i][1].position.x = INT32_MIN;

  }

  y_buffer_max = 0;
  y_buffer_min = MATRIX_HEIGHT-1;
  
}

static inline __attribute__ ((always_inline)) void reset_x_buffer() {
  for (int i = 0; i < MATRIX_WIDTH; i++) {
  x_buffer[i][0] = MATRIX_HEIGHT + 1;
  x_buffer[i][1] = -1;
  x_buffer_max = 0;
  x_buffer_min = MATRIX_WIDTH-1;
  }
}

static inline __attribute__ ((always_inline)) void reset_z_buffer(int16_t z_buffer[MATRIX_WIDTH][MATRIX_HEIGHT]) {

  //clear the Z buffer
  for (int x = 0; x < MATRIX_WIDTH; x++) {
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
      z_buffer[x][y] = INT16_MIN;
    }
  }

}




static inline __attribute__ ((always_inline)) void LED_black() {
  
  //clear the string
  memset(leds, 0, (MATRIX_WIDTH*MATRIX_HEIGHT)*3);
  

  if(z_buffer != nullptr) {
    z_buffer->reset();
  }

}
















#endif