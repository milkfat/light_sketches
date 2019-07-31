//A class for performing 3D rotation

class MATRIX {

    //3D ROTATION MATRIX VARIABLES
    public:
    MATRIX() {
        update();
    }

    private:
    static int z_scaler;

    static int32_t Cz; //camera Z
    static int32_t Sz; //projection screen Z (between camera and object)
    static int32_t Cz2;
    static int32_t Sz2;

    //find our angles in radians
    static float alpha; //Z
    static float beta; //X;
    static float gamma; //Y

    //store sin/cos in variables
    static float cZ;
    static float cX;
    static float cY;
    static float sZ;
    static float sX;
    static float sY;

    //our rotation matrix
    static float matrix[3][3];
    //END 3D ROTATION MATRIX VARIABLES

    static uint32_t update_time;

    static inline void update() {
        if (millis() - 16 > update_time) {
            update_time = millis();

            z_scaler = -100 * 256 + (200 * 256 * debug_scaler) / 256;

            Cz = camera_scaler * 256L; //camera Z
            Sz = screen_scaler * 256L; //projection screen Z (between camera and object)
            Cz2 = Cz/2;
            Sz2 = Sz/2;

            //construct ZXY rotation matrix

            //find our angles in radians
            alpha = (rotation_alpha * PI) / 180.f; //Z
            beta = (rotation_beta * PI) / 180.f; //X;
            gamma = (rotation_gamma * PI) / 180.f; //Y

            //store sin/cos in variables
            cZ = cos( alpha );
            cX = cos( beta );
            cY = cos( gamma );
            sZ = sin( alpha );
            sX = sin( beta );
            sY = sin( gamma );

            //create our rotation matrix
            matrix[0][0] = cZ * cY - sZ * sX * sY;
            matrix[0][1] = - cX * sZ;
            matrix[0][2] = cY * sZ * sX + cZ * sY;
            matrix[1][0] = cY * sZ + cZ * sX * sY;
            matrix[1][1] = cZ * cX;
            matrix[1][2] = sZ * sY - cZ * cY * sX;
            matrix[2][0] = - cX * sY;
            matrix[2][1] = sX;
            matrix[2][2] = cX * cY;
        }
    }

    public:

        //rotate X,Y,Z coordinate based on our current matrix
        // void rotate (int32_t in[3], int32_t out[3]) {
        
        //     //update the matrix if necessary
        //     update();
        //     out[0] = in[0] * matrix[0][0] + in[1] * matrix[0][1] + in[2] * matrix[0][2];
        //     out[2] = -(in[0] * matrix[1][0] + in[1] * matrix[1][1] + in[2] * matrix[1][2]);
        //     out[1] = in[0] * matrix[2][0] + in[1] * matrix[2][1] + in[2] * matrix[2][2];

        // }
        static inline __attribute__ ((always_inline))  void rotate (int32_t& in_x, int32_t& in_y, int32_t& in_z, int32_t& out_x, int32_t& out_y, int32_t& out_z) {
        
        //update the matrix if necessary
        update();
        out_x = in_x * matrix[0][0] + in_y * matrix[0][1] + in_z * matrix[0][2];
        out_z = -(in_x * matrix[1][0] + in_y * matrix[1][1] + in_z * matrix[1][2]);
        out_y = in_x * matrix[2][0] + in_y * matrix[2][1] + in_z * matrix[2][2];

        }

        static inline __attribute__ ((always_inline)) void rotate (VECTOR3& in) {
            VECTOR3 temp;
            temp.x = in.x;
            temp.y = in.y;
            temp.z = in.z;
            rotate(temp.x, temp.y, temp.z, in.x, in.y, in.z);
        }
        
        static inline __attribute__ ((always_inline)) void rotate (VECTOR3& in, VECTOR3& out) {
            rotate(in.x, in.y, in.z, out.x, out.y, out.z);
        }
            


        static inline __attribute__ ((always_inline)) void rotate (int32_t in[3], int32_t out[3]) {
            rotate(in[0],in[1],in[2],out[0],out[1],out[2]);
        }

        static inline __attribute__ ((always_inline)) void rotate (int32_t in[3]) {
        
            int32_t temp[3] = {
                in[0],
                in[1],
                in[2]
            };

            rotate(temp, in);

        }

