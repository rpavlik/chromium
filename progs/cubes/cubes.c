/* progs/cubes.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <GL/glut.h>

#include "timer.h"

float *data;

void ComputeCube( int dice )
{
    int i,j;
    float step = 2.0f/dice;
    float *temp;

    data = (float *) malloc( dice*dice*8*sizeof(float) );
    temp = data;
    for (i = 0 ; i < dice ; i++)
    {
        float x = ((float)i)/((float) dice)* 2 - 1;
        for (j = 0 ; j < dice ; j++)
        {
            float y = ((float)j)/((float) dice)* 2 - 1;

            *(temp++) = x;        *(temp++) = y;
            *(temp++) = x + step; *(temp++) = y;
            *(temp++) = x;        *(temp++) = y + step;
            *(temp++) = x + step; *(temp++) = y + step;
        }
    }
}

void DrawCubeXX( int dice )
{
    int i, j, start, finish, verts_per_strip, bytes_per_strip, strips, bytes;
    float ii, jj, delta = 2.0f / (float) dice;
    float seconds, verts_per_second, bytes_per_second;

    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glDisable( GL_CULL_FACE );

    start = glutGet( (GLenum) GLUT_ELAPSED_TIME );

    glColor3f( 1.0f, 0.0, 0.0 );
    for ( i = 0, ii = -1.0f; i < dice; i++, ii += delta ) {
        glBegin( GL_QUAD_STRIP );
        for ( j = 0, jj = -1.0f; j <= dice; j++, jj += delta ) {
            glVertex3f( ii, jj, -1.0f );
            glVertex3f( ii+delta, jj, -1.0f );
        }
        glEnd( );
    }

    for ( i = 0, ii = -1.0f; i < dice; i++, ii += delta ) {
        glBegin( GL_QUAD_STRIP );
        for ( j = 0, jj = -1.0f; j <= dice; j++, jj += delta ) {
            glVertex3f( ii, jj, 1.0f );
            glVertex3f( ii+delta, jj, 1.0f );
        }
        glEnd( );
    }

    glColor3f( 0.0f, 1.0f, 0.0f );
    for ( i = 0, ii = -1.0f; i < dice; i++, ii += delta ) {
        glBegin( GL_QUAD_STRIP );
        for ( j = 0, jj = -1.0f; j <= dice; j++, jj += delta ) {
            glVertex3f( ii, -1.0f, jj );
            glVertex3f( ii+delta, -1.0f, jj );
        }
        glEnd( );
    }

    for ( i = 0, ii = -1.0f; i < dice; i++, ii += delta ) {
        glBegin( GL_QUAD_STRIP );
        for ( j = 0, jj = -1.0f; j <= dice; j++, jj += delta ) {
            glVertex3f( ii, 1.0f, jj );
            glVertex3f( ii+delta, 1.0f, jj );
        }
        glEnd( );
    }

    glColor3f( 0.0f, 0.0f, 1.0f );
    for ( i = 0, ii = -1.0f; i < dice; i++, ii += delta ) {
        glBegin( GL_QUAD_STRIP );
        for ( j = 0, jj = -1.0f; j <= dice; j++, jj += delta ) {
            glVertex3f( -1.0f, ii, jj );
            glVertex3f( -1.0f, ii+delta, jj );
        }
        glEnd( );
    }

    for ( i = 0, ii = -1.0f; i < dice; i++, ii += delta ) {
        glBegin( GL_QUAD_STRIP );
        for ( j = 0, jj = -1.0f; j <= dice; j++, jj += delta ) {
            glVertex3f( 1.0f, ii, jj );
            glVertex3f( 1.0f, ii+delta, jj );
        }
        glEnd( );
    }

    finish = glutGet( (GLenum) GLUT_ELAPSED_TIME );

    verts_per_strip = 2 * ( dice + 1 );
    bytes_per_strip = 5 + verts_per_strip * 13 + 5;
    strips = 6 * dice;
    bytes = 3 * 13 + strips * bytes_per_strip;

    seconds = ( finish - start ) * 0.001f;

    verts_per_second = (float) ( verts_per_strip * strips ) / seconds;
    bytes_per_second = (float) bytes / seconds;

    printf( "seconds=%.3f bytes=%d  rate=%.3f Mvert/s, %.3f MB/s\n",
            seconds, bytes, 1e-6 * verts_per_second,
            1e-6 * bytes_per_second );
}

void DrawCube( GLenum mode, int dice )
{
    int    i, j;
    float *temp = data;

    /* glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); */

    glColor3f( 1.0, 1.0, 0.0 );
    glBegin( mode );
    for (i = 0 ; i < dice ; i++)
    {
        for (j = 0 ; j < dice ; j++)
        {
            glVertex3f( temp[0], temp[1], 1 );
            glVertex3f( temp[2], temp[3], 1 );
            glVertex3f( temp[4], temp[5], 1 );
            glVertex3f( temp[6], temp[7], 1 );
            /*glEnd(); 
             *glBegin( GL_TRIANGLE_STRIP ); */
            glVertex3f( temp[0], temp[1], -1 );
            glVertex3f( temp[2], temp[3], -1 );
            glVertex3f( temp[4], temp[5], -1 );
            glVertex3f( temp[6], temp[7], -1 );
            /*glEnd(); 
             *glBegin( GL_TRIANGLE_STRIP ); */
            glVertex3f( temp[0], 1, temp[1] );
            glVertex3f( temp[2], 1, temp[3] );
            glVertex3f( temp[4], 1, temp[5] );
            glVertex3f( temp[6], 1, temp[7] );
            /*glEnd(); 
             *glBegin( GL_TRIANGLE_STRIP ); */
            glVertex3f( temp[0], -1, temp[1] );
            glVertex3f( temp[2], -1, temp[3] );
            glVertex3f( temp[4], -1, temp[5] );
            glVertex3f( temp[6], -1, temp[7] );
            /*glEnd(); 
             *glBegin( GL_TRIANGLE_STRIP ); */
            glVertex3f( 1, temp[0], temp[1] );
            glVertex3f( 1, temp[2], temp[3] );
            glVertex3f( 1, temp[4], temp[5] );
            glVertex3f( 1, temp[6], temp[7] );
            /*glEnd(); 
             *glBegin( GL_TRIANGLE_STRIP ); */
            glVertex3f( -1, temp[0], temp[1] );
            glVertex3f( -1, temp[2], temp[3] );
            glVertex3f( -1, temp[4], temp[5] );
            glVertex3f( -1, temp[6], temp[7] );
            temp += 8;
        }
    }
    glEnd();
}

