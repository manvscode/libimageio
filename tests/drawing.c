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
#include <utility.h>
#include "../src/imageio.h"

#define deg2rad(a)   ((a) * M_PI / 180.0f)

static void draw_hexagon( image_t* img );
static void draw_octagon( image_t* img );
static void draw_square( image_t* img );
static void draw_triangle( image_t* img );


int main( int argc, char* argv[] )
{
	image_t img;
	imageio_image_create( &img, 600, 600, 32 );
	srand( time(NULL) );

	// Fill with white
	#if 0
	memset( img.pixels, 0xaa, img.width * img.height * img.channels );
	#else
	for( int y = 0; y < img.height; y++ )
	{
		for( int x = 0; x < img.width; x++ )
		{
			size_t index = img.channels * (y * img.width + x);
			img.pixels[ index + 0 ] = 255 * sin( ((float)y / img.height) );
			img.pixels[ index + 1 ] = 255 * cos( ((float)x) / img.width );
			img.pixels[ index + 2 ] = 255 * cos( ((float)y / img.height) );
			img.pixels[ index + 3 ] = 128;

		}
	}
	#endif

	/*
	for( int l = 0; l < 100; l++ )
	{
		imageio_draw_line_aa( &img, rand() % img.width, rand() % img.height,
			   	rand() % img.width, rand() % img.height,
				rgba(rand() % 255, rand() % 255, rand() % 255, 255) );
	}
	*/

#if 0
	imageio_draw_circle_filled_aa( &img, 200, 200, img.width * 0.20f, rgba(0xff, 0x00, 0x00, 0x55) );
	imageio_draw_circle_aa( &img, 200, 200, img.width * 0.20f, rgba(0, 0, 0, 0xFF) );
	imageio_draw_circle_filled_aa( &img, 400, 400, img.width * 0.30f, rgba(0x00, 0x00, 0xFF, 0x55) );
	imageio_draw_circle_aa( &img, 400, 400, img.width * 0.30f, rgba(0, 0, 0, 0xFF) );

	imageio_draw_rect_filled( &img, 50, 50, 100, 50, rgba(255, 255, 0, 0x66) );
	imageio_draw_rect( &img, 50, 50, 100, 50, rgba(0, 0, 0, 255) );
#endif


	//draw_hexagon( &img );
	//draw_octagon( &img );
	//draw_square( &img );
	//draw_triangle( &img );


	imageio_draw_pie_slice_filled( &img, 80, 450, deg2rad(0), deg2rad(90), img.width * 0.20f, rgba(22, 22, 255, 255) );
	imageio_draw_pie_slice( &img, 80, 450, deg2rad(0), deg2rad(90), img.width * 0.20f, rgba(255, 22, 255, 255) );

	imageio_draw_line( &img, 150, 550, 400, 550, rgba(255,0,0,255) );

	//imageio_draw_line( &img, 267, 0, 267, img.height - 1, rgba(255,255,255,255) );
	//imageio_draw_line( &img, 289, 0, 289, img.height - 1, rgba(255,255,255,255) );
	//imageio_draw_line( &img, 310, 0, 310, img.height - 1, rgba(255,255,255,255) );
	//imageio_draw_line( &img, 330, 0, 330, img.height - 1, rgba(255,255,255,255) );

	imageio_image_save( &img, "drawing.png", IMAGEIO_PNG );

	imageio_image_destroy( &img );
	return 0;
}

void draw_hexagon( image_t* img )
{
	const int hexagon_x[] = {
		480 + 100 * cos(deg2rad(1 * 60)),
		480 + 100 * cos(deg2rad(2 * 60)),
		480 + 100 * cos(deg2rad(3 * 60)),
		480 + 100 * cos(deg2rad(4 * 60)),
		480 + 100 * cos(deg2rad(5 * 60)),
		480 + 100 * cos(deg2rad(6 * 60))
	};
	const int hexagon_y[] = {
		110 + 100 * sin(deg2rad(1 * 60)),
		110 + 100 * sin(deg2rad(2 * 60)),
		110 + 100 * sin(deg2rad(3 * 60)),
		110 + 100 * sin(deg2rad(4 * 60)),
		110 + 100 * sin(deg2rad(5 * 60)),
		110 + 100 * sin(deg2rad(6 * 60))
	};

	imageio_draw_polygon_filled( img, hexagon_x, hexagon_y, 6, rgba(255, 0, 0, 255) );
	imageio_draw_polygon_aa( img, hexagon_x, hexagon_y, 6, rgba(255, 255, 0, 255) );
}

void draw_octagon( image_t* img )
{
	const int octagon_x[] = {
		300 + 50 * cos(deg2rad(1 * 45)),
		300 + 50 * cos(deg2rad(2 * 45)),
		300 + 50 * cos(deg2rad(3 * 45)),
		300 + 50 * cos(deg2rad(4 * 45)),
		300 + 50 * cos(deg2rad(5 * 45)),
		300 + 50 * cos(deg2rad(6 * 45)),
		300 + 50 * cos(deg2rad(7 * 45)),
		300 + 50 * cos(deg2rad(8 * 45))
	};
	const int octagon_y[] = {
		300 + 50 * sin(deg2rad(1 * 45)),
		300 + 50 * sin(deg2rad(2 * 45)),
		300 + 50 * sin(deg2rad(3 * 45)),
		300 + 50 * sin(deg2rad(4 * 45)),
		300 + 50 * sin(deg2rad(5 * 45)),
		300 + 50 * sin(deg2rad(6 * 45)),
		300 + 50 * sin(deg2rad(7 * 45)),
		300 + 50 * sin(deg2rad(8 * 45))
	};

	imageio_draw_polygon_filled( img, octagon_x, octagon_y, 8, rgba(255, 0, 255, 255) );
	imageio_draw_polygon_aa( img, octagon_x, octagon_y, 8, rgba(0, 0, 0, 255) );
}

void draw_square( image_t* img )
{
	const int square_x[] = {
		75 + 50 * cos(deg2rad(0 * 90)),
		75 + 50 * cos(deg2rad(1 * 90)),
		75 + 50 * cos(deg2rad(2 * 90)),
		75 + 50 * cos(deg2rad(3 * 90))
	};
	const int square_y[] = {
		450 + 50 * sin(deg2rad(0 * 90)),
		450 + 50 * sin(deg2rad(1 * 90)),
		450 + 50 * sin(deg2rad(2 * 90)),
		450 + 50 * sin(deg2rad(3 * 90))
	};

	imageio_draw_polygon_filled( img, square_x, square_y, 4, rgba(255, 255, 0, 255) );
	imageio_draw_polygon_aa( img, square_x, square_y, 4, rgba(0, 0, 0, 255) );
}

void draw_triangle( image_t* img )
{
	const int triangle_x[] = {
		525 + 60 * cos(deg2rad(30 + 1 * 120)),
		525 + 60 * cos(deg2rad(30 + 2 * 120)),
		525 + 60 * cos(deg2rad(30 + 3 * 120)),
	};
	const int triangle_y[] = {
		525 + 60 * sin(deg2rad(30 + 1 * 120)),
		525 + 60 * sin(deg2rad(30 + 2 * 120)),
		525 + 60 * sin(deg2rad(30 + 3 * 120)),
	};

	imageio_draw_polygon_filled( img, triangle_x, triangle_y, 3, rgba(0, 0, 255, 100) );
	imageio_draw_polygon_aa( img, triangle_x, triangle_y, 3, rgba(0, 0, 0, 255) );

}
