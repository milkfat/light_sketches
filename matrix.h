#ifndef LIGHTS_MATRIX_H
#define LIGHTS_MATRIX_H

#include "math_helpers.h"

//A class for performing 3D rotation

class MATRIX {

    float * rotation_alpha;
    float * rotation_beta;
    float * rotation_gamma;
    VECTOR3 * camera_position;

    //3D ROTATION MATRIX VARIABLES
    public:
    
    MATRIX(float * a, float * b, float * g, VECTOR3 * c) {
        rotation_alpha = a;
        rotation_beta = b;
        rotation_gamma = g;
        camera_position = c;
        update();
    }

    private:

    //find our angles in radians
    float alpha; //Z
    float beta; //X;
    float gamma; //Y

    //store sin/cos in variables
    float cZ;
    float cX;
    float cY;
    float sZ;
    float sX;
    float sY;

    //our rotation matrix
    float matrix[3][3];
    //END 3D ROTATION MATRIX VARIABLES

    uint32_t update_time = 0;

    void update() {
        if (millis() - 16 > update_time) {
            update_time = millis();

            //construct ZXY rotation matrix

            //find our angles in radians
            alpha = ((*rotation_alpha) * PI) / 180.f; //Z
            beta = ((*rotation_beta) * PI) / 180.f; //X;
            gamma = ((*rotation_gamma) * PI) / 180.f; //Y

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

        void rotate (int32_t& in_x, int32_t& in_y, int32_t& in_z, int32_t& out_x, int32_t& out_y, int32_t& out_z) {
        
            //update the matrix if necessary
            update();
            out_x = in_x * matrix[0][0] + in_y * matrix[0][1] + in_z * matrix[0][2];
            out_z = -(in_x * matrix[1][0] + in_y * matrix[1][1] + in_z * matrix[1][2]);
            out_y = in_x * matrix[2][0] + in_y * matrix[2][1] + in_z * matrix[2][2];

        }

        void rotate_camera (int32_t in_x, int32_t in_y, int32_t in_z, int32_t& out_x, int32_t& out_y, int32_t& out_z) {
        
            //update the matrix if necessary
            update();
            in_x -= camera_position->x;
            in_y -= camera_position->y;
            in_z -= camera_position->z;
            out_x = in_x * matrix[0][0] + in_y * matrix[0][1] + in_z * matrix[0][2];
            out_z = -(in_x * matrix[1][0] + in_y * matrix[1][1] + in_z * matrix[1][2]);
            out_y = in_x * matrix[2][0] + in_y * matrix[2][1] + in_z * matrix[2][2];
            out_x += camera_position->x;
            out_y += camera_position->y;
            out_z += camera_position->z;

        }

        inline void rotate (VECTOR3& in) {
            VECTOR3 temp;
            temp.x = in.x;
            temp.y = in.y;
            temp.z = in.z;
            rotate(temp.x, temp.y, temp.z, in.x, in.y, in.z);
        }

        inline void rotate_camera (VECTOR3& in) {
            rotate_camera(in.x, in.y, in.z, in.x, in.y, in.z);
        }
        
        inline void rotate (VECTOR3& in, VECTOR3& out) {
            rotate(in.x, in.y, in.z, out.x, out.y, out.z);
        }

        inline void rotate_camera(VECTOR3& in, VECTOR3& out) {
            rotate_camera(in.x, in.y, in.z, out.x, out.y, out.z);
        }
            


        inline void rotate (int32_t in[3], int32_t out[3]) {
            rotate(in[0],in[1],in[2],out[0],out[1],out[2]);
        }

        inline void rotate (int32_t in[3]) {
        
            int32_t temp[3] = {
                in[0],
                in[1],
                in[2]
            };

            rotate(temp, in);

        }

        inline void rotate (int32_t& x, int32_t& y, int32_t& z) {
        
            int32_t temp_x = x;
            int32_t temp_y = y;
            int32_t temp_z = z;

            rotate(temp_x, temp_y, temp_z, x, y, z);

        }

};


#endif