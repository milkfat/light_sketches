#ifndef LIGHTS_CUBE_H
#define LIGHTS_CUBE_H

#define NUMBER_OF_CUBES 200

#include "rotate.h"
#include "scale.h"

static void draw_quad(VECTOR3 a, VECTOR3 b, VECTOR3 c, VECTOR3 d, VECTOR3 orig, VECTOR3 norm_in, CRGB rgb_in = CRGB(255,0,0) ) {
  
  //optimization:
  //identify clockwise/counterclockwise orientation
  //draw in only one orientation (facing toward the camera)


  bool orientation = ((float)(b.y-a.y))*(c.x-b.x) - ((float)(c.y-b.y))*(b.x-a.x) < 0;
  //bool orientation = (((float)b.y-a.y)*((float)c.x-b.x) - ((float)c.y-b.y)*((float)b.x-a.x)) < 0;
  //bool orientation = ((b.y/16-a.y/16)*(c.x/16-b.x/16) - (c.y/16-b.y/16)*(b.x/16-a.x/16)) < 0;
  //bool orientation = (((b.y-a.y)/16)*((c.x-b.x)/16) - ((c.y-b.y)/16)*((b.x-a.x)/16)) < 0;

  if ( orientation ) {

    VECTOR3 norm = norm_in;

    // draw_line_ybuffer(a, b);
    // draw_line_ybuffer(b, c);
    // draw_line_ybuffer(c, d);
    // draw_line_ybuffer(d, a);

    int32_t z_depth = orig.z+norm.z; 

    rotate_x(norm,24);
    rotate_y(norm,16);

    //shading according to surface normal
    uint8_t bri = _min(_max((norm.z*7)/8,0)+32,255);

    CRGB rgb = rgb_in;
    color_scale(rgb, bri);
    y_buffer->reset();
    reset_x_buffer();
    a.z = z_depth-16;
    b.z = z_depth-16;
    c.z = z_depth-16;
    d.z = z_depth-16;
    draw_line_fine_new(led_screen, a, b, rgb);
    draw_line_fine_new(led_screen, b, c, rgb);
    draw_line_fine_new(led_screen, c, d, rgb);
    draw_line_fine_new(led_screen, d, a, rgb);
    
    //what's this for?
    // for (int x = 0; x < MATRIX_WIDTH; x++) {
        
    //     if (z_buffer != nullptr) {
    //       if (x_buffer[x][0]-1 >= 0 && x_buffer[x][0]-1 < MATRIX_HEIGHT) {
    //           (*z_buffer)[x][x_buffer[x][0]-1] += 1;
    //       }

    //       if (x_buffer[x][1]+1 >= 0 && x_buffer[x][1]+1 < MATRIX_HEIGHT) {
    //           (*z_buffer)[x][x_buffer[x][1]+1] += 1;
    //       }
    //     }

    // }
    
    //fill between the pixels of our lines
    
    fill_shape(z_depth+256, rgb);
    

    //y_buffer_max = 0;
    //y_buffer_min = MATRIX_HEIGHT-1;

  }
} //draw_quad()

struct CUBE {
    VECTOR3 p = VECTOR3(0,0,0); //position X,Y,Z
    VECTOR3_16 d = VECTOR3(0,0,0); //dimensions X,Y,Z
    VECTOR3_16 r = VECTOR3(0,0,0); //rotation X,Y,Z
    bool r_fine = false;
    int32_t z = 0;
    CRGB rgb = CRGB(0,0,0);
    int16_t prev = -1;
    int16_t next = -1;
    bool persist = false;
};

CUBE* cubes;

int16_t first_cube = -1;
int16_t last_cube = -1;
int16_t recent_cube = -1;




int16_t get_current_cube() {
    for (int i = 0; i < NUMBER_OF_CUBES; i++) {
        if (cubes[i].next == -1 && i != last_cube) {
            return i;
        }
    }
    return -1;
}



//find cube's z depth and sort it into our buffer (ascending Z order, back-to-front)
void draw_cube(const VECTOR3& p, const VECTOR3& d = VECTOR3(256,256,256), const VECTOR3& r = VECTOR3(0,0,0), const CHSV& hsv = CHSV(0,0,255), const bool& persist=false, const bool& r_fine = false) {
  
  int16_t current_cube = get_current_cube();
  int16_t most_recent_cube = recent_cube;
  recent_cube = current_cube;

  if (current_cube != -1) {
    CUBE* c = &cubes[current_cube];
    
    VECTOR3 newp = p;
    led_screen.matrix.rotate(newp);

    int x = (newp.x-led_screen.camera_position.x)/256;
    int y = (newp.y-led_screen.camera_position.y)/256;
    int z = (newp.z-led_screen.camera_position.z)/256;

    c->persist = persist;
    c->z = -sqrt(x*x+y*y+z*z); //use distance-from-camera to sort our cubes
    c->p = p;
    c->d = d;
    c->r = r;
    c->r_fine = r_fine;
    hsv2rgb_rainbow(hsv, c->rgb);

    if (first_cube == -1) {

        //buffer is empty, this is the first cube
        first_cube = current_cube;
        last_cube = current_cube;
        c->prev = -1;
        c->next = -1;

    } else {
        
        //place the current cube next to the most recent cube in the list
        //sorting should be faster when cubes are added in local groups (similar Z coordinate)
        c->prev = most_recent_cube;
        c->next = cubes[most_recent_cube].next;

        //slide to the right if necessary
        while (c->next != -1 && c->z > cubes[c->next].z) {
            c->prev = c->next;
            c->next = cubes[c->next].next;
        }

        //slide to the left if necessary
        while (c->prev != -1 && c->z < cubes[c->prev].z) {
            c->next = c->prev;
            c->prev = cubes[c->prev].prev;
        }

        //cube becomes the last cube in the list
        if (c->next == -1) {
            last_cube = current_cube;
            cubes[c->prev].next = current_cube;
            return;
        }

        //cube becomes the first cube in the list
        if (c->prev == -1) {
            first_cube = current_cube;
            cubes[c->next].prev = current_cube;
            return;
        }

        //cube is inserted into the middle of the list
        cubes[c->prev].next = current_cube;
        cubes[c->next].prev = current_cube;


    }    

  }

} //draw_cube()

