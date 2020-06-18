#ifndef LIGHTS_DRAW_HEIGHT_MAP_H
#define LIGHTS_DRAW_HEIGHT_MAP_H

//make the heightmap one pixel wider in each dimension to avoid surface normal aberations along the edges 
#define HEIGHTMAP_WIDTH (MATRIX_WIDTH+2)
#define HEIGHTMAP_HEIGHT (MATRIX_HEIGHT+2)


//CRGB debug_canvas[HEIGHTMAP_WIDTH*HEIGHTMAP_HEIGHT]; //object for teh drawing
//int height_map[HEIGHTMAP_WIDTH][HEIGHTMAP_HEIGHT];
int16_t (*height_map_ptr)[HEIGHTMAP_HEIGHT][HEIGHTMAP_WIDTH];

static void height_map_to_LED(const int& threshold = -128*256, const int& light_x = 100, const int& light_y = 100, const int& spec_x = 15, const int& spec_y = 15, const CRGB& rgb = CRGB(255,0,0), const CRGB& rgb2 = CRGB(255,255,255), const uint8_t& base_focus = 24, const uint8_t& spec_focus = 1) {
  //write our computed values to the screen
  
  for (uint16_t y = 0; y < MATRIX_HEIGHT; y++) {
    for (uint16_t x = 0; x < MATRIX_WIDTH; x++) {
      //height map coordinates
      //our height map is 1 pixel wider in each dimension than the screen

      if ((*height_map_ptr)[y+1][x+1] >= threshold) {
        int x2 = x+1;
        int y2 = y+1;
        uint32_t led = XY(x,y);
        //attempt to find the approximate surface normal

        
        //horizontal pixel difference
        int u = (*height_map_ptr)[y2][x2+1] - (*height_map_ptr)[y2][x2-1]; //between -128*32 and 127*32
  
        //vertical pixel difference
        int v = (*height_map_ptr)[y2+1][x2] - (*height_map_ptr)[y2-1][x2]; //between -128*32 and 127*32
        
        
        //find the brightness based on a specific difference (angle)
        //0-255; 0 = full bright; 255 = off;
        uint16_t u_norm = abs(-light_x*32 - u)/base_focus;
        uint16_t v_norm = abs(-light_y*32 - v)/base_focus;

        if (1==1) {
          //change angle of light to be more extreme at top
          v_norm = abs(-(light_y-(y >> 4))*32 - v)/base_focus;
        }
        
        //0 = off; 255 = full bright
        u_norm = _max(255 - u_norm,0);
        v_norm = _max(255 - v_norm,0);
        
        //combine the vertical and horizontal components to find our final brightness for this pixel
        uint16_t norm = (u_norm*v_norm);
  
        //light fades by distance
        uint16_t val = (norm*(MATRIX_HEIGHT*2-y))/(MATRIX_HEIGHT*2);
        uint8_t val2 = gamma16_encode(val);
        leds[led].r = (val2*rgb.r)/255;
        leds[led].g = (val2*rgb.g)/255;
        leds[led].b = (val2*rgb.b)/255;



        //specular highlights
        uint16_t su_norm = abs(-spec_x*32 - u)/spec_focus;
        uint16_t sv_norm = abs(-spec_y*32 - v)/spec_focus;

        su_norm = 255 - _min(su_norm, 255);
        sv_norm = 255 - _min(sv_norm, 255);
        
        uint16_t snorm = (su_norm*sv_norm) >> 2;
        

        color_add_linear16(leds[led].r, (snorm*rgb2.r)/255);
        color_add_linear16(leds[led].g, (snorm*rgb2.g)/255);
        color_add_linear16(leds[led].b, (snorm*rgb2.b)/255);
        

        
      }
    }
  }
}



static inline __attribute__ ((always_inline)) void reset_heightmap() {

  // for (int y = 0; y < MATRIX_HEIGHT; y++) {
  //   for (int x = 0; x < MATRIX_WIDTH; x++) {
  //     (*height_map_ptr)[y][x] = 0;
  //   }
  // }
  memset(height_map_ptr, 0, HEIGHTMAP_WIDTH*HEIGHTMAP_HEIGHT*sizeof((*height_map_ptr)[0][0]));

}



#endif