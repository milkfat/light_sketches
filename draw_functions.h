//various functions shared amongst lighting sketches


#ifndef _min
#define _min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef _max
#define _max(a,b) ((a)>(b)?(a):(b))
#endif


//make the heightmap one pixel wider in each dimension to avoid surface normal aberations along the edges 
#define HEIGHTMAP_WIDTH (MATRIX_WIDTH+2)
#define HEIGHTMAP_HEIGHT (MATRIX_HEIGHT+2)

bool button0_down = false;
bool button1_down = false;
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

        static uint16_t compress(float value)
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

        static float decompress(uint16_t value)
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

std::string display_text = "";
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
int16_t * height_map[HEIGHTMAP_WIDTH];


//uint8_t led_mask[NUM_LEDS];
uint8_t * led_mask;
//uint8_t led_mask2[NUM_LEDS];
uint8_t * led_mask2;

uint8_t default_color = 0;
uint8_t default_saturation = 255;
uint8_t default_value = 255;

uint8_t debug_scaler = 128;
uint8_t camera_scaler = 232;
uint8_t screen_scaler = 100;
float rotation_alpha = 0;
float rotation_beta = 90;
float rotation_gamma = 0;

int already_beat = 0;
int effect_beat = 0;
int growing = 0;


#define PI 355/113.f

int PI_m (const int num) {
    return (num*355)/113;
};

float PI_m (const float& num) {
    return (num*355)/113;
};

int PI_d (const int& num) {
    return (num*113)/355;
};

float PI_d (const float& num) {
    return (num*113)/355;
};

uint8_t ease8In (const uint8_t& stp) {
  //ease in
  return stp*stp/255.f;
}
uint8_t ease8Out (uint8_t stp) {
  //ease out
  stp = 255 - stp;
  stp = stp*stp/255;
  return 255 - stp;
}

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

int32_t cursor_position_x = 0;
int32_t cursor_position_y = 0;

//CIE 1931 luminescence scale (or some shit)
uint8_t cie (const uint8_t& a) {
  float y = (a*100.f)/255.f;
  if(y <= 8) {
    return y/903.3f;
  } else {
    float f = (y+16.f)/116.f;
    f = f*f*f;
    return f*255;
  }
}

#define NUM_POINTERS 6
POINTER pointers[NUM_POINTERS];


