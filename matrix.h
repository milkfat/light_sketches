//A class for performing 3D rotation

class MATRIX {

    //3D ROTATION MATRIX VARIABLES
    public:
    MATRIX() {
        update();
    }

    private:
    int z_scaler;

    long Cz; //camera Z
    long Sz; //projection screen Z (between camera and object)
    long Cz2;
    long Sz2;

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

    unsigned long update_time = millis();

    void update() {
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
        void rotate (long in[3], long out[3]) {
        
            //update the matrix if necessary
            update();
            out[0] = in[0] * matrix[0][0] + in[1] * matrix[0][1] + in[2] * matrix[0][2];
            out[2] = -(in[0] * matrix[1][0] + in[1] * matrix[1][1] + in[2] * matrix[1][2]);
            out[1] = in[0] * matrix[2][0] + in[1] * matrix[2][1] + in[2] * matrix[2][2];

        }

        void rotate (long in[3]) {
        
            long temp[3] = {
                in[0],
                in[1],
                in[2]
            };

            rotate(temp, in);

        }

        //take X,Y,Z coordinate
        //modifies X,Y to screen coordinates
        bool perspective(long p[3]) {
            if (p[2] < Cz) {
                p[0]/=2;//half precision to double each axis of our available coordinate space
                p[1]/=2;
                p[2]/=2;
                p[0] = ( p[0] * (Sz - Cz) ) / ( p[2] - Cz ) + (MATRIX_WIDTH * 256L/2) / 2;
                p[1] = ( p[1] * (Sz - Cz) ) / ( p[2] - Cz ) + (MATRIX_HEIGHT * 256L/2) / 2;
                p[0]*=2;
                p[1]*=2;
                p[2]*=2;
                return true;
            }
            return false;
        }

        //find the 3D coordinate of a pixel on the screen
        //takes screen X,Y coordinate along with the desired Z coordinate
        //modifies X,Y to provide X,Y,Z coordinate
        bool reverse_perspective(long p[3]) {
            p[0]/=2;//half precision to double each axis of our available coordinate space
            p[1]/=2;
            p[2]/=2;
            p[0] = ( ( p[0] - (MATRIX_WIDTH*256L/2)/2 ) * ( p[2] - Cz ) ) / ( Sz - Cz );
            p[1] = ( ( p[1] - (MATRIX_HEIGHT*256L/2)/2 ) * ( p[2] - Cz ) ) / ( Sz - Cz );
            p[0]*=2;
            p[1]*=2;
            p[2]*=2;
            return true;
        }

        void rotate_x(long p[3], int8_t s, int8_t c) {
            long temp = ( p[1]*c - p[2]*s ) / 128;
            p[2] = ( p[1]*s + p[2]*c ) / 128;
            p[1] = temp;
        }

        void rotate_x(long p[3], uint8_t ang) {
            int8_t s = sin8(ang)-128;
            int8_t c = cos8(ang)-128;
            rotate_x(p, s, c);
        }


        void rotate_y(long p[3], int8_t s, int8_t c) {
            long temp = ( p[0]*c - p[2]*s ) / 128;
            p[2] = ( p[0]*s + p[2]*c ) / 128;
            p[0] = temp;
        }

        void rotate_y(long p[3], uint8_t ang) {
            int8_t s = sin8(ang)-128;
            int8_t c = cos8(ang)-128;
            rotate_y(p, s, c);
        }

        void rotate_z(long p[3], int8_t s, int8_t c) {
            long temp = ( p[0]*c - p[1]*s ) / 128;
            p[1] = ( p[0]*s + p[1]*c ) / 128;
            p[0] = temp;
        }

        void rotate_z(long p[3], uint8_t ang) {
            int8_t s = sin8(ang)-128;
            int8_t c = cos8(ang)-128;
            rotate_z(p, s, c);
        }


        void scale_z(long p[3]) {
            scale_z(p[2]);
        }

        void scale_z(long& z) {
            z += z_scaler;
        }
        



};

MATRIX matrix;