#ifndef LIGHTS_DRAW_CURVE_H
#define LIGHTS_DRAW_CURVE_H

//DRAW CURVE

//draw a curve by simultaneously shortening and rotating the line segment vectors

static inline __attribute__ ((always_inline)) void matt_curve(int32_t coordinate_array[][2], const size_t& len, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255, const bool& flipXY = false, const bool& closedShape = false, const bool& extraSmooth = false) {

  //a variable to store the angle for segment 2 from the previous pass (which will become segment 1 of the current pass)
  //we must blend the curves together to make one smooth continuous curve
  float last_a2b;

  //a variable to store the angle and length of the previous segment
  float last_exp; 
  float last_len;

  int starting_point = 0;
  if (closedShape) {
    starting_point = -1;
  }
  int ending_point = len-2;
  if (closedShape) {
    ending_point = len;
  }

  for (int i = starting_point;i<ending_point;i++) {
    int i0 = i;
    if (i0 == -1) {
      i0 = len-1;
    }
    int i1 = (i+1) % len;
    int i2 = (i+2) % len;


    //calculate three points at a time (in other words: two line segments)

    int32_t& x0 = coordinate_array[i0][0];
    int32_t& y0 = coordinate_array[i0][1];
       //segment 1 between these points
    int32_t& x1 = coordinate_array[i1][0];
    int32_t& y1 = coordinate_array[i1][1];
       //segment 2 between these points
    int32_t& x2 = coordinate_array[i2][0];
    int32_t& y2 = coordinate_array[i2][1];

    //angle of first segment
    float a0  = atan2(x1 - x0, y1 - y0);
    
    //normalize angle to the range: -PI to PI
    if (a0 > PI) {
      a0 -= 2.f*PI;
    } else if (a0 < -PI) {
      a0 += 2.f*PI;
    }

    //angle of second segment
    float a0b  = atan2(x1 - x2, y1 - y2);
    
    //normalize angle
    if (a0b > PI) { 
      a0b -= 2.f*PI;
    } else if (a0b < -PI) {
      a0b += 2.f*PI;
    }

    //figure out our orientation so that we can accurately add or subtract PI/2 (90 degrees)
    int dir = 1;
    float diff = a0-a0b;
    
    if(diff < -PI)
        diff += PI;
    if(diff > PI)
        diff -= PI;

    if (diff < 0) {  
      dir = -1;
    }
    
    //calculate the angle of the "normal line" for the two segments (perpendicular to the angle halfway between the two segments)
    float a4 = (a0+a0b)/2.f+dir*PI/2.f;
    
    if (a4 > PI) { //normalize angle
      a4 -= 2.f*PI;
    } else if (a4 < -PI) {
      a4 += 2.f*PI;
    }
    
    //the angular difference between line 1 and the "normal line"
    //we will sweep through this angle to draw our curve (while simultaneously shortening the vector)
    float a2 = a0-a4;
    
    //normalize angle
    if (a2 > PI) { 
      a2 -= 2.f*PI;
    } else if (a2 < -PI) {
      a2 += 2.f*PI;
    }

    //the angular difference between segment 2 and the "normal line"
    //we will sweep through this angle to draw our curve (while simultaneously shortening the vector)
    float a2b = a0b-(a4+PI); 

    //normalize angle
    if (a2b > PI) {
      a2b -= 2.f*PI;
    } else if (a2b < -PI) {
      a2b += 2.f*PI;
    }

    //time from 0-256 (8-bit equivalent of 0.0-1.0)
    int stp = 0; 
    float last_x;
    float last_y;
    float last_xb;
    float last_yb;

    //draw the curve *later* (sharper) for small angles
    //90 degrees = linear curve
    //45 degrees = exponential curve 
    //0 and 180 degrees = no curve
    
    float peepee0 = fabs(0-a2); //calculate the angle difference between the two segments: 0 = no change (0 and 180 degrees), 1 = max change (90 degrees)
    float peepee1 = fabs(0-a2b); //calculate the angle difference between the two segments: 0 = no change (0 and 180 degrees), 1 = max change (90 degrees)

    float exp0=sin(peepee0*2);
    float exp1=sin(peepee1*2);

    //draw the curve *later* (sharper) for short segments
    //long = linear curve
    //short = exponential curve
    //really short = no curve

    float len0 = _min((x0-x1) * (x0-x1)/256.f + (y0-y1) * (y0-y1)/256.f, 255)/255.f;
    float len1 = _min((x1-x2) * (x1-x2)/256.f + (y1-y2) * (y1-y2)/256.f, 255)/255.f;

    
    float len = _min(len0,len1);
    len = _max((len-.5f)*2,0);
    float exp = _min(exp0,exp1);
    float w = exp*len;

    float len2 = _min(last_len,len0);
    len2 = _max((len2-.5f)*2,0);
    float exp2 = _min(last_exp,exp0);
    float w2 = exp2*len2;


    //do not draw the first segment for closed shapes
    while(stp <= 256 && !(closedShape && i == -1)) {

      
      int stp_b = stp;
      if (extraSmooth) {
        for (int j = 0; j < 4; j++) {
          stp_b = ease8In(stp_b);
        }
        stp_b = (stp*w + stp_b*(1.f-w));
      }

      //our current offset angle for segment 1
      float angle = (a2*stp_b)/256.f;

      //calculate the angle for segment 1
      float x = ( cos(angle)*(x0-x1) - sin(angle)*(y0-y1) );
      float y = ( sin(angle)*(x0-x1) + cos(angle)*(y0-y1) );

      //calculate the length for segment 1
      x *= (256-stp)/256.f;
      y *= (256-stp)/256.f;

      //translate the vector to its original x,y coordinates
      x += x1;
      y += y1;

      //now we must blend our value with the previous pass
      //do not blend the first segment when drawing non-closed shapes
      if ((i > 0 || closedShape)) {
        
        //the previous pass is drawn in reverse (x1,x2 in the first pass becomes x0,x1 in the second pass)
        int stp2 = 256-stp;

        int stp2_b = stp2;
        if (extraSmooth) {
          for (int j = 0; j < 4; j++) {
            stp2_b = ease8In(stp2_b);
          }
          stp2_b = (stp2*w2 + stp2_b*(1.f-w2));
        }
        
        //our angle offset for segment 2 from the previous pass (which is segment 1 of the current pass)
        float angle2 = (last_a2b*stp2_b)/256.f; 

        //calculate the angle
        float xl = ( cos(angle2)*(x1-x0) - sin(angle2)*(y1-y0) );
        float yl = ( sin(angle2)*(x1-x0) + cos(angle2)*(y1-y0) );

        //calculate the length
        xl *= (stp)/256.f;
        yl *= (stp)/256.f;

        //translate our new vector back to original coordinates
        xl += x0;
        yl += y0;

        //debug points
        //blendXY(led_screen,xl,yl,96);
        //blendXY(led_screen,x,y,160);

        //blend our two passes together, slowly fading from one to the other

        x = (x*(stp))/256.f + (xl*stp2)/256.f;
        y = (y*(stp))/256.f + (yl*stp2)/256.f;


        
      }
      // uint8_t asdf = 0;
      // if (stp%64 == 0) {
      //   asdf = 1;
      // }
      //draw our curve (starting from the second iteration, we need two points, duh)
      if (stp > 0) {
        //draw line between points
        if (flipXY) {
          draw_line_fine(led_screen, last_y, last_x, y, x, hue, sat, val, -10000, val, true);
        } else {

          draw_line_fine(led_screen, last_x, last_y, x, y, hue, sat, val, -10000, val, true);
        }
      }
      //record x,y for the next iteration
      last_x = x;
      last_y = y;
      
      //debug points
      //blendXY(led_screen,x0,y0,96);
      //blendXY(led_screen,x1,y1,160);
      //blendXY(led_screen, x, y);
      //blendXY(led_screen,x,y);

      //on our final pass we need to calculate the last segment without blending
      //do not draw this segment for non-closed shapes
      if (i == ending_point-1 && !closedShape) {
        //blendXY(led_screen,x2,y2,48);
        
        //our current offset angle for segment 2
        float angle2 = (a2b*stp)/256.f;

        //calculate the current angle
        float xb = ( cos(angle2)*(x2-x1) - sin(angle2)*(y2-y1) );
        float yb = ( sin(angle2)*(x2-x1) + cos(angle2)*(y2-y1) );

        //calculate the current length
        xb *= (256-stp)/256.f;
        yb *= (256-stp)/256.f;

        //move the new vector back to its original coordinates
        xb += x1;
        yb += y1;
        
        //blendXY(led_screen,xb,yb);

        //draw our curve
        if (stp > 0) {
          //draw line between points
          if (flipXY) {
            draw_line_fine(led_screen, last_yb, last_xb, yb, xb, hue, sat, val, -10000, val, true);
          } else {
            draw_line_fine(led_screen, last_xb, last_yb, xb, yb, hue, sat, val, -10000, val, true);
          }
        }

        //record our points for the next step in time
        last_xb = xb;
        last_yb = yb;
      }

      //iterate time
      stp+=32;
    }
    
    //store our segment 2 angle to blend with the next pair of line segments
    last_a2b = a2b;
    last_exp = exp1;
    last_len = len1;
  }
}


















