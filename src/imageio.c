/*
 * Copyright (C) 2009-2015 Joseph A. Marrero.  http://www.manvscode.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <assert.h>
#include <png.h>
#include "imageio.h"
#include "blending.h"


#ifdef _WIN32
#define strcasecmp    _stricmp
#define snprintf      _snprintf
#endif



/*
 *	Miscellaneous Utility functions...
 */
/* (x,y) to bitmap array index mapping macros */
#define pixel_index( x, y, byte_count, width )		((size_t) ((width) * (y) * (byte_count) + (x) * (byte_count)))
#define linear_interpolate( alpha, x2, x1 ) ( x1 + alpha * ( x2 - x1 ) )
#define lerp linear_interpolate
#define bilinear_interpolate( alpha, beta, x1, x2, x3, x4 )		(lerp( beta, lerp( alpha, x1, x2 ), lerp( alpha, x3, x4 ) ))
#define bilerp bilinear_interpolate


#define linear_interpolate( alpha, x2, x1 ) ( x1 + alpha * ( x2 - x1 ) )
#define lerp linear_interpolate
#define bilinear_interpolate( alpha, beta, x1, x2, x3, x4 )		(lerp( beta, lerp( alpha, x1, x2 ), lerp( alpha, x3, x4 ) ))
#define bilerp bilinear_interpolate

#define convertRGBtoBGR    imageio_swap_red_and_blue
#define convertBGRtoRGB    imageio_swap_red_and_blue

/*
 *	Windows Bitmap
 */
#define	BITMAP_ID	0x4D42

#define BI_RGB			0L
#define BI_RLE8			1L
#define BI_RLE4			2L
#define BI_BITFIELDS	3L
#define BI_JPEG			4L
#define BI_PNG			5L

#pragma pack(push, 2)
imageio_api typedef struct imageio_bitmap_file_header {
	uint16_t bfType;       // Specifies File Type; Must be BM (0x4D42)
	uint32_t bfSize;       // Specifies the size in bytes of the bitmap
	uint16_t bfReserved1;  // Reserved; Must be zero!
	uint16_t bfReserved2;  // Reserved; Must be zero!
	uint32_t bfOffBits;    // Specifies the offset, in bytes, from the beginning to the bitmap bits
} bitmap_file_header_t;

imageio_api typedef struct imageio_bitmap_info_header {
	uint32_t   biSize;          // Specifies number of bytes required by the structure
	int32_t    biWidth;	        // Specifies the width of the bitmap, in pixels
	int32_t    biHeight;        // Specifies the height of the bitmap, in pixels
	uint16_t   biPlanes;        // Specifies the number of color planes, must be 1
	uint16_t   biBitCount;      // Specifies the bits per pixel, must be 1, 4,
			                    // 8, 16, 24, or 32
	uint32_t   biCompression;   // Specifies the type of compression
	uint32_t   biSizeImage;     // Specifies the size of the image in bytes
	int32_t    biXPelsPerMeter; // Specifies the number of pixels per meter in x axis
	int32_t    biYPelsPerMeter; // Specifies the number of pixels per meter in y axis
	uint32_t   biClrUsed;       // Specifies the number of colors used by the bitmap
	uint32_t   biClrImportant;  // Specifies the number of colors that are important
} bitmap_info_header_t;
#pragma pack(pop)

/*
 *  Targa
 */
#pragma pack(push, 1)
imageio_api typedef struct imageio_targa_file_header {
	uint8_t imageIDLength;      // number of bytes in identification field;
                                // 0 denotes no identification is included.
	uint8_t colorMapType;       // type of color map; always 0
	uint8_t imageTypeCode;      //  0  -  No image data included.
                                //	1  -  Uncompressed, color-mapped images.
                                //	2  -  Uncompressed, RGB images.
                                //	3  -  Uncompressed, black and white images.
                                //	9  -  Runlength encoded color-mapped images.
                                //  10 -  Runlength encoded RGB images.
                                //  11 -  Compressed, black and white images.
                                //  32 -  Compressed color-mapped data, using Huffman, Delta, and
                                //	  	  runlength encoding.
                                //  33 -  Compressed color-mapped data, using Huffman, Delta, and
                                //		  runlength encoding.  4-pass quadtree-type process.
	int16_t colorMapOrigin;     // origin of color map (lo-hi); always 0
	int16_t colorMapLength;     // length of color map (lo-hi); always 0
	uint8_t colorMapEntrySize;  // color map entry size (lo-hi); always 0;
	int16_t imageXOrigin;       // x coordinate of lower-left corner of image; (lo-hi); always 0
	int16_t imageYOrigin;       // y coordinate of lower-left corner of image; (lo-hi); always 0
	uint16_t width;             // width of image in pixels (lo-hi)
	uint16_t height;            // height of image in pixels (lo-hi)
	uint8_t bitCount;           // number of bits; 16, 24, 32
	uint8_t imageDescriptor;    // 24 bit = 0x00; 32-bit = 0x08
} targa_file_header_t;
#pragma pack(pop)



typedef struct pvr_header {
    uint32_t header_length;
    uint32_t height;
    uint32_t width;
    uint32_t num_mipmaps;
    uint32_t flags;
    uint32_t data_length;
    uint32_t bit_depth;
    uint32_t bitmask_red;
    uint32_t bitmask_green;
    uint32_t bitmask_blue;
    uint32_t bitmask_alpha;
    uint32_t pvr_tag;
    uint32_t num_surfs;
} pvr_header_t;


static __inline bool imageio_bitmap_load ( const char* filename, bitmap_info_header_t* info_header, uint8_t** bitmap );
static __inline bool imageio_bitmap_save ( const char* filename, uint32_t width, uint32_t height, uint32_t bit_depth, uint8_t* imageData );
static __inline bool imageio_targa_load  ( const char* filename, targa_file_header_t* p_file_header, uint8_t** bitmap );
static __inline bool imageio_targa_save  ( const char* filename, targa_file_header_t* p_file_header, uint8_t* bitmap );
static __inline bool imageio_pvr_load    ( const char* filename, pvr_header_t* p_header, uint8_t** bitmap );

static __inline bool imageio_png_load ( const char* filename, image_t* image );
static __inline bool imageio_png_save ( const char* filename, const image_t* image );

static __inline bool imageio_resize_nearest_neighbor      ( uint32_t src_width, uint32_t src_height, uint32_t srcBitsPerPixel, const uint8_t* src_bitmap, uint32_t dst_width, uint32_t dst_height, uint32_t dstBitsPerPixel, uint8_t* dst_bitmap );
static __inline bool imageio_resize_bilinear_rgba         ( uint32_t src_width, uint32_t src_height, const uint8_t* src_bitmap, uint32_t dst_width, uint32_t dst_height, uint8_t* dst_bitmap, uint32_t byte_count );
static __inline bool imageio_resize_bilinear_sharper_rgba ( uint32_t src_width, uint32_t src_height, const uint8_t* src_bitmap, uint32_t dst_width, uint32_t dst_height, uint8_t* dst_bitmap, uint32_t byte_count );
static __inline bool imageio_resize_bilinear_rgb          ( uint32_t src_width, uint32_t src_height, const uint8_t* src_bitmap, uint32_t dst_width, uint32_t dst_height, uint8_t* dst_bitmap, uint32_t byte_count );
static __inline bool imageio_resize_bilinear_sharper_rgb  ( uint32_t src_width, uint32_t src_height, const uint8_t* src_bitmap, uint32_t dst_width, uint32_t dst_height, uint8_t* dst_bitmap, uint32_t byte_count );
static __inline bool imageio_resize_bicubic_rgba          ( uint32_t src_width, uint32_t src_height, const uint8_t* src_bitmap, uint32_t dst_width, uint32_t dst_height, uint8_t* dst_bitmap, uint32_t byte_count );
static __inline bool imageio_resize_bicubic_rgb           ( uint32_t src_width, uint32_t src_height, const uint8_t* src_bitmap, uint32_t dst_width, uint32_t dst_height, uint8_t* dst_bitmap, uint32_t byte_count );

static __inline bool is_power_of_2( uint16_t x )
{
	return (x & (x - 1)) == 0;
}

bool imageio_load( image_t* img, const char* filename, image_file_format_t* fmt )
{
	bool result = false;
	const char* extension = strrchr( filename, '.' );
	image_file_format_t format = IMAGEIO_PNG;

	if( extension )
	{
		extension += 1;

		if( strcasecmp( "png", extension ) == 0 )
		{
			format = IMAGEIO_PNG;
		}
		else if( strcasecmp( "bmp", extension ) == 0 )
		{
			format = IMAGEIO_BMP;
		}
		else if( strcasecmp( "tga", extension ) == 0 )
		{
			format = IMAGEIO_TGA;
		}
		else if( strcasecmp( "pvr", extension ) == 0 || strcasecmp( "pvrtc", extension ) == 0 )
		{
			format = IMAGEIO_PVR;
		}
		else
		{
			goto failure;
		}
	}

	if( imageio_image_load( img, filename, format ) )
	{
		if( format == IMAGEIO_PVR )
		{
			if( !is_power_of_2(img->width) || !is_power_of_2(img->height) )
			{
				   /* iOS devices require texture dimensions to be
					* a power of 2.
					*/
				   imageio_image_destroy( img );
				   goto failure;
			}
		}

		result = true;
	}

	if( fmt )
	{
		*fmt = format;
	}

failure:
	return result;
}

bool imageio_image_load( image_t* img, const char* filename, image_file_format_t format )
{
	bool result = false;
	#ifdef NDEBUG
	img->pixels = NULL;
	#endif

	switch( format )
	{
		case IMAGEIO_BMP:
		{
			bitmap_info_header_t bmpInfoHeader;
			result = imageio_bitmap_load( filename, &bmpInfoHeader, &img->pixels );
			if( result )
			{
				assert( img->pixels != NULL );
				img->bit_depth = (uint8_t) bmpInfoHeader.biBitCount;
				img->channels  = bmpInfoHeader.biBitCount >> 3;
				img->width     = bmpInfoHeader.biWidth;
				img->height    = bmpInfoHeader.biHeight;
			}
			break;
		}
		case IMAGEIO_TGA:
		{
			targa_file_header_t tgaHeader;
			result = imageio_targa_load( filename, &tgaHeader, &img->pixels );
			if( result )
			{
				assert( img->pixels != NULL );
				img->bit_depth = tgaHeader.bitCount;
				img->channels  = tgaHeader.bitCount >> 3;
				img->width     = tgaHeader.width;
				img->height    = tgaHeader.height;
			}
			break;
		}
		case IMAGEIO_PNG:
		{
			result = imageio_png_load( filename, img );
			break;
		}
		case IMAGEIO_PVR:
		{
			pvr_header_t header;
			result = imageio_pvr_load( filename, &header, &img->pixels );
			if( result )
			{
				assert( img->pixels != NULL );
				img->bit_depth = header.bit_depth;
				img->channels  = header.bitmask_alpha > 0 ? 4 : 3;
				img->width     = header.width;
				img->height    = header.height;
			}
			break;
		}
		default:
			break;
	}

	if( !result )
	{
		img->bit_depth = 0;
		img->channels  = 0;
		img->width     = 0;
		img->height    = 0;
		img->pixels    = 0;
	}

	return result;
}

bool imageio_image_save( const image_t* img, const char* filename, image_file_format_t format )
{
	bool result = false;

	switch( format )
	{
		case IMAGEIO_BMP:
		{
			result = imageio_bitmap_save( filename, img->width, img->height, img->bit_depth, img->pixels );
			assert( img->pixels != NULL );
			break;
		}
		case IMAGEIO_TGA:
		{
			targa_file_header_t tgaFileHeader;
			tgaFileHeader.bitCount = img->bit_depth;
			tgaFileHeader.width = img->width;
			tgaFileHeader.height = img->height;
			result = imageio_targa_save( filename, &tgaFileHeader, img->pixels );
			break;
		}
		case IMAGEIO_PNG:
		{
			result = imageio_png_save( filename, img );
			break;
		}
		default:
			break;
	}


	return result;
}

bool imageio_image_create( image_t* img, uint16_t width, uint16_t height, uint8_t bit_depth )
{
	bool result = false;

	if( img )
	{
		img->bit_depth = bit_depth;
		img->channels  = bit_depth >> 3;
		img->width     = width;
		img->height    = height;
		img->pixels    = malloc( img->width * img->height * img->channels );

		result = img->pixels != NULL;
	}

	return result;
}

void imageio_image_destroy( image_t* img )
{
	free( img->pixels );
	#ifdef NDEBUG
	img->pixels = NULL;
	#endif
}

