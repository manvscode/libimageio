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
#ifndef _IMAGEIO_H_
#define _IMAGEIO_H_
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32) && defined(DLL)
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
typedef int            int32_t;
#define bool unsigned int
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
	IMAGEIO_PNG,
	IMAGEIO_PVR /* Compressed Texture Format */
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
	uint8_t  bit_depth;
	uint8_t  channels;
	uint8_t* pixels;
} image_t;


imageio_api bool imageio_load          ( image_t* img, const char* filename, image_file_format_t* fmt );
imageio_api bool imageio_image_load    ( image_t* img, const char* filename, image_file_format_t format );
imageio_api bool imageio_image_save    ( const image_t* img, const char* filename, image_file_format_t format );
imageio_api bool imageio_image_create  ( image_t* img, uint16_t width, uint16_t height, uint8_t bit_depth );
imageio_api void imageio_image_destroy ( image_t* img );
imageio_api void imageio_image_resize  ( uint32_t src_width, uint32_t src_height, const uint8_t* src_bitmap,
                                         uint32_t dst_width, uint32_t dst_height, uint8_t* dst_bitmap, uint32_t bit_depth,
                                         resize_algorithm_t algorithm );
imageio_api bool imageio_blit          ( uint32_t pos_x, uint32_t pos_y,
                                         uint32_t dst_width, uint32_t dst_height, uint32_t dst_bytes_per_pixel, uint8_t* dst_pixels,
                                         uint32_t src_width, uint32_t src_height, uint32_t src_bytes_per_pixel, uint8_t* src_pixels );


static inline size_t imageio_image_size( const image_t* img )
{
	return img->width * img->height * (img->bit_depth >> 3);
}

imageio_api typedef enum imageio_blend_mode {
	IMAGEIO_BLEND_NORMAL,
	IMAGEIO_BLEND_LIGHTEN,
	IMAGEIO_BLEND_DARKEN,
	IMAGEIO_BLEND_MULTIPLY,
	IMAGEIO_BLEND_AVERAGE,
	IMAGEIO_BLEND_ADD,
	IMAGEIO_BLEND_SUBTRACT,
	IMAGEIO_BLEND_DIFFERENCE,
	IMAGEIO_BLEND_NEGATION,
	IMAGEIO_BLEND_SCREEN,
	IMAGEIO_BLEND_EXCLUSION,
	IMAGEIO_BLEND_OVERLAY,
	IMAGEIO_BLEND_SOFT_LIGHT,
	IMAGEIO_BLEND_HARD_LIGHT,
	IMAGEIO_BLEND_COLOR_DODGE,
	IMAGEIO_BLEND_COLOR_BURN,
	IMAGEIO_BLEND_LINEAR_DODGE,
	IMAGEIO_BLEND_LINEAR_BURN,
	IMAGEIO_BLEND_LINEAR_LIGHT,
	IMAGEIO_BLEND_VIVID_LIGHT,
	IMAGEIO_BLEND_PIN_LIGHT,
	IMAGEIO_BLEND_HARD_MIX,
	IMAGEIO_BLEND_REFLECT,
	IMAGEIO_BLEND_GLOW,
	IMAGEIO_BLEND_PHOENIX,
	IMAGEIO_BLEND_ALPHA,
} blend_mode_t;

imageio_api bool imageio_blend( image_t* dst, uint32_t pos_x, uint32_t pos_y, const image_t* src, blend_mode_t mode );

imageio_api void imageio_blend_rgb( uint8_t* result, uint8_t* top, uint8_t* bottom, blend_mode_t mode );
imageio_api void imageio_blend_rgba( uint8_t* result, uint8_t* top, uint8_t* bottom, blend_mode_t mode );


