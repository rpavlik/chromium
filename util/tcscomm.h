
#include <errno.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <elan3/elan3.h>
/*
#include <elan3/events.h>
#include <elan3/dma.h>
*/

/* These are the old defintions.... */
/*
#define EADDR_DMA	((E3_Addr) 0x100000)
#define EADDR_SEVENT	((E3_Addr) 0x102000)
#define EADDR_REVENT	((E3_Addr) 0x104000)
#define EADDR_MSND	((E3_Addr) 0x106000)
#define EADDR_MRCV	((E3_Addr) 0x108000)
#define EADDR_ESND	((E3_Addr) 0x10A000)
#define EADDR_ERCV	((E3_Addr) 0x10C000)
#define EADDR_MBUFF	((E3_Addr) 0x200000)
#define EADDR_QUEUE	((E3_Addr) 0x202000)
#define EADDR_Q_DATA	((E3_Addr) 0x204000)
#define EADDR_BUFFER	((E3_Addr) 0x300000)
#define EADDR_ALLOCATOR	((E3_Addr) 0xC00000)

#define E_QDATA_SIZE 	(EADDR_Q_DATA - EADDR_QUEUE)
#define E_BUFFER_SIZE 	(EADDR_ALLOCATOR - EADDR_BUFFER)
*/

/* NOTE - the allocator must be at least 2 pages */
/* #define ALLOCATOR_SIZE	0x4000 */


/* define our elan address (E3_Addr) space 
 * 0x00000000 -> 0x00020000	reserved for required mappings
 * 0x00020000 -> 0x00040000	elan main allocator
 * 0x00040000 -> 0x00060000     elan elan allocator
 * 0x00060000 -> 0x00080000	mysdram allocator
 * 0x00080000 -> 0x000A0000	source buffer
 * 0x000a0000 -> 0x000C0000     dest buffer
 *
 * NOTE - since we may be communicating between architectures
 *        with a different word size, then we cannot assume that
 *        calls to elan3_allocElan/elan3_allocMain will return the
 *        same sdramaddr_t/address.  Hence we explicitly map in
 *        our own communication space.
 */
#define EADDR_ALLOC_MAIN	0x200000
#define ALLOC_MAIN_SIZE		0x200000
#define EADDR_ALLOC_ELAN	0x400000
#define ALLOC_ELAN_SIZE		0x200000

#define EADDR_SDRAM		0x600000
#define SDRAM_SIZE		0x200000

#define EADDR_QUEUE		0x800000
#define QUEUE_SIZE		0x7F0000

#define EADDR_BUFFER		0x1000000
#define BUFFER_SIZE		0x1000000
#define EADDR_MALLOC		0x2000000
#define MALLOC_SIZE		0x2000000
#define MALLOC_PAGE		0x200
#define MALLOC_ELEMS		(MALLOC_SIZE / MALLOC_PAGE)

/* define how I've carve up mysdram */
/*
  #define MYSDRAM_OFF_EVENT	0x0000
  #define MYSDRAM_OFF_OUT_BUF	0x2000
  #define MYSDRAM_OFF_IN_BUF	0x4000
*/

/*
 * stuff that I needed in sdram before, but likely has to be in
 * elan memory now comes first...
 */
/*
#define SDRAM_DMA	0x00000
#define SDRAM_SEVENT	0x02000
#define SDRAM_REVENT	0x04000
#define SDRAM_MSND	0x06000
#define SDRAM_MRCV	0x08000
#define SDRAM_ESND	0x0A000
#define SDRAM_ERCV	0x0C000
#define SDRAM_MBUFF	0x10000
#define SDRAM_QUEUE	0x12000
#define SDRAM_Q_DATA	0x14000
*/
/* defines for the _sdram area for messages */
#define SDRAM_SEVENT	0x00000
#define SDRAM_REVENT	0x02000
#define SDRAM_MSND	0x04000
#define SDRAM_MRCV	0x06000
#define SDRAM_ESND	0x08000
#define SDRAM_ERCV	0x0A000
#define SDRAM_MBUFF	0x0C000

/* defines for the message queue */
#define QUEUE_PTR	0x00000
#define QUEUE_DATA	0x02000
#define QUEUE_ELEMENTS	(QUEUE_SIZE / sizeof(tcomm_msg_t))
#define	EADDR_QBASE	(EADDR_QUEUE + QUEUE_DATA)