bool imageio_bitmap_load( const char* filename, bitmap_info_header_t* info_header, uint8_t** bitmap )
{
	bitmap_file_header_t bmp_file_header;
	register uint32_t imageIdx = 0;
	unsigned short bytesPerPixel = 0;
	uint32_t bitmapSize = 0;
	uint32_t scanlineBytes = 0;
	uint32_t stride = 0;
	FILE* filePtr = fopen( filename, "rb" );

	if( !filePtr )
	{
		*bitmap = NULL;
		return false;
	}

	fread( &bmp_file_header.bfType, sizeof(uint16_t), 1, filePtr );
	fread( &bmp_file_header.bfSize, sizeof(uint32_t), 1, filePtr );
	fread( &bmp_file_header.bfReserved1, sizeof(uint16_t), 1, filePtr );
	fread( &bmp_file_header.bfReserved2, sizeof(uint16_t), 1, filePtr );
	fread( &bmp_file_header.bfOffBits, sizeof(uint32_t), 1, filePtr );

	if( bmp_file_header.bfType != BITMAP_ID )
	{
		fclose(filePtr);
		*bitmap = NULL;
		return false;
	}

	fread( info_header, sizeof(bitmap_info_header_t), 1, filePtr );
	bytesPerPixel = info_header->biBitCount >> 3;
	scanlineBytes = info_header->biWidth * bytesPerPixel;
	stride = (info_header->biWidth * bytesPerPixel + 3) & ~3;
	fseek( filePtr, bmp_file_header.bfOffBits, SEEK_SET );

	bitmapSize = scanlineBytes * info_header->biHeight;
	*bitmap = (uint8_t*) malloc( bitmapSize );

#ifdef NDEBUG
	memset( *bitmap, 0, bitmapSize );
#endif

	/* check if allocation failed... */
	if( *bitmap == NULL )
	{
		fclose( filePtr );
		return false;
	}

	for( imageIdx = 0; imageIdx < bitmapSize; imageIdx += scanlineBytes )
	{
		// read in pixels and skip padding...
		fread( *bitmap + imageIdx, scanlineBytes, 1, filePtr );
		fseek( filePtr, stride - scanlineBytes, SEEK_CUR );
	}

	convertBGRtoRGB( info_header->biWidth, info_header->biHeight, bytesPerPixel, *bitmap );

	fclose( filePtr );
	return true;
}

bool imageio_bitmap_save( const char* filename, uint32_t width, uint32_t height, uint32_t bit_depth, uint8_t* imageData )
{
	FILE* filePtr;
	bitmap_file_header_t bmp_file_header;
	bitmap_info_header_t info_header;
	uint8_t nullByte;
	uint32_t imageIdx = 0;
	uint32_t bitmapIdx = 0;
	uint32_t bytesPerPixel = bit_depth >> 3;
	uint32_t stride = (width * bytesPerPixel + 3) & ~3;
	uint32_t scanlineBytes = width * bytesPerPixel;

	if( (filePtr = fopen(filename, "wb")) == NULL )
	{
		return false;
	}

	/* Define the bmp_file_header */
	bmp_file_header.bfSize = sizeof(bitmap_file_header_t);
	bmp_file_header.bfType = BITMAP_ID;
	bmp_file_header.bfReserved1 = 0;
	bmp_file_header.bfReserved2 = 0;
	bmp_file_header.bfOffBits = sizeof(bitmap_file_header_t) + sizeof(bitmap_info_header_t);
	/* Define the info_header */
	info_header.biSize = sizeof(bitmap_info_header_t);
	info_header.biPlanes = 1;
	info_header.biBitCount = bit_depth;
	info_header.biCompression = BI_RGB;		/* No compression */
	info_header.biSizeImage = stride * height;  /* w * h * (4 bytes) */
	info_header.biXPelsPerMeter = 0;
	info_header.biYPelsPerMeter = 0;
	info_header.biClrUsed = 0;
	info_header.biClrImportant = 0;
	info_header.biWidth = width;
	info_header.biHeight = height;

	convertRGBtoBGR( width, height, bytesPerPixel, imageData );

	fwrite( &bmp_file_header, sizeof(bitmap_file_header_t), 1, filePtr );
	fwrite( &info_header, sizeof(bitmap_info_header_t), 1, filePtr );

	for( imageIdx = 0, bitmapIdx = 0; imageIdx <  info_header.biSizeImage; imageIdx += stride, bitmapIdx += scanlineBytes )
	{
		fwrite( imageData + bitmapIdx, scanlineBytes, 1, filePtr );
		fwrite( &nullByte, 1, stride - scanlineBytes, filePtr );
	}

	fclose( filePtr );
	return true;
}

bool imageio_targa_load( const char* filename, targa_file_header_t* p_file_header, uint8_t** bitmap )
{
	uint32_t colorMode;		/* 4 for RGBA or 3 for RGB */
	uint32_t imageSize = 0;
	FILE* filePtr = fopen( filename, "rb" );

	if( !filePtr )
	{
		return false;
	}

	/* read in first two bytes we don't need */
	fread( p_file_header, sizeof(targa_file_header_t), 1, filePtr );


	if( (p_file_header->imageTypeCode != 2) && (p_file_header->imageTypeCode != 3) )
	{
		fclose( filePtr );
		return false;
	}

	/* colorMode-> 3 = BGR, 4 = BGRA */
	colorMode = p_file_header->bitCount >> 3; // bytes per pixel


	imageSize = p_file_header->width * p_file_header->height * colorMode;

	*bitmap = (uint8_t*) malloc( imageSize );

	/* check if allocation failed... */
	if( *bitmap == NULL )
	{
		return false;
	}

	fread( *bitmap, 1, imageSize, filePtr );

	/* indexed color mode not handled!!! */
	convertBGRtoRGB( p_file_header->width, p_file_header->height, colorMode, *bitmap );

	fclose( filePtr );
	return true;
}

bool imageio_targa_save( const char* filename, targa_file_header_t* p_file_header, uint8_t* bitmap )
{
	FILE* filePtr;
	long imageSize;		/* size of the Targa image */
	uint32_t colorMode = p_file_header->bitCount >> 3; /* 4 for RGBA or 3 for RGB */

	if( ( filePtr = fopen( filename, "wb" ) ) == NULL )
	{
		return false;
	}

	p_file_header->imageIDLength     = 0;
	p_file_header->imageTypeCode     = 2;
	p_file_header->colorMapOrigin    = 0;
	p_file_header->colorMapLength    = 0;
	p_file_header->colorMapEntrySize = 0;
	p_file_header->colorMapType      = 0;
	p_file_header->imageXOrigin      = 0;
	p_file_header->imageYOrigin      = 0;
	p_file_header->imageDescriptor   = colorMode == 4 ? 0x08 : 0x00;

	assert( p_file_header->imageTypeCode == 0x2 || p_file_header->imageTypeCode == 0x3 ); // must be 2 or 3

	fwrite( p_file_header, sizeof(targa_file_header_t), 1, filePtr );

	convertRGBtoBGR( p_file_header->width, p_file_header->height, colorMode, bitmap );

	imageSize = p_file_header->width * p_file_header->height * colorMode;
	fwrite( bitmap, imageSize, 1, filePtr );

	fclose( filePtr );
	return true;
}

bool imageio_pvr_load( const char* filename, pvr_header_t* p_header, uint8_t** bitmap )
{
	FILE* file;

	if( (file = fopen( filename, "rb" )) == NULL )
	{
		return false;
	}

	/* Read in pvr header */
	fread( p_header, sizeof(pvr_header_t), 1, file );

	/* Jump to start of pixel data */
	fseek( file, p_header->header_length, SEEK_SET );

	size_t image_size = p_header->data_length;

	uint8_t* pixel_data = malloc( image_size );

	if( pixel_data )
	{
		fread( pixel_data, image_size, 1, file );
	}
	else
	{
		return false;
	}

	*bitmap = pixel_data;
	fclose( file );
	return true;
}

bool imageio_png_load( const char* filename, image_t* image )
{
	png_structp png_ptr;
	png_infop info_ptr;
	uint8_t header[8];
	FILE *file;

	file = fopen( filename, "rb" );

	/* test if a png */
	if( !file )
	{
		return false;
	}

	fread( header, 1, sizeof(header), file );
	if( png_sig_cmp( header, 0, sizeof(header) ) )
	{
		/* not a png file */
		return false;
	}

	/* initialize stuff */
	png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );

	if( !png_ptr )
	{
		return false;
	}

	info_ptr = png_create_info_struct( png_ptr );
	if( !info_ptr )
	{
        png_destroy_read_struct( &png_ptr, (png_infopp) NULL, (png_infopp) NULL );
		fclose( file );
		return false;
	}

	if( setjmp(png_jmpbuf(png_ptr)) )
	{
        png_destroy_read_struct( &png_ptr, &info_ptr, NULL );
		fclose( file );
		return false;
	}

	png_init_io( png_ptr, file );
	png_set_sig_bytes( png_ptr, sizeof(header) );

	png_read_info( png_ptr, info_ptr );

	//png_uint_32 width, height;
	//int bits_per_channel, color_type, interlace_type;
	//png_get_IHDR( png_ptr, info_ptr, &width, &height, &bits_per_channel, &color_type, &interlace_type, NULL, NULL );

	//png_byte interlace_type   = png_get_interlace_type( png_ptr, info_ptr );
	png_byte color_type       = png_get_color_type( png_ptr, info_ptr );
	png_byte bits_per_channel = png_get_bit_depth( png_ptr, info_ptr );

	image->width  = png_get_image_width( png_ptr, info_ptr );
	image->height = png_get_image_height( png_ptr, info_ptr );

	switch( color_type )
	{
		case PNG_COLOR_TYPE_PALETTE:
      		png_set_palette_to_rgb( png_ptr );

			/* expanded to RGB */
			image->bit_depth = 3 * bits_per_channel; //png_get_bit_depth( png_ptr, info_ptr );
			image->channels  = 3;
			break;
		case PNG_COLOR_TYPE_RGB:
			image->bit_depth = 3 * bits_per_channel; //png_get_bit_depth( png_ptr, info_ptr );
			image->channels  = 3;
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			image->bit_depth = 4 * bits_per_channel; //png_get_bit_depth( png_ptr, info_ptr );
			image->channels  = 4;
			break;
		default:
			/* paletted types and grayscale not supported */
			png_destroy_read_struct( &png_ptr, &info_ptr, NULL );
			fclose( file );
			return false;
	}

	#if 0
	int screen_gamma = PNG_DEFAULT_sRGB;  /* A good guess for a PC monitor */
	int intent;

	if( png_get_sRGB( png_ptr, info_ptr, &intent ) != 0 )
	{
		png_set_gamma( png_ptr, screen_gamma, PNG_DEFAULT_sRGB );
	}
	else
	{
		double image_gamma;
		if( png_get_gAMA( png_ptr, info_ptr, &image_gamma ) != 0 )
		{
			png_set_gamma( png_ptr, screen_gamma, image_gamma );
		}
		else
		{
			png_set_gamma( png_ptr, screen_gamma, 0.45455 );
		}
	}
	#endif
	#if 0
	if( (color_type & PNG_COLOR_MASK_COLOR) != 0 )
	{
		/* Flip the RGB pixels to BGR (or RGBA to BGRA) */
		png_set_bgr(png_ptr);
	}
	#endif

	/*number_of_passes = png_set_interlace_handling( png_ptr );*/
	png_read_update_info( png_ptr, info_ptr);

    png_size_t row_bytes = png_get_rowbytes( png_ptr, info_ptr );

	image->pixels = malloc( row_bytes * image->height * sizeof(png_byte) );

    if( !image->pixels )
    {
        png_destroy_read_struct( &png_ptr, &info_ptr, NULL );
        fclose( file );
        return false;
    }

    /* row_pointers is for pointing to image->pixels for reading the png with libpng */
    png_bytep* row_pointers = malloc( image->height * sizeof(png_bytep) );

    if( !row_pointers )
    {
        png_destroy_read_struct( &png_ptr, &info_ptr, NULL );
        free( image->pixels );
        fclose( file );
        return false;
    }

	/* set the individual row_pointers to point at the correct offsets of image->pixels */
    int i;
    for( i = 0; i < image->height; i++ )
    {
        row_pointers[ i ] = image->pixels + i * row_bytes;
    }

	if( setjmp(png_jmpbuf(png_ptr)) )
	{
        png_destroy_read_struct( &png_ptr, &info_ptr, NULL );
		free( row_pointers );
        free( image->pixels );
        fclose( file );
		return false;
	}

	png_read_image( png_ptr, row_pointers );

	free( row_pointers );
	png_read_end( png_ptr, info_ptr );
    png_destroy_read_struct( &png_ptr, &info_ptr, NULL );
	fclose( file );

	return true;
}

