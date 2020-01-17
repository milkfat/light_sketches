#ifndef LIGHTS_PERSPECTIVE_H
#define LIGHTS_PERSPECTIVE_H

#include "vector3.h"
#include "matrix.h"


class PERSPECTIVE {

    private:

        int32_t Cz; //camera Z
        int32_t Sz; //projection screen Z (between camera and object)
        int32_t Cz2;
        int32_t Sz2;


    public:
        int16_t camera_scaler = 232;
        int16_t screen_scaler = 100;
        const uint32_t screen_width;
        const uint32_t screen_height;
        CRGB * screen_buffer;
        float rotation_alpha = 0;
        float rotation_beta = 90;
        float rotation_gamma = 0;

        MATRIX matrix = MATRIX(&rotation_alpha, &rotation_beta, &rotation_gamma);
        
        PERSPECTIVE (const uint& width, const uint& height) : screen_width(width), screen_height(height) {}

    //take X,Y,Z coordinate
    //modifies X,Y to screen coordinates
    #define MATRIX_PRECISION 4

    inline __attribute__ ((always_inline)) void update() {

            Cz = camera_scaler * 256L; //camera Z
            Sz = screen_scaler * 256L; //projection screen Z (between camera and object)
            Cz2 = Cz/2;
            Sz2 = Sz/2;
    }

    inline __attribute__ ((always_inline)) bool perspective(int32_t& x, int32_t& y, int32_t& z) {
        if (z < Cz) {
            x/=MATRIX_PRECISION;//half precision to double each axis of our available coordinate space
            y/=MATRIX_PRECISION;
            z/=MATRIX_PRECISION;
            int32_t zCz = (z-Cz);
            if (zCz == 0) {
                zCz = 1;
            }
            x = ( x * ((Sz - Cz)) ) / zCz + ((screen_width * 128)/MATRIX_PRECISION);
            y = ( y * ((Sz - Cz)) ) / zCz + ((screen_height * 128)/MATRIX_PRECISION);
            x*=MATRIX_PRECISION;
            y*=MATRIX_PRECISION;
            z*=MATRIX_PRECISION;
            return true;
        }
        return false;
    }

    inline __attribute__ ((always_inline)) bool perspective(int32_t p[3]) {
        return perspective(p[0], p[1], p[2]);
    }

    inline __attribute__ ((always_inline)) bool perspective(VECTOR3& p) {
        return perspective(p.x, p.y, p.z);
    }

    inline __attribute__ ((always_inline)) bool perspective(VECTOR3& p, VECTOR3& p2) {
        p2 = p;
        return perspective(p2.x, p2.y, p2.z);
    }

    //find the 3D coordinate of a pixel on the screen
    //takes screen X,Y coordinate along with the desired Z coordinate
    //modifies X,Y to provide X,Y,Z coordinate
    inline __attribute__ ((always_inline)) bool reverse_perspective(int32_t& x, int32_t& y, int32_t& z) {
        x/=MATRIX_PRECISION;//half precision to double each axis of our available coordinate space
        y/=MATRIX_PRECISION;
        z/=MATRIX_PRECISION;
        x = ( x - (screen_width*128)/MATRIX_PRECISION ) * ( z - Cz );
        x /= ( Sz - Cz );
        y = ( y - (screen_height*128)/MATRIX_PRECISION ) * ( z - Cz );
        y /=  ( Sz - Cz );
        x*=MATRIX_PRECISION;
        y*=MATRIX_PRECISION;
        z*=MATRIX_PRECISION;
        return true;
    }

    inline __attribute__ ((always_inline)) bool reverse_perspective(int32_t p[3]) {
        return reverse_perspective(p[0], p[1], p[2]);
    }

    inline __attribute__ ((always_inline)) bool reverse_perspective(VECTOR3& p) {
        return reverse_perspective(p.x, p.y, p.z);
    }

    //return LED position from X,Y coordinates
    //return NUM_LEDS-1 (our safety "invisible" pixel) if coordinates are off-screen
    inline __attribute__ ((always_inline)) uint32_t XY(const int& x, const int& y) {
    if (x >= 0 && x < screen_width && y >= 0 && y < screen_height) {
      int32_t location = y*screen_width + x;
      if (location > screen_width*screen_height || location < 0) {
          return screen_width*screen_height;
      } else {
          return location;
      }
    } else {
      return screen_width*screen_height;
    }
}
};

#endif