int adjust (const int& p) {
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
__attribute__ ((always_inline)) uint32_t XY(const int& x, const int& y) {
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

void drawXY_fine(CRGB crgb_object[], const int32_t& xpos, const int32_t& ypos, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255) {

  crgb_object[XY(xpos/256,ypos/256)] += CHSV(hue,sat,val);

}


void drawXY(CRGB crgb_object[], const int& x, const int& y, const uint8_t& hue, const uint8_t& sat, const uint8_t& val) {
  
  crgb_object[XY(x,y)] += CHSV(hue,sat,val);
  
}

void drawXY_fineRGB(CRGB crgb_object[], const int32_t& xpos, const int32_t& ypos, const uint8_t& r, const uint8_t& g, const uint8_t& b) {

  crgb_object[XY(xpos / 256,ypos / 256)] += CRGB(r,g,b);

}

void drawXY_RGB(CRGB crgb_object[], const int& x, const int& y, const uint8_t& r, const uint8_t& g, const uint8_t& b) {
  
  crgb_object[XY(x,y)] += CRGB(r,g,b);
  
}

//int y_buffer[MATRIX_HEIGHT][2]; //stores the min/max X values per Y so that we can fill between them
int * y_buffer[MATRIX_HEIGHT]; //stores the min/max X values per Y so that we can fill between them
int y_buffer_max = 0;
int y_buffer_min = MATRIX_HEIGHT-1;

//int z_buffer[MATRIX_WIDTH][MATRIX_HEIGHT];
int16_t * z_buffer[MATRIX_WIDTH];

void drawXY_blend(CRGB crgb_object[], const int& x, const int& y, CRGB& rgb, const uint8_t& brightness = 255) {
  
  nblend(crgb_object[XY(x,y)], rgb, brightness);

}

void drawXY_blend(CRGB crgb_object[], const int& x, const int& y, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255, const uint8_t& brightness = 255) {
  
  CRGB rgb;
  rgb = CHSV(hue,sat,val);
  drawXY_blend(crgb_object, x, y, rgb, brightness);

}

void drawXYZ(CRGB crgb_object[], const int32_t& x, const int32_t& y, const int32_t& z, CRGB& rgb) {
  
  if (y >= 0 && y < MATRIX_HEIGHT) {

    if (x >= 0 && x < MATRIX_WIDTH) {

      if (z > z_buffer[x][y]) {

        z_buffer[x][y] = z; 
        crgb_object[XY(x,y)] = rgb;
      }

    }

  }

}

void drawXYZ(CRGB crgb_object[], const int& x, const int& y, const int& z, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255) {
  
  if (y >= 0 && y < MATRIX_HEIGHT) {
    y_buffer[y][0] = _min(y_buffer[y][0], x);
    y_buffer[y][1] = _max(y_buffer[y][1], x);
  
  
    if (x >= 0 && x < MATRIX_WIDTH) {
      if (z > -10000 && z > z_buffer[x][y]) {
        z_buffer[x][y] = z; 
        crgb_object[XY(x,y)] = CHSV(hue,sat,val);
      } else if (z == -10000) {
        //crgb_object[XY(x,y)] += CHSV(hue,sat,val);
        drawXY_blend(crgb_object, x, y, hue, sat, val, 255);
      }
    }

  }

}


void drawXYZ2(CRGB crgb_object[], const int& x, const int& y, const int& z, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255, const uint8_t& brightness = 255) {
  
  drawXY_blend(crgb_object, x, y, hue, sat, val, brightness);

}

void drawXYZ2(CRGB crgb_object[], const int& x, const int& y, const int& z, CRGB& rgb, const uint8_t& brightness = 255) {
  
  drawXY_blend(crgb_object, x, y, rgb, brightness);

}


void blendXY(CRGB crgb_object[], const int32_t& xpos, const int32_t& ypos, CRGB& rgb) {
  
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
  
  uint8_t l1 = (x2val*y2val*1L)/(255L); //top left
  uint8_t l2 = (xval*y2val*1L)/(255L); //top right
  uint8_t l3 = (xval*yval*1L)/(255L); //bottom right
  uint8_t l4 = (x2val*yval*1L)/(255L); //bottom left
  
  uint16_t led1 = XY(x,y);
  uint16_t led2 = XY(x+1,y);
  uint16_t led3 = XY(x+1,y+1);
  uint16_t led4 = XY(x,y+1);

  crgb_object[led1].r = qadd8(crgb_object[led1].r, (rgb.r*l1)/255);
  crgb_object[led1].g = qadd8(crgb_object[led1].g, (rgb.g*l1)/255);
  crgb_object[led1].b = qadd8(crgb_object[led1].b, (rgb.b*l1)/255);
  if (x < MATRIX_WIDTH-1) {
    crgb_object[led2].r = qadd8(crgb_object[led2].r, (rgb.r*l2)/255);
    crgb_object[led2].g = qadd8(crgb_object[led2].g, (rgb.g*l2)/255);
    crgb_object[led2].b = qadd8(crgb_object[led2].b, (rgb.b*l2)/255);
  }
  
  if (x < MATRIX_WIDTH-1 && y < MATRIX_HEIGHT-1) {
    crgb_object[led3].r = qadd8(crgb_object[led3].r, (rgb.r*l3)/255);
    crgb_object[led3].g = qadd8(crgb_object[led3].g, (rgb.g*l3)/255);
    crgb_object[led3].b = qadd8(crgb_object[led3].b, (rgb.b*l3)/255);
  }
  if (y < MATRIX_HEIGHT-1) {
    crgb_object[led4].r = qadd8(crgb_object[led4].r, (rgb.r*l4)/255);
    crgb_object[led4].g = qadd8(crgb_object[led4].g, (rgb.g*l4)/255);
    crgb_object[led4].b = qadd8(crgb_object[led4].b, (rgb.b*l4)/255);
  }

//  leds[XY(x,y)] += CHSV(hue,sat,(x2val*y2val*1L*val)/(255L*255L));
//  leds[XY(x+1,y)] += CHSV(hue,sat,(xval*y2val*1L*val)/(255L*255L));
//  leds[XY(x,y+1)] += CHSV(hue,sat,(x2val*yval*1L*val)/(255L*255L));
//  leds[XY(x+1,y+1)] += CHSV(hue,sat,(xval*yval*1L*val)/(255L*255L));
}



void blendXY(CRGB crgb_object[], const int32_t& xpos, const int32_t& ypos, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255) {
  CRGB rgb;
  hsv2rgb_rainbow(CHSV(hue,sat,val),rgb);
  blendXY(crgb_object, xpos, ypos, rgb);
}

struct VECTOR3 {
  int32_t x;
  int32_t y;
  int32_t z;

  VECTOR3 () {}

  VECTOR3 (const int32_t& x_in, const int32_t& y_in, const int32_t& z_in): x(x_in), y(y_in), z(z_in) {}
  
  VECTOR3 operator + (VECTOR3 const &p_in) { 
    VECTOR3 p; 
    p.x = x + p_in.x; 
    p.y = y + p_in.y; 
    p.z = z + p_in.z; 
    return p; 
  } 
  
  VECTOR3 operator - (VECTOR3 const &p_in) { 
    VECTOR3 p; 
    p.x = x - p_in.x; 
    p.y = y - p_in.y; 
    p.z = z - p_in.z; 
    return p; 
  } 

  VECTOR3 operator / (int const &num) { 
    VECTOR3 p; 
    p.x = x / num; 
    p.y = y / num; 
    p.z = z / num; 
    return p; 
  } 


  //overload -=
  VECTOR3& operator-= (const VECTOR3& rhs) {
    this->x -= rhs.x;
    this->y -= rhs.y;
    this->z -= rhs.z;
    return *this;
  }
  //overload +=
  VECTOR3& operator+= (const VECTOR3& rhs) {
    this->x += rhs.x;
    this->y += rhs.y;
    this->z += rhs.z;
    return *this;
  }

  //overload +=
  VECTOR3& operator+= (const int& rhs) {
    this->x += rhs;
    this->y += rhs;
    this->z += rhs;
    return *this;
  }
  
  //overload /=
  VECTOR3& operator/= (const int& rhs) {
    this->x /= rhs;
    this->y /= rhs;
    this->z /= rhs;
    return *this;
  }

  int32_t& operator[] (const int& index)
  {
      return index == 0 ? x : index == 1 ? y : z;
  }

  void invert () {
    x = -x;
    y = -y;
    z = -z;
  }

  

};

VECTOR3 abs(const VECTOR3& v) {
    VECTOR3 temp;
    temp.x = abs(v.x);
    temp.y = abs(v.y);
    temp.z = abs(v.z);
    return temp;
  }

struct Y_BUF {
  VECTOR3 position;
  VECTOR3 normal;
  VECTOR3 rgb;
};
Y_BUF y_buffer2[MATRIX_HEIGHT][2];


void blendXY(CRGB crgb_object[], const VECTOR3& point, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255) {
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

void blendXY_RGBA(alpha_pixel ap[], const int32_t& xpos, const int32_t& ypos, const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a) {
  
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

void swap_coords(int32_t& x1, int32_t& y1, int32_t& x2, int32_t& y2, int32_t& dist) {
  int32_t tempx = x1;
  int32_t tempy = y1;
  x1 = x2;
  y1 = y2;
  x2 = tempx;
  y2 = tempy;
  dist = -dist;
}

void draw_line_ybuffer(const int32_t& x1i, const int32_t& y1i, const int32_t& x2i, const int32_t& y2i) {
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

void draw_line_ybuffer(VECTOR3& a, VECTOR3& b) {
  draw_line_ybuffer(a.x, a.y, b.x, b.y);
}



void swap_coords(VECTOR3& a, VECTOR3& a_norm, VECTOR3& a_rgb, VECTOR3& b, VECTOR3& b_norm, VECTOR3& b_rgb, VECTOR3& dist) {
  VECTOR3 temp = a;
  VECTOR3 temp_norm = a_norm;
  VECTOR3 temp_rgb = a_rgb;
  a = b;
  a_norm = b_norm;
  a_rgb = b_rgb;
  b = temp;
  b_norm = temp_norm;
  b_rgb = temp_rgb;
  dist.x = -dist.x;
  dist.y = -dist.y;
  dist.z = -dist.z;
}





void draw_line_ybuffer(VECTOR3 a, VECTOR3 a_norm, VECTOR3 a_rgb, VECTOR3 b, VECTOR3 b_norm, VECTOR3 b_rgb) {

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
      swap_coords(a,a_norm,a_rgb,b,b_norm,b_rgb,dist);
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
          y_buffer2[a.y][0].rgb = a_rgb;
        }
        if (a.x > y_buffer2[a.y][1].position.x) {
          y_buffer2[a.y][1].position = a;
          y_buffer2[a.y][1].rgb = a_rgb;
        }
      }
      //drawXY(leds,x1,y1,0,0,255);
      a.x++;

      err_rgb.y += a_dist_rgb.y;
      while (err_rgb.y >= a_dist.x) {
        a_rgb.y += step_rgb.y;
        err_rgb.y -= a_dist.x;
      }
      err_rgb.x += a_dist_rgb.x;
      while (err_rgb.x >= a_dist.x) {
        a_rgb.x += step_rgb.x;
        err_rgb.x -= a_dist.x;
      }
      err_rgb.z += a_dist_rgb.z;
      while (err_rgb.z >= a_dist.x) {
        a_rgb.z += step_rgb.z;
        err_rgb.z -= a_dist.x;
      }

      err.y += a_dist.y;
      if (err.y >= a_dist.x) {
        a.y += step.y;
        err.y -= a_dist.x;
      }
      err.z += a_dist.z;
      if (err.z >= a_dist.x) {
        a.z += step.z;
        err.z -= a_dist.x;
      }
    }
  } else {
    //draw vertically
    if (a.y > b.y) {
      swap_coords(a,a_norm,a_rgb,b,b_norm,b_rgb,dist);
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
          y_buffer2[a.y][0].rgb = a_rgb;
        }
        if (a.x > y_buffer2[a.y][1].position.x) {
          y_buffer2[a.y][1].position = a;
          y_buffer2[a.y][1].rgb = a_rgb;
        }
      }
      //drawXY(leds,x1,y1,0,0,255);
      a.y++;

      err_rgb.y += a_dist_rgb.y;
      while (err_rgb.y >= a_dist.y) {
        a_rgb.y += step_rgb.y;
        err_rgb.y -= a_dist.y;
      }
      err_rgb.x += a_dist_rgb.x;
      while (err_rgb.x >= a_dist.y) {
        a_rgb.x += step_rgb.x;
        err_rgb.x -= a_dist.y;
      }
      err_rgb.z += a_dist_rgb.z;
      while (err_rgb.z >= a_dist.y) {
        a_rgb.z += step_rgb.z;
        err_rgb.z -= a_dist.y;
      }

      err.x += a_dist.x;
      if (err.x >= a_dist.y) {
        a.x += step.x;
        err.x -= a_dist.y;
      }
      err.z += a_dist.z;
      if (err.z >= a_dist.y) {
        a.z += step.z;
        err.z -= a_dist.y;
      }
    }
  }


}

