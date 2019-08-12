//various functions shared amongst lighting sketches


#ifndef _min
#define _min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef _max
#define _max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef _clamp8
#define _clamp8(a) _min(_max(a,0),255)
#endif


std::string display_text = "";
std::string old_display_text = "";

//make the heightmap one pixel wider in each dimension to avoid surface normal aberations along the edges 
#define HEIGHTMAP_WIDTH (MATRIX_WIDTH+2)
#define HEIGHTMAP_HEIGHT (MATRIX_HEIGHT+2)

bool button0_down = false;
bool button1_down = false;
bool button1_click = false;
bool button2_down = false;
bool button3_down = false;

//https://stackoverflow.com/questions/1659440/32-bit-to-16-bit-floating-point-conversion
//by Phernost
//conserve memory by compressing floats to 16 bits when we don't need the precision of 32 bits
class Float16Compressor
    {
        union Bits
        {
            float f;
            int32_t si;
            uint32_t ui;
        };

        static int const shift = 13;
        static int const shiftSign = 16;

        static int32_t const infN = 0x7F800000; // flt32 infinity
        static int32_t const maxN = 0x477FE000; // max flt16 normal as a flt32
        static int32_t const minN = 0x38800000; // min flt16 normal as a flt32
        static int32_t const signN = 0x80000000; // flt32 sign bit

        static int32_t const infC = infN >> shift;
        static int32_t const nanN = (infC + 1) << shift; // minimum flt16 nan as a flt32
        static int32_t const maxC = maxN >> shift;
        static int32_t const minC = minN >> shift;
        static int32_t const signC = signN >> shiftSign; // flt16 sign bit

        static int32_t const mulN = 0x52000000; // (1 << 23) / minN
        static int32_t const mulC = 0x33800000; // minN / (1 << (23 - shift))

        static int32_t const subC = 0x003FF; // max flt32 subnormal down shifted
        static int32_t const norC = 0x00400; // min flt32 normal down shifted

        static int32_t const maxD = infC - maxC - 1;
        static int32_t const minD = minC - subC - 1;

    public:

        static inline uint16_t compress(const float& value)
        {
            Bits v, s;
            v.f = value;
            uint32_t sign = v.si & signN;
            v.si ^= sign;
            sign >>= shiftSign; // logical shift
            s.si = mulN;
            s.si = s.f * v.f; // correct subnormals
            v.si ^= (s.si ^ v.si) & -(minN > v.si);
            v.si ^= (infN ^ v.si) & -((infN > v.si) & (v.si > maxN));
            v.si ^= (nanN ^ v.si) & -((nanN > v.si) & (v.si > infN));
            v.ui >>= shift; // logical shift
            v.si ^= ((v.si - maxD) ^ v.si) & -(v.si > maxC);
            v.si ^= ((v.si - minD) ^ v.si) & -(v.si > subC);
            return v.ui | sign;
        }

        static inline float decompress(const uint16_t& value)
        {
            Bits v;
            v.ui = value;
            int32_t sign = v.si & signC;
            v.si ^= sign;
            sign <<= shiftSign;
            v.si ^= ((v.si + minD) ^ v.si) & -(v.si > subC);
            v.si ^= ((v.si + maxD) ^ v.si) & -(v.si > maxC);
            Bits s;
            s.si = mulC;
            s.f *= v.si;
            int32_t mask = -(norC > v.si);
            v.si <<= shift;
            v.si ^= (s.si ^ v.si) & mask;
            v.si |= sign;
            return v.f;
        }
    };

Float16Compressor FC;

bool text_mask = 0;
bool invert_mask = 0;
uint8_t update_since_text = 1;

bool filter_particles = 1;


//CRGB leds[NUM_LEDS+1];
CRGB * leds;

//CRGB temp_canvas[NUM_LEDS+1]; //object for teh drawing
CRGB * temp_canvas; //object for teh drawing

//CRGB debug_canvas[HEIGHTMAP_WIDTH*HEIGHTMAP_HEIGHT]; //object for teh drawing
//int height_map[HEIGHTMAP_WIDTH][HEIGHTMAP_HEIGHT];
int16_t * height_map[HEIGHTMAP_HEIGHT];


//uint8_t led_mask[NUM_LEDS];
uint8_t * led_mask;
//uint8_t led_mask2[NUM_LEDS];
uint8_t * led_mask2;

uint8_t default_color = 0;
uint8_t default_saturation = 255;
uint8_t default_value = 255;

uint8_t debug_scaler = 128;
int16_t camera_scaler = 232;
int16_t screen_scaler = 100;
float rotation_alpha = 0;
float rotation_beta = 90;
float rotation_gamma = 0;

int already_beat = 0;
int effect_beat = 0;
int growing = 0;


#define PI 355/113.f

static inline __attribute__ ((always_inline)) uint PI_m (const uint num) {
    return (num*355)/113;
};
static inline __attribute__ ((always_inline)) int PI_m (const int num) {
    return (num*355)/113;
};

static inline __attribute__ ((always_inline)) float PI_m (const float& num) {
    return (num*355)/113;
};

static inline __attribute__ ((always_inline)) int PI_d (const int& num) {
    return (num*113)/355;
};

static inline __attribute__ ((always_inline)) float PI_d (const float& num) {
    return (num*113)/355;
};

static inline __attribute__ ((always_inline)) uint8_t ease8In (const uint8_t& stp) {
  //ease in
  return stp*stp/255.f;
}
static inline __attribute__ ((always_inline)) uint8_t ease8Out (uint8_t stp) {
  //ease out
  stp = 255 - stp;
  stp = stp*stp/255;
  return 255 - stp;
}

//gamma 2.2 lookup tables:
//convert 8-bit linear to 8-bit gamma
uint8_t gamma8_e[256]; 

//convert 16-bit linear to 8-bit gamma
//first 256 values only, higher values should be divided by 256 and converted as 8-bit linear
uint8_t gamma8_e_16_low[256]; 

//decode 8-bit gamma to 8-bit linear
uint8_t gamma8_d[256];

//decode 8-bit gamma to 16-bit linear
uint16_t gamma16_d[256];

//decode 8-bit gamma to 12-bit linear
uint16_t gamma12_d[256];



static inline __attribute__ ((always_inline)) uint16_t gamma16_decode(const uint8_t& value) {
  //decode 8-bit gamma 2.2 into 16-bit linear
  return gamma16_d[value];
}

static inline __attribute__ ((always_inline)) uint16_t gamma12_decode(const uint8_t& value) {
  //decode 8-bit gamma 2.2 into 16-bit linear
  return gamma12_d[value];
}

static inline __attribute__ ((always_inline)) uint8_t gamma8_encode(const uint8_t& value) {
  //encode 8-bit linear into 8-bit gamma 2.2
  return gamma8_e[value];
}

static inline __attribute__ ((always_inline)) uint8_t gamma16_encode(const uint16_t& value) {
  //encode 16-bit linear into 8-bit gamma 2.2
  return (value>>8) ? gamma8_e[value>>8] : gamma8_e_16_low[value];

}

static inline __attribute__ ((always_inline)) uint8_t gamma8_decode(const uint8_t& value) {
  //decode 8-bit gamma 2.2 into 8-bit linear
  return gamma8_d[value];
}

static inline __attribute__ ((always_inline)) CRGB gamma8_encode(const CRGB& value) {
  CRGB rgb( gamma8_e[value.r], gamma8_e[value.g], gamma8_e[value.b] );
  return rgb;
}

static inline __attribute__ ((always_inline)) CRGB gamma8_decode(const CRGB& value) {
  CRGB rgb( gamma8_d[value.r], gamma8_d[value.g], gamma8_d[value.b] );
  return rgb;
}


static inline __attribute__ ((always_inline)) uint8_t gamma8_add_linear8(const uint8_t& gval, const uint8_t& lval) {
  //add an 8-bit linear value to an 8-bit gamma encoded value  
  return gamma16_encode( _min( ( gamma16_decode(gval) + (lval << 8) ), 65535) );
  
}

static inline __attribute__ ((always_inline)) void color_add_linear8(uint8_t& value, const uint8_t& value2) {
  value = gamma8_add_linear8(value, value2);
}

static inline __attribute__ ((always_inline)) uint8_t gamma8_add_linear16(const uint8_t& gval, const uint16_t& lval) {
  //add an 8-bit linear value to an 8-bit gamma encoded value  
  return gamma16_encode( _min( ( gamma16_decode(gval) + lval ), 65535) );
  
}

static inline __attribute__ ((always_inline)) void color_add_linear16(uint8_t& value, const uint16_t& value2) {
  value = gamma8_add_linear16(value, value2);
}

static inline __attribute__ ((always_inline)) void color_add_linear8(CRGB& rgb, const CRGB& rgb2) {
  color_add_linear8(rgb.r, rgb2.r);
  color_add_linear8(rgb.g, rgb2.g);
  color_add_linear8(rgb.b, rgb2.b);
}

static inline __attribute__ ((always_inline)) void color_scale(uint8_t& value, const uint8_t& scaler) {

  value = gamma16_encode(((gamma16_decode(value)*scaler)/255));

}

static inline __attribute__ ((always_inline)) CRGB& color_scale(CRGB& rgb, const uint8_t& scaler) {

  color_scale(rgb.r, scaler);
  color_scale(rgb.g, scaler);
  color_scale(rgb.b, scaler);
  return(rgb);

}

