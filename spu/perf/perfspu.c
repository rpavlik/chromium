/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include <string.h>
#include "cr_mothership.h"
#include "cr_spu.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_pixeldata.h"
#include "perfspu.h"


#define DUMP_DATA(s,o,n) \
	if (o < n) { \
		sprintf(str, "%s%s%s%d%s%d", output, s, perf_spu.separator, n, perf_spu.separator, n - o); \
		perfspuDump( str ); \
	}

#define DUMP_DATA2(s,o,n) \
	if (o.count < n.count) { \
		sprintf(str, "%s%s%s%d%s%d", output, s, perf_spu.separator, n.count, perf_spu.separator, n.count - o.count); \
		perfspuDump( str ); \
		perfspuDumpVertices(pstring, &o, &n); \
	}

void perfspuDump( char *str )
{
	if ( perf_spu.mothership_log )
		crMothershipSendString ( perf_spu.conn, NULL, "logperf %s", str );
	else {
		fprintf( perf_spu.log_file, str );
		fprintf( perf_spu.log_file, "\n" );
	}
}

void perfspuDumpVertices(char *pstring, PerfVertex *old, PerfVertex *new)
{
	char output[200];
	char str[200];

	sprintf(output, "%s%s%s%s%s%s", perf_spu.token, perf_spu.separator, perf_spu.hostname, perf_spu.separator, pstring, perf_spu.separator);

	DUMP_DATA("glVertex2d", old->v2d, new->v2d);
	DUMP_DATA("glVertex2f", old->v2f, new->v2f);
	DUMP_DATA("glVertex2i", old->v2i, new->v2i);
	DUMP_DATA("glVertex2s", old->v2s, new->v2s);
	DUMP_DATA("glVertex2dv", old->v2dv, new->v2dv);
	DUMP_DATA("glVertex2fv", old->v2fv, new->v2fv);
	DUMP_DATA("glVertex2iv", old->v2iv, new->v2iv);
	DUMP_DATA("glVertex2sv", old->v2sv, new->v2sv);
	DUMP_DATA("glVertex3d", old->v3d, new->v3d);
	DUMP_DATA("glVertex3f", old->v3f, new->v3f);
	DUMP_DATA("glVertex3i", old->v3i, new->v3i);
	DUMP_DATA("glVertex3s", old->v3s, new->v3s);
	DUMP_DATA("glVertex3dv", old->v3dv, new->v3dv);
	DUMP_DATA("glVertex3fv", old->v3fv, new->v3fv);
	DUMP_DATA("glVertex3iv", old->v3iv, new->v3iv);
	DUMP_DATA("glVertex3sv", old->v3sv, new->v3sv);
	DUMP_DATA("glVertex4d", old->v4d, new->v4d);
	DUMP_DATA("glVertex4f", old->v4f, new->v4f);
	DUMP_DATA("glVertex4i", old->v4i, new->v4i);
	DUMP_DATA("glVertex4s", old->v4s, new->v4s);
	DUMP_DATA("glVertex4dv", old->v4dv, new->v4dv);
	DUMP_DATA("glVertex4fv", old->v4fv, new->v4fv);
	DUMP_DATA("glVertex4iv", old->v4iv, new->v4iv);
	DUMP_DATA("glVertex4sv", old->v4sv, new->v4sv);

	DUMP_DATA("INTERP_POINTS", old->ipoints, new->ipoints);
	DUMP_DATA("INTERP_LINES", old->ilines, new->ilines);
	DUMP_DATA("INTERP_TRIS", old->itris, new->itris);
	DUMP_DATA("INTERP_QUADS", old->iquads, new->iquads);
	DUMP_DATA("INTERP_POLYGONS", old->ipolygons, new->ipolygons);

	/* Break up for the next dump */
	perfspuDump( " " );
}

