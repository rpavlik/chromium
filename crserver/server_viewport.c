#include "server_dispatch.h"
#include "server.h"
#include "cr_error.h"

void SERVER_DISPATCH_APIENTRY crServerDispatchViewport( GLint x, GLint y, GLsizei width, GLsizei height )
{
	crWarning( "Ignoring viewport call: %d %d %d %d", x, y, width, height );
}
