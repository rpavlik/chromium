#ifndef STATE_INTERNALS_H
#define STATE_INTERNALS_H

#define FLUSH() do { if (g->flush_func != NULL) { g->flush_func( g ); } } while(0)

#endif