/*
#define E_BUFFER_SIZE 	(EADDR_ALLOCATOR - EADDR_BUFFER)
#define E_QDATA_SIZE 	(EADDR_Q_DATA - EADDR_QUEUE)
*/
/*
#define Q_DATA_SIZE	(SDRAM_SIZE - SDRAM_Q_DATA)
*/
#define Q_DATA_SIZE	(QUEUE_SIZE - QUEUE_DATA)
#define MAXIDS		256
#define NUMRAILS	1
#define MAXRAILS	32
#define MAXQENTRIES	256

#define TC_8K 8192
#define TC_64K 65535

/* #define	DATA_SIZE	TC_8K - 96 */
/* #define	EQUE_PAD	16 - 5 */
/* pad data size to 64 boundary */
#define	EQUE_PAD	16 - 7
/* pad data size to 8k boundary */

typedef struct _tcomm_msg  {
  E3_uint32	msgnum;
  E3_uint32	vp;
  E3_uint32	size;
  E3_uint32	t_size;
  E3_Addr		s_mptr;
  E3_Addr		d_mptr;
  E3_Addr		clr_event;
  E3_uint32	pad[EQUE_PAD];
} tcomm_msg_t;
/* tcomm_msg_t is padded by data to be 128 bytes in length */

typedef struct _tcscomm_malloc  {
  caddr_t		ptr;
  int		size;
} tcscomm_malloc_t;

typedef struct _tcscomm_rail  {

  ELAN3_CTX		*ctx;
  ELAN_CAPABILITY		cap;
  ELAN3_DEVINFO		info;
  ELAN3_SDRAM		*sdram;

  E3_DMA		*dma;
  sdramaddr_t	e_dma, s_event, r_event;
  E3_uint32	*m_snd, *m_rcv;

  tcomm_msg_t	*mbuff;

  E3_Queue	*queue;
  tcomm_msg_t	*q_elan;

  tcomm_msg_t	*m_ptr;
  tcomm_msg_t	*m_end;

  int		objid, dev;
} tcomm_rail_t;


typedef struct _tcscomm  {
  int			vp;
  int			hhost, lhost;
  int			hctx, lctx;
  int			myrank;
  int			msgnum;
  int			rails;

  char			*buffer;
  char			*malloc;

  tcscomm_malloc_t	*pool;
  tcomm_rail_t		*dev[MAXRAILS];
} tcscomm_t;

typedef struct _tcsconn  {
  int	rvp;
  tcscomm_t	*comm;
} tcsconn_t;


typedef struct _host {
  char	*name;
  int	rail;
  int	id;
} host_t;

#ifdef never

static host_t hosts[] = { { "mini-t0", 0, 0 }, { "mini-t1", 0, 1 },
			  { "mini-t2", 0, 2 }, { "mini-t3", 0, 3 }, { "tcsini0", 0, 0 },
			  { "tcsini1", 0, 1 }, { "tcsini2", 0, 2 }, { "tcsini3", 0, 3 },
			  { "tcsini4", 0, 4 }, { "tcsini5", 0, 5 }, { "tcsini6", 0, 6 },
			  { "tcsini7", 0, 7 }, { "tcsini8", 0, 8 }, { "tcsini9", 0, 9 },
			  { "tcsini10", 0, 10 }, { "tcsini11", 0, 11 }, { "tcsini12", 0, 12 },
			  { "tcsini13", 0, 13 }, { "tcsini14", 0, 14 }, { "tcsini15", 0, 15 },
			  { "tcsini16", 0, 16 }, { "tcsini17", 0, 17 }, { "tcsini18", 0, 18 },
			  { "tcsini19", 0, 19 }, { "tcsini20", 0, 20 }, { "tcsini21", 0, 21 },
			  { "tcsini22", 0, 22 }, { "tcsini23", 0, 23 }, { "tcsini24", 0, 24 },
			  { "tcsini25", 0, 25 }, { "tcsini26", 0, 26 }, { "tcsini27", 0, 27 },
			  { "tcsini28", 0, 28 }, { "tcsini29", 0, 29 }, { "tcsini30", 0, 30 },
			  { "tcsini31", 0, 31 }, { "tcsini32", 0, 32 }, { "tcsini33", 0, 33 },
			  { "tcsini34", 0, 34 }, { "tcsini35", 0, 35 }, { "tcsini36", 0, 36 },
			  { "tcsini37", 0, 37 }, { "tcsini38", 0, 38 }, { "tcsini39", 0, 39 },
			  { "tcsini40", 0, 40 }, { "tcsini41", 0, 41 }, { "tcsini42", 0, 42 },
			  { "tcsini43", 0, 43 }, { "tcsini44", 0, 44 }, { "tcsini45", 0, 45 },
			  { "tcsini46", 0, 46 }, { "tcsini47", 0, 47 }, { "tcsini48", 0, 48 },
			  { "tcsini49", 0, 49 }, { "tcsini50", 0, 50 }, { "tcsini51", 0, 51 },
			  { "tcsini52", 0, 52 }, { "tcsini53", 0, 53 }, { "tcsini54", 0, 54 },
			  { "tcsini55", 0, 55 }, { "tcsini56", 0, 56 }, { "tcsini57", 0, 57 },
			  { "tcsini58", 0, 58 }, { "tcsini59", 0, 59 }, { "tcsini60", 0, 60 },
			  { "tcsini61", 0, 61 }, { "tcsini62", 0, 62 }, { "tcsini63", 0, 63 },
			  { "vis0", 0, 16 }, { "vis1", 0, 17 },
			  { "vis2", 0, 18 }, { "cog14", 0, 19} };

