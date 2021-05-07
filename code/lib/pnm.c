#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "pnm.h"
//
//---------------------------------------------------------------------------------------------
// forward declaration of non-public functions used in this module
//---------------------------------------------------------------------------------------------
//
static int skip_comments ( FILE * fp );
//
//---------------------------------------------------------------------------------------------
//
static int read_sample_16 ( FILE * fhandle );
//
//---------------------------------------------------------------------------------------------
//
static int read_sample_8 ( FILE * fhandle );
//
//---------------------------------------------------------------------------------------------
//
static int read_sample_1 ( FILE * fhandle, unsigned char * buffer, unsigned char* mask );
//
//---------------------------------------------------------------------------------------------
//
static int read_sample_ascii ( FILE * fhandle );
//
//---------------------------------------------------------------------------------------------
//
static int write_sample_16 ( int c, FILE * fhandle );
//
//---------------------------------------------------------------------------------------------
//
static int write_sample_8 ( int c, FILE * fhandle );
//
//---------------------------------------------------------------------------------------------
//
static int write_sample_1 ( int c, FILE * fhandle, unsigned char * buffer, unsigned char* mask );
//
//---------------------------------------------------------------------------------------------
//
static int write_sample_ascii ( int c, FILE * fhandle );
//
//---------------------------------------------------------------------------------------------
// main interface
//---------------------------------------------------------------------------------------------
//
static size_t bytes_written = 0; // for debugging
static size_t bytes_read = 0; // for debugging

//
//---------------------------------------------------------------------------------------------
//
image_info_t read_pnm_info ( FILE * fhandle ) {
    assert ( fhandle );
    rewind ( fhandle );
    image_info_t info;
    memset ( &info, 0, sizeof( image_info_t ) );
    int res;
    if ( fgetc ( fhandle ) != 'P' ) {
        fprintf ( stderr, "pnm: this is not a PNM file.\n" );
        info.result = RESULT_ERROR;
        return info;
    }
    info.type = fgetc ( fhandle ) - '0';
    switch ( info.type ) {
    case 1:
        info.channels = 1;
        info.encoding = PNM_ASCII;
        break;

    case 4:
        info.channels = 1;
        info.encoding = PNM_BINARY;
        break;

    case 2: // ASCII PGM
        info.channels = 1;
        info.encoding = PNM_ASCII;
        break;

    case 5: // binary PGM
        info.channels = 1;
        info.encoding = PNM_BINARY;
        break;

    case 3: // ASCII PPM
        info.channels = 3;
        info.encoding = PNM_ASCII;
        break;

    case 6: // binary PPM
        info.channels = 3;
        info.encoding = PNM_BINARY;
        break;
    }

    if ( ( res = skip_comments ( fhandle ) ) != RESULT_OK ) {
        fprintf ( stderr, "pnm: error skipping header comments.\n" );
        info.result = RESULT_ERROR;
        return info;
    }
    if ( ( res = fscanf ( fhandle, "%d", &info.width ) ) <= 0 ) {
        fprintf ( stderr, "pnm: error reading image width.\n" );
        info.result = RESULT_ERROR;
        return info;
    }
    if ( ( res = fscanf ( fhandle, " %d", &info.height ) ) <= 0 ) {
        fprintf ( stderr, "pnm: error reading image height.\n" );
        info.result = RESULT_ERROR;
        return info;
    }
    if ( ( info.type != 1 ) && ( info.type != 4 ) ) {
        if ( ( res = skip_comments ( fhandle ) ) != RESULT_OK ) {
            fprintf ( stderr, "pnm: error skipping comments (after dimensins).\n" );
            info.result = RESULT_ERROR;
            return info;
        }
        if ( ( res = fscanf ( fhandle, " %d ", &info.maxval ) ) <= 0 ) {
            fprintf ( stderr, "pnm: error reading maxval.\n" );
            info.result = RESULT_ERROR;
            return info;
        }
        info.depth = info.maxval < 256 ? 8 : 16;
    } else {
        info.maxval = 1;
        info.depth = 1;
    }
    // read trailing newline
    int c;
    if ( ( c = fgetc ( fhandle ) ) != '\n' ) {
        ungetc ( c, fhandle );
    }
    info.result = RESULT_OK;
    //
    // additional data derived from the other fields
    //
    return info;
}

