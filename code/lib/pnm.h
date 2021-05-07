#ifndef PNM_H
#define PNM_H

#include <stdint.h>
#include <stdio.h>

#include "image.h"

#define PNM_ASCII 0
#define PNM_BINARY 1

//
// PNM image type codes
//
// types 2 (ascii gray) and 3 (ascii color) are
// squashed onto types 5 and 6
// binary image types 4 (binary mono) and 1 (ascii mono)
// are not supported
//
#define PNM_GRAY  5
#define PNM_PGM   PNM_GRAY
#define PNM_COLOR 6
#define PNM_PPM   PNM_COLOR
#define RESULT_OK 0
#define RESULT_ERROR -1

//
//---------------------------------------------------------------------------------------------
// high-level interface (using image_info_t and image_t structs)
//---------------------------------------------------------------------------------------------
//
image_t * read_pnm ( const char * fname );
//
//---------------------------------------------------------------------------------------------
//
image_info_t read_pnm_info ( FILE * fhandle );
//
//---------------------------------------------------------------------------------------------
//
int read_all ( FILE * fhandle, const image_info_t * info, pixel_t * pixels );
//
//---------------------------------------------------------------------------------------------
//
int read_rows ( FILE * fhandle, const image_info_t * info, const int nrows, pixel_t * pixels );
//
//---------------------------------------------------------------------------------------------
//
int read_pixels ( FILE * fhandle, const int depth, const int channels, const int encoding, const int num, pixel_t * pixels );
//
//---------------------------------------------------------------------------------------------
//
int write_pnm ( const char * fname, const image_t * img );
//
//---------------------------------------------------------------------------------------------
//
int write_pnm_info ( const image_info_t * info, FILE * fhandle );
//
//---------------------------------------------------------------------------------------------
//
int write_pixels ( const int depth, const int channels, const int encoding, const int num, const pixel_t * pixels, FILE * fhandle );
//
//---------------------------------------------------------------------------------------------
//
int write_rows ( const image_info_t * info, const int nrows, const pixel_t * pixels, FILE * fhandle );
//
//---------------------------------------------------------------------------------------------
//
int write_all ( const image_info_t * info, const pixel_t * pixels, FILE * fhandle );

#endif