//DRAW LINE FINE


void draw_line_fine(CRGB crgb_object[], int32_t x1, int32_t y1, int32_t x2, int32_t y2, CRGB& rgb, const int& z_depth = -10000, const uint8_t& val = 255, const uint8_t& val2 = 255, const bool& trim = false) {
  
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
  
    x1_led = floor((x1+128)/256.f);
    x1_r = (x1_led+1)*256L - (x1+128);
    x2_led = ceil((x2-128)/256.f);
    x2_r = ((x2-128) - (x2_led-1)*256L);


    y1_led = floor((y1+128)/256.f);
    y1_r = (y1_led+1)*256L - (y1+128);
    y2_led = ceil((y2-128)/256.f);
    y2_r = ((y2-128) - (y2_led-1)*256L);
  
  } else {
    
    x1_led = floor((x1)/256.f);
    x1_r = (x1_led+1)*256L - (x1);
    x2_led = ceil((x2)/256.f);
    x2_r = ((x2) - (x2_led-1)*256L);


    y1_led = floor((y1)/256.f);
    y1_r = (y1_led+1)*256L - (y1);
    y2_led = ceil((y2)/256.f);
    y2_r = ((y2) - (y2_led-1)*256L);
  
  }

  int32_t x_dist = x2 - x1;
  int32_t y_dist = y2 - y1;
  float x_step = (1.f*x_dist)/(1.f*y_dist);
  float y_step = (1.f*y_dist)/(1.f*x_dist);
  if (abs(x1 - x2) > abs(y1 - y2)) {
    //calculate horizontally
    
    float y_start = y1 + (x1_led*256.f - x1)*y_step;
    y_step *= 256.f;
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
        float y = y_start;
        int Hy = ceil(y/256.f);
        int Ly = floor(y/256.f);
        int progress = abs(y - Hy*256);
        int b = _min( _max(progress, 0), 255);
        int b2 = 255-b;
        if (i == x1_led) {
          b = (b*x1_r)/256;
          b2 = (b2*x1_r)/256;
        }
        if (i == x2_led) {
          b = (b*x2_r)/256;
          b2 = (b2*x2_r)/256;
        }
        drawXYZ2(crgb_object,i, Hy, z_depth, rgb, (b2*v1)/256 + (b2*v2)/256 );
        drawXYZ2(crgb_object,i, Ly, z_depth, rgb, (b *v1)/256 + (b *v2)/256 );
        //drawXYZ(crgb_object,i, Hy, z_depth, hue, sat, (b2*v1)/256 + (b2*v2)/256 );
        //drawXYZ(crgb_object,i, Ly, z_depth, hue, sat, (b *v1)/256 + (b *v2)/256 );
      }
      y_start += y_step;
    }

    
  } else {
    //calculate vertically

    float x_start = x1 + (y1_led*256.f - y1)*x_step;
    x_step *= 256.f;
    for (int i = (y1_led); i <= (y2_led); i++) {
      //if ((i != y1_led && i != y2_led) || !trim) {
      if (true) {
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
        float x = x_start;
        int Hx = ceil(x/256.f);
        int Lx = floor(x/256.f);
        int progress = abs(x - Hx*256);
        int b = _min( _max(progress, 0), 255);
        int b2 = 255-b;
        if (i == y1_led) {
          b  = (b*y1_r)/256;
          b2 = (b2*y1_r)/256;
        }
        if (i == y2_led) {
          b  = (b*y2_r)/256;
          b2 = (b2*y2_r)/256;
        }
        drawXYZ2(crgb_object,  ceil(x_start/256L), i, z_depth, rgb, (b2*v1)/256 + (b2*v2)/256 );
        drawXYZ2(crgb_object, floor(x_start/256L), i, z_depth, rgb, (b *v1)/256 + (b *v2)/256 );
        //drawXYZ(crgb_object,  ceil(x_start/256L), i, z_depth, h, s, (b2*v1)/256 + (b2*v2)/256 );
        //drawXYZ(crgb_object, floor(x_start/256L), i, z_depth, h, s, (b *v1)/256 + (b *v2)/256 );
      }
      x_start += x_step;
    }
    
  }

} //draw_line_fine()

