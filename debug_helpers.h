/*


static MEASURE_TIME m0 = MEASURE_TIME("grid effects: ");
m0.start();
m0.end();
measurements.print();


*/



struct MEASURE_TIME;

struct MEASUREMENTS {
  MEASURE_TIME * objects[10];
  int num_objects = 0;

  void reg (MEASURE_TIME * p) {
    objects[num_objects] = p;
    num_objects++;
  }

  void print();

};

MEASUREMENTS measurements;

struct MEASURE_TIME {
  char name[30] = "";
  uint32_t time_start = 0;
  uint32_t time_accum = 0;
  uint32_t samples = 0;

  MEASURE_TIME (const char * n) {
    strcpy(name, n);
    measurements.reg(this);
  }

  void start() {
    time_start = micros();
  }

  void end() {
    time_accum += micros() - time_start;
    samples++;
  }

};


void MEASUREMENTS::print() {
    static int cnt = 0;
    cnt++;
    if (cnt%600 == 0) {
      for(int j = 0; j < num_objects; j++) {
        if(objects[j]->samples != 0 ) {
          #ifdef ARDUINO
          Serial.print(objects[j]->name);
          Serial.println(objects[j]->time_accum / objects[j]->samples);
          #else
          std::cout << objects[j]->name << (objects[j]->time_accum / objects[j]->samples) << "\n";
          #endif
          objects[j]->samples = 0;
          objects[j]->time_accum = 0;
        }
      }
    }
  }