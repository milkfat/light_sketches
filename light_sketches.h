#ifndef LIGHTS_LIGHT_SKETCHES_H
#define LIGHTS_LIGHT_SKETCHES_H

#include "control_variables.h"

//Memory for light sketches is allocated dynamically, but only once.
//We allocate the amount of memory required to store the largest sketch.
//All sketches then share the same memory (only one sketch is active at a time).

#ifndef MATRIX_WIDTH
#define MATRIX_WIDTH 128
#define MATRIX_HEIGHT 32
#endif
#ifndef NUM_LEDS
#define NUM_LEDS (MATRIX_WIDTH*MATRIX_HEIGHT+1)
#endif
#define NUM_DOTS 8000

#define MAX_NUMBER_OF_LIGHT_SKETCHES 30

uint8_t global_brightness = 255;

void update_matrix();

class LIGHT_SKETCH {
  public:
    virtual void next_effect() = 0;
    virtual void reset() = 0;
    virtual void setup() = 0;
    virtual void loop() = 0;
    virtual ~LIGHT_SKETCH () {}
};


#include "cursor.h"
#include "math_helpers.h"
#include "draw_functions.h"
#include "draw_line.h"
#include "draw_line_new.h"
#include "draw_curve.h"
#include "draw_circle.h"
#include "rotate.h"
#include "rotate16.h"
#include "matrix.h"
#include "draw_height_map.h"
#include "cube.h"

//this is an abstract object for storing registered light sketches
class REGISTER_BASE {
  
  public:
    virtual void loop() = 0;
    virtual void next_effect() = 0;
    virtual void reset() = 0;
    virtual void destroy() = 0;
    virtual void create() = 0;
    virtual char* name() = 0;
    virtual int size() = 0;
};


//the main object for handling all light sketches
class LIGHT_SKETCHES {
  public:
    ~LIGHT_SKETCHES () {
      light_sketches[current_light_sketch]->destroy();
    }
  private:
    //a LIGHT_SKETCH pointer to the currently active light sketch
    //we will later allocate the necessary amount of memory to initialize our largest sketch
    static char * buffer;


    //an array to store our registered light sketches
    static REGISTER_BASE * light_sketches[MAX_NUMBER_OF_LIGHT_SKETCHES];
    static int number_of_light_sketches;
    static int current_light_sketch;
    static int next_light_sketch;
    static int largest_sketch;
    static bool need_to_allocate;
    void (*send_sketch_controls)() = nullptr;

