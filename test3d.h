
//3D

#include "phos.h"

int skipped_frames = 0;
class TEST3D: public LIGHT_SKETCH {
  public:
    TEST3D () {setup();}
    ~TEST3D () {}
  private:
    //0 = test object
    //1 = grid
    //2 = snow
    //3 = spiral
    //4 = box
    //5 = tunnel
    //6 = firework!
    enum effects {
      TEST_OBJECT,
      GRID,
      SNOW,
      SPIRAL,
      BOX,
      TUNNEL,
      FIREWORK,
      NUM_EFFECTS
    };
    
    enum grid_types {
      GRID_RED,
      GRID_PHOS,
      GRID_NOISE,
      NUM_GRID_TYPES
    };

    float compress(float value) {
      //return FC.compress(value);
      return value;
    }

    float decompress(float value) {
      //return FC.decompress(value);
      return value;
    }

    uint8_t grid_type = GRID_RED;

    uint8_t current_variation = BOX;

    uint16_t frame_count = 2100;

    bool do_not_update = 0;

    int32_t square[4][3] = {
      { -3 * 256, -4 * 256, 0},
      { -3 * 256, 4 * 256, 0},
      {3 * 256, 4 * 256, 0},
      {3 * 256, -4 * 256, 0}
    };

    int32_t spiral[12][3] = {
      { -8 * 256L, 0, 0},
      {0, 8 * 256L, -8 * 256L},
      {8 * 256L, 16 * 256L, 0},
      {0, 24 * 256L, 8 * 256L},
      { -8 * 256L, 32 * 256L, 0},
      {0, 40 * 256L, -8 * 256L},
      {8 * 256L, 48 * 256L, 0},
      {0, 56 * 256L, 8 * 256L},
      { -8 * 256L, 64 * 256L, 0},
      {0, 72 * 256L, -8 * 256L},
      {8 * 256L, 80 * 256L, 0},
      {0, 88 * 256L, 8 * 256L}
    };
    int32_t square_curvetest[4][3] = {
      { -18 * 256L, -18 * 256L, 0},
      { 18 * 256L, -18 * 256L, 0},
      { 18 * 256L, 18 * 256L, 0},
      { -18 * 256L, 18 * 256L, 0}
    };


    int32_t linex[2][3] = {
      {0, 0, 0},
      {8 * 256, 0, 0}
    };

    int32_t liney[2][3] = {
      {0, 0, 0},
      {0, 8 * 256, 0}
    };


    int32_t linez[2][3] = {
      {0, 0, 0},
      {0, 0, 8 * 256}
    };
    //
    int32_t square2[4][3];

    void rotate(int32_t in[3], int32_t out[3]) {
      matrix.rotate(in, out);
    }

    void rotate(VECTOR3& p) {
      matrix.rotate(p);
    }


    struct PARTICLE {
        //const unsigned char resistsCold      = 1 << 0;
        //const unsigned char resistsTheft     = 1 << 1;
        //const unsigned char resistsAcid      = 1 << 2;
        //myflags |= option4;              // turn option 4 on
        //myflags |= (option4 | option5);  //turn option 4 and 5 on at the same time
        //myflags &= ~option4;             // turn option 4 off
        //myflags &= ~(option4 | option5); //turn option 4 and 5 off at the same time
        //myflags ^= option4;              // flip option4 from on to off, or vice versa
        //myflags ^= (option4 | option5);  // flip options 4 and 5 at the same time
        //if (myflags & option4)           //myflags has option 4 set
        //if (!(myflags & option5))        //myflags does not have option 5 set
      
        int16_t x;  //positional coordinates
        int16_t y;
        int16_t z;
        int16_t vx; //velocities
        int16_t vy;
        int16_t vz;
        uint8_t attributes = 0; //index of shared attributes (shared by other particles) such as hue, saturation, value, mass, radius... etc.
        uint8_t function = 0;   //index of the function that will update this particle
        int16_t age = 256*8;    //age of the particle

    };



    struct PARTICLE_ATTRIBUTES {
        uint32_t m = 64;        //mass
        uint32_t r = 3;         //radius
        uint8_t h = 255;       //hue
        uint8_t s = 255;       //saturation
        uint8_t v = 255;       //value (brightness)
        uint8_t trail = 0;     // trail > 0 = This particle will generate a trail of particles. The trail particles will be defined by this attribute index.
        uint8_t available = 1;
    };

  private:

  #define NUM_PARTICLES 1000
  PARTICLE particles[NUM_PARTICLES];
  
  PARTICLE * current_particle() {
    static uint16_t cp = 0;
    uint16_t cnt = 0;
    PARTICLE * cpo = nullptr;
    while (cnt < NUM_PARTICLES) {
      cp++;
      if (cp > NUM_PARTICLES-1) {
        cp = 0;
      }
      if (!particles[cp].function) {
        cpo = &particles[cp];
        break;
      }
      cnt++;
    }
    return cpo;
  }


  //create an array for storing attributes that are shared between multiple active particles
  #define NUM_PARTICLE_ATTRIBUTES 64
  PARTICLE_ATTRIBUTES particle_attributes[NUM_PARTICLE_ATTRIBUTES];
  
  uint16_t current_particle_attributes() {
    static uint16_t cpa_index = 0;
    uint16_t cnt = 0;
    while (cnt < NUM_PARTICLE_ATTRIBUTES-1) {
      cpa_index++;
      if (cpa_index > NUM_PARTICLE_ATTRIBUTES-1) {
        cpa_index = 0;
      }
      if (true/*particle_attributes[cpa_index].available*/) {
        particle_attributes[cpa_index].available = 0;
        break;
      }
      cnt++;
    }
    return cpa_index;
  }


    int32_t update_time = millis();

