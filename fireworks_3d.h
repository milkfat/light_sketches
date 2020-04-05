#ifndef LIGHTS_FIREWORKS_3D_H
#define LIGHTS_FIREWORKS_3D_H
//3D


int skipped_frames = 0;
class FIREWORKS_3D: public LIGHT_SKETCH {
  public:
    FIREWORKS_3D () {setup();}
    ~FIREWORKS_3D () {}
  private:

    uint16_t next_firework_frame = 0;

    enum particle_types {
      NULL_PARTICLE,
      STAR_PARTICLE,
      LIFT_PARTICLE,
      CRACKLE_PARTICLE,
      COLOR_CHANGE_PARTICLE,
      BURST_PARTICLE
    };

    float compress(float value) {
      //return FC.compress(value);
      return value;
    }

    float decompress(float value) {
      //return FC.decompress(value);
      return value;
    }

    uint16_t frame_count = 2100;

    bool do_not_update = 0;

    void rotate(int32_t in[3], int32_t out[3]) {
      led_screen.matrix.rotate(in, out);
    }

    void rotate(VECTOR3& p) {
      led_screen.matrix.rotate(p);
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
        uint8_t function = NULL_PARTICLE;   //index of the functions (physics and draw) that will update this particle
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

  #define NUM_FIREWORK_PARTICLES 2000
  PARTICLE particles[NUM_FIREWORK_PARTICLES];
  
  PARTICLE * current_particle() {
    static uint16_t cp = 0;
    uint16_t cnt = 0;
    PARTICLE * cpo = nullptr;
    while (cnt < NUM_FIREWORK_PARTICLES) {
      cp++;
      if (cp > NUM_FIREWORK_PARTICLES-1) {
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
     
        for (int i = 0; i < NUM_FIREWORK_PARTICLES; i++) {
          particles[i].function = NULL_PARTICLE;
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
        led_screen.reverse_perspective(p);
        fs->y = p[1]; //start "on the ground"
        fs->z = 0;
        uint16_t r = random(105);
        //std::cout << "SIZE:" << r << "\n";

        next_firework_frame = random(100,100+(r*5));
        fs->vy = r+200; //random launch velocity (burst height), weighted toward lower bursts
        uint16_t r2 = (105-r)/6;
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
            cp->function = LIFT_PARTICLE;
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
                cp->function = STAR_PARTICLE;
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
      abs(p.y) > 250 * 256L ||
      abs(p.z) > 127 * 256L
      ) {
        p.function = NULL_PARTICLE;
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

    void star_physics(PARTICLE &p) {

      //increment the particle's age
      p.age+=4;

      if (p.age > 256*8) {p.function = NULL_PARTICLE; return;}
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
          cp->age = _max(p.age,256);
          cp->function = STAR_PARTICLE;
          cp->attributes = pa->trail;
        }
      }
 
    } //star_particle()

    void star_draw(uint16_t pi) {
          PARTICLE * cp = &particles[pi];
          PARTICLE_ATTRIBUTES * pa = &particle_attributes[cp->attributes];
          uint16_t age = cp->age/8;
          //darken (v = HSV value)
          int16_t v_temp = 255;
          uint8_t r = fmix32(pi);
          r=0;
          r = r/4 + (_max(pa->r-35,0))+32;
          if (age > r) {
            v_temp -= (age-r)*4;
          }
          uint8_t v = _min(_max(v_temp,0), pa->v);
          if (v == 0) {
            cp->function = NULL_PARTICLE;
            return;
          }
          //colors become more saturated over time
          uint8_t s = _max(_min(cp->age,pa->s),0);

          VECTOR3 p(cp->x*2,cp->y*2,cp->z*2);
          
          draw_particle(p,pa->h,s,v);
    }

    void star_color_change_draw(uint16_t pi) {
          PARTICLE * cp = &particles[pi];
          PARTICLE_ATTRIBUTES * pa = &particle_attributes[cp->attributes];
          int16_t age = cp->age;
          uint8_t v = 0;
          uint8_t s = 255;
          if (age > 0) {
            v = _min(age,255);
          }
          if (age > 2*256) {
            v = _max(3*256-age, 0);
          }
          if (age > 3*256) {
            cp->function = NULL_PARTICLE;
          }

          VECTOR3 p(cp->x*2,cp->y*2,cp->z*2);
          
          draw_particle(p,pa->h,s,v);
    }

    void crackle_draw(uint16_t pi) {

          PARTICLE * cp = &particles[pi];
          PARTICLE_ATTRIBUTES * pa = &particle_attributes[cp->attributes];
          int16_t age = cp->age;
          uint16_t r = fmix32(pi)%512;
          
          if (r < 256) {
            r = 256 - r;
            r = (((r*r)/256)+r)/2;
            r = 256 - r;
          }
          if (r > 255) {
            r -= 256;
            r = (((r*r)/256)+r)/2;
            r += 256;
          }
          r/=2;
          age -= r;
          if (age > 2*256) {
            if (age < 2*256+32) {
              VECTOR3 p(cp->x*2,cp->y*2,cp->z*2);
              draw_particle(p,0,0,96);
            } else {
              cp->function = NULL_PARTICLE;
            }
          }

    }

    void burst_particle_draw(uint16_t pi) {

          PARTICLE * cp = &particles[pi];
          PARTICLE_ATTRIBUTES * pa = &particle_attributes[cp->attributes];
          int16_t age = cp->age;
          uint16_t r = fmix32(pi)%512;
          if (age < 108) {
              VECTOR3 p(cp->x*2,cp->y*2,cp->z*2);
              if (age < 16) {
                draw_particle(p,32,0,255);
              } else {
                draw_particle(p,32,128,214-age*2);
              }
            } else {
              cp->function = NULL_PARTICLE;
            }
          

    }

    void draw_particle(VECTOR3& p, uint8_t h, uint8_t s, uint8_t v) {

          //rotate all particles using our orientation matrix
          rotate(p);

          //translate vectors to coordinates
          scale_z(p[2]);

          //correct 3d perspective
          if (led_screen.perspective(p)) {
            int16_t v_temp2 = (p[2] + 200 * 256L) / 256L;
            blendXY(led_screen, p[0], p[1], h, s, (_max(_min(v_temp2, 255), 0)*v)/255);
          }
    }


    void lift_physics(PARTICLE &p) {
      //increment the particle's age
      p.age+=8;
      if (p.age > 256*8) {p.function = NULL_PARTICLE; return;}
      //PARTICLE_ATTRIBUTES * pa = &particle_attributes[p.attributes];
      move_particle(p);
 
    } //lift_particle()


    //here we call the handling function for this particle
    void handle_particles() {
      int active_particles = 0;
      for (int i = 0; i < NUM_FIREWORK_PARTICLES; i++) {
        if (particles[i].function) {
          active_particles++;
          (this->*particle_physics_functions[particles[i].function])(particles[i]);
        }
      }
    }

    //an array of function pointers to functions that modify the PARTICLE structure
    //referred to as a jump table?
    void (FIREWORKS_3D::*particle_physics_functions[6])(PARTICLE&) = {nullptr,&FIREWORKS_3D::star_physics,&FIREWORKS_3D::lift_physics,&FIREWORKS_3D::star_physics,&FIREWORKS_3D::star_physics,&FIREWORKS_3D::star_physics};
    void (FIREWORKS_3D::*particle_draw_functions[6])(uint16_t pi) = {nullptr,&FIREWORKS_3D::star_draw,&FIREWORKS_3D::star_draw,&FIREWORKS_3D::crackle_draw,&FIREWORKS_3D::star_color_change_draw,&FIREWORKS_3D::burst_particle_draw};

    //render the particles in 3D to the buffer
    void draw_particles() {

      //clear the LED buffer
      LED_black();

      //draw the particles
      for (int i = 0; i < NUM_FIREWORK_PARTICLES; i++) {
        if (particles[i].function && particles[i].age >= 0) {
          (this->*particle_draw_functions[particles[i].function])(i);
        }
      }
    }



    void setup() {
      //particles = (PARTICLE*) malloc (NUM_FIREWORK_PARTICLES * sizeof(PARTICLE));
      for (int i = 0; i < NUM_FIREWORK_PARTICLES; i++) {
        particles[i].x = random(-30 * 256L, 30 * 256L);
        particles[i].y = -127*256;
        particles[i].z = random(-148 * 256L, 148 * 256L);
        particles[i].vx = random(-255, 256);
        particles[i].vy = -255;
        particles[i].vz = random(-255, 256);
      }

      reset();

      control_variables.add(led_screen.camera_position.z, "Camera Z:", 0, 256*256);
      control_variables.add(led_screen.screen_distance, "Screen Z:", 0, 256*256);

    }

    void reset() {
      led_screen.rotation_alpha = 0;
      led_screen.rotation_beta = 90;
      led_screen.rotation_gamma = 0;
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


    handle_fireworks();

        //update the display
        if (!do_not_update) {
          skipped_frames--;
          //update LEDS
          LED_show();
          //clear LED buffer
          LED_black();
        }

    } //loop();

void handle_fireworks() {
  //
          //
          //FIREWORKS!!!
          //
          //
          
          //static uint16_t max_speed = 32768/8; //this is the max speed, above which particles will start to act weird (accelerating backwards)
          
          //spawn a new firework based on the number of frames that have been shown
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


    //burst into colors!
    void spawn_firework_burst(const int16_t& start_x, const int16_t& start_y, const int16_t& start_z, const int16_t& start_vx, const int16_t& start_vy, const int16_t& start_vz) {
      
      int do_crackle = 0;
      int do_double_crackle = 0;
      if (random(3)) {
        do_crackle = 1;

        if (random(3)==0) {
          do_double_crackle = 1;
        }
      }
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
      //uint16_t radius = random(range_min,range_max);
      uint16_t radius = range_max;
      uint16_t burst_size = radius+35; //35 is the minimum for the current physics calculations
      //std::cout << "range_min: " << range_min << " range_max: " << range_max << " radius: " << radius << " burst_size: " << burst_size << "\n";


      //create particle attributes for our burst
      PARTICLE_ATTRIBUTES * a[6];
      uint16_t ai[6];

      //random hue
      uint8_t h = random(256);

      //default attributes
      for (int i = 0; i < 6; i++) {
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

      //attributes for crackle
      a[4]->s = 0;
      a[4]->m = a[4]->m/2;

      a[5]->s = 0;
      a[5]->m /= 3;


      //number of particles (stars) is based on size
      //number of particles is higher for larger bursts
      uint16_t np = (radius*radius)/28; //square the number of particles per radius
      
      //calculate how far to step along the z-axis
      //in other words: this defines the number of slices into which we will split our sphere
      //small sphere, large z_step, lower number of slices
      //big sphere, small z_step, higher number of slices
      //uint16_t z_step = 32768/4;
      uint16_t z_step = 4500-(np*3000)/28;
      //z_step *= 4;
      //std::cout << "np: " << np << " z_step: " << z_step << "\n";

      //calculate how many stars each slice will get (number of stars in a circle)
      uint16_t npxy = 15+(np*15)/28;

      //random rotation angles for our sphere
      uint16_t r0 = random(0,65535);
      uint16_t r1 = random(0,65535);

      //iterate over the z-axis
      uint16_t ring_count = 0;
      int age_count = 0;
      while (true) {
        uint16_t iz = ring_count*z_step;
        if (iz > 32767) {break;}
        
        ring_count++;
        for (uint16_t k = 0; k < 1; k++) {
          iz += k;
        //for (uint16_t k = 0; k < 200; k+=50) {
          //calculate our Z-velocity (you could pretend that this is a coordinate)
          int16_t vz = cos16(iz); //distribution from 0 - 32767
          //calculate the radius of this slice
          int16_t r = abs(sin16(iz));
          //calculate the number of particles for this slice (based on absolute size of the shell combined with the relative radius size)
          //int16_t particles_this_ring = r/500;
          int16_t particles_this_ring = (r*npxy)/32767;
          //particles_this_ring = _max(particles_this_ring/4,1);
          //particles_this_ring=8;
          //std::cout << "stars: " << particles_this_ring << " iz:" << npxy << "iz:" << iz  << " vz:" << vz  << " radius:" << r << "\n";

          //iterate around the circumference of the slice
          for (int i = 0; i < particles_this_ring; i++) {
            for (int k = 0; k < 1; k++) {
            //for (int k = 0; k < 200; k+=50) {
              //find the next available particle object
              PARTICLE * cp = current_particle();
              //create particle if an object is available
              if(cp) {
                cp->function = STAR_PARTICLE;
                cp->function = COLOR_CHANGE_PARTICLE;
                //initial starting position
                cp->x = start_x/2;
                cp->y = start_y/2;
                cp->z = start_z/2;

                //distribute particles evenly in a ring, X and Y
                int16_t angle = (i*65535)/particles_this_ring + k;
                uint8_t xy_mag = random(95,105); //randomize the angle a bit
                //xy_mag = 100;
                if (k != 0) {
                  xy_mag = random(85,115);
                }
                angle = (angle*xy_mag)/100;
                //calculate the X and Y components of the particle for this slice
                int16_t vx = sin16(angle);
                int16_t vy = cos16(angle);
                
                //add the Z component
                uint8_t z_mag = random(95,105); //randomize the angle a bit
/*
                if (k != 0) {
                  z_mag = random(85,115);
                }
*/
                //z_mag = 100;
                int16_t riz = (iz*z_mag)/100; 
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
                cp->age = age_count;
                age_count -= 5;

                cp->attributes = ai[0];
                
                //alternate color for one hemisphere
                if (vz < 0) {
                  cp->attributes = ai[1];
                }
                //modify a ring of particles to produce trails
                if(abs(iz - 16000) < 1000) {
                  //cp->attributes = ai[2];
                }
                /*
                if (ring_count % 4 == 0 && i % 4 == 0) {
                  if (k == 0) {
                    cp->attributes = ai[2];
                  }
                } else {
                  cp->function = NULL_PARTICLE;
                }
                */


                if (random(4)) {
                  PARTICLE * cp2 = current_particle();
                  if (cp2) {
                    *cp2 = *cp;
                    cp2->age = random(96);
                    uint16_t r = random(16,128);
                    cp2->vx = (cp2->vx*r)/256;
                    cp2->vy = (cp2->vy*r)/256;
                    cp2->vz = (cp2->vz*r)/256;
                    cp2->function = BURST_PARTICLE;
                  }
                }

                if (k == 0) {
                  //crackle particles
                  if (do_crackle) {
                    PARTICLE * cp2 = current_particle();
                    if (cp2) {
                      *cp2 = *cp;
                      cp2->attributes = ai[4];
                      cp2->function = CRACKLE_PARTICLE;
                      cp2->age = -256;
                    }

                    if (do_double_crackle) {
                      PARTICLE * cp3 = current_particle();
                      if (cp3) {
                        *cp3 = *cp;
                        cp3->attributes = ai[5];
                        cp3->function = CRACKLE_PARTICLE;
                        cp3->age = 128;
                        a[4]->m = (a[0]->m*2)/3;
                      }
                    }
                  }
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
      }
    }





}; //FIREWORKS_3D





LIGHT_SKETCHES::REGISTER<FIREWORKS_3D> fireworks_3d("fireworks_3d");
//3D

#endif