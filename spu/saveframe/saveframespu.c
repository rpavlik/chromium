/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include <assert.h>

#include "cr_error.h"
#include "cr_string.h"
#include "cr_spu.h"
#include "cr_mem.h"
#include "saveframespu.h"

#define MAX_FILENAME_LENGTH 511

static int RGBA_to_PPM(char *filename, int width, int height,
											 GLubyte * buffer, int binary);
#ifdef JPEG
static int RGB_to_JPG(char *filename, int width, int height,
											GLubyte * buffer);
#endif

SaveFrameSPU saveframe_spu;

static void SAVEFRAMESPU_APIENTRY
swapBuffers(GLint window, GLint flags)
{
	if (saveframe_spu.enabled)
	{
		int saveThisFrame = 0;

		if ((saveframe_spu.single != -1)
				&& (saveframe_spu.single == saveframe_spu.framenum))
			saveThisFrame = 1;

		if ((saveframe_spu.single == -1)
				&& (saveframe_spu.framenum % saveframe_spu.stride == 0))
			saveThisFrame = 1;

		if (saveframe_spu.width == -1)
		{
			/* We've never been given a size.  Try to get one from OpenGL. */
			GLint geom[4];
			saveframe_spu.child.GetIntegerv(GL_VIEWPORT, geom);
			saveframe_spu.x = geom[0];
			saveframe_spu.y = geom[1];
			saveframe_spu.width = geom[2];
			saveframe_spu.height = geom[3];

			ResizeBuffer();
		}

		if (saveThisFrame && !(flags & CR_SUPPRESS_SWAP_BIT))
		{
			char filename[MAX_FILENAME_LENGTH + 1];
			int numchars;
#ifdef WINDOWS									/* ? */
			numchars =
				_snprintf(filename, MAX_FILENAME_LENGTH, saveframe_spu.spec,
									saveframe_spu.framenum);
#else
			numchars =
				snprintf(filename, MAX_FILENAME_LENGTH, saveframe_spu.spec,
								 saveframe_spu.framenum);
#endif

			if (numchars < MAX_FILENAME_LENGTH)
			{
				/* Only save the frame if the filename wasn't truncated by snprintf */
				if (!crStrcmp(saveframe_spu.format, "ppm"))
				{
					saveframe_spu.child.ReadBuffer(GL_BACK);
					saveframe_spu.child.ReadPixels(0, 0, saveframe_spu.width,
								       saveframe_spu.height, GL_RGBA,
								       GL_UNSIGNED_BYTE,
								       saveframe_spu.buffer);
					
					RGBA_to_PPM(filename, saveframe_spu.width, saveframe_spu.height,
						    saveframe_spu.buffer, saveframe_spu.binary);
				}
#ifdef JPEG
				else if (!crStrcmp(saveframe_spu.format, "jpeg"))
				  {
				    GLubyte* in;
				    GLubyte* out;
				    saveframe_spu.child.ReadBuffer(GL_BACK);
				    saveframe_spu.child.ReadPixels(0, 0, saveframe_spu.width,
								   saveframe_spu.height, GL_RGBA,
								   GL_UNSIGNED_BYTE,
								   saveframe_spu.buffer);
				    /* Work around an apparent but in the NVIDIA OpenGL driver */
				    in= out= saveframe_spu.buffer;
				    while (in<saveframe_spu.buffer+4*saveframe_spu.height*saveframe_spu.width) {
				      *out++= *in++; /* R */
				      *out++= *in++; /* G */
				      *out++= *in++; /* B */
				      in++; /* skip A */
				    }
				    
				    RGB_to_JPG(filename, saveframe_spu.width, saveframe_spu.height,
					       saveframe_spu.buffer);
				  }
#endif
				else {
				  crWarning("Invalid value for saveframe_spu.format: %s",
					    saveframe_spu.format);
				}
			}
			else
			  {
			    crWarning
			      ("saveframespu: Filename longer than %d characters isn't allowed. Skipping frame %d.",
			       MAX_FILENAME_LENGTH, saveframe_spu.framenum);
			  }
		}
	}

	if (!(flags & CR_SUPPRESS_SWAP_BIT)) saveframe_spu.framenum++;

	saveframe_spu.child.SwapBuffers(window, flags);
}

static void SAVEFRAMESPU_APIENTRY
viewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	saveframe_spu.height = height;
	saveframe_spu.width = width;
	saveframe_spu.x = x;
	saveframe_spu.y = y;

	ResizeBuffer();
	saveframe_spu.child.Viewport(x, y, width, height);
}

static void SAVEFRAMESPU_APIENTRY
saveframeChromiumParameteri(GLenum param, GLint i)
{
	switch (param)
	{
	case GL_SAVEFRAME_ENABLED_CR:
		saveframe_spu.enabled = i == GL_TRUE ? GL_TRUE : GL_FALSE;
		break;
	case GL_SAVEFRAME_FRAMENUM_CR:
		if (i >= 0)
			saveframe_spu.framenum = i;
		break;
	case GL_SAVEFRAME_STRIDE_CR:
		if (i > 0)
			saveframe_spu.stride = i;
		break;
	case GL_SAVEFRAME_SINGLE_CR:
		i = i > -2 ? i : -1;
		saveframe_spu.single = i;
		break;
	default:
		break;
	}
}

