#ifndef CR_STATE_CURRENT_H
#define CR_STATE_CURRENT_H

#include "cr_glwrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLboolean inBeginEnd;
	GLenum mode;
} CRCurrentState;

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_CURRENT_H */