bool imageio_png_save( const char* filename, const image_t* image )
{
	png_structp png_ptr;
	png_infop info_ptr;
	FILE *file;

	if( !image )
	{
		return false;
	}

	/* create file */
	file = fopen( filename, "wb" );

	if( !file )
	{
		return false;
	}

	/* initialize stuff */
	png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );

	if( !png_ptr )
	{
		return false;
	}

	info_ptr = png_create_info_struct( png_ptr );

	if( !info_ptr)
	{
        png_destroy_write_struct( &png_ptr, (png_infopp) NULL );
		fclose( file );
	}

	if( setjmp(png_jmpbuf(png_ptr)))
	{
        png_destroy_write_struct( &png_ptr, &info_ptr );
		fclose( file );
		return false;
	}

	png_init_io( png_ptr, file );

	int bit_depth       = 8;
	int color_type      = PNG_COLOR_TYPE_RGB_ALPHA;
	int bytes_per_pixel = image->bit_depth >> 3;


	switch( bytes_per_pixel )
	{
		case 4:

			color_type = PNG_COLOR_TYPE_RGB_ALPHA;
			break;
		case 3:
		case 2:
			color_type = PNG_COLOR_TYPE_RGB;
			break;
		case 1:
			color_type = PNG_COLOR_TYPE_GRAY;
			break;
		default:
			png_destroy_write_struct( &png_ptr, &info_ptr );
			fclose( file );
			return false;
	}


	png_set_IHDR( png_ptr, info_ptr, image->width, image->height,
	              bit_depth, color_type, PNG_INTERLACE_NONE,
	              PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE );

	png_write_info( png_ptr, info_ptr );

    /* row_pointers is for pointing to image->pixels for reading the png with libpng */
    png_bytep* row_pointers = malloc( image->height * sizeof(png_bytep) );

    if( !row_pointers )
    {
		png_destroy_write_struct( &png_ptr, &info_ptr );
        fclose( file );
        return false;
    }

	/* set the individual row_pointers to point at the correct offsets of image->pixels */
	int row_bytes = image->width * bytes_per_pixel;
    int i;
    for( i = 0; i < image->height; i++ )
    {
        //row_pointers[ image->height - 1 - i] = image->pixels + i * row_bytes;
        row_pointers[ i ] = image->pixels + i * row_bytes;
    }

	if( setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_write_struct( &png_ptr, &info_ptr );
		free( row_pointers );
		fclose( file );
		return false;
	}

	png_write_image( png_ptr, row_pointers );
	png_write_end( png_ptr, NULL );
	png_destroy_write_struct( &png_ptr, &info_ptr );
	free( row_pointers );
	fclose( file );

	return true;
}

/*
 *  Image stretching functions...
 */
void imageio_image_resize( uint32_t src_width, uint32_t src_height, const uint8_t* src_bitmap,
                           uint32_t dst_width, uint32_t dst_height, uint8_t* dst_bitmap, uint32_t bit_depth,
                           resize_algorithm_t algorithm )
{
	uint32_t byte_count = bit_depth >> 3;
	assert( src_bitmap != NULL || dst_bitmap != NULL );
	assert( src_width != 0 || src_height != 0 );
	assert( byte_count > 2 );

	switch( algorithm )
	{
		case ALG_NEARESTNEIGHBOR:
			/* forces both bitmaps to have the same bit depth */
			imageio_resize_nearest_neighbor( src_width, src_height, bit_depth, src_bitmap,
				             dst_width, dst_height, bit_depth, dst_bitmap );
			break;
		case ALG_BILINEAR:
			switch( byte_count )
			{
				case 0:
				case 1:
				case 2:
					assert( false ); /* unsupported bit_depth's */
					break;
				case 3:
						imageio_resize_bilinear_rgb( src_width, src_height, src_bitmap, dst_width, dst_height, dst_bitmap, byte_count );
						break;
				case 4:
						imageio_resize_bilinear_rgba( src_width, src_height, src_bitmap, dst_width, dst_height, dst_bitmap, byte_count );
						break;
				default: break;
			}
			break;
		case ALG_BILINEAR_SHARPER:
			switch( byte_count )
			{
				case 0:
				case 1:
				case 2:
					assert( false ); /* unsupported bytes per pixels */
					break;
				case 3:
						imageio_resize_bilinear_sharper_rgb( src_width, src_height, src_bitmap, dst_width, dst_height, dst_bitmap, byte_count );
						break;
				case 4:
						imageio_resize_bilinear_sharper_rgba( src_width, src_height, src_bitmap, dst_width, dst_height, dst_bitmap, byte_count );
						break;
				default: break;
			}
			break;
		case ALG_BICUBIC:
			switch( byte_count )
			{
				case 0:
				case 1:
				case 2:
					assert( false ); /* unsupported bit_depth's */
					break;
				case 3:
						imageio_resize_bicubic_rgb( src_width, src_height, src_bitmap, dst_width, dst_height, dst_bitmap, byte_count );
						break;
				case 4:
						imageio_resize_bicubic_rgba( src_width, src_height, src_bitmap, dst_width, dst_height, dst_bitmap, byte_count );
						break;
				default: break;
			}
			break;
		default:
			assert( false ); /* bad algorithm... */
	}
}

/* uses nearest neighbor... */
bool imageio_resize_nearest_neighbor( uint32_t src_width, uint32_t src_height, uint32_t srcBitsPerPixel, const uint8_t* src_bitmap,
                                      uint32_t dst_width, uint32_t dst_height, uint32_t dstBitsPerPixel, uint8_t* dst_bitmap )
{
	uint32_t srcByteCount = srcBitsPerPixel >> 3;
	uint32_t dstByteCount = dstBitsPerPixel >> 3;
	register uint32_t x = 0;
	register uint32_t y = 0;
	float horizontalStretchFactor = 0;
	float verticalStretchFactor = 0;
	/* uint32_t srcY, srcX;  don't remove */
	uint32_t dstPos, srcPos;
	register uint32_t srcYtimesWidth;
	register uint32_t yTimesWidth;

	assert( srcBitsPerPixel != 0 || dstBitsPerPixel != 0 );
	assert( src_bitmap != NULL || dst_bitmap != NULL );
	assert( src_width != 0 || src_height != 0 );

	if( !src_bitmap || !dst_bitmap || srcBitsPerPixel == 0 || dstBitsPerPixel == 0 || src_width == 0 || src_height == 0 )
	{
		return false;
	}

	horizontalStretchFactor = (float) src_width / (float) dst_width;
	verticalStretchFactor = (float) src_height / (float) dst_height;

	/* don't remove...
	 * x_in = x_out * (w_in / w_out), using nearest neighbor
	 */
	for( y = 0; y < dst_height; y++ )
	{
		srcYtimesWidth = ((uint32_t)(y * verticalStretchFactor)) * src_width * srcByteCount;
		yTimesWidth = y * dst_width * dstByteCount;

		for( x = 0; x < dst_width; x++ )
		{
			dstPos = yTimesWidth + x * dstByteCount;
			srcPos = srcYtimesWidth + ((uint32_t)(x * horizontalStretchFactor)) * srcByteCount;

			memcpy( &dst_bitmap[ dstPos ], &src_bitmap[ srcPos ], dstByteCount * sizeof(uint8_t) );
		}
	}

	return true;
}

/* bi-linear: nearest neighbor with bilinear interpolation */
bool imageio_resize_bilinear_rgba( uint32_t src_width, uint32_t src_height, const uint8_t* src_bitmap,
                                   uint32_t dst_width, uint32_t dst_height, uint8_t* dst_bitmap,
                                   uint32_t byte_count )
{
	register uint32_t x = 0;
	register uint32_t y = 0;
	float horizontalStretchFactor = 0;
	float verticalStretchFactor = 0;
	register uint32_t srcY = 0;
	register uint32_t srcX = 0;
	register uint32_t dstPos,
		 neighborPos1,
		 neighborPos2,
		 neighborPos3,
		 neighborPos4;
	register uint8_t n1R, n1G, n1B, n1A,	/* color vectors */
				  n2R, n2G, n2B, n2A,
				  n3R, n3G, n3B, n3A,
				  n4R, n4G, n4B, n4A;
	uint32_t largestSrcIndex = src_width * src_height * byte_count;

	assert( byte_count == 4 );

	if( !src_bitmap || !dst_bitmap || byte_count != 4 )
	{
		return false;
	}


	horizontalStretchFactor = (float) src_width / (float) dst_width;  /* stretch vector */
	verticalStretchFactor = (float) src_height / (float) dst_height;

	for( y = 0; y < dst_height; y++ )
	{
		srcY = (uint32_t) (y * verticalStretchFactor);
		if( srcY == 0 ) srcY = 1;

		for( x = 0; x < dst_width; x++ )
		{
			srcX = (uint32_t) (x * horizontalStretchFactor);
			dstPos = y * dst_width * byte_count + x * byte_count;

			neighborPos1 = (srcY) * src_width * byte_count + (srcX) * byte_count;
			neighborPos2 = (srcY) * src_width * byte_count + (srcX+1) * byte_count;
			neighborPos3 = (srcY+1) * src_width * byte_count + (srcX) * byte_count;
			neighborPos4 = (srcY+1) * src_width * byte_count + (srcX+1) * byte_count;
			if( neighborPos1 >= largestSrcIndex ) neighborPos1 = largestSrcIndex - byte_count;
			if( neighborPos2 >= largestSrcIndex ) neighborPos2 = largestSrcIndex - byte_count;
			if( neighborPos3 >= largestSrcIndex ) neighborPos3 = largestSrcIndex - byte_count;
			if( neighborPos4 >= largestSrcIndex ) neighborPos4 = largestSrcIndex - byte_count;

			/* source pixel... */
			n1R = src_bitmap[ neighborPos1 ];
			n1G = src_bitmap[ neighborPos1 + 1 ];
			n1B = src_bitmap[ neighborPos1 + 2 ];
			n1A = src_bitmap[ neighborPos1 + 3 ];

			/* source pixel's neighbor2... */
			n2R = src_bitmap[ neighborPos2 ];
			n2G = src_bitmap[ neighborPos2 + 1 ];
			n2B = src_bitmap[ neighborPos2 + 2 ];
			n2A = src_bitmap[ neighborPos2 + 3 ];

			/* source pixel's neighbor3... */
			n3R = src_bitmap[ neighborPos3 ];
			n3G = src_bitmap[ neighborPos3 + 1 ];
			n3B = src_bitmap[ neighborPos3 + 2 ];
			n3A = src_bitmap[ neighborPos3 + 3 ];

			/* source pixel's neighbor4... */
			n4R = src_bitmap[ neighborPos4 ];
			n4G = src_bitmap[ neighborPos4 + 1 ];
			n4B = src_bitmap[ neighborPos4 + 2 ];
			n4A = src_bitmap[ neighborPos4 + 3 ];

			dst_bitmap[ dstPos ] = (uint8_t) bilerp( 0.5, 0.5, n4R, n3R, n2R, n1R );
			dst_bitmap[ dstPos + 1 ] = (uint8_t) bilerp( 0.5, 0.5, n4G, n3G, n2G, n1G );
			dst_bitmap[ dstPos + 2 ] = (uint8_t) bilerp( 0.5, 0.5, n4B, n3B, n2B, n1B );
			dst_bitmap[ dstPos + 3 ] = (uint8_t) bilerp( 0.5, 0.5, n4A, n3A, n2A, n1A );
		}
	}

	return true;
}

