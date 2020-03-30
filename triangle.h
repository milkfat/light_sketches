#ifndef LIGHTS_TRIANGLE_H
#define LIGHTS_TRIANGLE_H

    //return a unit vector representing the surface normal of triangle a,b,c
    static VECTOR3 inline __attribute__((always_inline)) normal(const VECTOR3& a, const VECTOR3& b, const VECTOR3& c) {
        VECTOR3 norm;
        VECTOR3 u(b.x-a.x, b.y-a.y, b.z-a.z);
        VECTOR3 v(c.x-a.x, c.y-a.y, c.z-a.z);
        //std::cout << " u: " << u.x << ", " << u.y << ", " << u.z << "\n";
        //std::cout << " v: " << v.x << ", " << v.y << ", " << v.z << "\n";
        norm.x = u.y*v.z - u.z*v.y;
        norm.y = u.z*v.x - u.x*v.z;
        norm.z = u.x*v.y - u.y*v.x;
        //std::cout << " n: " << norm.x << ", " << norm.y << ", " << norm.z << "\n";
        //norm /= 1024;
        uint32_t m = _max(_max(abs(norm.x),abs(norm.y)),abs(norm.z))/128;
        if (m) {
            norm/=m;
        }
        //std::cout << "n2: " << norm.x << ", " << norm.y << ", " << norm.z << "\n";
        int32_t length = sqrt16(norm.x*norm.x+norm.y*norm.y+norm.z*norm.z);
        //std::cout << " l: " << length << "\n";
        if (length != 0) {
          norm.x = (norm.x*255)/length;
          norm.y = (norm.y*255)/length;
          norm.z = (norm.z*255)/length;
        } else {
          norm.x = 0;
          norm.y = 0;
          norm.z = 0;
        }
        //rotate_x(norm,-24);
        return norm;
    }

    //draw a flat shaded triangle
    void draw_triangle_flat(VECTOR3& a, VECTOR3& b, VECTOR3& c, VECTOR3& orig, VECTOR3& norm, uint8_t& hue, uint8_t& sat, uint8_t& val) {
      
      draw_line_ybuffer(a, b);
      draw_line_ybuffer(b, c);
      draw_line_ybuffer(c, a);

      int32_t z_depth = orig.z+norm.z;


      //rotate_x(norm,32);
      //rotate_y(norm,32);

      int bri = _min(_max(((255-norm.z)*3)/4, 0) + bri/4,220);

      CRGB rgb = CHSV(hue,sat,val);
      color_scale(rgb, bri);

      //draw_line_fine(led_screen, a, b, rgb, z_depth, 255, 255, true);
      //draw_line_fine(led_screen, b, c, rgb, z_depth, 255, 255, true);
      //draw_line_fine(led_screen, c, a, rgb, z_depth, 255, 255, true);
      
      //CRGB rgb(0,0,0);
      //CRGB rgb2 = CHSV(hue,sat,val);
      //nblend(rgb, rgb2, bri);

      //fill between the pixels of our lines
      for (int y = y_buffer_min; y <= _min(y_buffer_max,MATRIX_HEIGHT-1); y++) {
          if (y_buffer[y][0] <= y_buffer[y][1]) {

          for (int x = y_buffer[y][0]; x <= y_buffer[y][1]; x++) {
            drawXYZ(led_screen, x, y, orig.z, rgb);
          }

        }
        //clear the buffer to be used for filling the triangle
        y_buffer[y][0] = MATRIX_WIDTH + 1;
        y_buffer[y][1] = -1;
      
      }

      y_buffer_max = 0;
      y_buffer_min = MATRIX_HEIGHT-1;

    
    } //void draw_triangle_flat(VECTOR3& a, VECTOR3& b, VECTOR3& c, VECTOR3& orig, VECTOR3& norm, uint8_t& hue, uint8_t& sat, uint8_t& val)





    //draw a triangle and calculate x,y,z as well as the ratio of a-b-c for each pixel
    bool draw_triangle(VECTOR3& a, VECTOR3& b, VECTOR3& c, VECTOR3& norm_a, VECTOR3& norm_b, VECTOR3& norm_c, const CRGB& rgb = CRGB(255,255,255)) {
      bool on_screen = false;
      Y_BUF y_buffer2[MATRIX_HEIGHT][2];

      reset_y_buffer2(y_buffer2);

      int orientation = ((b.y-a.y))*((c.x-b.x)) - ((c.y-b.y))*((b.x-a.x));
      //int orientation = ((b.y-a.y)/256)*((c.x-b.x)/256) - ((c.y-b.y)/256)*((b.x-a.x)/256);
      
      if ( orientation < 0 ) {

        static const VECTOR3 a_val(255,0,0);
        static const VECTOR3 b_val(0,255,0);
        static const VECTOR3 c_val(0,0,255);

        draw_line_ybuffer(y_buffer2, a, a_val, b, b_val);
        draw_line_ybuffer(y_buffer2, b, b_val, c, c_val);
        draw_line_ybuffer(y_buffer2, c, c_val, a, a_val);

        // CRGB rgb(a_norm.x,a_norm.y,a_norm.z);

        // draw_line_fine(led_screen, a, b, rgb, a.z, 255, 255, true);
        // draw_line_fine(led_screen, b, c, rgb, a.z, 255, 255, true);
        // draw_line_fine(led_screen, c, a, rgb, a.z, 255, 255, true);
        
        //fill between the pixels of our lines
        for (int y = _max(y_buffer_min,0); y <= _min(y_buffer_max,MATRIX_HEIGHT-1); y++) {

            int32_t dist_x = y_buffer2[y][1].position.x - y_buffer2[y][0].position.x;

          if (dist_x >= 0) {


            VECTOR3 * ratio  = &y_buffer2[y][0].ratio;
            VECTOR3 * ratio2 = &y_buffer2[y][1].ratio;
            // std::cout << "a: " << ratio->x << ", " << ratio->y << ", " << ratio->z << "\n";
            // std::cout << "b: " << ratio2->x << ", " << ratio2->y << ", " << ratio2->z << "\n\n";

            VECTOR3 err_ratio(0,0,0);

            VECTOR3 dist_ratio = *ratio2 - *ratio;
            VECTOR3 a_dist_ratio = abs(dist_ratio);
            VECTOR3 step_ratio( sgn(dist_ratio.x), sgn(dist_ratio.y), sgn(dist_ratio.z) );

            int32_t x = y_buffer2[y][0].position.x;

            while (x <= y_buffer2[y][1].position.x) {

              VECTOR3 norm = ( (norm_a*ratio->x)/255 + (norm_b*ratio->y)/255 + (norm_c*ratio->z)/255 ).unit();

              CRGB new_rgb;

              //new_rgb.r = _max((ratio->x*norm.z)/255,0);
              //new_rgb.g = _max((ratio->y*norm.z)/255,0);
              //new_rgb.b = _max((ratio->z*norm.z)/255,0);
              uint8_t norm_bri = (_min(_max(norm.z,0),255)*3)/4 + 64;

              new_rgb.r = (norm_bri*rgb.r)/256;
              new_rgb.g = (norm_bri*rgb.g)/256;
              new_rgb.b = (norm_bri*rgb.b)/256;

              on_screen = drawXYZ(led_screen, x, y, y_buffer2[y][0].position.z*256, new_rgb,false) || on_screen; //gamma
              x++;
            
              if (dist_x > 0) {

                iterate(*ratio,step_ratio,a_dist_ratio,err_ratio,dist_x);
                
              }

            }
            // CRGB rgb(  y_buffer2[y][0].ratio.x, y_buffer2[y][0].ratio.y, y_buffer2[y][0].ratio.z );
            // CRGB rgb2( y_buffer2[y][1].ratio.x, y_buffer2[y][1].ratio.y, y_buffer2[y][1].ratio.z );
            // drawXYZ(led_screen, y_buffer2[y][0].position.x, y_buffer2[y][0].position.y, y_buffer2[y][0].position.z, rgb, true);
            // drawXYZ(led_screen, y_buffer2[y][1].position.x, y_buffer2[y][1].position.y, y_buffer2[y][1].position.z, rgb2, true);

          }
          //clear the buffer to be used for filling the triangle
          //y_buffer2[y][0].position.x = MATRIX_WIDTH*256;
          //y_buffer2[y][1].position.x = -1;
        
        }

        y_buffer_max = 0;
        y_buffer_min = MATRIX_HEIGHT-1;

      }
      return on_screen;
    } //void draw_triangle()


