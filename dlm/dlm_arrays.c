/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include <stdarg.h>
#include "chromium.h"
#include "cr_dlm.h"
#include "dlm.h"
#include "dlm_dispatch.h"

/*
 * XXX this code is awefully similar to the code in arrayspu.c
 * We should try to write something reusable.
 */

void DLM_APIENTRY crdlm_compile_ArrayElement (GLint index)
{
  unsigned char *p;
  int unit;
  CRDLMContextState *state = CURRENT_STATE();
  CRClientState *c = state->clientState;

  if (c->array.e.enabled)
  {
    crdlm_compile_EdgeFlagv(c->array.e.p + index*c->array.e.stride);
  }
  for (unit = 0; unit < CR_MAX_TEXTURE_UNITS; unit++)
  {
    if (c->array.t[unit].enabled)
    {
      p = c->array.t[unit].p + index*c->array.t[unit].stride;
      switch (c->array.t[unit].type)
      {
        case GL_SHORT:
          switch (c->array.t[c->curClientTextureUnit].size)
          {
            case 1: crdlm_compile_MultiTexCoord1svARB(GL_TEXTURE0_ARB + unit, (GLshort *)p); break;
            case 2: crdlm_compile_MultiTexCoord2svARB(GL_TEXTURE0_ARB + unit, (GLshort *)p); break;
            case 3: crdlm_compile_MultiTexCoord3svARB(GL_TEXTURE0_ARB + unit, (GLshort *)p); break;
            case 4: crdlm_compile_MultiTexCoord4svARB(GL_TEXTURE0_ARB + unit, (GLshort *)p); break;
          }
          break;
        case GL_INT:
          switch (c->array.t[c->curClientTextureUnit].size)
          {
            case 1: crdlm_compile_MultiTexCoord1ivARB(GL_TEXTURE0_ARB + unit, (GLint *)p); break;
            case 2: crdlm_compile_MultiTexCoord2ivARB(GL_TEXTURE0_ARB + unit, (GLint *)p); break;
            case 3: crdlm_compile_MultiTexCoord3ivARB(GL_TEXTURE0_ARB + unit, (GLint *)p); break;
            case 4: crdlm_compile_MultiTexCoord4ivARB(GL_TEXTURE0_ARB + unit, (GLint *)p); break;
          }
          break;
        case GL_FLOAT:
          switch (c->array.t[c->curClientTextureUnit].size)
          {
            case 1: crdlm_compile_MultiTexCoord1fvARB(GL_TEXTURE0_ARB + unit, (GLfloat *)p); break;
            case 2: crdlm_compile_MultiTexCoord2fvARB(GL_TEXTURE0_ARB + unit, (GLfloat *)p); break;
            case 3: crdlm_compile_MultiTexCoord3fvARB(GL_TEXTURE0_ARB + unit, (GLfloat *)p); break;
            case 4: crdlm_compile_MultiTexCoord4fvARB(GL_TEXTURE0_ARB + unit, (GLfloat *)p); break;
          }
          break;
        case GL_DOUBLE:
          switch (c->array.t[c->curClientTextureUnit].size)
          {
            case 1: crdlm_compile_MultiTexCoord1dvARB(GL_TEXTURE0_ARB + unit, (GLdouble *)p); break;
            case 2: crdlm_compile_MultiTexCoord2dvARB(GL_TEXTURE0_ARB + unit, (GLdouble *)p); break;
            case 3: crdlm_compile_MultiTexCoord3dvARB(GL_TEXTURE0_ARB + unit, (GLdouble *)p); break;
            case 4: crdlm_compile_MultiTexCoord4dvARB(GL_TEXTURE0_ARB + unit, (GLdouble *)p); break;
          }
          break;
      }
    }
  } /* loop over texture units */

  if (c->array.i.enabled)
  {
    p = c->array.i.p + index*c->array.i.stride;
    switch (c->array.i.type)
    {
      case GL_SHORT: crdlm_compile_Indexsv((GLshort *)p); break;
      case GL_INT: crdlm_compile_Indexiv((GLint *)p); break;
      case GL_FLOAT: crdlm_compile_Indexfv((GLfloat *)p); break;
      case GL_DOUBLE: crdlm_compile_Indexdv((GLdouble *)p); break;
    }
  }
  if (c->array.c.enabled)
  {
    p = c->array.c.p + index*c->array.c.stride;
    switch (c->array.c.type)
    {
      case GL_BYTE:
        switch (c->array.c.size)
        {
          case 3: crdlm_compile_Color3bv((GLbyte *)p); break;
          case 4: crdlm_compile_Color4bv((GLbyte *)p); break;
        }
        break;
      case GL_UNSIGNED_BYTE:
        switch (c->array.c.size)
        {
          case 3: crdlm_compile_Color3ubv((GLubyte *)p); break;
          case 4: crdlm_compile_Color4ubv((GLubyte *)p); break;
        }
        break;
      case GL_SHORT:
        switch (c->array.c.size)
        {
          case 3: crdlm_compile_Color3sv((GLshort *)p); break;
          case 4: crdlm_compile_Color4sv((GLshort *)p); break;
        }
        break;
      case GL_UNSIGNED_SHORT:
        switch (c->array.c.size)
        {
          case 3: crdlm_compile_Color3usv((GLushort *)p); break;
          case 4: crdlm_compile_Color4usv((GLushort *)p); break;
        }
        break;
      case GL_INT:
        switch (c->array.c.size)
        {
          case 3: crdlm_compile_Color3iv((GLint *)p); break;
          case 4: crdlm_compile_Color4iv((GLint *)p); break;
        }
        break;
      case GL_UNSIGNED_INT:
        switch (c->array.c.size)
        {
          case 3: crdlm_compile_Color3uiv((GLuint *)p); break;
          case 4: crdlm_compile_Color4uiv((GLuint *)p); break;
        }
        break;
      case GL_FLOAT:
        switch (c->array.c.size)
        {
          case 3: crdlm_compile_Color3fv((GLfloat *)p); break;
          case 4: crdlm_compile_Color4fv((GLfloat *)p); break;
        }
        break;
      case GL_DOUBLE:
        switch (c->array.c.size)
        {
          case 3: crdlm_compile_Color3dv((GLdouble *)p); break;
          case 4: crdlm_compile_Color4dv((GLdouble *)p); break;
        }
        break;
    }
  }
  if (c->array.n.enabled)
  {
    p = c->array.n.p + index*c->array.n.stride;
    switch (c->array.n.type)
    {
      case GL_BYTE: crdlm_compile_Normal3bv((GLbyte *)p); break;
      case GL_SHORT: crdlm_compile_Normal3sv((GLshort *)p); break;
      case GL_INT: crdlm_compile_Normal3iv((GLint *)p); break;
      case GL_FLOAT: crdlm_compile_Normal3fv((GLfloat *)p); break;
      case GL_DOUBLE: crdlm_compile_Normal3dv((GLdouble *)p); break;
    }
  }
#ifdef CR_EXT_secondary_color
  if (c->array.s.enabled)
  {
    p = c->array.s.p + index*c->array.s.stride;
    switch (c->array.s.type)
    {
      case GL_BYTE:
        crdlm_compile_SecondaryColor3bvEXT((GLbyte *)p); break;
      case GL_UNSIGNED_BYTE:
        crdlm_compile_SecondaryColor3ubvEXT((GLubyte *)p); break;
      case GL_SHORT:
        crdlm_compile_SecondaryColor3svEXT((GLshort *)p); break;
      case GL_UNSIGNED_SHORT:
        crdlm_compile_SecondaryColor3usvEXT((GLushort *)p); break;
      case GL_INT:
        crdlm_compile_SecondaryColor3ivEXT((GLint *)p); break;
      case GL_UNSIGNED_INT:
        crdlm_compile_SecondaryColor3uivEXT((GLuint *)p); break;
      case GL_FLOAT:
        crdlm_compile_SecondaryColor3fvEXT((GLfloat *)p); break;
      case GL_DOUBLE:
        crdlm_compile_SecondaryColor3dvEXT((GLdouble *)p); break;
    }
  }
#endif
  if (c->array.v.enabled)
  {
    p = c->array.v.p + (index*c->array.v.stride);

    switch (c->array.v.type)
    {
      case GL_SHORT:
        switch (c->array.v.size)
        {
          case 2: crdlm_compile_Vertex2sv((GLshort *)p); break;
          case 3: crdlm_compile_Vertex3sv((GLshort *)p); break;
          case 4: crdlm_compile_Vertex4sv((GLshort *)p); break;
        }
        break;
      case GL_INT:
        switch (c->array.v.size)
        {
          case 2: crdlm_compile_Vertex2iv((GLint *)p); break;
          case 3: crdlm_compile_Vertex3iv((GLint *)p); break;
          case 4: crdlm_compile_Vertex4iv((GLint *)p); break;
        }
        break;
      case GL_FLOAT:
        switch (c->array.v.size)
        {
          case 2: crdlm_compile_Vertex2fv((GLfloat *)p); break;
          case 3: crdlm_compile_Vertex3fv((GLfloat *)p); break;
          case 4: crdlm_compile_Vertex4fv((GLfloat *)p); break;
        }
        break;
      case GL_DOUBLE:
        switch (c->array.v.size)
        {
          case 2: crdlm_compile_Vertex2dv((GLdouble *)p); break;
          case 3: crdlm_compile_Vertex3dv((GLdouble *)p); break;
          case 4: crdlm_compile_Vertex4dv((GLdouble *)p); break;
        }
        break;
    }
  }
}