void perfspuDumpCounters(char *pstring, PerfData *old, PerfData *new)
{
	PerfPrim *oldprim = &old->vertex_data;
	PerfPrim *newprim = &new->vertex_data;
	char output[200];
	char str[200];

	sprintf(output, "%s%s%s%s%s%s", perf_spu.token, perf_spu.separator, perf_spu.hostname, perf_spu.separator, pstring, perf_spu.separator);
	
	DUMP_DATA2("POINTS", oldprim->points, newprim->points);
	DUMP_DATA2("LINES", oldprim->lines, newprim->lines);
	DUMP_DATA2("LINELOOPS", oldprim->lineloop, newprim->lineloop);
	DUMP_DATA2("LINESTRIPS", oldprim->linestrip, newprim->linestrip);
	DUMP_DATA2("TRANGLES", oldprim->triangles, newprim->triangles);
	DUMP_DATA2("TRISTRIPS", oldprim->tristrip, newprim->tristrip);
	DUMP_DATA2("TRIFANS", oldprim->trifan, newprim->trifan);
	DUMP_DATA2("QUADS", oldprim->quads, newprim->quads);
	DUMP_DATA2("QUADSTRIPS", oldprim->quadstrip, newprim->quadstrip);
	DUMP_DATA2("POLYGONS", oldprim->polygon, newprim->polygon);

	DUMP_DATA("DRAWPIXELS", old->draw_pixels, new->draw_pixels);
	DUMP_DATA("READPIXELS", old->read_pixels, new->read_pixels);
		
	/* Copy to the old structures for variance output */
	crMemcpy(old, new, sizeof(PerfData));
}

void perfspuInterpretVertexData( PerfVertex *stats, int diff )
{
	if (diff) {

		if (diff == 1) 
			stats->ipoints++;

		if (diff == 2)
			stats->ilines++;

		if (diff == 3)
			stats->itris++;

		if (diff == 4)
			stats->iquads++;

		if (diff > 4)
			stats->ipolygons++;

	}
}

void perfspuCheckVertexData( PerfVertex *stats, PerfVertex *snapshot )
{
	if (stats->v2d - snapshot->v2d)
		perfspuInterpretVertexData( stats, stats->v2d - snapshot->v2d );
	if (stats->v2dv - snapshot->v2dv)
		perfspuInterpretVertexData( stats, stats->v2dv - snapshot->v2dv );
	if (stats->v2f - snapshot->v2f)
		perfspuInterpretVertexData( stats, stats->v2f - snapshot->v2f );
	if (stats->v2fv - snapshot->v2fv)
		perfspuInterpretVertexData( stats, stats->v2fv - snapshot->v2fv );
	if (stats->v2i - snapshot->v2i)
		perfspuInterpretVertexData( stats, stats->v2i - snapshot->v2i );
	if (stats->v2iv - snapshot->v2iv)
		perfspuInterpretVertexData( stats, stats->v2iv - snapshot->v2iv );
	if (stats->v2s - snapshot->v2s)
		perfspuInterpretVertexData( stats, stats->v2s - snapshot->v2s );
	if (stats->v2sv - snapshot->v2sv)
		perfspuInterpretVertexData( stats, stats->v2sv - snapshot->v2sv );
	if (stats->v3d - snapshot->v3d)
		perfspuInterpretVertexData( stats, stats->v3d - snapshot->v3d );
	if (stats->v3dv - snapshot->v3dv)
		perfspuInterpretVertexData( stats, stats->v3dv - snapshot->v3dv );
	if (stats->v3f - snapshot->v3f)
		perfspuInterpretVertexData( stats, stats->v3f - snapshot->v3f );
	if (stats->v3fv - snapshot->v3fv)
		perfspuInterpretVertexData( stats, stats->v3fv - snapshot->v3fv );
	if (stats->v3i - snapshot->v3i)
		perfspuInterpretVertexData( stats, stats->v3i - snapshot->v3i );
	if (stats->v3iv - snapshot->v3iv)
		perfspuInterpretVertexData( stats, stats->v3iv - snapshot->v3iv );
	if (stats->v3s - snapshot->v3s)
		perfspuInterpretVertexData( stats, stats->v3s - snapshot->v3s );
	if (stats->v3sv - snapshot->v3sv)
		perfspuInterpretVertexData( stats, stats->v3sv - snapshot->v3sv );
	if (stats->v4d - snapshot->v4d)
		perfspuInterpretVertexData( stats, stats->v4d - snapshot->v4d );
	if (stats->v4dv - snapshot->v4dv)
		perfspuInterpretVertexData( stats, stats->v4dv - snapshot->v4dv );
	if (stats->v4f - snapshot->v4f)
		perfspuInterpretVertexData( stats, stats->v4f - snapshot->v4f );
	if (stats->v4fv - snapshot->v4fv)
		perfspuInterpretVertexData( stats, stats->v4fv - snapshot->v4fv );
	if (stats->v4i - snapshot->v4i)
		perfspuInterpretVertexData( stats, stats->v4i - snapshot->v4i );
	if (stats->v4iv - snapshot->v4iv)
		perfspuInterpretVertexData( stats, stats->v4iv - snapshot->v4iv );
	if (stats->v4s - snapshot->v4s)
		perfspuInterpretVertexData( stats, stats->v4s - snapshot->v4s );
	if (stats->v4sv - snapshot->v4sv)
		perfspuInterpretVertexData( stats, stats->v4sv - snapshot->v4sv );
}






