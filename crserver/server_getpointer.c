#include "cr_glwrapper.h"
#include "cr_error.h" 
#include "server_dispatch.h"
#include "server.h"

void SERVER_DISPATCH_APIENTRY crServerDispatchGetPointerv( GLenum pname, GLvoid **pointer )
{
	crError( "glGetPointerv isn't *ever* allowed to be on the wire!" );
	(void) pname;
	(void) pointer;
}