void draw_line_fine(CRGB crgb_object[], const int32_t& x1, const int32_t& y1, const int32_t& x2, const int32_t& y2, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255, const int& z_depth = -10000, const uint8_t& val2 = 255, const bool& trim = false) {
  CRGB rgb = CHSV(hue,sat,255);
  draw_line_fine(crgb_object, x1, y1, x2, y2, rgb, z_depth, val, val2, trim);
}

void draw_line_fine(CRGB crgb_object[], const VECTOR3& a, const VECTOR3& b, CRGB& rgb, const int& z_depth = -10000, const uint8_t& val = 255, const uint8_t& val2 = 255, const bool& trim = false) {
  draw_line_fine(crgb_object, a.x, a.y, b.x, b.y, rgb, z_depth, val, val2, trim);
}

//DRAW CURVE

//draw a curve by simultaneously shortening and rotating the line segment vectors

void matt_curve(int32_t coordinate_array[][2], const size_t& len, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255, const bool& flipXY = false, const bool& closedShape = false, const bool& extraSmooth = false) {

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
    len = _max((len-.5)*2,0);
    float exp = _min(exp0,exp1);
    float w = exp*len;

    float len2 = _min(last_len,len0);
    len2 = _max((len2-.5)*2,0);
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

void matt_curve8(CRGB crgb_object[], int32_t coordinate_array[][2], const size_t& len, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255, const bool& flipXY = false, const bool& closedShape = false, const bool& extraSmooth = false, const uint8_t& percentage = 255, const uint8_t& step_size = 32) {
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


void matt_curve8(int32_t coordinate_array[][2], const size_t& len, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255, const bool& flipXY = false, const bool& closedShape = false, const bool& extraSmooth = false, const uint8_t& percentage = 255, const uint8_t& step_size = 32) {

  matt_curve8(leds, coordinate_array, len, hue, sat, val, flipXY, closedShape, extraSmooth, percentage, step_size);

}





uint8_t circle_angles[20][17];

void reset_circle_angles() {
  for (int i = 0; i < 20; i++) {
    for (int j = 0; j < 16; j++) {
      circle_angles[i][j] = 255;
    }
  }
}

void draw_circle_fine(const int32_t& x, const int32_t& y, const int32_t& r, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255, const int& ballnum = -1, const uint8_t& step_size = 16) {
  
  if(step_size == 255) {
    blendXY(leds, x, y, hue, sat, val);
    return;
  }

  int t0 = 0;
  int32_t xl2;
  int32_t yl2;
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
      draw_line_fine(leds, xl, yl, xl2, yl2, hue, sat, val, -10000, val, true);
    }
    xl2 = xl;
    yl2 = yl;
    t0+=step_size;
  }
  //blendXY(leds, x, y, hue, sat, val);
}



