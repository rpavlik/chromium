# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.


import sys

sys.path.append( "../../glapi_parser" )
import apiutil


apiutil.CopyrightC()


print """#include <stdio.h>
#include "cr_error.h"
#include "cr_spu.h"
#include "cr_glstate.h"
#include "state/cr_statetypes.h"
#include "nopspu.h"
#include "cr_mem.h"

#if defined(WINDOWS)
#define NOP_APIENTRY __stdcall
#else
#define NOP_APIENTRY
#endif

#define NOP_UNUSED(x) ((void)x)"""


keys = apiutil.GetDispatchedFunctions("../../glapi_parser/APIspec.txt")

for func_name in keys:
	if not (apiutil.FindSpecial( "nop", func_name) or
			"get" in apiutil.Properties(func_name)):
		return_type = apiutil.ReturnType(func_name)
		params = apiutil.Parameters(func_name)
		print '\nstatic %s NOP_APIENTRY nop%s( %s )' % (return_type, func_name, apiutil.MakeDeclarationString(params))
		print '{'
		# Handle the void parameter list
		for (name, type, vecSize) in params:
			print '\tNOP_UNUSED(%s);' % name

		if return_type != "void":
			print '\treturn (%s)0;' % return_type
		print '}'


print """
static GLint NOP_APIENTRY nopCreateContext( const char *dpyName, GLint visual )
{
	static int slot = 0;

	/* generate a sequential window ID */
	if (nop_spu.return_ids)
		slot++;

	return slot;
}
static GLint NOP_APIENTRY nopWindowCreate( const char *dpyName, GLint visBits )
{
	static int slot = 0;

	/* generate a sequential window ID */
	if (nop_spu.return_ids)
		slot++;

	return slot;
}
static void NOP_APIENTRY nopGetChromiumParametervCR( GLenum target, GLuint index, GLenum type, GLsizei count, GLvoid *values )
{
	switch( target )
	{
		case GL_HEAD_SPU_NAME_CR:
			crMemcpy((char*)values,"nop",3);
			return;
		case GL_WINDOW_SIZE_CR:
			{
				int *size = (int *) values;
				size[0] = size[1] = 1;
			}
			break;
		default:
			break;
	}
}

static void NOP_APIENTRY nopReadPixels(GLint x, GLint y, GLsizei width, 
																		 GLsizei height, GLenum format,
																		 GLenum type, GLvoid *pixels,
																		 const CRPixelPackState *packstate,
																		 int *writeback)
{
  /* nothing */
}

#if 0
static PICAcontextID NOP_APIENTRY nopPicaCreateContext(const PICAuint *config, 
                                                     const PICAnodeID *nodes, 
                                                     PICAuint numNodes)
{
    return 0;
}

static PICAerror NOP_APIENTRY nopPicaGetCompositorParamiv(PICAcompID compositor,
                                                     PICAparam pname,
                                                     PICAint *params)
{
    return 0;
}

static PICAerror NOP_APIENTRY nopPicaGetCompositorParamfv(PICAcompID compositor,
                                                     PICAparam pname,
                                                     PICAfloat *params)
{
    return 0;
}

static PICAerror NOP_APIENTRY nopPicaGetCompositorParamcv(PICAcompID compositor,
                                                     PICAparam pname,
                                                     PICAchar **params)
{
    return 0;
}

static PICAerror NOP_APIENTRY nopPicaGetContextParamiv(PICAcontextID ctx,
                                                  PICAparam pname,
                                                  PICAint *param)
{
    return 0;
}
	
static PICAerror NOP_APIENTRY nopPicaGetContextParamfv(PICAcontextID ctx,
                                                  PICAparam pname,
                                                  PICAfloat *param)
{
    return 0;
}

static PICAerror NOP_APIENTRY nopPicaGetContextParamcv(PICAcontextID ctx,
                                                  PICAparam pname,
                                                  PICAchar **param)
{
    return 0;
}

static PICAerror NOP_APIENTRY nopPicaGetContextParamv(PICAcontextID ctx,
                                                 PICAparam pname,
                                                 PICAvoid *param)
{
    return 0;
}

static PICAerror NOP_APIENTRY nopPicaListCompositors(const PICAuint *config, 
                                                PICAint *numResults,
                                                PICAcompItem *results)
{
    return 0;
}

static PICAerror NOP_APIENTRY nopPicaListNodes(PICAcompID compositor, 
                                          PICAint *num,
                                          PICAnodeItem *results)
{
    return 0;
}

static PICAstatus NOP_APIENTRY nopPicaQueryFrame(PICAcontextID ctx,
                                            PICAframeID frameID,
                                            PICAnodeID nodeID,
                                            PICAfloat timeout)
{
    return 0;
}

static PICAerror NOP_APIENTRY nopPicaReadFrame(PICAcontextID lctx,
                                          PICAframeID frameID,
                                          PICAuint format,
                                          PICAvoid *colorbuffer,
                                          PICAvoid *depthbuffer,
                                          const PICArect *rect)
{
    return 0;
}
#endif


"""

print 'SPUNamedFunctionTable _cr_nop_table[] = {'
for func_name in keys:
	if apiutil.FindSpecial( "nop", func_name):
		print '\t{ "%s", (SPUGenericFunction) nop%s },' % (func_name, func_name )
		#print '\t{ "%s", (SPUGenericFunction) nopGeneric },' % (func_name)
	elif "get" in apiutil.Properties(func_name):
		print '\t{ "%s", (SPUGenericFunction) crState%s },' % (func_name, func_name )
	else:
		print '\t{ "%s", (SPUGenericFunction) nop%s },' % (func_name, func_name )
print '\t{ NULL, NULL }'
print '};'
