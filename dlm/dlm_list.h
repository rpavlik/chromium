#ifndef DLM_LIST_H
#define DLM_LIST_H

/*** NewList ***/
struct instanceNewList {
	DLMInstanceList *next;
	void (DLM_APIENTRY *execute)(DLMInstanceList *instance, SPUDispatchTable *dispatchTable);
	GLuint list;
	GLuint mode;
};
void DLM_APIENTRY crdlm_compile_NewList(GLenum op, GLuint value);

/*** EndList ***/
struct instanceEndList {
	DLMInstanceList *next;
	void (DLM_APIENTRY *execute)(DLMInstanceList *instance, SPUDispatchTable *dispatchTable);
};
void DLM_APIENTRY crdlm_compile_EndList(void);
#endif
