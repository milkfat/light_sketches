#ifndef LIGHTS_MATRIX_H
#define LIGHTS_MATRIX_H

//A class for performing 3D rotation

class MATRIX {

    //3D ROTATION MATRIX VARIABLES
    public:
    MATRIX() {
        update();
    }

    private:
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

};
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


#endif