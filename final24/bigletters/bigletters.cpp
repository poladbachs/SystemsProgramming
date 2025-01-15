#include <stdlib.h>
#include <stdio.h>

struct big_char {
    char c;
    unsigned width;
    unsigned height;
    unsigned depth;
    char * matrix;
};

struct big_char C[256];

unsigned max_Hs;
unsigned max_D;

void init_font () {
    for (unsigned int i = 0; i < 256; ++i) {
        C[i].c = i;
        C[i].width = 1;
        C[i].height = 1;
        C[i].depth = 0;
        C[i].matrix = NULL;
    }
    max_Hs = 1;
    max_D = 0;
}