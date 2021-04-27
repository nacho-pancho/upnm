#include <stdio.h>
#include <stdlib.h>

#include "pnm.h"

int main(int argc, char* argv[]) {
    char ofname[128];
    if (argc < 2) { 
        fprintf(stderr,"usage: test <image>.\n"); 
        return RESULT_ERROR; 
    }
    const char* fname = argv[1];    
    image_t* img = read_pnm(fname);
    if (img == NULL) {
        fprintf(stderr,"error reading image %s.\n",fname);
        return RESULT_ERROR;
    }
    if (img->info.result != RESULT_OK) {
        fprintf(stderr,"error reading image %s.\n",fname);
        pixels_free(img->pixels);
        free(img);
        return RESULT_ERROR;
    }
    snprintf(ofname,128,"copy_of_%s",fname);
    int res = write_pnm(ofname,img);
    if (res != RESULT_OK) {
        fprintf(stderr,"error writing image %s.\n",ofname);
    }

    pixels_free(img->pixels);
    free(img);
    return res;
}
