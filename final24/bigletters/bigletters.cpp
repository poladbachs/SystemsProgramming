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

const char * big_c_row (const struct big_char * c, unsigned level) {
    if (level < max_Hs - (c->height - c->depth))
	return 0;
    level -= max_Hs - (c->height - c->depth);
    if (level >= c->height)
	return 0;
    if (c->matrix)
	return c->matrix + (level * c->width);
    else
	return &(c->c);
}

int read_font (const char * filename) {
    FILE * input = fopen(filename, "r");
    if (!input)
	    return 0;
    char c;
    unsigned w, h, d;
    int res;
    while ((res = fscanf(input, "%c %u %u %u:", &c, &w, &h, &d)) != EOF ) {
	    if (res != 4)
	        goto error;
        unsigned c_idx = (unsigned char)c;
        C[c_idx].width = w;
	    C[c_idx].height = h;
	    C[c_idx].depth = d;
        if (d > h)
	        goto error;
        if (h - d > max_Hs)
	        max_Hs = h - d;
        if (d > max_D)
	        max_D = d;
        if (C[c_idx].matrix)
	        goto error;
        C[c_idx].matrix = (char *)malloc(w*h*sizeof(char));
        if (!C[c_idx].matrix)
	        goto error;
        if (fread(C[c_idx].matrix, w*h*sizeof(char), 1, input) != 1)
	        goto error;
        if (fgetc(input) != '\n')
	        goto error;
        }
        fclose(input);
        return 1;
    error:
        fclose(input);
        return 0;
};

void clear_font() {
    for (unsigned int i = 0; i < 256; ++i)
        if (C[i].matrix)
            free(C[i].matrix);
}

int main (int argc, char * argv[]) {
    const char * font_filename = "FONT";
    if (argc > 1)
	font_filename = argv[1];
    init_font();
    if (!read_font(font_filename)) {
        clear_font();
        return 1;
    }
    char line[1001];
    int first_line = 1;
    while (fgets(line, 1001, stdin)) {
        if (first_line)
            first_line = 0;
        else
            putchar('\n');
        	for (unsigned l = 0; l < max_Hs + max_D; ++l) {
	    unsigned pending_hspace = 0;
	    for (char * c = line; *c != '\n' && *c != 0; ++c) {
		const struct big_char * big_c = C + (unsigned char)*c;
		if (c != line)
		    pending_hspace += 1;
		const char * row = big_c_row(big_c, l);
		if (row) {
		    while (pending_hspace > 0) {
			putchar(' ');
			--pending_hspace;
		    }
		    for (unsigned j = 0; j < big_c->width; ++j)
			putchar(row[j]);
		} else
		    pending_hspace += big_c->width;
	    }
	    putchar('\n');
	}
    }
    clear_font();
    return 0;
}