static inline __attribute__ ((always_inline)) void color_add_scaled_linear(CRGB& rgb, const CRGB& rgb2, const uint8_t& scaler) {
  color_add_linear8(rgb.r, (rgb2.r*scaler)/255);
  color_add_linear8(rgb.g, (rgb2.g*scaler)/255);
  color_add_linear8(rgb.b, (rgb2.b*scaler)/255);
}

static inline __attribute__ ((always_inline)) void color_blend_linear(CRGB& rgb1, const CRGB& rgb2, const uint8_t& brightness) {
  
  //treat RGB values as gamma 2.2
  //must be decoded, added, then re-encoded

  rgb1.r = gamma16_encode( ( (rgb2.r<<8)*brightness + gamma16_decode(rgb1.r)*(255-brightness) ) >> 8);
  rgb1.g = gamma16_encode( ( (rgb2.g<<8)*brightness + gamma16_decode(rgb1.g)*(255-brightness) ) >> 8);
  rgb1.b = gamma16_encode( ( (rgb2.b<<8)*brightness + gamma16_decode(rgb1.b)*(255-brightness) ) >> 8);

  //nblend(crgb_object[XY(x,y)], rgb, brightness);

}

static inline __attribute__ ((always_inline)) void color_blend_linear16(CRGB& rgb1, const uint16_t& r, const uint16_t& g, const uint16_t& b, const uint16_t& brightness) {
  
  //treat RGB values as gamma 2.2
  //must be decoded, added, then re-encoded

  rgb1.r = gamma16_encode( (r*brightness + gamma16_decode(rgb1.r)*(65535-brightness) ) >> 16);
  rgb1.g = gamma16_encode( (g*brightness + gamma16_decode(rgb1.g)*(65535-brightness) ) >> 16);
  rgb1.b = gamma16_encode( (b*brightness + gamma16_decode(rgb1.b)*(65535-brightness) ) >> 16);

  //nblend(crgb_object[XY(x,y)], rgb, brightness);

}


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







//CIE 1931 luminescence scale (or some shit)
static inline __attribute__ ((always_inline)) uint8_t cie (const uint8_t& a) {
  float y = (a*100.f)/255.f;
  if(y <= 8) {
    return y/903.3f;
  } else {
    float f = (y+16.f)/116.f;
    f = f*f*f;
    return f*255;
  }
}



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

//return LED position from X,Y coordinates
//return NUM_LEDS-1 (our safety "invisible" pixel) if coordinates are off-screen
static inline __attribute__ ((always_inline)) uint32_t XY(const int& x, const int& y) {
    if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT) {
      int32_t location = y*MATRIX_WIDTH + x;
      if (location > NUM_LEDS-1 || location < 0) {
          return NUM_LEDS-1;
      } else {
          return location;
      }
    } else {
      return NUM_LEDS-1;
    }
}

static inline __attribute__ ((always_inline)) void drawXY_fine(CRGB crgb_object[], const int32_t& xpos, const int32_t& ypos, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255) {
  
  CRGB rgb = CHSV(hue,sat,255);
  color_add_scaled_linear(crgb_object[XY(xpos/256,ypos/256)], rgb, val);

}


static inline __attribute__ ((always_inline)) void drawXY(CRGB crgb_object[], const int& x, const int& y, const uint8_t& hue, const uint8_t& sat, const uint8_t& val) {
  
  CRGB rgb = CHSV(hue,sat,255);
  color_add_scaled_linear(crgb_object[XY(x,y)], rgb, val);
  
}

static inline __attribute__ ((always_inline)) void drawXY_fineRGB(CRGB crgb_object[], const int32_t& xpos, const int32_t& ypos, const uint8_t& r, const uint8_t& g, const uint8_t& b) {

  color_add_linear8(crgb_object[XY(xpos / 256,ypos / 256)], CRGB(r,g,b));

}

static inline __attribute__ ((always_inline)) void drawXY_RGB(CRGB crgb_object[], const int& x, const int& y, const uint8_t& r, const uint8_t& g, const uint8_t& b) {
  
  color_add_linear8(crgb_object[XY(x,y)], CRGB(r,g,b));
  
}

//int y_buffer[MATRIX_HEIGHT][2]; //stores the min/max X values per Y so that we can fill between them
int32_t * y_buffer[MATRIX_HEIGHT]; //stores the min/max X values per Y so that we can fill between them
int32_t y_buffer_max = 0;
int32_t y_buffer_min = MATRIX_HEIGHT-1;
int32_t x_buffer[MATRIX_WIDTH][2]; //stores the min/max Y values per X so that we can fill between them

//int z_buffer[MATRIX_WIDTH][MATRIX_HEIGHT];
int16_t * z_buffer[MATRIX_WIDTH];

// static inline __attribute__ ((always_inline)) void drawXY_blend(CRGB crgb_object[], const int& x, const int& y, CRGB& rgb, const uint8_t& brightness = 255) {
  
//   nblend(crgb_object[XY(x,y)], rgb, brightness);

// }

// static inline __attribute__ ((always_inline)) void drawXY_blend(CRGB crgb_object[], const int& x, const int& y, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255, const uint8_t& brightness = 255) {
  
//   CRGB rgb;
//   rgb = CHSV(hue,sat,val);
//   drawXY_blend(crgb_object, x, y, rgb, brightness);

// }

static inline __attribute__ ((always_inline)) void drawXY_blend_gamma(CRGB crgb_object[], const uint16_t& led, const CRGB& rgb, const uint8_t& brightness = 255) {
  
  //treat RGB values as gamma 2.2
  //must be decoded, added, then re-encoded

  crgb_object[led].r = gamma16_encode( ( gamma16_decode(rgb.r)*brightness + gamma16_decode(crgb_object[led].r)*(255-brightness) ) >> 8);
  crgb_object[led].g = gamma16_encode( ( gamma16_decode(rgb.g)*brightness + gamma16_decode(crgb_object[led].g)*(255-brightness) ) >> 8);
  crgb_object[led].b = gamma16_encode( ( gamma16_decode(rgb.b)*brightness + gamma16_decode(crgb_object[led].b)*(255-brightness) ) >> 8);

  //nblend(crgb_object[XY(x,y)], rgb, brightness);

}

static inline __attribute__ ((always_inline)) void drawXY_blend_gamma(CRGB crgb_object[], const int& x, const int& y, const int& z, const CRGB& rgb_in, const uint8_t& brightness = 255, const bool& ignore_z = true) {
  
  //treat RGB values as gamma 2.2
  //must be decoded, added, then re-encoded
  if (y >= 0 && y < MATRIX_HEIGHT && x >= 0 && x < MATRIX_WIDTH) {

    int z_depth = z/16;
    if (ignore_z || z_depth >= z_buffer[x][y]) {

      if (z_depth > z_buffer[x][y]) {
        z_buffer[x][y] = z_depth; 
      }
      
      //uint8_t bri = _clamp8(100 - z/768);
      uint32_t bri = _max(camera_scaler - z/256,0);
      bri /= 2;
      bri = _min(bri,255);
      bri = (bri*bri)>>8;
      bri = 255-bri;
      
      CRGB rgb = rgb_in;
      color_scale(rgb, bri);
      
      drawXY_blend_gamma(crgb_object, XY(x,y), rgb, brightness);

      //crgb_object[XY(x,y)] = rgb;
    }

  }

}

static inline __attribute__ ((always_inline)) void drawXYZ(CRGB crgb_object[], const int32_t& x, const int32_t& y, const int32_t& z, CRGB rgb, const bool& gamma = false) {
  
  if (y >= 0 && y < MATRIX_HEIGHT && x >= 0 && x < MATRIX_WIDTH) {

    if (z/16 > z_buffer[x][y]) {

      z_buffer[x][y] = z/16; 


      //uint8_t bri = _clamp8(100 - z/768);
      uint32_t bri = _max(camera_scaler - z/256,0);
      bri /= 2;
      bri = _min(bri,255);
      bri = (bri*bri)>>8;
      
      bri = 255-bri;
      
      color_scale(rgb, bri);
      

      if (gamma) {
        crgb_object[XY(x,y)] = gamma8_decode(rgb);
        return;
      }

      crgb_object[XY(x,y)] = rgb;

    }

  }

}

// static inline __attribute__ ((always_inline)) void drawXYZ(CRGB crgb_object[], const int& x, const int& y, const int& z, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255) {
  
//   if (y >= 0 && y < MATRIX_HEIGHT) {
//     y_buffer[y][0] = _min(y_buffer[y][0], x);
//     y_buffer[y][1] = _max(y_buffer[y][1], x);
  
  
//     if (x >= 0 && x < MATRIX_WIDTH) {
//       if (z > -10000 && z > z_buffer[x][y]) {
//         z_buffer[x][y] = z; 
//         crgb_object[XY(x,y)] = CHSV(hue,sat,val);
//       } else if (z == -10000) {
//         //crgb_object[XY(x,y)] += CHSV(hue,sat,val);
//         drawXY_blend(crgb_object, x, y, hue, sat, val, 255);
//       }
//     }

//   }

// }


// static inline __attribute__ ((always_inline)) void drawXYZ2(CRGB crgb_object[], const int& x, const int& y, const int& z, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255, const uint8_t& brightness = 255) {
//   CRGB rgb = CHSV(hue,sat,val);
//   drawXY_blend_gamma(crgb_object, x, y, rgb, brightness);