        //take X,Y,Z coordinate
        //modifies X,Y to screen coordinates
        #define MATRIX_PRECISION 4
        static inline __attribute__ ((always_inline)) bool perspective(int32_t& x, int32_t& y, int32_t& z) {
            if (z < Cz) {
                x/=MATRIX_PRECISION;//half precision to double each axis of our available coordinate space
                y/=MATRIX_PRECISION;
                z/=MATRIX_PRECISION;
                int32_t zCz = (z-Cz/MATRIX_PRECISION);
                if (zCz == 0) {
                    zCz = 1;
                }
                x = ( x * ((Sz - Cz)/MATRIX_PRECISION) ) / zCz + ((MATRIX_WIDTH * 128)/MATRIX_PRECISION);
                y = ( y * ((Sz - Cz)/MATRIX_PRECISION) ) / zCz + ((MATRIX_HEIGHT * 128)/MATRIX_PRECISION);
                x*=MATRIX_PRECISION;
                y*=MATRIX_PRECISION;
                z*=MATRIX_PRECISION;
                return true;
            }
            return false;
        }

        static inline __attribute__ ((always_inline)) bool perspective(int32_t p[3]) {
            return perspective(p[0], p[1], p[2]);
        }

        static inline __attribute__ ((always_inline)) bool perspective(VECTOR3& p) {
            return perspective(p.x, p.y, p.z);
        }

        static inline __attribute__ ((always_inline)) bool perspective(VECTOR3& p, VECTOR3& p2) {
            p2 = p;
            return perspective(p2.x, p2.y, p2.z);
        }

        //find the 3D coordinate of a pixel on the screen
        //takes screen X,Y coordinate along with the desired Z coordinate
        //modifies X,Y to provide X,Y,Z coordinate
        static inline __attribute__ ((always_inline)) bool reverse_perspective(int32_t p[3]) {
            p[0]/=2;//half precision to double each axis of our available coordinate space
            p[1]/=2;
            p[2]/=2;
            p[0] = ( ( p[0] - (MATRIX_WIDTH*128)/2 ) * (( p[2] - Cz )/2) ) / (( Sz - Cz )/2);
            p[1] = ( ( p[1] - (MATRIX_HEIGHT*128)/2 ) * (( p[2] - Cz )/2) ) / (( Sz - Cz )/2);
            p[0]*=2;
            p[1]*=2;
            p[2]*=2;
            return true;
        }

        static inline __attribute__ ((always_inline)) void rotate_x(int32_t& x, int32_t& y, int32_t& z, int8_t& s, int8_t& c) {
            int32_t temp = ( y*c - z*s ) / 128;
            z = ( y*s + z*c ) / 128;
            y = temp;
        }

        static inline __attribute__ ((always_inline)) void rotate_x(int32_t p[3], const uint8_t& ang) {
            int8_t s = sin8(ang)-128;
            int8_t c = cos8(ang)-128;
            rotate_x(p[0], p[1], p[2], s, c);
        }

        static inline __attribute__ ((always_inline)) void rotate_x(VECTOR3& p, const uint8_t& ang) {
            int8_t s = sin8(ang)-128;
            int8_t c = cos8(ang)-128;
            rotate_x(p.x, p.y, p.z, s, c);
        }


        static inline __attribute__ ((always_inline)) void rotate_y(int32_t& x, int32_t& y, int32_t& z, int8_t& s, int8_t& c) {
            int32_t temp = ( x*c - z*s ) / 128;
            z = ( x*s + z*c ) / 128;
            x = temp;
        }

        static inline __attribute__ ((always_inline)) void rotate_y(int32_t p[3], const uint8_t& ang) {
            int8_t s = sin8(ang)-128;
            int8_t c = cos8(ang)-128;
            rotate_y(p[0], p[1], p[2], s, c);
        }

        static inline __attribute__ ((always_inline)) void rotate_y(VECTOR3& p, const uint8_t& ang) {
            int8_t s = sin8(ang)-128;
            int8_t c = cos8(ang)-128;
            rotate_y(p.x, p.y, p.z, s, c);
        }

        static inline __attribute__ ((always_inline)) void rotate_z(int32_t& x, int32_t& y, int32_t& z, int8_t& s, int8_t& c) {
            int32_t temp = ( x*c - y*s ) / 128;
            y = ( x*s + y*c ) / 128;
            x = temp;
        }

        static inline __attribute__ ((always_inline)) void rotate_z(int32_t p[3], const uint8_t& ang) {
            int8_t s = sin8(ang)-128;
            int8_t c = cos8(ang)-128;
            rotate_z(p[0], p[1], p[2], s, c);
        }

        static inline __attribute__ ((always_inline)) void rotate_z(VECTOR3& p, const uint8_t& ang) {
            int8_t s = sin8(ang)-128;
            int8_t c = cos8(ang)-128;
            rotate_z(p.x, p.y, p.z, s, c);
        }


