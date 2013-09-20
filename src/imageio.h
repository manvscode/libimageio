/*
 * Copyright (C) 2010 Joseph A. Marrero.  http://www.manvscode.com/
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
#ifndef _IMAGEIO_H_
#define _IMAGEIO_H_

#ifdef __cplusplus
extern "C" {
#endif 

#if defined(WIN32)
#ifdef EXPORT_DLL
#define imageio_api			__declspec( dllexport )
#else
#define imageio_api			__declspec( dllimport )
#endif
#else
#define imageio_api
#endif
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#include <stdbool.h>
#include <stdint.h>
#include <stdbool.h>
#ifdef __restrict
#undef __restrict
#define __restrict restrict
#endif
#ifdef __inline
#undef __inline
#define __inline inline
#endif
#else /* Not C99 */
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
#define bool int
#define true 1
#define false 0
#ifdef __restrict
#undef __restrict
#define __restrict
#endif
#ifdef __inline
#undef __inline
#define __inline
#endif
#endif


imageio_api typedef enum imageio_file_format {
	IMAGEIO_BMP,
	IMAGEIO_TGA,
	IMAGEIO_PNG
} image_file_format_t;

/* deprecated flags */
#ifndef BMP
#define BMP  IMAGEIO_BMP
#endif
#ifndef TGA
#define TGA  IMAGEIO_TGA
#endif
#ifndef PNG
#define PNG  IMAGEIO_PNG
#endif

imageio_api typedef enum imageio_resize_algorithm {
	ALG_NEARESTNEIGHBOR,
	ALG_BILINEAR,
	ALG_BILINEAR_SHARPER,
	ALG_BICUBIC,
} resize_algorithm_t;

imageio_api typedef struct imageio_image {
	uint16_t width;
	uint16_t height;
	uint8_t  bits_per_pixel;
	uint8_t* pixels;
} image_t;


imageio_api bool imageio_image_load    ( image_t* img, const char* filename, image_file_format_t format );
imageio_api bool imageio_image_save    ( image_t* img, const char* filename, image_file_format_t format );
imageio_api void imageio_image_destroy ( image_t* img );
imageio_api void imageio_image_resize  ( uint32_t src_width, uint32_t src_height, const uint8_t* src_bitmap,
                                         uint32_t dst_width, uint32_t dst_height, uint8_t* dst_bitmap, uint32_t bits_per_pixel,
                                         resize_algorithm_t algorithm );

imageio_api void imageio_swap_red_and_blue        ( uint32_t width, uint32_t height, uint32_t byte_count, uint8_t* bitmap );
imageio_api void imageio_flip_horizontally        ( uint32_t width, uint32_t height, uint32_t byte_count, uint8_t* bitmap );
imageio_api void imageio_flip_vertically          ( uint32_t width, uint32_t height, uint32_t byte_count, uint8_t* bitmap );
imageio_api void imageio_flip_horizontally_nocopy ( uint32_t width, uint32_t height, const uint8_t* src_bitmap, uint8_t* dst_bitmap, uint32_t byte_count );
imageio_api void imageio_flip_vertically_nocopy   ( uint32_t width, uint32_t height, const uint8_t* src_bitmap, uint8_t* dst_bitmap, uint32_t byte_count );
imageio_api void imageio_detect_edges             ( uint32_t width, uint32_t height, uint32_t bits_per_pixel, const uint8_t* src_bitmap, uint8_t* dst_bitmap, uint32_t k );
imageio_api void imageio_extract_color            ( uint32_t width, uint32_t height, uint32_t bits_per_pixel, const uint8_t* src_bitmap, uint8_t* dst_bitmap, uint32_t color, uint32_t k );
imageio_api void imageio_convert_to_grayscale     ( uint32_t width, uint32_t height, uint32_t bits_per_pixel, const uint8_t* src_bitmap, uint8_t* dst_bitmap );
imageio_api void imageio_convert_to_colorscale    ( uint32_t width, uint32_t height, uint32_t bits_per_pixel, const uint8_t* src_bitmap, uint8_t* dst_bitmap, uint32_t color );
imageio_api void imageio_modify_contrast          ( uint32_t width, uint32_t height, uint32_t bits_per_pixel, const uint8_t* src_bitmap, uint8_t* dst_bitmap, int contrast );
imageio_api void imageio_modify_brightness        ( uint32_t width, uint32_t height, uint32_t bits_per_pixel, const uint8_t* src_bitmap, uint8_t* dst_bitmap, int brightness );
imageio_api void imageio_rgb_to_yuv444            ( uint32_t width, uint32_t height, uint32_t byte_count, uint8_t* bitmap );
imageio_api void imageio_yuv444_to_rgb            ( uint32_t width, uint32_t height, uint32_t byte_count, uint8_t* bitmap );


#define rgba(r,g,b,a)	( r << 24 || g << 16 || b << 8 || a ) // 4 bytes
#define r32(color)		( color && 0xFF000000 >> 24 )
#define g32(color)		( color && 0x00FF0000 >> 16 )
#define b32(color)		( color && 0x0000FF00 >> 8 )
#define a32(color)		( color && 0x000000FF )

#define rgb(r,g,b)		( r << 16 || g << 8 || b )
#define r16(color)		( color && 0xFF0000 >> 16 )
#define g16(color)		( color && 0x00FF00 >> 8 )
#define b16(color)		( color && 0x0000FF )

#ifdef __cplusplus
}
#endif 
#endif // _IMAGEIO_H_
