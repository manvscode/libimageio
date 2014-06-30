/*
 * Copyright (C) 2010-2014 by Joseph A. Marrero.  http://www.manvscode.com/
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
#include <err.h>
#include <getopt.h>
#include "../src/imageio.h"

static void help( void );
static void output_source_code( const image_t* image, const char* input, const char* output, uint8_t bits_per_pixel );

static struct option long_options[] = {
	{"help",     no_argument,       0, 'h'},
	{"input",    required_argument, 0, 'i'},
	{"output",   required_argument, 0, 'o'},
	{"bpp",      required_argument, 0, 'b'},
	{0, 0, 0, 0}
};

int main( int argc, char* argv[] )
{
	image_t image;
	image_file_format_t format;

	if( argc < 2 )
	{
		help( );
		return -1;
	}


	const char* input = NULL;
	const char* output = NULL;
	int opt;
	int opt_idx;
	uint8_t bits_per_pixel = 1;

	while( (opt = getopt_long(argc, argv, "hi:o:b:", long_options, &opt_idx)) != -1 )
	{
		switch( opt )
		{
			case 'i':
				input = optarg;
				break;
			case 'o':
				output = optarg;
				break;
			case 'b':
				bits_per_pixel = atoi( optarg );
				break;
			default:
			case 'h':
				help();
				break;
		}
	}

	if( !input )
	{
		fprintf( stderr, "Missing input filename.\n" );
		return -1;
	}
	else if( !imageio_load( &image, input, &format ) )
	{
		fprintf( stderr, "Unable to open %s.\n", input );
		return -1;
	}

	if( !output )
	{
		fprintf( stderr, "Need an output file.\n" );
		return -1;
	}
	else
	{
		imageio_flip_vertically_nocopy( image.width, image.height, image.pixels, image.pixels, image.channels );
		output_source_code( &image, input, output, bits_per_pixel );
	}

	return 0;
}

void help( void )
{
	printf( "----[ Image to C Source Converter ]-----------------\n" );
	printf( "Copyright 2010-2014, Joseph A. Marrero\n" );
	printf( "http://www.manvscode.com/\n\n" );
	printf( "Command line options:\n" );

	//for( size_t i = 0; i < sizeof(long_options)/sizeof(long_options[0]); i++ )
	{
		printf( "  -%c, --%-12s %-s\n", 'h', "help",       "Get help on how to use this program." );
		printf( "  -%c, --%-12s %-s\n", 'i', "input",      "The input image filename." );
		printf( "  -%c, --%-12s %-s\n", 'o', "output",     "The output C source filename." );
		printf( "  -%c, --%-12s %-s\n", 'b', "bpp",        "The output bits per pixel." );
	}

	printf( "----------------------------------------------------\n" );
}


void output_source_code( const image_t* image, const char* input, const char* output, uint8_t bits_per_pixel )
{
	//uint8_t channels = bits_per_pixel > 1 ? (bits_per_pixel >> 3) : 1;
	uint8_t channels = bits_per_pixel >> 3;

	FILE* ofile = fopen( output, "w" );

	printf( "image = %s\n", imageio_image_string(image) );

	if( ofile )
	{
		fprintf( ofile, "/* Exported from %s */\n", input );
		fprintf( ofile,
			"struct {\n"
			"\tunsigned short width;\n"
			"\tunsigned short height;\n"
			"\tunsigned short bits_per_pixel;\n"
			"\tunsigned char channels;\n"
			"\tunsigned char pixels[ %u * %u * %u ];\n"
			"} exported_image = {\n"
			"\t%u,\n"
			"\t%u,\n"
			"\t%u,\n"
			"\t%u,\n",
			image->width, image->height, channels,
			image->width,
			image->height,
			bits_per_pixel,
			channels
		);

		fprintf( ofile, "\t\"");

		printf( "bits_per_pixel = %d\n", bits_per_pixel );
		if( bits_per_pixel == 1 )
		{
			size_t size = image->width * image->height * image->channels;

			unsigned char byte = 0;
			size_t pixel = 0;

			for( size_t i = 0; i < size; i += image->channels )
			{
				if( pixel != 0 && (pixel % 64) == 0 )
				{
					fprintf( ofile, "\"\n\t\"" );
				}

				float sum = 0.0f;
				for( size_t j = 0; j < image->channels; j++ )
				{
					sum += image->pixels[ i + j ];
				}
				sum /= image->channels;

				int bit = pixel % 8;
				byte |= (sum > 0.0f ? (1 << (7 - bit)) : 0);

				if( bit == 7 )
				{
					fprintf( ofile, "\\x%02x", byte );
					byte = 0;
				}
				pixel++;
			}
		}
		else
		{
			printf( "channels = %d\n", channels );
			size_t size = image->width * image->height * image->channels;
			size_t pixel = 0;

			for( size_t i = 0; i < size; i += image->channels )
			{
				for( size_t j = 0; j < channels; j++ )
				{
					size_t index = i + j;

					if( pixel != 0 && (pixel % (channels * image->width) == 0) )
					{
						fprintf( ofile, "\"\n\t\"" );
					}
					fprintf( ofile, "\\x%02x", image->pixels[ index ] );
					pixel += 1;
				}
			}
		}

		fprintf( ofile, "\"\n");
		fprintf( ofile, "};\n" );

		fclose( ofile );
	}
}