static void SAVEFRAMESPU_APIENTRY
saveframeChromiumParameterv(GLenum param, GLenum type, GLsizei count,
														const GLvoid * p)
{
	switch (param)
	{
	case GL_SAVEFRAME_FILESPEC_CR:
		if (!p)
			break;
		if (saveframe_spu.spec)
			crFree(saveframe_spu.spec);
		saveframe_spu.spec = crStrdup((const char *) p);
		break;
	default:
		break;
	}
}

static void SAVEFRAMESPU_APIENTRY
saveframeGetChromiumParameterv(GLenum target, GLuint index, GLenum type,
															 GLsizei count, GLvoid * values)
{
	switch (target)
	{
	case GL_SAVEFRAME_FRAMENUM_CR:
		if (type == GL_INT && count == 1)
			*((GLint *) values) = saveframe_spu.framenum;
		return;
	case GL_SAVEFRAME_STRIDE_CR:
		if (type == GL_INT && count == 1)
			*((GLint *) values) = saveframe_spu.stride;
		return;
	case GL_SAVEFRAME_SINGLE_CR:
		if (type == GL_INT && count == 1)
			*((GLint *) values) = saveframe_spu.single;
		return;
	case GL_SAVEFRAME_FILESPEC_CR:
		if (type == GL_BYTE && count > 0)
			crStrncpy((char *) values, saveframe_spu.spec, count);
		return;
	default:
		saveframe_spu.super.GetChromiumParametervCR(target, index, type, count,
																								values);
		return;
	}
}

SPUNamedFunctionTable _cr_saveframe_table[] = {
	{"SwapBuffers", (SPUGenericFunction) swapBuffers}
	,
	{"Viewport", (SPUGenericFunction) viewport}
	,
	{"ChromiumParameteriCR", (SPUGenericFunction) saveframeChromiumParameteri}
	,
	{"ChromiumParametervCR", (SPUGenericFunction) saveframeChromiumParameterv}
	,
	{"GetChromiumParametervCR",
	 (SPUGenericFunction) saveframeGetChromiumParameterv}
	,
	{NULL, NULL}
};

void
ResizeBuffer(void)
{
	if (saveframe_spu.buffer != NULL)
		crFree(saveframe_spu.buffer);

	saveframe_spu.buffer =
		(GLubyte *) crAlloc(sizeof(GLubyte) * saveframe_spu.height *
											 saveframe_spu.width * 4);
}

static int
RGBA_to_PPM(char *filename, int width, int height, GLubyte * buffer,
						int binary)
{
	FILE *file;
	int i, j;

	file = fopen(filename, "wb");

	if (file == NULL)
	{
		crError("Unable to create file %s.\n", filename);
		return 1;
	}

	if (binary)
	{
		GLubyte *row;

		fprintf(file, "P6\n%d %d\n255\n", width, height);

		for (i = height - 1; i >= 0; i--)
		{
			row = &buffer[i * width * 4];
			for (j = 0; j < width; j++)
			{
				fwrite(row, 3, 1, file);
				row += 4;
			}
		}
	}
	else
	{
		fprintf(file, "P3\n%d %d\n255\n", width, height);
		for (i = height - 1; i >= 0; i--)
		{
			for (j = 0; j < width; j++)
			{
				fprintf(file, "%d %d %d \n",
								buffer[i * width * 4 + j * 4 + 0],
								buffer[i * width * 4 + j * 4 + 1],
								buffer[i * width * 4 + j * 4 + 2]);
			}
			fprintf(file, "\n");
		}
	}

	fclose(file);

	return 0;
}

#ifdef JPEG
static int
RGB_to_JPG(char *filename, int width, int height, GLubyte * buffer)
{
	FILE *file;
	int row_stride;
	JSAMPROW row_pointer[1];

	file = fopen(filename, "wb");

	if (file == NULL)
	{
		crError("Unable to create file %s.\n", filename);
		return 1;
	}

	/* Write image to file */
	jpeg_stdio_dest(&saveframe_spu.cinfo, file);
	saveframe_spu.cinfo.image_width = width;
	saveframe_spu.cinfo.image_height = height;
	saveframe_spu.cinfo.input_components = 3;
	saveframe_spu.cinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults(&saveframe_spu.cinfo);
	saveframe_spu.cinfo.dct_method = JDCT_FLOAT;
	jpeg_set_quality(&saveframe_spu.cinfo, 100, TRUE);

	jpeg_start_compress(&saveframe_spu.cinfo, TRUE);
	row_stride = width * 3;
	while (saveframe_spu.cinfo.next_scanline < saveframe_spu.cinfo.image_height)
	{
		row_pointer[0] = (JSAMPROW) ((buffer + (height - 1) * row_stride)
					     - saveframe_spu.cinfo.next_scanline * row_stride);
		jpeg_write_scanlines(&saveframe_spu.cinfo, row_pointer, 1);
	}
	jpeg_finish_compress(&saveframe_spu.cinfo);

	fclose(file);

	return 0;
}
#endif /* JPEG */
