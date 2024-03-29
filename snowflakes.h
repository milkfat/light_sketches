#ifndef LIGHTS_SNOWFLAKES_H
#define LIGHTS_SNOWFLAKES_H


class SNOWFLAKES: public LIGHT_SKETCH {
  public:
    SNOWFLAKES () {setup();}
    ~SNOWFLAKES () {}
#define MAX_NUM_SNOWFLAKES 255


  private:
    uint8_t num_snowflakes = 32;

    int current_effect = 0;

    uint32_t frame_time = 0;

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
        VECTOR3 norm;
        int32_t flag = 0;
        uint8_t on_screen;
    };

    Z_BUF _z_buffer;
    SNOWFLAKE snowflakes[MAX_NUM_SNOWFLAKES];
    uint32_t big_seed = 0;
    uint32_t snow_noise_position = 0;


    void flake_reset(SNOWFLAKE * flake, uint8_t on_screen) {
            flake->age = 60;
            flake->seed = random(65536);

            VECTOR3 p;

            p.z = random(-1000*256,0*256);

            int offset = 8000;
            if (current_effect/2 % 2) {
                offset = 0;
            }


            if(on_screen) { //initial snowflake coordinates will be created on-screen
                p.x = random(MATRIX_WIDTH*256);
                p.y = random(MATRIX_HEIGHT*256);
                led_screen.reverse_perspective(p);
                flake->vx = random(-128,128);
            } else { //new snowflakes will be created slightly off the edge of the screen
                int r = random(MATRIX_HEIGHT+MATRIX_WIDTH);
                if (r >= MATRIX_HEIGHT) {
                    //snowflakes originate at top of screen
                    p.x = random(MATRIX_WIDTH*256);
                    p.y = (MATRIX_HEIGHT)*256;
                    led_screen.reverse_perspective(p);
                    p.y += offset;
                    flake->vx = random(-96,96);
                } else if (r >= MATRIX_HEIGHT/2) {
                    //snowflakes originate at right of screen
                    p.x = (MATRIX_WIDTH)*256;
                    int r = random(MATRIX_HEIGHT);
                    //r = (r*r)/MATRIX_HEIGHT;
                    r = MATRIX_HEIGHT-r;
                    p.y = r*256;
                    led_screen.reverse_perspective(p);
                    p.x += offset;
                    flake->vx = random(8,64);
                    flake->vx *= -1;
                } else {
                    //snowflakes originate at left of screen
                    p.x = -256;
                    int r = random(MATRIX_HEIGHT);
                    //r = (r*r)/MATRIX_HEIGHT;
                    r = MATRIX_HEIGHT-r;
                    p.y = r*256;
                    led_screen.reverse_perspective(p);
                    p.x -= offset;
                    flake->vx = random(8,64);
                }
            }
            flake->x = p.x;
            flake->y = p.y;
            flake->z = p.z;
            flake->vy = -480;
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
        current_effect++;
        current_effect%=4;
    }

    void setup() {
        reset();
        control_variables.add(num_snowflakes, "Number of flakes", 0, 255);
        control_variables.add(led_screen.camera_position.z, "Camera Z", 0, 1024*256);
        control_variables.add(led_screen.screen_distance, "Screen Z", 0, 1024*256);
        control_variables.add(led_screen.light_falloff, "Light Distance:", 1, 16);
    }

    void reset() {
        frame_time = millis();
        led_screen.light_falloff = 10;
        led_screen.rotation_alpha = 0;
        led_screen.rotation_beta = 90;
        led_screen.rotation_gamma = 0;
        big_seed = random(65535);
        for (int i = 0; i < num_snowflakes; i++) {
            flake_reset(&snowflakes[i], true);
        }
        led_screen.camera_position.z = 570*256;
        led_screen.screen_distance = 396*256;
        LED_black();

    }


    void loop() {
        snow_noise_position+=2;
        
        //update the screen
        LED_show();

        LED_black();

        //reset the snowflake's "drawn" flag
        for (int i = 0; i < num_snowflakes; i++) {
            snowflakes[i].flag = 1;
        }

        //snowflake physics
        while (millis() > frame_time) {
            for (int i = 0; i < num_snowflakes; i++) {
                flake_physics(&snowflakes[i]);
            }
            frame_time+=16;
        }

        //draw snowflakes
        for (int f = 0; f < num_snowflakes; f++) {

            //we want to draw our snowflakes from back to front
            //so we find the most distant snowflake that hasn't yet been drawn
            int max_z = 1000000;
            int max_flake = 0;
            for (int i = 0; i < num_snowflakes; i++) {
                if (snowflakes[i].flag) {
                    if (snowflakes[i].z < max_z) {
                        max_z = snowflakes[i].z;
                        max_flake = i;
                    }
                }
            }
            SNOWFLAKE * flake = &snowflakes[max_flake];
            flake->flag=0;

            CRGB rgb;
            
            if (current_effect%2)
            {
                rgb = CHSV(flake->hue,0,255);
            }
            else
            {
                rgb = CHSV(flake->hue,flake->sat,255);
            }

            if (current_effect/2 % 2) {
                draw_simple_flake(flake, rgb);
            } else {
                draw_fancy_flake(flake, rgb);
            }

            if (flake->age) {
                flake->age--;
            } else if (flake->y < 0 && !flake->on_screen) {
                flake_reset(flake, false);
            }

        }

    }





    
    void flake_physics(SNOWFLAKE * flake) {

        //uint32_t seed = flake->seed;

        //rotation around each axis at a random rate
        flake->rx+= ((uint8_t)fmix32(flake->seed)-128)*2;
        flake->ry+= ((uint8_t)fmix32(flake->seed+1)-128)*2;
        flake->rz+= ((uint8_t)fmix32(flake->seed+2)-128)*2;
        
        //update the snowflake's position
        flake->x+=flake->vx;
        flake->y+=flake->vy;
        flake->z+=flake->vz;

        //hmm, some sort of wind?
        flake->x+=(inoise8(flake->x/500, flake->y/500, flake->z/1000+1000+snow_noise_position)-128)*3;
        flake->y+=(inoise8(flake->x/500, flake->y/1000+snow_noise_position*.8f, flake->z/500)-32);
        flake->z+=(inoise8(flake->x/1000+1000+snow_noise_position*1.2f, flake->y/500, flake->z/500)-128)*3;

        //gravity
        flake->vy -= 8;

        //figure out air-resistance

        //surface normal
        //large y value means the flake is flat
        VECTOR3 normal = VECTOR3(0,0,256);
        rotate_x(normal, flake->rx/256);
        rotate_y(normal, flake->ry/256);
        rotate_z(normal, flake->rz/256);
        led_screen.matrix.rotate(normal);
        flake->norm = normal;

        //figure out the magnitude of our x and z components (horizontal motion)
        //multiply by inverse y
        int x_magnitude = (normal.x*-(0-normal.y))/256; // -128 to 128
        int z_magnitude = (normal.z*-(0-normal.y))/256; // -128 to 128

        //combine magnitude with vy to find the desired velocity
        int vx_wind = (x_magnitude*abs(flake->vy))/256;
        int vz_wind = (z_magnitude*abs(flake->vy))/256;


        //adjust our current velocity toward our desired velocity (by a fraction)
        flake->vx+= (vx_wind-flake->vx)/20;
        flake->vz+= (vz_wind-flake->vz)/20;

        //air resistance is velocity squared
        int vy2 = (flake->vy*flake->vy); // 0-65535

        //finally adjust our y velocity for air resistance
        //air resistance never fully goes to zero
        flake->vy+= ((((abs(normal.y)*250)/256+5)*vy2)/((256*256*256)))*4;



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

        scale_z(normal);
        led_screen.perspective(normal);


        VECTOR3 flake_v = VECTOR3(flake->x,flake->y,flake->z);
        scale_z(flake_v);
        led_screen.perspective(flake_v);

        draw_line_fine(led_screen, flake_v.x, flake_v.y, normal.x, normal.y, 196, 0, 255);
*/
    }





    //snowflakes are 0-dimensional dots
    void draw_simple_flake(SNOWFLAKE * flake, CRGB& rgb) {

        flake->on_screen = 0;
        VECTOR3 p = VECTOR3(flake->x, flake->y, flake->z);
        
        //rotate according to our global matrix
        led_screen.matrix.rotate(p);
        //add 3D perspective
        led_screen.perspective(p);

        blendXY(led_screen, p, rgb);

        //check to make sure at least one point is visible on the screen
        if (!flake->on_screen) {
            if (p.x >= 0 && p.x < MATRIX_WIDTH*256 && p.y >= 0 && p.y < MATRIX_HEIGHT*256) {
                flake->on_screen=true;
            }

        }


    }






    //draw each snowflake procedurally
    void draw_fancy_flake(SNOWFLAKE * flake, CRGB& rgb) {

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


        //now to draw each snowflake
        //go through our array of points and draw each line
        flake->on_screen = 0;
        
        for (int i = 0; i < num_points; i+=2) {
            VECTOR3 p_a = points[i];
            VECTOR3 p_b = points[i+1];
            
            //draw six times to make a symmetrical hexagonal shape
            for (int j = 0; j < 6; j++) {

                VECTOR3 p0 = p_a;
                VECTOR3 p1 = p_b;

                //there are 256 degrees in a full circle
                rotate_z(p0, (256*j)/6);
                rotate_z(p1, (256*j)/6);

                //add the snowflake's own rotation
                rotate_x(p0, flake->rx/256);
                rotate_y(p0, flake->ry/256);
                rotate_z(p0, flake->rz/256);
                rotate_x(p1, flake->rx/256);
                rotate_y(p1, flake->ry/256);
                rotate_z(p1, flake->rz/256);

                //translate the snowflake to absolute coordinates
                p0.x+=flake->x;
                p0.y+=flake->y;
                p0.z+=flake->z;
                p1.x+=flake->x;
                p1.y+=flake->y;
                p1.z+=flake->z;
                
                //rotate according to our global matrix
                led_screen.matrix.rotate(p0);
                led_screen.matrix.rotate(p1);
                //add 3D perspective
                led_screen.perspective_lp(p0.x,p0.y,p0.z);
                led_screen.perspective_lp(p1.x,p1.y,p1.z);

                //finally draw the line
                draw_line_fine(led_screen, p0, p1, rgb);

                //check to make sure at least one point is visible on the screen
                if (!flake->on_screen) {
                    if (p0[0] >= 0 && p0[0] < MATRIX_WIDTH*256 && p0[1] >= 0 && p0[1] < MATRIX_HEIGHT*256) {
                        flake->on_screen = true;
                    }

                    if (p1[0] >= 0 && p1[0] < MATRIX_WIDTH*256 && p1[1] >= 0 && p1[1] < MATRIX_HEIGHT*256) {
                        flake->on_screen = true;
                    }
                }

            }


        }

    }

};


LIGHT_SKETCHES::REGISTER<SNOWFLAKES> snowflakes("snowflakes");
//END NEON

#endif