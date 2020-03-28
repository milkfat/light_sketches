#ifndef LIGHTS_SCALE_H
#define LIGHTS_SCALE_H


static int z_scaler;
uint8_t debug_scaler = 128;

static inline __attribute__ ((always_inline)) void scale_update() {
    static uint32_t update_time = 0;
    if (millis() - 16 > update_time) {
        update_time = millis();

        z_scaler = -100 * 256 + (200 * 256 * debug_scaler) / 256;

    }
}

static inline __attribute__ ((always_inline)) void scale_z(int32_t& z) {
    //z += z_scaler;
}

static inline __attribute__ ((always_inline)) void scale_z(int32_t p[3]) {
    //scale_z(p[2]);
}

static inline __attribute__ ((always_inline)) void scale_z(VECTOR3& p) {
    //scale_z(p.z);
}


static inline __attribute__ ((always_inline)) void reverse_scale_z(int32_t& z) {
    //z -= z_scaler;
}

static inline __attribute__ ((always_inline)) void reverse_scale_z(int32_t p[3]) {
    //reverse_scale_z(p[2]);
}

static inline __attribute__ ((always_inline)) void reverse_scale_z(VECTOR3& p) {
    //reverse_scale_z(p.z);
}

#endif