// }

static inline __attribute__ ((always_inline)) void drawXYZ2(CRGB crgb_object[], const int32_t& x, const int32_t& y, const int32_t& z, const CRGB& rgb, const uint8_t& brightness = 255, const bool& ignore_z = true) {
  
  drawXY_blend_gamma(crgb_object, x, y, z, rgb, brightness, ignore_z);

}


static inline __attribute__ ((always_inline)) void blendXY(CRGB crgb_object[], const int32_t& xpos, const int32_t& ypos, CRGB& rgb) {
  
  //find the base x and y positions
  //add 2 pixels before division to avoid rounding errors at 0 (between -1 and 0)
  int x = xpos+512;
  int y = ypos+512;
  x /= 256;
  y /= 256;
  x -= 2;
  y -= 2;

  int xval = (xpos + 512) % 256; //amount of light right
  int yval = (ypos + 512) % 256; //amount of light bottom
  xval = ease8InOutApprox(xval);
  yval = ease8InOutApprox(yval);
  int x2val = 255 - xval; //amount of light left
  int y2val = 255 - yval; //amount of light top
  
  color_add_scaled_linear(crgb_object[XY(x,y)], rgb, (x2val*y2val*1L)/(255L)); //top left

  if (x < MATRIX_WIDTH-1) {
    color_add_scaled_linear(crgb_object[XY(x+1,y)], rgb, (xval*y2val*1L)/(255L)); //top right
  }
  
  if (x < MATRIX_WIDTH-1 && y < MATRIX_HEIGHT-1) {
    color_add_scaled_linear(crgb_object[XY(x+1,y+1)], rgb, (xval*yval*1L)/(255L)); //bottom right
  }

  if (y < MATRIX_HEIGHT-1) {
    color_add_scaled_linear(crgb_object[XY(x,y+1)], rgb, (x2val*yval*1L)/(255L)); //bottom left
  }

//  leds[XY(x,y)] += CHSV(hue,sat,(x2val*y2val*1L*val)/(255L*255L));
//  leds[XY(x+1,y)] += CHSV(hue,sat,(xval*y2val*1L*val)/(255L*255L));
//  leds[XY(x,y+1)] += CHSV(hue,sat,(x2val*yval*1L*val)/(255L*255L));
//  leds[XY(x+1,y+1)] += CHSV(hue,sat,(xval*yval*1L*val)/(255L*255L));
}



static inline __attribute__ ((always_inline)) void blendXY(CRGB crgb_object[], const int32_t& xpos, const int32_t& ypos, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255) {
  CRGB rgb;
  hsv2rgb_rainbow(CHSV(hue,sat,val),rgb);
  blendXY(crgb_object, xpos, ypos, rgb);
}

template <class T>
struct VECTOR3_CLASS {
  T x;
  T y;
  T z;

  inline VECTOR3_CLASS<T> () {}

  inline VECTOR3_CLASS<T> (const T& x_in, const T& y_in, const T& z_in): x(x_in), y(y_in), z(z_in) {}
  
  inline VECTOR3_CLASS<T> operator + (VECTOR3_CLASS<T> const &p_in) { 
    VECTOR3_CLASS<T> p; 
    p.x = x + p_in.x; 
    p.y = y + p_in.y; 
    p.z = z + p_in.z; 
    return p; 
  } 

  inline VECTOR3_CLASS<T> operator - (VECTOR3_CLASS<T> const &p_in) { 
    VECTOR3_CLASS<T> p; 
    p.x = x - p_in.x; 
    p.y = y - p_in.y; 
    p.z = z - p_in.z; 
    return p; 
  } 
  
  inline VECTOR3_CLASS<T> operator - (VECTOR3_CLASS<T> const &p_in) const { 
    VECTOR3_CLASS<T> p; 
    p.x = x - p_in.x; 
    p.y = y - p_in.y; 
    p.z = z - p_in.z; 
    return p; 
  } 

  inline VECTOR3_CLASS<T> operator * (int const &num) { 
    VECTOR3_CLASS<T> p; 
    p.x = x * num; 
    p.y = y * num; 
    p.z = z * num; 
    return p; 
  } 

  inline VECTOR3_CLASS<T> operator / (int const &num) { 
    VECTOR3_CLASS<T> p; 
    p.x = x / num; 
    p.y = y / num; 
    p.z = z / num; 
    return p; 
  } 


  //overload -=
  inline VECTOR3_CLASS<T>& operator-= (const VECTOR3_CLASS<T>& rhs) {
    this->x -= rhs.x;
    this->y -= rhs.y;
    this->z -= rhs.z;
    return *this;
  }
  //overload +=
  inline VECTOR3_CLASS<T>& operator+= (const VECTOR3_CLASS<T>& rhs) {
    this->x += rhs.x;
    this->y += rhs.y;
    this->z += rhs.z;
    return *this;
  }

  //overload +=
  inline VECTOR3_CLASS<T>& operator+= (const int& rhs) {
    this->x += rhs;
    this->y += rhs;
    this->z += rhs;
    return *this;
  }
  
  //overload /=
  inline VECTOR3_CLASS<T>& operator/= (const int& rhs) {
    this->x /= rhs;
    this->y /= rhs;
    this->z /= rhs;
    return *this;
  }

  inline T& operator[] (const int& index)
  {
      return index == 0 ? x : index == 1 ? y : z;
  }

  inline void invert () {
    x = -x;
    y = -y;
    z = -z;
  }

  VECTOR3_CLASS<T> inline __attribute__((always_inline))  unit() {

    VECTOR3_CLASS<T> norm;
    int32_t length = sqrt16(x*x+y*y+z*z);
    if (length != 0) {
      norm.x = (x*255)/length;
      norm.y = (y*255)/length;
      norm.z = (z*255)/length;
    } else {
      norm.x = 0;
      norm.y = 0;
      norm.z = 0;
    }
    return norm;

  }

};


typedef VECTOR3_CLASS<int32_t> VECTOR3;
typedef VECTOR3_CLASS<int32_t> VECTOR3_8;

inline VECTOR3 abs(const VECTOR3& v) {
    VECTOR3 temp;
    temp.x = abs(v.x);
    temp.y = abs(v.y);
    temp.z = abs(v.z);
    return temp;
  }

struct Y_BUF {
  VECTOR3 position;
  VECTOR3 ratio;
};
Y_BUF* y_buffer2[MATRIX_HEIGHT];


static inline __attribute__ ((always_inline)) CRGB gamma8_decode(const VECTOR3& value) {
  CRGB rgb( gamma8_d[_clamp8(value.x)], gamma8_d[_clamp8(value.y)], gamma8_d[_clamp8(value.z)] );
  return rgb;
}


static inline __attribute__ ((always_inline)) void blendXY(CRGB crgb_object[], const VECTOR3& point, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255) {
  CRGB rgb;
  hsv2rgb_rainbow(CHSV(hue,sat,val),rgb);
  blendXY(crgb_object, point.x, point.y, rgb);
}


struct alpha_pixel {
    uint16_t r = 0;
    uint16_t g = 0;
    uint16_t b = 0;
    uint16_t a = 0;
    uint16_t cnt = 0;
};

static inline __attribute__ ((always_inline)) void blendXY_RGBA(alpha_pixel ap[], const int32_t& xpos, const int32_t& ypos, const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a) {
  
  //find the base x and y positions
  //add 2 pixels before division to avoid rounding errors at 0 (between -1 and 0)
  int x = xpos+512;
  int y = ypos+512;
  x /= 256;
  y /= 256;
  x -= 2;
  y -= 2;

  int xval = (xpos + 512) % 256; //amount of light right
  int yval = (ypos + 512) % 256; //amount of light bottom
  xval = ease8InOutApprox(xval);
  yval = ease8InOutApprox(yval);
  int x2val = 255 - xval; //amount of light left
  int y2val = 255 - yval; //amount of light top
  
  uint8_t l1 = 255; //top left
  uint8_t l2 = 255; //top right
  uint8_t l3 = 255; //bottom right
  uint8_t l4 = 255; //bottom left

  uint8_t l1a = (l1*a)/255;
  uint8_t l2a = (l2*a)/255;
  uint8_t l3a = (l3*a)/255;
  uint8_t l4a = (l4*a)/255;
  
  ap[XY(x,y)].r += (r*l1)/255;
  ap[XY(x,y)].g += (g*l1)/255;
  ap[XY(x,y)].b += (b*l1)/255;
  ap[XY(x,y)].a += (a*l1)/255;
  ap[XY(x,y)].cnt++;
  if (x < MATRIX_WIDTH-1) {
    ap[XY(x+1,y)].r += (r*l2)/255;
    ap[XY(x+1,y)].g += (g*l2)/255;
    ap[XY(x+1,y)].b += (b*l2)/255;
    ap[XY(x+1,y)].a += (a*l2)/255;
    ap[XY(x+1,y)].cnt++;
  }
  
  if (x < MATRIX_WIDTH-1 && y < MATRIX_HEIGHT-1) {
    ap[XY(x+1,y+1)].r += (r*l3)/255;
    ap[XY(x+1,y+1)].g += (g*l3)/255;
    ap[XY(x+1,y+1)].b += (b*l3)/255;
    ap[XY(x+1,y+1)].a += (a*l3)/255;
    ap[XY(x+1,y+1)].cnt++;
  }
  if (y < MATRIX_HEIGHT-1) {
    ap[XY(x,y+1)].r += (r*l4)/255;
    ap[XY(x,y+1)].g += (g*l4)/255;
    ap[XY(x,y+1)].b += (b*l4)/255;
    ap[XY(x,y+1)].a += (a*l4)/255;
    ap[XY(x,y+1)].cnt++;
  }

//  leds[XY(x,y)] += CHSV(hue,sat,(x2val*y2val*1L*val)/(255L*255L));
//  leds[XY(x+1,y)] += CHSV(hue,sat,(xval*y2val*1L*val)/(255L*255L));
//  leds[XY(x,y+1)] += CHSV(hue,sat,(x2val*yval*1L*val)/(255L*255L));
//  leds[XY(x+1,y+1)] += CHSV(hue,sat,(xval*yval*1L*val)/(255L*255L));
}

