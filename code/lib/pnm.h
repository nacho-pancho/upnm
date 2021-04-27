#ifndef PNM_H
#define PNM_H

#include <stdint.h>

typedef uint16_t pixel_t;
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

#define PNM_ASCII 0
#define PNM_BINARY 1

typedef struct image_info {
  int width;    // width of image
  int height;   // height of image
  int channels; // number of channels
  int type;     // image type (grayscale, color) using PNM conventions
  int depth;    // number of *bits* per pixel, usually 8 or 16
  int maxval;   // maximum value, usually 255 or 65535
  int result;    // result of reading: image is valid only if this is RESULT_OK
  int encoding; // PNM_ASCII or PNM_BINARY
} image_info_t;

typedef struct image {
  image_info_t info;
  //
  // pixel data, stored in row major order (first row pixels, then second row, etc.)
  // multichannel pixels are stored in interleaved fashion: r,g,b,r,g,b,r,g,b...
  //
  pixel_t* pixels; 
} image_t;

//
//---------------------------------------------------------------------------------------------
//
pixel_t* pixels_alloc(const image_info_t* info);
//
//---------------------------------------------------------------------------------------------
//
void pixels_free(pixel_t* pix);
//
//---------------------------------------------------------------------------------------------
// high-level interface (using image_info_t and image_t structs)
//---------------------------------------------------------------------------------------------
//
image_t* read_pnm(const char* fname);
//
//---------------------------------------------------------------------------------------------
//
image_info_t read_pnm_info(FILE* fhandle);
//
//---------------------------------------------------------------------------------------------
//
int read_all(FILE* fhandle, const image_info_t* info, pixel_t* pixels);
//
//---------------------------------------------------------------------------------------------
//
int read_rows(FILE* fhandle, const image_info_t* info, const int nrows, pixel_t* pixels);
//
//---------------------------------------------------------------------------------------------
//
int read_pixels(FILE* fhandle, const int depth, const int channels, const int encoding, const int num, pixel_t* pixels);
//
//---------------------------------------------------------------------------------------------
//
int write_pnm(const char* fname, const image_t* img);
//
//---------------------------------------------------------------------------------------------
//
int write_pnm_info(const image_info_t* info, FILE* fhandle);
//
//---------------------------------------------------------------------------------------------
//
int write_pixels(const int depth, const int channels, const int encoding, const int num, const pixel_t* pixels, FILE* fhandle);
//
//---------------------------------------------------------------------------------------------
//
int write_rows(const image_info_t* info, const int nrows, const pixel_t* pixels, FILE* fhandle);
//
//---------------------------------------------------------------------------------------------
//
int write_all(const image_info_t* info, const pixel_t* pixels, FILE* fhandle);

#endif
