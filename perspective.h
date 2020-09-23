#ifndef LIGHTS_PERSPECTIVE_H
#define LIGHTS_PERSPECTIVE_H

#include "vector3.h"
#include "matrix.h"

#define MATRIX_PRECISION 16

class PERSPECTIVE {

    private:

        int32_t Cz0;
        int32_t Sz0;
        int32_t Cz; //camera Z
        int32_t Sz; //projection screen Z (between camera and object)
        int32_t Cz2;
        int32_t Sz2;
        int32_t _X_MAX = INT32_MIN;
        int32_t _X_MIN = INT32_MAX;
        int32_t _Y_MAX = INT32_MIN;
        int32_t _Y_MIN = INT32_MAX;


    public:
        VECTOR3 camera_position = VECTOR3(0,0,232*256);
        VECTOR3 camera_offset = VECTOR3(0,0,0);
        uint16_t camera_rotate_x = 0;
        uint16_t camera_rotate_y = 0;
        uint16_t camera_rotate_z = 0;
        int32_t screen_distance = camera_position.z - 100*256;
        const int32_t screen_width;
        const int32_t screen_height;
        CRGB * screen_buffer;
        float rotation_alpha = 0;
        float rotation_beta = 90;
        float rotation_gamma = 0;
        uint8_t light_falloff = 8; //(bitshift value, 1 = near, 16 = far)

        MATRIX matrix = MATRIX(&rotation_alpha, &rotation_beta, &rotation_gamma, &camera_position, &camera_offset, &camera_rotate_x, &camera_rotate_y, &camera_rotate_z);
        
        PERSPECTIVE (const uint& width, const uint& height) : screen_width(width), screen_height(height) {}

    //take X,Y,Z coordinate
    //modifies X,Y to screen coordinates

    void camera_direction (VECTOR3& dir) {
        rotate16_z(dir, camera_rotate_z);
        rotate16_x(dir, camera_rotate_x);
        rotate16_y(dir, camera_rotate_y);
    }

    void camera_move (VECTOR3 v) {
        //matrix.rotate(v);
        //v.x = -v.x;
        //v.y = -v.y;
        camera_direction(v);
        camera_offset+=v;
    }

    inline __attribute__ ((always_inline)) void update() {

            Cz0 = camera_position.z;
            Sz0 = camera_position.z-screen_distance;
            Cz = Cz0/MATRIX_PRECISION; //camera Z
            Sz = Sz0/MATRIX_PRECISION; //projection screen Z (between camera and object)
            Cz2 = Cz0/(MATRIX_PRECISION*16);
            Sz2 = Sz0/(MATRIX_PRECISION*16);
    }

    void reset_camera () {
        camera_position = VECTOR3(0,0,232*256);
        camera_offset = VECTOR3(0,0,0);

        camera_rotate_x = 0;
        camera_rotate_y = 0;
        camera_rotate_z = 0;
        
        screen_distance = 100*256;
        rotation_alpha = 0;
        rotation_beta = 90;
        rotation_gamma = 0;
        update();
    }


    inline __attribute__ ((always_inline)) bool perspective(int32_t& x, int32_t& y, int32_t& z)
    {
        // if (abs(x) + abs(Sz0-Cz0) < 16384)
        // {
        //     return perspective_hp(x, y, z);
        // }
        // else if (abs(x) + abs(Sz0-Cz0) < 16384*4)
        // {
            return perspective_mp(x, y, z);
        // }
        // else
        // {
        //     return perspective_lp(x, y, z);
        // }
    }

    inline __attribute__ ((always_inline)) bool perspective_hp(int32_t& x, int32_t& y, int32_t& z) {
        z = _min(z, Cz0-1);
        if (z < Cz0) {
            x = ( x * ((Sz0 - Cz0)) ) / (z-Cz0) + ((screen_width * 128));
            y = ( y * ((Sz0 - Cz0)) ) / (z-Cz0) + ((screen_height * 128));
            return true;
        }
        return false;
    }

    inline __attribute__ ((always_inline)) bool perspective_mp(int32_t& x, int32_t& y, int32_t& z) {
        z/=MATRIX_PRECISION;
        if (z-Cz == 0) {
            z -= 1;
        }
        if (z-Cz != 0) {
            x/=MATRIX_PRECISION;//half precision to double each axis of our available coordinate space
            y/=MATRIX_PRECISION;
            x = ( x * ((Sz - Cz)) ) / (z-Cz);
            y = ( y * ((Sz - Cz)) ) / (z-Cz);
            if (z > Cz) {
                x = -x;
                y = -y;
            }
            x*=MATRIX_PRECISION;
            y*=MATRIX_PRECISION;
            x+=screen_width * 128;
            y+=screen_height * 128;
            if (z > Cz) {
                return false;
            }
            z*=MATRIX_PRECISION;
            if ( (x < 0 || x > MATRIX_WIDTH*256) ) {
                return false;
            }
            return true;
        }
        z*=MATRIX_PRECISION;
        return false;
    }

