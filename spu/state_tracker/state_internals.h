#ifndef STATE_INTERNALS_H
#define STATE_INTERNALS_H

#define FLUSH() if (g->flush_func != NULL) { g->flush_func( g->flush_arg ); g->flush_func = NULL; }

#endif