  public:
    void next_effect() {
      if (current_variation == GRID) {
        grid_type++;
        if (grid_type == NUM_GRID_TYPES) {
          grid_type = 0;
          current_variation++;
        }
      } else {
        current_variation++;
      }
      current_variation %= NUM_EFFECTS;


      //reset rotation
      reset();

      //snow
      if (current_variation == SNOW) {
        for (int i = 0; i < NUM_PARTICLES; i++) {
          PARTICLE * cp = &particles[i];
          cp->x = random(-30 * 256L, 30 * 256L);
          cp->y = random(-148 * 256L, 148 * 256L);
          cp->z = random(-148 * 256L, 148 * 256L);
          cp->vx = random(-255, 256);
          cp->vy = -255;
          cp->vz = random(-255, 256);
        }
      }

      //fireworks
      if (current_variation == FIREWORK) {
        for (int i = 0; i < NUM_PARTICLES; i++) {
          particles[i].function = 0;
        }
      }

    }

    //burst into colors!
    void spawn_firework_burst(const int16_t& start_x, const int16_t& start_y, const int16_t& start_z, const int16_t& start_vx, const int16_t& start_vy, const int16_t& start_vz) {
      
     
      //adjust the minimum/maximum size of the burst based on the height of the shell
      //255 = big
      //15 = small
      //24000 = high = (25,35);
      //-10000 = low (10,15);
      int16_t range_min = ((start_y - (-10000) ) * (10-0)) / (24000 - (-10000));
      int16_t range_max = ((start_y - (-10000) ) * (25-2)) / (24000 - (-10000));
      range_min = range_min*range_min/(10-0);
      range_max = range_max*range_max/(25-2);

      //choose a random size within our minimum/maximum boundaries
      uint16_t radius = random(range_min,range_max);
      uint16_t burst_size = radius+35; //35 is the minimum for the current physics calculations
      //std::cout << "range_min: " << range_min << " range_max: " << range_max << " radius: " << radius << " burst_size: " << burst_size << "\n";


      //create particle attributes for our burst
      PARTICLE_ATTRIBUTES * a[4];
      uint16_t ai[4];

      //random hue
      uint8_t h = random(256);

      //default attributes
      for (int i = 0; i < 4; i++) {
        ai[i] = current_particle_attributes();
        a[i] = &particle_attributes[ai[i]];
        a[i]->h = h;
        a[i]->s = 255;
        a[i]->v = 255;
        a[i]->trail = 0;
        a[i]->r = burst_size;
        a[i]->m = burst_size*50-500;
      }
      
      //attributes for alternate color
      a[1]->h += 64;
      
      //attributes for stars that generate trail particles
      a[2]->s = 32;
      a[2]->m += (2+radius)*50;
      a[2]->trail = ai[3];
      
      //attributes for the trail particles
      a[3]->s = 32;
      a[3]->m = 20*50;
      a[3]->r = 4;


      //number of particles (stars) is based on size
      //number of particles is higher for larger bursts
      uint16_t np = (radius*radius)/28; //square the number of particles per radius
      
      //calculate how far to step along the z-axis
      //in other words: this defines the number of slices into which we will split our sphere
      //small sphere, large z_step, lower number of slices
      //big sphere, small z_step, higher number of slices
      uint16_t z_step = 4500-(np*3000)/28;
      //std::cout << "np: " << np << " z_step: " << z_step << "\n";

      //calculate how many stars each slice will get (number of stars in a circle)
      uint16_t npxy = 15+(np*15)/20;

      //random rotation angles for our sphere
      uint16_t r0 = random(0,65535);
      uint16_t r1 = random(0,65535);

      //iterate over the z-axis
      for (uint16_t iz = 0; iz < 32767; iz+=z_step) {
        //calculate our Z-velocity (you could pretend that this is a coordinate)
        int16_t vz = cos16(iz); //distribution from 0 - 32767
        //calculate the radius of this slice
        int16_t r = abs(sin16(iz));
        //calculate the number of particles for this slice (based on absolute size of the shell combined with the relative radius size)
        int16_t particles_this_ring = (r*npxy)/32767;

        //iterate around the circumference of the slice
        for (int i = 0; i < particles_this_ring; i++) {
          //find the next available particle object
          PARTICLE * cp = current_particle();
          //create particle if an object is available
          if(cp) {
            cp->function = 1;
            //initial starting position
            cp->x = start_x/2;
            cp->y = start_y/2;
            cp->z = start_z/2;

            //distribute particles evenly in a ring, X and Y
            int16_t angle = (i*65535)/particles_this_ring;
            angle = (angle*random(90,110))/100; //randomize the angle a bit
            //calculate the X and Y components of the particle for this slice
            int16_t vx = sin16(angle);
            int16_t vy = cos16(angle);
            
            //add the Z component
            int16_t riz = (iz*random(85,116))/100; //randomize the angle a bit
            vz = cos16(riz);
            //calculate the XY component (radius) for this Z
            int16_t vxy = abs(sin16(riz));
            
            //calculate the final X and Y velocities (positions)
            //at this point we have the particles evenly distributed around a sphere
            cp->vx = (vx*vxy)/32768;
            cp->vy = (vy*vxy)/32768;
            cp->vz = vz;

            //rotate the sphere around x-axis
            int16_t rz = (cp->vz*cos16(r0))/32768 - (cp->vy*sin16(r0))/32768;
            int16_t ry = (cp->vy*cos16(r0))/32768 + (cp->vz*sin16(r0))/32768;
            cp->vz = rz;
            cp->vy = ry;

            //rotate the sphere around z-axis
            int16_t rx = (cp->vx*cos16(r1))/32768 - (cp->vy*sin16(r1))/32768;
                    ry = (cp->vy*cos16(r1))/32768 + (cp->vx*sin16(r1))/32768;
            cp->vx = rx;
            cp->vy = ry;

            //apply speed divisor to change the overall burst size
            // cp->vx /= (15*20);
            // cp->vy /= (15*20);
            // cp->vz /= (15*20);
            //std::cout << " x: " << cp->vx << " y: " << cp->vy << "  z: " << cp->vx << "\n";

            //default color
            cp->age = 0;
            cp->attributes = ai[0];
            
            //alternate color for one hemisphere
            if (vz < 0) {
              cp->attributes = ai[1];
            }
            //modify a ring of particles to produce trails
            if(abs(iz - 16000) < 1000) {
              cp->attributes = ai[2];
            }
            
            //cp->age = random(-20,20);
            
            /*
            //pistil (smaller inner sphere)
            if (random(3) == 0) {
              cp->vx /= 6;
              cp->vy /= 6;
              cp->vz /= 6;
              cp->h += 127;
              cp->age = -100;
            }
            */

            //finally, add the velocity of the shell that produced this burst
            cp->vx += start_vx;
            cp->vy += start_vy;
            cp->vz += start_vz;
            



          }
        }
      }
    }