bool imageio_resize_bilinear_sharper_rgba( uint32_t src_width, uint32_t src_height, const uint8_t* src_bitmap,
									uint32_t dst_width, uint32_t dst_height, uint8_t* dst_bitmap,
									uint32_t byte_count )
{
	register uint32_t x = 0;
	register uint32_t y = 0;
	float horizontalStretchFactor = 0;
	float verticalStretchFactor = 0;
	register uint32_t srcY = 0;
	register uint32_t srcX = 0;
	register uint32_t dstPos, srcPos;
	register int32_t neighborPos1,
		 neighborPos2,
		 neighborPos3,
		 neighborPos4;
	register uint8_t R, G, B, A,
			 n1R, n1G, n1B, n1A,	/* color vectors */
			 n2R, n2G, n2B, n2A,
			 n3R, n3G, n3B, n3A,
			 n4R, n4G, n4B, n4A;
	uint32_t largestSrcIndex = src_width * src_height * byte_count;

	assert( byte_count == 4 );

	if( !src_bitmap || !dst_bitmap || byte_count != 4 )
	{
		return false;
	}

	horizontalStretchFactor = (float) src_width / (float) dst_width;  /* stretch vector */
	verticalStretchFactor = (float) src_height / (float) dst_height;

	for( y = 0; y < dst_height; y++ )
	{
		srcY = (uint32_t) (y * verticalStretchFactor);
		if( srcY == 0 ) srcY = 1;

		for( x = 0; x < dst_width; x++ )
		{
			srcX = (uint32_t) (x * horizontalStretchFactor);
			dstPos = y * dst_width * byte_count + x * byte_count;

			neighborPos1 = (srcY-1) * src_width * byte_count + (srcX-1) * byte_count;
			neighborPos2 = (srcY-1) * src_width * byte_count + (srcX+1) * byte_count;
			neighborPos3 = (srcY+1) * src_width * byte_count + (srcX-1) * byte_count;
			neighborPos4 = (srcY+1) * src_width * byte_count + (srcX+1) * byte_count;
			srcPos = (uint32_t) srcY * src_width * byte_count + srcX * byte_count;

			if( neighborPos1 < 0 ) neighborPos1 = 0;
			if( neighborPos2 < 0 ) neighborPos2 = 0;
			if( neighborPos3 < 0 ) neighborPos3 = 0;
			if( neighborPos4 < 0 ) neighborPos4 = 0;
			if( neighborPos1 >= largestSrcIndex ) neighborPos1 = largestSrcIndex - byte_count;
			if( neighborPos2 >= largestSrcIndex ) neighborPos2 = largestSrcIndex - byte_count;
			if( neighborPos3 >= largestSrcIndex ) neighborPos3 = largestSrcIndex - byte_count;
			if( neighborPos4 >= largestSrcIndex ) neighborPos4 = largestSrcIndex - byte_count;

			/* source pixel... */
			R = src_bitmap[ srcPos ];
			G = src_bitmap[ srcPos + 1 ];
			B = src_bitmap[ srcPos + 2 ];
			A = src_bitmap[ srcPos + 3 ];

			/* source pixel's neighbor1... */
			n1R = src_bitmap[ neighborPos1 ];
			n1G = src_bitmap[ neighborPos1 + 1 ];
			n1B = src_bitmap[ neighborPos1 + 2 ];
			n1A = src_bitmap[ neighborPos1 + 3 ];

			/* source pixel's neighbor2... */
			n2R = src_bitmap[ neighborPos2 ];
			n2G = src_bitmap[ neighborPos2 + 1 ];
			n2B = src_bitmap[ neighborPos2 + 2 ];
			n2A = src_bitmap[ neighborPos2 + 3 ];

			/* source pixel's neighbor3... */
			n3R = src_bitmap[ neighborPos3 ];
			n3G = src_bitmap[ neighborPos3 + 1 ];
			n3B = src_bitmap[ neighborPos3 + 2 ];
			n3A = src_bitmap[ neighborPos3 + 3 ];

			/* source pixel's neighbor4... */
			n4R = src_bitmap[ neighborPos4 ];
			n4G = src_bitmap[ neighborPos4 + 1 ];
			n4B = src_bitmap[ neighborPos4 + 2 ];
			n4A = src_bitmap[ neighborPos4 + 3 ];

			/* comes out a little more burry than I expected, so I lerp between R and the
			 * output from the bilerp of the other 4 samples.
			 */
			dst_bitmap[ dstPos ] = (uint8_t) lerp( 0.5, bilerp( 0.5, 0.5, n4R, n3R, n2R, n1R ), R );
			dst_bitmap[ dstPos + 1 ] = (uint8_t) lerp( 0.5, bilerp( 0.5, 0.5, n4G, n3G, n2G, n1G ), G );
			dst_bitmap[ dstPos + 2 ] = (uint8_t) lerp( 0.5, bilerp( 0.5, 0.5, n4B, n3B, n2B, n1B ), B );
			dst_bitmap[ dstPos + 3 ] = (uint8_t) lerp( 0.5, bilerp( 0.5, 0.5, n4A, n3A, n2A, n1A ), A );
		}
	}

	return true;
}

bool imageio_resize_bilinear_rgb( uint32_t src_width, uint32_t src_height, const uint8_t* src_bitmap,
				  uint32_t dst_width, uint32_t dst_height, uint8_t* dst_bitmap,
				  uint32_t byte_count )
{
	register uint32_t x = 0;
	register uint32_t y = 0;
	float horizontalStretchFactor = 0;
	float verticalStretchFactor = 0;
	register uint32_t srcY = 0;
	register uint32_t srcX = 0;
	register uint32_t dstPos,
                  neighborPos1,
                  neighborPos2,
                  neighborPos3,
                  neighborPos4;
	register uint8_t n1R, n1G, n1B, 	/* color vectors */
                  n2R, n2G, n2B,
                  n3R, n3G, n3B,
                  n4R, n4G, n4B;

	assert( byte_count == 3 );

	if( !src_bitmap || !dst_bitmap || byte_count != 3 )
	{
		return false;
	}

	horizontalStretchFactor = (float) src_width / (float) dst_width;  /* stretch vector */
	verticalStretchFactor = (float) src_height / (float) dst_height;


	for( y = 0; y < dst_height; y++ )
	{
		srcY = (uint32_t) (y * verticalStretchFactor);
		assert( srcY < src_height );

		for( x = 0; x < dst_width; x++ )
		{
			srcX = (uint32_t) (x * horizontalStretchFactor);
			dstPos = y * dst_width * byte_count + x * byte_count;

			neighborPos1 = (srcY) * src_width * byte_count + (srcX) * byte_count;
			neighborPos2 = (srcY) * src_width * byte_count + (srcX+1) * byte_count;
			neighborPos3 = (srcY+1) * src_width * byte_count + (srcX) * byte_count;
			neighborPos4 = (srcY+1) * src_width * byte_count + (srcX+1) * byte_count;

			/* source pixel... */
			n1R = src_bitmap[ neighborPos1 ];
			n1G = src_bitmap[ neighborPos1 + 1 ];
			n1B = src_bitmap[ neighborPos1 + 2 ];

			/* source pixel's neighbor2... */
			n2R = src_bitmap[ neighborPos2 ];
			n2G = src_bitmap[ neighborPos2 + 1 ];
			n2B = src_bitmap[ neighborPos2 + 2 ];

			/* source pixel's neighbor3... */
			n3R = src_bitmap[ neighborPos3 ];
			n3G = src_bitmap[ neighborPos3 + 1 ];
			n3B = src_bitmap[ neighborPos3 + 2 ];

			/* source pixel's neighbor4... */
			n4R = src_bitmap[ neighborPos4 ];
			n4G = src_bitmap[ neighborPos4 + 1 ];
			n4B = src_bitmap[ neighborPos4 + 2 ];

			dst_bitmap[ dstPos ]     = (uint8_t) bilerp( 0.5, 0.5, n4R, n3R, n2R, n1R );
			dst_bitmap[ dstPos + 1 ] = (uint8_t) bilerp( 0.5, 0.5, n4G, n3G, n2G, n1G );
			dst_bitmap[ dstPos + 2 ] = (uint8_t) bilerp( 0.5, 0.5, n4B, n3B, n2B, n1B );
		}
	}

	return true;
}


bool imageio_resize_bilinear_sharper_rgb( uint32_t src_width, uint32_t src_height, const uint8_t* src_bitmap,
								   uint32_t dst_width, uint32_t dst_height, uint8_t* dst_bitmap,
								   uint32_t byte_count )
{
	register uint32_t x = 0;
	register uint32_t y = 0;
	float horizontalStretchFactor = 0;
	float verticalStretchFactor = 0;
	register uint32_t srcY = 0;
	register uint32_t srcX = 0;
	register uint32_t dstPos, srcPos;
	register int32_t neighborPos1,
		 neighborPos2,
		 neighborPos3,
		 neighborPos4;
	register uint8_t R, G, B,
			 n1R, n1G, n1B, 	/* color vectors */
			 n2R, n2G, n2B,
			 n3R, n3G, n3B,
			 n4R, n4G, n4B;
	uint32_t largestSrcIndex = src_width * src_height * byte_count;

	assert( byte_count == 3 );

	if( !src_bitmap || !dst_bitmap || byte_count != 3 )
	{
		return false;
	}

	horizontalStretchFactor = (float) src_width / (float) dst_width;  /* stretch vector */
	verticalStretchFactor   = (float) src_height / (float) dst_height;

	for( y = 0; y < dst_height; y++ )
	{
		srcY = (uint32_t) (y * verticalStretchFactor);
		assert( srcY < src_height );

		for( x = 0; x < dst_width; x++ )
		{
			srcX = (uint32_t) (x * horizontalStretchFactor);
			dstPos = y * dst_width * byte_count + x * byte_count;

			neighborPos1 = (srcY-1) * src_width * byte_count + (srcX-1) * byte_count;
			neighborPos2 = (srcY-1) * src_width * byte_count + (srcX+1) * byte_count;
			neighborPos3 = (srcY+1) * src_width * byte_count + (srcX-1) * byte_count;
			neighborPos4 = (srcY+1) * src_width * byte_count + (srcX+1) * byte_count;
			srcPos = (uint32_t) srcY * src_width * byte_count + srcX * byte_count;

			if( neighborPos1 < 0 ) neighborPos1 = 0;
			if( neighborPos2 < 0 ) neighborPos2 = 0;
			if( neighborPos3 < 0 ) neighborPos3 = 0;
			if( neighborPos4 < 0 ) neighborPos4 = 0;
			if( neighborPos1 >= largestSrcIndex ) neighborPos1 = largestSrcIndex - byte_count;
			if( neighborPos2 >= largestSrcIndex ) neighborPos2 = largestSrcIndex - byte_count;
			if( neighborPos3 >= largestSrcIndex ) neighborPos3 = largestSrcIndex - byte_count;
			if( neighborPos4 >= largestSrcIndex ) neighborPos4 = largestSrcIndex - byte_count;

			/* source pixel... */
			R = src_bitmap[ srcPos ];
			G = src_bitmap[ srcPos + 1 ];
			B = src_bitmap[ srcPos + 2 ];

			/* source pixel's neighbor1... */
			n1R = src_bitmap[ neighborPos1 ];
			n1G = src_bitmap[ neighborPos1 + 1 ];
			n1B = src_bitmap[ neighborPos1 + 2 ];

			/* source pixel's neighbor2... */
			n2R = src_bitmap[ neighborPos2 ];
			n2G = src_bitmap[ neighborPos2 + 1 ];
			n2B = src_bitmap[ neighborPos2 + 2 ];

			/* source pixel's neighbor3... */
			n3R = src_bitmap[ neighborPos3 ];
			n3G = src_bitmap[ neighborPos3 + 1 ];
			n3B = src_bitmap[ neighborPos3 + 2 ];

			/* source pixel's neighbor4... */
			n4R = src_bitmap[ neighborPos4 ];
			n4G = src_bitmap[ neighborPos4 + 1 ];
			n4B = src_bitmap[ neighborPos4 + 2 ];

			/* comes out a little more burry than I expected, so I lerp between R and the
			 * output from the bilerp of the other 4 samples.
			 */
			dst_bitmap[ dstPos ]     = (uint8_t) lerp( 0.5, bilerp( 0.5, 0.5, n4R, n3R, n2R, n1R ), R );
			dst_bitmap[ dstPos + 1 ] = (uint8_t) lerp( 0.5, bilerp( 0.5, 0.5, n4G, n3G, n2G, n1G ), G );
			dst_bitmap[ dstPos + 2 ] = (uint8_t) lerp( 0.5, bilerp( 0.5, 0.5, n4B, n3B, n2B, n1B ), B );
		}
	}

	return true;
}

#define P(x)	( (x) > 0 ? (x) : 0 )
#define R(x)	( (1.0/6) * (P((x)+2) * P((x)+2) * P((x)+2) - 4.0 * P((x)+1) * P((x)+1) * P((x)+1) + 6.0 * P((x)) * P((x)) * P(x) - 4.0 * P((x)-1) * P((x)-1) * P((x)-1)) )

