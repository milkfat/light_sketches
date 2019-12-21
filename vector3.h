#ifndef LIGHTS_VECTOR3_H
#define LIGHTS_VECTOR3_H

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


#endif