void height_map_to_LED(const int& threshold = -128*256, const int& light_x = 100, const int& light_y = 100, const int& spec_x = 15, const int& spec_y = 15) {
  //write our computed values to the screen
  for (int x = 0; x < MATRIX_WIDTH; x++) {
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
      //height map coordinates
      //our height map is 1 pixel wider in each dimension than the screen
      int x2 = x+1;
      int y2 = y+1;
      if (height_map[x2][y2] >= threshold) {
        //attempt to find the approximate surface normal

        
        //horizontal pixel difference
        int u = height_map[x2+1][y2] - height_map[x2-1][y2]; //between -128*32 and 127*32
  
        //vertical pixel difference
        int v = height_map[x2][y2+1] - height_map[x2][y2-1]; //between -128*32 and 127*32
        
        
        //find the brightness based on a specific difference (angle)
        //0-255; 0 = full bright; 255 = off;
        int u_norm = abs(-light_x*32 - u)/24;
        int v_norm = abs(-light_y*32 - v)/24;

        if (1==1) {
          //change angle of light to be more extreme at top
          v_norm = abs(-(light_y-(y/16))*32 - v)/24;
        }
        //specular highlights
        int su_norm = abs(-spec_x*32 - u);
        int sv_norm = abs(-spec_y*32 - v);
        

        //0 = off; 255 = full bright
        u_norm = 255.f - u_norm;
        v_norm = 255.f - v_norm;

        su_norm = 255 - _min(su_norm, 255);
        sv_norm = 255 - _min(sv_norm, 255);
        
        
        //combine the vertical and horizontal components to find our final brightness for this pixel
        int norm = (u_norm*v_norm)/256;
  
        //specular highlights
        int snorm = (su_norm*sv_norm)/256L;

        int val = _max(_min(norm, 255), 20);

        if (1==1) {
          //light fades by distance
          //val = (val*(sq(255-y)/256L))/256L;
          val = (val*(255-y))/256L;
        }
        
        leds[XY(x,y)] = CHSV(default_color, default_saturation, val);
  
        //specular highlights
        leds[XY(x,y)] += CHSV(48, 64, _max(_min(snorm, 255), 0)/4);
        

        
      }
    }
  }
}













