#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <libutility/utility.h>
#include "../src/imageio.h"


void png_save32( void )
{
	image_t image;
	image.width          = 512;
	image.height         = 512;
	image.bits_per_pixel = 32;
	size_t len           = 4 * 512 * 512;
	image.pixels         = (uint8_t*) malloc( sizeof(uint8_t) * len );
	uint32_t* colors     = (uint32_t*) image.pixels;

	for( size_t i = 1; i < 511; i++ )
	{
		for( size_t j = 1; j < 511; j++ )
		{
			if( i == j || (512 - i) == j )
			{
				colors[ i * 512 + j-1 ]     = 0xFF0000FF;
				colors[ i * 512 + j ]     = 0xFF0000FF;
				colors[ i * 512 + j+1 ]     = 0xFF0000FF;
			}
			else
			{
				colors[ i * 512 + j ]     = 0xFFFFFFFF;
			}
		}
	}

	if( !imageio_image_save( &image, "test-32bpp.png", IMAGEIO_PNG ) )
	{
		perror( "ERROR" );
	}
}

void png_save8( void )
{
	image_t image;
	image.width          = 512;
	image.height         = 512;
	image.bits_per_pixel = 8;
	size_t len           = 1 * 512 * 512;
	image.pixels         = (uint8_t*) malloc( sizeof(uint8_t) * len );

	for( size_t i = 0; i < 512; i++ )
	{
		for( size_t j = 0; j < 512; j++ )
		{
			if( i == j || (511 - i) == j )
			{
				image.pixels[ i * 512 + j ] = 0xFF;
			}
			else
			{
				image.pixels[ i * 512 + j ] = 0x00;
			}
		}
	}

	if( !imageio_image_save( &image, "test-8bpp.png", IMAGEIO_PNG ) )
	{
		perror( "ERROR" );
	}
}

int main( int argc, char* argv[] )
{
	//png_save32( );
	//png_save8( );

	if( argc > 1 )
	{
		image_t image;
		imageio_image_load( &image, argv[1], IMAGEIO_PNG );

		char* str = debug_buffer_to_string ( image.pixels, 4 * 10, 4, true );
		printf( "First 10 pixels = %s\n", str );

		free( str );
	}

	return 0;
}
