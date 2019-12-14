#ifndef LIGHTS_SNOWFLAKES_H
#define LIGHTS_SNOWFLAKES_H


class SNOWFLAKES: public LIGHT_SKETCH {
  public:
    SNOWFLAKES () {setup();}
    ~SNOWFLAKES () {}
#define NUM_SNOWFLAKES 50


  private:

    struct SNOWFLAKE {
        uint8_t hue;
        uint8_t sat;
        uint32_t age = 20;
        uint16_t seed = random(65536); //random seed for our procedurally-generated snowflake
        int32_t x = 0;
        int32_t y = 0;
        int32_t z = 0;
        int32_t vx = 0;
        int32_t vy = 0;
        int32_t vz = 0;
        int32_t rx = 0;
        int32_t ry = 0;
        int32_t rz = 0;
        int32_t flag = 0;
    };

    SNOWFLAKE snowflakes[NUM_SNOWFLAKES];
    uint32_t big_seed = 0;


    void flake_reset(SNOWFLAKE * flake, uint8_t on_screen) {
            flake->age = 20;
            flake->seed = random(65536);

            VECTOR3 p;

            p.z = random(-1500*256,0*256);


            if(on_screen) { //initial snowflake coordinates will be created on-screen
                p.x = random(MATRIX_WIDTH*256);
                p.y = random(MATRIX_HEIGHT*256);
                matrix.reverse_perspective(p);
                matrix.reverse_scale_z(p);
                flake->vx = random(-128,128);
            } else { //new snowflakes will be created slightly off the edge of the screen
                int r = random(MATRIX_HEIGHT+MATRIX_WIDTH);
                if (r >= MATRIX_HEIGHT) {
                    //snowflakes originate at top of screen
                    p.x = random(MATRIX_WIDTH*256);
                    p.y = (MATRIX_HEIGHT)*256;
                    matrix.reverse_perspective(p);
                    matrix.reverse_scale_z(p);
                    p.y += 8000;
                    flake->vx = random(-96,96);
                } else if (r >= MATRIX_HEIGHT/2) {
                    //snowflakes originate at right of screen
                    p.x = MATRIX_WIDTH*256;
                    int r = random(MATRIX_HEIGHT);
                    r = (r*r)/MATRIX_HEIGHT;
                    r = MATRIX_HEIGHT-r;
                    p.y = r*256;
                    matrix.reverse_perspective(p);
                    matrix.reverse_scale_z(p);
                    p.x += 8000;
                    flake->vx = random(64,8);
                    flake->vx *= -1;
                } else {
                    //snowflakes originate at left of screen
                    p.x = 0;
                    int r = random(MATRIX_HEIGHT);
                    r = (r*r)/MATRIX_HEIGHT;
                    r = MATRIX_HEIGHT-r;
                    p.y = r*256;
                    matrix.reverse_perspective(p);
                    matrix.reverse_scale_z(p);
                    p.x -= 8000;
                    flake->vx = random(64,8);
                }
            }
            flake->x = p.x;
            flake->y = p.y;
            flake->z = p.z;
            flake->vy = -48;
            flake->vz = random(-128,128);
            flake->rx = random(64*256,256*256);
            flake->ry = random(64*256,256*256);
            flake->rz = random(64*256,256*256);
            if (random(2)) {
                flake->rx*=-1;
            }
            if (random(2)) {
                flake->ry*=-1;
            }
            if (random(2)) {
                flake->rz*=-1;
            }
            uint8_t r = random(4);
            switch (r) {
                case 0:
                    flake->hue = 96;
                    flake->sat = 195;
                    break;
                case 1:
                    flake->hue = 160;
                    flake->sat = 250;
                    break;
                case 2:
                    flake->hue = 48;
                    flake->sat = 200;
                    break;
                case 3:
                    flake->hue = 0;
                    flake->sat = 200;
                    break;
            }
    }
    
  public:

    void next_effect() {
    }

    void setup() {
        rotation_alpha = 0;
        rotation_beta = 90;
        rotation_gamma = 0;
        big_seed = random(65535);
        for (int i = 0; i < NUM_SNOWFLAKES; i++) {
            flake_reset(&snowflakes[i], true);
        }

    }

