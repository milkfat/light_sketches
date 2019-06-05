
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
    uint8_t current_variation = 1;

    uint16_t frame_count = 2100;

    bool do_not_update = 0;

    //x,y,z
    int cubesizex = 12 * 256;
    int cubesizey = 3 * 256;
    int cubesizez = 30 * 256;
    long cube[36][3] = {
      //front
      { -cubesizex, -cubesizey, -cubesizez},
      { -cubesizex, cubesizey, -cubesizez},
      {cubesizex, cubesizey, -cubesizez},

      { -cubesizex, -cubesizey, -cubesizez},
      {cubesizex, cubesizey, -cubesizez},
      {cubesizex, -cubesizey, -cubesizez},

      //back
      { -cubesizex, -cubesizey, cubesizez},
      { -cubesizex, cubesizey, cubesizez},
      {cubesizex, cubesizey, cubesizez},

      { -cubesizex, -cubesizey, cubesizez},
      {cubesizex, cubesizey, cubesizez},
      {cubesizex, -cubesizey, cubesizez},

      //top
      {cubesizex, -cubesizey, -cubesizez},
      {cubesizex, -cubesizey, cubesizez},
      {cubesizex, cubesizey, cubesizez},

      {cubesizex, -cubesizey, -cubesizez},
      {cubesizex, cubesizey, cubesizez},
      {cubesizex, cubesizey, -cubesizez},

      //bottom
      { -cubesizex, -cubesizey, -cubesizez},
      { -cubesizex, -cubesizey, cubesizez},
      { -cubesizex, cubesizey, cubesizez},

      { -cubesizex, -cubesizey, -cubesizez},
      { -cubesizex, cubesizey, cubesizez},
      { -cubesizex, cubesizey, -cubesizez},

      //left
      { -cubesizex, -cubesizey, -cubesizez},
      { -cubesizex, -cubesizey, cubesizez},
      {cubesizex, -cubesizey, cubesizez},

      { -cubesizex, -cubesizey, -cubesizez},
      {cubesizex, -cubesizey, cubesizez},
      {cubesizex, -cubesizey, -cubesizez},

      //right
      { -cubesizex, cubesizey, -cubesizez},
      { -cubesizex, cubesizey, cubesizez},
      {cubesizex, cubesizey, cubesizez},

      { -cubesizex, cubesizey, -cubesizez},
      {cubesizex, cubesizey, cubesizez},
      {cubesizex, cubesizey, -cubesizez}
    };


    long square[4][3] = {
      { -3 * 256, -4 * 256, 0},
      { -3 * 256, 4 * 256, 0},
      {3 * 256, 4 * 256, 0},
      {3 * 256, -4 * 256, 0}
    };

    long spiral[12][3] = {
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
    long square_curvetest[4][3] = {
      { -18 * 256L, -18 * 256L, 0},
      { 18 * 256L, -18 * 256L, 0},
      { 18 * 256L, 18 * 256L, 0},
      { -18 * 256L, 18 * 256L, 0}
    };


    long linex[2][3] = {
      {0, 0, 0},
      {8 * 256, 0, 0}
    };

    long liney[2][3] = {
      {0, 0, 0},
      {0, 8 * 256, 0}
    };


    long linez[2][3] = {
      {0, 0, 0},
      {0, 0, 8 * 256}
    };
    //
    long square2[4][3];

    void rotate(long in[3], long out[3]) {
      matrix.rotate(in, out);
    }

    struct PARTICLE {
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
        uint8_t m = 64;        //mass
        uint8_t r = 3;         //radius
        uint8_t h = 255;       //hue
        uint8_t s = 255;       //saturation
        uint8_t v = 255;       //value (brightness)
        uint8_t trail = 0;     // trail > 0 = This particle will generate a trail of particles. The trail particles will be defined by this attribute index.
        uint8_t available = 1;
    };

  private:

  CRGB trails[MATRIX_HEIGHT*MATRIX_WIDTH+1];

  #define NUM_PARTICLES 2700
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
  #define NUM_PARTICLE_ATTRIBUTES 256
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


    unsigned long update_time = millis();

  public:
    void next_effect() {
      current_variation++;
      current_variation %= 7;


      //reset rotation
      rotation_alpha = 0;
      rotation_beta = 0;
      rotation_gamma = 0;

      //snow
      if (current_variation == 2) {
        for (int i = 0; i < NUM_PARTICLES; i++) {
        particles[i].x = random(-30 * 256L, 30 * 256L);
        particles[i].y = random(-148 * 256L, 148 * 256L);
        particles[i].z = random(-148 * 256L, 148 * 256L);
        particles[i].vx = random(-255, 256);
        particles[i].vy = -255;
        particles[i].vz = random(-255, 256);
        }
      }

      //fireworks
      if (current_variation == 6) {
        for (int i = 0; i < NUM_PARTICLES; i++) {
        particles[i].function = 0;
        }
      }

    }

    //burst into colors!
    void spawn_firework_burst(int16_t start_x, int16_t start_y, int16_t start_z, int16_t start_vx, int16_t start_vy, int16_t start_vz) {
      
     
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
        a[i]->m = 64;
      }
      
      //attributes for alternate color
      a[1]->h += 64;
      
      //attributes for stars that generate trail particles
      a[2]->s = 32;
      a[2]->m += 2+radius;
      a[2]->trail = ai[3];
      
      //attributes for the trail particles
      a[3]->s = 32;
      a[3]->m = 20;
      a[3]->r = 4;


      //number of particles (stars) is based on size
      //number of particles is higher for larger bursts
      uint16_t np = (radius*radius)/25; //square the number of particles per radius
      
      //calculate how far to step along the z-axis
      //in other words: this defines the number of slices into which we will split our sphere
      //small sphere, large z_step, lower number of slices
      //big sphere, small z_step, higher number of slices
      uint16_t z_step = 4500-(np*3000)/25;
      
      //calculate how many stars each slice will get (number of stars in a circle)
      uint16_t npxy = 15+(np*15)/25;

      //random rotation angles for our sphere
      uint16_t r0 = random(0,65535);
      uint16_t r1 = random(0,65535);

      //iterate over the z-axis
      for (uint16_t iz = 0; iz < 32668; iz+=z_step) {
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
            cp->x = start_x;
            cp->y = start_y;
            cp->z = start_z;

            //distribute particles evenly in a ring, X and Y
            int16_t angle = (i*65535)/particles_this_ring;
            angle = (angle*random(85,116))/100; //randomize the angle a bit
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
            float fvx = (vx*vxy)/32768;
            float fvy = (vy*vxy)/32768;
            float fvz = vz;

            //rotate the sphere around x-axis
            int16_t rz = (fvz*cos16(r0))/32768 - (fvy*sin16(r0))/32768;
            int16_t ry = (fvy*cos16(r0))/32768 + (fvz*sin16(r0))/32768;
            fvz = rz;
            fvy = ry;

            //rotate the sphere around z-axis
            int16_t rx = (fvx*cos16(r1))/32768 - (fvy*sin16(r1))/32768;
                    ry = (fvy*cos16(r1))/32768 + (fvx*sin16(r1))/32768;
            fvx = rx;
            fvy = ry;

            //apply speed divisor to change the overall burst size
            fvx /= 15;
            fvy /= 15;
            fvz /= 15;

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
            fvx += start_vx;
            fvy += start_vy;
            fvz += start_vz;

            cp->vx = FC.compress(fvx);
            cp->vy = FC.compress(fvy);
            cp->vz = FC.compress(fvz);

          }
        }
      }
    }

    struct FIREWORK_SHELL {
      bool active = 0;
      int16_t x;
      int16_t y;
      int16_t z;
      float vx;
      float vy;
      float vz;
      uint8_t trail;
      uint16_t mass;
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
        fs->x = 0;
        fs->y = -127*256; //start "on the ground"
        fs->z = 0;
        uint8_t r = random(105);
        fs->vy = (r*r)/105+180; //random launch velocity (burst height), weighted toward lower bursts
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
        particle_attributes[fs->trail].m = 20;
        particle_attributes[fs->trail].trail = 0;
      }
      
    }

    //handle each active shell
    void handle_firework_shells() {
      for (int i = 0; i < NUM_FIREWORK_SHELLS; i++) {
        if (firework_shells[i].active) {
          
          //burst at peak height
          if (firework_shells[i].vy < 5) {
            firework_shells[i].active = false;
            //create a burst using this shell's final position and velocity
            spawn_firework_burst(firework_shells[i].x,firework_shells[i].y,firework_shells[i].z,firework_shells[i].vx,firework_shells[i].vy,firework_shells[i].vz);
          }

          //update active shells
          if (firework_shells[i].active && firework_shells[i].y < 120*256) {

            //add random spark particles to create a tail for the rising shell
            PARTICLE * cp = current_particle();
            if( cp && random(3) == 0) {
              cp->x = firework_shells[i].x;
              cp->y = firework_shells[i].y;
              cp->z = firework_shells[i].z;
              cp->vx = FC.compress(firework_shells[i].vx);
              cp->vy = FC.compress(firework_shells[i].vy);
              cp->vz = FC.compress(firework_shells[i].vz);
              cp->age = 32*8;
              cp->function = 1;
              cp->attributes = firework_shells[i].trail;

            }

            //update the shell's position
            firework_shells[i].x += firework_shells[i].vx;
            firework_shells[i].y += firework_shells[i].vy;
            firework_shells[i].z += firework_shells[i].vz;

            //apply gravity
            firework_shells[i].vy -= .5;
            //calculate the velocity of the shell
            float velocity = sqrt(firework_shells[i].vx*firework_shells[i].vx+firework_shells[i].vy*firework_shells[i].vy+firework_shells[i].vz*firework_shells[i].vz);
            
            //calculate the amount of force based on velocity (wind resistance)
            //smaller numbers represent higher amounts of force
            float velocity_force = (200000 - velocity)/200000;
            
            //apply the force to our particle
            //firework_shells[i].vx *= velocity_force;
            firework_shells[i].vy -= firework_shells[i].vy*(1.f-velocity_force);

            //firework_shells[i].vz *= velocity_force;
          }
        }
      }
    }

    void handle_particle(PARTICLE &p) {
      if (
          p.age < 256*8 &&
          p.x > -127 * 256L &&
          p.x < 127 * 256L &&
          p.y > -127 * 256L &&
          p.y < 127 * 256L &&
          p.z > -127 * 256L &&
          p.z < 127 * 256L
        ) {
          PARTICLE_ATTRIBUTES * pa = &particle_attributes[p.attributes];
          uint16_t age = p.age/8;
          float fvx = FC.decompress(p.vx);
          float fvy = FC.decompress(p.vy);
          float fvz = FC.decompress(p.vz);
                    
          //air resistance is something like F = (bunch of constants)(r^2)(v^2)
          float velocity_squared = fvx * fvx + fvy * fvy + fvz * fvz;

          //F = ma
          //a = F/m
          //a = ((r^2)(v^2))/(r^3)
          //I cancelled r^2 from force and mass
          //a = v^2/r 
          float acceleration = velocity_squared/(pa->r*pa->m);
          //acceleration /= 6400;

          //vector magnitude
          float magnitude = sqrt(velocity_squared);

          //apply force to find our new magnitude
          float new_magnitude = magnitude - acceleration;

          //find the ratio between old and new magnitudes to apply to our x,y,z velocity components
          float ratio = _max(new_magnitude/magnitude, .01);

          fvx *= ratio;
          fvy *= ratio;
          fvz *= ratio;

          //std::cout << "vy2 " << p.vy << "\n";

          //move our particles
          p.x += fvx;
          p.y += fvy;
          p.z += fvz;

          //apply gravity
          fvy -= .167f;

          p.vx = FC.compress(fvx);
          p.vy = FC.compress(fvy); 
          p.vz = FC.compress(fvz);

          //increment the particle's age
          p.age+=8;

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
              cp->age = p.age/8;
              cp->function = 1;
              cp->attributes = pa->trail;
            }
          }

        } else {
          p.function = 0;
        }
    }

    //here we call the handling function for this particle
    void handle_particles() {
      for (int i = 0; i < NUM_PARTICLES; i++) {
        if (particles[i].function) {
          (this->*particle_funcs[particles[i].function])(particles[i]);
        }
      }
    }

    //an array of function pointers to functions that modify the PARTICLE structure
    //referred to as a jump table?
    void (TEST3D::*particle_funcs[2])(PARTICLE&) = {nullptr,&TEST3D::handle_particle};

    //render the particles in 3D to the buffer
    void draw_particles() {

      //clear the LED buffer
      LED_black();

      //draw the particles
      for (int i = 0; i < NUM_PARTICLES; i++) {
        if ( particles[i].function) {
          PARTICLE_ATTRIBUTES * pa = &particle_attributes[particles[i].attributes];
          uint16_t age = particles[i].age/8;
          //darken (v = HSV value)
          uint8_t v = _min(_max(255-(age*age*age*2)/(255*255),0), pa->v);
          //colors become more saturated over time
          uint8_t s = _max(_min(age*8,pa->s),0);
          
          //create a three-dimensional structure that will be rotated
          long p[3] = {
            particles[i].x,
            particles[i].y,
            particles[i].z
          };
          long p0[3];

          //rotate all particles using our orientation matrix
          rotate(p, p0);

          //translate vectors to coordinates
          p0[2] += -100 * 256 + (200 * 256 * debug_scaler) / 256;

          //correct 3d perspective
          if (matrix.perspective(p0)) {
            blendXY(leds, p0[0], p0[1], pa->h, s, (_max(_min((p0[2] + 148 * 256L) / 256L, 255), 0)*v)/256);
          }
        }
      }
    }


    #define GRID_SIZE 20
    CHSV grid_hsv[GRID_SIZE*2+1][GRID_SIZE*2+1];


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
      rotation_alpha = 0;
      rotation_beta = 90;
      rotation_gamma = 0;

      for (int i = 0; i < MATRIX_HEIGHT+MATRIX_WIDTH; i++) {
        trails[i] = 0;
      }

      phos.setup();
     

    }

    void reset() {
      rotation_alpha = 0;
      rotation_beta = 90;
      rotation_gamma = 0;
    }

    void loop() {
      


      if (effect_beat == 1) {
        effect_beat = 0;
        //reset();
      }
      if (millis() - 16 > update_time) {

        if (current_variation == 0) {

          //3D TEST OBJECT
          for (int i = 0; i < 4; i++) {
            rotate(square[i], square2[i]);
          }

          for (int i = 0; i < 4; i++) {
            draw_line_fine(leds, square2[i][0] + 3 * 256, square2[i][1] + 50 * 256, square2[(i + 1) % 4][0] + 3 * 256, square2[(i + 1) % 4][1] + 50 * 256);
          }

          long r0[3];
          long r1[3];
          rotate(linex[0], r0);
          rotate(linex[1], r1);

          draw_line_fine(leds, r0[0] + 3 * 256, r0[1] + 50 * 256, r1[0] + 3 * 256, r1[1] + 50 * 256, 160);


          rotate(liney[0], r0);
          rotate(liney[1], r1);

          draw_line_fine(leds, r0[0] + 3 * 256, r0[1] + 50 * 256, r1[0] + 3 * 256, r1[1] + 50 * 256, 48);


          rotate(linez[0], r0);
          rotate(linez[1], r1);

          draw_line_fine(leds, r0[0] + 3 * 256, r0[1] + 50 * 256, r1[0] + 3 * 256, r1[1] + 50 * 256, 96);


        } else if (current_variation == 1) {

          //GRID

          phos.loop();
          for (int x = 0; x < GRID_SIZE*2+1; x++) {
            for (int y = 0; y < GRID_SIZE*2+1; y++) {
              grid_hsv[x][y].h = 96;
              grid_hsv[x][y].s = 255;
              grid_hsv[x][y].v = _min(_max(phos.getVal(x,y)*255,0),255);
            }
          }


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


          static float step0 = 0;
          static float step1 = 0;


          long p0[3];
          long p1[3];
          long pt[3]; //temporary storage


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
              rotate(pt, p0);

              //pt[0] *= -1;
              pt[0] += (MATRIX_HEIGHT * 256L) / (GRID_SIZE-1);
              rotate(pt, p1);

              //translate vectors to coordinates
              p0[2] += -100 * 256 + (200 * 256 * debug_scaler) / 256;
              p1[2] += -100 * 256 + (200 * 256 * debug_scaler) / 256;

              //correct 3d perspective

              matrix.perspective(p0);
              matrix.perspective(p1);



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
              rotate(pt, p0);

              //pt[1] *= -1;
              pt[1] += (MATRIX_HEIGHT * 256L) / (GRID_SIZE-1);
              rotate(pt, p1);

              //translate vectors to coordinates
              p0[2] += -100 * 256 + (200 * 256 * debug_scaler) / 256;
              p1[2] += -100 * 256 + (200 * 256 * debug_scaler) / 256;


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

          rotation_alpha += (elapsed_time/20000.f);
          if (rotation_alpha > 360) {
            rotation_alpha -= 360;
          }
          rotation_beta += elapsed_time/18000.f;
          if (rotation_beta > 360) {
            rotation_beta -= 360;
          }
          rotation_gamma += elapsed_time/17000.f;
          if (rotation_gamma > 360) {
            rotation_gamma -= 360;
          }







        } else if (current_variation == 2) {

          //SNOW
          for (int i = 0; i < NUM_PARTICLES; i++) {
            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;
            particles[i].z += particles[i].vz;
            if (particles[i].y < -127 * 256L) {
              particles[i].x = random(-100 * 256L, 100 * 256L);
              particles[i].y += 299 * 256L;
              particles[i].z = random(-148 * 256L, 148 * 256L);
              particles[i].vx = random(-128, 128);
              particles[i].vz = random(-255, 256);
            }

            long p[3] = {
              particles[i].x,
              particles[i].y,
              particles[i].z
            };
            long p0[3];

            rotate(p, p0);

            //translate vectors to coordinates
            p0[2] += -100 * 256 + (200 * 256 * debug_scaler) / 256;

            //correct 3d perspective
              
            if (matrix.perspective(p0)) {
              blendXY(leds, p0[0], p0[1], 0, 0, _max(_min((p0[2] + 148 * 256L) / 256L, 255), 0));
            }

          }
        } else if (current_variation == 3) {

          //SPIRAL
          long coords[12][2];
          for (int i = 0; i < 12; i++) {


            long p[3];
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
            //long* p = square_curvetest[i];
            long p0[3];

            rotate(p, p0);

            //translate vectors to coordinates
            p0[2] += -100 * 256 + (200 * 256 * debug_scaler) / 256;

            //correct 3d perspective

            
            matrix.perspective(p0);

            coords[i][0] = p0[0];
            coords[i][1] = p0[1];

            //blendXY(leds, p0[0], p0[1], 0, 0, _max(_min((150*256L-p0[2])/256L,255),0));


          }
          matt_curve8(coords, 12, default_color, default_saturation, 255, false, false, true);
          rotation_alpha+=.2;
          rotation_beta+=.3;
        } else if (current_variation == 4) {

          //3D SHADING


          //rotation_alpha += 1;
          //            if (rotation_alpha > 179) {
          //              rotation_alpha = -180;
          //            }
          //rotation_beta = 90;
          //rotation_beta += 1;
          //            if (rotation_beta > 179) {
          //              rotation_beta = -180;
          //            }
          //            rotation_gamma += 1;
          //            if (rotation_gamma > 179) {
          //              rotation_gamma = -180;
          //            }


          uint8_t hue;


          //draw some triangles

          for (int i = 0; i < 12; i++) {

            hue = 96;//( 256 * (i/2) ) /12; //one color for each side of the cube


            //clear the buffer to be used for filling the triangle
            for (int i = 0; i < MATRIX_HEIGHT; i++) {
              y_buffer[i][0] = MATRIX_WIDTH + 1;
              y_buffer[i][1] = -1;
            }

            //find the surface normal of our side
            long normal[3] = {0, 0, 0};
            long n[3];
            if (cube[i * 3][0] == cube[i * 3 + 1][0] && cube[i * 3 + 1][0] == cube[i * 3 + 2][0]) {
              normal[0] = 128 * (cube[i * 3][0] / abs(cube[i * 3][0]));
            } else if (cube[i * 3][1] == cube[i * 3 + 1][1] && cube[i * 3 + 1][1] == cube[i * 3 + 2][1]) {
              normal[1] = 128 * (cube[i * 3][1] / abs(cube[i * 3][1]));
            } else if (cube[i * 3][2] == cube[i * 3 + 1][2] && cube[i * 3 + 1][2] == cube[i * 3 + 2][2]) {
              normal[2] = 128 * (cube[i * 3][2] / abs(cube[i * 3][2]));
            }
            rotate(normal, n);
            uint8_t bri = _min(_max(0, n[2]), 128) + 10;


            //a place to store the rotated vectors
            long v0[3];
            long v1[3];
            long v2[3];

            //rotate them vectors
            rotate(cube[i * 3],   v0);
            rotate(cube[i * 3 + 1], v1);
            rotate(cube[i * 3 + 2], v2);

            //get the combined (average) z_depth of this triangle
            int z_depth = _max(v0[2], v1[2]);
            z_depth = _max(z_depth, v2[2]);

            //translate vectors to coordinates
            v0[2] += -100 * 256 + (200 * 256 * debug_scaler) / 256;
            v1[2] += -100 * 256 + (200 * 256 * debug_scaler) / 256;
            v2[2] += -100 * 256 + (200 * 256 * debug_scaler) / 256;

            //correct 3d perspective


            
            matrix.perspective(v0);
            matrix.perspective(v1);
            matrix.perspective(v2);

            //draw the line on our temporary canvas
            draw_line_fine(temp_canvas, v0[0], v0[1], v1[0], v1[1], hue, 255, bri, -10000);
            draw_line_fine(temp_canvas, v1[0], v1[1], v2[0], v2[1], hue, 255, bri, -10000);
            draw_line_fine(temp_canvas, v2[0], v2[1], v0[0], v0[1], hue, 255, bri, -10000);

            //fill between the pixels of our lines
            for (int y = 0; y < MATRIX_HEIGHT; y++) {
              if (y_buffer[y][0] + 1 < y_buffer[y][1]) {
                //leds[XY(y_buffer[y][0],y)] += temp_canvas[XY(y_buffer[y][0],y)];
                //leds[XY(y_buffer[y][1],y)] += temp_canvas[XY(y_buffer[y][1],y)];
                //for (int x = y_buffer[y][0] + 1; x < y_buffer[y][1]; x++) {

                for (int x = y_buffer[y][0]; x <= y_buffer[y][1]; x++) {
                  //drawXYZ(leds, x, y, n[2]+j*200*priority, hue, 255, bri );
                  drawXYZ(leds, x, y, n[2], hue, 255, bri );
                }
              }
            }

            for (int i = 0; i < NUM_LEDS; i++) {
              temp_canvas[i] = CRGB::Black;
            }

          }


        } else if (current_variation == 5) {
          static uint16_t stp = 0; //rotation
          static uint8_t cnt = 0;
          static uint8_t stp2 = 0; //bright light
          static uint8_t stp3 = 0; //circle width
          static uint16_t stp4 = 0; //tunnel speed
          #define TUNNEL_DETAIL 32
          for (int i = 0; i < TUNNEL_DETAIL+1; i++) {
            //wavy
            //long v0[3] = {0, 20*256L-2*256L*i, -150*256L};
            //long v1[3] = {-3*256L+6*sin8(stp+i*20), 17*256L-4*256L*i, 210*256L};
            static long old_v1[3];
            //tunnel thing
            //float val = (36.f * sin8(stp3)) / 256.f;
            long v0[3] = {static_cast<long>(cos16(stp + i * (65536L/TUNNEL_DETAIL)))/7, static_cast<long>(sin16(stp + i * (65536L/TUNNEL_DETAIL)))/7, -10000 * 256L};
            long v1[3] = {static_cast<long>(cos16(stp + i * (65536L/TUNNEL_DETAIL)))/7, static_cast<long>(sin16(stp + i * (65536L/TUNNEL_DETAIL)))/7, 210 * 256L};
            long p0[3];
            long p1[3];

            rotate(v0, p0);
            rotate(v1, p1);

            //translate vectors to coordinates
            //p0[2] += -100*256 + (200*256*debug_scaler)/256;
            //p1[2] += -100*256 + (200*256*debug_scaler)/256;

            //correct 3d perspective
            
            matrix.perspective(p0);
            matrix.perspective(p1);


            if (i!=TUNNEL_DETAIL) {
              draw_line_fine(leds, p0[0], p0[1], p1[0], p1[1], i * (256/TUNNEL_DETAIL), 255, 4, -10000, 255, true);
              if ((i+stp2)%8 == 0 ) {
                draw_line_fine(leds, p0[0], p0[1], p1[0], p1[1], i * (256/TUNNEL_DETAIL), 255, 128, -10000, 255, true);
              }
            }
            //draw_line_fine(CRGB crgb_object[], long x1, long y1, long x2, long y2, uint8_t hue = 0, uint8_t sat = 255, uint8_t val = 255, int z_depth = -10000, uint8_t val2 = 255)

            #define NUM_CIRCLES_TEST3D 4
            if ( i > 0 ) {
              for (int j = 0; j < NUM_CIRCLES_TEST3D; j++) {
                v1[2] = stp4 - j * 65536 - 40*256;
                old_v1[2] = stp4 - j * 65536 - 40*256;
                rotate(old_v1, p0);
                rotate(v1, p1);
                
                matrix.perspective(p0);
                matrix.perspective(p1);

                draw_line_fine(leds, p0[0], p0[1], p1[0], p1[1], i * (256/TUNNEL_DETAIL), 255, sq((255 - (255 / NUM_CIRCLES_TEST3D)) + stp4/256 / NUM_CIRCLES_TEST3D - j * (255 / NUM_CIRCLES_TEST3D)) / 256L, -10000, sq((255 - (255 / NUM_CIRCLES_TEST3D)) + stp4/256 / NUM_CIRCLES_TEST3D - j * (255 / NUM_CIRCLES_TEST3D)) / 256L , true);
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

        } else if (current_variation == 6) {
          
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
          while ( millis()/8 > total_frame_count ) {
            do_not_update = 0;

            //launch shells
            if (frame_count>next_firework_frame) {
              frame_count = 0;
              next_firework_frame = random(45,200);
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
            rotation_gamma += .10;
            if (rotation_gamma > 360) {
              rotation_gamma -= 360;
            }

          }

          

          //render particles to the LED buffer
          draw_particles();
            
        }

        //update the display
        if (!do_not_update) {
          skipped_frames--;
          //update LEDS
          LED_show();
          //clear LED buffer
          LED_black();
        }

      }
    }

};


LIGHT_SKETCHES::REGISTER<TEST3D> test3d("test3d");
//3D