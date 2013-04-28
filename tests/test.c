/*
 * Copyright (C) 2012 by Joseph A. Marrero and Shrewd LLC. http://www.manvscode.com/
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
#include <GL/freeglut.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <imageio.h>

#define ESC_KEY			27

void initialize         ( void );
void deinitialize       ( void );
void render             ( void );
void resize             ( int width, int height );
void keyboard_keypress  ( unsigned char key, int x, int y );
void mouse              ( int button, int state, int x, int y );
void mouse_motion       ( int x, int y );
void idle               ( void );
void write_text         ( void *font, const char* text, int x, int y, float r, float g, float b );

int windowWidth;
int windowHeight;
image_t image;
GLuint texture;

int main( int argc, char *argv[] )
{
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE );

#if 1
	if( argc <= 1 || (argc != 1 && (argc - 1) % 2 != 0) )
	{
		printf( "Usage: %s -f <filename>\n\n", argv[0] );
		return 1;
	}
	for( int c = 1; c < argc; c += 2 )
	{
		if( strncmp( argv[ c ], "-f", 2 ) == 0 )
		{
			imageio_image_load( &image, argv[ c + 1], PNG );

			printf( "Loaded: %s\n", argv[ c + 1 ] );
			printf( "%d x %d @ %dbpp\n", image.width, image.height, image.bits_per_pixel );
		}
	}
#else
	imageio_image_load( &image, "./black-cat.png", PNG );
	printf( "%d x %d @ %dbpp\n", image.width, image.height, image.bits_per_pixel );
#endif




#ifndef FULLSCREEN
	glutInitWindowSize( 800, 600 );
	int windowId = glutCreateWindow( "[ImageIO Test] - http://www.ManVsCode.com/" );
#else
	glutGameModeString( "640x480" );

	if( glutGameModeGet( GLUT_GAME_MODE_POSSIBLE ) )
		glutEnterGameMode( );
	else
	{
		glutGameModeString( "640x480" );
		if( glutGameModeGet( GLUT_GAME_MODE_POSSIBLE ) )
			glutEnterGameMode( );
		else
		{
			cerr << "The requested mode is not available!" << endl;
			return -1;
		}
	}
#endif


	glutDisplayFunc( render );
	glutReshapeFunc( resize );
	glutKeyboardFunc( keyboard_keypress );
	//glutIdleFunc( idle );
	glutMouseFunc( mouse );
	glutMotionFunc( mouse_motion );


	glutSetOption( GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS );

	initialize( );
	glutMainLoop( );
#ifndef FULLSCREEN
	glutDestroyWindow( windowId );
#endif
	deinitialize( );

	return 0;
}

void initialize( void )
{
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_CULL_FACE );

	glEnable( GL_POINT_SMOOTH );
	glEnable( GL_LINE_SMOOTH );
	glHint( GL_POINT_SMOOTH_HINT, GL_FASTEST );
	glHint( GL_LINE_SMOOTH_HINT, GL_FASTEST );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );


	glShadeModel( GL_FLAT );
	glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );


	glPixelStorei( GL_PACK_ALIGNMENT, 4 );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );

	glEnable( GL_TEXTURE_2D );
	glGenTextures( 1, &texture );
	glBindTexture( GL_TEXTURE_2D, texture );
	glTexImage2D( GL_TEXTURE_2D, 0, image.bits_per_pixel == 32 ? GL_RGBA : GL_RGB, image.width, image.height, 0, image.bits_per_pixel == 32 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, image.pixels );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
}

void deinitialize( void )
{
	imageio_image_destroy( &image );
}


void render( void )
{
	glClear( GL_COLOR_BUFFER_BIT  );
	glLoadIdentity( );


	glBindTexture( GL_TEXTURE_2D, texture );
	glBegin( GL_QUADS );
		glVertex2i( -1, -1 );  glTexCoord2i( 0, 1 );
		glVertex2i( 1, -1 );  glTexCoord2i( 0, 0 );
		glVertex2i( 1, 1 );  glTexCoord2i( -1, 0 );
		glVertex2i( -1, 1 );  glTexCoord2i( -1, 1 );
	glEnd( );

	/* Write text */
	//int width = glutGet((GLenum)GLUT_WINDOW_WIDTH);
	//int height = glutGet((GLenum)GLUT_WINDOW_HEIGHT);

	write_text( GLUT_BITMAP_HELVETICA_18, "ImageIO Test", 2, 22, 1.0f, 1.0f, 1.0f );
	write_text( GLUT_BITMAP_8_BY_13, "Press <ESC> to quit.", 2, 5, 1.0f, 1.0f, 1.0f );

	glutSwapBuffers( );
}

void resize( int width, int height )
{
	glViewport( 0, 0, width, height );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );


#define max( x, y )              ((x) ^ (((x) ^ (y)) & -((x) < (y))))
	height = max( 1, height );

	float aspect = ((float)width) / height;

	glOrtho( -1 * aspect, 1 * aspect, -1, 1, 0.0, 1.0 );
	glMatrixMode( GL_MODELVIEW );

	windowWidth  = width;
	windowHeight = height;
}

void keyboard_keypress( unsigned char key, int x, int y )
{
	switch( key )
	{
		case ESC_KEY:
			deinitialize( );
			exit( 0 );
			break;
		default:
			break;
	}

}

int mouseButton = 0;
int mouseButtonState = 0;

void mouse( int button, int state, int x, int y )
{
	mouseButton = button;
	mouseButtonState = state;
}

void mouse_motion( int x, int y )
{
	if( mouseButton == GLUT_LEFT_BUTTON && mouseButtonState == GLUT_DOWN && y <= windowHeight )
	{
	}
}

void idle( void )
{ /*glutPostRedisplay( );*/ }

void write_text( void *font, const char* text, int x, int y, float r, float g, float b )
{
	int width   = glutGet( (GLenum) GLUT_WINDOW_WIDTH );
	int height  = glutGet( (GLenum) GLUT_WINDOW_HEIGHT );
	size_t size = strlen( text ) + 1;

	glPushAttrib( GL_LIGHTING_BIT | GL_TEXTURE_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
		glDisable( GL_DEPTH_TEST );
		glDisable( GL_TEXTURE_2D );
		glDisable( GL_LIGHTING );

		glMatrixMode( GL_PROJECTION );
		glPushMatrix( );
			glLoadIdentity( );
			glOrtho( 0, width, 0, height, 1.0, 10.0 );

			glMatrixMode( GL_MODELVIEW );
			glPushMatrix( );
				glLoadIdentity( );
				glColor3f( r, g, b );
				glTranslatef( 0.0f, 0.0f, -1.0f );
				glRasterPos2i( x, y );

				for( unsigned int i = 0; i < size; i++ )
					glutBitmapCharacter( font, text[ i ] );

			glPopMatrix( );
			glMatrixMode( GL_PROJECTION );
		glPopMatrix( );
		glMatrixMode( GL_MODELVIEW );
	glPopAttrib( );
}