void PERFSPU_APIENTRY perfspuChromiumParameteriCR(GLenum target, GLint value)
{
	char rstr[100];

	/* We need to check the SPU ID, as in GetChromiumParameter(), this
	 * way we only set the token on the required SPU */

	switch (target) {
	case GL_PERF_SET_DUMP_ON_SWAP_CR:
		perf_spu.dump_on_swap_count = value;
		break;
	case GL_PERF_SET_DUMP_ON_FINISH_CR:
		perf_spu.dump_on_finish = value;
		break;
	case GL_PERF_SET_DUMP_ON_FLUSH_CR:
		perf_spu.dump_on_flush = value;
		break;
	case GL_PERF_DUMP_COUNTERS_CR:
		sprintf(rstr, "REQUESTED%s0", perf_spu.separator);
		perfspuDumpCounters(rstr, &perf_spu.old_framestats, &perf_spu.framestats);
		break;
	default:
		break;
	}

 	/* we always pass this down, as there could be other perfSPU's
  	 * attached anywhere in the chain */
	perf_spu.super.ChromiumParameteriCR( target, value );
}

void PERFSPU_APIENTRY perfspuChromiumParameterfCR(GLenum target, GLfloat value)
{
	/* We need to check the SPU ID, as in GetChromiumParameter(), this
	 * way we only set the token on the required SPU */

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
		break;
	}

 	/* we always pass this down, as there could be other perfSPU's
  	 * attached anywhere in the chain */
	perf_spu.super.ChromiumParameterfCR( target, value );
}

void PERFSPU_APIENTRY perfspuChromiumParametervCR(GLenum target, GLenum type, GLsizei count, const GLvoid *values)
{
	/* We need to check the SPU ID, as in GetChromiumParameter(), this
	 * way we only set the token on the required SPU */

	switch (target) {
	case GL_PERF_SET_TOKEN_CR:
		strncpy(perf_spu.token, (char *)values, strlen((char*)values));
		break;
	default:
		break;
	}

 	/* we always pass this down, as there could be other perfSPU's
  	 * attached anywhere in the chain */
	perf_spu.super.ChromiumParametervCR( target, type, count, values );
}

void PERFSPU_APIENTRY perfspuGetChromiumParametervCR(GLenum target, GLuint index, GLenum type, GLsizei count, GLvoid *values)
{
	/* We use the index value to identify the SPU so that we
	 * return a pointer to the correct stats */

	if (perf_spu.id != index) {
		perf_spu.super.GetChromiumParametervCR( target, index, type, count, values);
		return;
	}

	switch (target) {
	case GL_PERF_GET_FRAME_DATA_CR:
		values = (GLvoid *)&perf_spu.framestats;
		break;
	case GL_PERF_GET_TIMER_DATA_CR:
		values = (GLvoid *)&perf_spu.timerstats;
		break;
	default:
		perf_spu.super.GetChromiumParametervCR( target, index, type, count, values);
		break;
	}
}