bool imageio_resize_bicubic_rgba( uint32_t src_width, uint32_t src_height, const uint8_t* src_bitmap,
							 uint32_t dst_width, uint32_t dst_height, uint8_t* dst_bitmap,
							 uint32_t byte_count )
{
	register uint32_t x = 0;
	register uint32_t y = 0;
	register int m = 0, n = 0, i = 0, j = 0;
	float horizontalStretchFactor = 0;
	float verticalStretchFactor = 0;
	register float srcY = 0;
	register float srcX = 0;
	register float dy = 0;
	register float dx = 0;
	register size_t dstPos, srcPos;
	uint32_t largestSrcIndex = src_width * src_height * byte_count;
	register uint32_t sumR = 0;
	register uint32_t sumG = 0;
	register uint32_t sumB = 0;
	register uint32_t sumA = 0;

	assert( byte_count == 4 );

	if( !src_bitmap || !dst_bitmap || byte_count != 4 )
	{
		return false;
	}

	horizontalStretchFactor = (float) src_width / (float) dst_width;  /* stretch vector */
	verticalStretchFactor = (float) src_height / (float) dst_height;

	for( y = 0; y < dst_height; y++ )
	{
		srcY = y * verticalStretchFactor;
		j = (int) floor( srcY );
		if( srcY == 0 ) srcY = 1;


		for( x = 0; x < dst_width; x++ )
		{
			srcX = x * horizontalStretchFactor;
			i = (int) floor( srcX );

			dstPos = y * dst_width * byte_count + x * byte_count;
			sumR = 0;
			sumG = 0;
			sumB = 0;
			sumA = 0;
			dy = srcY - j;
			dx = srcX - i;

			for( m = -1; m <= 2; m++ )
				for( n = -1; n <= 2; n++ )
				{
					srcPos = pixel_index( i + m, j + n, byte_count, src_width );
					if( srcPos > largestSrcIndex ) srcPos = largestSrcIndex - byte_count;

					sumR +=	(uint32_t) src_bitmap[ srcPos ] * R( m - dx ) * R( dy - n );
					sumG +=	(uint32_t) src_bitmap[ srcPos + 1 ] * R( m - dx ) * R( dy - n );
					sumB +=	(uint32_t) src_bitmap[ srcPos + 2 ] * R( m - dx ) * R( dy - n );
					sumA +=	(uint32_t) src_bitmap[ srcPos + 3 ] * R( m - dx ) * R( dy - n );
				}

			dst_bitmap[ dstPos ]     = (uint8_t) sumR;
			dst_bitmap[ dstPos + 1 ] = (uint8_t) sumG;
			dst_bitmap[ dstPos + 2 ] = (uint8_t) sumB;
			dst_bitmap[ dstPos + 3 ] = (uint8_t) sumA;
		}
	}

	return true;
}

bool imageio_resize_bicubic_rgb( uint32_t src_width, uint32_t src_height, const uint8_t* src_bitmap,
							uint32_t dst_width, uint32_t dst_height, uint8_t* dst_bitmap,
							uint32_t byte_count )
{
	register uint32_t x = 0;
	register uint32_t y = 0;
	register int m = 0, n = 0, i = 0, j = 0;
	float horizontalStretchFactor = 0;
	float verticalStretchFactor = 0;
	register float srcY = 0;
	register float srcX = 0;
	register float dy = 0;
	register float dx = 0;
	register size_t dstPos, srcPos;
	uint32_t largestSrcIndex = src_width * src_height * byte_count;
	register uint32_t sumR = 0;
	register uint32_t sumG = 0;
	register uint32_t sumB = 0;
	assert( byte_count == 4 );

	if( !src_bitmap || !dst_bitmap || byte_count != 4 )
	{
		return false;
	}

	horizontalStretchFactor = (float) src_width / (float) dst_width;  /* stretch vector */
	verticalStretchFactor = (float) src_height / (float) dst_height;

	for( y = 0; y < dst_height; y++ )
	{
		srcY = y * verticalStretchFactor;
		j = (int) floor( srcY );
		if( srcY == 0 ) srcY = 1;


		for( x = 0; x < dst_width - 2; x++ )
		{
			srcX = x * horizontalStretchFactor;
			i    = (int) floor( srcX );

			dstPos = y * dst_width * byte_count + x * byte_count;
			sumR   = 0;
			sumG   = 0;
			sumB   = 0;
			dy     = srcY - j;
			dx     = srcX - i;

			for( m = -1; m <= 2; m++ )
			{
				for( n = -1; n <= 2; n++ )
				{
					srcPos = pixel_index( i + m, j + n, byte_count, src_width );
					if( srcPos > largestSrcIndex ) srcPos = largestSrcIndex - byte_count;

					sumR +=	(uint32_t) src_bitmap[ srcPos ] * R( m - dx ) * R( dy - n );
					sumG +=	(uint32_t) src_bitmap[ srcPos + 1 ] * R( m - dx ) * R( dy - n );
					sumB +=	(uint32_t) src_bitmap[ srcPos + 2 ] * R( m - dx ) * R( dy - n );
				}
			}

			dst_bitmap[ dstPos ]     = (uint8_t) sumR;
			dst_bitmap[ dstPos + 1 ] = (uint8_t) sumG;
			dst_bitmap[ dstPos + 2 ] = (uint8_t) sumB;
		}
	}

	return true;
}

bool imageio_blit( uint32_t pos_x, uint32_t pos_y,
                   uint32_t dst_width, uint32_t dst_height, uint32_t dst_bytes_per_pixel, uint8_t* dst_pixels,
                   uint32_t src_width, uint32_t src_height, uint32_t src_bytes_per_pixel, uint8_t* src_pixels )
{
	if( dst_bytes_per_pixel < src_bytes_per_pixel )
	{
		return false;
	}

	size_t last_y = pos_y + src_height;
	size_t last_x = pos_x + src_width;

	for( size_t y = pos_y; y < last_y; y++ )
	{
		for( size_t x = pos_x; x < last_x; x++ )
		{
			size_t dst_index = (y * dst_width + x) * dst_bytes_per_pixel;
			size_t src_index = ((y - pos_y) * src_width + (x - pos_x)) * src_bytes_per_pixel;

			memcpy( &dst_pixels[ dst_index ], &src_pixels[ src_index ], src_bytes_per_pixel );
		}
	}

	return true;
}

bool imageio_blend( image_t* dst, uint32_t pos_x, uint32_t pos_y, const image_t* src, blend_mode_t mode )
{
	if( dst->channels < src->channels )
	{
		return false;
	}

	void (*blender)( uint8_t* result, uint8_t* a, uint8_t* b, blend_mode_t mode );

	if( src->channels == 4 )
	{
		blender = imageio_blend_rgba;
	}
	else
	{
		blender = imageio_blend_rgb;
	}

	size_t last_y = pos_y + src->height;
	size_t last_x = pos_x + src->width;

	for( size_t y = pos_y; y < last_y; y++ )
	{
		for( size_t x = pos_x; x < last_x; x++ )
		{
			size_t dst_index = (y * dst->width + x) * dst->channels;
			size_t src_index = ((y - pos_y) * src->width + (x - pos_x)) * src->channels;
			blender( &dst->pixels[dst_index], &src->pixels[src_index], &dst->pixels[dst_index], mode );
		}
	}

	return true;
}

void imageio_blend_rgb( uint8_t* result, uint8_t* top, uint8_t* bottom, blend_mode_t mode )
{
	switch( mode )
	{
		case IMAGEIO_BLEND_LIGHTEN:
			colorblend_rgb_lighten( result, top, bottom );
			break;
		case IMAGEIO_BLEND_DARKEN:
			colorblend_rgb_darken( result, top, bottom );
			break;
		case IMAGEIO_BLEND_MULTIPLY:
			colorblend_rgb_multiply( result, top, bottom );
			break;
		case IMAGEIO_BLEND_AVERAGE:
			colorblend_rgb_average( result, top, bottom );
			break;
		case IMAGEIO_BLEND_ADD:
			colorblend_rgb_add( result, top, bottom );
			break;
		case IMAGEIO_BLEND_SUBTRACT:
			colorblend_rgb_subtract( result, top, bottom );
			break;
		case IMAGEIO_BLEND_DIFFERENCE:
			colorblend_rgb_difference( result, top, bottom );
			break;
		case IMAGEIO_BLEND_NEGATION:
			colorblend_rgb_negation( result, top, bottom );
			break;
		case IMAGEIO_BLEND_SCREEN:
			colorblend_rgb_screen( result, top, bottom );
			break;
		case IMAGEIO_BLEND_EXCLUSION:
			colorblend_rgb_exclusion( result, top, bottom );
			break;
		case IMAGEIO_BLEND_OVERLAY:
			colorblend_rgb_overlay( result, top, bottom );
			break;
		case IMAGEIO_BLEND_SOFT_LIGHT:
			colorblend_rgb_softlight( result, top, bottom );
			break;
		case IMAGEIO_BLEND_HARD_LIGHT:
			colorblend_rgb_hardlight( result, top, bottom );
			break;
		case IMAGEIO_BLEND_COLOR_DODGE:
			colorblend_rgb_colordodge( result, top, bottom );
			break;
		case IMAGEIO_BLEND_COLOR_BURN:
			colorblend_rgb_colorburn( result, top, bottom );
			break;
		case IMAGEIO_BLEND_LINEAR_DODGE:
			colorblend_rgb_lineardodge( result, top, bottom );
			break;
		case IMAGEIO_BLEND_LINEAR_BURN:
			colorblend_rgb_linearburn( result, top, bottom );
			break;
		case IMAGEIO_BLEND_LINEAR_LIGHT:
			colorblend_rgb_linearlight( result, top, bottom );
			break;
		case IMAGEIO_BLEND_VIVID_LIGHT:
			colorblend_rgb_vividlight( result, top, bottom );
			break;
		case IMAGEIO_BLEND_PIN_LIGHT:
			colorblend_rgb_pinlight( result, top, bottom );
			break;
		case IMAGEIO_BLEND_HARD_MIX:
			colorblend_rgb_hardmix( result, top, bottom );
			break;
		case IMAGEIO_BLEND_REFLECT:
			colorblend_rgb_reflect( result, top, bottom );
			break;
		case IMAGEIO_BLEND_GLOW:
			colorblend_rgb_glow( result, top, bottom );
			break;
		case IMAGEIO_BLEND_PHOENIX:
			colorblend_rgb_phoenix( result, top, bottom );
			break;
		case IMAGEIO_BLEND_ALPHA: /* not supported so fallback to normal */
		case IMAGEIO_BLEND_NORMAL: /* fall-through */
		default:
			colorblend_rgb_normal( result, top, bottom );
			break;
	}
}

void imageio_blend_rgba( uint8_t* result, uint8_t* top, uint8_t* bottom, blend_mode_t mode )
{
	switch( mode )
	{
		case IMAGEIO_BLEND_LIGHTEN:
			colorblend_rgba_lighten( result, top, bottom );
			break;
		case IMAGEIO_BLEND_DARKEN:
			colorblend_rgba_darken( result, top, bottom );
			break;
		case IMAGEIO_BLEND_MULTIPLY:
			colorblend_rgba_multiply( result, top, bottom );
			break;
		case IMAGEIO_BLEND_AVERAGE:
			colorblend_rgba_average( result, top, bottom );
			break;
		case IMAGEIO_BLEND_ADD:
			colorblend_rgba_add( result, top, bottom );
			break;
		case IMAGEIO_BLEND_SUBTRACT:
			colorblend_rgba_subtract( result, top, bottom );
			break;
		case IMAGEIO_BLEND_DIFFERENCE:
			colorblend_rgba_difference( result, top, bottom );
			break;
		case IMAGEIO_BLEND_NEGATION:
			colorblend_rgba_negation( result, top, bottom );
			break;
		case IMAGEIO_BLEND_SCREEN:
			colorblend_rgba_screen( result, top, bottom );
			break;
		case IMAGEIO_BLEND_EXCLUSION:
			colorblend_rgba_exclusion( result, top, bottom );
			break;
		case IMAGEIO_BLEND_OVERLAY:
			colorblend_rgba_overlay( result, top, bottom );
			break;
		case IMAGEIO_BLEND_SOFT_LIGHT:
			colorblend_rgba_softlight( result, top, bottom );
			break;
		case IMAGEIO_BLEND_HARD_LIGHT:
			colorblend_rgba_hardlight( result, top, bottom );
			break;
		case IMAGEIO_BLEND_COLOR_DODGE:
			colorblend_rgba_colordodge( result, top, bottom );
			break;
		case IMAGEIO_BLEND_COLOR_BURN:
			colorblend_rgba_colorburn( result, top, bottom );
			break;
		case IMAGEIO_BLEND_LINEAR_DODGE:
			colorblend_rgba_lineardodge( result, top, bottom );
			break;
		case IMAGEIO_BLEND_LINEAR_BURN:
			colorblend_rgba_linearburn( result, top, bottom );
			break;
		case IMAGEIO_BLEND_LINEAR_LIGHT:
			colorblend_rgba_linearlight( result, top, bottom );
			break;
		case IMAGEIO_BLEND_VIVID_LIGHT:
			colorblend_rgba_vividlight( result, top, bottom );
			break;
		case IMAGEIO_BLEND_PIN_LIGHT:
			colorblend_rgba_pinlight( result, top, bottom );
			break;
		case IMAGEIO_BLEND_HARD_MIX:
			colorblend_rgba_hardmix( result, top, bottom );
			break;
		case IMAGEIO_BLEND_REFLECT:
			colorblend_rgba_reflect( result, top, bottom );
			break;
		case IMAGEIO_BLEND_GLOW:
			colorblend_rgba_glow( result, top, bottom );
			break;
		case IMAGEIO_BLEND_PHOENIX:
			colorblend_rgba_phoenix( result, top, bottom );
			break;
		case IMAGEIO_BLEND_ALPHA:
			colorblend_rgba_alpha( result, top, bottom);
			break;
		case IMAGEIO_BLEND_NORMAL: /* fall-through */
		default:
			colorblend_rgba_normal( result, top, bottom );
			break;
	}
}

