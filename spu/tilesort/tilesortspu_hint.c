#include "tilesortspu.h"
#include "cr_glstate.h"
#include "cr_error.h"
#include "cr_applications.h"

#include <float.h>


static const GLvectorf maxVector = {FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX};
static const GLvectorf minVector = {-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX};

void TILESORTSPU_APIENTRY tilesortspu_Hint( GLenum target, GLenum mode )
{
	switch( target )
	{
		case CR_SCREEN_BBOX_HINT:
		case CR_OBJECT_BBOX_HINT:
			if (mode)
			{
				GLfloat *bbox = (GLfloat *) mode;
				cr_packer_globals.bounds_min.x = bbox[0];
				cr_packer_globals.bounds_min.y = bbox[1];
				cr_packer_globals.bounds_min.z = bbox[2];
				cr_packer_globals.bounds_min.w = bbox[3];
				cr_packer_globals.bounds_max.x = bbox[4];
				cr_packer_globals.bounds_max.y = bbox[5];
				cr_packer_globals.bounds_max.z = bbox[6];
				cr_packer_globals.bounds_max.w = bbox[7];
				//crWarning( "I should really switch to the non-bbox API now, but API switching doesn't work" );
				cr_packer_globals.updateBBOX = 0;
				tilesort_spu.providedBBOX = target;
				return;
			}
			mode = 1;
			// FALLTHROUGH
		case CR_DEFAULT_BBOX_HINT:
			cr_packer_globals.bounds_min = maxVector;
			cr_packer_globals.bounds_max = minVector;
			//crWarning( "I should really switch to the bbox API now, but API switching doesn't work" );
			cr_packer_globals.updateBBOX = 1;
			tilesort_spu.providedBBOX = CR_DEFAULT_BBOX_HINT;
			return;
		default:
			crWarning( "Unknown hint ENUM: %d", target );
			break;
	}
}