//https://stackoverflow.com/questions/1903954/is-there-a-standard-sign-function-signum-sgn-in-c-c
template <typename T> int sgn(const T& val) {
    return (T(0) < val) - (val < T(0));
}

static inline __attribute__ ((always_inline)) void swap_coords(int32_t& x1, int32_t& y1, int32_t& x2, int32_t& y2, int32_t& dist) {
  int32_t tempx = x1;
  int32_t tempy = y1;
  x1 = x2;
  y1 = y2;
  x2 = tempx;
  y2 = tempy;
  dist = -dist;
}

static inline __attribute__ ((always_inline)) void draw_line_ybuffer(const int32_t& x1i, const int32_t& y1i, const int32_t& x2i, const int32_t& y2i) {
  int32_t x1 = (x1i+128)/256;
  int32_t y1 = (y1i+128)/256;
  int32_t x2 = (x2i+128)/256;
  int32_t y2 = (y2i+128)/256;
  int32_t x_dist = x2-x1;
  int32_t y_dist = y2-y1;
  int32_t ax_dist = abs(x_dist);
  int32_t ay_dist = abs(y_dist);
  uint32_t err = 0;

  if (ax_dist > ay_dist) {
    //draw horizontally
    if (x1 > x2) {
      swap_coords(x1,y1,x2,y2,y_dist);
    }
    int32_t step = sgn(y_dist);
    while (x1 <= x2) {
      if (y1 >= 0 && y1 < MATRIX_HEIGHT) {
        y_buffer_min = _min(y1,y_buffer_min);
        y_buffer_max = _max(y1,y_buffer_max);
        y_buffer[y1][0] = _min(y_buffer[y1][0], x1);
        y_buffer[y1][1] = _max(y_buffer[y1][1], x1);
      }
      //drawXY(leds,x1,y1,0,0,255);
      x1++;;
      err += ay_dist;
      if (err >= ax_dist) {
        y1 += step;
        err -= ax_dist;
      }
    }
  } else {
    //draw vertically
    if (y1 > y2) {
      swap_coords(x1,y1,x2,y2,x_dist);
    }
    int32_t step = sgn(x_dist);
    while (y1 <= y2) {
      if (y1 >= 0 && y1 < MATRIX_HEIGHT) {
        y_buffer_min = _min(y1,y_buffer_min);
        y_buffer_max = _max(y1,y_buffer_max);
        y_buffer[y1][0] = _min(y_buffer[y1][0], x1);
        y_buffer[y1][1] = _max(y_buffer[y1][1], x1);
      }
      //drawXY(leds,x1,y1,0,0,255);
      y1++;
      err += ax_dist;
      if (err >= ay_dist) {
        x1 += step;
        err -= ay_dist;
      }
    }
  }


}

static inline __attribute__ ((always_inline)) void draw_line_ybuffer(VECTOR3& a, VECTOR3& b) {
  draw_line_ybuffer(a.x, a.y, b.x, b.y);
}



static inline __attribute__ ((always_inline)) void swap_coords(VECTOR3& a, VECTOR3& a_rgb, VECTOR3& b, VECTOR3& b_rgb, VECTOR3& dist) {
  VECTOR3 temp = a;
  VECTOR3 temp_rgb = a_rgb;
  a = b;
  a_rgb = b_rgb;
  b = temp;
  b_rgb = temp_rgb;
  dist.x = -dist.x;
  dist.y = -dist.y;
  dist.z = -dist.z;
}


static inline __attribute__((always_inline)) void iterate(VECTOR3& a, const VECTOR3& step, const VECTOR3& dist, VECTOR3& err, const int32_t& target_dist ) {

  if (target_dist > 0) {
            
    err += dist;
    while (err.x >= target_dist) {
      a.x += step.x;
      err.x -= target_dist;
    }

    while (err.y >= target_dist) {
      a.y += step.y;
      err.y -= target_dist;
    }

    while (err.z >= target_dist) {
      a.z += step.z;
      err.z -= target_dist;
    }

  }

}

static inline __attribute__ ((always_inline)) void draw_line_ybuffer(VECTOR3 a, VECTOR3 a_rgb, VECTOR3 b, VECTOR3 b_rgb) {

  a += 128;
  b += 128;
  a /= 256;
  b /= 256;
  VECTOR3 dist = b-a;
  VECTOR3 a_dist = abs(dist);

  VECTOR3 dist_rgb = b_rgb-a_rgb;
  VECTOR3 a_dist_rgb = abs(dist_rgb);

  VECTOR3 err(0,0,0);
  VECTOR3 step(0,0,0);

  VECTOR3 err_rgb(0,0,0);
  VECTOR3 step_rgb(0,0,0);


  if (a_dist.x > a_dist.y) {
    //draw horizontally
    if (a.x > b.x) {
      swap_coords(a,a_rgb,b,b_rgb,dist);
      dist_rgb.x = -dist_rgb.x;
      dist_rgb.y = -dist_rgb.y;
      dist_rgb.z = -dist_rgb.z;
    }
    step.y = sgn(dist.y);
    step.z = sgn(dist.z);
    step_rgb.x = sgn(dist_rgb.x);
    step_rgb.y = sgn(dist_rgb.y);
    step_rgb.z = sgn(dist_rgb.z);
    while (a.x <= b.x) {
      if (a.y >= 0 && a.y < MATRIX_HEIGHT) {
        y_buffer_min = _min(a.y,y_buffer_min);
        y_buffer_max = _max(a.y,y_buffer_max);
        //TODO: take into account a.z when a.x is equal, if necessary
        if (a.x < y_buffer2[a.y][0].position.x) {
          y_buffer2[a.y][0].position = a;
          y_buffer2[a.y][0].ratio = a_rgb;
        }
        if (a.x > y_buffer2[a.y][1].position.x) {
          y_buffer2[a.y][1].position = a;
          y_buffer2[a.y][1].ratio = a_rgb;
        }
      }
      //drawXY(leds,a.x,a.y,0,0,255);
      a.x++;
      
      iterate(a, step, a_dist, err, a_dist.x);
      
      iterate(a_rgb, step_rgb, a_dist_rgb, err_rgb, a_dist.x);

    }
  } else {
    //draw vertically
    if (a.y > b.y) {
      swap_coords(a,a_rgb,b,b_rgb,dist);
      dist_rgb.x = -dist_rgb.x;
      dist_rgb.y = -dist_rgb.y;
      dist_rgb.z = -dist_rgb.z;
    }
    step.x = sgn(dist.x);
    step.z = sgn(dist.z);
    step_rgb.x = sgn(dist_rgb.x);
    step_rgb.y = sgn(dist_rgb.y);
    step_rgb.z = sgn(dist_rgb.z);
    while (a.y <= b.y) {
      if (a.y >= 0 && a.y < MATRIX_HEIGHT) {
        y_buffer_min = _min(a.y,y_buffer_min);
        y_buffer_max = _max(a.y,y_buffer_max);
        if (a.x < y_buffer2[a.y][0].position.x) {
          y_buffer2[a.y][0].position = a;
          y_buffer2[a.y][0].ratio = a_rgb;
        }
        if (a.x > y_buffer2[a.y][1].position.x) {
          y_buffer2[a.y][1].position = a;
          y_buffer2[a.y][1].ratio = a_rgb;
        }
      }
      //drawXY(leds,a.x,a.y,0,0,255);

      a.y++;

      iterate(a, step, a_dist, err, a_dist.y);

      iterate(a_rgb, step_rgb, a_dist_rgb, err_rgb, a_dist.y);


    }
  }


}

//DRAW LINE FINE


