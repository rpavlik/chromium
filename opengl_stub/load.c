#include "cr_spu.h"

SPU *stub_spu = NULL;
extern void FakerInit( SPU *fns );

static void __stubInit(void)
{
	// Here is where we contact the mothership to find out what we're supposed to
	// be doing.
	//
	// For now, let's just load up the Error SPU and use that, since we're
	// debugging.
	// 
	// In the future, we need a mechanism for loading and initializing a
	// chain of SPU's, but we're not there yet.
	
	stub_spu = LoadSPU( "renderspu" );
	
	// This is unlikely to change -- We still want to initialize our dispatch
	// table with the functions of the first SPU in the chain.

	FakerInit( stub_spu );
}

#ifdef LINUX
// GCC crap
void (*stub_init_ptr)(void) __attribute__((section(".ctors"))) = __stubInit;
#endif

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Windows crap
BOOL WINAPI DllMain( HINSTANCE instance, DWORD fdwReason, LPVOID lpvReserved )
{
	(void) lpvReserved;
	(void) instance;
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		__stubInit();
	}
	return TRUE;
}
#endif
