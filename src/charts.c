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
#include <stdlib.h>
#include "imageio.h"

void chart_bar( image_t* img, const char* names[], double values[], size_t count )
{
	double sum = 0.0;

	for( size_t i = 0; i < count; i++ )
	{
		sum += values[ i ];
	}

	double percentages[ count ];

	for( size_t i = 0; i < count; i++ )
	{
		percentages[ i ] = values[ i ] / sum;
	}

	imageio_draw_line( img, 5, 5, 5, 100, rgb(33,33,33) );
	imageio_draw_line( img, 5, 100, 100, 100, rgb(33,33,33) );
}

void chart_pie( image_t* img, const char* names[], double values[], size_t count )
{
	double sum = 0.0;

	for( size_t i = 0; i < count; i++ )
	{
		sum += values[ i ];
	}

	int cx = img->width / 2;
	int cy = img->height / 2;


}