/*
 *	Swap Red and blue colors in RGB abd RGBA functions
 */
void imageio_swap_red_and_blue( uint32_t width, uint32_t height, uint32_t byte_count, uint8_t* bitmap ) /* RGB to BGR */
{
	register uint32_t imageIdx;
	register uint32_t imageSize = width * height * byte_count;
	assert( byte_count != 0 );
	assert( bitmap != NULL );

	if( byte_count > 2 ) /* 32 bpp or 24 bpp */
	{
		for( imageIdx = 0; imageIdx < imageSize; imageIdx += byte_count )
		{
			/* fast swap using XOR... */
			bitmap[ imageIdx ]     = bitmap[ imageIdx ] ^ bitmap[ imageIdx + 2 ];
			bitmap[ imageIdx + 2 ] = bitmap[ imageIdx + 2 ] ^ bitmap[ imageIdx ];
			bitmap[ imageIdx ]     = bitmap[ imageIdx ] ^ bitmap[ imageIdx + 2 ];
		}
	}
	else { /* 16 bpp */
		/* Swap ARRRRRGGGGGBBBBB to GGGBBBBBARRRRRGG, whereeach R,G,B, A is a bit, or... */
		/* Swap GGGBBBBBARRRRRGG to ARRRRRGGGGGBBBBB, whereeach R,G,B, A is a bit */
		for( imageIdx = 0; imageIdx < imageSize; imageIdx += byte_count )
		{
			bitmap[ imageIdx ]     = bitmap[ imageIdx + 1 ];
			bitmap[ imageIdx + 1 ] = bitmap[ imageIdx ];
		}
	}

}

/*
 *  Image Flipping Routines. These are slow... :(
 */
void imageio_flip_horizontally( uint32_t width, uint32_t height, uint32_t byte_count, uint8_t* bitmap )
{
	register uint32_t x = 0;
	register uint32_t y = 0;
	uint8_t* src_bitmap = (uint8_t*) malloc( sizeof(uint8_t) * width * height * byte_count );

	memcpy( src_bitmap, bitmap, sizeof(uint8_t) * width * height * byte_count );

	for( y = 0; y < height; y++ )
		for( x = 0; x < width; x++ )
			memcpy( &bitmap[ pixel_index(width - x - 1, y, byte_count, width) ], &src_bitmap[ pixel_index(x, y, byte_count, width) ], sizeof(uint8_t) * byte_count );

	free( src_bitmap );
}

void imageio_flip_vertically( uint32_t width, uint32_t height, uint32_t byte_count, uint8_t* bitmap )
{
	uint8_t* src_bitmap = (uint8_t*) malloc( sizeof(uint8_t) * width * height * byte_count );

	memcpy( src_bitmap, bitmap, sizeof(uint8_t) * width * height * byte_count );

	#if 1
	register uint32_t y = 0;
	for( y = 0; y < height; y++ )
		memcpy( &bitmap[ pixel_index(0, height - y - 1, byte_count, width) ], &src_bitmap[ pixel_index(0, y, byte_count, width) ], sizeof(uint8_t) * byte_count * width );
	#else
	register uint32_t x = 0;
	register uint32_t y = 0;
	for( y = 0; y < height; y++ )
		for( x = 0; x < width; x++ )
			memcpy( &bitmap[ pixel_index(x, height - y - 1, byte_count, width) ], &src_bitmap[ pixel_index(x, y, byte_count, width) ], sizeof(uint8_t) * byte_count * width );
	#endif

	free( src_bitmap );
}

void imageio_flip_horizontally_nocopy( uint32_t width, uint32_t height, const uint8_t* src_bitmap, uint8_t* dst_bitmap, uint32_t byte_count )
{
	register uint32_t j, i;
	register uint32_t j_times_width = 0;
	register uint8_t* temp = (uint8_t*) malloc( width * byte_count * sizeof(uint8_t) ); /* temp scan line, just in case src = dst */

	for( j = 0; j < height; j++ )
	{
		j_times_width = j* width * byte_count; /* avoids doing this twice */

		memcpy( &temp[ 0 ], &src_bitmap[ j_times_width ], width * byte_count * sizeof(uint8_t) );

		for( i = 0; i < width; i++ )
			memcpy( &dst_bitmap[ j_times_width + (width - 1 - i) * byte_count ], &temp[ i * byte_count ], byte_count * sizeof(uint8_t) );
	}

	free( temp );
}

void imageio_flip_vertically_nocopy( uint32_t width, uint32_t height, const uint8_t* src_bitmap, uint8_t* dst_bitmap, uint32_t byte_count )
{
	register uint32_t j, i;
	register uint32_t i_times_bytecount = 0;
	register uint32_t width_times_bytecount = 0;
	register uint8_t* temp = (uint8_t*) malloc( height * byte_count * sizeof(uint8_t) ); /* temp column line, just in case src = dst */

	for( i = 0; i < width; i++ )
	{
		i_times_bytecount = i * byte_count;
		width_times_bytecount = width * byte_count;

		for( j = 0; j < height; j++ )
			memcpy( &temp[ j * byte_count ], &src_bitmap[ j * width_times_bytecount + i_times_bytecount ], byte_count * sizeof(uint8_t) );

		for( j = 0; j < height; j++ )
			memcpy( &dst_bitmap[ (height - 1 - j) * width_times_bytecount + i_times_bytecount ], &temp[ j * byte_count ], byte_count * sizeof(uint8_t) );
	}

	free( temp );
}



/*
 * Edge Detection: k is the maximum color distance that signifies an edge
 */
bool imageio_detect_edges( uint32_t width, uint32_t height, uint32_t bit_depth, const uint8_t* src_bitmap, uint8_t* dst_bitmap, int32_t k )
{
	uint32_t byte_count = bit_depth >> 3; /* 4 ==> RGBA32, 3==>RGB32 , 2 ==> RGB16 */
	register uint32_t y;
	register uint32_t x;
	assert( bit_depth != 0 );
	assert( src_bitmap != NULL || dst_bitmap != NULL );

	if( !src_bitmap || !dst_bitmap || bit_depth == 0 )
	{
		return false;
	}

	for( y = 0; y < height - 1; y++ )
	{
		for( x = 0; x < width - 1; x++ )
		{
			/* r = right, b = bottom */
			uint32_t pos = width * y * byte_count + x * byte_count;
			uint32_t rpos = width * y * byte_count + (x + 1) * byte_count;
			uint32_t bpos = width * (y + 1) * byte_count + x * byte_count;

			/* this pixel */
			uint8_t R = src_bitmap[ pos ];
			uint8_t G = src_bitmap[ pos + 1 ];
			uint8_t B = src_bitmap[ pos + 2 ];

			/* right neighbor */
			uint8_t rR = src_bitmap[ rpos ];
			uint8_t rG = src_bitmap[ rpos + 1 ];
			uint8_t rB = src_bitmap[ rpos + 2 ];

			/* bottom neighbor */
			uint8_t bR = src_bitmap[ bpos ];
			uint8_t bG = src_bitmap[ bpos + 1 ];
			uint8_t bB = src_bitmap[ bpos + 2 ];

			/*
			uint32_t D1 = sqrt( (R-rR)*(R-rR) + (G-rG)*(G-rG) + (B-rB)*(B-rB) );
			uint32_t D2 = sqrt( (R-bR)*(R-bR) + (G-bG)*(G-bG) + (B-bB)*(B-bB) );
			*/

			/* if any of the distances are greater than k, put a white pixel in the destination */
			if( (R-rR)*(R-rR) + (G-rG)*(G-rG) + (B-rB)*(B-rB) >= k*k ||
				(R-bR)*(R-bR) + (G-bG)*(G-bG) + (B-bB)*(B-bB) >= k*k ) /* if( D1 > k || D2 > k ) */
			{
				dst_bitmap[ pos + 0 ] = 0xFF;
				dst_bitmap[ pos + 1 ] = 0xFF;
				dst_bitmap[ pos + 2 ] = 0xFF;
			}
			else /* otherwise place a black pixel; */
			{
				dst_bitmap[ pos + 0 ] = 0x00;
				dst_bitmap[ pos + 1 ] = 0x00;
				dst_bitmap[ pos + 2 ] = 0x00;
			}
		}
	}

	return true;
}

/*
 * Color Extraction: Marks white all the pixels that are no greater than k distance to the color.
 */
bool imageio_extract_color( uint32_t width, uint32_t height, uint32_t bit_depth, const uint8_t* src_bitmap, uint8_t* dst_bitmap, uint32_t color, uint32_t k )
{
	uint32_t byte_count = bit_depth >> 3; /* 4 ==> RGBA32, 3==>RGB32 , 2 ==> RGB16 */
	register uint32_t y;
	register uint32_t x;
	assert( bit_depth != 0 );
	assert( src_bitmap != NULL || dst_bitmap != NULL );

	if( !src_bitmap || !dst_bitmap || bit_depth == 0 )
	{
		return false;
	}

	for( y = 0; y < height; y++ )
	{
		for( x = 0; x < width; x++ )
		{
			uint32_t pos = width * y * byte_count + x * byte_count;
			/* this pixel */
			uint8_t r_diff = src_bitmap[ pos + 0 ] - r32(color);
			uint8_t g_diff = src_bitmap[ pos + 1 ] - g32(color);
			uint8_t b_diff = src_bitmap[ pos + 2 ] - b32(color);

			if( r_diff * r_diff + g_diff * g_diff + b_diff * b_diff <= k*k ) /*if( sqrt( (R-color->r) + (G-color->g) + (B-color->b) ) <= k )*/
			{
				dst_bitmap[ pos + 0 ] = 0xFF;
				dst_bitmap[ pos + 1 ] = 0xFF;
				dst_bitmap[ pos + 2 ] = 0xFF;
			}
			else {
				dst_bitmap[ pos + 0 ] = 0x00;
				dst_bitmap[ pos + 1 ] = 0x00;
				dst_bitmap[ pos + 2 ] = 0x00;
			}
		}
	}

	return true;
}

/*
 * Grayscale conversion
 */
bool imageio_convert_to_grayscale( uint32_t width, uint32_t height, uint32_t bit_depth, const uint8_t* src_bitmap, uint8_t* dst_bitmap )
{
	uint32_t byte_count = bit_depth >> 3; /* 4 ==> RGBA32, 3==>RGB32 , 2 ==> RGB16 */
	register uint32_t y;
	register uint32_t x;
	assert( bit_depth != 0 );
	assert( src_bitmap != NULL || dst_bitmap != NULL );

	if( !src_bitmap || !dst_bitmap || bit_depth == 0 )
	{
		return false;
	}

	for( y = 0; y < height; y++ )
	{
		for( x = 0; x < width; x++ )
		{
			uint32_t pos = width * y * byte_count + x * byte_count;
			/* this pixel */
			uint8_t R = src_bitmap[ pos ];
			uint8_t G = src_bitmap[ pos + 1 ];
			uint8_t B = src_bitmap[ pos + 2 ];

			uint32_t colorAverage = (R + G + B) / 3;

			dst_bitmap[ pos ] = colorAverage;
			dst_bitmap[ pos + 1 ] = colorAverage;
			dst_bitmap[ pos + 2 ] = colorAverage;
		}
	}

	return true;
}

/*
 * Colorscale conversion
 */
bool imageio_convert_to_colorscale( uint32_t width, uint32_t height, uint32_t bit_depth, const uint8_t* src_bitmap, uint8_t* dst_bitmap, uint32_t color )
{
	uint32_t byte_count = bit_depth >> 3; /* 4 ==> RGBA32, 3==>RGB32 , 2 ==> RGB16 */
	register uint32_t y;
	register uint32_t x;
	uint8_t r = r32(color);
	uint8_t g = g32(color);
	uint8_t b = b32(color);
	assert( src_bitmap != NULL || dst_bitmap != NULL );
	assert( bit_depth != 0 );

	if( !src_bitmap || !dst_bitmap || bit_depth == 0 )
	{
		return false;
	}

	if( r == 0 && g == 0 && b == 0 )
	{
		/* no black, because 0 vector */
		return false;
	}

	for( y = 0; y < height; y++ )
	{
		for( x = 0; x < width; x++ )
		{
			uint32_t pos = width * y * byte_count + x * byte_count;
			/* this pixel */
			uint8_t R = src_bitmap[ pos ];
			uint8_t G = src_bitmap[ pos + 1 ];
			uint8_t B = src_bitmap[ pos + 2 ];

			float colorScaled = (float) ( (R*r + G*g + B*b) / ( sqrt((float) R*R + G*G + B*B) * sqrt((float) r*r + g*g + b*b) ));

			dst_bitmap[ pos ] = (uint8_t) colorScaled * R;
			dst_bitmap[ pos + 1 ] = (uint8_t) colorScaled * G;
			dst_bitmap[ pos + 2 ] = (uint8_t) colorScaled * B;
		}
	}

	return true;
}