        static inline __attribute__ ((always_inline)) void scale_z(int32_t& z) {
            z += z_scaler;
        }
        
        static inline __attribute__ ((always_inline)) void scale_z(int32_t p[3]) {
            scale_z(p[2]);
        }

        static inline __attribute__ ((always_inline)) void scale_z(VECTOR3& p) {
            scale_z(p.z);
        }
        



};
int MATRIX::z_scaler;

int32_t MATRIX::Cz; //camera Z
int32_t MATRIX::Sz; //projection screen Z (between camera and object)
int32_t MATRIX::Cz2;
int32_t MATRIX::Sz2;

    //find our angles in radians
float MATRIX::alpha; //Z
float MATRIX::beta; //X;
float MATRIX::gamma; //Y

    //store sin/cos in variables
float MATRIX::cZ;
float MATRIX::cX;
float MATRIX::cY;
float MATRIX::sZ;
float MATRIX::sX;
float MATRIX::sY;

    //our rotation matrix
float MATRIX::matrix[3][3];
uint32_t MATRIX::update_time = 0;

MATRIX matrix;


















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

    matrix.rotate_x(norm,32);
    matrix.rotate_y(norm,32);

    
    uint8_t bri = _min(_max((norm.z*7)/8,0)+32,255);

    CRGB rgb = rgb_in;
    color_scale(rgb, bri);
    reset_y_buffer();
    reset_x_buffer();
    draw_line_fine(leds, a, b, rgb, z_depth-16, 255, 255, true, false);
    draw_line_fine(leds, b, c, rgb, z_depth-16, 255, 255, true, false);
    draw_line_fine(leds, c, d, rgb, z_depth-16, 255, 255, true, false);
    draw_line_fine(leds, d, a, rgb, z_depth-16, 255, 255, true, false);
    

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
          drawXYZ(leds, x, y, z_depth, rgb);
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




int16_t get_current_cube() {
    for (int i = 0; i < NUMBER_OF_CUBES; i++) {
        if (cubes[i].next == -1 && i != last_cube) {
            return i;
        }
    }
    return -1;
}



//find cube's z depth and sort it into our buffer
static void draw_cube(const VECTOR3& p, const VECTOR3& d = VECTOR3(256,256,256), const VECTOR3_8& r = VECTOR3_8(0,0,0), const CHSV& hsv = CHSV(0,0,255), const bool& persist=false) {
  
  int16_t current_cube = get_current_cube();

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
        first_cube = current_cube;
        last_cube = current_cube;
        c->prev = -1;
        c->next = -1;
    } else {
        cubes[last_cube].next = current_cube;
        c->prev = last_cube;
        c->next = -1;
        last_cube = current_cube;
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
    matrix.rotate_y(normals[i],c->r.y);
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
    matrix.rotate_y(points[i],c->r.y);
    points[i]+=c->p;
    matrix.rotate(points[i]);
    
    //translate vectors to coordinates
    matrix.scale_z(points[i]);

    //correct 3d perspective
    matrix.perspective(points[i]);

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


void sort_cubes() {
    int16_t cube = first_cube;
    while (cube != -1) {
        int16_t next_cube = cubes[cube].next;

        int16_t current_position = cubes[cube].next;

        while (current_position != -1 && cubes[cube].z > cubes[current_position].z) {
            current_position = cubes[current_position].next;
        }

        if (current_position != cubes[cube].next) {

            //update old neighbors
            if (cube == first_cube) {
                first_cube = cubes[cube].next;
            } else {
                cubes[cubes[cube].prev].next = cubes[cube].next;
            }
            cubes[cubes[cube].next].prev = cubes[cube].prev;

            
            if (current_position == -1) {
                //move to end
                cubes[last_cube].next = cube;
                cubes[cube].next = -1;
                cubes[cube].prev = last_cube;
                last_cube = cube;

            } else {

                //update new neighbors
                cubes[cube].next = current_position;
                cubes[cube].prev = cubes[current_position].prev;
                cubes[cubes[current_position].prev].next = cube;
                cubes[current_position].prev = cube;

            } 
        }
        cube = next_cube;
    }
} //sort_cubes()

static void draw_cubes() {
    
    sort_cubes();

    int16_t cube = first_cube;
    int16_t new_first_cube = -1;
    int16_t new_last_cube = -1;
    while (cube != -1) {
        int16_t next_cube = cubes[cube].next;
        draw_cached_cube(cube);
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
} //draw_cubes()