static void draw_line_fine_base(CRGB crgb_object[], const VECTOR3& a, const VECTOR3& b, const CRGB& rgb, const uint8_t& val = 255, const uint8_t& val2 = 255, const bool& trim = false, const bool& ignore_z = true, const bool& wide_fill = true) {
  
  int32_t z_depth = a.z;

  //add one pixel to compensate for rounding errors between -1 and 0
  int32_t x1 = a.x+256;
  int32_t y1 = a.y+256;
  int32_t x2 = b.x+256;
  int32_t y2 = b.y+256;
  
  //avoid vertical and horizontal lines by fudging a bit
  if (x1 == x2 ) {
    x1++;
  }

  if (y1 == y2 ) {
    y2++;
  }
  bool flip = false;
  if ( abs(x1 - x2) > abs(y1 - y2) ) {
    //calculate horizontally
    //flip coordinates if necessary to calculate always in the positive direction
    if (x1 > x2) {
      int tempx = x1;
      int tempy = y1;
      x1 = x2;
      y1 = y2;
      x2 = tempx;
      y2 = tempy;
      flip = true;
    }
  } else {
    //calculate vertically
    //flip coordinates if necessary to calculate always in the positive direction
    if (y1 > y2) {
      int tempx = x1;
      int tempy = y1;
      x1 = x2;
      y1 = y2;
      x2 = tempx;
      y2 = tempy;
      flip = true;
    }
  }

  //extend ends to an LED integer
  int x1_led;
  int x1_r;
  int y1_led;
  int y1_r;
  int x2_led;
  int x2_r;
  int y2_led;
  int y2_r;
  
  if (trim) {
  
    x1_led = (x1+128)>>8;
    x1_r = ((x1_led+1)<<8) - (x1+128);
    x2_led = (x2-128+255)/256; //ceil
    x2_r = ((x2-128) - ((x2_led-1)<<8));


    y1_led = (y1+128)>>8;
    y1_r = ((y1_led+1)<<8) - (y1+128);
    y2_led = (y2-128+255)/256; //ceil
    y2_r = ((y2-128) - ((y2_led-1)<<8));
  
  } else {
    
    x1_led = x1>>8;
    x1_r = ((x1_led+1)<<8) - (x1);
    x2_led = (x2+255)/256; //ceil
    x2_r = ((x2) - ((x2_led-1)<<8));


    y1_led = y1>>8;
    y1_r = ((y1_led+1)<<8) - (y1);
    y2_led = (y2+255)/256; //ceil
    y2_r = ((y2) - ((y2_led-1)<<8));
  
  }

  int32_t x_dist = x2 - x1;
  int32_t y_dist = y2 - y1;
  int32_t x_step = (x_dist*256)/y_dist;
  int32_t y_step = (y_dist*256)/x_dist;
  if (abs(x1 - x2) > abs(y1 - y2)) {
    //calculate horizontally
    
    int y_start = y1 + ((x1_led*256 - x1)*y_step)/256;
    for (int i = (x1_led); i <= (x2_led); i++) {
      //if ((i != x1_led && i != x2_led) || !trim) {
      if (true) {
        uint8_t v1 = val;
        int pos = i - x1_led;
        if (flip) {
          pos = x2_led - i;
        }
        uint8_t v2;
        if (x2_led-x1_led != 0) {
          v2 = ((val2-val)*(pos))/(x2_led-x1_led);
        } else {
          v2 = (val2-val)*pos;
        }
        int y = y_start;
        int Hy = (y+255)/256; //ceil
        int Ly = y/256;
        uint progress = abs(y - Hy*256);
        uint8_t b = _min( _max(progress, 0), 255);
        uint8_t b2 = 255-b;
        if (i == x1_led) {
          b = (b*x1_r)>>8;
          b2 = (b2*x1_r)>>8;
        }
        if (i == x2_led) {
          b = (b*x2_r)>>8;
          b2 = (b2*x2_r)>>8;
        }

        //subtract the pixel we added to compensate for rounding errors between -1 and 0
        Hy--;
        Ly--;
        i--;

        drawXYZ2(crgb_object,i, Hy, z_depth, rgb, ((b2*v1)>>8) + ((b2*v2)>>8), ignore_z );
        drawXYZ2(crgb_object,i, Ly, z_depth, rgb, ((b *v1)>>8) + ((b *v2)>>8), ignore_z );
       
        //record stuff in our x and y buffers for other functions to use
        if (i >= 0 && i < MATRIX_WIDTH) {

          if (!wide_fill) {
            int temp = Hy;
            Ly = Hy;
            Hy = Ly;
          }

          if (Hy <= x_buffer[i][0]) {
            x_buffer[i][0] = _min(x_buffer[i][0], Hy);

            y_buffer_min = _min(y_buffer_min,Hy);

            if (Hy >= 0 && Hy < MATRIX_HEIGHT) {
              y_buffer[Hy][0] = _min(y_buffer[Hy][0], i);
              y_buffer[Hy][1] = _max(y_buffer[Hy][1], i);
            }

          }

          if (Ly >= x_buffer[i][1]) {
            x_buffer[i][1] = _max(x_buffer[i][1], Ly);

            y_buffer_max = _max(y_buffer_max,Ly);

            if (Ly >= 0 && Ly < MATRIX_HEIGHT) {
              y_buffer[Ly][0] = _min(y_buffer[Ly][0], i);
              y_buffer[Ly][1] = _max(y_buffer[Ly][1], i);

            }

          }

        } else {

          if (Ly >= 0 && Ly < MATRIX_HEIGHT) {
            y_buffer[Ly][0] = _min(y_buffer[Ly][0], i);
            y_buffer[Ly][1] = _max(y_buffer[Ly][1], i);

          } 
          if (Hy >= 0 && Hy < MATRIX_HEIGHT) {
            y_buffer[Hy][0] = _min(y_buffer[Hy][0], i);
            y_buffer[Hy][1] = _max(y_buffer[Hy][1], i);
          }
         
        }
        //drawXYZ(crgb_object,i, Hy, z_depth, hue, sat, (b2*v1)/256 + (b2*v2)/256 );
        //drawXYZ(crgb_object,i, Ly, z_depth, hue, sat, (b *v1)/256 + (b *v2)/256 );
      }
      i++;
      y_start += y_step;
    }

    
  } else {
    //calculate vertically

    int32_t x_start = x1 + ((y1_led*256-y1)*x_step)/256;
    for (int i = (y1_led); i <= (y2_led); i++) {
      uint8_t v1 = val;
      int pos = i - y1_led;
      if (flip) {
        pos = y2_led - i;
      }
      uint8_t v2;
      if (y2_led-y1_led != 0) {
        v2 = ((val2-val)*(pos))/(y2_led-y1_led);
      } else {
        v2 = (val2-val)*pos;
      }
      int x = x_start;
      int Hx = (x+255)/256; //ceil
      int Lx = x/256;
      uint progress = abs(x - Hx*256);
      uint8_t b = _min( _max(progress, 0), 255);
      uint8_t b2 = 255-b;
      if (i == y1_led) {
        b  = (b*y1_r)>>8;
        b2 = (b2*y1_r)>>8;
      }
      if (i == y2_led) {
        b  = (b*y2_r)>>8;
        b2 = (b2*y2_r)>>8;
      }


      //subtract the pixel we added to compensate for rounding errors between -1 and 0
      Hx--;
      Lx--;
      i--;

      drawXYZ2(crgb_object,  Hx, i, z_depth, rgb, ((b2*v1)>>8) + ((b2*v2)>>8), ignore_z );
      drawXYZ2(crgb_object,  Lx, i, z_depth, rgb, ((b *v1)>>8) + ((b *v2)>>8), ignore_z );
  
      if (!wide_fill) {
        int temp = Hx;
        Lx = Hx;
        Hx = Lx;
      }
      
      //record stuff in our x and y buffers for other functions to use
      if (i >= 0 && i < MATRIX_HEIGHT) {
        y_buffer_min = _min(i,y_buffer_min);
        y_buffer_max = _max(i,y_buffer_max);
        y_buffer[i][0] = _min(y_buffer[i][0], Hx);
        y_buffer[i][1] = _max(y_buffer[i][1], Lx);
      }

      if (Lx >= 0 && Lx < MATRIX_WIDTH) {
          x_buffer[Lx][0] = _min(x_buffer[Lx][0], i);
          x_buffer[Lx][1] = _max(x_buffer[Lx][1], i);
      }
      if (Hx >= 0 && Hx < MATRIX_WIDTH) {
          x_buffer[Hx][0] = _min(x_buffer[Hx][0], i);
          x_buffer[Hx][1] = _max(x_buffer[Hx][1], i);
      }
      
      //drawXYZ(crgb_object,  ceil(x_start/256L), i, z_depth, h, s, (b2*v1)/256 + (b2*v2)/256 );
      //drawXYZ(crgb_object, floor(x_start/256L), i, z_depth, h, s, (b *v1)/256 + (b *v2)/256 );


      //add the pixel we subtracted to compensate for rounding errors between -1 and 0
      i++;
      x_start += x_step;
    }
    
  }

} //draw_line_fine()

