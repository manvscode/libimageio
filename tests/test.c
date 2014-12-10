/* Copyright (C) 2009-2014 by Joseph A. Marrero, http://www.manvscode.com/
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
#include <time.h>
#include "../src/simplegl.h"
#include <lib3dmath/vec3.h>
#include <lib3dmath/quat.h>
#include <SDL2/SDL.h>

static void initialize     ( void );
static void deinitialize   ( void );
static void render         ( void );
static void dump_sdl_error ( void );

SDL_Window* window = NULL;
SDL_GLContext ctx = NULL;

GLuint program = 0;
GLint attribute_vertex = 0;
GLint attribute_tex_coord = 0;
GLint attribute_color = 0;
GLint uniform_model_view = 0;
GLint uniform_voxel_data = 0;
GLint uniform_back_voxels = 0;
GLint uniform_color_transfer = 0;
GLint uniform_rendering_pass = 0;
GLint uniform_seed = 0;

GLuint vao = 0;
GLuint vbo_vertices = 0;
GLuint vbo_tex_coords = 0;
GLuint ibo_indices = 0;
GLuint voxel_texture = 0;
GLuint framebuffer = 0;
GLuint colorRenderBuffer;
GLuint back_voxel_texture;
GLuint color_transfer_texture;



//                       (-1,1,-1)    (1,1,-1)                            (0,1,0)    (1,1,0)
//                            +------+                                        +------+
//                          .'|    .'|                                      .'|    .'|
//               (-1,1,1) +---+--+'  | (1,1,1)                    (0,1,1) +---+--+'  | (1,1,1)
//                        |   |  |   |                                    |   |  |   |
//             (-1,-1,-1) |   +--+---+ (1,-1,-1)                  (0,0,0) |   +--+---+ (1,0,0)
//                        | .'   | .'                                     | .'   | .'
//                        +------+'                                       +------+'
//                 (-1,-1,1)     (1,-1,1)                          (0,0,1)     (1,0,1)

#define array_length( arr )   (sizeof(arr) / sizeof(arr[0]))

static GLfloat vertices[] = {
	// front
	-1, -1,  1,
	 1, -1,  1,
	 1,  1,  1,
	-1,  1,  1,
	// top
	-1,  1,  1,
	 1,  1,  1,
	 1,  1, -1,
	-1,  1, -1,
	// back
	 1, -1, -1,
	-1, -1, -1,
	-1,  1, -1,
	 1,  1, -1,
	// bottom
	-1, -1, -1,
	 1, -1, -1,
	 1, -1,  1,
	-1, -1,  1,
	// left
	-1, -1, -1,
	-1, -1,  1,
	-1,  1,  1,
	-1,  1, -1,
	// right
	 1, -1,  1,
	 1, -1, -1,
	 1,  1, -1,
	 1,  1,  1,
};
#define vertex_count array_length(vertices)

static GLfloat tex_coords[] = {
	// front
	0, 0, 1,
	1, 0, 1,
	1, 1, 1,
	0, 1, 1,
	// top
	0, 1, 1,
	1, 1, 1,
	1, 1, 0,
	0, 1, 0,
	// back
	1, 0, 0,
	0, 0, 0,
	0, 1, 0,
	1, 1, 0,
	// bottom
	0, 0, 0,
	1, 0, 0,
	1, 0, 1,
	0, 0, 1,
	// left
	0, 0, 0,
	0, 0, 1,
	0, 1, 1,
	0, 1, 0,
	// right
	1, 0, 1,
	1, 0, 0,
	1, 1, 0,
	1, 1, 1,
};

#define tex_coords_count array_length(tex_coords)

static GLushort indices[] = {
	// front
	0,  1,  2,
	2,  3,  0,
	// top
	4,  5,  6,
	6,  7,  4,
	// back
	8,  9, 10,
	10, 11,  8,
	// bottom
	12, 13, 14,
	14, 15, 12,
	// left
	16, 17, 18,
	18, 19, 16,
	// right
	20, 21, 22,
	22, 23, 20,
};

#define indices_count array_length(indices)

int main( int argc, char* argv[] )
{
	if( SDL_Init(SDL_INIT_VIDEO) < 0 )
	{
		goto quit;
	}

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 2 );

	int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
	//flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	window = SDL_CreateWindow( "Volume Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, flags );

	if( window == NULL )
	{
		dump_sdl_error( );
		goto quit;
	}

	ctx = SDL_GL_CreateContext( window );

	if( !ctx )
	{
		dump_sdl_error( );
		goto quit;
	}

	initialize( );

	SDL_Event e;

	while( e.type != SDL_KEYDOWN && e.type != SDL_QUIT )
	{
		SDL_PollEvent( &e );      // Check for events.
		render( );
		//SDL_Delay(10);              // Pause briefly before moving on to the next cycle.
	}

	deinitialize( );

quit:
	if( ctx ) SDL_GL_DeleteContext( ctx );
	if( window ) SDL_DestroyWindow( window );
	SDL_Quit( );
	return 0;
}

void initialize( void )
{
	dump_gl_info( );
	//glClearColor( 0.2f, 0.2f, 0.2f, 1.0f );
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
	glFrontFace( GL_CCW );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable( GL_LINE_SMOOTH );
	glEnable( GL_POLYGON_SMOOTH );
	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	int width; int height;
	SDL_GetWindowSize( window, &width, &height );
	glViewport( 0, 0, width, height );
	assert(check_gl() == GL_NO_ERROR);

	GLchar* shader_log  = NULL;
	GLchar* program_log = NULL;
	const shader_info_t shaders[] = {
		{ GL_VERTEX_SHADER,   "./tests/assets/shaders/volume-render.v.glsl" },
		{ GL_FRAGMENT_SHADER, "./tests/assets/shaders/volume-render.f.glsl" }
	};

	if( !glsl_program_from_shaders( &program, shaders, shader_info_count(shaders), &shader_log, &program_log ) )
	{
		if( shader_log )
		{
			printf( " [Shader Log] %s\n", shader_log );
			free( shader_log );
		}
		if( program_log )
		{
			printf( "[Program Log] %s\n", program_log );
			free( program_log );
		}

		exit( EXIT_FAILURE );
	}

	attribute_vertex = glsl_bind_attribute( program, "a_vertex" ); assert(check_gl() == GL_NO_ERROR);
	attribute_tex_coord = glsl_bind_attribute( program, "a_tex_coord" ); assert(check_gl() == GL_NO_ERROR);
	attribute_color  = glsl_bind_attribute( program, "a_color" ); assert(check_gl() == GL_NO_ERROR);
	uniform_model_view = glsl_bind_uniform( program, "u_model_view" ); assert(check_gl() == GL_NO_ERROR);
	uniform_voxel_data = glsl_bind_uniform( program, "u_voxel_data" ); assert(check_gl() == GL_NO_ERROR);
	uniform_back_voxels = glsl_bind_uniform( program, "u_back_voxels" ); assert(check_gl() == GL_NO_ERROR);
	uniform_color_transfer = glsl_bind_uniform( program, "u_color_transfer" ); assert(check_gl() == GL_NO_ERROR);
	uniform_rendering_pass = glsl_bind_uniform( program, "u_render_pass" ); assert(check_gl() == GL_NO_ERROR);
	uniform_seed = glsl_bind_uniform( program, "u_seed" ); assert(check_gl() == GL_NO_ERROR);

	voxel_texture = tex_create( );

	if( voxel_texture )
	{
		glActiveTexture( GL_TEXTURE0 );
		assert(check_gl() == GL_NO_ERROR);
		int flags = TEX_CLAMP_S | TEX_CLAMP_T | TEX_CLAMP_R;
		#if 0
		const char* filename = "./tests/assets/head-256x256x256.raw";
		if( tex_load_3d( voxel_texture, filename, 8, 256, 256, 256, GL_LINEAR, GL_LINEAR, flags ) )
		{
			printf( "Loaded %s\n", filename );
		}
		else
		{
			dump_sdl_error( );
			exit( EXIT_FAILURE );
		}
		#elif 1
		const char* filename = "./tests/assets/foot-256x256x256.raw";
		if( tex_load_3d( voxel_texture, filename, 8, 256, 256, 256, GL_LINEAR, GL_LINEAR, flags ) )
		{
			printf( "Loaded %s\n", filename );
		}
		else
		{
			dump_sdl_error( );
			exit( EXIT_FAILURE );
		}
		#elif 0
		const char* filename = "./tests/assets/skull-256x256x256.raw";
		if( tex_load_3d( voxel_texture, filename, 8, 256, 256, 256, GL_LINEAR, GL_LINEAR, flags ) )
		{
			printf( "Loaded %s\n", filename );
		}
		else
		{
			dump_sdl_error( );
			exit( EXIT_FAILURE );
		}
		#else
		const char* filename = "./tests/assets/lobster-301x324x56.raw";
		if( tex_load_3d( voxel_texture, filename, 8, 301, 324, 56, GL_LINEAR, GL_LINEAR, flags ) )
		{
			printf( "Loaded %s\n", filename );
		}
		else
		{
			dump_sdl_error( );
			exit( EXIT_FAILURE );
		}
		#endif

		assert(check_gl() == GL_NO_ERROR);
	}
	else
	{
		dump_sdl_error( );
	}

	color_transfer_texture = tex_create( );

	if( color_transfer_texture )
	{
		//glActiveTexture( GL_TEXTURE2 );
		assert(check_gl() == GL_NO_ERROR);
		if( !tex_load_1d( color_transfer_texture, "./tests/assets/intensity.png", GL_LINEAR, GL_LINEAR, TEX_CLAMP_S ) )
		{
			dump_sdl_error( );
			exit( EXIT_FAILURE );
		}

		assert(check_gl() == GL_NO_ERROR);
	}
	else
	{
		dump_sdl_error( );
	}

	glGenVertexArrays( 1, &vao );
	assert(check_gl() == GL_NO_ERROR);
	glBindVertexArray( vao );
	assert(check_gl() == GL_NO_ERROR);

	assert( vertex_count > 4 );
	assert( tex_coords_count > 4 );

	if( buffer_create( &vbo_vertices, vertices, sizeof(GLfloat), vertex_count, GL_ARRAY_BUFFER, GL_STATIC_DRAW ) )
	{
		assert(check_gl() == GL_NO_ERROR);
		glEnableVertexAttribArray( attribute_vertex );
		assert(check_gl() == GL_NO_ERROR);
		glVertexAttribPointer( attribute_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0 );
		assert(check_gl() == GL_NO_ERROR);
		glDisableVertexAttribArray( attribute_vertex );
		assert(check_gl() == GL_NO_ERROR);
	}
	else
	{
		dump_sdl_error( );
	}

	if( buffer_create( &vbo_tex_coords, tex_coords, sizeof(GLfloat), tex_coords_count, GL_ARRAY_BUFFER, GL_STATIC_DRAW ) )
	{
		assert(check_gl() == GL_NO_ERROR);
		glEnableVertexAttribArray( attribute_tex_coord );
		assert(check_gl() == GL_NO_ERROR);
		glVertexAttribPointer( attribute_tex_coord, 3, GL_FLOAT, GL_FALSE, 0, 0 );
		assert(check_gl() == GL_NO_ERROR);
		glDisableVertexAttribArray( attribute_tex_coord );
		assert(check_gl() == GL_NO_ERROR);
	}
	else
	{
		dump_sdl_error( );
	}

	if( buffer_create( &ibo_indices, indices, sizeof(GLushort), indices_count, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW ) )
	{
		assert(check_gl() == GL_NO_ERROR);
	}
	else
	{
		dump_sdl_error( );
	}

	/* Create FBO */
	{
		glGenFramebuffers( 1, &framebuffer );
		glBindFramebuffer( GL_FRAMEBUFFER, framebuffer );


		glGenRenderbuffers( 1, &colorRenderBuffer );
		glBindRenderbuffer( GL_RENDERBUFFER, colorRenderBuffer );
		glRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA8, width, height );
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderBuffer );
		assert(check_gl() == GL_NO_ERROR);


		GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
		if( status != GL_FRAMEBUFFER_COMPLETE )
		{
			printf( "Failed to make complete framebuffer object %x", status );
		}

		glGenTextures( 1, &back_voxel_texture );
		glBindTexture( GL_TEXTURE_2D, back_voxel_texture );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8,  width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, back_voxel_texture, 0 );

		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}

	glPointSize( 4.0 );
	assert(check_gl() == GL_NO_ERROR);
}

