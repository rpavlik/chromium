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
#include "cr_pixeldata.h"
#include "perfspu.h"

void perfspuDumpVertices(char *pstring, PerfVertex *old, PerfVertex *new)
{
	char *output;

	output = (char *) crAlloc ( strlen(perf_spu.token) + strlen(perf_spu.hostname) + strlen(pstring) + (strlen(perf_spu.separator) * 4) );

	sprintf(output, "%s%s%s%s%s%s", perf_spu.token, perf_spu.separator, perf_spu.hostname, perf_spu.separator, pstring, perf_spu.separator);

	if (old->v2d < new->v2d)
		fprintf( perf_spu.log_file, "%sglVertex2d%s%d%s%d\n", output, perf_spu.separator, new->v2d, perf_spu.separator, new->v2d - old->v2d);

	if (old->v2f < new->v2f)
		fprintf( perf_spu.log_file, "%sglVertex2f%s%d%s%d\n", output, perf_spu.separator, new->v2f, perf_spu.separator, new->v2f - old->v2f);

	if (old->v2i < new->v2i)
		fprintf( perf_spu.log_file, "%sglVertex2i%s%d%s%d\n", output, perf_spu.separator, new->v2i, perf_spu.separator, new->v2i - old->v2i);

	if (old->v2s < new->v2s)
		fprintf( perf_spu.log_file, "%sglVertex2s%s%d%s%d\n", output, perf_spu.separator, new->v2s, perf_spu.separator, new->v2s - old->v2s);

	if (old->v2dv < new->v2dv)
		fprintf( perf_spu.log_file, "%sglVertex2dv%s%d%s%d\n", output, perf_spu.separator, new->v2dv, perf_spu.separator, new->v2dv - old->v2dv);

	if (old->v2fv < new->v2fv)
		fprintf( perf_spu.log_file, "%sglVertex2fv%s%d%s%d\n", output, perf_spu.separator, new->v2fv, perf_spu.separator, new->v2fv - old->v2fv);

	if (old->v2iv < new->v2iv)
		fprintf( perf_spu.log_file, "%sglVertex2iv%s%d%s%d\n", output, perf_spu.separator, new->v2iv, perf_spu.separator, new->v2iv - old->v2iv);

	if (old->v2sv < new->v2sv)
		fprintf( perf_spu.log_file, "%sglVertex2sv%s%d%s%d\n", output, perf_spu.separator, new->v2sv, perf_spu.separator, new->v2sv - old->v2sv);

	if (old->v3d < new->v3d)
		fprintf( perf_spu.log_file, "%sglVertex3d%s%d%s%d\n", output, perf_spu.separator, new->v3d, perf_spu.separator, new->v3d - old->v3d);

	if (old->v3f < new->v3f)
		fprintf( perf_spu.log_file, "%sglVertex3f%s%d%s%d\n", output, perf_spu.separator, new->v3f, perf_spu.separator, new->v3f - old->v3f);

	if (old->v3i < new->v3i)
		fprintf( perf_spu.log_file, "%sglVertex3i%s%d%s%d\n", output, perf_spu.separator, new->v3i, perf_spu.separator, new->v3i - old->v3i);

	if (old->v3s < new->v3s)
		fprintf( perf_spu.log_file, "%sglVertex3s%s%d%s%d\n", output, perf_spu.separator, new->v3s, perf_spu.separator, new->v3s - old->v3s);

	if (old->v3dv < new->v3dv)
		fprintf( perf_spu.log_file, "%sglVertex3dv%s%d%s%d\n", output, perf_spu.separator, new->v3dv, perf_spu.separator, new->v3dv - old->v3dv);

	if (old->v3fv < new->v3fv)
		fprintf( perf_spu.log_file, "%sglVertex3fv%s%d%s%d\n", output, perf_spu.separator, new->v3fv, perf_spu.separator, new->v3fv - old->v3fv);

	if (old->v3iv < new->v3iv)
		fprintf( perf_spu.log_file, "%sglVertex3iv%s%d%s%d\n", output, perf_spu.separator, new->v3iv, perf_spu.separator, new->v3iv - old->v3iv);

	if (old->v3sv < new->v3sv)
		fprintf( perf_spu.log_file, "%sglVertex3sv%s%d%s%d\n", output, perf_spu.separator, new->v3sv, perf_spu.separator, new->v3sv - old->v3sv);

	if (old->v4d < new->v4d)
		fprintf( perf_spu.log_file, "%sglVertex4d%s%d%s%d\n", output, perf_spu.separator, new->v4d, perf_spu.separator, new->v4d - old->v4d);

	if (old->v4f < new->v4f)
		fprintf( perf_spu.log_file, "%sglVertex4f%s%d%s%d\n", output, perf_spu.separator, new->v4f, perf_spu.separator, new->v4f - old->v4f);

	if (old->v4i < new->v4i)
		fprintf( perf_spu.log_file, "%sglVertex4i%s%d%s%d\n", output, perf_spu.separator, new->v4i, perf_spu.separator, new->v4i - old->v4i);

	if (old->v4s < new->v4s)
		fprintf( perf_spu.log_file, "%sglVertex4s%s%d%s%d\n", output, perf_spu.separator, new->v4s, perf_spu.separator, new->v4s - old->v4s);

	if (old->v4dv < new->v4dv)
		fprintf( perf_spu.log_file, "%sglVertex4dv%s%d%s%d\n", output, perf_spu.separator, new->v4dv, perf_spu.separator, new->v4dv - old->v4dv);

	if (old->v4fv < new->v4fv)
		fprintf( perf_spu.log_file, "%sglVertex4fv%s%d%s%d\n", output, perf_spu.separator, new->v4fv, perf_spu.separator, new->v4fv - old->v4fv);

	if (old->v4iv < new->v4iv)
		fprintf( perf_spu.log_file, "%sglVertex4iv%s%d%s%d\n", output, perf_spu.separator, new->v4iv, perf_spu.separator, new->v4iv - old->v4iv);

	if (old->v4sv < new->v4sv)
		fprintf( perf_spu.log_file, "%sglVertex4sv%s%d%s%d\n", output, perf_spu.separator, new->v4sv, perf_spu.separator, new->v4sv - old->v4sv);

	if (old->ipoints < new->ipoints)
		fprintf( perf_spu.log_file, "%sINTERP_POINTS%s%d%s%d\n", output, perf_spu.separator, new->ipoints, perf_spu.separator, new->ipoints - old->ipoints);

	if (old->ilines < new->ilines)
		fprintf( perf_spu.log_file, "%sINTERP_LINES%s%d%s%d\n", output, perf_spu.separator, new->ilines, perf_spu.separator, new->ilines - old->ilines);

	if (old->itris < new->itris)
		fprintf( perf_spu.log_file, "%sINTERP_TRIS%s%d%s%d\n", output, perf_spu.separator, new->itris, perf_spu.separator, new->itris - old->itris);

	if (old->iquads < new->iquads)
		fprintf( perf_spu.log_file, "%sINTERP_QUADS%s%d%s%d\n", output, perf_spu.separator, new->iquads, perf_spu.separator, new->iquads - old->iquads);

	if (old->ipolygons < new->ipolygons)
		fprintf( perf_spu.log_file, "%sINTERP_POLYGONS%s%d%s%d\n", output, perf_spu.separator, new->ipolygons, perf_spu.separator, new->ipolygons - old->ipolygons);

	/* Break up for the next dump */
	fprintf( perf_spu.log_file, "\n");

	crFree(output);
}