//draw a triangle and calculate x,y,z as well as the ratio of a-b-c for each pixel
    bool draw_triangle_fine(VECTOR3& a, VECTOR3& b, VECTOR3& c, VECTOR3& norm_a, VECTOR3& norm_b, VECTOR3& norm_c, const CRGB& rgb = CRGB(255,255,255)) {
      bool on_screen = false;
      Y_BUF y_buffer2[MATRIX_HEIGHT][2];

      reset_y_buffer2(y_buffer2);

      int orientation = ((b.y-a.y))*((c.x-b.x)) - ((c.y-b.y))*((b.x-a.x));
      //int orientation = ((b.y-a.y)/256)*((c.x-b.x)/256) - ((c.y-b.y)/256)*((b.x-a.x)/256);
      
      if ( orientation < 0 ) {

        static const VECTOR3 a_val(255,0,0);
        static const VECTOR3 b_val(0,255,0);
        static const VECTOR3 c_val(0,0,255);

        draw_line_ybuffer_fine(y_buffer2, a, a_val, b, b_val);
        draw_line_ybuffer_fine(y_buffer2, b, b_val, c, c_val);
        draw_line_ybuffer_fine(y_buffer2, c, c_val, a, a_val);

        // CRGB rgb(a_norm.x,a_norm.y,a_norm.z);

        // draw_line_fine(led_screen, a, b, rgb, a.z, 255, 255, true);
        // draw_line_fine(led_screen, b, c, rgb, a.z, 255, 255, true);
        // draw_line_fine(led_screen, c, a, rgb, a.z, 255, 255, true);
        
        //fill between the pixels of our lines
        for (int y = _max(y_buffer_min,0); y <= _min(y_buffer_max,MATRIX_HEIGHT-1); y++) {
            int32_t dist_x = y_buffer2[y][1].position.x/256 - y_buffer2[y][0].position.x/256;

          if (dist_x >= 0) {


            VECTOR3 * ratio  = &y_buffer2[y][0].ratio;
            VECTOR3 * ratio2 = &y_buffer2[y][1].ratio;
        
            uint16_t opacity = y_buffer2[y][0].opacity;
            opacity = 255;
            uint16_t opacity2 = y_buffer2[y][1].opacity;
            // std::cout << "a: " << ratio->x << ", " << ratio->y << ", " << ratio->z << "\n";
            // std::cout << "b: " << ratio2->x << ", " << ratio2->y << ", " << ratio2->z << "\n\n";
            int16_t opacity_dist = 0;
            // if ( (y+1)<MATRIX_HEIGHT && y_buffer2[y+1][0].position.x/256 > y_buffer2[y][0].position.x/256+1) {
            //     opacity_dist = y_buffer2[y+1][0].position.x/256 - y_buffer2[y][0].position.x/256+1;
            //     opacity = 0;
            // }
            // if ( (y-1) > 0 && y_buffer2[y-1][0].position.x/256 > y_buffer2[y][0].position.x/256+1) {
            //     opacity_dist = y_buffer2[y-1][0].position.x/256 - y_buffer2[y][0].position.x/256;
            //     opacity = 0;
            // }
            int16_t step_opacity = 255;
            // if(opacity_dist > 0) {
            //     step_opacity = (255-opacity)/opacity_dist;
            // }

            VECTOR3 err_ratio(0,0,0);
            VECTOR3 dist_ratio = *ratio2 - *ratio;
            VECTOR3 a_dist_ratio = abs(dist_ratio);
            VECTOR3 step_ratio( sgn(dist_ratio.x), sgn(dist_ratio.y), sgn(dist_ratio.z) );

            int32_t x = y_buffer2[y][0].position.x/256;
            int32_t remainder_low = 255-((y_buffer2[y][0].position.x+512)%256);
            int32_t remainder_high = (y_buffer2[y][1].position.x+512)%256;

            int32_t z = y_buffer2[y][0].position.z;
            int32_t err_z = 0;
            int32_t dist_z = y_buffer2[y][1].position.z - y_buffer2[y][0].position.z;
            int32_t a_dist_z = abs(dist_z);
            int32_t step_z = sgn(dist_z);
            while (x <= y_buffer2[y][1].position.x/256) {

              VECTOR3 norm = ( (norm_a*ratio->x)/255 + (norm_b*ratio->y)/255 + (norm_c*ratio->z)/255 ).unit();

              CRGB new_rgb;


              //new_rgb.r = _max((ratio->x*norm.z)/255,0);
              //new_rgb.g = _max((ratio->y*norm.z)/255,0);
              //new_rgb.b = _max((ratio->z*norm.z)/255,0);
              uint8_t norm_bri = (_min(_max(norm.z,0),255)*3)/4 + 64; //minimum brightness 64
                if (x == y_buffer2[y][0].position.x/256) {
                    new_rgb.r = (norm_bri*rgb.r*opacity)/(255*255);
                    new_rgb.g = (norm_bri*rgb.g*opacity)/(255*255);
                    new_rgb.b = (norm_bri*rgb.b*opacity)/(255*255);
                    on_screen = drawXYZ2(led_screen, x, y, z, new_rgb,remainder_low,false) || on_screen; //gamma
                } else if (x == y_buffer2[y][1].position.x/256) {
                        new_rgb.r = (norm_bri*rgb.r*opacity)/(255*255);
                        new_rgb.g = (norm_bri*rgb.g*opacity)/(255*255);
                        new_rgb.b = (norm_bri*rgb.b*opacity)/(255*255);
                    on_screen = drawXYZ2(led_screen, x, y, z, new_rgb,remainder_high,false) || on_screen; //gamma
                } else {
                    new_rgb.r = (norm_bri*rgb.r*opacity)/(255*255);
                    new_rgb.g = (norm_bri*rgb.g*opacity)/(255*255);
                    new_rgb.b = (norm_bri*rgb.b*opacity)/(255*255);
                    on_screen = drawXYZ(led_screen, x, y, z, new_rgb,false) || on_screen; //gamma
                }

              x++;
            
              if (dist_x > 0) {

                iterate(*ratio,step_ratio,a_dist_ratio,err_ratio,dist_x);
                opacity+=step_opacity;
                opacity=_min(opacity,255);

                err_z += a_dist_z;
                while (err_z >= dist_x) {
                    z += step_z;
                    err_z -= dist_x;
                }
                
              }

            }
            // CRGB rgb(  y_buffer2[y][0].ratio.x, y_buffer2[y][0].ratio.y, y_buffer2[y][0].ratio.z );
            // CRGB rgb2( y_buffer2[y][1].ratio.x, y_buffer2[y][1].ratio.y, y_buffer2[y][1].ratio.z );
            // drawXYZ(led_screen, y_buffer2[y][0].position.x, y_buffer2[y][0].position.y, y_buffer2[y][0].position.z, rgb, true);
            // drawXYZ(led_screen, y_buffer2[y][1].position.x, y_buffer2[y][1].position.y, y_buffer2[y][1].position.z, rgb2, true);

          }
          //clear the buffer to be used for filling the triangle
          //y_buffer2[y][0].position.x = MATRIX_WIDTH*256;
          //y_buffer2[y][1].position.x = -1;
        
        }

        y_buffer_max = 0;
        y_buffer_min = MATRIX_HEIGHT-1;

      }
      return on_screen;
    } //void draw_triangle_fine()







    void draw_triangle_flat(VECTOR3& a, VECTOR3& b, VECTOR3& c, uint8_t& hue, uint8_t& sat, uint8_t& val, const bool& two_sided = true) {
      
      //optimization:
      //identify clockwise/counterclockwise orientation
      //draw in only one orientation (facing toward the camera)
      int orientation = (b.y-a.y)*(c.x-b.x) - (c.y-b.y)*(b.x-a.x);
      
      

      if ( orientation < 0 ) {
        VECTOR3 norm = normal(a,b,c);
        VECTOR3 orig;
        orig = (a+b+c)/3;
        draw_triangle_flat(a,b,c,orig,norm,hue,sat,val);
        return;
      }

      if (two_sided) {
        VECTOR3 norm = normal(b,a,c);
        VECTOR3 orig;
        orig = (a+b+c)/3;
        draw_triangle_flat(a,c,b,orig,norm,hue,sat,val);
      }

    } //void draw_triangle_flat(VECTOR3& a, VECTOR3& b, VECTOR3& c, uint8_t& hue, uint8_t& sat, uint8_t& val)


#endif