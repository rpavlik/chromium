#ifndef CRUT_CLIENTAPI_H
#define CRUT_CLEINTAPI_H

#ifdef WINDOWS
#define CRUT_CLIENT_APIENTRY __stdcall
#else
#define CRUT_CLIENT_APIENTRY
#endif

#include "chromium.h"
#include "crut_api.h"

#define EVENT_BUFFER_SIZE 1024*50 /* 50K, for now */

#ifdef __cplusplus
extern "C" {
#endif

struct client_menu_item
{
    int type;
    char *name;
    int value;
    struct client_menu_item* next;
};

typedef struct client_menu_item clientMenuItem;

struct client_menu_list
{
    int clientMenuID;
    void (*menu)(int value);
    int att_button;
    clientMenuItem* beginItems;
    clientMenuItem* lastItem;

    struct client_menu_list* next;
};

typedef struct client_menu_list clientMenuList;

typedef struct 
{
    void (*mouse)(int button, int state, int x, int y);
    int mouse_param;
    void (*reshape)(int width, int height);
    int reshape_param;
    void (*keyboard)(unsigned char key, int x, int y);
    int keyboard_param;
    void (*motion)(int x, int y);
    int motion_param;
    void (*passivemotion)(int x, int y);
    int passivemotion_param;
    void (*idle)(void);
    void (*display)(void);

} CRUTClientCallbacks;

struct ev_buffer 
{
    char* buffer;
    char* empty;
    char* buffer_end;
    struct ev_buffer* next;
};

typedef struct ev_buffer EventBuffer;

typedef struct 
{
    unsigned short tcpip_port;
    int mtu;
    int buffer_size;
    char protocol[1024];
    int num_bytes;
    CRUTMessage *msg;
    CRConnection *recv_conn;
    CRUTClientCallbacks callbacks;

    int numclients;
    CRUTClientPointer *crutclients;

    EventBuffer *event_buffer;
    EventBuffer *last_buffer;
    char* next_event;
 
    char menuBuffer[MENU_MAX_SIZE];
    clientMenuList* beginMenuList;
    clientMenuList* lastMenu;
 
} CRUTClient;


extern CRUTClient crut_client;

void CRUT_CLIENT_APIENTRY crutInitClient(void);
void CRUT_CLIENT_APIENTRY crutReceiveEventType(int type);
void CRUT_CLIENT_APIENTRY crutMouseFunc( void (*func)(int button, int state, int x, int y) );
void CRUT_CLIENT_APIENTRY crutKeyboardFunc( void (*func) (unsigned char key, int x, int y) );
void CRUT_CLIENT_APIENTRY crutReshapeFunc( void (*func) (int width, int height) );
void CRUT_CLIENT_APIENTRY crutMotionFunc( void (*func) (int x, int y) );
void CRUT_CLIENT_APIENTRY crutPassiveMotionFunc( void (*func) (int x, int y) );
void CRUT_CLIENT_APIENTRY crutIdleFunc( void (*func)(void));
void CRUT_CLIENT_APIENTRY crutDisplayFunc(void (*func)(void));
void CRUT_CLIENT_APIENTRY crutMainLoop(void);
int  CRUT_CLIENT_APIENTRY crutCreateContext ( unsigned int visual );
void CRUT_CLIENT_APIENTRY crutReceiveEvent(CRUTMessage **msg);
int  CRUT_CLIENT_APIENTRY crutCheckEvent( void );
int  CRUT_CLIENT_APIENTRY crutPeekNextEvent( void );
int  CRUT_CLIENT_APIENTRY crutCreateMenu( void (*func) (int val) );
void CRUT_CLIENT_APIENTRY crutAddMenuEntry( char* name, int value );
void CRUT_CLIENT_APIENTRY crutAddSubMenu( char* name, int menuID );
void CRUT_CLIENT_APIENTRY crutAttachMenu( int button );

#ifdef __cplusplus
}
#endif

#endif /* CRUT_CLIENTAPI_H */