imageio_api void imageio_swap_red_and_blue        ( uint32_t width, uint32_t height, uint32_t byte_count, uint8_t* bitmap );
imageio_api void imageio_flip_horizontally        ( uint32_t width, uint32_t height, uint32_t byte_count, uint8_t* bitmap );
imageio_api void imageio_flip_vertically          ( uint32_t width, uint32_t height, uint32_t byte_count, uint8_t* bitmap );
imageio_api void imageio_flip_horizontally_nocopy ( uint32_t width, uint32_t height, const uint8_t* src_bitmap, uint8_t* dst_bitmap, uint32_t byte_count );
imageio_api void imageio_flip_vertically_nocopy   ( uint32_t width, uint32_t height, const uint8_t* src_bitmap, uint8_t* dst_bitmap, uint32_t byte_count );
imageio_api bool imageio_detect_edges             ( uint32_t width, uint32_t height, uint32_t bit_depth, const uint8_t* src_bitmap, uint8_t* dst_bitmap, int32_t k );
imageio_api bool imageio_extract_color            ( uint32_t width, uint32_t height, uint32_t bit_depth, const uint8_t* src_bitmap, uint8_t* dst_bitmap, uint32_t color, uint32_t k );
imageio_api bool imageio_convert_to_grayscale     ( uint32_t width, uint32_t height, uint32_t bit_depth, const uint8_t* src_bitmap, uint8_t* dst_bitmap );
imageio_api bool imageio_convert_to_colorscale    ( uint32_t width, uint32_t height, uint32_t bit_depth, const uint8_t* src_bitmap, uint8_t* dst_bitmap, uint32_t color );
imageio_api void imageio_modify_contrast          ( uint32_t width, uint32_t height, uint32_t bit_depth, const uint8_t* src_bitmap, uint8_t* dst_bitmap, int contrast );
imageio_api void imageio_modify_brightness        ( uint32_t width, uint32_t height, uint32_t bit_depth, const uint8_t* src_bitmap, uint8_t* dst_bitmap, int brightness );
imageio_api void imageio_rgb_to_yuv444            ( uint32_t width, uint32_t height, uint32_t byte_count, uint8_t* bitmap );
imageio_api void imageio_yuv444_to_rgb            ( uint32_t width, uint32_t height, uint32_t byte_count, uint8_t* bitmap );
imageio_api bool imageio_is_opaque                ( const image_t* img, bool* p_partially_opaque );

imageio_api const char* imageio_image_string  ( const image_t* img );


#define rgba(r,g,b,a)	( (((uint8_t)(r)) << 24) | (((uint8_t)(g)) << 16) | (((uint8_t)(b)) << 8) | ((uint8_t)(a)) ) // 4 bytes
#define r32(color)		( ((color) >> 24) & 255 )
#define g32(color)		( ((color) >> 16) & 255 )
#define b32(color)		( ((color) >> 8 ) & 255 )
#define a32(color)		  ((color) & 255)

#define rgb(r,g,b)		( ((r) << 16) | ((g) << 8) | (b) )
#define r24(color)		( ((color) >> 16) & 255)
#define g24(color)		( ((color) >> 8 ) & 255)
#define b24(color)	 	  ((color) & 255)

uint32_t imageio_get_pixel             ( image_t* img, int x, int y );
void     imageio_set_pixel             ( image_t* img, int x, int y, uint32_t color );
void     imageio_set_pixel_aa          ( image_t* img, int x, int y, uint32_t color, float intensity );
void     imageio_draw_line             ( image_t* img, int x0, int y0, int x1, int y1, uint32_t color );
void     imageio_draw_line_aa          ( image_t* img, int x0, int y0, int x1, int y1, uint32_t color );
void     imageio_draw_rect             ( image_t* img, int x, int y, int w, int h, uint32_t color );
void     imageio_draw_rect_filled      ( image_t* img, int x, int y, int w, int h, uint32_t color );
void     imageio_draw_circle           ( image_t* img, int xm, int ym, int radius, uint32_t color );
void     imageio_draw_circle_aa        ( image_t* img, int xm, int ym, int radius, uint32_t color );
void     imageio_draw_circle_filled    ( image_t* img, int xm, int ym, int radius, uint32_t color );
void     imageio_draw_circle_filled_aa ( image_t* img, int xm, int ym, int radius, uint32_t color );
void     imageio_draw_polygon          ( image_t* img, const int x[], const int y[], size_t size, uint32_t color );
void     imageio_draw_polygon_aa       ( image_t* img, const int x[], const int y[], size_t size, uint32_t color );
void     imageio_draw_polygon_filled   ( image_t* img, const int x[], const int y[], size_t size, uint32_t color );
void     imageio_draw_pie_slice        ( image_t* img, int xm, int ym, float start_angle, float end_angle, int radius, uint32_t color );
void     imageio_draw_pie_slice_aa     ( image_t* img, int xm, int ym, float start_angle, float end_angle, int radius, uint32_t color );
void     imageio_draw_pie_slice_filled ( image_t* img, int xm, int ym, float start_angle, float end_angle, int radius, uint32_t color );

#ifdef __cplusplus
}
#endif
#endif // _IMAGEIO_H_
