#ifndef CR_PACKER_H
#define CR_PACKER_H

#ifdef WINDOWS
#ifdef DLLDATA
#undef DLLDATA
#endif
#define DLLDATA __declspec(dllexport)
#endif

#include "cr_pack.h"
#include "cr_packfunctions.h"
#include "packer_extensions.h"

#endif /* CR_PACKER_H */
