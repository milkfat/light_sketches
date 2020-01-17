#ifndef LIGHTS_DRAW_HEIGHT_MAP_H
#define LIGHTS_DRAW_HEIGHT_MAP_H

//make the heightmap one pixel wider in each dimension to avoid surface normal aberations along the edges 
#define HEIGHTMAP_WIDTH (MATRIX_WIDTH+2)
#define HEIGHTMAP_HEIGHT (MATRIX_HEIGHT+2)


//CRGB debug_canvas[HEIGHTMAP_WIDTH*HEIGHTMAP_HEIGHT]; //object for teh drawing
//int height_map[HEIGHTMAP_WIDTH][HEIGHTMAP_HEIGHT];
int16_t * height_map[HEIGHTMAP_HEIGHT];

static void height_map_to_LED(const int& threshold = -128*256, const int& light_x = 100, const int& light_y = 100, const int& spec_x = 15, const int& spec_y = 15) {
  //write our computed values to the screen
  uint16_t led = 0;
  for (uint16_t y = 0; y < MATRIX_HEIGHT; y++) {
    for (uint16_t x = 0; x < MATRIX_WIDTH; x++) {
      //height map coordinates
      //our height map is 1 pixel wider in each dimension than the screen
      int x2 = x+1;
      int y2 = y+1;
      if (height_map[y2][x2] >= threshold) {
        //attempt to find the approximate surface normal

        
        //horizontal pixel difference
        int u = height_map[y2][x2+1] - height_map[y2][x2-1]; //between -128*32 and 127*32
  
        //vertical pixel difference
        int v = height_map[y2+1][x2] - height_map[y2-1][x2]; //between -128*32 and 127*32
        
        
        //find the brightness based on a specific difference (angle)
        //0-255; 0 = full bright; 255 = off;
        uint16_t u_norm = abs(-light_x*32 - u)/24;
        uint16_t v_norm = abs(-light_y*32 - v)/24;

        if (1==1) {
          //change angle of light to be more extreme at top
          v_norm = abs(-(light_y-(y >> 4))*32 - v)/24;
        }
        
        //0 = off; 255 = full bright
        u_norm = _max(255 - u_norm,0);
        v_norm = _max(255 - v_norm,0);
        
        //combine the vertical and horizontal components to find our final brightness for this pixel
        uint16_t norm = (u_norm*v_norm);
  
        //light fades by distance
        uint16_t val = (norm*(255-y))/255;
        
        leds[led].r = gamma16_encode(_max(val,2));



        //specular highlights
        uint16_t su_norm = abs(-spec_x*32 - u);
        uint16_t sv_norm = abs(-spec_y*32 - v);

        su_norm = 255 - _min(su_norm, 255);
        sv_norm = 255 - _min(sv_norm, 255);
        
        uint16_t snorm = (su_norm*sv_norm) >> 2;
        

        color_add_linear16(leds[led].r, snorm);
        color_add_linear16(leds[led].g, snorm);
        color_add_linear16(leds[led].b, snorm);
        

        
      }
      led++;
    }
  }
}



static inline __attribute__ ((always_inline)) void reset_heightmap() {

  for (int y = 0; y < MATRIX_HEIGHT; y++) {
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      height_map[y][x] = 0;
    }
  }

}



#endif