void y_buffer_fill(CRGB crgb_object[], const CRGB& rgb, const int32_t& z_depth) {

        int32_t x_min = MATRIX_WIDTH;
        int32_t x_max = -1;

        int32_t x_cnt = 0;
        int32_t x_min_avg = 0;
        int32_t x_max_avg = 0;
        
        for (int i = _max(y_buffer_min, 0); i <= _min(y_buffer_max,MATRIX_HEIGHT-1); i++) {

          x_min_avg += _max(y_buffer[i][0], 0);
          x_max_avg += _min(y_buffer[i][1], MATRIX_WIDTH-1);
          x_cnt++;
          
          if (y_buffer[i][0] < x_min) {
            x_min = y_buffer[i][0];
          }
          if (y_buffer[i][1] > x_max) {
            x_max = y_buffer[i][1];
          }
        }

        int32_t y_min = MATRIX_HEIGHT;
        int32_t y_max = -1;

        int32_t y_cnt = 0;
        int32_t y_min_avg = 0;
        int32_t y_max_avg = 0;

        for (int i = _max(x_min, 0); i <= _min(x_max,MATRIX_WIDTH-1); i++) {

          y_min_avg += _max(x_buffer[i][0],0);
          y_max_avg += _min(x_buffer[i][1],MATRIX_HEIGHT-1);
          y_cnt++;

          if (x_buffer[i][0] < y_min) {
            y_min = x_buffer[i][0];
          }
          if (x_buffer[i][1] > y_max) {
            y_max = x_buffer[i][1];
          }
        }


        int32_t x_center = x_min;
        int32_t y_center = y_min;

        if (x_cnt != 0) {
          x_center = (x_min_avg + x_max_avg + x_cnt) / (x_cnt*2);
        }

        if (y_cnt != 0) {
          y_center = (y_min_avg + y_max_avg + y_cnt) / (y_cnt*2);
        }

        //drawXYZ(leds, x_center, y_center, 100000, CRGB(255,255,255));

  for (int y = _max(y_buffer_min, 0); y <= _min(y_buffer_max,MATRIX_HEIGHT-1); y++) {
    if (y_buffer[y][0] <= y_buffer[y][1]) {
      
        int32_t x_min2 = y_buffer[y][0];
        int32_t x_max2 = y_buffer[y][1];
        
        
        int32_t x_dist = _max(x_center-x_min2, x_max2-x_center);

      for (int x = y_buffer[y][0]; x <= y_buffer[y][1]; x++) {

        int32_t y_min2 = x_buffer[x][0];
        int32_t y_max2 = x_buffer[x][1];

        int32_t x_pos = (x - x_min2);
        int32_t y_pos = (y - y_min2);

        int32_t y_dist = _max(y_center-y_min2, y_max2-y_center);

        uint8_t x_amount = 255;

        if (x_dist != 0) {
          x_amount = _clamp8(255-(abs(x_dist-x_pos)*255)/x_dist);
          x_amount = gamma8_encode(x_amount);
        }
        uint8_t y_amount = 255;
        if (y_dist != 0) {
          y_amount = _clamp8(255-(abs(y_dist-y_pos)*255)/y_dist);
          y_amount = gamma8_encode(y_amount);
        }
        drawXY_blend_gamma( crgb_object, x, y, z_depth, rgb,  (x_amount*y_amount)/255  );
        //drawXYZ(crgb_object, x, y, z_depth, rgb);
      }
    }
  }
}

static inline __attribute__ ((always_inline)) void draw_line_fine(CRGB crgb_object[], const int32_t& x1, const int32_t& y1, const int32_t& x2, const int32_t& y2, CRGB& rgb, const int& z_depth = -10000, const uint8_t& val = 255, const uint8_t& val2 = 255, const bool& trim = false, const bool& ignore_z = true, const bool wide_fill = true) {
  VECTOR3 a(x1,y1,z_depth);
  VECTOR3 b(x2,y2,z_depth);
  draw_line_fine_base(crgb_object, a, b, rgb, val, val2, trim, ignore_z, wide_fill);
}


static inline __attribute__ ((always_inline)) void draw_line_fine(CRGB crgb_object[], const int32_t& x1, const int32_t& y1, const int32_t& x2, const int32_t& y2, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255, const int& z_depth = -10000, const uint8_t& val2 = 255, const bool& trim = false) {
  CRGB rgb = CHSV(hue,sat,255);
  draw_line_fine(crgb_object, x1, y1, x2, y2, rgb, z_depth, val, val2, trim);
}


static inline __attribute__ ((always_inline)) void draw_line_fine(CRGB crgb_object[], const VECTOR3& a, const VECTOR3& b, CRGB& rgb, const int& z_depth = -10000, const uint8_t& val = 255, const uint8_t& val2 = 255, const bool& trim = false, const bool& ignore_z = true, const bool& wide_fill = true) {
  draw_line_fine(crgb_object, a.x, a.y, b.x, b.y, rgb, z_depth, val, val2, trim, ignore_z, wide_fill);
}


//DRAW CURVE

//draw a curve by simultaneously shortening and rotating the line segment vectors

static inline __attribute__ ((always_inline)) void matt_curve(int32_t coordinate_array[][2], const size_t& len, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255, const bool& flipXY = false, const bool& closedShape = false, const bool& extraSmooth = false) {

  //a variable to store the angle for segment 2 from the previous pass (which will become segment 1 of the current pass)
  //we must blend the curves together to make one smooth continuous curve
  float last_a2b;

  //a variable to store the angle and length of the previous segment
  float last_exp; 
  float last_len;

  int starting_point = 0;
  if (closedShape) {
    starting_point = -1;
  }
  int ending_point = len-2;
  if (closedShape) {
    ending_point = len;
  }

  for (int i = starting_point;i<ending_point;i++) {
    int i0 = i;
    if (i0 == -1) {
      i0 = len-1;
    }
    int i1 = (i+1) % len;
    int i2 = (i+2) % len;


    //calculate three points at a time (in other words: two line segments)

    int32_t& x0 = coordinate_array[i0][0];
    int32_t& y0 = coordinate_array[i0][1];
       //segment 1 between these points
    int32_t& x1 = coordinate_array[i1][0];
    int32_t& y1 = coordinate_array[i1][1];
       //segment 2 between these points
    int32_t& x2 = coordinate_array[i2][0];
    int32_t& y2 = coordinate_array[i2][1];

    //angle of first segment
    float a0  = atan2(x1 - x0, y1 - y0);
    
    //normalize angle to the range: -PI to PI
    if (a0 > PI) {
      a0 -= 2.f*PI;
    } else if (a0 < -PI) {
      a0 += 2.f*PI;
    }

    //angle of second segment
    float a0b  = atan2(x1 - x2, y1 - y2);
    
    //normalize angle
    if (a0b > PI) { 
      a0b -= 2.f*PI;
    } else if (a0b < -PI) {
      a0b += 2.f*PI;
    }

    //figure out our orientation so that we can accurately add or subtract PI/2 (90 degrees)
    int dir = 1;
    float diff = a0-a0b;
    
    if(diff < -PI)
        diff += PI;
    if(diff > PI)
        diff -= PI;

    if (diff < 0) {  
      dir = -1;
    }
    
    //calculate the angle of the "normal line" for the two segments (perpendicular to the angle halfway between the two segments)
    float a4 = (a0+a0b)/2.f+dir*PI/2.f;
    
    if (a4 > PI) { //normalize angle
      a4 -= 2.f*PI;
    } else if (a4 < -PI) {
      a4 += 2.f*PI;
    }
    
    //the angular difference between line 1 and the "normal line"
    //we will sweep through this angle to draw our curve (while simultaneously shortening the vector)
    float a2 = a0-a4;
    
    //normalize angle
    if (a2 > PI) { 
      a2 -= 2.f*PI;
    } else if (a2 < -PI) {
      a2 += 2.f*PI;
    }

    //the angular difference between segment 2 and the "normal line"
    //we will sweep through this angle to draw our curve (while simultaneously shortening the vector)
    float a2b = a0b-(a4+PI); 

    //normalize angle
    if (a2b > PI) {
      a2b -= 2.f*PI;
    } else if (a2b < -PI) {
      a2b += 2.f*PI;
    }

    //time from 0-256 (8-bit equivalent of 0.0-1.0)
    int stp = 0; 
    float last_x;
    float last_y;
    float last_xb;
    float last_yb;

    //draw the curve *later* (sharper) for small angles
    //90 degrees = linear curve
    //45 degrees = exponential curve 
    //0 and 180 degrees = no curve
    
    float peepee0 = fabs(0-a2); //calculate the angle difference between the two segments: 0 = no change (0 and 180 degrees), 1 = max change (90 degrees)
    float peepee1 = fabs(0-a2b); //calculate the angle difference between the two segments: 0 = no change (0 and 180 degrees), 1 = max change (90 degrees)

    float exp0=sin(peepee0*2);
    float exp1=sin(peepee1*2);

    //draw the curve *later* (sharper) for short segments
    //long = linear curve
    //short = exponential curve
    //really short = no curve

    float len0 = _min((x0-x1) * (x0-x1)/256.f + (y0-y1) * (y0-y1)/256.f, 255)/255.f;
    float len1 = _min((x1-x2) * (x1-x2)/256.f + (y1-y2) * (y1-y2)/256.f, 255)/255.f;

    
    float len = _min(len0,len1);
    len = _max((len-.5f)*2,0);
    float exp = _min(exp0,exp1);
    float w = exp*len;

    float len2 = _min(last_len,len0);
    len2 = _max((len2-.5f)*2,0);
    float exp2 = _min(last_exp,exp0);
    float w2 = exp2*len2;


    //do not draw the first segment for closed shapes
    while(stp <= 256 && !(closedShape && i == -1)) {

      
      int stp_b = stp;
      if (extraSmooth) {
        for (int j = 0; j < 4; j++) {
          stp_b = ease8In(stp_b);
        }
        stp_b = (stp*w + stp_b*(1.f-w));
      }

      //our current offset angle for segment 1
      float angle = (a2*stp_b)/256.f;

      //calculate the angle for segment 1
      float x = ( cos(angle)*(x0-x1) - sin(angle)*(y0-y1) );
      float y = ( sin(angle)*(x0-x1) + cos(angle)*(y0-y1) );

      //calculate the length for segment 1
      x *= (256-stp)/256.f;
      y *= (256-stp)/256.f;

      //translate the vector to its original x,y coordinates
      x += x1;
      y += y1;

      //now we must blend our value with the previous pass
      //do not blend the first segment when drawing non-closed shapes
      if ((i > 0 || closedShape)) {
        
        //the previous pass is drawn in reverse (x1,x2 in the first pass becomes x0,x1 in the second pass)
        int stp2 = 256-stp;

        int stp2_b = stp2;
        if (extraSmooth) {
          for (int j = 0; j < 4; j++) {
            stp2_b = ease8In(stp2_b);
          }
          stp2_b = (stp2*w2 + stp2_b*(1.f-w2));
        }
        
        //our angle offset for segment 2 from the previous pass (which is segment 1 of the current pass)
        float angle2 = (last_a2b*stp2_b)/256.f; 

        //calculate the angle
        float xl = ( cos(angle2)*(x1-x0) - sin(angle2)*(y1-y0) );
        float yl = ( sin(angle2)*(x1-x0) + cos(angle2)*(y1-y0) );

        //calculate the length
        xl *= (stp)/256.f;
        yl *= (stp)/256.f;

        //translate our new vector back to original coordinates
        xl += x0;
        yl += y0;

        //debug points
        //blendXY(leds,xl,yl,96);
        //blendXY(leds,x,y,160);

        //blend our two passes together, slowly fading from one to the other

        x = (x*(stp))/256.f + (xl*stp2)/256.f;
        y = (y*(stp))/256.f + (yl*stp2)/256.f;


        
      }
      uint8_t asdf = 0;
      if (stp%64 == 0) {
        asdf = 1;
      }
      //draw our curve (starting from the second iteration, we need two points, duh)
      if (stp > 0) {
        //draw line between points
        if (flipXY) {
          draw_line_fine(leds, last_y, last_x, y, x, hue, sat, val, -10000, val, true);
        } else {

          draw_line_fine(leds, last_x, last_y, x, y, hue, sat, val, -10000, val, true);
        }
      }
      //record x,y for the next iteration
      last_x = x;
      last_y = y;
      
      //debug points
      //blendXY(leds,x0,y0,96);
      //blendXY(leds,x1,y1,160);
      //blendXY(leds, x, y);
      //blendXY(leds,x,y);

      //on our final pass we need to calculate the last segment without blending
      //do not draw this segment for non-closed shapes
      if (i == ending_point-1 && !closedShape) {
        //blendXY(leds,x2,y2,48);
        
        //our current offset angle for segment 2
        float angle2 = (a2b*stp)/256.f;

        //calculate the current angle
        float xb = ( cos(angle2)*(x2-x1) - sin(angle2)*(y2-y1) );
        float yb = ( sin(angle2)*(x2-x1) + cos(angle2)*(y2-y1) );

        //calculate the current length
        xb *= (256-stp)/256.f;
        yb *= (256-stp)/256.f;

        //move the new vector back to its original coordinates
        xb += x1;
        yb += y1;
        
        //blendXY(leds,xb,yb);

        //draw our curve
        if (stp > 0) {
          //draw line between points
          if (flipXY) {
            draw_line_fine(leds, last_yb, last_xb, yb, xb, hue, sat, val, -10000, val, true);
          } else {
            draw_line_fine(leds, last_xb, last_yb, xb, yb, hue, sat, val, -10000, val, true);
          }
        }

        //record our points for the next step in time
        last_xb = xb;
        last_yb = yb;
      }

      //iterate time
      stp+=32;
    }
    
    //store our segment 2 angle to blend with the next pair of line segments
    last_a2b = a2b;
    last_exp = exp1;
    last_len = len1;
  }
}


















