#ifndef LIGHTS_ROTATE_H
#define LIGHTS_ROTATE_H

float rotation_alpha = 0;
float rotation_beta = 90;
float rotation_gamma = 0;

static inline __attribute__ ((always_inline)) void rotate_x(int32_t& x, int32_t& y, int32_t& z, int8_t& s, int8_t& c) {
    int32_t temp = ( y*c - z*s ) / 128;
    z = ( y*s + z*c ) / 128;
    y = temp;
}

static inline __attribute__ ((always_inline)) void rotate_x(int32_t p[3], const uint8_t& ang) {
    int8_t s = sin8(ang)-128;
    int8_t c = cos8(ang)-128;
    rotate_x(p[0], p[1], p[2], s, c);
}

static inline __attribute__ ((always_inline)) void rotate_x(VECTOR3& p, const uint8_t& ang) {
    int8_t s = sin8(ang)-128;
    int8_t c = cos8(ang)-128;
    rotate_x(p.x, p.y, p.z, s, c);
}


static inline __attribute__ ((always_inline)) void rotate_y(int32_t& x, int32_t& y, int32_t& z, int8_t& s, int8_t& c) {
    int32_t temp = ( x*c - z*s ) / 128;
    z = ( x*s + z*c ) / 128;
    x = temp;
}

static inline __attribute__ ((always_inline)) void rotate_y(int32_t& x, int32_t& y, int32_t& z, const uint8_t& ang) {
    int8_t s = sin8(ang)-128;
    int8_t c = cos8(ang)-128;
    rotate_y(x, y, z, s, c);
}

static inline __attribute__ ((always_inline)) void rotate_y(int32_t p[3], const uint8_t& ang) {
    int8_t s = sin8(ang)-128;
    int8_t c = cos8(ang)-128;
    rotate_y(p[0], p[1], p[2], s, c);
}

static inline __attribute__ ((always_inline)) void rotate_y(VECTOR3& p, const uint8_t& ang) {
    int8_t s = sin8(ang)-128;
    int8_t c = cos8(ang)-128;
    rotate_y(p.x, p.y, p.z, s, c);
}

static inline __attribute__ ((always_inline)) void rotate_z(int32_t& x, int32_t& y, int32_t& z, int8_t& s, int8_t& c) {
    int32_t temp = ( x*c - y*s ) / 128;
    y = ( x*s + y*c ) / 128;
    x = temp;
}


static inline __attribute__ ((always_inline)) void rotate_z(int32_t& x, int32_t& y, int32_t& z, const uint8_t& ang) {
    int8_t s = sin8(ang)-128;
    int8_t c = cos8(ang)-128;
    rotate_z(x, y, z, s, c);
}

static inline __attribute__ ((always_inline)) void rotate_z(int32_t p[3], const uint8_t& ang) {
    int8_t s = sin8(ang)-128;
    int8_t c = cos8(ang)-128;
    rotate_z(p[0], p[1], p[2], s, c);
}

static inline __attribute__ ((always_inline)) void rotate_z(VECTOR3& p, const uint8_t& ang) {
    int8_t s = sin8(ang)-128;
    int8_t c = cos8(ang)-128;
    rotate_z(p.x, p.y, p.z, s, c);
}


#endif