/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*

  exec.cpp

  This is an example of GL_ARB_multitexture, described
  on page 18 of the NVidia OpenGL Extension Specifications.

  Textures and algorithm ideas, thanks to Tashco Studios:
	http://www.tashco.com/

  Christopher Niederauer, ccn@graphics.stanford.edu, 6/25/2001

*/

/* --- Preprocessor --------------------------------------------------------- */

#include "exec.h"
#include "../common/logo.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>


#ifndef DISPLAY_LISTS
#error This program requires display lists to do anything :-(
#endif


enum
  {
    eGrassTex = 0,
    eSandTex,
    numTextures
  };

enum
  {
    eGrassList = 1,
    eSandList,
    eShadowList,
    eMultiTexList
  };


/* --- Global Variables ----------------------------------------------------- */

static GLuint	currentWidth, currentHeight;
static GLuint	textureID[numTextures];
static GLfloat	bgColor[4] = { 0.4, 0.7, 1.0, 0.0 };

#ifdef WINDOWS
PFNGLMULTITEXCOORD2FARBPROC	glMultiTexCoord2fARB;
PFNGLACTIVETEXTUREARBPROC	glActiveTextureARB;
#endif


/* --- Function Definitions ------------------------------------------------- */

void	InitGL	( void )
{
  currentWidth = 320;
  currentHeight = 240;

#ifdef MULTIPLE_VIEWPORTS
  currentWidth <<= 1;
#endif

  glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
  glutInitWindowPosition( 5, 25 );
  glutInitWindowSize( currentWidth, currentHeight );
  glutCreateWindow( TEST_EXTENSION_STRING );
	
  glClearColor( bgColor[0], bgColor[1], bgColor[2], bgColor[3] );
  glClear( GL_COLOR_BUFFER_BIT );
	
  glutIdleFunc( Idle );
  glutDisplayFunc( Display );
  glutReshapeFunc( Reshape );
  glutKeyboardFunc( Keyboard );
  glutMouseFunc( NULL );
  glutMotionFunc( NULL );
  glutSpecialFunc( NULL );
}


void	InitSpecial	( void )
{
  glBindTexture( GL_TEXTURE_2D, textureID[1]);
  glEnable( GL_POLYGON_OFFSET_FILL );
  glEnable( GL_BLEND );

  glPolygonOffset( 0, -1 );
  //	glCallList( 2 );

  glPolygonOffset( 0, -2 );
  glDisable( GL_TEXTURE_2D );
  //	glCallList( 3 );
  glDisable( GL_POLYGON_OFFSET_FILL );

  GLint	numTexUnits,
    currentActiveUnit;
  GLint	x, y;

  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LEQUAL );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glFogfv( GL_FOG_COLOR, bgColor );
  glFogf( GL_FOG_START, 5 );
  glFogf( GL_FOG_END, 30 );
  glFogf( GL_FOG_MODE, GL_LINEAR );
	
  glGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, &numTexUnits ); // Up to 4
  glGetIntegerv( GL_ACTIVE_TEXTURE_ARB, &currentActiveUnit );
  cout << "MAX_TEXTURE_UNITS_ARB = " << numTexUnits << endl;
  if( numTexUnits < 3 )
    {
      cout << "Sorry, this program requires at least three texture units to work properly." << endl;
      exit( 0 );
    }
  cout << "ACTIVE_TEXTURE_ARB = ";
  switch( currentActiveUnit )
    {
    case GL_TEXTURE0_ARB:
      cout << "TEXTURE0_ARB";
      break;
    case GL_TEXTURE1_ARB:
      cout << "TEXTURE1_ARB";
      break;
    case GL_TEXTURE2_ARB:
      cout << "TEXTURE2_ARB";	
      break;
    case GL_TEXTURE3_ARB:
      cout << "TEXTURE3_ARB";
      break;
    default:
      cout << "Oh no! (0x" << hex << currentActiveUnit << ")";
      break;
    }
  cout << endl << endl;
#ifdef WIN32
  glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress( "glMultiTexCoord2fARB" );
  glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress( "glActiveTextureARB" );
  if( !glMultiTexCoord2fARB || !glActiveTextureARB )
    {
      cout << "Error trying to link to extensions!" << endl;
      exit( 0 );
    }
