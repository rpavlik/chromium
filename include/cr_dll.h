#ifndef CR_DLL_H
#define CR_DLL_H

#if defined(WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

typedef struct {
	char *name;
#if defined(WINDOWS)
	HINSTANCE hinstLib;
#elif defined(IRIX) || defined(IRIX64) || defined(Linux)
	void *hinstLib;
#else
#error ARCHITECTURE DLL NOT IMPLEMENTED
#endif
} CRDLL;

typedef void (*CRDLLFunc)(void);
CRDLL *CRDLLOpen( const char *dllname );
CRDLLFunc CRDLLGetNoError( CRDLL *dll, const char *symname );
CRDLLFunc CRDLLGet( CRDLL *dll, const char *symname );
void CRDLLClose( CRDLL *dll );

#endif /* CR_DLL_H */