    struct FIREWORK_SHELL {
      bool active = 0;
      int32_t x;
      int32_t y;
      int32_t z;
      float vx;
      float vy;
      float vz;
      uint8_t trail;
      uint16_t mass;
      uint16_t age;
    };

    #define NUM_FIREWORK_SHELLS 8
    FIREWORK_SHELL firework_shells[NUM_FIREWORK_SHELLS];
    uint8_t current_firework_shell = 0;

    //located the next available shell
    //return nullptr if none are available
    FIREWORK_SHELL * next_firework_shell() {
      uint8_t cnt = 0;
      while(cnt < NUM_FIREWORK_SHELLS) {
          current_firework_shell++;
          if(current_firework_shell > NUM_FIREWORK_SHELLS-1) {
            current_firework_shell = 0;
          }
          if(!firework_shells[current_firework_shell].active) {
            return &firework_shells[current_firework_shell]; 
          }
          cnt++;
      }
      return nullptr;
    }

    //launch a shell into the air
    void spawn_firework_shell() {
      FIREWORK_SHELL * fs = next_firework_shell();
      if(fs) {
        fs->age = 0;
        fs->x = 0;
        int32_t p[3] = {(MATRIX_WIDTH*256)/2,0,0};
        matrix.reverse_perspective(p);
        fs->y = p[1]; //start "on the ground"
        fs->z = 0;
        uint8_t r = random(105);
        fs->vy = r+200; //random launch velocity (burst height), weighted toward lower bursts
        uint8_t r2 = (105-r)/6;
        fs->vx = random(-(r2+10),(r2+10)); //random side-to-side velocity
        //fs->vy = 300;
        fs->vz = random(-(r2+10),(r2+10));
        //fs->vz = random(-30,30); //random front-to-back velocity
        fs->active = true;
        fs->trail = current_particle_attributes();
         //define trail particle attributes
        particle_attributes[fs->trail].h = 32;
        particle_attributes[fs->trail].s = 64;
        particle_attributes[fs->trail].v = random(32,64);
        particle_attributes[fs->trail].r = 4;
        particle_attributes[fs->trail].m = 20*50;
        particle_attributes[fs->trail].trail = 0;


        //LIFT CHARGE PARTICLES
        uint8_t lift_attribute = current_particle_attributes();
         //define life particle attributes
        particle_attributes[lift_attribute].h = 8;
        particle_attributes[lift_attribute].s = 255;
        particle_attributes[lift_attribute].v = 255;
        particle_attributes[lift_attribute].r = 20;
        particle_attributes[lift_attribute].m = ((r*r)/(2*105)+20)*50;
        particle_attributes[lift_attribute].trail = 0;

        for (int i = 0; i < (r*r)/(2*105)+20; i++) {
          //find the next available particle object
          PARTICLE * cp = current_particle();
          if (cp) {
            cp->x = 0;
            cp->y = p[1]/2;
            cp->z = 0;
            cp->vx = random(-20*256,20*256);
            cp->vy = random(64*256, 127*256);
            int rv = random(256);
            cp->vx = (cp->vx*rv)/256;
            cp->vy = (cp->vy*rv)/256;
            cp->vz = 0;
            cp->attributes = lift_attribute;
            cp->function = 2;
            cp->age = random(1*256,3*256);
          }
        }
        
      }
      
    }

    //handle each active shell
    void handle_firework_shells() {
      for (int i = 0; i < NUM_FIREWORK_SHELLS; i++) {
        if (firework_shells[i].active) {
          FIREWORK_SHELL * fs = &firework_shells[i];
          fs->age++;

          //burst at peak height
          if (firework_shells[i].vy < -5) {
            fs->active = false;
            //create a burst using this shell's final position and velocity
            spawn_firework_burst(fs->x,fs->y,fs->z,fs->vx,fs->vy,fs->vz);
          }

          //update active shells
          if (fs->active && fs->y < 120*256) {

            if(fs->age > 10) {
              //add random spark particles to create a tail for the rising shell
              PARTICLE * cp = current_particle();
              if( cp && random(3) == 0) {
                cp->x = fs->x/2;
                cp->y = fs->y/2;
                cp->z = fs->z/2;
                cp->vx = fs->vx;
                cp->vy = fs->vy;
                cp->vz = fs->vz;
                cp->age = 32*8;
                cp->function = 1;
                cp->attributes = fs->trail;

              }
            }

            //update the shell's position
            fs->x += fs->vx;
            fs->y += fs->vy;
            fs->z += fs->vz;

            //apply gravity
            fs->vy -= .5;
            //calculate the velocity of the shell
            float velocity = sqrt(fs->vx*fs->vx+fs->vy*fs->vy+fs->vz*fs->vz);
            
            //calculate the amount of force based on velocity (wind resistance)
            //smaller numbers represent higher amounts of force
            float velocity_force = (200000 - velocity)/200000;
            
            //apply the force to our particle
            //fs->vx *= velocity_force;
            fs->vy -= fs->vy*(1.f-velocity_force);

            //fs->vz *= velocity_force;
          }
        }
      }
    }

