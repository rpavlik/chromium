/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */


#include <stdio.h>
#include <stdlib.h>
#include "cr_error.h"
#include "cr_string.h"
#include "cr_spu.h"
#include "saveframespu.h"

static int RGBA_to_PPM(char *basename, int width, int height, GLubyte *buffer);

SaveFrameSPU saveframe_spu;

void SAVEFRAMESPU_APIENTRY
swapBuffers(GLint window, GLint flags)
{
    int saveThisFrame = 0;

    if ((saveframe_spu.single != -1) && (saveframe_spu.single == saveframe_spu.framenum))
        saveThisFrame = 1;

    if ((saveframe_spu.single == -1) && (saveframe_spu.framenum % saveframe_spu.stride == 0))
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

    if (saveThisFrame)
    {
        char filename[500];
        sprintf(filename,"%s%04ld",saveframe_spu.basename,saveframe_spu.framenum);

        saveframe_spu.child.ReadBuffer(GL_BACK);
        saveframe_spu.child.ReadPixels(0, 0, saveframe_spu.width,
                                       saveframe_spu.height, GL_RGBA,
                                       GL_UNSIGNED_BYTE,
                                       saveframe_spu.buffer);

        RGBA_to_PPM(filename, saveframe_spu.width, saveframe_spu.height,
                    saveframe_spu.buffer);
    }

    saveframe_spu.framenum++;

    saveframe_spu.child.SwapBuffers( window, flags );
}

void SAVEFRAMESPU_APIENTRY
viewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    saveframe_spu.height = height;
    saveframe_spu.width = width;
    saveframe_spu.x = x;
    saveframe_spu.y = y;

    ResizeBuffer();
    saveframe_spu.child.Viewport(x, y, width, height);
}

SPUNamedFunctionTable saveframe_table[] = {
	{ "SwapBuffers", (SPUGenericFunction) swapBuffers },
	{ "Viewport", (SPUGenericFunction) viewport },
	{ NULL, NULL }
};

void
ResizeBuffer(void)
{
    if (saveframe_spu.buffer != NULL)
        free(saveframe_spu.buffer);

    saveframe_spu.buffer =
        (GLubyte *) malloc(sizeof(GLubyte) * saveframe_spu.height *
                           saveframe_spu.width * 4);
}

static int
RGBA_to_PPM(char *basename, int width, int height, GLubyte *buffer)
{
    char *filename = (char*)malloc(sizeof(char)*crStrlen(basename)+4);
    FILE   *file;
    int     i, j;

    sprintf(filename,"%s.ppm",basename);

    file = fopen(filename, "w");

    if (file == NULL)
    {
        fprintf(stderr, "Unable to create file %s.\n", filename);
        free(filename);
        return 1;
    }

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

    fclose(file);

    free(filename);
    return 0;
}
