/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include <string.h>
#include "cr_spu.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "perfspu.h"

typedef struct __perf_list {
	int number;
	PerfCounters *pc;
	struct __perf_list *next;
	struct __perf_list *prev;
} PerfList;

PerfList *perf_list = NULL;

#if CR_PERF_PER_INSTANCE_COUNTERS
static char *primitives[] = {
	"POINTS",
	"LINES",
	"LINELOOP",
	"LINESTRIP",
	"TRIANGLES",
	"TRISTRIP",
	"TRIFAN",
	"QUADS",
	"QUADSTRIP",
	"POLYGON"
};
#endif

void perfspuDumpVertices(char *pstring, PerfVertex *old, PerfVertex *new)
{
	char *output;

	output = (char *) crAlloc ( strlen(perf_spu.token) + strlen(perf_spu.hostname) + strlen(pstring) + (strlen(perf_spu.separator) * 4) );

	sprintf(output, "%s%s%s%s%s%s", perf_spu.token, perf_spu.separator, perf_spu.hostname, perf_spu.separator, pstring, perf_spu.separator);

	if (old->v2d < new->v2d)
		fprintf( perf_spu.log_file, "%sglVertex2d(v)%s%d%s%d\n", output, perf_spu.separator, new->v2d, perf_spu.separator, new->v2d - old->v2d);

	if (old->v2f < new->v2f)
		fprintf( perf_spu.log_file, "%sglVertex2f(v)%s%d%s%d\n", output, perf_spu.separator, new->v2f, perf_spu.separator, new->v2f - old->v2f);

	if (old->v2i < new->v2i)
		fprintf( perf_spu.log_file, "%sglVertex2i(v)%s%d%s%d\n", output, perf_spu.separator, new->v2i, perf_spu.separator, new->v2i - old->v2i);

	if (old->v2s < new->v2s)
		fprintf( perf_spu.log_file, "%sglVertex2s(v)%s%d%s%d\n", output, perf_spu.separator, new->v2s, perf_spu.separator, new->v2s - old->v2s);

	if (old->v3d < new->v3d)
		fprintf( perf_spu.log_file, "%sglVertex3d(v)%s%d%s%d\n", output, perf_spu.separator, new->v3d, perf_spu.separator, new->v3d - old->v3d);

	if (old->v3f < new->v3f)
		fprintf( perf_spu.log_file, "%sglVertex3f(v)%s%d%s%d\n", output, perf_spu.separator, new->v3f, perf_spu.separator, new->v3f - old->v3f);

	if (old->v3i < new->v3i)
		fprintf( perf_spu.log_file, "%sglVertex3i(v)%s%d%s%d\n", output, perf_spu.separator, new->v3i, perf_spu.separator, new->v3i - old->v3i);

	if (old->v3s < new->v3s)
		fprintf( perf_spu.log_file, "%sglVertex3s(v)%s%d%s%d\n", output, perf_spu.separator, new->v3s, perf_spu.separator, new->v3s - old->v3s);

	if (old->v4d < new->v4d)
		fprintf( perf_spu.log_file, "%sglVertex4d(v)%s%d%s%d\n", output, perf_spu.separator, new->v4d, perf_spu.separator, new->v4d - old->v4d);

	if (old->v4f < new->v4f)
		fprintf( perf_spu.log_file, "%sglVertex4f(v)%s%d%s%d\n", output, perf_spu.separator, new->v4f, perf_spu.separator, new->v4f - old->v4f);

	if (old->v4i < new->v4i)
		fprintf( perf_spu.log_file, "%sglVertex4i(v)%s%d%s%d\n", output, perf_spu.separator, new->v4i, perf_spu.separator, new->v4i - old->v4i);

	if (old->v4s < new->v4s)
		fprintf( perf_spu.log_file, "%sglVertex4s(v)%s%d%s%d\n", output, perf_spu.separator, new->v4s, perf_spu.separator, new->v4s - old->v4s);

	crFree(output);
}

