#ifndef LIGHTS_HELPER_FUNCTIONS_H
#define LIGHTS_HELPER_FUNCTIONS_H

void mystrcat( char *& dest_i, const char* src )
{
     char * dest = dest_i;
     while (*dest) dest++;
     while ((*dest++ = *src++));
     dest_i = --dest;
}

#endif