//DRAW CURVE 8

//draw a curve by simultaneously shortening and rotating the line segment vectors

static inline __attribute__ ((always_inline)) void matt_curve8(CRGB crgb_object[], int32_t coordinate_array[][2], const size_t& len, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255, const bool& flipXY = false, const bool& closedShape = false, const bool& extraSmooth = false, const uint8_t& percentage = 255, const uint8_t& step_size = 32) {
  CRGB rgb = CHSV(hue, sat, val);
  //draw simple lines for step size of 255
  if (step_size == 255) {
    for (int i = 1; i < len; i++) {
      //draw_line_fine2(crgb_object, coordinate_array[i-1][0],coordinate_array[i-1][1],coordinate_array[i][0],coordinate_array[i][1],hue,sat,val);
      draw_line_fine(crgb_object,coordinate_array[i-1][0],coordinate_array[i-1][1],coordinate_array[i][0],coordinate_array[i][1], rgb, -10000, val, val, true);
    } 
    return;
  }

  //a variable to store the angle for segment 2 from the previous pass (which will become segment 1 of the current pass)
  //we must blend the curves together to make one smooth continuous curve
  int16_t last_a2b;

  //a variable to store the angle and length of the previous segment
  uint16_t last_w;

  int starting_point = -1;
  int ending_point = len-1;
  if (closedShape) {
    ending_point = len;
  }

  int total_length = ending_point - starting_point;

  for (int i = starting_point;i<ending_point;i++) {
    int i0=i;
    if (i == -1) {
      if (closedShape) {
        i0 = len-1;
      } else {
        i0 = 1;
      }
    }
    int i1 = (i+1) % len;
    int i2 = (i+2) % len;
    if (i == len-2 && !closedShape) {
      i2 = i0;
    }


    //calculate three points at a time (in other words: two line segments)

    int32_t& x0 = coordinate_array[i0][0];
    int32_t& y0 = coordinate_array[i0][1];
       //segment 1 between these points
    int32_t& x1 = coordinate_array[i1][0];
    int32_t& y1 = coordinate_array[i1][1];
       //segment 2 between these points
    int32_t& x2 = coordinate_array[i2][0];
    int32_t& y2 = coordinate_array[i2][1];

    //angle of first segment
    float a0f  = atan2(x1 - x0, y1 - y0);
    int16_t a0 = (a0f*65536*113)/(355*2);

    
    //angle of second segment
    float a0bf  = atan2(x1 - x2, y1 - y2);
    int16_t a0b = (a0bf*65536*113)/(355*2);
    

    //figure out our orientation so that we can accurately add or subtract PI/2 (90 degrees)
    int dir = (a0-a0b < 0) ? -1 : 1;
    
    //calculate the angle of the "normal line" for the two segments (perpendicular to the angle halfway between the two segments)
    int16_t a4 = (a0+a0b)/2+dir*16384;
    
    
    //the angular difference between line 1 and the "normal line"
    //we will sweep through this angle to draw our curve (while simultaneously shortening the vector)
    int16_t a2 = a0-a4;
    

    //the angular difference between segment 2 and the "normal line"
    //we will sweep through this angle to draw our curve (while simultaneously shortening the vector)
    int16_t a2b = a0b-(a4+32768); 


    //time from 0-256 (8-bit equivalent of 0.0-1.0)
    int stp = 0; 
    int32_t last_x;
    int32_t last_y;
    int32_t last_xb;
    int32_t last_yb;

    //modify the curve for smaller angles
    //180 degrees = full linear curve
    //90 degrees = smaller curve 
    //0 degrees = no curve
    
    int16_t exp0=a2-a2b; //angle difference between segments
    exp0 = abs(0 - exp0)/128; //convert to 0-255
    //exp0 = (exp0*exp0)/256; //change is weighted toward the smaller end
    exp0 = 256-exp0; //convert to 255-0
    //exp0 = (sin8(exp0/4)-128)*2; //apply easing with sin8() to get smooth changes at very small angles

    //modify the curve when segments differ greatly in length
    //small difference in length = full linear curve
    //medium difference in length = exponential curve
    //huge difference in length = no curve

    int32_t len0 = sqrt((x0-x1)*(x0-x1) + (y0-y1)*(y0-y1)); //length of first segment
    int32_t len1 = sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)); //length of second segment

    
    int32_t max_len = _max(len0,len1); //maximum length
    if (max_len == 0) {
      continue;
    }
    int32_t min_len = _min(len0,len1); //minimum length
    uint32_t len_d = (min_len << 8)/max_len; //ratio of length: 0 = infinite difference in length, 256 = equal lengths
    uint16_t w_x = len_d;
    //uint16_t w_x = (sin8(len_d/4)-128)*2; //apply easing with sin8() to get smooth changes when lengths differ greatly

    //here is the combined weight to apply to our curve
    //this takes into account two factors:
    // 1) the ratio of the lengths of the two segments
    // 2) the size of the angle between the two segments
    uint16_t w = _min((w_x*exp0)/256,256);
    w = 256-w;
    w = (w*w)>>8;
    w = (w*w)>>8;
    w = (w*w)>>8;
    w = (w*w)>>8;
    w = 256-w;

    //do not draw the first segment
    while(stp <= 256 && i != -1) {

      if ( ((i+1)*stp) / total_length > percentage) {
        return;
      }
      
      int stp_reverse = 256-stp;
      


      int stp_blergh = stp_reverse;
      if (extraSmooth) {
        stp_blergh = (stp_reverse*w)/256;
      }

      //our current offset angle for segment 1
      int16_t angle = (a2*stp)/65536;

      //calculate the angle for segment 1
      int16_t angle_c = cos8(angle)-128;
      int16_t angle_s = sin8(angle)-128;
      int32_t x = ( angle_c*(x0-x1) - angle_s*(y0-y1) )/128;
      int32_t y = ( angle_s*(x0-x1) + angle_c*(y0-y1) )/128;


      //calculate the length for segment 1
      x = (x*stp_blergh)/256;
      y = (y*stp_blergh)/256;

      //translate the vector to its original x,y coordinates
      x += x1;
      y += y1;

      //now we must blend our value with the previous pass
      //the previous pass is drawn in reverse (x1,x2 in the first pass becomes x0,x1 in the second pass)
      //do not blend the first segment when drawing non-closed shapes
      //if (i > -1 || closedShape) {
      if (true) {
        
        int stp_blergh2 = stp;
        if (extraSmooth) {
          stp_blergh2 = (stp*last_w)/256;
        }

        //our angle offset for segment 2 from the previous pass (which is segment 1 of the current pass)
        int16_t angle2 = (last_a2b*stp_reverse)/65536; 

        //calculate the angle
        int16_t angle2_c = cos8(angle2)-128;
        int16_t angle2_s = sin8(angle2)-128;
        int32_t xl = ( angle2_c*(x1-x0) - angle2_s*(y1-y0) )/128;
        int32_t yl = ( angle2_s*(x1-x0) + angle2_c*(y1-y0) )/128;


        //calculate the length
        xl = (xl*stp_blergh2)/256;
        yl = (yl*stp_blergh2)/256;

        //translate our new vector back to original coordinates
        xl += x0;
        yl += y0;

        //debug points
        //blendXY(leds,xl,yl,0, stp);
        //blendXY(leds,x,y,160, stp);

        //blend our two passes (curves) together, fade from one to the other
        //int stp_blend = ease8InOutQuad(ease8InOutQuad(_min(stp,255)));
        //int stp_blend = ease8InOutQuad(_min(stp,255));
        int stp_blend = _min(stp,255);
        int stp_blend2 = 255-stp_blend;
        x = ((x*stp_blend) + (xl*stp_blend2))/256;
        y = ((y*stp_blend) + (yl*stp_blend2))/256;


        
      }
      uint8_t asdf = 0;
      if (stp%64 == 0) {
        asdf = 1;
      }
      //draw our curve (starting from the second iteration, we need two points, duh)
      if (stp > 0) {
        //draw line between points
        if (flipXY) {
          //draw_line_fine2(crgb_object, last_y, last_x, y, x, hue, sat, val);
          draw_line_fine(crgb_object, last_y, last_x, y, x, rgb, -10000, val, val, true);
            //blendXY(leds, y, x, 0, 0, 255);
        } else {

          //draw_line_fine2(crgb_object, last_x, last_y, x, y, hue, sat, val);
          draw_line_fine(crgb_object, last_x, last_y, x, y, rgb, -10000, val, val, true);
            //blendXY(leds, x, y, 0, 0, 255);
        }
      }
      //record x,y for the next iteration
      last_x = x;
      last_y = y;
      
      //debug points
      //blendXY(leds,x0,y0,96);
      //blendXY(leds,x1,y1,160);
      //blendXY(leds, x, y);
      //blendXY(leds,x,y);

      //on our final pass we need to calculate the last segment without blending
      //do not draw this segment for closed shapes
      if (i == ending_point-1 && !closedShape && false) {
        //blendXY(leds,x2,y2,48);
        
        //our current offset angle for segment 2
        int16_t angle2 = (a2b*stp)/65536;
        int16_t angle2_s = sin8(angle2);
        int16_t angle2_c = cos8(angle2);
        //calculate the current angle
        int32_t xb = ( angle2_c*(x2-x1)/128 - angle2_s*(y2-y1)/128 );
        int32_t yb = ( angle2_s*(x2-x1)/128 + angle2_c*(y2-y1)/128 );

        //calculate the current length
        xb = (xb*(256-stp))/256;
        yb = (yb*(256-stp))/256;

        //move the new vector back to its original coordinates
        xb += x1;
        yb += y1;
        
        //blendXY(leds,xb,yb);

        //draw our curve
        if (stp > 0) {
          //draw line between points
          if (flipXY) {
            draw_line_fine(crgb_object, last_yb, last_xb, yb, xb, rgb, -10000, val, val, true);
            //blendXY(leds, yb, xb, 0, 0, 255);
          } else {
            draw_line_fine(crgb_object, last_xb, last_yb, xb, yb, rgb, -10000, val, val, true);
            //blendXY(leds, xb, yb, 0, 0, 255);
          }
        }

        //record our points for the next step in time
        last_xb = xb;
        last_yb = yb;
      }

      //iterate time
      stp+=step_size;
    }
    
    //store our segment 2 angle to blend with the next pair of line segments
    last_a2b = a2b;
    last_w = w;
  }
} //matt_curve8_base