void perfspuDumpCounters(char *pstring, PerfPrim *old, PerfPrim *new)
{
#if CR_PERF_PER_INSTANCE_COUNTERS
	PerfList *l = perf_list;
	PerfList *lStart = perf_list;
	int count = 0;

	do {
		count++;
		l = l->next;
	
		fprintf( perf_spu.log_file, "%s %s %s %s\n", perf_spu.token, perf_spu.hostname, pstring, primitives[l->pc->mode]);

		if (l->pc->v2d)
			fprintf( perf_spu.log_file, "%s %s %s glVertex2f(d) %d\n", perf_spu.token, perf_spu.hostname, pstring, l->pc->v2d);

		if (l->pc->v3d)
			fprintf( perf_spu.log_file, "%s %s %s glVertex3d(d) %d\n", perf_spu.token, perf_spu.hostname, pstring, l->pc->v3d);

		if (l->pc->v4d)
			fprintf( perf_spu.log_file, "%s %s %s glVertex4d(d) %d\n", perf_spu.token, perf_spu.hostname, pstring, l->pc->v4d);

		if (l->pc->v2f)
			fprintf( perf_spu.log_file, "%s %s %s glVertex2f(v) %d\n", perf_spu.token, perf_spu.hostname, pstring, l->pc->v2f);

		if (l->pc->v3f)
			fprintf( perf_spu.log_file, "%s %s %s glVertex3f(v) %d\n", perf_spu.token, perf_spu.hostname, pstring, l->pc->v3f);

		if (l->pc->v4f)
			fprintf( perf_spu.log_file, "%s %s %s glVertex4f(v) %d\n", perf_spu.token, perf_spu.hostname, pstring, l->pc->v4f);

		if (l->pc->v2i)
			fprintf( perf_spu.log_file, "%s %s %s glVertex2i(v) %d\n", perf_spu.token, perf_spu.hostname, pstring, l->pc->v2i);

		if (l->pc->v3i)
			fprintf( perf_spu.log_file, "%s %s %s glVertex3i(v) %d\n", perf_spu.token, perf_spu.hostname, pstring, l->pc->v3i);

		if (l->pc->v4i)
			fprintf( perf_spu.log_file, "%s %s %s glVertex4i(v) %d\n", perf_spu.token, perf_spu.hostname, pstring, l->pc->v4i);

		if (l->pc->v2s)
			fprintf( perf_spu.log_file, "%s %s %s glVertex2s(v) %d\n", perf_spu.token, perf_spu.hostname, pstring, l->pc->v2s);

		if (l->pc->v3s)
			fprintf( perf_spu.log_file, "%s %s %s glVertex3s(v) %d\n", perf_spu.token, perf_spu.hostname, pstring, l->pc->v3s);

		if (l->pc->v4s)
			fprintf( perf_spu.log_file, "%s %s %s glVertex4s(v) %d\n", perf_spu.token, perf_spu.hostname, pstring, l->pc->v4s);

	} while (l != lStart);
#else
	char *output;

	output = (char *) crAlloc ( strlen(perf_spu.token) + strlen(perf_spu.hostname) + strlen(pstring) + (strlen(perf_spu.separator) * 4) );

	sprintf(output, "%s%s%s%s%s%s", perf_spu.token, perf_spu.separator, perf_spu.hostname, perf_spu.separator, pstring, perf_spu.separator);
	
	if (old->points.count < new->points.count) {
		fprintf( perf_spu.log_file, "%sPOINTS%s%d%s%d\n", output, perf_spu.separator, new->points.count, perf_spu.separator, new->points.count - old->points.count);

		perfspuDumpVertices(pstring, &old->points, &new->points);
	}

	if (old->lines.count < new->lines.count) {
		fprintf( perf_spu.log_file, "%sLINES%s%d%s%d\n", output, perf_spu.separator, new->lines.count, perf_spu.separator, new->lines.count - old->lines.count);

		perfspuDumpVertices(pstring, &old->lines, &new->lines);
	}

	if (old->lineloop.count < new->lineloop.count) {
		fprintf( perf_spu.log_file, "%sLINELOOPS%s%d%s%d\n", output, perf_spu.separator, new->lineloop.count, perf_spu.separator, new->lineloop.count - old->lineloop.count);
	
		perfspuDumpVertices(pstring, &old->lineloop, &new->lineloop);
	}

	if (old->linestrip.count < new->linestrip.count) {
		fprintf( perf_spu.log_file, "%sLINESTRIPS%s%d%s%d\n", output, perf_spu.separator, new->linestrip.count, perf_spu.separator, new->linestrip.count - old->linestrip.count);
	
		perfspuDumpVertices(pstring, &old->lineloop, &new->lineloop);
	}

	if (old->triangles.count < new->triangles.count) {
		fprintf( perf_spu.log_file, "%sTRIANGLES%s%d%s%d\n", output, perf_spu.separator, new->triangles.count, perf_spu.separator, new->triangles.count - old->triangles.count);
	
		perfspuDumpVertices(pstring, &old->triangles, &new->triangles);
	}

	if (old->tristrip.count < new->tristrip.count) {
		fprintf( perf_spu.log_file, "%sTRISTRIPS%s%d%s%d\n", output, perf_spu.separator, new->tristrip.count, perf_spu.separator, new->tristrip.count - old->tristrip.count);
	
		perfspuDumpVertices(pstring, &old->tristrip, &new->tristrip);
	}

	if (old->trifan.count < new->trifan.count) {
		fprintf( perf_spu.log_file, "%sTRIFANS%s%d%s%d\n", output, perf_spu.separator, new->trifan.count, perf_spu.separator, new->trifan.count - old->trifan.count);
	
		perfspuDumpVertices(pstring, &old->tristrip, &new->tristrip);
	}

	if (old->quads.count < new->quads.count) {
		fprintf( perf_spu.log_file, "%sQUADS%s%d%s%d\n", output, perf_spu.separator, new->quads.count, perf_spu.separator, new->quads.count - old->quads.count);

		perfspuDumpVertices(pstring, &old->quads, &new->quads);
	}

	if (old->quadstrip.count < new->quadstrip.count) {
		fprintf( perf_spu.log_file, "%sQUADSTRIPS%s%d%s%d\n", output,  perf_spu.separator, new->quadstrip.count, perf_spu.separator, new->quadstrip.count - old->quadstrip.count);

		perfspuDumpVertices(pstring, &old->quadstrip, &new->quadstrip);
	}
	
	if (old->polygon.count < new->polygon.count) {
		fprintf( perf_spu.log_file, "%sPOLYGONS%s%d%s%d\n", output, perf_spu.separator, new->polygon.count, perf_spu.separator, new->polygon.count - old->polygon.count);

		perfspuDumpVertices(pstring, &old->polygon, &new->polygon);
	}

	crMemcpy(old, new, sizeof(PerfPrim));

	crFree(output);
	
#endif

	/* Break up for the next dump */
	fprintf( perf_spu.log_file, "\n");
	fflush( perf_spu.log_file );
}

