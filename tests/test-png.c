#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include "../src/imageio.h"

int main( int argc, char* argv[] )
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

	if( !imageio_image_save( &image, "test.png", IMAGEIO_PNG ) )
	{
		perror( "ERROR" );
	}
	return 0;
}
