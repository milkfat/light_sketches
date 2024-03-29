#ifndef LIGHTS_VECTOR3_H
#define LIGHTS_VECTOR3_H

#include "vector4.h"

template <class T>
struct VECTOR3_CLASS {
  T x;
  T y;
  T z;

  inline VECTOR3_CLASS<T> () {}

  inline VECTOR3_CLASS<T> (const T& x_in, const T& y_in, const T& z_in): x(x_in), y(y_in), z(z_in) {}
  
  template <class U>
  inline VECTOR3_CLASS<T> operator + (VECTOR3_CLASS<U> const &p_in) { 
    VECTOR3_CLASS<T> p; 
    p.x = x + p_in.x; 
    p.y = y + p_in.y; 
    p.z = z + p_in.z; 
    return p; 
  } 

  template <class U>
  inline VECTOR3_CLASS<T> operator - (VECTOR3_CLASS<U> const &p_in) { 
    VECTOR3_CLASS<T> p; 
    p.x = x - p_in.x; 
    p.y = y - p_in.y; 
    p.z = z - p_in.z; 
    return p; 
  } 
  
  template <class U>
  inline VECTOR3_CLASS<T> operator - (VECTOR3_CLASS<U> const &p_in) const { 
    VECTOR3_CLASS<T> p; 
    p.x = x - p_in.x; 
    p.y = y - p_in.y; 
    p.z = z - p_in.z; 
    return p; 
  } 
  
  inline VECTOR3_CLASS<T> operator- () const { 
    return VECTOR3_CLASS<T>(-x,-y,-z); 
  } 

  inline VECTOR3_CLASS<T> operator * (int const &num) { 
  VECTOR3_CLASS<T> p; 
    p.x = x * num; 
    p.y = y * num; 
    p.z = z * num; 
    return p; 
  }

  template <class U>
  inline VECTOR3_CLASS<T> operator * (VECTOR3_CLASS<U> const& v) { 
    VECTOR3_CLASS<T> p; 
    p.x = x * v.x; 
    p.y = y * v.y; 
    p.z = z * v.z; 
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
  template <class U>
  inline VECTOR3_CLASS<T>& operator-= (const VECTOR3_CLASS<U>& rhs) {
    this->x -= rhs.x;
    this->y -= rhs.y;
    this->z -= rhs.z;
    return *this;
  }
  //overload +=
  template <class U>
  inline VECTOR3_CLASS<T>& operator+= (const VECTOR3_CLASS<U>& rhs) {
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

  //overload *=
  inline VECTOR3_CLASS<T>& operator*= (const int& rhs) {
    this->x *= rhs;
    this->y *= rhs;
    this->z *= rhs;
    return *this;
  }

  //conversion from other types
  template <class U>
  inline VECTOR3_CLASS<T>(const VECTOR3_CLASS<U>& n) {
      x = n.x;
      y = n.y;
      z = n.z;
  }

  //conversion from other types
  template <class U>
  inline VECTOR3_CLASS<T>(const VECTOR4_CLASS<U>& n) {
      x = n.x;
      y = n.y;
      z = n.z;
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

    VECTOR3_CLASS<T> norm = *this;
    uint32_t m = _max(_max(abs(x),abs(y)),abs(z))/128+1;
    norm/=m;
    int32_t length = sqrt16(norm.x*norm.x+norm.y*norm.y+norm.z*norm.z);
    if (length != 0) {
      norm.x = (norm.x*255)/length;
      norm.y = (norm.y*255)/length;
      norm.z = (norm.z*255)/length;
    } else {
      norm.x = 0;
      norm.y = 0;
      norm.z = 0;
    }
    return norm;

  }

  int32_t inline __attribute__((always_inline))  unit_ip() {

    VECTOR3_CLASS<int32_t> norm = *this;
    uint32_t m = _max(_max(abs(x),abs(y)),abs(z))/128+1;
    norm = *this;
    norm/=m;
    int32_t length = sqrt16(norm.x*norm.x+norm.y*norm.y+norm.z*norm.z);
    //int32_t length = sqrt16(x*x+y*y+z*z);
    if (length != 0) {
      x = (x*255)/length;
      y = (y*255)/length;
      z = (z*255)/length;
    } else {
      x = 0;
      y = 0;
      z = 0;
    }
    return length;

  }

};


typedef VECTOR3_CLASS<int32_t> VECTOR3;
typedef VECTOR3_CLASS<uint8_t> VECTOR3_8;
typedef VECTOR3_CLASS<int8_t> VECTOR3_8s;
typedef VECTOR3_CLASS<uint16_t> VECTOR3_16;


inline VECTOR3 abs(const VECTOR3& v) {
    VECTOR3 temp;
    temp.x = abs(v.x);
    temp.y = abs(v.y);
    temp.z = abs(v.z);
    return temp;
  }


#endif