void PERFSPU_APIENTRY perfspuBegin( GLenum mode )
{
	switch (mode) {
		case GL_POINTS:
			perf_spu.framestats.cur_vertex = &perf_spu.framestats.vertex_data.points;
			perf_spu.timerstats.cur_vertex = &perf_spu.timerstats.vertex_data.points;
			break;
		case GL_LINES:
			perf_spu.framestats.cur_vertex = &perf_spu.framestats.vertex_data.lines;
			perf_spu.timerstats.cur_vertex = &perf_spu.timerstats.vertex_data.lines;
			break;
		case GL_LINE_LOOP:
			perf_spu.framestats.cur_vertex = &perf_spu.framestats.vertex_data.lineloop;
			perf_spu.framestats.cur_vertex = &perf_spu.timerstats.vertex_data.lineloop;
			break;
		case GL_LINE_STRIP:
			perf_spu.framestats.cur_vertex = &perf_spu.framestats.vertex_data.linestrip;
			perf_spu.timerstats.cur_vertex = &perf_spu.timerstats.vertex_data.linestrip;
			break;
		case GL_TRIANGLES:
			perf_spu.framestats.cur_vertex = &perf_spu.framestats.vertex_data.triangles;
			perf_spu.timerstats.cur_vertex = &perf_spu.timerstats.vertex_data.triangles;
			break;
		case GL_TRIANGLE_STRIP:
			perf_spu.framestats.cur_vertex = &perf_spu.framestats.vertex_data.tristrip;
			perf_spu.timerstats.cur_vertex = &perf_spu.timerstats.vertex_data.tristrip;
			break;
		case GL_TRIANGLE_FAN:
			perf_spu.framestats.cur_vertex = &perf_spu.framestats.vertex_data.trifan;
			perf_spu.timerstats.cur_vertex = &perf_spu.timerstats.vertex_data.trifan;
			break;
		case GL_QUADS:
			perf_spu.framestats.cur_vertex = &perf_spu.framestats.vertex_data.quads;
			perf_spu.timerstats.cur_vertex = &perf_spu.timerstats.vertex_data.quads;
			break;
		case GL_QUAD_STRIP:
			perf_spu.framestats.cur_vertex = &perf_spu.framestats.vertex_data.quadstrip;
			perf_spu.timerstats.cur_vertex = &perf_spu.timerstats.vertex_data.quadstrip;
			break;
		case GL_POLYGON:
			perf_spu.framestats.cur_vertex = &perf_spu.framestats.vertex_data.polygon;
			perf_spu.timerstats.cur_vertex = &perf_spu.timerstats.vertex_data.polygon;
			break;
		default:
			printf("Ooops, bad glBegin mode to Performance SPU\n");
			break;
	}

	/* Make a snapshot of all vertex data, so we can interpret the
	 * geometric shapes in End() */
	crMemcpy(&perf_spu.framestats.vertex_snapshot, perf_spu.framestats.cur_vertex, sizeof(PerfVertex));

	if (perf_spu.timer_event)
		crMemcpy(&perf_spu.timerstats.vertex_snapshot, perf_spu.timerstats.cur_vertex, sizeof(PerfVertex));

	perf_spu.framestats.cur_vertex->count++;

	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->count++;

	perf_spu.mode = mode;

	perf_spu.super.Begin( mode );
}

void PERFSPU_APIENTRY perfspuEnd( )
{
	perfspuCheckVertexData(perf_spu.framestats.cur_vertex, &perf_spu.framestats.vertex_snapshot);

	if (perf_spu.timer_event) 
		perfspuCheckVertexData(perf_spu.timerstats.cur_vertex, &perf_spu.timerstats.vertex_snapshot);

	perf_spu.super.End( );
}

void PERFSPU_APIENTRY perfspuDrawPixels( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels )
{
	perf_spu.framestats.draw_pixels += crImageSize( format, type, width, height );
	if (perf_spu.timer_event)
		perf_spu.timerstats.draw_pixels += crImageSize( format, type, width, height );

	perf_spu.super.DrawPixels( width, height, format, type, pixels );
}

