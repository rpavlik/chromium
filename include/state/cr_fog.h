#ifndef CR_STATE_FOG_H 
#define CR_STATE_FOG_H 

#include "cr_glwrapper.h"
#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLbitvalue dirty;
	GLbitvalue color;
	GLbitvalue index;
	GLbitvalue density;
	GLbitvalue start;
	GLbitvalue end;
	GLbitvalue mode;
	GLbitvalue enable;
} CRFogBits;

typedef struct {
	GLcolorf  color;
	GLint     index;
	GLfloat   density;
	GLfloat   start;
	GLfloat   end;
	GLint     mode;
	GLboolean enable;
} CRFogState;

void crStateFogInitBits (CRFogBits *fb);
void crStateFogInit(CRFogState *f);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_FOG_H */
