/* Copyright (C) 2009-2015 by Joseph A. Marrero, http://www.manvscode.com/
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
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <libutility/utility.h>
#include "../src/imageio.h"

#define deg2rad(a)   ((a) * M_PI / 180.0f)

int main( int argc, char* argv[] )
{
	image_t img;
	imageio_image_create( &img, 600, 600, 32 );
	srand( time(NULL) );

	// Fill with white
	#if 0
	memset( img.pixels, 0, img.width * img.height * img.channels );
	#else
	for( int y = 0; y < img.height; y++ )
	{
		for( int x = 0; x < img.width; x++ )
		{
			size_t index = img.channels * (y * img.width + x);
			img.pixels[ index + 0 ] = 255 * sin( ((float)y / img.width) );
			img.pixels[ index + 1 ] = 255 * cos( ((float)x) / img.width );
			img.pixels[ index + 2 ] = 255;
			img.pixels[ index + 3 ] = 255;

		}
	}
	#endif


	for( int l = 0; l < 100; l++ )
	{
		imageio_draw_line_aa( &img, rand() % img.width, rand() % img.height,
			   	rand() % img.width, rand() % img.height,
				rgba(rand() % 255, rand() % 255, rand() % 255, 255) );
	}

	imageio_draw_circle_filled_aa( &img, 200, 200, img.width * 0.20f, rgba(0xff, 0x00, 0x00, 0x55) );
	imageio_draw_circle_aa( &img, 200, 200, img.width * 0.20f, rgba(0, 0, 0, 0xFF) );
	imageio_draw_circle_filled_aa( &img, 400, 400, img.width * 0.20f, rgba(0x00, 0x00, 0xFF, 0x55) );
	imageio_draw_circle_aa( &img, 400, 400, img.width * 0.20f, rgba(0, 0, 0, 0xFF) );

	imageio_draw_rect_filled( &img, 50, 50, 100, 50, rgba(255, 255, 0, 0x66) );
	imageio_draw_rect( &img, 50, 50, 100, 50, rgba(0, 0, 0, 255) );

	//imageio_draw_pie_slice( &img, 250, 250, deg2rad(220), deg2rad(290), img.width * 0.20f, rgba(22, 22, 255, 255) );

	imageio_image_save( &img, "drawing.png", IMAGEIO_PNG );

	imageio_image_destroy( &img );
	return 0;
}