void PERFSPU_APIENTRY perfspuReadPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels )
{
	perf_spu.framestats.read_pixels += crImageSize( format, type, width, height );
	if (perf_spu.timer_event)
		perf_spu.timerstats.read_pixels += crImageSize( format, type, width, height );

	perf_spu.super.ReadPixels( x, y, width, height, format, type, pixels );
}

void PERFSPU_APIENTRY perfspuVertex2d( GLdouble v0, GLdouble v1 )
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v2d++;

	perf_spu.framestats.cur_vertex->v2d++;
		
	perf_spu.super.Vertex2d( v0, v1 );
}

void PERFSPU_APIENTRY perfspuVertex2dv( GLdouble *v0 )
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v2dv++;

	perf_spu.framestats.cur_vertex->v2dv++;

	perf_spu.super.Vertex2dv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex2f( GLfloat v0, GLfloat v1 )
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v2f++;

	perf_spu.framestats.cur_vertex->v2f++;

	perf_spu.super.Vertex2f( v0, v1 );
}

void PERFSPU_APIENTRY perfspuVertex2fv( GLfloat *v0 )
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v2fv++;

	perf_spu.framestats.cur_vertex->v2fv++;

	perf_spu.super.Vertex2fv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex2i( GLint v0, GLint v1 )
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v2i++;

	perf_spu.framestats.cur_vertex->v2i++;

	perf_spu.super.Vertex2i( v0, v1 );
}

void PERFSPU_APIENTRY perfspuVertex2iv( GLint *v0 )
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v2iv++;

	perf_spu.framestats.cur_vertex->v2iv++;

	perf_spu.super.Vertex2iv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex2s( GLshort v0, GLshort v1 )
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v2s++;

	perf_spu.framestats.cur_vertex->v2s++;

	perf_spu.super.Vertex2s( v0, v1 );
}

void PERFSPU_APIENTRY perfspuVertex2sv( GLshort *v0 )
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v2sv++;

	perf_spu.framestats.cur_vertex->v2sv++;

	perf_spu.super.Vertex2sv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex3d( GLdouble v0, GLdouble v1, GLdouble v2)
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v3d++;

	perf_spu.framestats.cur_vertex->v3d++;

	perf_spu.super.Vertex3d( v0, v1, v2 );
}

void PERFSPU_APIENTRY perfspuVertex3dv( GLdouble *v0 )
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v3dv++;

	perf_spu.framestats.cur_vertex->v3dv++;

	perf_spu.super.Vertex3dv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex3f( GLfloat v0, GLfloat v1, GLfloat v2 )
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v3f++;

	perf_spu.framestats.cur_vertex->v3f++;

	perf_spu.super.Vertex3f( v0, v1, v2 );
}

void PERFSPU_APIENTRY perfspuVertex3fv( GLfloat *v0)
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v3fv++;

	perf_spu.framestats.cur_vertex->v3fv++;

	perf_spu.super.Vertex3fv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex3i( GLint v0, GLint v1, GLint v2 )
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v3i++;

	perf_spu.framestats.cur_vertex->v3i++;

	perf_spu.super.Vertex3i( v0, v1, v2 );
}

void PERFSPU_APIENTRY perfspuVertex3iv( GLint *v0 )
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v3iv++;

	perf_spu.framestats.cur_vertex->v3iv++;

	perf_spu.super.Vertex3iv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex3s( GLshort v0, GLshort v1, GLshort v2 )
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v3s++;

	perf_spu.framestats.cur_vertex->v3s++;

	perf_spu.super.Vertex3s( v0, v1, v2 );
}

void PERFSPU_APIENTRY perfspuVertex3sv( GLshort *v0 )
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v3sv++;

	perf_spu.framestats.cur_vertex->v3sv++;

	perf_spu.super.Vertex3sv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex4d( GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3 )
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v4d++;

	perf_spu.framestats.cur_vertex->v4d++;

	perf_spu.super.Vertex4d( v0, v1, v2, v3 );
}

