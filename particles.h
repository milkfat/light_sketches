#ifndef LIGHTS_PARTICLES_NOISE_H
#define LIGHTS_PARTICLES_NOISE_H

//PARTICLES

class PARTICLES: public LIGHT_SKETCH {
  public:
    PARTICLES () {setup();}
    ~PARTICLES () {}
  private:
    #define NUM_PARTICLE_EFFECTS 3
    int current_effect = 2;
    struct PARTICLE {
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
        if (random(100)==0) {
        int cnt = 1;
        while (cnt--) {
            uint16_t angle = random(UINT16_MAX);
            uint8_t vel = random(20,256);
            particles[current_particle].active = 1;
            particles[current_particle].pos.x = 0;
            particles[current_particle].pos.y = MATRIX_HEIGHT*140;
            particles[current_particle].pos.z = 0;
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
                    scale_z(p);
                    //correct 3d perspective
                    led_screen.perspective(p);

                    blendXY(led_screen, p, particles[i].rgb); 
                    if (p.x < -64*256 || p.x > MATRIX_WIDTH*256*2 || p.y < -64*256 || p.y > MATRIX_HEIGHT*256*2 || length < 8) {
        
                        particles[i].active = false;
                        break;
                    }
                }
            }
        }

        static VECTOR3 planet = VECTOR3(0, 0, 0);
        static VECTOR3 p;
        //rotate with our global matrix
        led_screen.matrix.rotate(planet, p);
        //translate vectors to coordinates
        scale_z(p);
        //correct 3d perspective
        led_screen.perspective(p);

        draw_circle_fine(p.x, p.y, 768);
        

    }

};

LIGHT_SKETCHES::REGISTER<PARTICLES> particles("particles");

#endif