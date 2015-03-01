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
#include <assert.h>
#include <libutility/utility.h>
#include "../src/imageio.h"

int main( int argc, char* argv[] )
{

	if( argc > 1 )
	{
		image_t a;
		imageio_load( &a, argv[1], NULL );
		image_t b;
		imageio_load( &b, argv[2], NULL );


		image_t dst = (image_t) {
			.width     = (uint16_t) fmax( a.width, b.width ),
			.height    = (uint16_t) fmax( a.width, b.width ),
			.bit_depth = (uint8_t) fmax( a.bit_depth, b.bit_depth ),
			.channels  = 4
		};

		dst.pixels = malloc( dst.width * dst.height * dst.channels );

#if 0
		memset( dst.pixels, 255, dst.width * dst.height * dst.channels );
#else
		for( int y = 0; y < dst.height * 0.8; y++ )
		{
			for( int x = 0; x < dst.width * 0.8; x++ )
			{
				size_t index = dst.channels * (y * dst.width + x);
				dst.pixels[ index + 0 ] = 255 * sin( ((float)y / dst.width) );
				dst.pixels[ index + 1 ] = 255 * cos( ((float)x) / dst.width );
				dst.pixels[ index + 2 ] = 255;
				dst.pixels[ index + 3 ] = 255;

			}
		}
#endif


		bool blend_result = false;

		blend_result = imageio_blend( &dst, 0, 0, &a, IMAGEIO_BLEND_ALPHA );
		assert( blend_result );
		blend_result = imageio_blend( &dst, 0, 0, &b, IMAGEIO_BLEND_ALPHA );
		assert( blend_result );

		imageio_image_save( &dst, "blended.png", IMAGEIO_PNG );

		imageio_image_destroy( &b );
		imageio_image_destroy( &a );
		imageio_image_destroy( &dst );
	}

	return 0;
}