    void reset() {
      setup();
      for (int i = 0; i < NUM_LEDS; i++)
      {
        leds[i] = CRGB::Black;
      }

    }


    void loop() {
        
        //update the screen
        LED_show();

        //reset our led buffer
        for (int i = 0; i < NUM_LEDS; i++)
        {
            leds[i] = CRGB::Black;
        }

        //reset the snowflake's "drawn" flag
        for (int i = 0; i < NUM_SNOWFLAKES; i++) {
            snowflakes[i].flag = 1;
        }

        //draw each snowflake procedurally
        for (int f = 0; f < NUM_SNOWFLAKES; f++) {

            //we want to draw our snowflakes from back to front
            //so we find the most distant snowflake that hasn't yet been drawn
            int max_z = 1000000;
            int max_flake = 0;
            for (int j = 0; j < NUM_SNOWFLAKES; j++) {
                if (snowflakes[j].flag) {
                    if (snowflakes[j].z < max_z) {
                        max_z = snowflakes[j].z;
                        max_flake = j;
                    }
                }
            }
            SNOWFLAKE * flake = &snowflakes[max_flake];
            flake->flag=0;

            uint32_t seed = flake->seed;
            uint32_t pos = 0;

            //a variable to hold our snowflake geometry
            VECTOR3 points[72];

            //build one branch of the snowflake
            //this branch will be drawn six times to create a neat-o hexagonal snowflake
            //first build the trunk:
            points[0] = VECTOR3(0,0,0);
            points[1] = VECTOR3(4000,4000,0);

            //now we'll build random branches along the trunk until we reach the tip
            uint8_t num_points = 2; //keep track of how many points we create
            while(pos < 4000) {

                uint8_t r = fmix32(big_seed+seed++); //a nice repeatable random value

                //r = (r*r)/256;

                pos += 256+r*8; //the position of this branch

                if (pos >= 4000) { //don't go beyond the tip of the trunk
                    break;
                }

                //here is our branch
                points[num_points] = VECTOR3(pos, pos,0);
                points[num_points+1] = VECTOR3(pos, pos+128+r*12,0);

                //create a symmetrical branch on the opposite side
                points[num_points+2] = VECTOR3(pos, pos,0);
                points[num_points+3] = VECTOR3(pos+128+r*12, pos,0);

                num_points+=4;

            }

            //alter the brightness based on z-distance from the camera
            int16_t v = ((flake->z/256+1500)*224)/1500+32;
            v = _min(_max(v,0), 255);
            CRGB rgb = CHSV(flake->hue,flake->sat,v);

            //rotation around each axis at a random rate
            flake->rx+= ((uint8_t)fmix32(flake->seed)-128)*2;
            flake->ry+= ((uint8_t)fmix32(flake->seed+1)-128)*2;
            flake->rz+= ((uint8_t)fmix32(flake->seed+2)-128)*2;
            
            //update the snowflake's position
            flake->x+=flake->vx;
            flake->y+=flake->vy;
            flake->z+=flake->vz;


            //figure out acceleration due to gravity and air-resistance
            VECTOR3 normal = VECTOR3(0,0,256);
            matrix.rotate_x(normal, flake->rx/256);
            matrix.rotate_y(normal, flake->ry/256);
            matrix.rotate_z(normal, flake->rz/256);
            matrix.rotate(normal);

            VECTOR3 normal2 = normal; //make a copy that we'll use later

            //angle around y-axis
            uint8_t angle_y = -(atan2(normal.x, normal.z)*256)/(2*PI)+64;

            //normalize the angle around the y-axis
            matrix.rotate_y(normal, -angle_y);
            //rotate 90 degrees to always point down
            matrix.rotate_z(normal, -64);
            //we'll use the z-angle later
            uint8_t angle_z = -(atan2(normal.x, normal.y)*256)/(2*PI)+64;
            //restore the y-axis
            matrix.rotate_y(normal, angle_y);

            //use the y value as our magnitude
            //large y: pointing down, minimum air resistance, fast
            //y = 0: flat, max air resistance, slow
            normal.x = (normal.x * abs(normal.y)) / 128;
            normal.z = (normal.z * abs(normal.y)) / 128;
            normal.y = (normal.y * abs(normal.y)) / 128;

            //now we can add this value as our "acceleration" (not really treating it as acceleration though, just speed) due to gravity
            flake->x += normal.x;
            flake->y += normal.y;
            flake->z += normal.z;



            //now we'll simulate wind-resistance "pushing" the snowflake to a neutral flat position

            // int ryc0 = _min(0 - flake->ry, 256*256 - flake->ry);
            // int ryc1 = 128*256 - flake->ry;

            // if (abs(ryc0) < abs(ryc1)) {
            //     flake->ry += ryc0/100;
            // } else {
            //     flake->ry += ryc1/100;
            // }

            // int rxc0 = 64*256 - flake->rx;
            // int rxc1 = 192*256 - flake->rx;

            // if (abs(rxc0) < abs(rxc1)) {
            //     flake->rx += rxc0/100;
            // } else {
            //     flake->rx += rxc1/100;
            // }

            

/*
            //debug stuff
            normal.x*=20;
            normal.y*=20;
            normal.z*=20;

            normal.x += flake->x;
            normal.y += flake->y;
            normal.z += flake->z;

            matrix.scale_z(normal);
            matrix.perspective(normal);


            VECTOR3 flake_v = VECTOR3(flake->x,flake->y,flake->z);
            matrix.scale_z(flake_v);
            matrix.perspective(flake_v);

            draw_line_fine(leds, flake_v.x, flake_v.y, normal.x, normal.y, 196, 0, 255);
*/
            //now to draw each snowflake
            //go through our array of points and draw each line
            uint8_t on_screen = 0;
            for (int i = 0; i < num_points; i+=2) {
                VECTOR3 p_a = points[i];
                VECTOR3 p_b = points[i+1];
                
                //draw six times to make a symmetrical hexagonal shape
                for (int j = 0; j < 6; j++) {

                    VECTOR3 p0 = p_a;
                    VECTOR3 p1 = p_b;

                    //there are 256 degrees in a full circle
                    matrix.rotate_z(p0, (256*j)/6);
                    matrix.rotate_z(p1, (256*j)/6);

                    //add the snowflake's own rotation
                    matrix.rotate_x(p0, flake->rx/256);
                    matrix.rotate_y(p0, flake->ry/256);
                    matrix.rotate_z(p0, flake->rz/256);
                    matrix.rotate_x(p1, flake->rx/256);
                    matrix.rotate_y(p1, flake->ry/256);
                    matrix.rotate_z(p1, flake->rz/256);

                    //translate the snowflake to absolute coordinates
                    p0.x+=flake->x;
                    p0.y+=flake->y;
                    p0.z+=flake->z;
                    p1.x+=flake->x;
                    p1.y+=flake->y;
                    p1.z+=flake->z;
                    
                    //rotate according to our global matrix
                    matrix.rotate(p0);
                    matrix.rotate(p1);
                    //adjustable scale for the z-axis
                    matrix.scale_z(p0.z);
                    matrix.scale_z(p1.z);
                    //add 3D perspective
                    matrix.perspective(p0);
                    matrix.perspective(p1);

                    //finally draw the line
                    draw_line_fine(leds, p0, p1, rgb);

                    //check to make sure at least one point is visible on the screen
                    if (!on_screen) {
                        if (p0[0] >= 0 && p0[0] < MATRIX_WIDTH*256 && p0[1] >= 0 && p0[1] < MATRIX_HEIGHT*256) {
                            on_screen = true;
                        }

                        if (p1[0] >= 0 && p1[0] < MATRIX_WIDTH*256 && p1[1] >= 0 && p1[1] < MATRIX_HEIGHT*256) {
                            on_screen = true;
                        }
                    }

                }
    

            }

            if (flake->age) {
                flake->age--;
            } else if (!on_screen) {
                flake_reset(flake, false);
            }

        }

    }


};


LIGHT_SKETCHES::REGISTER<SNOWFLAKES> snowflakes("snowflakes");
//END NEON

#endif