//DRAW CURVE 8

//draw a curve by simultaneously shortening and rotating the line segment vectors

static inline __attribute__ ((always_inline)) void matt_curve8(PERSPECTIVE screen_object, int32_t coordinate_array[][2], const size_t& len, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255, const bool& flipXY = false, const bool& closedShape = false, const bool& extraSmooth = false, const uint8_t& percentage = 255, const uint8_t& step_size = 32) {
  CRGB rgb = CHSV(hue, sat, val);
  //draw simple lines for step size of 255
  if (step_size == 255) {
    for (int i = 1; i < len; i++) {
      //draw_line_fine2(screen_object, coordinate_array[i-1][0],coordinate_array[i-1][1],coordinate_array[i][0],coordinate_array[i][1],hue,sat,val);
      draw_line_fine(screen_object,coordinate_array[i-1][0],coordinate_array[i-1][1],coordinate_array[i][0],coordinate_array[i][1], rgb, -10000, val, val, true);
    } 
    return;
  }

  //a variable to store the angle for segment 2 from the previous pass (which will become segment 1 of the current pass)
  //we must blend the curves together to make one smooth continuous curve
  int16_t last_a2b = 0;

  //a variable to store the angle and length of the previous segment
  uint16_t last_w = 0;

  int starting_point = -1;
  int ending_point = len-1;
  if (closedShape) {
    ending_point = len;
  }

  int total_length = ending_point - starting_point;

  for (int i = starting_point;i<ending_point;i++) {
    int i0=i;
    if (i == -1) {
      if (closedShape) {
        i0 = len-1;
      } else {
        i0 = 1;
      }
    }
    int i1 = (i+1) % len;
    int i2 = (i+2) % len;
    if (i == len-2 && !closedShape) {
      i2 = i0;
    }


    //calculate three points at a time (in other words: two line segments)

    int32_t& x0 = coordinate_array[i0][0];
    int32_t& y0 = coordinate_array[i0][1];
       //segment 1 between these points
    int32_t& x1 = coordinate_array[i1][0];
    int32_t& y1 = coordinate_array[i1][1];
       //segment 2 between these points
    int32_t& x2 = coordinate_array[i2][0];
    int32_t& y2 = coordinate_array[i2][1];

    //angle of first segment
    float a0f  = atan2(x1 - x0, y1 - y0);
    int16_t a0 = (a0f*65536*113)/(355*2);

    
    //angle of second segment
    float a0bf  = atan2(x1 - x2, y1 - y2);
    int16_t a0b = (a0bf*65536*113)/(355*2);
    

    //figure out our orientation so that we can accurately add or subtract PI/2 (90 degrees)
    int dir = (a0-a0b < 0) ? -1 : 1;
    
    //calculate the angle of the "normal line" for the two segments (perpendicular to the angle halfway between the two segments)
    int16_t a4 = (a0+a0b)/2+dir*16384;
    
    
    //the angular difference between line 1 and the "normal line"
    //we will sweep through this angle to draw our curve (while simultaneously shortening the vector)
    int16_t a2 = a0-a4;
    

    //the angular difference between segment 2 and the "normal line"
    //we will sweep through this angle to draw our curve (while simultaneously shortening the vector)
    int16_t a2b = a0b-(a4+32768); 


    //time from 0-256 (8-bit equivalent of 0.0-1.0)
    int stp = 0; 
    int32_t last_x;
    int32_t last_y;
    int32_t last_xb;
    int32_t last_yb;

    //modify the curve for smaller angles
    //180 degrees = full linear curve
    //90 degrees = smaller curve 
    //0 degrees = no curve
    
    int16_t exp0=a2-a2b; //angle difference between segments
    exp0 = abs(0 - exp0)/128; //convert to 0-255
    //exp0 = (exp0*exp0)/256; //change is weighted toward the smaller end
    exp0 = 256-exp0; //convert to 255-0
    //exp0 = (sin8(exp0/4)-128)*2; //apply easing with sin8() to get smooth changes at very small angles

    //modify the curve when segments differ greatly in length
    //small difference in length = full linear curve
    //medium difference in length = exponential curve
    //huge difference in length = no curve

    int32_t len0 = sqrt((x0-x1)*(x0-x1) + (y0-y1)*(y0-y1)); //length of first segment
    int32_t len1 = sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)); //length of second segment

    
    int32_t max_len = _max(len0,len1); //maximum length
    if (max_len == 0) {
      continue;
    }
    int32_t min_len = _min(len0,len1); //minimum length
    uint32_t len_d = (min_len << 8)/max_len; //ratio of length: 0 = infinite difference in length, 256 = equal lengths
    uint16_t w_x = len_d;
    //uint16_t w_x = (sin8(len_d/4)-128)*2; //apply easing with sin8() to get smooth changes when lengths differ greatly

    //here is the combined weight to apply to our curve
    //this takes into account two factors:
    // 1) the ratio of the lengths of the two segments
    // 2) the size of the angle between the two segments
    uint16_t w = _min((w_x*exp0)/256,256);
    w = 256-w;
    w = (w*w)>>8;
    w = (w*w)>>8;
    w = (w*w)>>8;
    w = (w*w)>>8;
    w = 256-w;

    //do not draw the first segment
    while(stp <= 256 && i != -1) {

      if ( ((i+1)*stp) / total_length > percentage) {
        return;
      }
      
      int stp_reverse = 256-stp;
      


      int stp_blergh = stp_reverse;
      if (extraSmooth) {
        stp_blergh = (stp_reverse*w)/256;
      }

      //our current offset angle for segment 1
      int16_t angle = (a2*stp)/65536;

      //calculate the angle for segment 1
      int16_t angle_c = cos8(angle)-128;
      int16_t angle_s = sin8(angle)-128;
      int32_t x = ( angle_c*(x0-x1) - angle_s*(y0-y1) )/128;
      int32_t y = ( angle_s*(x0-x1) + angle_c*(y0-y1) )/128;


      //calculate the length for segment 1
      x = (x*stp_blergh)/256;
      y = (y*stp_blergh)/256;

      //translate the vector to its original x,y coordinates
      x += x1;
      y += y1;

      //now we must blend our value with the previous pass
      //the previous pass is drawn in reverse (x1,x2 in the first pass becomes x0,x1 in the second pass)
      //do not blend the first segment when drawing non-closed shapes
      //if (i > -1 || closedShape) {
      if (true) {
        
        int stp_blergh2 = stp;
        if (extraSmooth) {
          stp_blergh2 = (stp*last_w)/256;
        }

        //our angle offset for segment 2 from the previous pass (which is segment 1 of the current pass)
        int16_t angle2 = (last_a2b*stp_reverse)/65536; 

        //calculate the angle
        int16_t angle2_c = cos8(angle2)-128;
        int16_t angle2_s = sin8(angle2)-128;
        int32_t xl = ( angle2_c*(x1-x0) - angle2_s*(y1-y0) )/128;
        int32_t yl = ( angle2_s*(x1-x0) + angle2_c*(y1-y0) )/128;


        //calculate the length
        xl = (xl*stp_blergh2)/256;
        yl = (yl*stp_blergh2)/256;

        //translate our new vector back to original coordinates
        xl += x0;
        yl += y0;

        //debug points
        //blendXY(led_screen,xl,yl,0, stp);
        //blendXY(led_screen,x,y,160, stp);

        //blend our two passes (curves) together, fade from one to the other
        //int stp_blend = ease8InOutQuad(ease8InOutQuad(_min(stp,255)));
        //int stp_blend = ease8InOutQuad(_min(stp,255));
        int stp_blend = _min(stp,255);
        int stp_blend2 = 255-stp_blend;
        x = ((x*stp_blend) + (xl*stp_blend2))/256;
        y = ((y*stp_blend) + (yl*stp_blend2))/256;


        
      }
      
      //draw our curve (starting from the second iteration, we need two points, duh)
      if (stp > 0) {
        //draw line between points
        if (flipXY) {
          //draw_line_fine2(screen_object, last_y, last_x, y, x, hue, sat, val);
          draw_line_fine(screen_object, last_y, last_x, y, x, rgb, -10000, val, val, true);
            //blendXY(led_screen, y, x, 0, 0, 255);
        } else {

          //draw_line_fine2(screen_object, last_x, last_y, x, y, hue, sat, val);
          draw_line_fine(screen_object, last_x, last_y, x, y, rgb, -10000, val, val, true);
            //blendXY(led_screen, x, y, 0, 0, 255);
        }
      }
      //record x,y for the next iteration
      last_x = x;
      last_y = y;
      
      //debug points
      //blendXY(led_screen,x0,y0,96);
      //blendXY(led_screen,x1,y1,160);
      //blendXY(led_screen, x, y);
      //blendXY(led_screen,x,y);

      //on our final pass we need to calculate the last segment without blending
      //do not draw this segment for closed shapes
      if (i == ending_point-1 && !closedShape && false) {
        //blendXY(led_screen,x2,y2,48);
        
        //our current offset angle for segment 2
        int16_t angle2 = (a2b*stp)/65536;
        int16_t angle2_s = sin8(angle2);
        int16_t angle2_c = cos8(angle2);
        //calculate the current angle
        int32_t xb = ( angle2_c*(x2-x1)/128 - angle2_s*(y2-y1)/128 );
        int32_t yb = ( angle2_s*(x2-x1)/128 + angle2_c*(y2-y1)/128 );

        //calculate the current length
        xb = (xb*(256-stp))/256;
        yb = (yb*(256-stp))/256;

        //move the new vector back to its original coordinates
        xb += x1;
        yb += y1;
        
        //blendXY(led_screen,xb,yb);

        //draw our curve
        if (stp > 0) {
          //draw line between points
          if (flipXY) {
            draw_line_fine(screen_object, last_yb, last_xb, yb, xb, rgb, -10000, val, val, true);
            //blendXY(led_screen, yb, xb, 0, 0, 255);
          } else {
            draw_line_fine(screen_object, last_xb, last_yb, xb, yb, rgb, -10000, val, val, true);
            //blendXY(led_screen, xb, yb, 0, 0, 255);
          }
        }

        //record our points for the next step in time
        last_xb = xb;
        last_yb = yb;
      }

      //iterate time
      stp+=step_size;
    }
    
    //store our segment 2 angle to blend with the next pair of line segments
    last_a2b = a2b;
    last_w = w;
  }
} //matt_curve8_base


static inline __attribute__ ((always_inline)) void matt_curve8(int32_t coordinate_array[][2], const size_t& len, const uint8_t& hue = default_color, const uint8_t& sat = default_saturation, const uint8_t& val = 255, const bool& flipXY = false, const bool& closedShape = false, const bool& extraSmooth = false, const uint8_t& percentage = 255, const uint8_t& step_size = 32) {

  matt_curve8(led_screen, coordinate_array, len, hue, sat, val, flipXY, closedShape, extraSmooth, percentage, step_size);

}


#endif