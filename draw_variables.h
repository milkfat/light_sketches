#ifndef LIGHTS_DRAW_VARIABLES_H
#define LIGHTS_DRAW_VARIABLES_H

#include "vector3.h"
#include "vector4.h"


//CRGB leds[NUM_LEDS+1];
CRGB * main_led_buffer;
CRGB * leds;

//CRGB temp_canvas[NUM_LEDS+1]; //object for teh drawing
CRGB * temp_canvas; //object for teh drawing

uint8_t light_rotation_x = 70;
uint8_t light_rotation_y = 10;

//uint8_t led_mask[NUM_LEDS];
uint8_t * led_mask;
//uint8_t led_mask2[NUM_LEDS];
uint8_t * led_mask2;

#define NUM_IMAGE_BUFFERS ((MATRIX_WIDTH*MATRIX_HEIGHT*3)/1472 + 1)
uint8_t (* image_buffer)[1472] = nullptr;

class IMAGE_BUFFER {
    //we're buffering enough UDP packets to store RGB data for the entire screen
    //max UDP packet size is 1472
    uint8_t buffer[NUM_IMAGE_BUFFERS][1472];

  public:
    IMAGE_BUFFER () {
        image_buffer = &buffer[0];
    }
    ~IMAGE_BUFFER () {
        image_buffer = nullptr;
    }
};


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



int32_t y_buffer_max = 0;
int32_t y_buffer_min = MATRIX_HEIGHT-1;

#ifdef ENABLE_MULTITHREAD

struct SHAPE_TO_FILL {
  int state = 1;
  bool ready = 0;
  bool processed = 1;
  bool stop = 0;
  uint16_t low_x;
  uint16_t high_x;
  uint16_t low_y;
  uint16_t high_y;
  int z;
  CRGB rgb;
};
SHAPE_TO_FILL shape_to_fill;

#endif

struct Y_BUF {
    union {
      struct {
        int32_t x;
        int32_t y;
        int32_t z;
      }; 
      VECTOR3 position;
    };
    VECTOR4 ratio;
    uint8_t alpha = 255;

    Y_BUF() {
    }
};

#ifdef ENABLE_MULTITHREAD
std::mutex mutex_;
std::condition_variable condVar; 
#endif

class Y_BUF2 {
  Y_BUF buf[MATRIX_HEIGHT][2];

 public:
  void reset() {
#ifdef ENABLE_MULTITHREAD
      if (!shape_to_fill.processed) {
        std::unique_lock<std::mutex> lck(mutex_);
        condVar.wait(lck, []{ return shape_to_fill.processed; });
      }
#endif
      y_buffer_min = MATRIX_HEIGHT-1;
      y_buffer_max = 0;
      for (int y = 0; y < MATRIX_HEIGHT; y++) {
        buf[y][0].x = INT32_MAX;
        buf[y][1].x = INT32_MIN;
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

struct X_BUF {
  int32_t y;
  uint8_t alpha;
};

X_BUF x_buffer[MATRIX_WIDTH][2]; //stores the min/max Y values per X so that we can fill between them
int32_t x_buffer_max = 0;
int32_t x_buffer_min = MATRIX_WIDTH-1;

typedef Y_BUF2 y_buffer_t;


y_buffer_t* y_buffer = nullptr; //stores the min/max X values per Y so that we can fill between them

class Z_BUF;

Z_BUF* z_buffer = nullptr;

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
    z_buffer = this;
  }
  // Overloading [] operator to access elements in array style 
  int16_t& operator[] (int index) {
    return buf[index];
  } 

};


#endif