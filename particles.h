#ifndef LIGHTS_PARTICLES_NOISE_H
#define LIGHTS_PARTICLES_NOISE_H

//PARTICLES

class PARTICLES: public LIGHT_SKETCH {
  public:
    PARTICLES () {setup();}
    ~PARTICLES () {}
  private:
    Z_BUF _z_buffer;
    #define NUM_PARTICLE_EFFECTS 3
    int current_effect = 2;
    struct PARTICLE {
        VECTOR3 old_pos;
        VECTOR3 pos;
        VECTOR3 spd;
        bool active;
        CRGB rgb;
    };
    #define NUM_PARTICLES 1000
    PARTICLE particles[NUM_PARTICLES];
    int current_particle = 0;

  public:
    void reset() {
        for (int i = 0; i < NUM_PARTICLES; i++) {
            particles[i].active = false;
        }
    }

    void setup() {
        z_buffer = &_z_buffer;
        led_screen.light_falloff = 10;
        for (int i = 0; i < NUM_PARTICLES; i++) {
            particles[i].active = false;
            particles[i].pos.z = 0;
            particles[i].spd.z = 0;
        }
    }

    void next_effect() {
        current_effect++;
        current_effect %= NUM_PARTICLE_EFFECTS;
    }
    void loop() {
        LED_show();

        switch (current_effect) {
            case 0:
                handle_burst();
                break;
            case 1:
                handle_falls();
                break;
            case 2:
                handle_gravity();
                break;
            default:
                break;

        }
        

    }