#endif
  // Gets of CURRENT_TEXTURE_COORDS return that of the active unit.
  // ActiveTextureARB( enum texture ) changes active unit for:
  // 	- Seperate Texture Matrices
  // 	- TexEnv calls
  //	- Get CURRENT_TEXTURE_COORDS

    // Create the tile texture.
  glGenTextures( 2, textureID );

  // Load the textures.
  {
    const int	texmapX = 128,
      texmapY = 128,
      texmapSize = texmapX*texmapY*3;
    FILE		*file;
    GLubyte		textureData[ texmapSize ];
    // Load grass texture.
    {
      if(( file = fopen( "terrain1.raw", "rb" )) == NULL )
	{
	  cout << "Error opening file: terrain1.raw" << endl;
	  file = NULL;
	}
      else
	{
	  fread( textureData, texmapSize, 1, file );
	  fclose( file );
				
				// Create Trilinear MipMapped Noise Texture
	  glBindTexture( GL_TEXTURE_2D, textureID[eGrassTex] );
	  glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
	  glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR );
	  gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGB8, texmapX, texmapY, GL_RGB, GL_UNSIGNED_BYTE, textureData );
	}
    }
    // Load sand texture.
    {
      if(( file = fopen( "terrain2.raw", "rb" )) == NULL )
	{
	  cout << "Error opening file: terrain2.raw" << endl;
	  file = NULL;
	}
      else
	{
	  fread( textureData, texmapSize, 1, file );
	  fclose( file );
				
				// Create Trilinear MipMapped Circle Texture
	  glBindTexture( GL_TEXTURE_2D, textureID[eSandTex] );
	  glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
	  glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR );
	  gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGB8, texmapX, texmapY, GL_RGB, GL_UNSIGNED_BYTE, textureData );
	}
    }
  }
  // Load heightmap data.
  {
    const int	heightmapX = 128,
      heightmapY = 128,
      heightNormRow = 3*heightmapX;
    const float	size = 0.5,
      sizeV = 0.05,
      offsetV = -10,
      texScale = 0.15;
		
    FILE	*file;
    GLubyte	height[heightmapX*heightmapY];
    GLfloat	normals[heightmapX*heightmapY*3],
      vec1[3], vec2[3];

    file = fopen( "height.raw", "rb" );
    if( file == NULL )
      {
	cout << "Error opening file: height.raw!  Exiting." << endl;
	exit( 0 );
      }
    fread( height, heightmapX*heightmapY, 1, file );
    fclose( file );
    
    // Create the normals.
    for( y=0; y<heightmapY; y++ )
      {
	for( x=0; x<heightmapX; x++ )
	  {
				// Sets all normals to 0,1,0 by default.
	    normals[ y*heightNormRow + x*3 + 1 ] = 1;
	  }
      }
    for( y=1; y<heightmapY-1; y++ )
      {
	for( x=1; x<heightmapX-1; x++ )
	  {
	    register float	i, j, k;

	    vec1[0] = size*2;
	    vec1[1] = sizeV*(height[y*heightmapX+(x+1)]-height[y*heightmapX+(x-1)]);
	    vec1[2] = 0;
				
	    vec2[0] = 0;
	    vec2[1] = sizeV*(height[(y+1)*heightmapX+x]-height[(y-1)*heightmapX+x]);
	    vec2[2] = size*2;
				
	    i = -vec1[1]*vec2[2]+vec2[1]*vec1[2];
	    j = -vec1[2]*vec2[0]+vec2[2]*vec1[0];
	    k = -vec1[0]*vec2[1]+vec2[0]*vec1[1];
				
	    float magInv = 1.0 / sqrt( i*i + j*j + k*k );
				
	    normals[ y*heightNormRow + x*3 + 0 ] = i*magInv;
	    normals[ y*heightNormRow + x*3 + 1 ] = j*magInv;
	    normals[ y*heightNormRow + x*3 + 2 ] = k*magInv;
	  }
      }

#ifdef DISPLAY_LISTS
    glNewList( eGrassList, GL_COMPILE );
    for( y=0; y<heightmapY-1; y++ )
      {
	glBegin( GL_QUAD_STRIP );
	for( x=0; x<heightmapX; x++ )
	  {
	    glTexCoord2f( x*texScale, y*texScale );
	    glVertex3f( size*(x-(heightmapX>>1)), sizeV*height[y*heightmapX+x]+offsetV, size*(y-(heightmapY>>1)) );
				
	    glTexCoord2f( x*texScale, (y+1)*texScale );
	    glVertex3f( size*(x-(heightmapX>>1)), sizeV*height[(y+1)*heightmapX+x]+offsetV, size*(y+1-(heightmapY>>1)) );
	  }
	glEnd();
      }
    glEndList();
    glNewList( eSandList, GL_COMPILE );
    for( y=0; y<heightmapY-1; y++ )
      {
	glBegin( GL_QUAD_STRIP );
	for( x=0; x<heightmapX; x++ )
	  {
	    GLfloat	normalThreshold1 = (normals[y*heightNormRow+x*3+1]-0.5)*1.8,
	      normalThreshold2 = (normals[(y+1)*heightNormRow+x*3+1]-0.5)*1.8;
				
	    glColor4f( 1, 1, 1, 1.0 - (normalThreshold1 > 1.0 ? 1 : normalThreshold1 < 0 ? 0 : normalThreshold1 ));
	    glTexCoord2f( x*texScale, y*texScale );
	    glVertex3f( size*(x-(heightmapX>>1)), sizeV*height[y*heightmapX+x]+offsetV, size*(y-(heightmapY>>1)) );
				
	    glColor4f( 1, 1, 1, 1.0 - (normalThreshold2 > 1.0 ? 1 : normalThreshold2 < 0 ? 0 : normalThreshold2 ));
	    glTexCoord2f( x*texScale, (y+1)*texScale );
	    glVertex3f( size*(x-(heightmapX>>1)), sizeV*height[(y+1)*heightmapX+x]+offsetV, size*(y+1-(heightmapY>>1)) );
	  }
	glEnd();
      }
    glEndList();
    glNewList( eShadowList, GL_COMPILE );
    for( y=0; y<heightmapY-1; y++ )
      {
	glBegin( GL_QUAD_STRIP );
	for( x=0; x<heightmapX; x++ )
	  {
	    glColor4f( 0, 0, 0, (normals[y*heightNormRow+x*3+2]));
	    glVertex3f( size*(x-(heightmapX>>1)), sizeV*height[y*heightmapX+x]+offsetV, size*(y-(heightmapY>>1)) );
				
	    glColor4f( 0, 0, 0, (normals[(y+1)*heightNormRow+x*3+2]));
	    glVertex3f( size*(x-(heightmapX>>1)), sizeV*height[(y+1)*heightmapX+x]+offsetV, size*(y+1-(heightmapY>>1)) );
	  }
	glEnd();
      }
    glEndList();
		
    // Multitexturing
    glNewList( eMultiTexList, GL_COMPILE );
    
    GLfloat blackColor[4] = {0,0,0,0};
    
    glActiveTextureARB( GL_TEXTURE0_ARB );
    glBindTexture( GL_TEXTURE_2D, textureID[eGrassTex] );
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    
    glActiveTextureARB( GL_TEXTURE1_ARB );
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, textureID[eSandTex] );
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    
    glActiveTextureARB( GL_TEXTURE2_ARB );
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, 0 );
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, blackColor );

    const GLfloat shade = 1.0, sandAlpha = 0;

    for( y=0; y<heightmapY-1; y++ )
      {
	glBegin( GL_QUAD_STRIP );
	for( x=0; x<heightmapX; x++ )
	  {
	    glColor4f( shade, shade, shade, sandAlpha );
	    glMultiTexCoord2fARB( GL_TEXTURE0_ARB, x*texScale, y*texScale );
	    glMultiTexCoord2fARB( GL_TEXTURE1_ARB, x*texScale, y*texScale );
	    //glMultiTexCoord2fARB( GL_TEXTURE2_ARB, x*texScale, y*texScale );
	    glVertex3f( size*(x-(heightmapX>>1)), sizeV*height[y*heightmapX+x]+offsetV, size*(y-(heightmapY>>1)) );
	    
	    glColor4f( shade, shade, shade, sandAlpha );
	    glMultiTexCoord2fARB( GL_TEXTURE0_ARB, x*texScale, (y+1)*texScale );
	    glMultiTexCoord2fARB( GL_TEXTURE1_ARB, x*texScale, (y+1)*texScale );
	    //glMultiTexCoord2fARB( GL_TEXTURE2_ARB, x*texScale, (y+1)*texScale );
	    glVertex3f( size*(x-(heightmapX>>1)), sizeV*height[(y+1)*heightmapX+x]+offsetV, size*(y+1-(heightmapY>>1)) );
	  }	
	glEnd();
      }
    glActiveTextureARB( GL_TEXTURE1_ARB );
    glDisable( GL_TEXTURE_2D );
    glActiveTextureARB( GL_TEXTURE0_ARB );
    glEndList();