/*
 * Light or contrast modification
 */
void imageio_modify_contrast( uint32_t width, uint32_t height, uint32_t bit_depth, const uint8_t* src_bitmap, uint8_t* dst_bitmap, int contrast )
{
	uint32_t byte_count = bit_depth >> 3; /* 4 ==> RGBA32, 3==>RGB32 , 2 ==> RGB16 */
	register uint32_t y;
	register uint32_t x;
	register uint32_t colorIndex;
	uint32_t transform[ 256 ];
	assert( src_bitmap != NULL || dst_bitmap != NULL );
	assert( bit_depth != 0 );

	for( colorIndex = 0; colorIndex < 256; colorIndex++ )
	{
		float slope = (float) tan((float)contrast);
		if( colorIndex < (uint32_t) (128.0f + 128.0f*slope) && colorIndex > (uint32_t) (128.0f-128.0f*slope) )
			transform[ colorIndex ] = (uint32_t) ((colorIndex - 128) / slope + 128);
		else if( colorIndex >= (uint32_t) (128.0f + 128.0f*slope) )
			transform[ colorIndex ] = 255;
		else /* colorIndex <= 128 -128*slope */
			transform[ colorIndex ] = 0;
	}


	for( y = 0; y < height; y++ )
		for( x = 0; x < width; x++ )
		{
			uint32_t pos = width * y * byte_count + x * byte_count;
			/* this pixel */
			uint8_t R = src_bitmap[ pos ];
			uint8_t G = src_bitmap[ pos + 1 ];
			uint8_t B = src_bitmap[ pos + 2 ];

			dst_bitmap[ pos ] = transform[ R ];
			dst_bitmap[ pos + 1 ] = transform[ G ];
			dst_bitmap[ pos + 2 ] = transform[ B ];
		}
}

void imageio_modify_brightness( uint32_t width, uint32_t height, uint32_t bit_depth, const uint8_t* src_bitmap, uint8_t* dst_bitmap, int brightness )
{
	uint32_t byte_count = bit_depth >> 3; /* 4 ==> RGBA32, 3==>RGB32 , 2 ==> RGB16 */
	register uint32_t y;
	register uint32_t x;
	register unsigned short colorIndex;
	uint8_t transform[ 256 ] = {0};
	assert( src_bitmap != NULL || dst_bitmap != NULL );
	assert( bit_depth != 0 );

	for( colorIndex = 0; colorIndex < 256; colorIndex++ )
	{
		short t = colorIndex + brightness;
		if( t > 255 ) t = 255;
		if( t < 0 )	t = 0;
		transform[ colorIndex ] = (uint8_t) t;
	}


	for( y = 0; y < height; y++ )
	{
		for( x = 0; x < width; x++ )
		{
			uint32_t pos = width * y * byte_count + x * byte_count;
			/* this pixel */
			uint8_t R = src_bitmap[ pos ];
			uint8_t G = src_bitmap[ pos + 1 ];
			uint8_t B = src_bitmap[ pos + 2 ];

			dst_bitmap[ pos ] = transform[ R ];
			dst_bitmap[ pos + 1 ] = transform[ G ];
			dst_bitmap[ pos + 2 ] = transform[ B ];
		}
	}
}



/*
 * Y = 0.299R + 0.587G + 0.114B
 * U'= (B-Y)*0.565
 * V'= (R-Y)*0.713
 */
void imageio_rgb_to_yuv444( uint32_t width, uint32_t height, uint32_t byte_count, uint8_t* bitmap )
{
	register uint32_t imageIdx;
	register uint32_t imageSize = width * height * byte_count;
	assert( byte_count != 0 );
	assert( bitmap != NULL );

	if( byte_count > 2 ) /* 32 bpp or 24 bpp */
	{
		for( imageIdx = 0; imageIdx < imageSize; imageIdx += byte_count )
		{
			uint8_t R = bitmap[ imageIdx + 0 ];
			uint8_t G = bitmap[ imageIdx + 1 ];
			uint8_t B = bitmap[ imageIdx + 2 ];

			float Y = (0.299f * R + 0.587f * G + 0.114f * B); /* Y */
			bitmap[ imageIdx + 0 ] = (uint8_t) Y; /* Y */
			bitmap[ imageIdx + 1 ] = (uint8_t) ((B - Y) * 0.565f); /* U' */
			bitmap[ imageIdx + 2 ] = (uint8_t) ((R - Y) * 0.713f); /* V' */
		}
	}
	else
	{
		/* not implemented */
		assert( false );
	}
}

/*
 * R = Y + 1.403V'
 * G = Y - 0.344U' - 0.714V'
 * B = Y + 1.770U'
 */
void imageio_yuv444_to_rgb( uint32_t width, uint32_t height, uint32_t byte_count, uint8_t* bitmap )
{
	register uint32_t imageIdx;
	register uint32_t imageSize = width * height * byte_count;
	assert( byte_count != 0 );
	assert( bitmap != NULL );

	if( byte_count > 2 ) /* 32 bpp or 24 bpp */
	{
		for( imageIdx = 0; imageIdx < imageSize; imageIdx += byte_count )
		{
			uint8_t Y = bitmap[ imageIdx + 0 ];
			uint8_t U = bitmap[ imageIdx + 1 ];
			uint8_t V = bitmap[ imageIdx + 2 ];

			bitmap[ imageIdx + 0 ] = (uint8_t) (Y + 1.403f * V); /* R */
			bitmap[ imageIdx + 1 ] = (uint8_t) (Y - 0.344f * U - 0.714f * V); /* G */
			bitmap[ imageIdx + 2 ] = (uint8_t) (Y + 1.770f * U); /* B */
		}
	}
	else
	{
		/* not implemented */
		assert( false );
	}
}

bool imageio_is_opaque( const image_t* img, bool* p_partially_opaque )
{
	bool completely_opaque     = true;
	bool contains_opaque_pixel = false;

	if( img->channels == 4 )
	{
		for( size_t y = 0; y < img->height; y++ )
		{
			for( size_t x = 0; x < img->width; x++ )
			{
				size_t index = (y * img->width + x) * img->channels;
				bool is_pixel_opaque = img->pixels[ index + 3 ] > 0;

				if( !contains_opaque_pixel && is_pixel_opaque )
				{
					contains_opaque_pixel = true;
				}

				if( completely_opaque && !is_pixel_opaque )
				{
					completely_opaque = false;
				}

			}
		}
	}
	else
	{
		completely_opaque     = true;
		contains_opaque_pixel = true;
	}

	if( p_partially_opaque )
	{
		*p_partially_opaque = contains_opaque_pixel;
	}

	return completely_opaque;
}

const char* imageio_image_string( const image_t* img )
{
	static char buffer[ 512 ];
	snprintf( buffer, sizeof(buffer), "[image bpp=%02u channels=%u w=%05u h=%05u data=%02x%02x%02x%02x%02x%02x%02x%02x... ]",
	          img->bit_depth, img->channels, img->width, img->height,
	          img->pixels[0], img->pixels[1], img->pixels[2], img->pixels[3],
	          img->pixels[4], img->pixels[5], img->pixels[6], img->pixels[7] );
	buffer[ sizeof(buffer) - 1 ] = '\0';
	return buffer;
}

uint32_t imageio_get_pixel( image_t* img, int x, int y )
{
	assert( img->channels == 3 || img->channels == 4 );
	size_t index = (img->width * y + x) * img->channels;
	uint32_t* color = (uint32_t*) &img->pixels[ index ];
	return *color;
}

void imageio_set_pixel( image_t* img, int x, int y, uint32_t color )
{
	assert( img->channels == 3 || img->channels == 4 );
	size_t index = (img->width * y + x) * img->channels;

	//printf( "imageio_set_pixel() #%06X \n", color );

	if( img->channels == 4 )
	{
		//uint8_t* pcolor = (uint8_t*) &color;
		//colorblend_rgba_alpha( &img->pixels[index], pcolor, &img->pixels[index] );
		img->pixels[ index + 0 ] = channelblend_alpha( r32(color), img->pixels[ index + 0 ], a32(color) / 255.0f );
		img->pixels[ index + 1 ] = channelblend_alpha( g32(color), img->pixels[ index + 1 ], a32(color) / 255.0f );
		img->pixels[ index + 2 ] = channelblend_alpha( b32(color), img->pixels[ index + 2 ], a32(color) / 255.0f );
		img->pixels[ index + 3 ] = channelblend_alpha( a32(color), img->pixels[ index + 3 ], a32(color) / 255.0f );
	}
	else
	{
		img->pixels[ index + 0 ] = r24(color);
		img->pixels[ index + 1 ] = g24(color);
		img->pixels[ index + 2 ] = b24(color);
	}
}

void imageio_set_pixel_aa( image_t* img, int x, int y, uint32_t color, float intensity )
{
	assert( img->channels == 3 || img->channels == 4 );
	size_t index = (img->width * y + x) * img->channels;

	//printf( "imageio_set_pixel() #%06X   %0.3f\n", color, intensity );

	if( img->channels == 4 )
	{
		img->pixels[ index + 0 ] = (1 - intensity) * r32(color) + intensity * img->pixels[ index + 0 ];
		img->pixels[ index + 1 ] = (1 - intensity) * g32(color) + intensity * img->pixels[ index + 1 ];
		img->pixels[ index + 2 ] = (1 - intensity) * b32(color) + intensity * img->pixels[ index + 2 ];
		img->pixels[ index + 3 ] = (1 - intensity) * a32(color) + intensity * img->pixels[ index + 3 ];

	}
	else
	{
		img->pixels[ index + 0 ] = (1 - intensity) * r32(color) + intensity * img->pixels[ index + 0 ];
		img->pixels[ index + 1 ] = (1 - intensity) * g32(color) + intensity * img->pixels[ index + 1 ];
		img->pixels[ index + 2 ] = (1 - intensity) * b32(color) + intensity * img->pixels[ index + 2 ];
	}
}

void imageio_draw_line( image_t* img, int x0, int y0, int x1, int y1, uint32_t color )
{
	int dx =  abs( x1 - x0 );
	int dy = -abs( y1 - y0 );
	int sx = x0 < x1 ? 1 : -1;
	int sy = y0 < y1 ? 1 : -1;
	int err = dx + dy;
	int	e2; /* error value e_xy */

	for(;;)
	{
		imageio_set_pixel( img, x0, y0, color );
		if( x0==x1 && y0==y1 ) break;
		e2 = 2 * err;
		if( e2 >= dy ) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
		if( e2 <= dx ) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
	}
}

void imageio_draw_line_aa( image_t* img, int x0, int y0, int x1, int y1, uint32_t color )
{
	int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
	int err = dx-dy, e2, x2;                       /* error value e_xy */
	int ed = dx+dy == 0 ? 1 : sqrtf( dx * dx + dy * dy );

	for ( ; ; ){                                         /* pixel loop */
		imageio_set_pixel_aa( img, x0, y0, color, 1.0f*abs(err-dx+dy)/ed);
		e2 = err; x2 = x0;
		if( 2*e2 >= -dx) {                                    /* x step */
			if( x0 == x1) break;
			if( e2+dy < ed) imageio_set_pixel_aa( img, x0, y0+sy, color, 1.0f*(e2+dy)/ed);
			err -= dy; x0 += sx;
		}
		if( 2*e2 <= dy) {                                     /* y step */
			if( y0 == y1) break;
			if( dx-e2 < ed) imageio_set_pixel_aa( img, x2+sx, y0, color, 1.0f*(dx-e2)/ed);
			err += dx; y0 += sy;
		}
	}
}

void imageio_draw_rect( image_t* img, int x, int y, int w, int h, uint32_t color )
{
	imageio_draw_line( img,     x,     y, x + w,     y, color );
	imageio_draw_line( img, x + w,     y, x + w, y + h, color );
	imageio_draw_line( img,     x, y + h, x + w, y + h, color );
	imageio_draw_line( img,     x,     y,     x, y + h, color );
}

