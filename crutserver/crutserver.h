#ifndef CRUTSERVER_H
#define CRUTSERVER_H

/* Uncomment this line to use menus with the crutserver */
/* #define USE_CRUT_MENUS */

#include "crut_api.h"

#include <GL/gl.h>

#ifdef GLX
#include <GL/glx.h>
#endif

#include <GL/glut.h>

#ifdef USE_CRUT_MENUS
#include <expat.h>
#endif

#include <stdio.h>

struct api_menu_item
{
    char* name;
    int value;
    struct api_menu_item* next;
};

typedef struct api_menu_item apiMenuItem;

struct api_menu
{
    int menuID, subMenus, glutMenuID;
    char* name;
    int att_button;
    
    apiMenuItem* beginItems;
    apiMenuItem* lastItem;

    struct api_menu* sNext; /* for the stack */
    struct api_menu* sLast;

    struct api_menu** tree; /* for the tree */
};

typedef struct api_menu apiMenu;

struct item_value
{
    int index;
    int menuID;
    int value;
    struct item_value* next;
};

typedef struct item_value itemValue;

typedef struct 
{
    int mouse;
    int keyboard;
    int motion;
    int passivemotion;
    int reshape;
    int visibility;
    int menu;
} CRUTServerCallback;

typedef struct 
{      
    CRUTServerCallback callbacks;
    
    apiMenu* stack;
    apiMenu* endStack;

    itemValue* values;
    itemValue* endValue;
    int numValues;

    itemValue* valueTable;

} CRUTServer;

extern CRUTServer crut_server;

#endif