void LED_show() {
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

void LED_black() {
  
  //clear the string
  for (int y = 0; y < MATRIX_HEIGHT; y++) {
    memset8(&leds[y*MATRIX_WIDTH].r, 0, MATRIX_WIDTH*3);
  }
  
  //clear the Z buffer
  for (int x = 0; x < MATRIX_WIDTH; x++) {
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
      z_buffer[x][y] = INT16_MIN;
    }
  }

}

void reset_heightmap() {

  for (int x = 0; x < MATRIX_WIDTH; x++) {
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
      height_map[x][y] = 0;
    }
  }

}

//mixer algorithm from MurmerHash3
//provides a fast, repeatable, reasonably random number for a given input
//MurmurHash3 was written by Austin Appleby
uint32_t fmix32 ( uint32_t h )
{
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;

  return h;
}



uint16_t matt_compress(int32_t val) {
    uint32_t aval = abs(val);
    uint32_t sign = ((uint32_t)val >> 31) << 10;
		uint shift = 0;
    uint shift1 = 0;
		uint shift2 = 0;
    uint offset = 0;
    uint offset2 = 0;
    if (aval >= 64) {  //precision of 1
      offset = 64;
      shift1++;
      if (aval >= 128) { //precision of 1
        shift++;
        if (aval >= 256) { //precision of 2
          shift++;
          if (aval >= 512) { //precision of 4
            shift++;
            if (aval >= 1024) { //precision of 16
              shift++;
              if (aval >= 2048) { //precision of 32
                shift++;
                if (aval >= 4096) { //precision of 32
                  offset2 = (aval-2048)/2048;
                  if (aval >= 16384) { //precision of 256
                    shift2+=4;
                    shift+=3;
                    offset2 = 0;
                    if (aval >= 32786) { //precision of 512
                      shift++;
                      if (aval >= 65536) { //precision of 1024
                        shift++;
                        if (aval >= 131071) { //max value
                          aval = 131071;
                        }
                      }
                    }
                  } else {
                    shift2 += offset2;
                  }
                }
              }
            }
          }
        }
      }
    }

    uint tshift = shift+shift1+shift2;
		uint cval = ( ( (aval - offset2*2048) >> shift ) - offset );
    uint16_t creturn = (tshift << 6) | cval | sign;
    return creturn; 
	}


