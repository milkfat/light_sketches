#ifndef LIGHTS_CUBE_H
#define LIGHTS_CUBE_H

#include "rotate.h"
#include "scale.h"

static void draw_quad(VECTOR3& a, VECTOR3& b, VECTOR3& c, VECTOR3& d, const VECTOR3& orig, const VECTOR3& norm_in, const CRGB& rgb_in = CRGB(255,0,0) ) {
  
  //optimization:
  //identify clockwise/counterclockwise orientation
  //draw in only one orientation (facing toward the camera)
  int orientation = (b.y-a.y)*(c.x-b.x) - (c.y-b.y)*(b.x-a.x);
  
  if ( orientation < 0 ) {

    VECTOR3 norm = norm_in;

    // draw_line_ybuffer(a, b);
    // draw_line_ybuffer(b, c);
    // draw_line_ybuffer(c, d);
    // draw_line_ybuffer(d, a);

    int32_t z_depth = orig.z+norm.z; 

    rotate_x(norm,32);
    rotate_y(norm,32);

    //shading according to surface normal
    uint8_t bri = _min(_max((norm.z*7)/8,0)+32,255);

    CRGB rgb = rgb_in;
    color_scale(rgb, bri);
    reset_y_buffer();
    reset_x_buffer();
    draw_line_fine(led_screen, a, b, rgb, z_depth-16, 255, 255, true, false, true);
    draw_line_fine(led_screen, b, c, rgb, z_depth-16, 255, 255, true, false, true);
    draw_line_fine(led_screen, c, d, rgb, z_depth-16, 255, 255, true, false, true);
    draw_line_fine(led_screen, d, a, rgb, z_depth-16, 255, 255, true, false, true);
    

    for (int x = 0; x < MATRIX_WIDTH; x++) {
        
        if (x_buffer[x][0]-1 >= 0 && x_buffer[x][0]-1 < MATRIX_HEIGHT) {
            z_buffer[x][x_buffer[x][0]-1] += 1;
        }

        if (x_buffer[x][1]+1 >= 0 && x_buffer[x][1]+1 < MATRIX_HEIGHT) {
            z_buffer[x][x_buffer[x][1]+1] += 1;
        }

    }
    //fill between the pixels of our lines
    for (int y = _max(y_buffer_min, 0); y <= _min(y_buffer_max,MATRIX_HEIGHT-1); y++) {
        if (y_buffer[y][0] <= y_buffer[y][1]) {

        for (int x = y_buffer[y][0]; x <= y_buffer[y][1]; x++) {
          drawXYZ(led_screen, x, y, z_depth, rgb);
        }

      }
      //reset the y_buffer
      //y_buffer[y][0] = MATRIX_WIDTH + 1;
      //y_buffer[y][1] = -1;
    
    }

    //y_buffer_max = 0;
    //y_buffer_min = MATRIX_HEIGHT-1;

  }
} //draw_quad()

struct CUBE {
    VECTOR3 p; //position X,Y,Z
    VECTOR3 d; //dimensions X,Y,Z
    VECTOR3_8 r; //rotation X,Y,Z
    int32_t z;
    CRGB rgb;
    int16_t prev = -1;
    int16_t next = -1;
    bool persist = false;
};

#define NUMBER_OF_CUBES 300

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
static void draw_cube(const VECTOR3& p, const VECTOR3& d = VECTOR3(256,256,256), const VECTOR3_8& r = VECTOR3_8(0,0,0), const CHSV& hsv = CHSV(0,0,255), const bool& persist=false) {
  
  int16_t current_cube = get_current_cube();
  int16_t most_recent_cube = recent_cube;
  recent_cube = current_cube;

  if (current_cube != -1) {
    CUBE* c = &cubes[current_cube];
    
    VECTOR3 newp = p;
    matrix.rotate(newp);

    c->persist = persist;
    c->z = newp.z;
    c->p = p;
    c->d = d;
    c->r = r;
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

static void draw_cached_cube(const int16_t& cp, int16_t bri) {

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
    rotate_y(normals[i],c->r.y);
    matrix.rotate(normals[i]);
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
    rotate_y(points[i],c->r.y);
    points[i]+=c->p;
    matrix.rotate(points[i]);
    
    //translate vectors to coordinates
    scale_z(points[i]);

    //correct 3d perspective
    led_screen.perspective(points[i]);

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
  matrix.rotate(c->p, p);
  
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
    int16_t cnt = 0;
    while (cube != -1) {
        int16_t next_cube = cubes[cube].next;
        draw_cached_cube(cube,cnt);
        cnt++;
        if (cubes[cube].persist) {
            cubes[cube].d.x = (cubes[cube].d.x*(50+(fmix32(cube)%32))) / 100;
            cubes[cube].d.y = (cubes[cube].d.y*(50+(fmix32(cube)%32))) / 100;
            cubes[cube].d.z = (cubes[cube].d.z*(50+(fmix32(cube)%32))) / 100;
            cubes[cube].p.z -= 5000;
            if (cubes[cube].d.x == 0 || cubes[cube].d.y == 0 || cubes[cube].d.z == 0 || cubes[cube].p.z < -200000) {
                cubes[cube].persist = false;
            }
            if (new_first_cube == -1) {
                new_first_cube = cube;
                new_last_cube = cube;
                cubes[cube].prev = -1;
                cubes[cube].next = -1;
            } else {
                cubes[new_last_cube].next = cube;
                cubes[cube].prev = new_last_cube;
                cubes[cube].next = -1;
                new_last_cube = cube;
            }
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