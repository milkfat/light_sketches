#ifndef LIGHTS_CONTROL_VARIABLES_H
#define LIGHTS_CONTROL_VARIABLES_H
#include "helper_functions.h"
#define MAX_NUMBER_OF_CONTROL_VARIABLES 20
class CONTROL_VARIABLES {

    int number_of_variables = 0;

    struct CONTROL_VARIABLE {

        union {
            uint8_t* ui8_t;   //1
            uint16_t* ui16_t; //2
            uint32_t* ui32_t; //3
            int8_t* i8_t;     //4
            int16_t* i16_t;   //5
            int32_t* i32_t;   //6
            CRGB* rgb;        //7
            bool* boo;        //8 = switch; 9 = momentary
            char* string;     //10
        };

        int32_t range_min=0;
        int32_t range_max=0;
        const char * name;
        char key = '\0';

        uint8_t type = 0;
    };

    CONTROL_VARIABLE cv[MAX_NUMBER_OF_CONTROL_VARIABLES];

  public:

    int avail() {
        return number_of_variables;
    }

    int type(int& pos) {
        if (!(pos >= 0 && pos < number_of_variables)) {
            return -1;
        }
        return cv[pos].type;
    }

    int read(int& pos, int& var, int& range_min, int& range_max, const char * & chr) {
        if (!(pos >= 0 && pos < number_of_variables)) {
            return 0;
        }

        switch (cv[pos].type) {
            case 1:
                var = *cv[pos].ui8_t;
                break;
            case 2:
                var = *cv[pos].ui16_t;
                break;
            case 3:
                var = *cv[pos].ui32_t;
                break;
            case 4:
                var = *cv[pos].i8_t;
                break;
            case 5:
                var = *cv[pos].i16_t;
                break;
            case 6:
                var = *cv[pos].i32_t;
                break;
            default:
                return 0;
        }
        range_min = cv[pos].range_min;
        range_max = cv[pos].range_max;
        chr = cv[pos].name;
        return 1;
    }

    int read(int& pos, CRGB& var, const char * & chr) {
        if (!(pos >= 0 && pos < number_of_variables)) {
            return 0;
        }

        switch (cv[pos].type) {
            case 7:
                var = *cv[pos].rgb;
                break;
            default:
                return 0;
        }
        chr = cv[pos].name;
        return 1;
    }

    int read(int& pos, bool& var, const char * & chr) {
        if (!(pos >= 0 && pos < number_of_variables)) {
            return 0;
        }

        switch (cv[pos].type) {
            case 8:
                var = *cv[pos].boo;
                break;
            case 9:
                var = *cv[pos].boo;
                break;
            default:
                return 0;
        }
        chr = cv[pos].name;
        return 1;
    }

    int read(int& pos, char *& c, const char * & chr) {
        if (!(pos >= 0 && pos < number_of_variables)) {
            return 0;
        }

        switch (cv[pos].type) {
            case 10:
                c = cv[pos].string;
                break;
            default:
                return 0;
        }
        chr = cv[pos].name;
        return 1;
    }

    int get(int pos) {
        if (!(pos >= 0 && pos < number_of_variables)) {
            return pos;
            return -1;
        }

        switch (cv[pos].type) {
            case 1:
                return *cv[pos].ui8_t;
                break;
            case 2:
                return *cv[pos].ui16_t;
                break;
            case 3:
                return *cv[pos].ui32_t;
                break;
            case 4:
                return *cv[pos].i8_t;
                break;
            case 5:
                return *cv[pos].i16_t;
                break;
            case 6:
                return *cv[pos].i32_t;
                break;
            default:
                return -2;
        }
        return -3;
    }

    int get(int pos, CRGB& rgb) {
        if (!(pos >= 0 && pos < number_of_variables)) {
            return pos;
            return -1;
        }

        switch (cv[pos].type) {
            case 7:
                rgb = *cv[pos].rgb;
                break;
            default:
                return 1;
        }
        return -2;
    }

    int get(int pos, bool& b) {
        if (!(pos >= 0 && pos < number_of_variables)) {
            return pos;
            return -1;
        }

        switch (cv[pos].type) {
            case 8:
                b = *cv[pos].boo;
                break;
            case 9:
                b = *cv[pos].boo;
                break;
            default:
                return 1;
        }
        return -2;
    }

    int get(int pos, char *& c) {
        if (!(pos >= 0 && pos < number_of_variables)) {
            return pos;
            return -1;
        }

        switch (cv[pos].type) {
            case 10:
                c = cv[pos].string;
                break;
            default:
                return 1;
        }
        return -2;
    }

    int add(CRGB& rgb, const char * name) {
        if(number_of_variables > MAX_NUMBER_OF_CONTROL_VARIABLES-1) {
            return 0;
        }
        cv[number_of_variables].rgb = &rgb;
        cv[number_of_variables].type = 7;
        cv[number_of_variables].name = name;
        number_of_variables++;
        return 1;
    };

    int add(bool& b, const char * name, bool momentary = 0, char key = '\0') {
        if(number_of_variables > MAX_NUMBER_OF_CONTROL_VARIABLES-1) {
            return 0;
        }
        cv[number_of_variables].boo = &b;
        cv[number_of_variables].type = (momentary) ? 9: 8;
        cv[number_of_variables].name = name;
        //cv[number_of_variables].key = (momentary) ? key: '\0';
        cv[number_of_variables].key = key;
        number_of_variables++;
        return 1;
    };

