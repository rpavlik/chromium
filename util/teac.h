/* 
 * This code is derived from the tcscomm library, John Kochmar, PSC, 2001-2001 
*/

#include <errno.h>
#include <sys/mman.h>
#include <elan3/elan3.h>
#include <elan3/events.h>
#include <elan3/dma.h>

#define E_BUFFER_INITIAL_SIZE 	0x200000
#define NUM_SEND_BUFFERS 4

#define	EQUE_PAD	16 - 6
/* pad data size to 64 boundary */

typedef struct _teac_msg  {
  E3_uint32     new;
  E3_uint32	host;
  E3_uint32	size;
  E3_uint32	msgnum;
  E3_Addr	mptr;
  E3_Addr	clr_event;
  E3_uint32	pad[EQUE_PAD];
} teacMsg;

typedef struct _teac_sbuffer {
  int bufId;
  long totSize;
  long validSize;
  void* buf;
} SBuffer;

typedef struct _teac_rbuffer {
  long totSize;
  long validSize;
  int  from;
  int  senderMsgnum;
  void* buf;
} RBuffer;

typedef struct _teac_comm  {
  ELAN3_CTX	       *ctx;
  ELAN_CAPABILITY	cap;
  ELAN3_DEVINFO	        info;
  E3_DMA_MAIN          *dma;
  sdramaddr_t           e_dma;
  sdramaddr_t           s_event;
  sdramaddr_t         **r_event;
  sdramaddr_t           sbuf_pull_event[NUM_SEND_BUFFERS];
  sdramaddr_t           rbuf_pull_event;
  volatile E3_uint32   *m_snd;
  volatile E3_uint32  **m_rcv;
  volatile E3_uint32   *sbuf_ready[NUM_SEND_BUFFERS];
  volatile E3_uint32   *rbuf_ready;
  teacMsg	      **mbuff;
  SBuffer*              sendWrappers[NUM_SEND_BUFFERS];
  int		        vp;
  int		        hhost, lhost;
  int		        hctx, lctx;
  int		        msgnum;
  int                   poll_shift; 
} Tcomm;

typedef struct _host {
  char	*name;
  int	railMask;
  int	id;
  sdramaddr_t sdramAddrBase;
  E3_Addr elanAddrBase;
} host_t;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

  /*
    - Initialize a elan capability and contexts to be used by
    all processes.
    - lh == the hostname of the "lowest" node in the range
    - hh == the hostname of the "highest" node in the range 
    - lctx == the low context number for each node
    - hctx == the high context number for each node
    - myrank == the rank number of the process on the node (0-N)
    Returns an elan connection pointer on good connection, NULL on failure.
  */
  Tcomm *teac_Init(char *lh, char *hh, int lctx, int hctx, int myrank);
  
  /*
    - teac_Close closes and destroys the given communication context.
  */
  extern void teac_Close(Tcomm *ptr);
  
  /*
    - teac_Select takes a list of connections, and returns the index of the
    connection id that has data available.  Returns -1 if no connection has 
    data waiting.
    
    - ids == an array of connections returned by teac_Open
    - num_ids == number of elements in the array
    - timeout == number of times to poll before returning; values
      less than 0 mean wait forever.
  */ 
  extern int teac_Select(Tcomm* tcomm, int *ids, int num_ids, int timeout);
  
  /*
    - teac_Poll returns the connection id that has data available and
    is at the head of the message queue.  Returns -1 if no connection has 
    data waiting.  Note that the return value is the id, not the index 
    of that id in the input array *ids.
  */ 
  extern int teac_Poll(Tcomm* tcomm, int *ids, int num_ids);
  
  /*
    - teac_sendBufferAvailable returns 0 if teac_getSendBuffer will
    block, non-zero if it will not block.
  */
  extern int teac_sendBufferAvailable(Tcomm* tcomm);

  /*
    - teac_getSendBuffer returns a one-use buffer to be used for an
    outgoing message.  This call will block if no such buffers are
    available, or returns NULL on other errors.
  */
  extern SBuffer* teac_getSendBuffer( Tcomm* tcomm, long size  );
  
  /*
    - teac_Send sends the given buffer to all the listed connections.
    Once the buffer is sent, the calling program is not allowed to
    touch it again!  
    
    teac_Send returns 0 on error, non-zero on success.
  */
  extern int teac_Send( Tcomm* tcomm, int* ids, int num_ids, SBuffer* buf, void *start );
  
  /*
    - teac_Recv receives the next message from the specified connection,
    or NULL in the case of an error.
    
    teac_Recv blocks until the receive completes.
  */
  extern RBuffer* teac_Recv(Tcomm* tcomm, int id);
  
  /*
    - teac_Dispose disposes of the given received message, allowing the buffer
    to be reused.  Once the buffer has been disposed, the calling program
    is not allowed to touch it again!
    
    teac_Dispose returns 0 on error, non-zero on success.
  */
  extern int teac_Dispose( Tcomm* tcomm, RBuffer* buf );
  
  /* 
     - Returns a string with information about a Tcomm data structure.
  */
  extern char* teac_getTcommString(Tcomm *c, char* buf, int buflen);
  
  /* 
     - Returns a string with information about a connection id
  */
  extern char* teac_getConnString(Tcomm *c, int id, char* buf, int buflen);
  
  /*
    - Returns a connection id relative to the given Tcomm for the given rank
    and host.  
  */
  extern int teac_getConnId(Tcomm *c, const char* host, int rank);

  /*
    - Returns information needed for this host's entry in the host table.
  */
  extern int teac_getHostInfo(Tcomm *c, char* host, const int hostLength,
			      int* railMask, int *nodeId, 
			      long* sdramBaseAddr, long* elanBaseAddr);
			      
  
#ifdef __cplusplus
}
#endif // __cplusplus
 