void PERFSPU_APIENTRY perfspuVertex4dv( GLdouble *v0 )
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v4dv++;

	perf_spu.framestats.cur_vertex->v4dv++;

	perf_spu.super.Vertex4dv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex4f( GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 )
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v4f++;

	perf_spu.framestats.cur_vertex->v4f++;

	perf_spu.super.Vertex4f( v0, v1, v2, v3 );
}

void PERFSPU_APIENTRY perfspuVertex4fv( GLfloat *v0 )
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v4fv++;

	perf_spu.framestats.cur_vertex->v4fv++;

	perf_spu.super.Vertex4fv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex4i( GLint v0, GLint v1, GLint v2, GLint v3 )
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v4i++;

	perf_spu.framestats.cur_vertex->v4i++;

	perf_spu.super.Vertex4i( v0, v1, v2, v3 );
}

void PERFSPU_APIENTRY perfspuVertex4iv( GLint *v0 )
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v4iv++;

	perf_spu.framestats.cur_vertex->v4iv++;

	perf_spu.super.Vertex4iv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex4s( GLshort v0, GLshort v1, GLshort v2, GLshort v3 )
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v4s++;

	perf_spu.framestats.cur_vertex->v4s++;

	perf_spu.super.Vertex4s( v0, v1, v2, v3 );
}

void PERFSPU_APIENTRY perfspuVertex4sv( GLshort *v0 )
{
	if (perf_spu.timer_event)
		perf_spu.timerstats.cur_vertex->v4sv++;

	perf_spu.framestats.cur_vertex->v4sv++;

	perf_spu.super.Vertex4sv( v0 );
}

void PERFSPU_APIENTRY perfspuClear( GLbitfield mask )
{
	char cstr[100];

	perf_spu.clear_counter++;

	if (perf_spu.dump_on_clear_count && 
		!(perf_spu.clear_counter % perf_spu.dump_on_clear_count) ) {
		sprintf(cstr, "CLEARCOUNT%s%d", perf_spu.separator, perf_spu.clear_counter);
		perfspuDumpCounters(cstr, &perf_spu.old_framestats, &perf_spu.framestats);
	}

	perf_spu.super.Clear( mask );
}

void PERFSPU_APIENTRY perfspuFinish( )
{
	char fstr[100];

	if (perf_spu.dump_on_finish) {
		sprintf(fstr, "FINISH%s0", perf_spu.separator);
		perfspuDumpCounters(fstr, &perf_spu.old_framestats, &perf_spu.framestats);
	}
	perf_spu.super.Finish( );
}

void PERFSPU_APIENTRY perfspuFlush( )
{
	char fstr[100];

	if (perf_spu.dump_on_flush) {
		sprintf(fstr, "FLUSH%s0", perf_spu.separator);
		perfspuDumpCounters(fstr, &perf_spu.old_framestats, &perf_spu.framestats);
	}
	perf_spu.super.Flush( );
}

void PERFSPU_APIENTRY perfspuSwapBuffers( GLint window, GLint flags )
{
	static float elapsed_base = 0;
	char sstr[100];
	float total_elapsed = (float) crTimerTime( perf_spu.timer );
	float elapsed = total_elapsed - elapsed_base;

	perf_spu.total_frames++;

	perf_spu.frame_counter++;

	if ((int)(elapsed / perf_spu.timer_event) && perf_spu.timer_event)
	{
		float fps = perf_spu.frame_counter / elapsed;
		elapsed_base = total_elapsed;
		perf_spu.frame_counter = 0;

		/* Put the FPS to the screen */
		printf( "PERFSPU FPS: %f\n", fps );

		/* And the timerstats to the log file */
		sprintf(sstr, "TIMERSTATS%s%2.2f", perf_spu.separator, perf_spu.timer_event);

		perfspuDumpCounters(sstr, &perf_spu.old_timerstats, &perf_spu.timerstats);
	}

	if (perf_spu.dump_on_swap_count && 
		!(perf_spu.total_frames % perf_spu.dump_on_swap_count) ) {
		sprintf(sstr, "FRAMESTATS%s%d", perf_spu.separator, perf_spu.total_frames);
		perfspuDumpCounters(sstr, &perf_spu.old_framestats, &perf_spu.framestats);
	}
	
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