    int key_down(char key) {
        for (int i = 0; i < number_of_variables; i++) {
            if (cv[i].type == 9 && cv[i].key == key) {
                *cv[i].boo = true;
                return 1;
            }
            if (cv[i].type == 8 && cv[i].key == key) {
                *cv[i].boo = !*cv[i].boo;
                return 1;
            }
        }
        return 0;
    }

    int key_up(char key) {
        for (int i = 0; i < number_of_variables; i++) {
            if (cv[i].type == 9 && cv[i].key == key) {
                *cv[i].boo = false;
                return 1;
            }
        }
        return 0;
    }

    int add(char * c, const char * name) {
        if(number_of_variables > MAX_NUMBER_OF_CONTROL_VARIABLES-1) {
            return 0;
        }
        cv[number_of_variables].string = c;
        cv[number_of_variables].type = 10;
        cv[number_of_variables].name = name;
        number_of_variables++;
        return 1;
    };

    int add(uint8_t& var, const char * name, int32_t r_min, int32_t r_max) {
        if(number_of_variables > MAX_NUMBER_OF_CONTROL_VARIABLES-1) {
            return 0;
        }
        cv[number_of_variables].ui8_t = &var;
        cv[number_of_variables].type = 1;
        cv[number_of_variables].name = name;
        cv[number_of_variables].range_min = r_min;
        cv[number_of_variables].range_max = r_max;
        number_of_variables++;
        return 1;
    };

    int add(uint16_t& var, const char * name, int32_t r_min, int32_t r_max) {
        if(number_of_variables > MAX_NUMBER_OF_CONTROL_VARIABLES-1) {
            return 0;
        }
        cv[number_of_variables].ui16_t = &var;
        cv[number_of_variables].type = 2;
        cv[number_of_variables].name = name;
        cv[number_of_variables].range_min = r_min;
        cv[number_of_variables].range_max = r_max;
        number_of_variables++;
        return 1;
    };

    int add(uint32_t& var, const char * name, int32_t r_min, int32_t r_max) {
        if(number_of_variables > MAX_NUMBER_OF_CONTROL_VARIABLES-1) {
            return 0;
        }
        cv[number_of_variables].ui32_t = &var;
        cv[number_of_variables].type = 3;
        cv[number_of_variables].name = name;
        cv[number_of_variables].range_min = r_min;
        cv[number_of_variables].range_max = r_max;
        number_of_variables++;
        return 1;
    };

    int add(int8_t& var, const char * name, int32_t r_min, int32_t r_max) {
        if(number_of_variables > MAX_NUMBER_OF_CONTROL_VARIABLES-1) {
            return 0;
        }
        cv[number_of_variables].i8_t = &var;
        cv[number_of_variables].type = 4;
        cv[number_of_variables].name = name;
        cv[number_of_variables].range_min = r_min;
        cv[number_of_variables].range_max = r_max;
        number_of_variables++;
        return 1;
    };

    int add(int16_t& var, const char * name, int32_t r_min, int32_t r_max) {
        if(number_of_variables > MAX_NUMBER_OF_CONTROL_VARIABLES-1) {
            return 0;
        }
        cv[number_of_variables].i16_t = &var;
        cv[number_of_variables].type = 5;
        cv[number_of_variables].name = name;
        cv[number_of_variables].range_min = r_min;
        cv[number_of_variables].range_max = r_max;
        number_of_variables++;
        return 1;
    };

    int add(int32_t& var, const char * name, int32_t r_min, int32_t r_max) {
        if(number_of_variables > MAX_NUMBER_OF_CONTROL_VARIABLES-1) {
            return 0;
        }
        cv[number_of_variables].i32_t = &var;
        cv[number_of_variables].type = 6;
        cv[number_of_variables].name = name;
        cv[number_of_variables].range_min = r_min;
        cv[number_of_variables].range_max = r_max;
        number_of_variables++;
        return 1;
    };

    void clear() {
        number_of_variables = 0;
        for (int i = 0; i < MAX_NUMBER_OF_CONTROL_VARIABLES; i++) {
            cv[i].key = '\0';
        }
    }

    int set(int& pos, int& val) {
        if(!(pos >= 0 && pos < number_of_variables)) {
            return 0;
        }
        switch (cv[pos].type) {
            case 1:
                *cv[pos].ui8_t = val;
                break;
            case 2:
                *cv[pos].ui16_t = val;
                break;
            case 3:
                *cv[pos].ui32_t = val;
                break;
            case 4:
                *cv[pos].i8_t = val;
                break;
            case 5:
                *cv[pos].i16_t = val;
                break;
            case 6:
                *cv[pos].i32_t = val;
                break;
            default:
                return 0;
        }
        return 1;
    }

    int set(int& pos, CRGB& rgb) {
        if(!(pos >= 0 && pos < number_of_variables)) {
            return 0;
        }
        switch (cv[pos].type) {
            case 7:
                *cv[pos].rgb = rgb;
                break;
            default:
                return 0;
        }
        return 1;
    }

    int set(int& pos, bool& b) {
        if(!(pos >= 0 && pos < number_of_variables)) {
            return 0;
        }
        switch (cv[pos].type) {
            case 8:
                *cv[pos].boo = b;
                break;
            case 9:
                *cv[pos].boo = b;
                break;
            default:
                return 0;
        }
        return 1;
    }


    int set(int& pos, char* c) {
        if(!(pos >= 0 && pos < number_of_variables)) {
            return 0;
        }
        char * p = cv[pos].string;
        switch (cv[pos].type) {
            case 10:
                *p = '\0';
                mystrcat(p, c);
                break;
            default:
                return 0;
        }
        return 1;
    }

};

CONTROL_VARIABLES control_variables;

#endif