    //allocate the memory necessary to store the largest registered light sketch
    //(if necessary)
    void salloc() {
      if(need_to_allocate) {

        //create some lookup tables for color conversion
        build_gamma_tables();

        int leds_size = sizeof(CRGB) * (NUM_LEDS +1);
        //int canvas_size = sizeof(CRGB) * (NUM_LEDS +1);
        //int heightmap_size = sizeof(int16_t) * (HEIGHTMAP_WIDTH*HEIGHTMAP_HEIGHT);
        //int mask_size = sizeof(uint8_t) * (NUM_LEDS);
        int ybuffer_size = sizeof(y_buffer_t);
        //int ybuffer2_size = sizeof(Y_BUF) * 2 * (MATRIX_HEIGHT);
        //int zbuffer_size = sizeof(int16_t[MATRIX_HEIGHT]) * (MATRIX_WIDTH);
        int cubes_size = sizeof(CUBE) * NUMBER_OF_CUBES;
        #ifdef __INC_FASTSPI_LED2_H 
        for (int i = 0; i < number_of_light_sketches; i++) {
            Serial.print(light_sketches[i]->name());
            Serial.print(" has been registered. Requires ");
            Serial.print(light_sketches[i]->size());
            Serial.println(" bytes.");
        }
        Serial.print(largest_sketch);
        Serial.println(" bytes required for largest sketch");
        Serial.print(leds_size);
        Serial.println(" bytes required for main CRGB object");
        //Serial.print(canvas_size);
        //Serial.println(" bytes required for temporary canvas");
        // Serial.print(heightmap_size);
        // Serial.println(" bytes required for height map");
        // Serial.print(mask_size);
        // Serial.println(" bytes required for led mask");
        // Serial.print(mask_size);
        // Serial.println(" bytes required for led mask2");
        Serial.print(ybuffer_size);
        Serial.println(" bytes required for Y buffer");
        // Serial.print(ybuffer2_size);
        // Serial.println(" bytes required for Y buffer2");
        // Serial.print(zbuffer_size);
        // Serial.println(" bytes required for Z buffer");
        Serial.print(cubes_size);
        Serial.println(" bytes required for cube cache");
        Serial.println();
        Serial.print(largest_sketch+leds_size+/*canvas_size+heightmap_size+mask_size+mask_size+*/ybuffer_size+/*ybuffer2_size+zbuffer_size+*/cubes_size);
        Serial.println(" bytes required total");
        Serial.printf("\r\nHeap Memory Available: %d bytes total, %d bytes largest free block: \r\n\r\n", heap_caps_get_free_size(0), heap_caps_get_largest_free_block(0));
        #else
        for (int i = 0; i < number_of_light_sketches; i++) {
            std::cout << light_sketches[i]->name() << " has been registered. Requires " << light_sketches[i]->size() << " bytes\n";
        }
        std::cout << largest_sketch << " bytes required for largest sketch\n";
        std::cout << leds_size << " bytes required for main CRGB object\n";
        //std::cout << canvas_size << " bytes required for temporary canvas\n";
        //std::cout << heightmap_size << " bytes required for height map\n";
        //std::cout << mask_size << " bytes required for led mask\n";
        //std::cout << mask_size << " bytes required for led mask2\n";
        std::cout << ybuffer_size << " bytes required for Y buffer\n";
        //std::cout << ybuffer2_size << " bytes required for Y buffer2\n";
        //std::cout << zbuffer_size << " bytes required for Z buffer\n\n";
        std::cout << cubes_size << " bytes required for cube cache\n\n";
        std::cout << largest_sketch+leds_size+/*canvas_size+heightmap_size+mask_size+mask_size+*/ybuffer_size+/*ybuffer2_size+zbuffer_size+*/cubes_size << " bytes required total\n\n";
        #endif

        buffer = new char[largest_sketch];

        #ifdef __INC_FASTSPI_LED2_H 
        Serial.println("*sketch buffer allocated");
        Serial.printf("Heap Memory Available: %d bytes total, %d bytes largest free block: \r\n\r\n", heap_caps_get_free_size(0), heap_caps_get_largest_free_block(0));
        #else
        std::cout << "*sketch buffer allocated\n";
        #endif

        //buffer = (char*)ps_malloc(largest_sketch);
        




        

        main_led_buffer = new CRGB[NUM_LEDS+1];
        leds = main_led_buffer;
        led_screen.screen_buffer = main_led_buffer;
        //leds = (CRGB*)ps_malloc(sizeof(CRGB)*(NUM_LEDS+1));

        for (int i = 0; i < NUM_LEDS; i++) {
          leds[i] = CRGB::Black;
        }

        #ifdef __INC_FASTSPI_LED2_H 
        Serial.println("*CRGB object allocated");
        Serial.printf("Heap Memory Available: %d bytes total, %d bytes largest free block: \r\n\r\n", heap_caps_get_free_size(0), heap_caps_get_largest_free_block(0));
        #else
        std::cout << "*CRGB object allocated\n";
        #endif





        // temp_canvas = new CRGB[NUM_LEDS+1];

        // for (int i = 0; i < NUM_LEDS; i++) {
        //   temp_canvas[i] = CRGB::Black;
        // }

        // #ifdef __INC_FASTSPI_LED2_H 
        // Serial.println("*temp canvas allocated");
        // Serial.printf("Heap Memory Available: %d bytes total, %d bytes largest free block: \r\n\r\n", heap_caps_get_free_size(0), heap_caps_get_largest_free_block(0));
        // #else
        // std::cout << "*temp canvas allocated\n";
        // #endif






        // int16_t* height_map_pool = new int16_t[HEIGHTMAP_WIDTH*HEIGHTMAP_HEIGHT];
        // for (int y = 0; y < HEIGHTMAP_HEIGHT; y++) {
        //   height_map[y] = &height_map_pool[y*HEIGHTMAP_WIDTH];
        //   for (int x = 0; x < HEIGHTMAP_WIDTH; x++) {
        //     height_map[y][x] = 0;
        //   }
        // }

        // #ifdef __INC_FASTSPI_LED2_H 
        // Serial.println("*height map allocated");
        // Serial.printf("Heap Memory Available: %d bytes total, %d bytes largest free block: \r\n\r\n", heap_caps_get_free_size(0), heap_caps_get_largest_free_block(0));
        // #else
        // std::cout << "*height map allocated\n";
        // #endif







        // led_mask = new uint8_t[NUM_LEDS];

        // #ifdef __INC_FASTSPI_LED2_H 
        // Serial.println("*led mask allocated");
        // Serial.printf("Heap Memory Available: %d bytes total, %d bytes largest free block: \r\n\r\n", heap_caps_get_free_size(0), heap_caps_get_largest_free_block(0));
        // #else
        // std::cout << "*led mask allocated\n";
        // #endif






        // led_mask2 = new uint8_t[NUM_LEDS];

        // #ifdef __INC_FASTSPI_LED2_H 
        // Serial.println("*led mask2 allocated");
        // Serial.printf("Heap Memory Available: %d bytes total, %d bytes largest free block: \r\n\r\n", heap_caps_get_free_size(0), heap_caps_get_largest_free_block(0));
        // #else
        // std::cout << "*led mask2 allocated\n";
        // #endif



        y_buffer = new y_buffer_t; //stores the min/max X values per Y so that we can fill between them
        for (int i = 0; i < MATRIX_HEIGHT; i++) {
          (*y_buffer)[i][0].x = MATRIX_WIDTH + 1;
          (*y_buffer)[i][1].x = -1;
        }


        #ifdef __INC_FASTSPI_LED2_H 
        Serial.println("*Y buffer allocated");
        Serial.printf("Heap Memory Available: %d bytes total, %d bytes largest free block: \r\n\r\n", heap_caps_get_free_size(0), heap_caps_get_largest_free_block(0));
        #else
        std::cout << "*Y buffer allocated\n";
        #endif



        // for (int i = 0; i < MATRIX_WIDTH; i++) {
        //   z_buffer[i] = new int16_t[MATRIX_HEIGHT];
        // }


        // #ifdef __INC_FASTSPI_LED2_H 
        // Serial.println("*Z buffer allocated");
        // Serial.printf("Heap Memory Available: %d bytes total, %d bytes largest free block: \r\n\r\n", heap_caps_get_free_size(0), heap_caps_get_largest_free_block(0));
        // #else
        // std::cout << "*Z buffer allocated\n";
        // #endif

        cubes = new CUBE[NUMBER_OF_CUBES];


        #ifdef __INC_FASTSPI_LED2_H 
        Serial.println("*CUBE cache allocated");
        Serial.printf("Heap Memory Available: %d bytes total, %d bytes largest free block: \r\n\r\n", heap_caps_get_free_size(0), heap_caps_get_largest_free_block(0));
        #else
        std::cout << "*CUBE cache allocated\n";
        #endif





        light_sketches[current_light_sketch]->create();
        need_to_allocate = false;
      }
    }