void PERFSPU_APIENTRY perfspuChromiumParameteriCR(GLenum target, GLint value)
{
	switch (target) {
	case GL_PERF_RESET_VERTEX_COUNTER_CR:
		break;
	case GL_PERF_RESET_BEGINEND_COUNTER_CR:
		break;
	case GL_PERF_RESET_DRAWPIXELS_COUNTER_CR:
		break;
	case GL_PERF_RESET_READPIXELS_COUNTER_CR:
		break;
	case GL_PERF_RESET_SWAPBUFFERS_COUNTER_CR:
		break;
	case GL_PERF_SET_SWAPBUFFERS_DUMP_COUNT_CR:
		perf_spu.swapdumpcount = value;
		break;
	case GL_PERF_DUMP_COUNTERS_CR:
		perfspuDumpCounters("REQUESTED 0", &perf_spu.old_framestats, &perf_spu.framestats);
		break;
	default:
		perf_spu.super.ChromiumParameteriCR( target, value );
		break;
	}

}

void PERFSPU_APIENTRY perfspuChromiumParameterfCR(GLenum target, GLfloat value)
{
	switch (target) {
 	case GL_PERF_START_TIMER_CR:
		perf_spu.timer_event = value;
		if (!perf_spu.timer->running)
			crStartTimer( perf_spu.timer );
		break;
	case GL_PERF_STOP_TIMER_CR:
		perf_spu.timer_event = 0.0f;
		if (perf_spu.timer->running)
			crStopTimer( perf_spu.timer );
		break;
	default:
		perf_spu.super.ChromiumParameterfCR( target, value );
		break;
	}
}

