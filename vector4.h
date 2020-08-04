#ifndef LIGHTS_VECTOR4_H
#define LIGHTS_VECTOR4_H

#include "vector3.h"
template <class T>
struct VECTOR3_CLASS;

template <class T>
struct VECTOR4_CLASS {
  T x;
  T y;
  T z;
  T w;

  inline VECTOR4_CLASS<T> () {}

  inline VECTOR4_CLASS<T> (const T& x_in, const T& y_in, const T& z_in, const T& w_in): x(x_in), y(y_in), z(z_in), w(w_in) {}
  
  template <class U>
  inline VECTOR4_CLASS<T> operator + (VECTOR4_CLASS<U> const &p_in) { 
    VECTOR4_CLASS<T> p; 
    p.w = w + p_in.w; 
    p.x = x + p_in.x; 
    p.y = y + p_in.y; 
    p.z = z + p_in.z; 
    return p; 
  } 

  template <class U>
  inline VECTOR4_CLASS<T> operator - (VECTOR4_CLASS<U> const &p_in) { 
    VECTOR4_CLASS<T> p; 
    p.w = w - p_in.w; 
    p.x = x - p_in.x; 
    p.y = y - p_in.y; 
    p.z = z - p_in.z; 
    return p; 
  } 
  
  template <class U>
  inline VECTOR4_CLASS<T> operator - (VECTOR4_CLASS<U> const &p_in) const { 
    VECTOR4_CLASS<T> p; 
    p.w = w - p_in.w; 
    p.x = x - p_in.x; 
    p.y = y - p_in.y; 
    p.z = z - p_in.z; 
    return p; 
  } 
  
  template <class U>
  inline VECTOR4_CLASS<T> operator - (VECTOR3_CLASS<U> const &p_in) const { 
    VECTOR4_CLASS<T> p; 
    p.x = x - p_in.x; 
    p.y = y - p_in.y; 
    p.z = z - p_in.z; 
    return p; 
  } 
  
  inline VECTOR4_CLASS<T> operator- () const { 
    return VECTOR4_CLASS<T>(-w,-x,-y,-z); 
  } 

  inline VECTOR4_CLASS<T> operator * (int const &num) { 
  VECTOR4_CLASS<T> p; 
    p.w = w * num; 
    p.x = x * num; 
    p.y = y * num; 
    p.z = z * num; 
    return p; 
  }

  template <class U>
  inline VECTOR4_CLASS<T> operator * (VECTOR4_CLASS<U> const& v) { 
    VECTOR4_CLASS<T> p; 
    p.w = w * v.w; 
    p.x = x * v.x; 
    p.y = y * v.y; 
    p.z = z * v.z; 
    return p; 
  } 

  inline VECTOR4_CLASS<T> operator / (int const &num) { 
    VECTOR4_CLASS<T> p; 
    p.w = w / num; 
    p.x = x / num; 
    p.y = y / num; 
    p.z = z / num; 
    return p; 
  } 


  //overload -=
  template <class U>
  inline VECTOR4_CLASS<T>& operator-= (const VECTOR4_CLASS<U>& rhs) {
    this->w -= rhs.w;
    this->x -= rhs.x;
    this->y -= rhs.y;
    this->z -= rhs.z;
    return *this;
  }
  //overload +=
  template <class U>
  inline VECTOR4_CLASS<T>& operator+= (const VECTOR4_CLASS<U>& rhs) {
    this->w += rhs.w;
    this->x += rhs.x;
    this->y += rhs.y;
    this->z += rhs.z;
    return *this;
  }

  //overload +=
  inline VECTOR4_CLASS<T>& operator+= (const int& rhs) {
    this->w += rhs;
    this->x += rhs;
    this->y += rhs;
    this->z += rhs;
    return *this;
  }
  
  //overload /=
  inline VECTOR4_CLASS<T>& operator/= (const int& rhs) {
    this->w /= rhs;
    this->x /= rhs;
    this->y /= rhs;
    this->z /= rhs;
    return *this;
  }

  //overload *=
  inline VECTOR4_CLASS<T>& operator*= (const int& rhs) {
    this->w *= rhs;
    this->x *= rhs;
    this->y *= rhs;
    this->z *= rhs;
    return *this;
  }

  //conversion from other types
  template <class U>
  inline VECTOR4_CLASS<T>(const VECTOR4_CLASS<U>& n) {
      w = n.w;
      x = n.x;
      y = n.y;
      z = n.z;
  }

  //conversion from VECTOR3
  template <class U>
  inline VECTOR4_CLASS<T>(const VECTOR3_CLASS<U>& n) {
      w = 0;
      x = n.x;
      y = n.y;
      z = n.z;
  }

  inline T& operator[] (const int& index)
  {
      return index == 0 ? x : index == 1 ? y : index == 2 ? z : w;
  }

  inline void invert () {
    w = -w;
    x = -x;
    y = -y;
    z = -z;
  }

};


typedef VECTOR4_CLASS<int32_t> VECTOR4;
typedef VECTOR4_CLASS<uint8_t> VECTOR4_8;
typedef VECTOR4_CLASS<int8_t> VECTOR4_8s;
typedef VECTOR4_CLASS<uint16_t> VECTOR4_16;
typedef VECTOR4_CLASS<float> VECTOR4_f;


inline VECTOR4 abs(const VECTOR4& v) {
    VECTOR4 temp;
    temp.w = abs(v.w);
    temp.x = abs(v.x);
    temp.y = abs(v.y);
    temp.z = abs(v.z);
    return temp;
  }


#endif