    void move_particle(PARTICLE &p) {
      if
      (
      abs(p.x) > 127 * 256L ||
      abs(p.y) > 127 * 256L ||
      abs(p.z) > 127 * 256L
      ) {
        p.function = 0;
        return;
      }
      PARTICLE_ATTRIBUTES * pa = &particle_attributes[p.attributes];
              //move our particles
      int32_t avx = p.vx;
      int32_t avy = p.vy;
      int32_t avz = p.vz;
      p.x += p.vx/100;
      p.y += p.vy/100;
      p.z += p.vz/100;

      int32_t velocity_squared = avx*avx + avy*avy + avz*avz;

      //F = ma
      //a = F/m
      //a = ((r^2)(v^2))/(r^3)
      //I cancelled r^2 from force and mass
      //a = v^2/r 
      int32_t acceleration = velocity_squared/(pa->r*pa->m);
      //acceleration /= 6400;

      //vector magnitude
      int32_t magnitude = sqrt(velocity_squared);

      //apply force to find our new magnitude
      int32_t new_magnitude = magnitude - acceleration/2;
      //std::cout << new_magnitude << "\n";

      //find the ratio between old and new magnitudes to apply to our x,y,z velocity components

      
      if (magnitude != 0) {
        if (new_magnitude > 0) {
          p.vx = (p.vx*new_magnitude)/magnitude;
          p.vy = (p.vy*new_magnitude)/magnitude;
          p.vz = (p.vz*new_magnitude)/magnitude;
          //p.vx = (p.vx*999)/1000;
          //p.vy = (p.vy*999)/1000;
          //p.vz = (p.vz*999)/1000;
        } else if (new_magnitude <= 0) {
          p.vx /= 20;
          p.vy /= 20;
          p.vz /= 20;
        }
      }
          
      //apply gravity
      p.vy -= 3;
      //apply wind
      p.vx -= 3;
      p.vz -= 3;

    } //move_particle()

    void star_particle(PARTICLE &p) {

      //increment the particle's age
      p.age+=4;

      if (p.age > 256*8) {p.function = 0; return;}
      PARTICLE_ATTRIBUTES * pa = &particle_attributes[p.attributes];

      move_particle(p);
            
      if (pa->trail) {
        //add random star trail particles
        PARTICLE * cp = current_particle();
        if( cp && random(p.age/32+5) == 0) {
          cp->x = p.x;
          cp->y = p.y;
          cp->z = p.z;
          cp->vx = p.vx;
          cp->vy = p.vy;
          cp->vz = p.vz;
          cp->age = p.age/16;
          cp->function = 1;
          cp->attributes = pa->trail;
        }
      }
 
    } //star_particle()



    void lift_particle(PARTICLE &p) {
      //increment the particle's age
      p.age+=8;
      if (p.age > 256*8) {p.function = 0; return;}
      PARTICLE_ATTRIBUTES * pa = &particle_attributes[p.attributes];
      move_particle(p);
 
    } //lift_particle()


    //here we call the handling function for this particle
    void handle_particles() {
      int active_particles = 0;
      for (int i = 0; i < NUM_PARTICLES; i++) {
        if (particles[i].function) {
          active_particles++;
          (this->*particle_funcs[particles[i].function])(particles[i]);
        }
      }
    }

    //an array of function pointers to functions that modify the PARTICLE structure
    //referred to as a jump table?
    void (TEST3D::*particle_funcs[3])(PARTICLE&) = {nullptr,&TEST3D::star_particle,&TEST3D::lift_particle};

    //render the particles in 3D to the buffer
    void draw_particles() {

      //clear the LED buffer
      LED_black();

      //draw the particles
      for (int i = 0; i < NUM_PARTICLES; i++) {
        PARTICLE * cp = &particles[i];
        if ( cp->function) {
          PARTICLE_ATTRIBUTES * pa = &particle_attributes[cp->attributes];
          uint16_t age = cp->age/8;
          //darken (v = HSV value)
          int16_t v_temp = 255;
          uint8_t r = fmix32(i);
          r = r/4 + (_max(pa->r-35,0))+32;
          if (age > r) {
            v_temp -= (age-r)*4;
          }
          uint8_t v = _min(_max(v_temp,0), pa->v);
          if (v == 0) {
            cp->function = 0;
            continue;
          }
          //colors become more saturated over time
          uint8_t s = _max(_min(cp->age,pa->s),0);
          
          //create a three-dimensional structure that will be rotated
          VECTOR3 p(cp->x*2,cp->y*2,cp->z*2);

          //rotate all particles using our orientation matrix
          rotate(p);

          //translate vectors to coordinates
          matrix.scale_z(p[2]);

          //correct 3d perspective
          if (matrix.perspective(p)) {
            int16_t v_temp2 = (p[2] + 200 * 256L) / 256L;
            blendXY(leds, p[0], p[1], pa->h, s, (_max(_min(v_temp2, 255), 0)*v)/255);
          }
        }
      }
    }


    #define GRID_SIZE 20
    CHSV grid_hsv[GRID_SIZE*2+1][GRID_SIZE*2+1];

    void grid_hsv_v(const int& x, const int& y, int32_t& var, int& cnt) {
      if (x >= 0 && x < GRID_SIZE*2+1 && y >= 0 && y < GRID_SIZE*2+1) {
        var += grid_hsv[x][y].v;
        cnt++;
      }
    }

    PHOS phos;