void PERFSPU_APIENTRY perfspuChromiumParametervCR(GLenum target, GLenum type, GLsizei count, const GLvoid *values)
{
	switch (target) {
	case GL_PERF_SET_TOKEN_CR:
		strncpy(perf_spu.token, (char *)values, strlen((char*)values));
		break;
	default:
		perf_spu.super.ChromiumParametervCR( target, type, count, values );
	}
}

void PERFSPU_APIENTRY perfspuGetChromiumParametervCR(GLenum target, GLuint index, GLenum type, GLsizei count, GLvoid *values)
{
	switch (target) {
	default:
		perf_spu.super.GetChromiumParametervCR( target, index, type, count, values);
	}
}

void perfspuAddToPerfList( PerfCounters *pc )
{
	PerfList *l = (PerfList *) crAlloc( sizeof( *l ) );

	l->pc = pc;

	if (!perf_list)
	{
		perf_list = l;
		l->next = l;
		l->prev = l;
	}
	else
	{
		l->next = perf_list->next;
		perf_list->next->prev = l;

		l->prev = perf_list;
		perf_list->next = l;
	}
}

#if CR_PERF_PER_INSTANCE_COUNTERS
void perfspuFreePerfList( )
{
	PerfList *l = perf_list;
	PerfList *lStart = perf_list;

	do {
		/* We don't want to free the current operating counters */
		if (l->pc != perf_spu.current_perf)
			crFree(l->pc);	/* Free perf counters */
		l = l->next;
		if (!l) break;
		l->prev->next = NULL;
		crFree(l->prev);
	} while (l != lStart);

	perf_list = NULL;

	/* Ensure currently used one is at the head */
	perfspuAddToPerfList( perf_spu.current_perf );
}
#endif

void PERFSPU_APIENTRY perfspuBegin( GLenum mode )
{
#if CR_PERF_PER_INSTANCE_COUNTERS
	PerfCounters *pc = (PerfCounters *) crAlloc ( sizeof (*pc) );

	crMemset(pc, 0, sizeof(*pc) );

	pc->mode = mode;

	perfspuAddToPerfList( pc );

	perf_spu.current_perf = pc;
#endif

	switch (mode) {
		case GL_POINTS:
			perf_spu.cur_framestats = &perf_spu.framestats.points;
			perf_spu.cur_timerstats = &perf_spu.timerstats.points;
			break;
		case GL_LINES:
			perf_spu.cur_framestats = &perf_spu.framestats.lines;
			perf_spu.cur_timerstats = &perf_spu.timerstats.lines;
			break;
		case GL_LINE_LOOP:
			perf_spu.cur_framestats = &perf_spu.framestats.lineloop;
			perf_spu.cur_timerstats = &perf_spu.timerstats.lineloop;
			break;
		case GL_LINE_STRIP:
			perf_spu.cur_framestats = &perf_spu.framestats.linestrip;
			perf_spu.cur_timerstats = &perf_spu.timerstats.linestrip;
			break;
		case GL_TRIANGLES:
			perf_spu.cur_framestats = &perf_spu.framestats.triangles;
			perf_spu.cur_timerstats = &perf_spu.timerstats.triangles;
			break;
		case GL_TRIANGLE_STRIP:
			perf_spu.cur_framestats = &perf_spu.framestats.tristrip;
			perf_spu.cur_timerstats = &perf_spu.timerstats.tristrip;
			break;
		case GL_TRIANGLE_FAN:
			perf_spu.cur_framestats = &perf_spu.framestats.trifan;
			perf_spu.cur_timerstats = &perf_spu.timerstats.trifan;
			break;
		case GL_QUADS:
			perf_spu.cur_framestats = &perf_spu.framestats.quads;
			perf_spu.cur_timerstats = &perf_spu.timerstats.quads;
			break;
		case GL_QUAD_STRIP:
			perf_spu.cur_framestats = &perf_spu.framestats.quadstrip;
			perf_spu.cur_timerstats = &perf_spu.timerstats.quadstrip;
			break;
		case GL_POLYGON:
			perf_spu.cur_framestats = &perf_spu.framestats.polygon;
			perf_spu.cur_timerstats = &perf_spu.timerstats.polygon;
			break;
		default:
			printf("Ooops, bad glBegin mode to Performance SPU\n");
			break;
	}

	perf_spu.cur_framestats->count++;

	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->count++;

	perf_spu.super.Begin( mode );
}

