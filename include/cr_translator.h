/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_TRANSLATOR_H
#define CR_TRANSLATOR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CRTranslator CRTranslator;

CRTranslator *crTranslatorShare(CRTranslator *translator);
CRTranslator *crTranslatorCreate(void);
void crTranslatorDestroy(void *translator);
GLuint crTranslateListId(const CRTranslator *translator, GLuint list);
void crTranslateAddListId(CRTranslator *translator, GLuint highLevelList, GLuint lowLevelList);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CR_TRANSLATOR_H */