//
//---------------------------------------------------------------------------------------------
//
int write_pnm_info ( const image_info_t * info, FILE * fhandle ) {
    assert ( fhandle );
    rewind ( fhandle );
    fprintf ( fhandle, "P%c\n", info->type + '0' );
    fprintf ( fhandle, "%d %d\n", info->width, info->height );
    if ( ( info->type != 1 )  && ( info->type != 4 ) ) {
        fprintf ( fhandle, "%d\n", info->maxval );
    }
    return ferror ( fhandle ) ? RESULT_ERROR : RESULT_OK;
}
//
//---------------------------------------------------------------------------------------------
//
image_t * read_pnm ( const char * fname ) {
    FILE * fhandle = fopen ( fname, "r" );
    if ( !fhandle ) {
        fprintf ( stderr, "pnm: error opening file %s for reading.\n", fname );
        return NULL;
    }
    image_info_t info = read_pnm_info ( fhandle );
    if ( info.result != RESULT_OK ) {
        fprintf ( stderr, "pnm: file %s is not a valid PNM.\n", fname );
        fclose ( fhandle );
        return NULL;
    }
    image_t * img = ( image_t * ) calloc ( 1, sizeof( image_t ) );

    img->info = info;
    img->pixels = pixels_alloc ( &info );
    if (read_all ( fhandle, &img->info, img->pixels ) != RESULT_OK) {
        fprintf( stderr, "pnm: error while reading pixels.\n");
    	fclose(fhandle);
    	return NULL;
    }
    fclose(fhandle);
    return img;
}
//
//---------------------------------------------------------------------------------------------
//
int write_pnm ( const char * fname, const image_t * img ) {
    int res;
    FILE * fhandle = fopen ( fname, "w" );
    if ( fhandle == NULL ) {
        fprintf ( stderr, "pnm: error opening file %s for writing.\n", fname );
        return RESULT_ERROR;
    }
    if ( ( res = write_pnm_info ( &img->info, fhandle ) ) == RESULT_ERROR ) {
        fprintf ( stderr, "pnm: error writing header on file %s.\n", fname );
        fclose ( fhandle );
        return RESULT_ERROR;
    }
    if ( ( res = write_all ( &img->info, img->pixels, fhandle ) ) == RESULT_ERROR ) {
        fprintf ( stderr, "pnm: error writing data on file %s.\n", fname );
        fclose ( fhandle );
        return RESULT_ERROR;
    }
    fclose ( fhandle );
    return RESULT_OK;
}
//
//---------------------------------------------------------------------------------------------
//
int read_pixels ( FILE * fhandle, const int depth, const int channels, const int encoding, const int num, pixel_t * pixels ) {
    const int nsamples = channels * num;
    if ( encoding == PNM_ASCII ) {

        for ( int i = 0 ; i < nsamples ; i++ ) {
            if ( ( pixels[ i ] = read_sample_ascii ( fhandle ) ) == RESULT_ERROR ) {
                return RESULT_ERROR;
            }
        }
        return RESULT_OK;

    } else if ( depth > 8 ) { // 9-16 bits: 2 bytes per sample

        for ( int i = 0 ; i < nsamples ; i++ ) {
            if ( ( pixels[ i ] = read_sample_16 ( fhandle ) ) == RESULT_ERROR ) {
                return RESULT_ERROR;
            }
        }
        return RESULT_OK;

    } else if ( depth > 1 ) { // 2-8

        for ( int i = 0 ; i < nsamples ; i++ ) {
            if ( ( pixels[ i ] = read_sample_8 ( fhandle ) ) == RESULT_ERROR ) {
                return RESULT_ERROR;
            }
        }
        return RESULT_OK;
    } else { // 1 bit binary
        fprintf(stderr,"Cannot read single pixels in binary-encoded bi-level images!");
        return RESULT_ERROR;
    }
}
//
//---------------------------------------------------------------------------------------------
//
int read_rows ( FILE * fhandle, const image_info_t * info, const int nrows, pixel_t * pixels ) {
    if (info->type != 4) {
        const int npixels = nrows * info->width;        
        return read_pixels ( fhandle, info->depth, info->channels, info->encoding, npixels, pixels );
    } else {
        index_t ncols = info->width;
        for ( int i = 0, li = 0 ; i < nrows ; i++ ) {
            unsigned char buffer = 0x00;
            unsigned char mask   = 0x00;
            for (int j = 0; j < ncols; ++j, ++li) {
                if ( ( pixels[ li ] = read_sample_1 ( fhandle, &buffer, &mask ) ) == RESULT_ERROR ) {
                    fprintf(stderr,"error reading PBM row %d col %d\n",i,j);                    
                    return RESULT_ERROR;
                }
            }
        }        
        printf("read %lu bytes\n",bytes_read);
        return RESULT_OK;
    }
}
//
//---------------------------------------------------------------------------------------------
//
int read_all ( FILE * fhandle, const image_info_t * info, pixel_t * pixels ) {
    return read_rows ( fhandle, info, info->height, pixels );
}
//
//---------------------------------------------------------------------------------------------
//
int write_pixels ( const int depth, const int channels, const int encoding, const int num, const pixel_t * pixels, FILE * fhandle ) {
    const int nsamples = channels * num;
    if ( encoding == PNM_ASCII ) {

        for ( int i = 0 ; i < nsamples ; i++ ) {
            if ( write_sample_ascii ( pixels[ i ], fhandle ) == RESULT_ERROR ) {
                return RESULT_ERROR;
            }
        }
        return RESULT_OK;

    } else if ( depth > 8 ) { // 9-16 bits: 2 bytes per sample

        for ( int i = 0 ; i < nsamples ; i++ ) {
            if ( write_sample_16 ( pixels[ i ], fhandle ) == RESULT_ERROR ) {
                return RESULT_ERROR;
            }
        }
        return RESULT_OK;

    } else if ( depth > 1 ) { // 2-8

        for ( int i = 0 ; i < nsamples ; i++ ) {
            if ( write_sample_8 ( pixels[ i ], fhandle ) == RESULT_ERROR ) {
                return RESULT_ERROR;
            }
        }
        return RESULT_OK;

    } else {
        fprintf(stderr,"Cannot write single pixels in PNM mode 4.");
        return RESULT_ERROR;
    }
}
//
//---------------------------------------------------------------------------------------------
//
int write_rows ( const image_info_t * info, const int nrows, const pixel_t * pixels, FILE * fhandle ) {
    if (info->type != 4) {
        const int npixels = nrows * info->width;
        return write_pixels ( info->depth, info->channels, info->encoding, npixels, pixels, fhandle );
    } else {
        const int ncols = info->width;
        for ( int i = 0, li = 0 ; i < nrows ; ++i ) {
            unsigned char buffer = 0x00;
            unsigned char mask = 0x80;
            for ( int j = 0 ; j < ncols ; ++j, ++li ) {
                if ( write_sample_1 ( pixels[ li ], fhandle, &buffer, &mask ) == RESULT_ERROR ) {
                    return RESULT_ERROR;
                }
            }            
            //
            // flush trailing bits
            //
            if (mask != 0x80) { // there are bits in the buffer
                fputc(buffer,fhandle);
                bytes_written++;
            }
        }
        printf("wrote %lu bytes\n",bytes_written);
        return RESULT_OK;
    }
}
//
//---------------------------------------------------------------------------------------------
//
int write_all ( const image_info_t * info, const pixel_t * pixels, FILE * fhandle ) {
    return write_rows ( info, info->height, pixels, fhandle );
}
//
//---------------------------------------------------------------------------------------------
// private interface
//---------------------------------------------------------------------------------------------
//
static int skip_comments ( FILE * fp ) {
    int ch;
    char line[ 100 ];

    while ( ( ch = fgetc ( fp ) ) != EOF && isspace ( ch ) )
        ;
    if ( ch == '#' ) {
        if (fgets ( line, sizeof( line ), fp ) == NULL)
            return RESULT_ERROR;
        skip_comments ( fp );
    } else
        fseek ( fp, -1, SEEK_CUR );
    return RESULT_OK;
}
//
//---------------------------------------------------------------------------------------------
//
static int read_sample_16 ( FILE * fhandle ) {
    unsigned char hilo[ 2 ];
    const int res = fread ( &hilo[ 0 ], 1, 2, fhandle );
    // PNM 16 bit samples are big endian!
    return res == 2 ? ( hilo[ 0 ] << 8 ) + hilo[ 1 ] : RESULT_ERROR;
}
//
//---------------------------------------------------------------------------------------------
//
static int read_sample_8 ( FILE * fhandle ) {
    unsigned char val;
    const int res = fread ( &val, 1, 1, fhandle );
    return res == 1 ? val : RESULT_ERROR;
}
//
//---------------------------------------------------------------------------------------------
//
static int read_sample_1 ( FILE * fhandle, unsigned char* pbuffer, unsigned char* pmask ) { 
    int res;
    unsigned char val;
    if ( *pmask == 0 ) {
        res = fread ( pbuffer, 1, 1, fhandle );
        if ( res != 1 ) return RESULT_ERROR;
        bytes_read++;
        *pmask = 0x80;
    }
    val = ( *pbuffer & *pmask ) ? 1 : 0;
    *pmask >>= 1; // shift one bit to right
    return val;
}
//
//---------------------------------------------------------------------------------------------
//
static int read_sample_ascii ( FILE * fhandle ) {
    int val;
    int res;
    res = fscanf ( fhandle, " %d ", &val );
    return res == 1 ? val : RESULT_ERROR;
}
//
//---------------------------------------------------------------------------------------------
//
static int write_sample_16 ( int c, FILE * fhandle ) {
    unsigned int hi = ( c >> 8 ) && 0xff;
    unsigned int lo = c && 0xff;
    // big endian: hi goes first
    if ( fputc ( hi, fhandle ) == EOF ) return RESULT_ERROR;
    if ( fputc ( lo, fhandle ) == EOF ) return RESULT_ERROR;
    return RESULT_OK;
}
//
//---------------------------------------------------------------------------------------------
//
static int write_sample_8 ( int c, FILE * fhandle ) {
    return fputc ( c, fhandle ) == EOF ? RESULT_ERROR : RESULT_OK;
}
//
//---------------------------------------------------------------------------------------------
//
static int write_sample_1 ( int v, FILE * fhandle, unsigned char* pbuffer, unsigned char* pmask ) { 
    int res;
    // write bit
    if ( v ) {
        *pbuffer |= *pmask;
    }
    // advance buffer
    *pmask >>= 1;
    if ( *pmask == 0 ) {  // write if buffer is full
        if ( ( res = fputc ( *pbuffer, fhandle ) ) == EOF ) {
            return RESULT_ERROR;
        }
        bytes_written++;
        *pmask = 0x80;
        *pbuffer = 0x00;
    }
    return RESULT_OK;
}
//
//---------------------------------------------------------------------------------------------
//
static int write_sample_ascii ( int c, FILE * fhandle ) {
    return fprintf ( fhandle, " %d", c ) <= 0 ? RESULT_ERROR : RESULT_OK;
}