void PERFSPU_APIENTRY perfspuEnd( )
{
	perf_spu.super.End( );
}

void PERFSPU_APIENTRY perfspuDrawPixels( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels )
{
	perf_spu.super.DrawPixels( width, height, format, type, pixels );
}

void PERFSPU_APIENTRY perfspuReadPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels )
{
	perf_spu.super.ReadPixels( x, y, width, height, format, type, pixels );
}

void PERFSPU_APIENTRY perfspuVertex2d( GLdouble v0, GLdouble v1 )
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v2d++;

	perf_spu.cur_framestats->v2d++;
		
	perf_spu.super.Vertex2d( v0, v1 );
}

void PERFSPU_APIENTRY perfspuVertex2dv( GLdouble *v0 )
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v2d++;

	perf_spu.cur_framestats->v2d++;

	perf_spu.super.Vertex2dv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex2f( GLfloat v0, GLfloat v1 )
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v2f++;

	perf_spu.cur_framestats->v2f++;

	perf_spu.super.Vertex2f( v0, v1 );
}

void PERFSPU_APIENTRY perfspuVertex2fv( GLfloat *v0 )
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v2f++;

	perf_spu.cur_framestats->v2f++;

	perf_spu.super.Vertex2fv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex2i( GLint v0, GLint v1 )
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v2i++;

	perf_spu.cur_framestats->v2i++;

	perf_spu.super.Vertex2i( v0, v1 );
}

void PERFSPU_APIENTRY perfspuVertex2iv( GLint *v0 )
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v2i++;

	perf_spu.cur_framestats->v2i++;

	perf_spu.super.Vertex2iv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex2s( GLshort v0, GLshort v1 )
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v2s++;

	perf_spu.cur_framestats->v2s++;

	perf_spu.super.Vertex2s( v0, v1 );
}

void PERFSPU_APIENTRY perfspuVertex2sv( GLshort *v0 )
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v2s++;

	perf_spu.cur_framestats->v2s++;

	perf_spu.super.Vertex2sv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex3d( GLdouble v0, GLdouble v1, GLdouble v2)
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v3d++;

	perf_spu.cur_framestats->v3d++;

	perf_spu.super.Vertex3d( v0, v1, v2 );
}

void PERFSPU_APIENTRY perfspuVertex3dv( GLdouble *v0 )
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v3d++;

	perf_spu.cur_framestats->v3d++;

	perf_spu.super.Vertex3dv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex3f( GLfloat v0, GLfloat v1, GLfloat v2 )
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v3f++;

	perf_spu.cur_framestats->v3f++;

	perf_spu.super.Vertex3f( v0, v1, v2 );
}

void PERFSPU_APIENTRY perfspuVertex3fv( GLfloat *v0)
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v3f++;

	perf_spu.cur_framestats->v3f++;

	perf_spu.super.Vertex3fv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex3i( GLint v0, GLint v1, GLint v2 )
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v3i++;

	perf_spu.cur_framestats->v3i++;

	perf_spu.super.Vertex3i( v0, v1, v2 );
}