    void handle_falls() {
        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i].r = (leds[i].r*200)/256;
            leds[i].g = (leds[i].g*200)/256;
            leds[i].b = (leds[i].b*200)/256;
        }

        int cnt = 8;
        while (cnt--) {
            particles[current_particle].active = 1;
            particles[current_particle].pos.x = random(MATRIX_WIDTH*256);
            particles[current_particle].pos.x = MATRIX_WIDTH*128;
            particles[current_particle].pos.y = MATRIX_HEIGHT*256;
            particles[current_particle].pos.z = 0;
            VECTOR3 p;
            led_screen.matrix.rotate(particles[current_particle].pos,p);
            led_screen.perspective(p);
            particles[current_particle].old_pos = p;
            particles[current_particle].spd.x = random(-100,100);
            particles[current_particle].spd.y = random(-50,1);
            particles[current_particle].rgb = CRGB::White;
            current_particle++;
            current_particle%=NUM_PARTICLES;
        }

        for (int i = 0; i < NUM_PARTICLES; i++) {
            if (particles[i].active) {
                int cnt2 = 2;
                while (cnt2--) {
                    particles[i].pos += particles[i].spd; 
                    particles[i].spd.y -= 2;
                    if (!blendXY(led_screen, particles[i].pos, particles[i].rgb)) {
                        particles[i].active = false;
                        break;
                    }
                }
            }
        }
    }

    void handle_burst() {
        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i].r = (leds[i].r*250)/256;
            leds[i].g = (leds[i].g*250)/256;
            leds[i].b = (leds[i].b*250)/256;
        }

        int cnt = 4;
        while (cnt--) {
            uint16_t angle = random(UINT16_MAX);
            uint8_t vel = random(20,256);
            particles[current_particle].active = 1;
            particles[current_particle].pos.x = MATRIX_WIDTH*128;
            particles[current_particle].pos.y = MATRIX_HEIGHT*128;
            particles[current_particle].pos.z = 0;
            VECTOR3 p;
            led_screen.matrix.rotate(particles[current_particle].pos,p);
            led_screen.perspective(p);
            particles[current_particle].old_pos = p;
            particles[current_particle].spd.x = (sin16(angle) * vel)/INT16_MAX;
            particles[current_particle].spd.y = (cos16(angle) * vel)/INT16_MAX;
            if (random(2)) {
                particles[current_particle].spd.x*=-1;
            }
            if (random(2)) {
                particles[current_particle].spd.y*=-1;
            }
            particles[current_particle].rgb = CHSV(random(256),255,128);
            current_particle++;
            current_particle%=NUM_PARTICLES;
        }

        for (int i = 0; i < NUM_PARTICLES; i++) {
            if (particles[i].active) {
                int cnt2 = 2;
                while (cnt2--) {
                    particles[i].pos += particles[i].spd; 
                    if (!blendXY(led_screen, particles[i].pos, particles[i].rgb)) {
                        particles[i].active = false;
                        break;
                    }
                }
            }
        }

    }


    void handle_gravity() {
        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i].r = (leds[i].r*200)/256;
            leds[i].g = (leds[i].g*200)/256;
            leds[i].b = (leds[i].b*200)/256;
        }
        if (random(25)==0) {
            int cnt = 1;
            while (cnt--) {
                uint16_t angle = random(UINT16_MAX);
                uint8_t vel = random(20,256);
                particles[current_particle].active = 1;
                particles[current_particle].pos.x = 0;
                particles[current_particle].pos.y = MATRIX_HEIGHT*140;
                particles[current_particle].pos.z = 0;
                VECTOR3 p;
                led_screen.matrix.rotate(particles[current_particle].pos,p);
                led_screen.perspective(p);
                particles[current_particle].old_pos = p;
                particles[current_particle].spd.x = (sin16(angle) * vel)/INT16_MAX;
                particles[current_particle].spd.y = (cos16(angle) * vel)/INT16_MAX;
                particles[current_particle].spd.z = (cos16(angle) * vel)/INT16_MAX;
                if (random(2)) {
                    particles[current_particle].spd.x*=-1;
                }
                if (random(2)) {
                    particles[current_particle].spd.y*=-1;
                }
                particles[current_particle].rgb = CHSV(random(256),255,255);
                current_particle++;
                current_particle%=NUM_PARTICLES;
            }
        }

        for (int i = 0; i < NUM_PARTICLES; i++) {
            if (particles[i].active) {
                int cnt2 = 1;
                static VECTOR3 planet = VECTOR3(0, 0, 0);
                VECTOR3 diff = planet - particles[i].pos;
                diff /= 256;
                int32_t length = diff.unit_ip();
                int32_t d2 = _max(length*length,1);
                particles[i].spd.x += (diff.x*128)/d2;
                particles[i].spd.y += (diff.y*128)/d2;
                particles[i].spd.z += (diff.z*128)/d2;
                while (cnt2--) {
                    particles[i].pos += particles[i].spd; 
                    VECTOR3 p;

                    //rotate with our global matrix
                    led_screen.matrix.rotate(particles[i].pos, p);
                    //translate vectors to coordinates
                    //scale_z(p);
                    //correct 3d perspective
                    led_screen.perspective(p);
                    uint16_t dist = _max(abs(p.x-particles[i].old_pos.x), abs(p.y-particles[i].old_pos.y));
                    uint8_t dist_b = dist >> 8;
                    // CRGB rgb = particles[i].rgb;
                    // rgb/=dist;
                    // gamma8_encode(rgb);
                    if (dist_b) {
                        uint16_t r = particles[i].rgb.r<<8;
                        uint16_t g = particles[i].rgb.g<<8;
                        uint16_t b = particles[i].rgb.b<<8;
                        r/=dist;
                        g/=dist;
                        b/=dist;
                        CRGB rgb = CRGB(gamma8_encode(r),gamma8_encode(g),gamma8_encode(b));
                        draw_line_fine(led_screen, p, particles[i].old_pos, rgb, -10000, 255, 255, true);
                    } else {
                        blendXY(led_screen, p, particles[i].rgb); 
                    }
                    particles[i].old_pos = p;
                    //if (p.x < -64*256 || p.x > MATRIX_WIDTH*256*2 || p.y < -64*256 || p.y > MATRIX_HEIGHT*256*2 || length < 8) {
                    if (length < 4) {
        
                        particles[i].active = false;
                        break;
                    }
                }
            }
        }

        VECTOR3 planet = VECTOR3(0, 0, 0);
        static uint32_t cnt = random(UINT32_MAX);
        cnt++;
        uint8_t r0 = inoise8(0,0,cnt*17);
        uint8_t r1 = inoise8(0,cnt*11,0);
        uint8_t r2 = inoise8(cnt*37,0,0);
        r0 = (r0*r0)/255;
        r1 = (r1*r1)/255;
        r2 = (r2*r2)/255;
        VECTOR3 planet_r = VECTOR3(384+r0+r1+r2, 0, 0);
        //rotate with our global matrix
        led_screen.matrix.rotate(planet);
        //correct 3d perspective
        led_screen.perspective(planet);
        led_screen.perspective_zero(planet_r.x,planet_r.y,planet_r.z);

        reset_y_buffer();
        reset_x_buffer();

        draw_circle_fine(planet.x, planet.y, planet_r.x,32,128,255,-1,16,planet.z);
        CRGB rgb = CHSV(32,128,255);
        fill_shape(planet.z+256, rgb);

    }

};

LIGHT_SKETCHES::REGISTER<PARTICLES> particles("particles");

#endif