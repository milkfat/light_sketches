//A class for performing 3D rotation

class MATRIX {

    //3D ROTATION MATRIX VARIABLES
    public:
    MATRIX() {
        update();
    }

    private:
    int z_scaler;

    int32_t Cz; //camera Z
    int32_t Sz; //projection screen Z (between camera and object)
    int32_t Cz2;
    int32_t Sz2;

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

    uint32_t update_time = millis();

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
        // void rotate (int32_t in[3], int32_t out[3]) {
        
        //     //update the matrix if necessary
        //     update();
        //     out[0] = in[0] * matrix[0][0] + in[1] * matrix[0][1] + in[2] * matrix[0][2];
        //     out[2] = -(in[0] * matrix[1][0] + in[1] * matrix[1][1] + in[2] * matrix[1][2]);
        //     out[1] = in[0] * matrix[2][0] + in[1] * matrix[2][1] + in[2] * matrix[2][2];

        // }
        void rotate (int32_t& in_x, int32_t& in_y, int32_t& in_z, int32_t& out_x, int32_t& out_y, int32_t& out_z) {
        
        //update the matrix if necessary
        update();
        out_x = in_x * matrix[0][0] + in_y * matrix[0][1] + in_z * matrix[0][2];
        out_z = -(in_x * matrix[1][0] + in_y * matrix[1][1] + in_z * matrix[1][2]);
        out_y = in_x * matrix[2][0] + in_y * matrix[2][1] + in_z * matrix[2][2];

        }

        void rotate (VECTOR3& in) {
            VECTOR3 temp;
            temp.x = in.x;
            temp.y = in.y;
            temp.z = in.z;
            rotate(temp.x, temp.y, temp.z, in.x, in.y, in.z);
        }
        
        void rotate (VECTOR3& in, VECTOR3& out) {
            rotate(in.x, in.y, in.z, out.x, out.y, out.z);
        }
            


        void rotate (int32_t in[3], int32_t out[3]) {
            rotate(in[0],in[1],in[2],out[0],out[1],out[2]);
        }

        void rotate (int32_t in[3]) {
        
            int32_t temp[3] = {
                in[0],
                in[1],
                in[2]
            };

            rotate(temp, in);

        }

        //take X,Y,Z coordinate
        //modifies X,Y to screen coordinates
        bool perspective(int32_t& x, int32_t& y, int32_t& z) {
            if (z < Cz) {
                x/=2;//half precision to double each axis of our available coordinate space
                y/=2;
                z/=2;
                x = ( x * (Sz - Cz) ) / ( z - Cz ) + (MATRIX_WIDTH * 256L/2) / 2;
                y = ( y * (Sz - Cz) ) / ( z - Cz ) + (MATRIX_HEIGHT * 256L/2) / 2;
                x*=2;
                y*=2;
                z*=2;
                return true;
            }
            return false;
        }

        bool perspective(int32_t p[3]) {
            return perspective(p[0], p[1], p[2]);
        }

        bool perspective(VECTOR3& p) {
            return perspective(p.x, p.y, p.z);
        }

        bool perspective(VECTOR3& p, VECTOR3& p2) {
            p2 = p;
            return perspective(p2.x, p2.y, p2.z);
        }

        //find the 3D coordinate of a pixel on the screen
        //takes screen X,Y coordinate along with the desired Z coordinate
        //modifies X,Y to provide X,Y,Z coordinate
        bool reverse_perspective(int32_t p[3]) {
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

        void rotate_x(int32_t& x, int32_t& y, int32_t& z, int8_t& s, int8_t& c) {
            int32_t temp = ( y*c - z*s ) / 128;
            z = ( y*s + z*c ) / 128;
            y = temp;
        }

        void rotate_x(int32_t p[3], const uint8_t& ang) {
            int8_t s = sin8(ang)-128;
            int8_t c = cos8(ang)-128;
            rotate_x(p[0], p[1], p[2], s, c);
        }

        void rotate_x(VECTOR3& p, const uint8_t& ang) {
            int8_t s = sin8(ang)-128;
            int8_t c = cos8(ang)-128;
            rotate_x(p.x, p.y, p.z, s, c);
        }


        void rotate_y(int32_t& x, int32_t& y, int32_t& z, int8_t& s, int8_t& c) {
            int32_t temp = ( x*c - z*s ) / 128;
            z = ( x*s + z*c ) / 128;
            x = temp;
        }

        void rotate_y(int32_t p[3], const uint8_t& ang) {
            int8_t s = sin8(ang)-128;
            int8_t c = cos8(ang)-128;
            rotate_y(p[0], p[1], p[2], s, c);
        }

        void rotate_y(VECTOR3& p, const uint8_t& ang) {
            int8_t s = sin8(ang)-128;
            int8_t c = cos8(ang)-128;
            rotate_y(p.x, p.y, p.z, s, c);
        }

        void rotate_z(int32_t& x, int32_t& y, int32_t& z, int8_t& s, int8_t& c) {
            int32_t temp = ( x*c - y*s ) / 128;
            y = ( x*s + y*c ) / 128;
            x = temp;
        }

        void rotate_z(int32_t p[3], const uint8_t& ang) {
            int8_t s = sin8(ang)-128;
            int8_t c = cos8(ang)-128;
            rotate_z(p[0], p[1], p[2], s, c);
        }

        void rotate_z(VECTOR3& p, const uint8_t& ang) {
            int8_t s = sin8(ang)-128;
            int8_t c = cos8(ang)-128;
            rotate_z(p.x, p.y, p.z, s, c);
        }


        void scale_z(int32_t& z) {
            z += z_scaler;
        }
        
        void scale_z(int32_t p[3]) {
            scale_z(p[2]);
        }

        void scale_z(VECTOR3& p) {
            scale_z(p.z);
        }
        



};

MATRIX matrix;