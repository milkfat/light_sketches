#ifndef LIGHTS_CURSOR_H
#define LIGHTS_CURSOR_H

int32_t cursor_position_x = 0;
int32_t cursor_position_y = 0;


//object to track cursor positions
class POINTER {
  public:
    uint8_t id;
    int x;
    int y;
    int x_old;
    int y_old;
    uint8_t pressure;
    bool down = false;
    bool new_press = false;
};

#define NUM_POINTERS 6
POINTER pointers[NUM_POINTERS];

int lookup_pointer(uint8_t id_in) {
  
  int next_available = -1;
  
  for (uint8_t i = 0; i < NUM_POINTERS; i++) {
    if (pointers[i].id == id_in) {
      return i;
      break;
    }
    if (pointers[i].down == false) {
      next_available = i;
    }
  }
  pointers[next_available].new_press = true;
  return next_available;
}

void update_pointer(int x_in, int y_in, int x_old_in, int y_old_in, uint8_t pressure_in, uint8_t id_in) {
  int pos = lookup_pointer(id_in);
  if(pos) {
    pointers[pos].id = id_in;
    pointers[pos].x = x_in;
    pointers[pos].y = y_in;
    pointers[pos].x_old = x_old_in;
    pointers[pos].y_old = y_old_in;
    pointers[pos].pressure = pressure_in;
    pointers[pos].down = true;
  }
}

void _remove_pointer(uint8_t id_in) {
  
  for (uint8_t i = 0; i < NUM_POINTERS; i++) {
    if (pointers[i].id == id_in) {
      pointers[i].down = false;
    }
  }
  
}






#endif