int32_t matt_decompress(uint16_t val) {
  bool sign = (val >> 10);
  val &= 0b01111111111;
  int32_t shift = val >> 6;
  int32_t cval = val & 0b0000111111;
  int32_t rval = 0;
  if (shift == 0) {
    rval = cval;
  } else if (shift < 7) {
    rval = cval + 64;
    rval <<= (shift-1);
  } else if (shift > 12) {
    rval = cval + 64;
    rval <<= (shift-5);
  } else {
    rval = cval + 64;
    rval <<= (6-1);
    rval += 2048*(shift-6);
  }
  return (sign) ? -rval : rval;
}

uint16_t matt_compress8(int32_t val) {
    uint32_t aval = abs(val);
    uint32_t sign = ((uint32_t)val >> 31) << 7;
		uint shift = 0;
    uint div = 4;
    if (aval >= 32) {
      shift++;
      aval -= 32;
      div = 4;
      if (aval >= 32) {
        shift++;
        aval -= 32;
        div = 8;
        if(aval >= 64) {
          shift++;
          aval -= 64;
          div = 16;
          if(aval >= 128) {
            shift++;
            aval -= 128;
            div = 32;
            if(aval >= 256) {
              shift++;
              aval -= 256;
              div = 64;
              if(aval >= 512) {
                shift++;
                aval -= 512;
                div = 128;
                if(aval >= 1024) {
                  shift++;
                  aval -= 1024;
                  div = 256;
                  if(aval >= 2048) {
                    shift++;
                    aval -= 2048;
                    if (aval >= 2048) {
                      shift++;
                      aval -= 2048;
                      if (aval >= 2048) {
                        shift++;
                        aval -= 2048;
                        if (aval >= 2048) {
                          shift++;
                          aval -= 2048;
                          if(aval >= 2048) {
                            shift++;
                            aval -= 2048;
                            div = 4096;
                            if(aval > 32768) {
                              shift++;
                              aval -= 32768;
                              if(aval > 32768) {
                                shift++;
                                aval -= 32768;
                                if(aval > 32768) {
                                  shift++;
                                  aval -= 32768;
                                  if(aval >= 32768) {
                                    aval = 32767;
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    } 

		uint cval = aval / div;
    uint8_t creturn = (shift << 3) | cval | sign;
    return creturn; 
	}

int32_t matt_decompress8(uint8_t val) {
  bool sign = (val >> 7);
  val &= 0b01111111;
  int32_t shift = val >> 3;
  int32_t cval = val & 0b00000111;
  int32_t rval = 0;
  if (shift == 0) {
    rval = cval*4;
  } else if (shift < 8) {
    rval = cval << (shift+1);
    rval += 32 << (shift-1);
  } else if (shift < 12) {
    rval = 4096 + cval*256 + (shift-8)*2048;
  } else {
    rval += 12288 + cval*4096 + (shift-12)*32768;
  }
  return (sign) ? -rval : rval;
}

    
  //create a datatype that uses 11 bits to store numbers from -131072 to 131072
  typedef class cint
  {
    private:
        uint16_t val;

    public:
        
        //conversion to int
        operator int() {
            return matt_decompress(val);
        }


        //conversion from int
        cint(int n) {
            val = matt_compress(n);
        }

        //conversion from float
        cint(float n) {
            val = matt_compress(n);
        }


         //overload +=
         void operator+= (int rhs) {
            val = matt_compress(matt_decompress(val) + rhs);
         }


         //overload -=
         void operator-= (int rhs) {
            val = matt_compress(matt_decompress(val) - rhs);
         }


         //overload *=
         void operator*= (int rhs) {
            val = matt_compress(matt_decompress(val) * rhs);
         }


         //overload /=
         void operator/= (int rhs) {
            val = matt_compress(matt_decompress(val) / rhs);
         }


         //overload +=
         void operator+= (float rhs) {
            val = matt_compress(matt_decompress(val) + rhs);
         }


         //overload -=
         void operator-= (float rhs) {
            val = matt_compress(matt_decompress(val) - rhs);
         }


         //overload *=
         void operator*= (float rhs) {
            val = matt_compress(matt_decompress(val) * rhs);
         }


         //overload /=
         void operator/= (float rhs) {
            val = matt_compress(matt_decompress(val) / rhs);
         }


   } cint;

   //create a datatype that uses 8 bits to store numbers from -139264 to 139264
  typedef class cint8
  {
    private:
        uint8_t val;

    public:
        
        //conversion to int
        operator int() {
            return matt_decompress8(val);
        }


        //conversion from int
        cint8(int n) {
            val = matt_compress8(n);
        }

        //conversion from float
        cint8(float n) {
            val = matt_compress8(n);
        }


         //overload +=
         void operator+= (int rhs) {
            val = matt_compress8(matt_decompress8(val) + rhs);
         }


         //overload -=
         void operator-= (int rhs) {
            val = matt_compress8(matt_decompress8(val) - rhs);
         }


         //overload *=
         void operator*= (int rhs) {
            val = matt_compress8(matt_decompress8(val) * rhs);
         }


         //overload /=
         void operator/= (int rhs) {
            val = matt_compress8(matt_decompress8(val) / rhs);
         }


         //overload +=
         void operator+= (float rhs) {
            val = matt_compress8(matt_decompress8(val) + rhs);
         }


         //overload -=
         void operator-= (float rhs) {
            val = matt_compress8(matt_decompress8(val) - rhs);
         }


         //overload *=
         void operator*= (float rhs) {
            val = matt_compress8(matt_decompress8(val) * rhs);
         }


         //overload /=
         void operator/= (float rhs) {
            val = matt_compress8(matt_decompress8(val) / rhs);
         }


   } cint8;

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
        operator int() {
            return val*CINT18_MULT;
        }

        //conversion from int
        cint18(const int& n) {
            val = n/CINT18_MULT;
        }

        //conversion from float
        cint18(const float& n) {
            val = n/CINT18_MULT;
        }

         //overload +=
         void operator+= (const int& rhs) {
            val = val + rhs/CINT18_MULT;
         }

         //overload -=
         void operator-= (const int& rhs) {
            val = val - rhs/CINT18_MULT;
         }


         //overload *=
         void operator*= (const int& rhs) {
            val = val * rhs;
         }


         //overload /=
         void operator/= (const int& rhs) {
            val = val / rhs;
         }


  } cint18;

  uint8_t gamma8_e[256];
  uint8_t gamma8_d[256];

  uint8_t gamma8_encode(const uint8_t& value) {
    return gamma8_e[value];
  }

  uint8_t gamma8_decode(const uint8_t& value) {
    return gamma8_d[value];
  }