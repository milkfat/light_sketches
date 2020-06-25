#ifndef LIGHTS_MATH_HELPERS_H
#define LIGHTS_MATH_HELPERS_H

#ifndef _min
#define _min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef _max
#define _max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef _clamp8
#define _clamp8(a) _min(_max(a,0),255)
#endif

#include "vector3.h"


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


#ifndef PI
#define PI 355/113.f
#endif

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

static inline __attribute__ ((always_inline)) int16_t easeSwing(uint16_t i) {
    i%=1024;
    if (i < 256) {
        return ease8InOutQuad(i);
    } else if (i < 256*2) {
        return ease8InOutQuad(255-(i-256));
    } else if (i < 256*3) {
        return -ease8InOutQuad(i-256*2);
    } else {
        return -ease8InOutQuad(255-(i-256*3));
    }
};

uint16_t bellCurve16(uint16_t n) {
  if (n < 32768) {
    n = 32767 - n;
    return 32767 - n*n/32767;
  }
  else
  {
    n = n - 32768;
    return 32768 + (n*n)/32767;
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

/*
typedef class half_t
{

  private:
      uint16_t val = 0;

  public:

      half_t() {}

      //conversion to float
      inline operator float() {
          uint32_t v = val;
          v <<= 16;
          return *reinterpret_cast<float*>(&v);
      }

      //conversion from float
      inline half_t(const float& n) {
          float v = n;
          val = (*reinterpret_cast<uint32_t*>(&v))>>16;
      }

      //overload +=
      inline void operator+= (const float& rhs) {
          uint32_t v = val;
          v <<= 16;
          float f = (*reinterpret_cast<float*>(&v) + (float)rhs);
          val = (*reinterpret_cast<uint32_t*>(&f))>>16;
      }

      //overload -=
      inline void operator-= (const float& rhs) {
          uint32_t v = val;
          v <<= 16;
          float f = (*reinterpret_cast<float*>(&v) - (float)rhs);
          val = (*reinterpret_cast<uint32_t*>(&f))>>16;
      }

      //overload *=
      inline void operator*= (const float& rhs) {
          uint32_t v = val;
          v <<= 16;
          float f = (*reinterpret_cast<float*>(&v) * (float)rhs);
          val = (*reinterpret_cast<uint32_t*>(&f))>>16;
      }

      //overload /=
      inline void operator/= (const float& rhs) {
          uint32_t v = val;
          v <<= 16;
          float f = (*reinterpret_cast<float*>(&v) / (float)rhs);
          val = (*reinterpret_cast<uint32_t*>(&f))>>16;
      }

} half_t;

*/

//create a datatype that uses 16 bits to store an 18-bit integer (-131072 to 131068)
typedef class cint18
{
  private:

  public:
      int16_t val = 0;

      cint18() {
      }

      #define CINT18_MULT 8
      
      //conversion to int
      inline operator int() const {
          return val*CINT18_MULT;
      }

      //conversion from int
      inline cint18(const int& n) {
          val = n/CINT18_MULT;
      }

      //conversion from float
      // inline cint18(const float& n) {
      //     val = n/CINT18_MULT;
      // }

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

uint msb(uint n) {
  if (!n) return 0;
  if (n==1) return 1;
  uint out = 1;
  while (n) {
    n = n>>1;
    out = out<<1;
  }
  return out;
}

#endif