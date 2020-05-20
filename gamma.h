#ifndef LIGHTS_GAMMA_H
#define LIGHTS_GAMMA_H

#include "math_helpers.h"

//gamma 2.2 lookup tables:
//convert 8-bit linear to 8-bit gamma
uint8_t gamma8_e[256]; 

//convert 16-bit linear to 8-bit gamma
//first 256 values only, higher values should be divided by 256 and converted as 8-bit linear
uint8_t gamma8_e_16[256]; 
uint8_t gamma8_e_12[256]; 
uint8_t gamma8_e_10[256]; 

//decode 8-bit gamma to 8-bit linear
uint8_t gamma8_d[256];

//decode 8-bit gamma to 16-bit linear
uint16_t gamma16_d[256];

//decode 8-bit gamma to 12-bit linear
uint16_t gamma12_d[256];

void build_gamma_tables() {
  //create some lookup tables for color conversion
  for (int i = 0; i < 256; i++) {
    gamma8_e[i] = round(255.f * pow(i/255.f,1.f/2.2f));
    gamma8_e_16[i] = round(255.f * pow(i/65535.f,1.f/2.2f));
    gamma8_e_12[i] = round(255.f * pow(i/4095.f,1.f/2.2f));
    gamma8_e_10[i] = round(255.f * pow(i/1023.f,1.f/2.2f));
    gamma8_d[i] = round(255.f * pow(i/255.f,2.2f));
    gamma16_d[i] = round(65535.f * pow(i/255.f,2.2f));
    gamma12_d[i] = round(4095.f * pow(i/255.f,2.2f));
    //std::cout << "i: " << i << " 12: " << gamma12_d[i] << "\n";
    //std::cout << "i: " << i << " 8e: " << (uint)gamma8_e[i] << " 8e16low: " << (uint)gamma8_e_16_low[i] << " 8d: " << (uint)gamma8_d[i] << " 16d " << (uint)gamma16_d[i] << "\n";
  }
}




//various gamma encode/decode functions


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
  //return (value>>8) ? gamma8_e[value>>8] : gamma8_e_16_low[value];
  if (value < 256) {
    return gamma8_e_16[value];
  }
  if ( value < 4096) {
    return gamma8_e_12[value>>4];
  }
  if ( value < 16384) {
    return gamma8_e_10[value>>6];
  }
  return gamma8_e[value>>8];


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

static inline __attribute__ ((always_inline)) CRGB gamma8_decode(const VECTOR3& value) {
  CRGB rgb( gamma8_d[_clamp8(value.x)], gamma8_d[_clamp8(value.y)], gamma8_d[_clamp8(value.z)] );
  return rgb;
}




//add gamma encoded color to another gamma encoded color

static inline __attribute__ ((always_inline)) uint8_t gamma8_add_gamma8(const uint8_t& gval, const uint8_t& gval2) {
  //add an 8-bit linear value to an 8-bit gamma encoded value  
  return gamma16_encode( _min( ( gamma16_decode(gval) + gamma16_decode(gval2) ), 65535) );
}

static inline __attribute__ ((always_inline)) void color_add_gamma8(uint8_t& value, const uint8_t& value2) {
  value = gamma8_add_gamma8(value, value2);
}

static inline __attribute__ ((always_inline)) void color_add_gamma8(CRGB& rgb, const CRGB& rgb2) {
  color_add_gamma8(rgb.r, rgb2.r);
  color_add_gamma8(rgb.g, rgb2.g);
  color_add_gamma8(rgb.b, rgb2.b);
}




//combine linear color with gamma color

static inline __attribute__ ((always_inline)) uint8_t gamma8_add_linear8(const uint8_t& gval, const uint8_t& lval) {
  //add an 8-bit linear value to an 8-bit gamma encoded value  
  return gamma16_encode( _min( ( gamma16_decode(gval) + (lval << 8) ), 65535) );
}

static inline __attribute__ ((always_inline)) uint8_t gamma8_sub_linear8(const uint8_t& gval, const uint8_t& lval) {
  //add an 8-bit linear value to an 8-bit gamma encoded value  
  return gamma16_encode( _max( ( gamma16_decode(gval) - (lval << 8) ), 0) );
}

static inline __attribute__ ((always_inline)) void color_add_linear8(uint8_t& value, const uint8_t& value2) {
  value = gamma8_add_linear8(value, value2);
}

static inline __attribute__ ((always_inline)) void color_sub_linear8(uint8_t& value, const uint8_t& value2) {
  value = gamma8_sub_linear8(value, value2);
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

static inline __attribute__ ((always_inline)) void color_sub_linear8(CRGB& rgb, const CRGB& rgb2) {
  color_sub_linear8(rgb.r, rgb2.r);
  color_sub_linear8(rgb.g, rgb2.g);
  color_sub_linear8(rgb.b, rgb2.b);
}

static inline __attribute__ ((always_inline)) void color_scale(uint8_t& value, const uint8_t& scaler) {

  //value = gamma16_encode(((gamma16_decode(value)*scaler)/255));
  value = (value*scaler)/255;

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


static inline __attribute__ ((always_inline)) void color_sub_scaled_linear(CRGB& rgb, const CRGB& rgb2, const uint8_t& scaler) {
  color_sub_linear8(rgb.r, (rgb2.r*scaler)/255);
  color_sub_linear8(rgb.g, (rgb2.g*scaler)/255);
  color_sub_linear8(rgb.b, (rgb2.b*scaler)/255);
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




#endif