void PERFSPU_APIENTRY perfspuVertex3iv( GLint *v0 )
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v3i++;

	perf_spu.cur_framestats->v3i++;

	perf_spu.super.Vertex3iv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex3s( GLshort v0, GLshort v1, GLshort v2 )
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v3s++;

	perf_spu.cur_framestats->v3s++;

	perf_spu.super.Vertex3s( v0, v1, v2 );
}

void PERFSPU_APIENTRY perfspuVertex3sv( GLshort *v0 )
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v3s++;

	perf_spu.cur_framestats->v3s++;

	perf_spu.super.Vertex3sv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex4d( GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3 )
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v4d++;

	perf_spu.cur_framestats->v4d++;

	perf_spu.super.Vertex4d( v0, v1, v2, v3 );
}

void PERFSPU_APIENTRY perfspuVertex4dv( GLdouble *v0 )
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v4d++;

	perf_spu.cur_framestats->v4d++;

	perf_spu.super.Vertex4dv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex4f( GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 )
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v4f++;

	perf_spu.cur_framestats->v4f++;

	perf_spu.super.Vertex4f( v0, v1, v2, v3 );
}

void PERFSPU_APIENTRY perfspuVertex4fv( GLfloat *v0 )
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v4f++;

	perf_spu.cur_framestats->v4f++;

	perf_spu.super.Vertex4fv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex4i( GLint v0, GLint v1, GLint v2, GLint v3 )
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v4i++;

	perf_spu.cur_framestats->v4i++;

	perf_spu.super.Vertex4i( v0, v1, v2, v3 );
}

void PERFSPU_APIENTRY perfspuVertex4iv( GLint *v0 )
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v4i++;

	perf_spu.cur_framestats->v4i++;

	perf_spu.super.Vertex4iv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex4s( GLshort v0, GLshort v1, GLshort v2, GLshort v3 )
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v4s++;

	perf_spu.cur_framestats->v4s++;

	perf_spu.super.Vertex4s( v0, v1, v2, v3 );
}

void PERFSPU_APIENTRY perfspuVertex4sv( GLshort *v0 )
{
	if (perf_spu.timer_event)
		perf_spu.cur_timerstats->v4s++;

	perf_spu.cur_framestats->v4s++;

	perf_spu.super.Vertex4sv( v0 );
}

void PERFSPU_APIENTRY perfspuClear( GLbitfield mask )
{
	char cstr[100];
#if CR_PERF_PER_INSTANCE_COUNTERS
	PerfCounters *pc;

	if ( !perf_spu.current_perf ) {
		pc = (PerfCounters *) crAlloc ( sizeof (*pc) );

		crMemset(pc, 0, sizeof(*pc) );

		perfspuAddToPerfList( pc );

		perf_spu.current_perf = pc;
	}
#endif

	perf_spu.clear_counter++;

	if (perf_spu.clear_counter & perf_spu.cleardumpcount) {
		sprintf(cstr, "CLEARCOUNT %d", perf_spu.cleardumpcount);
		perfspuDumpCounters(cstr, &perf_spu.old_framestats, &perf_spu.framestats);
#if CR_PERF_PER_INSTANCE_COUNTERS
		perfspuFreePerfList();
#endif
	}

	perf_spu.super.Clear( mask );
}

void PERFSPU_APIENTRY perfspuFinish( )
{
	perfspuDumpCounters("FINISH\t0", &perf_spu.old_framestats, &perf_spu.framestats);
	perf_spu.super.Finish( );
}

void PERFSPU_APIENTRY perfspuFlush( )
{
	perfspuDumpCounters("FLUSH\t0", &perf_spu.old_framestats, &perf_spu.framestats);
	perf_spu.super.Flush( );
}