Timer timer;
int   frame_count = 0;

void
PrintPerformance( int dice )
{
	int    vertexes, bytes;
    double verts_per_second, bytes_per_second, elapsed;

	frame_count++;
    elapsed = TimerTime( &timer );
	if ( elapsed < 1.0 )
		return;

	TimerReset( &timer );
	TimerStart( &timer );

    vertexes = frame_count * 24 * dice * dice;
    bytes    = vertexes * 13;
    verts_per_second = (double) vertexes / elapsed;
    bytes_per_second = (double) bytes    / elapsed;

    printf( "frames=%2d seconds=%.3f bytes=%d  rate=%.3f Mvert/s, %.3f MB/s\n",
            frame_count, elapsed, bytes, 1e-6 * verts_per_second,
            1e-6 * bytes_per_second );

	frame_count = 0;
}

int angle = 0;
int dice;
GLenum mode;

void Display( void )
{
	PrintPerformance( dice );

    glClearColor( 0.5, 0, 0, 0 );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( -10, 10, -10, 10, -10, 10 );
    glClear( GL_COLOR_BUFFER_BIT );
    glDisable( GL_DEPTH_TEST );
    glDepthMask( 0 );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    /* glTranslatef( -5, 0, 0 ); */
    glRotatef( (float) angle, 0, 0, 1 );
    glRotatef( (float) angle++, 1, 1, 0 );
    glScalef( 3.0f,3.0f,3.0f );
    DrawCube( mode, dice );
	glutPostRedisplay();
    glutSwapBuffers();
}

void Keyboard( unsigned char key, int x, int y )
{
    (void) key;
    (void) x;
    (void) y;
    exit( 0 );
}

void Reshape( int width, int height )
{
    /* Need this so that GLUT doesn't call ViewPort on us. */
    (void) width;
    (void) height;
}

void
usage( const char *argv0 )
{
	fprintf( stderr, "%s [-points] [-strip] [<dice>]\n", argv0 );
}

int main(int argc, char *argv[])
{
	int i;

    glutInitWindowSize( 256, 256 );
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA );

	mode = GL_TRIANGLE_STRIP;
	dice = 1;

	for ( i = 1; i < argc; i++ )
	{
		if ( !strcmp( argv[i], "-points" ) )
		{
			mode = GL_POINTS;
		}
		else if ( !strcmp( argv[i], "-strip" ) )
		{
			mode = GL_TRIANGLE_STRIP;
		}
		else if ( isdigit( argv[i][0] ) )
		{
			dice = atoi( argv[i] );
		}
		else if ( !strcmp( argv[i], "-h" ) || !strcmp( argv[i], "-help" ) )
		{
			usage( argv[0] );
			exit( 0 );
		}
		else
		{
			fprintf( stderr, "bad argument: %s\n", argv[i] );
			usage( argv[0] );
			exit( 1 );
		}
	}

    printf( "mode=%s dice=%d\n", 
			( mode == GL_POINTS ) ? "GL_POINTS" : "GL_TRIANGLE_STRIP", 
			dice );

    ComputeCube( dice );

    glutCreateWindow( "cubes" );
    glutDisplayFunc( Display );
    glutKeyboardFunc( Keyboard );
	glutReshapeFunc( Reshape );

	TimerInit( &timer );
	TimerReset( &timer );
	TimerStart( &timer );

    glutMainLoop();
    return 0;
}
