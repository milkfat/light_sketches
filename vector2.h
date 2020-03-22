#ifndef LIGHTS_VECTOR2_H
#define LIGHTS_VECTOR2_H

template <class T>
struct VECTOR2_CLASS {
  T x;
  T y;

  inline VECTOR2_CLASS<T> () {}

  inline VECTOR2_CLASS<T> (const T& x_in, const T& y_in): x(x_in), y(y_in) {}
  
  inline VECTOR2_CLASS<T> operator + (VECTOR2_CLASS<T> const &p_in) { 
    VECTOR2_CLASS<T> p; 
    p.x = x + p_in.x; 
    p.y = y + p_in.y; 
    return p; 
  } 

  inline VECTOR2_CLASS<T> operator - (VECTOR2_CLASS<T> const &p_in) { 
    VECTOR2_CLASS<T> p; 
    p.x = x - p_in.x; 
    p.y = y - p_in.y;
    return p; 
  } 
  
  inline VECTOR2_CLASS<T> operator - (VECTOR2_CLASS<T> const &p_in) const { 
    VECTOR2_CLASS<T> p; 
    p.x = x - p_in.x; 
    p.y = y - p_in.y; 
    return p; 
  } 

  inline VECTOR2_CLASS<T> operator * (int const &num) { 
    VECTOR2_CLASS<T> p; 
    p.x = x * num; 
    p.y = y * num; 
    return p; 
  } 

  inline VECTOR2_CLASS<T> operator / (int const &num) { 
    VECTOR2_CLASS<T> p; 
    p.x = x / num; 
    p.y = y / num; 
    return p; 
  } 


  //overload -=
  inline VECTOR2_CLASS<T>& operator-= (const VECTOR2_CLASS<T>& rhs) {
    this->x -= rhs.x;
    this->y -= rhs.y;
    return *this;
  }
  //overload +=
  inline VECTOR2_CLASS<T>& operator+= (const VECTOR2_CLASS<T>& rhs) {
    this->x += rhs.x;
    this->y += rhs.y;
    return *this;
  }

  //overload +=
  inline VECTOR2_CLASS<T>& operator+= (const int& rhs) {
    this->x += rhs;
    this->y += rhs;
    return *this;
  }
  
  //overload /=
  inline VECTOR2_CLASS<T>& operator/= (const int& rhs) {
    this->x /= rhs;
    this->y /= rhs;
    return *this;
  }

  inline T& operator[] (const int& index)
  {
      return index == 0 ? x : y;
  }

  inline void invert () {
    x = -x;
    y = -y;
  }

  VECTOR2_CLASS<T> inline __attribute__((always_inline))  unit() {

    VECTOR2_CLASS<T> norm;
    int32_t length = sqrt16(x*x+y*y);
    if (length != 0) {
      norm.x = (x*255)/length;
      norm.y = (y*255)/length;
    } else {
      norm.x = 0;
      norm.y = 0;
    }
    return norm;

  }

};


typedef VECTOR2_CLASS<int32_t> VECTOR2;
typedef VECTOR2_CLASS<int32_t> VECTOR2_8;
typedef VECTOR2_CLASS<int8_t> VECTOR2_8s;


inline VECTOR2 abs(const VECTOR2& v) {
    VECTOR2 temp;
    temp.x = abs(v.x);
    temp.y = abs(v.y);
    return temp;
  }


#endif