    void setup() {
      //particles = (PARTICLE*) malloc (NUM_PARTICLES * sizeof(PARTICLE));
      for (int i = 0; i < NUM_PARTICLES; i++) {
        particles[i].x = random(-30 * 256L, 30 * 256L);
        particles[i].y = -127*256;
        particles[i].z = random(-148 * 256L, 148 * 256L);
        particles[i].vx = random(-255, 256);
        particles[i].vy = -255;
        particles[i].vz = random(-255, 256);
      }

      reset();

      phos.setup();

    }

    void reset() {
      rotation_alpha = 0;
      rotation_beta = 90;
      rotation_gamma = 0;
    }

    void loop() {

      // debug
      // static int asdf = 0;
      // if (asdf == 0) {
      //   asdf++;
      //   int32_t test_num = 1;

      //   for (int i = 0; i < 32; i++ ) {
      //     PARTICLE p;
      //     int32_t boo = test_num;
      //     p.set_vx(boo);
      //     int32_t raw_vx = p.get_vx_raw();
      //     std::cout << "\n\n\noriginal boo: " << boo << " vx: " << p.vx << " get_vx(): " << p.get_vx() << " /p\n";
      //     raw_vx /= 2;
      //     boo /= 2;
      //     p.set_vx_raw(raw_vx);
      //     std::cout << "updated  boo: " << boo << " vx: " << p.vx << " get_vx(): " << p.get_vx() << " /p\n";
      //     test_num*= 2;
      //   }

      // }
      
      if (effect_beat == 1) {
        effect_beat = 0;
        //reset();
      }
      if (millis() - 16 > update_time) {
        update_time = millis();


        static uint8_t cube_step = 0;
        uint16_t cube_size = 80*sin8(cube_step)+30*256;

     //uint32_t debug_time2 = micros();
        switch (current_variation)
        {
          case TEST_OBJECT:
            handle_test_object();
            break;
          case GRID:
            handle_grid();
            break;
          case SNOW:
            handle_snow();
            break;
          case SPIRAL:
            handle_spiral();
            break;
          case BOX:
            cube_ang = 0;
            cube_ang2 = 0;
            cube_ang3 = 0;
            draw_cube(VECTOR3(0,cube_size,0),15*256,15*256,15*256);
            draw_cube(VECTOR3(0,-cube_size,0),15*256,15*256,15*256);
            draw_cube(VECTOR3(cube_size,0,0),15*256,15*256,15*256);
            draw_cube(VECTOR3(-cube_size,0,0),15*256,15*256,15*256);
            draw_cube(VECTOR3(0,0,cube_size),15*256,15*256,15*256);
            draw_cube(VECTOR3(0,0,-cube_size),15*256,15*256,15*256);
            cube_step +=1;
            rotation_alpha += .1;
            rotation_beta += .077;
            rotation_gamma += .068;
            //handle_cube();
            break;
          case TUNNEL:
            handle_tunnel();
            break;
          case FIREWORK:
            handle_fireworks();
            break;
        }
     //debug_micros1 += micros() - debug_time2;

        //update the display
        if (!do_not_update) {
          skipped_frames--;
          //update LEDS
          LED_show();
          //clear LED buffer
          LED_black();
        }

      }
    } //loop();



void handle_test_object() {

          //3D TEST OBJECT
          for (int i = 0; i < 4; i++) {
            rotate(square[i], square2[i]);
          }

          for (int i = 0; i < 4; i++) {
            draw_line_fine(leds, square2[i][0] + 3 * 256, square2[i][1] + 50 * 256, square2[(i + 1) % 4][0] + 3 * 256, square2[(i + 1) % 4][1] + 50 * 256);
          }

          int32_t r0[3];
          int32_t r1[3];
          rotate(linex[0], r0);
          rotate(linex[1], r1);

          draw_line_fine(leds, r0[0] + 3 * 256, r0[1] + 50 * 256, r1[0] + 3 * 256, r1[1] + 50 * 256, 160);


          rotate(liney[0], r0);
          rotate(liney[1], r1);

          draw_line_fine(leds, r0[0] + 3 * 256, r0[1] + 50 * 256, r1[0] + 3 * 256, r1[1] + 50 * 256, 48);


          rotate(linez[0], r0);
          rotate(linez[1], r1);

          draw_line_fine(leds, r0[0] + 3 * 256, r0[1] + 50 * 256, r1[0] + 3 * 256, r1[1] + 50 * 256, 96);

          rotation_alpha += 2;
          rotation_beta += .154;
          rotation_gamma += .136;

} //handle_test_object()




void handle_grid() {


          //GRID

/*
          //DISSOLVE EFFECT
          static bool reset = 1;
          static uint8_t d = 1;
          if (reset) {
            reset = 0;
            d++;
            d%=2;
            for (int x = 0; x < GRID_SIZE*2+1;x++) {
              for (int y = 0; y < GRID_SIZE*2+1; y++) {
                if (!(x == 20 && y == 20)) {
                  grid_hsv[x][y].h=0;
                  grid_hsv[x][y].s=255;
                  grid_hsv[x][y].v=255-(d*255);
                } else {
                  grid_hsv[x][y].v=d*255;
                }
              }
            }
          }
          reset = 1;
          for (int x = 0; x < GRID_SIZE*2+1;x++) {
              for (int y = 0; y < GRID_SIZE*2+1; y++) {
                if (grid_hsv[x][y].v != d*255) {
                  reset = 0;
                  uint8_t up = 0;
                  uint8_t down = 0;
                  uint8_t left = 0;
                  uint8_t right = 0;
                  
                  if (x > 0) {
                    left = 1;
                  }
                  if (x < GRID_SIZE*2) {
                    right = 1;
                  }
                  if (y > 0) {
                    down = 1;
                  }
                  if (y < GRID_SIZE*2) {
                    up = 1;
                  }

                  if
                  (
                    (
                      grid_hsv[x][y-down].v == d*255 ||
                      grid_hsv[x][y+up].v == d*255 ||
                      grid_hsv[x+right][y].v == d*255 ||
                      grid_hsv[x-left][y].v == d*255 ||
                      grid_hsv[x+right][y-down].v == d*255 ||
                      grid_hsv[x+right][y+up].v == d*255 ||
                      grid_hsv[x-left][y-down].v == d*255 ||
                      grid_hsv[x-left][y+up].v == d*255
                    )
                    && random(60) == 0
                  ) {
                    grid_hsv[x][y].v = d*255;
                  }
                
                }
              }
            }
          //END DISSOLVE EFFECT
*/
/*
          //CRAWLER EFFECT
          #define NUM_CRAWLERS 9
          static uint8_t crawlers[NUM_CRAWLERS][3] = 
          {
            {1,6,130},
            {11,2,0},
            {20,6,48},
            {17,17,210},
            {7,14,160},
            {3,19,96},
            {3,19,24},
            {3,19,70},
            {3,19,185}
          };

          for (int i = 0;i < NUM_CRAWLERS; i++) {
            //place crawlers on the grid
            grid_hsv[crawlers[i][0]][crawlers[i][1]].h = crawlers[i][2];
            grid_hsv[crawlers[i][0]][crawlers[i][1]].v = 255;
            grid_hsv[crawlers[i][0]][crawlers[i][1]].s = 255;

            //move the crawlers
            if (random(2)) {
              if (random(2)) {
                if (crawlers[i][0] < GRID_SIZE*2) {
                  crawlers[i][0] += 1;
                }
              } else {
                if (crawlers[i][0] > 0) {
                  crawlers[i][0] -= 1;
                }
              }
            } else {
              if (random(2)) {
                if (crawlers[i][1] < GRID_SIZE*2) {
                  crawlers[i][1] += 1;
                }
              } else {
                if (crawlers[i][1] > 0) {
                  crawlers[i][1] -= 1;
                }
              }
            }
          }

          for (int x = 0; x < GRID_SIZE*2+1;x++) {
            for (int y = 0; y < GRID_SIZE*2+1; y++) {
              if (grid_hsv[x][y].v > 2) {
                grid_hsv[x][y].v-=(uint8_t)2;
              } else {
                grid_hsv[x][y].v=0;
              }
            }
          }
          //END CRAWLER EFFECT
*/
          uint8_t grid_z = 0;
          if (grid_type == GRID_PHOS) {
            phos.loop();
            static int stp = 0;
            grid_z = 5;
            stp++;
            for (int x = 0; x < GRID_SIZE*2+1; x++) {
              for (int y = 0; y < GRID_SIZE*2+1; y++) {
                grid_hsv[x][y].h = 96;
                grid_hsv[x][y].s = 255;
                grid_hsv[x][y].v = _min(_max(phos.getVal(x,y)*255,0),255);
              }
            }
          } else {
          
            static int stp = 0;
            grid_z = 25;
            stp++;
            for (int x = 0; x < GRID_SIZE*2+1; x++) {
              for (int y = 0; y < GRID_SIZE*2+1; y++) {
                int tmp_hue = 0;
                int tmp_val = 255;
                if (grid_type == GRID_NOISE) {
                  tmp_hue = inoise8(x*15+stp*5,y*15+stp*5,stp*5);
                  tmp_val = tmp_hue;
                }
                grid_hsv[x][y].h = tmp_hue;
                grid_hsv[x][y].s = 255;
                grid_hsv[x][y].v = tmp_val;
              }
            }
          }

          static float step0 = 0;
          static float step1 = 0;


          int32_t p0[3];
          int32_t p1[3];
          int32_t pt[3]; //temporary storage


          //horizontal lines
          for (int i = 0; i < GRID_SIZE+1; i++) {
            uint8_t x = i*2;

            int32_t stepi = step0+20;
            uint8_t hue = 0;
            uint8_t sat = 255;
            int8_t dist = 41 - ((stepi+i) % 42);
            uint8_t val = 0;
            if (dist > 0) {
              val = dist*6;
            }
            for (int j = 0; j < GRID_SIZE+0; j++) {
              uint8_t y = j*2+1;

              hue = grid_hsv[x][y].h;
              sat = grid_hsv[x][y].s;
              val = grid_hsv[x][y].v;

              pt[0] = -(MATRIX_HEIGHT * 256L) / 2 - (MATRIX_HEIGHT * 256L) / (GRID_SIZE-1) + (j*MATRIX_HEIGHT * 256L) / (GRID_SIZE-1);
              pt[1] = (i * MATRIX_HEIGHT * 256L) / (GRID_SIZE-1) - (MATRIX_HEIGHT * 256L) / 2;
              pt[2] = 0;
              if (grid_type > 0) {
                int cnt = 0;
                grid_hsv_v(x  ,y-2,pt[2],cnt);
                grid_hsv_v(x  ,y  ,pt[2],cnt);
                grid_hsv_v(x+1,y-1,pt[2],cnt);
                grid_hsv_v(x-1,y-1,pt[2],cnt);
                pt[2] -= 128*cnt;
                pt[2] *= grid_z;
              }
              rotate(pt, p0);

              //pt[0] *= -1;
              pt[0] += (MATRIX_HEIGHT * 256L) / (GRID_SIZE-1);
              pt[2] = 0;
              if (grid_type > 0) {
                int cnt = 0;
                grid_hsv_v(x  ,y+2,pt[2],cnt);
                grid_hsv_v(x  ,y  ,pt[2],cnt);
                grid_hsv_v(x+1,y+1,pt[2],cnt);
                grid_hsv_v(x-1,y+1,pt[2],cnt);
                pt[2] -= 128*cnt;
                pt[2] *= grid_z;
              }
              rotate(pt, p1);

              //translate vectors to coordinates
              matrix.scale_z(p0[2]);
              matrix.scale_z(p1[2]);

              //correct 3d perspective

              matrix.perspective(p0);
              matrix.perspective(p1);



              //draw_line_fine2(leds, p0[0], p0[1], p1[0], p1[1], hue, sat, val);
              draw_line_fine(leds, p0[0], p0[1], p1[0], p1[1], hue, sat, val, -10000, val, true);

            }

          }
          
          
          //vertical lines
          for (int i = 0; i < GRID_SIZE+1; i++) {
            uint8_t y = i*2;
            int32_t stepi = step0;
            uint8_t hue = 0;
            uint8_t sat = 255;
            int8_t dist = 41 - ((stepi+i) % 42);
            uint8_t val = 0;
            if (dist > 0) {
              val = dist*6;
            }

            for (int j = 0; j < GRID_SIZE+0; j++) {
              uint8_t x = j*2+1;

              hue = grid_hsv[x][y].h;
              sat = grid_hsv[x][y].s;
              val = grid_hsv[x][y].v;
              
              pt[0] = (-MATRIX_HEIGHT * 256L) / (GRID_SIZE-1) + (i * MATRIX_HEIGHT * 256L) / (GRID_SIZE-1) - (MATRIX_HEIGHT * 256L) / 2;
              pt[1] = -(MATRIX_HEIGHT * 256L) / 2 + (j * MATRIX_HEIGHT * 256L) / (GRID_SIZE-1);
              pt[2] = 0;
              if (grid_type > 0) {
                int cnt = 0;
                grid_hsv_v(x-2,y  ,pt[2],cnt);
                grid_hsv_v(x  ,y  ,pt[2],cnt);
                grid_hsv_v(x-1,y+1,pt[2],cnt);
                grid_hsv_v(x-1,y-1,pt[2],cnt);
                pt[2] -= 128*cnt;
                pt[2] *= grid_z;
              }
              rotate(pt, p0);

              //pt[1] *= -1;
              pt[1] += (MATRIX_HEIGHT * 256L) / (GRID_SIZE-1);
              pt[2] = 0;
              if (grid_type > 0) {
                int cnt = 0;
                grid_hsv_v(x+2,y  ,pt[2],cnt);
                grid_hsv_v(x  ,y  ,pt[2],cnt);
                grid_hsv_v(x+1,y+1,pt[2],cnt);
                grid_hsv_v(x+1,y-1,pt[2],cnt);
                pt[2] -= 128*cnt;
                pt[2] *= grid_z;
              }
              rotate(pt, p1);

              //translate vectors to coordinates
              matrix.scale_z(p0[2]);
              matrix.scale_z(p1[2]);


              //correct 3d perspective
 
              
              //              p0[0] = (-150*256L * (p0[0] - 0))       / (-150*256L + p0[2]) + 0;
              //              p0[1] = (-150*256L * (p0[1] - 0)) / (-150*256L + p0[2]) + 0;
              //              p1[0] = (-150*256L * (p1[0] - 0))       / (-150*256L + p1[2]) + 0;
              //              p1[1] = (-150*256L * (p1[1] - 0)) / (-150*256L + p1[2]) + 0;
              if ( matrix.perspective(p0) && matrix.perspective(p1) ) {
                draw_line_fine(leds, p0[0], p0[1], p1[0], p1[1], hue, sat, val, -10000, val);
              }
            }
          }


          static uint32_t grid_time = micros();
          uint32_t current_time = micros();
          uint32_t elapsed_time = current_time - grid_time;
          grid_time = current_time;
          
          static uint32_t step_time = millis();
          if (millis() - 16 > step_time) {
            step0+=.9f;
            step1+=.9f;
            step_time = millis();
          }


          float rotation_dividy_thing = 1;
          if (grid_type > 0) {
            rotation_dividy_thing = .2;
          }
          rotation_alpha += (elapsed_time/20000.f)*rotation_dividy_thing;
          if (rotation_alpha > 360) {
            rotation_alpha -= 360;
          }
          rotation_beta += (elapsed_time/18000.f)*rotation_dividy_thing;
          if (rotation_beta > 360) {
            rotation_beta -= 360;
          }
          rotation_gamma += (elapsed_time/17000.f)*rotation_dividy_thing;
          if (rotation_gamma > 360) {
            rotation_gamma -= 360;
          }

} //handle_grid()



void handle_snow() {

          //SNOW
          for (int i = 0; i < NUM_PARTICLES; i++) {
            PARTICLE * cp = &particles[i];
            cp->x += cp->vx;
            cp->y += cp->vy;
            cp->z += cp->vz;
            if (cp->y < -127 * 256L) {
              cp->x = random(-100 * 256L, 100 * 256L);
              cp->y = 32767;
              cp->z = random(-148 * 256L, 148 * 256L);
              cp->vx = random(-128, 128);
              cp->vz = random(-255, 256);
            }

            int32_t p[3] = {
              cp->x,
              cp->y,
              cp->z
            };
            int32_t p0[3];

            rotate(p, p0);

            //translate vectors to coordinates
            matrix.scale_z(p0[2]);

            //correct 3d perspective
              
            if (matrix.perspective(p0)) {
              blendXY(leds, p0[0], p0[1], 0, 0, _max(_min((p0[2] + 148 * 256L) / 256L, 255), 0));
            }

          }
        
} //handle_snow()


void handle_spiral() {

          //SPIRAL
          int32_t coords[12][2];
          for (int i = 0; i < 12; i++) {


            int32_t p[3];
            static float taco = 1;
            static float taco_dir = .0000;
            taco += taco_dir;
            if (taco > 2) {
              taco_dir = -fabs(taco_dir);
            }
            if (taco < 1) {
              taco_dir = fabs(taco_dir);
            }
            p[0] = spiral[i][0]*2;
            p[1] = spiral[i][1]*taco;
            p[2] = spiral[i][2]*2;
            //int32_t* p = square_curvetest[i];
            int32_t p0[3];

            rotate(p, p0);

            //translate vectors to coordinates
            matrix.scale_z(p0[2]);

            //correct 3d perspective

            
            matrix.perspective(p0);

            coords[i][0] = p0[0];
            coords[i][1] = p0[1];

            //blendXY(leds, p0[0], p0[1], 0, 0, _max(_min((150*256L-p0[2])/256L,255),0));


          }
          matt_curve8(coords, 12, default_color, default_saturation, 255, false, false, true);
          rotation_alpha+=.2;
          rotation_beta+=.3;

} //handle_spiral();


void handle_tunnel() {
  static uint16_t stp = 0; //rotation
  static uint8_t cnt = 0;
  static uint8_t stp2 = 0; //bright light
  static uint8_t stp3 = 0; //circle width
  static uint16_t stp4 = 0; //tunnel speed
  #define TUNNEL_DETAIL 32
  for (int i = 0; i < TUNNEL_DETAIL+1; i++) {
    //wavy
    //int32_t v0[3] = {0, 20*256L-2*256L*i, -150*256L};
    //int32_t v1[3] = {-3*256L+6*sin8(stp+i*20), 17*256L-4*256L*i, 210*256L};
    static int32_t old_v1[3];
    //tunnel thing
    //float val = (36.f * sin8(stp3)) / 256.f;
    int32_t c = static_cast<int32_t>(cos16(stp + i * (65536L/TUNNEL_DETAIL)));
    int32_t s = static_cast<int32_t>(sin16(stp + i * (65536L/TUNNEL_DETAIL)));
    int32_t v0[3] = {c/4, s/4, -10000 * 256L};
    int32_t v1[3] = {c, s, 120 * 256L};
    int32_t p0[3];
    int32_t p1[3];

    rotate(v0, p0);
    rotate(v1, p1);

    //translate vectors to coordinates
    matrix.scale_z(p0);
    matrix.scale_z(p1);

    //correct 3d perspective
    
    matrix.perspective(p0);
    matrix.perspective(p1);
    uint8_t hue = i * (256/TUNNEL_DETAIL);

    if (i!=TUNNEL_DETAIL) {
      draw_line_fine(leds, p0[0], p0[1], p1[0], p1[1], hue, 255, 0, -10000, 128, true);
      if ((i+stp2)%8 == 0 ) {
        draw_line_fine(leds, p0[0], p0[1], p1[0], p1[1], hue, 255, 16, -10000, 255, true);
      }
    }
    //draw_line_fine(CRGB crgb_object[], int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t hue = 0, uint8_t sat = 255, uint8_t val = 255, int z_depth = -10000, uint8_t val2 = 255)

    #define NUM_CIRCLES_TEST3D 4
    if ( i > 0 ) {
      for (int j = 0; j < NUM_CIRCLES_TEST3D; j++) {
        v1[2] = (stp4 - j * 65536)*5 - 130*256*9;
        old_v1[2] = (stp4 - j * 65536)*5 - 130*256*9;
        rotate(old_v1, p0);
        rotate(v1, p1);
        
        matrix.scale_z(p0);
        matrix.scale_z(p1);
        
        matrix.perspective(p0);
        matrix.perspective(p1);

        int thing = (255 - (255 / NUM_CIRCLES_TEST3D)) + (stp4/256) / NUM_CIRCLES_TEST3D - j * (255 / NUM_CIRCLES_TEST3D);
        thing = (thing*thing)/256L;

        draw_line_fine(
            leds, 
            p0[0], 
            p0[1], 
            p1[0], 
            p1[1], 
            i * (256/TUNNEL_DETAIL), 
            255, 
            thing,
            -10000, 
            thing , 
            true);
      }
    }

    old_v1[0] = v1[0];
    old_v1[1] = v1[1];
    old_v1[2] = v1[2];


  }

  stp  = millis()*3;       //rotation
  stp2 = millis()/64;    //bright light
  stp3 = millis()/96; //circle width
  stp4 = millis()*128;   //tunnel speed

} //handle_tunnel();


void handle_fireworks() {
  //
          //
          //FIREWORKS!!!
          //
          //
          
          static uint16_t max_speed = 32768/8; //this is the max speed, above which particles will start to act weird (accelerating backwards)
          
          //spawn a new firework based on the number of frames that have been shown
          static uint16_t next_firework_frame = 0;
          static uint32_t total_frame_count = millis()/8;

          //physics is locked at 125fps (1000/8)...
          //because I can't be bothered to figure out how to calculate the sum of wind resistance for a non-constant time step.
          //(This is probably a better solution anyway.)
          //If we fall behind: continue calculating physics until we catch up (do not render, do not update the display)
          do_not_update = 1;
          uint8_t physics_count = 0;
          while ( physics_count < 10 && millis()/8 > total_frame_count ) {
            do_not_update = 0;
            physics_count++;

            if (millis()/8 - total_frame_count > 10) {
              total_frame_count = millis()/8;
            }

            //launch shells
            if (frame_count>next_firework_frame) {
              frame_count = 0;
              next_firework_frame = random(100,500);
              spawn_firework_shell();
            }

            //update all airborn shells
            handle_firework_shells();

            //update all particles (physics)
            handle_particles();
            
            frame_count++;
            total_frame_count++;
            skipped_frames++;

            //rotate everything (effectively rotating the camera)
            // rotation_gamma += .05;
            // if (rotation_gamma > 360) {
            //   rotation_gamma -= 360;
            // }

          }

          

          //render particles to the LED buffer
          draw_particles();
          
} //handle_fireworks()







}; //TEST3D





LIGHT_SKETCHES::REGISTER<TEST3D> test3d("test3d");
//3D