void APIENTRY crdlm_compile_DrawArrays(GLenum mode, GLint first, GLsizei count)
{
  int i;

  if (count < 0)
  {
    crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "DLM DrawArrays(negative count)");
    return;
  }

  if (mode > GL_POLYGON)
  {
    crdlmWarning(__LINE__, __FILE__, GL_INVALID_ENUM, "DLM DrawArrays(bad mode)");
    return;
  }

  crdlm_compile_Begin(mode);
  for (i=0; i<count; i++)
  {
    crdlm_compile_ArrayElement(first + i);
  }
  crdlm_compile_End();
}

void APIENTRY crdlm_compile_DrawElements(GLenum mode, GLsizei count,
                                      GLenum type, const GLvoid *indices)
{
  int i;
  GLubyte *p = (GLubyte *)indices;

  if (count < 0)
  {
    crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "DLM DrawElements(negative count)");
    return;
  }

  if (mode > GL_POLYGON)
  {
    crdlmWarning(__LINE__, __FILE__, GL_INVALID_ENUM, "DLM DrawElements(bad mode)");
    return;
  }

  if (type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_SHORT && type != GL_UNSIGNED_INT)
  {
    crdlmWarning(__LINE__, __FILE__, GL_INVALID_ENUM, "DLM DrawElements(bad type)");
    return;
  }

  crdlm_compile_Begin(mode);
  switch (type)
  {
  case GL_UNSIGNED_BYTE:
    for (i=0; i<count; i++)
    {
      crdlm_compile_ArrayElement((GLint) *p++);
    }
    break;
  case GL_UNSIGNED_SHORT:
    for (i=0; i<count; i++)
    {
      crdlm_compile_ArrayElement((GLint) * (GLushort *) p);
      p+=sizeof (GLushort);
    }
    break;
  case GL_UNSIGNED_INT:
    for (i=0; i<count; i++)
    {
      crdlm_compile_ArrayElement((GLint) * (GLuint *) p);
      p+=sizeof (GLuint);
    }
    break;
  default:
    crError( "this can't happen: DLM DrawElements" );
    break;
  }
  crdlm_compile_End();
}