static inline __attribute__ ((always_inline)) void matt_curve8(int32_t coordinate_array[][2], const size_t& len, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255, const bool& flipXY = false, const bool& closedShape = false, const bool& extraSmooth = false, const uint8_t& percentage = 255, const uint8_t& step_size = 32) {

  matt_curve8(leds, coordinate_array, len, hue, sat, val, flipXY, closedShape, extraSmooth, percentage, step_size);

}





uint8_t circle_angles[20][17];

static inline __attribute__ ((always_inline)) void reset_circle_angles() {
  for (int i = 0; i < 20; i++) {
    for (int j = 0; j < 16; j++) {
      circle_angles[i][j] = 255;
    }
  }
}

static inline __attribute__ ((always_inline)) void draw_circle_fine(const int32_t& x, const int32_t& y, const int32_t& r, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255, const int& ballnum = -1, const uint8_t& step_size = 16, const int32_t& z_depth = 0) {
  
  if(step_size == 255) {
    blendXY(leds, x, y, hue, sat, val);
    return;
  }

  int t0 = 0;
  int32_t xl2;
  int32_t yl2;
  //int32_t coords[16][2];
  //int32_t coord_pos = -1;
  while (t0 <= 256) {
    uint8_t t = t0;
    int32_t radius = r;
    if(ballnum > -1) {
      radius = r*circle_angles[ballnum][t/step_size]/256;
    }
    int32_t xl = ((cos8(t)-128)*radius)/128;
    int32_t yl = ((sin8(t)-128)*radius)/128;
    xl += x;
    yl += y;
    if (t0 > 0) {
      draw_line_fine(leds, xl, yl, xl2, yl2, hue, sat, val, z_depth, val, true);
      //coords[coord_pos][0] = xl2;
      //coords[coord_pos][1] = yl2;
    }
    xl2 = xl;
    yl2 = yl;
    //coord_pos++;
    t0+=step_size;
  }
  //matt_curve8(coords, 16, hue, sat, val, false, true, false,255,64);

  //blendXY(leds, x, y, hue, sat, val);
}



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













static inline __attribute__ ((always_inline)) void LED_show() {
  if (text_mask == 1) {
    for (int i = 0;i < NUM_LEDS;i++) {
      if (invert_mask == 0) {
        leds[i].nscale8(255-led_mask[i]);
      }
      if (invert_mask == 1) {
        leds[i].nscale8(led_mask[i]);
      }
    }
  } 
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

  y_buffer2[i][0].position.x = INT32_MAX;
  y_buffer2[i][1].position.x = INT32_MIN;

  }

  y_buffer_max = 0;
  y_buffer_min = MATRIX_HEIGHT-1;
  
}

static inline __attribute__ ((always_inline)) void reset_x_buffer() {
  for (int i = 0; i < MATRIX_WIDTH; i++) {
  x_buffer[i][0] = MATRIX_HEIGHT + 1;
  x_buffer[i][1] = -1;
  }
}

static inline __attribute__ ((always_inline)) void reset_z_buffer() {

  //clear the Z buffer
  for (int x = 0; x < MATRIX_WIDTH; x++) {
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
      z_buffer[x][y] = INT16_MIN;
    }
  }

}

static inline __attribute__ ((always_inline)) void LED_black() {
  
  //clear the string
  for (int y = 0; y < MATRIX_HEIGHT; y++) {
    memset8(&leds[y*MATRIX_WIDTH].r, 0, MATRIX_WIDTH*3);
  }

  reset_z_buffer();
  
}



static inline __attribute__ ((always_inline)) void reset_heightmap() {

  for (int y = 0; y < MATRIX_HEIGHT; y++) {
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      height_map[y][x] = 0;
    }
  }

}

//mixer algorithm from MurmerHash3
//provides a fast, repeatable, reasonably random number for a given input
//MurmurHash3 was written by Austin Appleby
static inline __attribute__ ((always_inline)) uint32_t fmix32 ( uint32_t h )
{
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;

  return h;
}

//create a datatype that uses 16 bits to store an 18-bit integer (-131072 to 131068)
typedef class cint18
{
  private:
      int16_t val = 0;

  public:

      cint18() {
      }

      #define CINT18_MULT 4
      
      //conversion to int
      inline operator int() {
          return val*CINT18_MULT;
      }

      //conversion from int
      inline cint18(const int& n) {
          val = n/CINT18_MULT;
      }

      //conversion from float
      inline cint18(const float& n) {
          val = n/CINT18_MULT;
      }

        //overload +=
        inline void operator+= (const int& rhs) {
          val = val + rhs/CINT18_MULT;
        }

        //overload -=
        inline void operator-= (const int& rhs) {
          val = val - rhs/CINT18_MULT;
        }


        //overload *=
        inline void operator*= (const int& rhs) {
          val = val * rhs;
        }


        //overload /=
        inline void operator/= (const int& rhs) {
          val = val / rhs;
        }


} cint18;

  