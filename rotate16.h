#ifndef LIGHTS_ROTATE16_H
#define LIGHTS_ROTATE16_H


static inline __attribute__ ((always_inline)) void rotate16_x(int32_t& x, int32_t& y, int32_t& z, int16_t& s, int16_t& c) {
    int32_t temp = ( y*c - z*s ) / 32768;
    z = ( y*s + z*c ) / 32768;
    y = temp;
}

static inline __attribute__ ((always_inline)) void rotate16_x(int32_t p[3], const uint16_t& ang) {
    int16_t s = sin16(ang);
    int16_t c = cos16(ang);
    rotate16_x(p[0], p[1], p[2], s, c);
}

static inline __attribute__ ((always_inline)) void rotate16_x(VECTOR3& p, const uint16_t& ang) {
    int16_t s = sin16(ang);
    int16_t c = cos16(ang);
    rotate16_x(p.x, p.y, p.z, s, c);
}


static inline __attribute__ ((always_inline)) void rotate16_y(int32_t& x, int32_t& y, int32_t& z, int16_t& s, int16_t& c) {
    int32_t temp = ( x*c - z*s ) / 32768;
    z = ( x*s + z*c ) / 32768;
    x = temp;
}

static inline __attribute__ ((always_inline)) void rotate16_y(int32_t& x, int32_t& y, int32_t& z, const uint16_t& ang) {
    int16_t s = sin16(ang);
    int16_t c = cos16(ang);
    rotate16_y(x, y, z, s, c);
}

static inline __attribute__ ((always_inline)) void rotate16_y(int32_t p[3], const uint16_t& ang) {
    int16_t s = sin16(ang);
    int16_t c = cos16(ang);
    rotate16_y(p[0], p[1], p[2], s, c);
}

static inline __attribute__ ((always_inline)) void rotate16_y(VECTOR3& p, const uint16_t& ang) {
    int16_t s = sin16(ang);
    int16_t c = cos16(ang);
    rotate16_y(p.x, p.y, p.z, s, c);
}

static inline __attribute__ ((always_inline)) void rotate16_z(int32_t& x, int32_t& y, int32_t& z, int16_t& s, int16_t& c) {
    int32_t temp = ( x*c - y*s ) / 32768;
    y = ( x*s + y*c ) / 32768;
    x = temp;
}


static inline __attribute__ ((always_inline)) void rotate16_z(int32_t& x, int32_t& y, int32_t& z, const uint16_t& ang) {
    int16_t s = sin16(ang);
    int16_t c = cos16(ang);
    rotate16_z(x, y, z, s, c);
}

static inline __attribute__ ((always_inline)) void rotate16_z(int32_t p[3], const uint16_t& ang) {
    int16_t s = sin16(ang);
    int16_t c = cos16(ang);
    rotate16_z(p[0], p[1], p[2], s, c);
}

static inline __attribute__ ((always_inline)) void rotate16_z(VECTOR3& p, const uint16_t& ang) {
    int16_t s = sin16(ang);
    int16_t c = cos16(ang);
    rotate16_z(p.x, p.y, p.z, s, c);
}

template <class T, class U>
static inline __attribute__ ((always_inline)) void rotate16(VECTOR3_CLASS<T>& p, VECTOR3_CLASS<U>& r) {
    rotate16_x(p, r.x);
    rotate16_y(p, r.y);
    rotate16_z(p, r.z);
}


#endif