void imageio_draw_rect_filled( image_t* img, int x, int y, int w, int h, uint32_t color )
{
	int end = y + h;
	for( ; y < end; y++ )
	{
		imageio_draw_line( img, x, y, x + w, y, color );
	}
}

void imageio_draw_circle( image_t* img, int xm, int ym, int radius, uint32_t color )
{
	int x = -radius, y = 0, err = 2-2*radius; /* II. Quadrant */
	do {
		imageio_set_pixel( img, xm-x, ym+y, color ); /*   I. Quadrant */
		imageio_set_pixel( img, xm-y, ym-x, color ); /*  II. Quadrant */
		imageio_set_pixel( img, xm+x, ym-y, color ); /* III. Quadrant */
		imageio_set_pixel( img, xm+y, ym+x, color ); /*  IV. Quadrant */
		radius = err;
		if( radius <= y) err += ++y*2+1;           /* e_xy+e_y < 0 */
		if( radius > x || err > y) err += ++x*2+1; /* e_xy+e_x > 0 or no 2nd y-step */
	} while (x < 0);
}

void imageio_draw_circle_aa( image_t* img, int xm, int ym, int radius, uint32_t color )
{
	int x = -radius, y = 0;
	int x2, e2, err = 2-2*radius;
	float intensity;
	radius = 1-err;
	do {
		intensity = 1.0f*abs(err-2*(x+y)-2)/radius;
		imageio_set_pixel_aa( img, xm-x, ym+y, color, intensity );
		imageio_set_pixel_aa( img, xm-y, ym-x, color, intensity );
		imageio_set_pixel_aa( img, xm+x, ym-y, color, intensity );
		imageio_set_pixel_aa( img, xm+y, ym+x, color, intensity );
		e2 = err; x2 = x;
		if( err+y > 0) {
			intensity = 1.0f*(err-2*x-1)/radius;
			if( intensity < 1.0f) {
				imageio_set_pixel_aa( img, xm-x, ym+y+1, color, intensity );
				imageio_set_pixel_aa( img, xm-y-1, ym-x, color, intensity );
				imageio_set_pixel_aa( img, xm+x, ym-y-1, color, intensity );
				imageio_set_pixel_aa( img, xm+y+1, ym+x, color, intensity );
			}
			err += ++x*2+1;
		}
		if( e2+x2 <= 0) {
			intensity = 1.0f*(2*y+3-e2)/radius;
			if( intensity < 1.0f) {
				imageio_set_pixel_aa( img, xm-x2-1, ym+y, color, intensity );
				imageio_set_pixel_aa( img, xm-y, ym-x2-1, color, intensity );
				imageio_set_pixel_aa( img, xm+x2+1, ym-y, color, intensity );
				imageio_set_pixel_aa( img, xm+y, ym+x2+1, color, intensity );
			}
			err += ++y*2+1;
		}
	} while (x < 0);
}

void imageio_draw_circle_filled( image_t* img, int xm, int ym, int radius, uint32_t color )
{
	int x = -radius, y = 0, err = 2-2*radius;
	do {
		imageio_draw_line( img, xm - x, ym + y, xm + x, ym + y, color );
		imageio_draw_line( img, xm - x, ym - y, xm + x, ym - y, color );
		radius = err;
		if( radius <= y) err += ++y*2+1;
		if( radius > x || err > y) err += ++x*2+1;
	} while (x < 0);
}

void imageio_draw_circle_filled_aa( image_t* img, int xm, int ym, int radius, uint32_t color )
{
	imageio_draw_circle_aa( img, xm, ym, radius, color );
#if 1
	imageio_draw_circle_filled( img, xm, ym, radius, color );
#else
	int x = -radius, y = 0, err = 2-2*radius; /* II. Quadrant */
	do {
		imageio_draw_line_aa( img, xm-x, ym+y, xm+x, ym+y, color );
		imageio_draw_line_aa( img, xm-x, ym-y, xm+x, ym-y, color );
		radius = err;
		if( radius <= y) err += ++y*2+1;           /* e_xy+e_y < 0 */
		if( radius > x || err > y) err += ++x*2+1; /* e_xy+e_x > 0 or no 2nd y-step */
	} while (x < 0);
#endif
}

void imageio_draw_polygon( image_t* img, const int x[], const int y[], size_t size, uint32_t color )
{
	for( size_t i = 0; i < (size - 1); i++ )
	{
		imageio_draw_line( img,
				x[ i + 0 ], y[ i + 0 ],
				x[ i + 1 ], y[ i + 1 ],
		        color );
	}
	imageio_draw_line( img,
			x[ (size - 1) + 0 ], y[ (size - 1) + 0 ],
			x[              0 ], y[              0 ],
			color );
}

void imageio_draw_polygon_aa( image_t* img, const int x[], const int y[], size_t size, uint32_t color )
{
	for( size_t i = 0; i < (size - 1); i++ )
	{
		imageio_draw_line_aa( img,
		       x[ i + 0 ], y[ i + 0 ],
		       x[ i + 1 ], y[ i + 1 ],
		       color );
	}
	imageio_draw_line_aa( img,
			x[ (size - 1) + 0 ], y[ (size - 1) + 0 ],
			x[              0 ], y[              0 ],
			color );
}

// a1 * x + b1 * y = c1   and a2 * x + b2 * y = c2
static bool line_intersection( float a1, float b1, float c1, float a2, float b2, float c2, float* x, float* y )
{
	float delta = a1 * b2 - a2 * b1;

	if( fabs(delta) < FLT_EPSILON )
	{
		/* Lines are parrallel */
		return false;
	}

	*x = (b2 * c1 - b1 * c2) / delta;
	*y = (a1 * c2 - a2 * c1) / delta;

	return true;
}

// m1 * x + b1 = y   and m2 * x + b2 = y
// m1 * x + b1 = m2 * x + b2
// (b1 - b2) = (m2 - m1) * x
static bool line_intersection_point_intercept( float m1, float b1, float m2, float b2, float* x, float* y )
{
	if( fabs(m2 - m1) < FLT_EPSILON )
	{
		/* Lines are parrallel */
		return false;
	}

	*x = (b1 - b2) / (m2 - m1);
	*y = m2 * (*x) + b2;

	return true;
}
// m * x + b = y
// (y - b) / m = x
static bool line_intersection_with_horizontal_line( int x0, int y0, int x1, int y1, int y, float* x )
{
	bool result = true;

	if( x1 == x0 )
	{
		/* Line is vertical */
		*x = x1;
	}
	else
	{
		if( y1 == y0 )
		{
			/* Line is horizontal */
			//if( y0 == y )
			if( (y0 <= y && y <= y1) ||
			    (y1 <= y && y <= y0) )
			{
				*x = x0;
			}
			else
			{
				/* Lines are parallel */

				printf( "y0=%d, y1=%d, y=%d\n", y0, y1, y);
				result = false;
			}
		}
		else
		{
			float m = ((float)y1 - y0) / ((float)x1 - x0);
			float b = y1 - m * x1;

			*x = (y - b) / m;
		}
	}

	return result;
}

void imageio_draw_polygon_filled( image_t* img, const int x[], const int y[], size_t size, uint32_t color )
{
	if( size < 3 )
	{
		return;
	}

	int min_x = img->width;
	int max_x = 0;
	int min_y = img->height;
	int max_y = 0;
	int intersections_x[ size ];
	int intersections_y[ size ];
	size_t intersection_count = 0;

	for( size_t i = 0; i < size; i++ )
	{
		if( x[i] < min_x ) min_x = x[i];
		if( x[i] > max_x ) max_x = x[i];
		if( y[i] < min_y ) min_y = y[i];
		if( y[i] > max_y ) max_y = y[i];
	}


	float m = 0;
	float b = 0;

	for( int Y = min_y; Y < max_y; Y++ )
	{
		/* Find intersections */
		{
			for( size_t i = 0; i < (size - 1); i++ )
			{
				float sx;
				if( line_intersection_with_horizontal_line( x[i + 0], y[i + 0], x[i + 1], y[i + 1], Y, &sx ) )
				{
					if( (sx >= x[i + 0] && sx <= x[i + 1]) ||
						(sx >= x[i + 1] && sx <= x[i + 0]) )
					{
						intersections_x[ intersection_count ] = (int) sx;
						intersections_y[ intersection_count ] = Y;

						intersection_count += 1;
					}
				}
			}

			float sx;
			if( line_intersection_with_horizontal_line( x[size - 1], y[size - 1], x[0], y[0], Y, &sx ) )
			{
				if( (sx >= x[size-1] && sx <= x[0]) ||
					(sx >= x[0] && sx <= x[size-1]) )
				{
					intersections_x[ intersection_count ] = (int) sx;
					intersections_y[ intersection_count ] = Y;

					intersection_count += 1;
				}
			}
		}

		if( intersection_count >= 2 )
		{
			bool swapped = true;

			/* Sort intersections */
			while( swapped )
			{
				swapped = false;
				for( size_t i = 1; i < intersection_count; i++ )
				{
					if( intersections_x[ i ] < intersections_x[ i - 1 ] )
					{
						int tmp_x = intersections_x[ i ];
						intersections_x[ i ]     = intersections_x[ i - 1 ];
						intersections_x[ i - 1 ] = tmp_x;

						int tmp_y = intersections_y[ i ];
						intersections_y[ i ]     = intersections_y[ i - 1 ];
						intersections_y[ i - 1 ] = tmp_y;
						swapped = true;
					}
				}
			}

			/* Draw polygon scan lines */
			{
				for( size_t c = 0; c < (intersection_count - 1); c++ )
				{
					imageio_draw_line( img,
							intersections_x[ c + 0 ], intersections_y[ c + 0 ],
							intersections_x[ c + 1 ], intersections_y[ c + 1 ],
							color );
				}
				imageio_draw_line( img,
						intersections_x[ (intersection_count-1) ], intersections_y[ (intersection_count-1) ],
						intersections_x[ 0 ], intersections_y[ 0 ],
						color );
			}
		}
		else
		{

			printf( "Oops at y = %d\n", Y );
		}

		intersection_count = 0;
	}
}

void imageio_draw_pie_slice( image_t* img, int xm, int ym, float start_angle, float end_angle, int radius, uint32_t color )
{
	float step = 0.05f;

	int x = xm + radius * cosf(start_angle);
	int y = ym + radius * sinf(start_angle);
	int xn;
	int yn;

	imageio_draw_line( img, xm, ym, xm + radius * cosf(start_angle), ym + radius * sinf(start_angle), color );
	for( float a = start_angle; a < end_angle; a += step )
	{
		xn = xm + radius * cosf(a);
		yn = ym + radius * sinf(a);
		imageio_draw_line( img, x, y, xn, yn, color );
		x = xn;
		y = yn;
	}
	xn = xm + radius * cosf(end_angle);
	yn = ym + radius * sinf(end_angle);

	imageio_draw_line( img, x, y, xn, yn, color );
	imageio_draw_line( img, xm, ym, xm + radius * cosf(end_angle), ym + radius * sinf(end_angle), color );
}

void imageio_draw_pie_slice_aa( image_t* img, int xm, int ym, float start_angle, float end_angle, int radius, uint32_t color )
{
	float step = 0.05f;

	int x = xm + radius * cosf(start_angle);
	int y = ym + radius * sinf(start_angle);
	int xn;
	int yn;

	imageio_draw_line_aa( img, xm, ym, xm + radius * cosf(start_angle), ym + radius * sinf(start_angle), color );
	for( float a = start_angle; a < end_angle; a += step )
	{
		xn = xm + radius * cosf(a);
		yn = ym + radius * sinf(a);
		imageio_draw_line_aa( img, x, y, xn, yn, color );
		x = xn;
		y = yn;
	}
	xn = xm + radius * cosf(end_angle);
	yn = ym + radius * sinf(end_angle);

	imageio_draw_line_aa( img, x, y, xn, yn, color );
	imageio_draw_line_aa( img, xm, ym, xm + radius * cosf(end_angle), ym + radius * sinf(end_angle), color );
}


void imageio_draw_pie_slice_filled( image_t* img, int xm, int ym, float start_angle, float end_angle, int radius, uint32_t color )
{
	//float step = 0.015f;
	float step = 0.1f;

	int count = (int) ((end_angle - start_angle) / step) + 2;
	int points_x[ count ];
	int points_y[ count ];
	float angle = start_angle;

	points_x[ 0 ] = xm;
	points_y[ 0 ] = ym;

	for( int i = 1; i < count; i++ )
	{
		points_x[ i ] = xm + radius * cosf(angle);
		points_y[ i ] = ym + radius * sinf(angle);

		angle += step;
	}

	imageio_draw_polygon_filled( img, points_x, points_y, count, color );
}


