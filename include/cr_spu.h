#ifndef CR_SPU_H
#define CR_SPU_H

#include "cr_dll.h"
#include "spu_dispatch_table.h"

#define SPU_ENTRY_POINT_NAME "SPULoad"

typedef struct _SPUSTRUCT SPU;

typedef struct {
	char *name;
	int arg_type;
	int arg_length;
	union { 
		char *string;
		int *intarr;
		float fltarr;
	} value;
} SPUArgs; 

typedef void (*SPUGenericFunction)(void);

typedef struct {
	char *name;
	SPUGenericFunction fn;
} SPUNamedFunctionTable;

typedef struct {
	SPUDispatchTable *childCopy;
	void *data;
	SPUNamedFunctionTable *table;
} SPUFunctions;

typedef SPUFunctions *(*SPUInitFuncPtr)(int id, SPU *child,
		SPU *super, unsigned int, unsigned int, unsigned int, 
		unsigned int, SPUArgs *, void *);
typedef void (*SPUSelfDispatchFuncPtr)(SPUDispatchTable *);
typedef int (*SPUCleanupFuncPtr)(void);
typedef int (*SPULoadFunction)(char **, char **, void *, void *, 
		void *, unsigned int *, SPUArgs **);

struct _SPUSTRUCT {
	char *name;
	char *super_name;
	struct _SPUSTRUCT *superSPU;
	CRDLL *dll;
	SPULoadFunction entry_point;
	SPUInitFuncPtr init;
	SPUSelfDispatchFuncPtr self;
	SPUCleanupFuncPtr cleanup;
	unsigned int nargs;
	SPUArgs *args;
	SPUFunctions *function_table;
	SPUDispatchTable dispatch_table;
};

SPU *LoadSPU( SPU *child, int id, char *name );
SPU *LoadSPUChain( int count, int *ids, char **names );

#endif /* CR_SPU_H */