#define MAXHOSTS (sizeof(hosts)/sizeof(host_t))

#endif

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
  tcscomm_t *tcscomm_Init(char *lh, char *hh, int lctx, int hctx, int myrank);

  /*
    - tcscomm_Destroy closes a communications channel 'comm' as
    returned by tcscomm_Init.  
  */
  extern int tcscomm_Destroy(tcscomm_t *ptr);

  /*
    - Opens a connection with host "hostname" at rank "rank"
    - ptr == the connection pointer returned from _Init
    - hostname === the host to make the connection to 
    - rank == the rank of the process running on the remote node
    Returns connection id (conn) on good connection, -1 on failure.
  */
  extern int tcscomm_Open(tcscomm_t *ptr, char *hostname, int rank);

  /*
    - tcscomm_Close closes a connection id 'conn' as
    returned by tcscomm_Open.  
  */
  extern int tcscomm_Close(int conn);

  /*
    - tcscomm_Vrfy verifies the local host can send & receive data from
    the remote host by passing and receiving data.  It ensures a valid
    connections on both ends.

    - conn == the connection returned by tcscomm_Open
  */ 
  extern int tcscomm_Vrfy(int conn);

  /*
    - tcscomm_Select takes a list of connections, and returns the index of the
    connection id that has data available.  Returns -1 if no connection has 
    data waiting.

    - conn == an array of connections returned by tcscomm_Open
    - num_conns == number of elements in the array
    - timeout == seconds to wait before returning: 
    0 == no wait
    -1 == wait forever
	
  */ 
  extern int tcscomm_Select(int *conn, int num_conns, int timeout);

  /*
    - tcscomm_Poll returns the connection id that has data available and
    is at the head of the message queue.  Returns -1 if no connection has 
    data waiting.

    - timeout == seconds to wait before returning: 
    0 == no wait
    -1 == wait forever
	
  */ 
  extern int tcscomm_Poll(int conn, int timeout);
  extern int tcscomm_Peek( int conn );

  /*
    - tcscomm_Send sends "size" bytes of data in msgPtr.  Returns number of
    bytes sent, 0 on error.  As there are no partial data transmissions,
    will always return 'size' or 0 on error.  

    tcscomm_Send blocks until the send completes.
  */
  extern int tcscomm_Send(int conn, void *msgPtr, int size);

  /*
    - tcscomm_Recv receives data from the matching tcscomm_Send.  Size is
    the number of bytes poitber to by msgPtr, and returns the number of
    bytes actually received, and 0 on an error.

    tcscomm_Recv blocks until the receive completes.
  */
  extern int tcscomm_Recv(int conn, void *msgPtr, int size);

  /*
    - tcscomm_Malloc
  */
  extern void *tcscomm_Malloc(tcscomm_t *comm, int size);

  /*
    - tcscomm_Free
  */
  extern void tcscomm_Free(tcscomm_t *comm, void *ptr);

#ifdef __cplusplus
}
#endif // __cplusplus

