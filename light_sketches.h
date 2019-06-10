//Memory for light sketches is allocated dynamically, but only once.
//We allocate the amount of memory required to store the largest sketch.
//All sketches then share the same memory (only one sketch is active at a time).

#ifndef MATRIX_WIDTH
#define MATRIX_WIDTH 128
#define MATRIX_HEIGHT 32
#endif
#define NUM_LEDS (MATRIX_WIDTH*MATRIX_HEIGHT+1)
#define NUM_DOTS 8000

#define MAX_NUMBER_OF_LIGHT_SKETCHES 20

void update_matrix();

class LIGHT_SKETCH {
  public:
    virtual void next_effect() = 0;
    virtual void reset() = 0;
    virtual void setup() = 0;
    virtual void loop() = 0;
    virtual ~LIGHT_SKETCH () {}
};


#include "draw_functions.h"
#include "matrix.h"

//this is an abstract object for storing registered light sketches
class REGISTER_BASE {
  
  public:
    virtual void loop() = 0;
    virtual void next_effect() = 0;
    virtual void reset() = 0;
    virtual void destroy() = 0;
    virtual void create() = 0;
};


//the main object for handling all light sketches
class LIGHT_SKETCHES {
  private:
    //a LIGHT_SKETCH pointer to the currently active light sketch
    //we will later allocate the necessary amount of memory to initialize our largest sketch
    static char * buffer;


    //an array to store our registered light sketches
    static REGISTER_BASE * light_sketches[MAX_NUMBER_OF_LIGHT_SKETCHES];
    static int number_of_light_sketches;
    static int current_light_sketch;
    static int largest_sketch;
    static bool need_to_allocate;

    //allocate the memory necessary to store the largest registered light sketch
    //(if necessary)
    void salloc() {
      if(need_to_allocate) {
        buffer = new char[largest_sketch];
        //buffer = (char*)ps_malloc(largest_sketch);
        leds = new CRGB[NUM_LEDS+1];
        //leds = (CRGB*)ps_malloc(sizeof(CRGB)*(NUM_LEDS+1));
        temp_canvas = new CRGB[NUM_LEDS+1];
        //temp_canvas = (CRGB*)ps_malloc(sizeof(CRGB)*(NUM_LEDS+1));
        for (int i = 0; i < HEIGHTMAP_WIDTH; i++) {
          height_map[i] = new int[HEIGHTMAP_HEIGHT];
        }
        led_mask = new uint8_t[NUM_LEDS];
        led_mask2 = new uint8_t[NUM_LEDS];
        for (int i = 0; i < MATRIX_HEIGHT; i++) {
          y_buffer[i] = new int[2]; //stores the min/max X values per Y so that we can fill between them
        }
        for (int i = 0; i < MATRIX_WIDTH; i++) {
          z_buffer[i] = new int[MATRIX_HEIGHT];
        }
        light_sketches[current_light_sketch]->create();
        need_to_allocate = false;
        #ifdef __INC_FASTSPI_LED2_H 
        Serial.print(largest_sketch);
        Serial.println(" bytes allocated for largest sketch");
        #else
        std::cout << largest_sketch << " bytes allocated\n";
        #endif
      }
    }

    public:

    void loop() {
      salloc();
      light_sketches[current_light_sketch]->loop();
    }

    void reset() {
      light_sketches[current_light_sketch]->reset();
    }

    void next_effect() {
      light_sketches[current_light_sketch]->next_effect();
    }

    void next_sketch() {
        salloc();
        light_sketches[current_light_sketch]->destroy();
        current_light_sketch++;
        if (current_light_sketch >= number_of_light_sketches) {
          current_light_sketch = 0;
        }
        light_sketches[current_light_sketch]->create();
    }
    
    //create a template class so that we can register different light_sketches
    //I believe this is referred to as a "class factory".
    template <class T>
    class REGISTER: public REGISTER_BASE {
      private:
        T * sketch = nullptr;
      public:
        REGISTER (char const * name) {
          light_sketches[number_of_light_sketches] = this;
          number_of_light_sketches++;
          unsigned int s = sizeof(T);
            if (s > largest_sketch) {
              largest_sketch = s;
            }
            //std::cout << name << " has been registered. Requires " << sizeof(T) << " bytes.\n";
        }
        void destroy() {
          sketch->~T();
        }
        void loop() {
          sketch->loop();
        }
        void next_effect() {
          sketch->next_effect();
        }
        void reset() {
          sketch->reset();
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
int LIGHT_SKETCHES::largest_sketch = 0;
bool LIGHT_SKETCHES::need_to_allocate = true;

//initialize our object to handle light sketches

LIGHT_SKETCHES light_sketches;

#include "balls_squishy.h"
#include "balls2d.h"
//#include "cinco_de_mayo.h"
#include "curvy.h"
//#include "fire.h"
//#include "growcircle.h"
#include "mattclock.h"
 //#include "metaballs.h"
 #include "neon.h"
 #include "shapes.h"
 //#include "simplex.h"
 //#include "test2d.h"
 //#include "test3d.h"
 #include "phosphene.h"
 //#include "waves.h"