void APIENTRY crdlm_compile_DrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, 
                                      GLenum type, const GLvoid *indices)
{
  int i;
  GLubyte *p = (GLubyte *)indices;

  (void) end;

  if (count < 0)
  {
    crdlmWarning(__LINE__, __FILE__, GL_INVALID_VALUE, "DLM DrawRangeElements(negative count)");
    return;
  }

  if (mode > GL_POLYGON)
  {
    crdlmWarning(__LINE__, __FILE__, GL_INVALID_ENUM, "DLM DrawRangeElements(bad mode)");
    return;
  }

  if (type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_SHORT && type != GL_UNSIGNED_INT)
  {
    crdlmWarning(__LINE__, __FILE__, GL_INVALID_ENUM, "DLM DrawRangeElements(bad type)");
    return;
  }

  crdlm_compile_Begin(mode);
  switch (type)
  {
  case GL_UNSIGNED_BYTE:
    for (i=start; i<count; i++)
    {
      crdlm_compile_ArrayElement((GLint) *p++);
    }
    break;
  case GL_UNSIGNED_SHORT:
    for (i=start; i<count; i++)
    {
      crdlm_compile_ArrayElement((GLint) * (GLushort *) p);
      p+=sizeof (GLushort);
    }
    break;
  case GL_UNSIGNED_INT:
    for (i=start; i<count; i++)
    {
      crdlm_compile_ArrayElement((GLint) * (GLuint *) p);
      p+=sizeof (GLuint);
    }
    break;
  default:
    crError( "this can't happen: DLM DrawRangeElements" );
    break;
  }
  crdlm_compile_End();
}

#ifdef CR_EXT_multi_draw_arrays
void APIENTRY crdlm_compile_MultiDrawArraysEXT( GLenum mode, GLint *first,
                          GLsizei *count, GLsizei primcount)
{
   GLint i;

   for (i = 0; i < primcount; i++) {
      if (count[i] > 0) {
         crdlm_compile_DrawArrays(mode, first[i], count[i]);
      }
   }
}


void APIENTRY crdlm_compile_MultiDrawElementsEXT( GLenum mode, const GLsizei *count, GLenum type,
                            const GLvoid **indices, GLsizei primcount)
{
   GLint i;

   for (i = 0; i < primcount; i++) {
      if (count[i] > 0) {
         crdlm_compile_DrawElements(mode, count[i], type, indices[i]);
      }
   }
}
#endif /* CR_EXT_multi_draw_arrays */
