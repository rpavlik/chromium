/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mem.h"
#include "cr_error.h"
#include "cr_string.h"
#include <stdio.h>
#include <ctype.h>

int main ( int argc, char *argv[] )
{
	FILE	*fp = NULL;
	unsigned char  *textureData = NULL;

	char *filename;
	unsigned char *temp;
	int width;
	int height;
	int i;

	if (argc != 5)
	{
		crError( "Usage: %s <file.raw> <width> <height> <outputfile.h>" );
	}

	filename = argv[1];
	width = crStrToInt( argv[2] );
	height = crStrToInt( argv[3] );

	fp = fopen( filename, "rb" );
	if (!fp)
	{
		crError( "Couldn't open %s", filename );
	}

	textureData = (unsigned char*) crAlloc( 4*width*height );
	fread( textureData, 4*width*height, 1, fp );
	fclose( fp );

	filename = argv[4];
	fp = fopen( filename, "wb" );
	if (!fp)
	{
		crError( "Couldn't open %s", filename );
	}

	for (temp = (unsigned char * )filename ; *temp ; temp++)
	{
		if (*temp == '.') *temp = '_';
		else *temp = (unsigned char) toupper(*temp);
	}

	fprintf( fp, "#ifndef %s\n#define %s\n\n", filename, filename );

	fprintf( fp, "#define %s_WIDTH %d\n", filename, width ); 
	fprintf( fp, "#define %s_HEIGHT %d\n\n", filename, height ); 

	fprintf( fp, "static unsigned char raw_bytes[] = {\n" );

	i = 0;
	for (temp = textureData; temp < textureData + width*height*4; temp++)
	{
		fprintf( fp, "0x%02x", *temp );
		if (temp != textureData + width*height*4 - 1)
		{
			fprintf( fp, "," );
		}
		i++;
		if (!(i%12))
		{
			fprintf( fp, "\n" );
		}
		else
		{
			fprintf( fp, " " );
		}
	}

	fprintf( fp, "};\n\n" );

	fprintf( fp, "#endif /* %s */\n", filename );
	fclose(fp);

	crFree( textureData );
}