    inline __attribute__ ((always_inline)) bool perspective_lp(int32_t& x, int32_t& y, int32_t& z) {
        z/=MATRIX_PRECISION*16;
        z = _min(z, Cz2-1);
        if (z < Cz2) {
            x/=MATRIX_PRECISION*16;//half precision to double each axis of our available coordinate space
            y/=MATRIX_PRECISION*16;
            x = ( x * ((Sz2 - Cz2)) ) / (z-Cz2) + ((screen_width * 128)/(MATRIX_PRECISION*16));
            y = ( y * ((Sz2 - Cz2)) ) / (z-Cz2) + ((screen_height * 128)/(MATRIX_PRECISION*16));
            x*=MATRIX_PRECISION*16;
            y*=MATRIX_PRECISION*16;
            z*=MATRIX_PRECISION*16;
            return true;
        }
        z*=MATRIX_PRECISION*16;
        return false;
    }

    inline __attribute__ ((always_inline)) bool perspective_zero(int32_t& x, int32_t& y, int32_t& z) {
        z/=MATRIX_PRECISION;
        if (z < Cz) {
            x-=camera_position.x;
            y-=camera_position.y;
            x/=MATRIX_PRECISION;//half precision to double each axis of our available coordinate space
            y/=MATRIX_PRECISION;
            x = ( x * ((Sz - Cz)) ) / (z-Cz);
            y = ( y * ((Sz - Cz)) ) / (z-Cz);
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
        if (Sz != Cz) {
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
            x+=camera_position.x;
            y+=camera_position.y;
            return true;
        }
        return false;
    }

    inline __attribute__ ((always_inline)) bool reverse_perspective(int32_t p[3]) {
        return reverse_perspective(p[0], p[1], p[2]);
    }

    inline __attribute__ ((always_inline)) bool reverse_perspective(VECTOR3& p) {
        return reverse_perspective(p.x, p.y, p.z);
    }

    //return LED position from X,Y coordinates
    //return NUM_LEDS-1 (our safety "invisible" pixel) if coordinates are off-screen
    inline __attribute__ ((always_inline)) uint32_t XY(int x, int y) {

        _X_MAX = _max(_X_MAX, x);
        _X_MIN = _min(_X_MIN, x);
        _Y_MAX = _max(_Y_MAX, y);
        _Y_MIN = _min(_Y_MIN, y);

        if (x >= 0 && x < screen_width && y >= 0 && y < screen_height) {
            int32_t location = ((screen_height-1)-y)*screen_width + x;

            if (location > screen_width*screen_height || location < 0) {
                return screen_width*screen_height;
            } else {
                return location;
            }
        } else {
            return screen_width*screen_height;
        }  
    }

    inline __attribute__ ((always_inline)) bool out_of_bounds() {
        return _X_MAX > screen_width-1 || _X_MIN < 0 || _Y_MAX > screen_height-1 || _Y_MIN < 0;
    }

    inline __attribute__ ((always_inline)) void reset_boundaries() {
        _X_MAX = INT32_MIN;
        _X_MIN = INT32_MAX;
        _Y_MAX = INT32_MIN;
        _Y_MIN = INT32_MAX;
    }

    //0 = all drawing is within screen boundaries
    //1 = drawing occurred beyond the lower boundary
    //2 = drawing occurred beyond the upper boundary
    //3 = drawing occurred beyond both boundaries
    inline __attribute__ ((always_inline)) uint8_t x_boundary_status() {
        return (_X_MIN < 0) + (_X_MAX > screen_width-1)*2;
    }

    inline __attribute__ ((always_inline)) uint8_t y_boundary_status() {
        return (_Y_MIN < 0) + (_Y_MAX > screen_height-1)*2;
    }

    //returns positive number (of pixels) if drawing occurred beyond a boundary
    inline __attribute__ ((always_inline)) int32_t y_boundary_status_upper() {
        return _Y_MAX - screen_height;
    }

    inline __attribute__ ((always_inline)) int32_t y_boundary_status_lower() {
        return -_Y_MIN;
    }

    inline int32_t camera_distance(int32_t x, int32_t y, int32_t z) {
        matrix.rotate(x, y, z);
        x = (x-camera_position.x)/256;
        y = (y-camera_position.y)/256;
        z = (z-camera_position.z)/256;
        return sqrt(x*x+y*y+z*z);
    }
    

};

#endif