void deinitialize( void )
{
	tex_destroy( voxel_texture );
	glDeleteFramebuffers( 1, &framebuffer );
	glDeleteRenderbuffers( 1, &colorRenderBuffer );
	glDeleteTextures( 1, &back_voxel_texture );
	glDeleteTextures( 1, &color_transfer_texture );
	glDeleteVertexArrays( 1, &vao );
	glDeleteBuffers( 1, &vbo_vertices );
	glDeleteBuffers( 1, &vbo_tex_coords );
	glDeleteBuffers( 1, &ibo_indices );
	glDeleteProgram( program );
}

void render( )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	static float angle = -270.0;
	if( angle >= 360.0f ) angle = 0.0f;
	#if 0
	int width; int height;
	SDL_GetWindowSize( window, &width, &height );
	GLfloat aspect = ((GLfloat)width) / height;
	vec3_t translation = VEC3_LITERAL( 0.0, 0.0, -10 );
	mat4_t projection = orthographic( -10.0, 10.0, -6.0*aspect, 6.0*aspect, -100.0, 100.0 );
	mat4_t rotation = rotate_xyz( "yx", angle, -5.0 );
	angle += 0.1f;
	mat4_t transform = translate( &translation );
	transform = mat4_mult_matrix( &transform, &rotation );
	mat4_t model_view = mat4_mult_matrix( &projection, &transform );
	#else
	int width; int height;
	SDL_GetWindowSize( window, &width, &height );
	GLfloat aspect = ((GLfloat)width) / height;
	vec3_t translation = VEC3_LITERAL( 0.0, 0.0, -6 );
	mat4_t projection = perspective( 22.0 * RADIANS_PER_DEGREE, aspect, 5, 100.0 );
	vec3_t axis = VEC3_LITERAL( 0.0f, 1.0f, 0.0f );
	quat_t q1 = quat_from_axis3_angle( &axis, angle * RADIANS_PER_DEGREE );
	mat4_t rotation = quat_to_mat4( &q1 );
	angle += 0.3f;
	//mat4_t matscale  = scale( &VEC3_LITERAL(1.0f, 1.0f, 1.0f) );
	mat4_t transform = translate( &translation );
	transform = mat4_mult_matrix( &transform, &rotation );
	mat4_t model_view = mat4_mult_matrix( &projection, &transform );
	#endif


	assert(check_gl() == GL_NO_ERROR);
	glUseProgram( program );

	glEnableVertexAttribArray( attribute_vertex );
	glEnableVertexAttribArray( attribute_tex_coord );
	//glEnableVertexAttribArray( attribute_color );
	glUniformMatrix4fv( uniform_model_view, 1, GL_FALSE, model_view.m );
	assert(check_gl() == GL_NO_ERROR);
	glUniform1i( uniform_voxel_data, 0 ); // texture unit 0 holds the voxel data
	assert(check_gl() == GL_NO_ERROR);
	glUniform1i( uniform_back_voxels, 1 ); // texture unit 1 holds the backing facing voxels
	assert(check_gl() == GL_NO_ERROR);
	glUniform1i( uniform_color_transfer, 2 ); // texture unit 2 holds the color transfer function
	assert(check_gl() == GL_NO_ERROR);
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_3D, voxel_texture );
	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D, back_voxel_texture );
	glActiveTexture( GL_TEXTURE2 );
	glBindTexture( GL_TEXTURE_1D, color_transfer_texture );


	glBindVertexArray( vao );

	// First rendering pass
	glBindFramebuffer( GL_FRAMEBUFFER, framebuffer );
	glCullFace( GL_FRONT );
	glUniform1ui( uniform_rendering_pass, 0 );
	glDrawElements( GL_TRIANGLES, indices_count, GL_UNSIGNED_SHORT, 0 );
	glCullFace( GL_BACK );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	// Second rendering pass
	glUniform1ui( uniform_rendering_pass, 1 );
	glUniform1ui( uniform_seed, clock() );
	glDrawElements( GL_TRIANGLES, indices_count, GL_UNSIGNED_SHORT, 0 );
	assert(check_gl() == GL_NO_ERROR);


	glDisableVertexAttribArray( attribute_vertex );
	glDisableVertexAttribArray( attribute_tex_coord );
	//glDisableVertexAttribArray( attribute_color );


	SDL_GL_SwapWindow( window );
}

void dump_sdl_error( void )
{
	const char* sdl_error = SDL_GetError( );

	if( sdl_error && *sdl_error != '\0' )
	{
		fprintf( stderr, "[SDL] %s\n", sdl_error );
	}
}
