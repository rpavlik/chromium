#ifndef STATE_INTERNALS_H
#define STATE_INTERNALS_H

#include "cr_spu.h"
#include "state/cr_statetypes.h"

#define FLUSH() if (g->flush_func != NULL) { g->flush_func( g->flush_arg ); g->flush_func = NULL; }

typedef void (SPU_APIENTRY *glAble)(GLenum);

#endif