#endif /* DISPLAY_LISTS */
  }

  return;
}


void	Idle		( void )
{
  glutPostRedisplay();
  return;
}


void	Display		( void )
{
  static double	theta = 0.0;

  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
  theta += 0.1;

  glLoadIdentity();
  glTranslatef( 0.0, -2.0, 0.0 );
  glRotated( 30.0, 1.0, 0.0, 0.0 );
  glRotated( theta, 0.0, 1.0, 0.0 );
  glColor3f( 1.0, 1.0, 1.0 );

#ifdef MULTIPLE_VIEWPORTS
  // Left viewport
  glViewport( 0, 0, currentWidth >> 1, currentHeight );
  glEnable( GL_FOG );
  glEnable( GL_TEXTURE_2D );
  glColor3f( 1, 1, 1 );
	
  glBindTexture( GL_TEXTURE_2D, textureID[eGrassTex]);
  glCallList( eGrassList );
	
  glBindTexture( GL_TEXTURE_2D, textureID[eSandTex]);
  glEnable( GL_POLYGON_OFFSET_FILL );
  glEnable( GL_BLEND );
  glPolygonOffset( 0, -1 );
  glCallList( eSandList );

  glPolygonOffset( 0, -2 );
  glDisable( GL_TEXTURE_2D );
  glCallList( eShadowList );
  glDisable( GL_POLYGON_OFFSET_FILL );

  glBegin( GL_QUADS );
  glColor4f( 0.0, 0.0, 0.8, 0.7 );
  glVertex3f( -50, -7, -50 );
  glVertex3f( -50, -7,  50 );
  glVertex3f(  50, -7,  50 );
  glVertex3f(  50, -7, -50 );
  glEnd();

  glDisable( GL_BLEND );
  glDisable( GL_FOG );
  glDisable( GL_TEXTURE_2D );

  glPushMatrix();glLoadIdentity();
  glColor3f( 0, 0, 0 );
  RenderString( -1.1, 1, "Multiple Passes (3)" );
  glPopMatrix();

  // Right viewport
  glViewport( currentWidth >> 1, 0, currentWidth - (currentWidth>>1), currentHeight );
#endif /* MULTIPLE_VIEWPORTS */

  glEnable( GL_FOG );
  glEnable( GL_TEXTURE_2D );
  glColor3f( 1, 1, 1 );
  glBindTexture( GL_TEXTURE_2D, textureID[0]);
  glCallList( eMultiTexList );
	
  glEnable( GL_BLEND );
  glBegin( GL_QUADS );
  glColor4f( 0.0, 0.0, 0.8, 0.7 );
  glVertex3f( -50, -7, -50 );
  glVertex3f( -50, -7,  50 );
  glVertex3f(  50, -7,  50 );
  glVertex3f(  50, -7, -50 );
  glEnd();
	
  glDisable( GL_BLEND );
  glDisable( GL_FOG );
  glDisable( GL_TEXTURE_2D );
	
  glPushMatrix();glLoadIdentity();
  glColor3f( 0, 0, 0 );
  RenderString( -1.1, 1, "Single Pass" );
  glPopMatrix();

  glViewport( 0, 0, currentWidth, currentHeight );
  crExtensionsDrawLogo( currentWidth, currentHeight );

  glutSwapBuffers();
}

void	Reshape		( int width, int height )
{
  currentWidth = width;
  currentHeight = height;

  glViewport( 0, 0, currentWidth, currentHeight );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glFrustum( -1.0, 1.0, -1.0, 1.0, 1.0, 40.0 );
  glMatrixMode( GL_MODELVIEW );
}


void	Keyboard	( unsigned char key, int, int )
{
  switch( key )
    {
    case 'Q':
    case 'q':
      cout << "User has quit. Exiting." << endl;
      exit( 0 );
    default:
      break;
    }
  return;
}
