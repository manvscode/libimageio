#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include "../src/imageio.h"

int main( int argc, char* argv[] )
{
	image_t image;
	image_file_format_t format;

	if( argc < 2 )
	{
		printf( "Missing filename parameter.\n" );
		return -1;
	}

	if( !imageio_load( &image, argv[1], &format ) )
	{
		printf( "Unable to convert %s.\n", argv[1] );
		return -1;
	}


	printf(
		"struct {\n"
		"	uint16_t width;\n"
		"	uint16_t height;\n"
		"	uint8_t  bits_per_pixel;\n"
		"	uint8_t  channels;\n"
		"	uint8_t  pixels[ %u * %u * %u ];\n"
		"} image = {\n"
		"	%u,\n"
		"	%u,\n"
		"	%u,\n"
		"	%u,\n"
		"	%s,\n"
		"}\n",
		image.width, image.height, image.channels,
		image.width,
		image.height,
		image.bits_per_pixel,
		image.channels,
		"\"\""
	);

	return 0;
}
