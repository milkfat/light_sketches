#ifndef LIGHTS_DRAW_VARIABLES_H
#define LIGHTS_DRAW_VARIABLES_H

#include "vector3.h"


//CRGB leds[NUM_LEDS+1];
CRGB * main_led_buffer;
CRGB * leds;

//CRGB temp_canvas[NUM_LEDS+1]; //object for teh drawing
CRGB * temp_canvas; //object for teh drawing


//uint8_t led_mask[NUM_LEDS];
uint8_t * led_mask;
//uint8_t led_mask2[NUM_LEDS];
uint8_t * led_mask2;


//various functions shared amongst lighting sketches

bool spacebar = false;
bool next_sketch = false;
char next_sketch_name[20] = "";
bool reset_sketch = false;
bool typing_mode = false;

std::string display_text = "";
std::string old_display_text = "";


bool button0_down = false;
bool button1_down = false;
bool button1_click = false;
bool button2_down = false;
bool button3_down = false;

bool button_up_pressed = false;
bool button_down_pressed = false;
bool button_left_pressed = false;
bool button_right_pressed = false;

bool text_mask = 0;
bool invert_mask = 0;
uint8_t update_since_text = 1;

bool filter_particles = 1;


uint8_t default_color = 0;
uint8_t default_saturation = 255;
uint8_t default_value = 255;

int already_beat = 0;
int effect_beat = 0;
int growing = 0;



//int y_buffer[MATRIX_HEIGHT][2]; //stores the min/max X values per Y so that we can fill between them
int32_t * y_buffer[MATRIX_HEIGHT]; //stores the min/max X values per Y so that we can fill between them
int32_t y_buffer_max = 0;
int32_t y_buffer_min = MATRIX_HEIGHT-1;
int32_t x_buffer[MATRIX_WIDTH][2]; //stores the min/max Y values per X so that we can fill between them
int32_t x_buffer_max = 0;
int32_t x_buffer_min = MATRIX_WIDTH-1;

//int z_buffer[MATRIX_WIDTH][MATRIX_HEIGHT];
//int16_t * z_buffer[MATRIX_WIDTH];



struct Y_BUF {
    VECTOR3 position;
    VECTOR3 ratio;
    uint8_t opacity = 255;
};

class Y_BUF2 {
  Y_BUF buf[MATRIX_HEIGHT][2];
/*
public:
  void reset() {
    static MEASURE_TIME m0 = MEASURE_TIME("Y_BUF reset time: ");
    m0.start();
      y_buffer_min = MATRIX_HEIGHT-1;
      y_buffer_max = 0;
      for (int y = 0; y < MATRIX_HEIGHT; y+=2) {
        memcpy(&buf[y], &buf_zero[0], sizeof(Y_BUF)*4);
      }

    m0.end();
    measurements.print();
  }

  Y_BUF2 () {
    buf_zero[0].position = VECTOR3(INT32_MAX,0,0);
    buf_zero[1].position = VECTOR3(INT32_MIN,0,0);
    buf_zero[2].position = VECTOR3(INT32_MAX,0,0);
    buf_zero[3].position = VECTOR3(INT32_MIN,0,0);
    reset();
  }
  */



 public:
  void reset() {
      y_buffer_min = MATRIX_HEIGHT-1;
      y_buffer_max = 0;
      for (int y = 0; y < MATRIX_HEIGHT; y++) {
        buf[y][0].position = VECTOR3(INT32_MAX,0,0);
        buf[y][1].position = VECTOR3(INT32_MIN,0,0);
      }

  }

  Y_BUF2 () {
    reset();
  }
  // Overloading [] operator to access elements in array style 
  Y_BUF* operator[] (int index) {
    return buf[index];
  } 

};

Y_BUF2* y_buffer2 = nullptr;

class Z_BUF {
  int16_t buf[MATRIX_WIDTH*MATRIX_HEIGHT];

 public:
  void reset() {

    //method 3: fastest
    //memset
    //optimization sets all values to -32640 (rather than -32768)
    memset(&buf, -128, (MATRIX_WIDTH*MATRIX_HEIGHT)*sizeof(int16_t));

    //method 1: slow
    //reset all values in loop

    // for (int x = 0; x < MATRIX_WIDTH; x++) {
    //   for (int y = 0; y < MATRIX_HEIGHT; y++) {
    //     buf[x][y] = INT16_MIN;
    //  }
    // }

    //method 2: a bit faster
    //use std::fill
    //std::fill(&buf[0][0], &buf[MATRIX_WIDTH-1][MATRIX_HEIGHT-1], -32768);

  }

  Z_BUF () {
    reset();
  }
  // Overloading [] operator to access elements in array style 
  int16_t& operator[] (int index) {
    return buf[index];
  } 

};

Z_BUF* z_buffer = nullptr;

#endif