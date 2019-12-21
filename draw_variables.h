#ifndef LIGHTS_DRAW_VARIABLES_H
#define LIGHTS_DRAW_VARIABLES_H


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

//int z_buffer[MATRIX_WIDTH][MATRIX_HEIGHT];
int16_t * z_buffer[MATRIX_WIDTH];


struct Y_BUF {
  VECTOR3 position;
  VECTOR3 ratio;
};
Y_BUF* y_buffer2[MATRIX_HEIGHT];



#endif