void perfspuDumpCounters(char *pstring, PerfData *old, PerfData *new)
{
	PerfPrim *oldprim = &old->vertex_data;
	PerfPrim *newprim = &new->vertex_data;
	char *output;

	output = (char *) crAlloc ( strlen(perf_spu.token) + strlen(perf_spu.hostname) + strlen(pstring) + (strlen(perf_spu.separator) * 4) );

	sprintf(output, "%s%s%s%s%s%s", perf_spu.token, perf_spu.separator, perf_spu.hostname, perf_spu.separator, pstring, perf_spu.separator);
	
	if (oldprim->points.count < newprim->points.count) {
		fprintf( perf_spu.log_file, "%sPOINTS%s%d%s%d\n", output, perf_spu.separator, newprim->points.count, perf_spu.separator, newprim->points.count - oldprim->points.count);

		perfspuDumpVertices(pstring, &oldprim->points, &newprim->points);
	}

	if (oldprim->lines.count < newprim->lines.count) {
		fprintf( perf_spu.log_file, "%sLINES%s%d%s%d\n", output, perf_spu.separator, newprim->lines.count, perf_spu.separator, newprim->lines.count - oldprim->lines.count);

		perfspuDumpVertices(pstring, &oldprim->lines, &newprim->lines);
	}

	if (oldprim->lineloop.count < newprim->lineloop.count) {
		fprintf( perf_spu.log_file, "%sLINELOOPS%s%d%s%d\n", output, perf_spu.separator, newprim->lineloop.count, perf_spu.separator, newprim->lineloop.count - oldprim->lineloop.count);
	
		perfspuDumpVertices(pstring, &oldprim->lineloop, &newprim->lineloop);
	}

	if (oldprim->linestrip.count < newprim->linestrip.count) {
		fprintf( perf_spu.log_file, "%sLINESTRIPS%s%d%s%d\n", output, perf_spu.separator, newprim->linestrip.count, perf_spu.separator, newprim->linestrip.count - oldprim->linestrip.count);
	
		perfspuDumpVertices(pstring, &oldprim->linestrip, &newprim->linestrip);
	}

	if (oldprim->triangles.count < newprim->triangles.count) {
		fprintf( perf_spu.log_file, "%sTRIANGLES%s%d%s%d\n", output, perf_spu.separator, newprim->triangles.count, perf_spu.separator, newprim->triangles.count - oldprim->triangles.count);
	
		perfspuDumpVertices(pstring, &oldprim->triangles, &newprim->triangles);
	}

	if (oldprim->tristrip.count < newprim->tristrip.count) {
		fprintf( perf_spu.log_file, "%sTRISTRIPS%s%d%s%d\n", output, perf_spu.separator, newprim->tristrip.count, perf_spu.separator, newprim->tristrip.count - oldprim->tristrip.count);
	
		perfspuDumpVertices(pstring, &oldprim->tristrip, &newprim->tristrip);
	}

	if (oldprim->trifan.count < newprim->trifan.count) {
		fprintf( perf_spu.log_file, "%sTRIFANS%s%d%s%d\n", output, perf_spu.separator, newprim->trifan.count, perf_spu.separator, newprim->trifan.count - oldprim->trifan.count);
	
		perfspuDumpVertices(pstring, &oldprim->tristrip, &newprim->tristrip);
	}

	if (oldprim->quads.count < newprim->quads.count) {
		fprintf( perf_spu.log_file, "%sQUADS%s%d%s%d\n", output, perf_spu.separator, newprim->quads.count, perf_spu.separator, newprim->quads.count - oldprim->quads.count);

		perfspuDumpVertices(pstring, &oldprim->quads, &newprim->quads);
	}

	if (oldprim->quadstrip.count < newprim->quadstrip.count) {
		fprintf( perf_spu.log_file, "%sQUADSTRIPS%s%d%s%d\n", output,  perf_spu.separator, newprim->quadstrip.count, perf_spu.separator, newprim->quadstrip.count - oldprim->quadstrip.count);

		perfspuDumpVertices(pstring, &oldprim->quadstrip, &newprim->quadstrip);
	}
	
	if (oldprim->polygon.count < newprim->polygon.count) {
		fprintf( perf_spu.log_file, "%sPOLYGONS%s%d%s%d\n", output, perf_spu.separator, newprim->polygon.count, perf_spu.separator, newprim->polygon.count - oldprim->polygon.count);

		perfspuDumpVertices(pstring, &oldprim->polygon, &newprim->polygon);
	}

	if (old->draw_pixels < new->draw_pixels) {
		fprintf( perf_spu.log_file, "%sDRAWPIXELS%s%d%s%d\n", output, perf_spu.separator, new->draw_pixels, perf_spu.separator, new->draw_pixels - old->draw_pixels);
	}
		
	if (old->read_pixels < new->read_pixels) {
		fprintf( perf_spu.log_file, "%sREADPIXELS%s%d%s%d\n", output, perf_spu.separator, new->read_pixels, perf_spu.separator, new->read_pixels - old->read_pixels);
	}
		
	/* Copy to the old structures for variance output */
	crMemcpy(old, new, sizeof(PerfData));

	crFree(output);
	
	fflush( perf_spu.log_file );
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
	switch (target) {
	case GL_PERF_SET_DUMP_ON_SWAP_CR:
		perf_spu.dump_on_swap_count = value;
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
	case GL_PERF_GET_FRAME_DATA_CR:
		values = (GLvoid *)&perf_spu.framestats;
		break;
	case GL_PERF_GET_TIMER_DATA_CR:
		values = (GLvoid *)&perf_spu.timerstats;
		break;
	default:
		perf_spu.super.GetChromiumParametervCR( target, index, type, count, values);
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

	if (perf_spu.clear_counter & perf_spu.dump_on_clear_count) {
		sprintf(cstr, "CLEARCOUNT %d", perf_spu.dump_on_clear_count);
		perfspuDumpCounters(cstr, &perf_spu.old_framestats, &perf_spu.framestats);
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
	char sstr[100];
	float total_elapsed = (float) crTimerTime( perf_spu.timer );
	float elapsed = total_elapsed - elapsed_base;

	perf_spu.frame_counter++;
	if ((int)(elapsed / perf_spu.timer_event))
	{
		float fps = perf_spu.frame_counter / elapsed;
		elapsed_base = total_elapsed;
		perf_spu.frame_counter = 0;

		/* Put the FPS to the screen */
		printf( "FPS: %f\n", fps );

		/* And the timerstats to the log file */
		sprintf(sstr, "TIMERSTATS %2.2f", perf_spu.timer_event);

		perfspuDumpCounters(sstr, &perf_spu.old_timerstats, &perf_spu.timerstats);
	}

	if (perf_spu.frame_counter & perf_spu.dump_on_swap_count) {
		sprintf(sstr, "FRAMESTATS %d", perf_spu.dump_on_swap_count);
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