    public:

    void send_sketch_controls_reg(void (*func)()) {
      send_sketch_controls = func;
    }

    void change_sketch() {
      light_sketches[current_light_sketch]->destroy();
      height_map_ptr = nullptr;
      z_buffer = nullptr;
      led_screen.light_falloff = 8;
      led_screen.reset_camera();
      next_light_sketch %= number_of_light_sketches;
      current_light_sketch=next_light_sketch;
      light_sketches[current_light_sketch]->create();
      if (send_sketch_controls) send_sketch_controls();
    }

    void loop() {
      salloc();
      if (current_light_sketch != next_light_sketch) {
        change_sketch();
      }
      light_sketches[current_light_sketch]->loop();
    }

    void reset() {
      light_sketches[current_light_sketch]->reset();
      if (send_sketch_controls) send_sketch_controls();
    }

    char* name() {
      return light_sketches[current_light_sketch]->name();
    }
    
    int current_sketch() {
      return current_light_sketch;
    }

    const char* names(uint8_t pos) {
      if (pos < number_of_light_sketches) {
        return light_sketches[pos]->name();
      } else {
        return nullptr;
      }
    }

    void next_effect() {
      light_sketches[current_light_sketch]->next_effect();
      if (send_sketch_controls) send_sketch_controls();
    }

