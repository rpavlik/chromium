WINGDIAPI void APIENTRY glBarrierCreate (GLuint name, GLuint count);
WINGDIAPI void APIENTRY glBarrierDestroy (GLuint name);
WINGDIAPI void APIENTRY glBarrierExec (GLuint name);
WINGDIAPI void APIENTRY glSemaphoreCreate (GLuint name, GLuint count);
WINGDIAPI void APIENTRY glSemaphoreDestroy (GLuint name);
WINGDIAPI void APIENTRY glSemaphoreP (GLuint name);
WINGDIAPI void APIENTRY glSemaphoreV (GLuint name);
WINGDIAPI void APIENTRY glBoundsInfo (GLrecti *bounds, GLbyte *payload, GLint len, GLint num_opcodes);