void PERFSPU_APIENTRY perfspuSwapBuffers( GLint window, GLint flags )
{
	static float elapsed_base = 0;
	int dumped = 0;
	char sstr[100];
	float total_elapsed = (float) crTimerTime( perf_spu.timer );
	float elapsed = total_elapsed - elapsed_base;

	perf_spu.frame_counter++;
	if ((int)(elapsed / perf_spu.timer_event))
	{
		float fps = perf_spu.frame_counter / elapsed;
		elapsed_base = total_elapsed;
		perf_spu.frame_counter = 0;
		printf( "ALAN FPS: %f\n", fps );
		sprintf(sstr, "TIMERSTATS %2.2f", perf_spu.timer_event);
		perfspuDumpCounters(sstr, &perf_spu.old_timerstats, &perf_spu.timerstats);
		dumped = 1;
	}

	if (perf_spu.frame_counter & perf_spu.swapdumpcount) {
		sprintf(sstr, "FRAMESTATS %d", perf_spu.swapdumpcount);
		perfspuDumpCounters(sstr, &perf_spu.old_framestats, &perf_spu.framestats);
		dumped = 1;
	}
	
#if CR_PERF_PER_INSTANCE_COUNTERS
	if (dumped)
		perfspuFreePerfList();
#endif

	perf_spu.swap_counter++;

	perf_spu.super.SwapBuffers( window, flags );
}

SPUNamedFunctionTable perf_table[] = {
	{ "ChromiumParameteriCR", (SPUGenericFunction) perfspuChromiumParameteriCR },
	{ "ChromiumParameterfCR", (SPUGenericFunction) perfspuChromiumParameterfCR },
	{ "ChromiumParametervCR", (SPUGenericFunction) perfspuChromiumParametervCR },
	{ "GetChromiumParametervCR", (SPUGenericFunction) perfspuGetChromiumParametervCR },
	{ "Begin", (SPUGenericFunction) perfspuBegin },
	{ "End", (SPUGenericFunction) perfspuEnd },
	{ "DrawPixels", (SPUGenericFunction) perfspuDrawPixels },
	{ "ReadPixels", (SPUGenericFunction) perfspuReadPixels },
	{ "Vertex2d", (SPUGenericFunction) perfspuVertex2d },
	{ "Vertex2dv", (SPUGenericFunction) perfspuVertex2dv },
	{ "Vertex2f", (SPUGenericFunction) perfspuVertex2f },
	{ "Vertex2fv", (SPUGenericFunction) perfspuVertex2fv },
	{ "Vertex2i", (SPUGenericFunction) perfspuVertex2i },
	{ "Vertex2iv", (SPUGenericFunction) perfspuVertex2iv },
	{ "Vertex2s", (SPUGenericFunction) perfspuVertex2s },
	{ "Vertex2sv", (SPUGenericFunction) perfspuVertex2sv },
	{ "Vertex3d", (SPUGenericFunction) perfspuVertex3d },
	{ "Vertex3dv", (SPUGenericFunction) perfspuVertex3dv },
	{ "Vertex3f", (SPUGenericFunction) perfspuVertex3f },
	{ "Vertex3fv", (SPUGenericFunction) perfspuVertex3fv },
	{ "Vertex3i", (SPUGenericFunction) perfspuVertex3i },
	{ "Vertex3iv", (SPUGenericFunction) perfspuVertex3iv },
	{ "Vertex3s", (SPUGenericFunction) perfspuVertex3s },
	{ "Vertex3sv", (SPUGenericFunction) perfspuVertex3sv },
	{ "Vertex4d", (SPUGenericFunction) perfspuVertex4d },
	{ "Vertex4dv", (SPUGenericFunction) perfspuVertex4dv },
	{ "Vertex4f", (SPUGenericFunction) perfspuVertex4f },
	{ "Vertex4fv", (SPUGenericFunction) perfspuVertex4fv },
	{ "Vertex4i", (SPUGenericFunction) perfspuVertex4i },
	{ "Vertex4iv", (SPUGenericFunction) perfspuVertex4iv },
	{ "Vertex4s", (SPUGenericFunction) perfspuVertex4s },
	{ "Vertex4sv", (SPUGenericFunction) perfspuVertex4sv },
	{ "SwapBuffers", (SPUGenericFunction) perfspuSwapBuffers },
	{ "Clear", (SPUGenericFunction) perfspuClear },
	{ "Finish", (SPUGenericFunction) perfspuFinish },
	{ "Flush", (SPUGenericFunction) perfspuFlush },
	{ NULL, NULL }
};
