/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */


#include "cr_error.h"
#include "cr_environment.h"
#include "cr_pack.h"
#include "packer.h"
#include <stdarg.h>

void crPackErrorHandlerFunc( CRPackContext *pc, CRPackErrorHandlerFunc errf )
{
	pc->Error = errf;
}

void __PackError( int line, const char *file, GLenum error, char *format, ... )
{
	GET_PACKER_CONTEXT(pc);
	char errstr[8096];
	va_list args;

	if (pc->Error)
		pc->Error( error );

	if (crGetenv("CR_DEBUG"))
	{
		char *glerr;
		va_start( args, format );
		vsprintf( errstr, format, args );
		va_end( args );

		switch (error) {
		case GL_NO_ERROR:
			glerr = "GL_NO_ERROR";
	    		break;
		case GL_INVALID_VALUE:
	    		glerr = "GL_INVALID_VALUE";
	    		break;
		case GL_INVALID_ENUM:
	    		glerr = "GL_INVALID_ENUM";
	    		break;
		case GL_INVALID_OPERATION:
	    		glerr = "GL_INVALID_OPERATION";
	    		break;
		case GL_STACK_OVERFLOW:
	    		glerr = "GL_STACK_OVERFLOW";
	    		break;
		case GL_STACK_UNDERFLOW:
	    		glerr = "GL_STACK_UNDERFLOW";
	    		break;
		case GL_OUT_OF_MEMORY:
	    		glerr = "GL_OUT_OF_MEMORY";
	    		break;
		case GL_TABLE_TOO_LARGE:
			glerr = "GL_TABLE_TOO_LARGE";
			break;
		default:
	    		glerr = "unknown";
	    		break;
		}

		crWarning( "GL error in packer: %s, line %d: %s: %s",
						 file, line, glerr, errstr );
	}
}