    void next_sketch() {
        next_light_sketch++;
    }

    void set_sketch(int i) {
        next_light_sketch=i;
    }

    void set_sketch(const char * c) {
        for (int i = 0; i < number_of_light_sketches; i++) {
          if ( strcmp(light_sketches[i]->name(), c) == 0 ) {
            set_sketch(i);
          }
        }
    }
    
    //create a template class so that we can register different light_sketches
    //I believe this is referred to as a "class factory".
    template <class T>
    class REGISTER: public REGISTER_BASE {
      private:
        char sketch_name[30];
      public:
        T * sketch = nullptr;
        REGISTER (char const * name) {
          strcpy(sketch_name, name);
          light_sketches[number_of_light_sketches] = this;
          number_of_light_sketches++;
          unsigned int s = sizeof(T);
            if (s > largest_sketch) {
              largest_sketch = s;
            }
        }
        void destroy() {
          sketch->~T();
          control_variables.clear();
          control_variables.add(global_brightness, "Global Brightness", 0, 255);
        }
        void loop() {
          #ifdef __INC_FASTSPI_LED2_H 
          static uint8_t current_brightness = 255;
          if (global_brightness != current_brightness) {
            FastLED.setBrightness(global_brightness);
          }
          #endif
          sketch->loop();
        }
        void next_effect() {
          sketch->next_effect();
        }
        void reset() {
          sketch->reset();
        }
        char* name() {
          return sketch_name;
        }
        int size() {
          return sizeof(T);
        }
        void create() {
            //construct our new light sketch object in the reserved memory location 
            //(effectively overwriting the previous sketch)
            new (buffer) T;
            sketch = reinterpret_cast<T*>(buffer);
        }
    };
   
};

//initialize some values
REGISTER_BASE * LIGHT_SKETCHES::light_sketches[MAX_NUMBER_OF_LIGHT_SKETCHES];
char * LIGHT_SKETCHES::buffer = nullptr;
int LIGHT_SKETCHES::number_of_light_sketches = 0;
int LIGHT_SKETCHES::current_light_sketch = 0;
int LIGHT_SKETCHES::next_light_sketch = 0;
int LIGHT_SKETCHES::largest_sketch = 0;
bool LIGHT_SKETCHES::need_to_allocate = true;




//initialize our object to handle light sketches

LIGHT_SKETCHES light_sketches;
#ifndef DISABLE_DEFAULT_SKETCHES
//#include "cube_collision.h"
//#include "video_stream.h"
#include "rollercoaster.h"
//#include "raytrace.h"
#include "tree_grow.h"
#include "leas_lights.h"
#include "tetris.h"
#include "balls_squishy.h"
#include "balls2d.h"
//  #include "cinco_de_mayo.h"
#include "curvy.h"
#include "fire.h"
#include "fireworks_3d.h"
#include "flight.h"
//  #include "growcircle.h"
//#include "the_matrix.h"
#include "mattclock.h"
#include "mandlebrot.h"
#include "metaballs.h"
#include "orbit.h"
// #include "neon.h"
#include "particles.h"
// #include "plasma_globe.h"
#include "rope.h"
// #include "shapes.h"
// #include "simplex.h"
#include "snowflakes.h"
#include "spinny.h"
#include "squares.h"
//  #include "test2d.h"
#include "test3d.h"
#include "tetris.h"
#include "text_display.h"
// //#include "tree_sim.h"
#include "tunnel.h"
#include "phosphene.h"
#include "untitled.h"
#include "waves.h"
#endif

#endif