static void draw_cached_cube(const int16_t& cp) {

  CUBE* c = &cubes[cp];

  VECTOR3 normals[] = {
    VECTOR3(255,0,0),  //right
    VECTOR3(-255,0,0), //left
    VECTOR3(0,255,0),  //top
    VECTOR3(0,-255,0), //bottom
    VECTOR3(0,0,255),  //front
    VECTOR3(0,0,-255)  //back
  };

  for (int i = 0; i < 6; i++) {
    (c->r_fine) ? rotate16(normals[i],c->r) : rotate(normals[i],c->r);
    led_screen.matrix.rotate(normals[i]);
  }

  VECTOR3 points[] = {
    
    VECTOR3(c->d.x,c->d.y,c->d.z), //top right front
    VECTOR3(c->d.x,c->d.y,-c->d.z), //top right back
    VECTOR3(-c->d.x,c->d.y,-c->d.z), //top left  back
    VECTOR3(-c->d.x,c->d.y,c->d.z), //top left  front
    
    VECTOR3(c->d.x,-c->d.y,c->d.z), //bottom right front
    VECTOR3(c->d.x,-c->d.y,-c->d.z), //bottom right back
    VECTOR3(-c->d.x,-c->d.y,-c->d.z), //bottom left  back
    VECTOR3(-c->d.x,-c->d.y,c->d.z)  //bottom left  front

  };

  for (int i = 0; i < 8; i++) {
    (c->r_fine) ? rotate16(points[i],c->r) : rotate(points[i],c->r);
    points[i]+=c->p;
    led_screen.matrix.rotate(points[i]);

    //correct 3d perspective
    led_screen.perspective(points[i]);

    //don't draw cube if any part is behind the camera
    //if (points[i].z > led_screen.camera_position.z) return;

  }

  int16_t cube_face_order[6][2]={{1000,0},{1000,0},{1000,0},{1000,0},{1000,0},{1000,0}};

  for (int i = 0; i < 6; i++) {
    int pos = -1000;
    int last0;
    int last1;
    for (int j = 0; j < 6; j++) {
      if (pos == -1000) {
        if (normals[i].z < cube_face_order[j][0]) {
          pos = j;
          last0 = cube_face_order[j][0];
          last1 = cube_face_order[j][1];
          cube_face_order[pos][0] = normals[i].z;
          cube_face_order[pos][1] = i;
        }
      } else {
        int temp0 = cube_face_order[j][0];
        int temp1 = cube_face_order[j][1];
        cube_face_order[j][0] = last0;
        cube_face_order[j][1] = last1;
        last0 = temp0;
        last1 = temp1;
      }
    }
  }

  VECTOR3 p;
  led_screen.matrix.rotate(c->p, p);
  
  //draw faces from back to front
  for (int i = 0; i < 6; i++) {
    uint8_t next_side = cube_face_order[i][1];
    switch (next_side) {
        case 0:
            draw_quad(points[0],points[4],points[5],points[1],p,normals[0],c->rgb);  //right
            break;
        case 1:
            draw_quad(points[2],points[6],points[7],points[3],p,normals[1],c->rgb); //left
            break;
        case 2:
            draw_quad(points[0],points[1],points[2],points[3],p,normals[2],c->rgb);  //top
            break;
        case 3:
            draw_quad(points[7],points[6],points[5],points[4],p,normals[3],c->rgb); //bottom
            break;
        case 4:
            draw_quad(points[0],points[3],points[7],points[4],p,normals[4],c->rgb);  //front
            break;
        case 5:
            draw_quad(points[1],points[5],points[6],points[2],p,normals[5],c->rgb); //back
            break;
        default:
            break;
    }

  }
} //draw_cached_cube()



static void draw_cubes() {
    
    int16_t cube = first_cube;
    int16_t new_first_cube = -1;
    int16_t new_last_cube = -1;
    while (cube != -1) {
        int16_t next_cube = cubes[cube].next;
        draw_cached_cube(cube);
        //make cubes fall away
        if (false && cubes[cube].persist) {
        } else {
            cubes[cube].prev = -1;
            cubes[cube].next = -1;
        }
        cube = next_cube;
    }
    first_cube = new_first_cube;
    last_cube = new_last_cube;
    recent_cube = first_cube;
} //draw_cubes()


#endif