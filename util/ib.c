/* Copyright (c) 2001, Stanford University
 * InfiniBand code Copyright (c) 2003, GraphStream, Inc.
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */


/* Infiniband Layer for Chromium
 * Carl Rosenberg, GraphStream, Belmont, CA. 650-714-2424. carl@graphstream.com
 *
 * This is the Infiniband network layer for Chromium. It is invoked
 * when the python script asks for 'ib'. It is a parallel TCP/IP + IB
 * layer developed using the Chromium TCP/IP layer as a base.
 *
 * Infiniband nominally expects to have a connection manager running
 * within the fabric. With a connection manager, concepts such as ports
 * and addresses are supported; asking to connect to port xyz on a 128-bit
 * (IPv6) address makes sense.
 *
 * As of December 2, 2003, connection managers have been a bit of 
 * an anomaly in our IB fabric. Since it may well be non-existent middleware
 * in other environments as well, this version of IB connects through TCP/IP,
 * transfers a little bit of control information, and then opens a parallel
 * connection in Infiniband. The normal TCP/IP infrastructure is therefore 
 * in place; hosts are referenced by their IP address, and have ports, etc.
 *
 * As part of the TCP connection process, immediately upon connection,
 * the handshake required for IB is passed. This consists of remote
 * access keys and some information so that a queue pair can be formed
 * on each end.
 *
 * The total TCP usage is on the order of bytes per session. Since the
 * TCP code is largely left from the original TCP layer, it is rather
 * ornate for such a small task. E.g., sockets that will only transfer
 * 20 bytes, ever, don't need to be tuned for high volume throughput,
 * but that code has not been changed.
 *
 * */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <math.h>
#include <sched.h>

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>

#include <sys/ioctl.h>
#include <unistd.h>

#include <vapi.h>
#include <vapi_common.h>

#include "cr_error.h"
#include "cr_mem.h"
#include "cr_string.h"
#include "cr_bufpool.h"
/*VAPI includes are in cr_net.h*/
#include "cr_net.h"
#include "cr_endian.h"
#include "cr_threads.h"
#include "cr_environment.h"
#include "net_internals.h"
/*IB code should have assertions on even if the rest of Chromium
 * is stable enough to have them off. IBASSERT==CRASSERT*/
#define IBASSERT( PRED ) ((PRED)?(void)0:crError( "Assertion failed: %s, file %s, line %d", #PRED, __FILE__, __LINE__))

int __crIBSelect( int n, fd_set *readfds, int sec, int usec );
static void checksends(CRConnection *conn);

#ifdef ADDRINFO
#define PF PF_UNSPEC
#endif

typedef enum {
	CRIBMemory,
	CRIBMemoryBig
} CRIBBufferKind;

#define CR_IB_BUFFER_MAGIC 0x89134533
/* size of buffer. This needs to be checked against an mtu which exists
 * elsewhere in Chromium; just setting it to say 131K will cause 
 * protection errors. */
#define BUFMAX 256*1024

/*max number on queue. Must be at least 2, not sure it needs to be more*/
#define K 64

/* max number of open connections. Presumably this needs to be as big as 
 * the node count. */
#define MK 1024

/* Size of ack package. Since the data is not used yet, it can be any size
 * greater than zero. Eventually it may be nice to generate these only 
 * for every Kth transmission, in which case there will likely be content.*/
#define ACKSZ 1

/*level 1 includes copious information. This can be made a variable if desired*/
#define IBDEBUGLEVEL 0

enum ibstate {ibnull, /*baseline state*/

         /*Read states*/
         ibrdposted,  /*a read has been posted*/
	 ibrdready,   /*a read has been fulfilled; data is ready*/
	 ibrdacked,   /*a read has been acknowledged; data has been processed*/

	 /*Send states*/
         ibsdposted,  /*a send has been posted*/
	 ibsdsent,    /*a send has been sent; IB says it is finished*/
	 ibsdacked,   /*a send has been acknowledged, the other side is 
			finished reading, a buffer is now available again*/
     
	 /*Ack Send States*/
	 ibasdposted, /*a received data acknowledgement has been posted*/
	 ibasdsent,   /*a received data ack has been sent*/

	 /*Ack Read States*/
         ibardposted, /*a read for the ack has been posted*/
	 ibardready   /*an ack has arrived*/
};

struct bufib {
    char buf[BUFMAX];
    int len;
    int pos;
    enum ibstate state;
    int id; /*Infiniband transmission id*/
    int transid; /*Chromium transmission id. 0x10000 = ack. low order 16=num*/
    int next;	/*integer pointer, for read buffers, to the next one*/
    unsigned int ack;
    VAPI_rr_desc_t rr;
    VAPI_sr_desc_t sr;
    VAPI_sg_lst_entry_t sgl;
};

struct ibmem {
        /* This is the holder for all of the buffers for a given IB
	 * connection. All buffers for each connection will be locked down 
	 * together and given one set of l/r keys.*/
	VAPI_mr_hndl_t		mhndl;	/*Memory handle*/
	VAPI_lkey_t		lkey;
	VAPI_rkey_t		rkey;
	VAPI_hca_hndl_t hndl;	/*HCA handle*/
	VAPI_pd_hndl_t pdhndl;	/*Protection domain handle*/
	int handleset;		/*are HCA and pdhndl set? don't clear on close*/
	VAPI_cq_hndl_t rcqhndl;   /*Receive queue handle*/
	VAPI_cq_hndl_t scqhndl;   /*Send queue handle*/
	unsigned int ibport;    /*physical port*/
	VAPI_qp_hndl_t qphndl;	/*queue pair handle*/
	VAPI_qp_num_t qpn,remoteqpn; /*local and remote queue pair numbers*/
	IB_lid_t lid,remotelid;	/*local and remote lids*/
	VAPI_psn_t sqpsn,remotesqpsn;	/*local and remote sq psn initial values.
					  rq will be that of remote sq.*/

	struct bufib 
	    r[K],	/*data receive buffers*/
	    w[K],	/*data transmit buffers*/
	    ra,		/*ack receive buffer*/
	    sa;		/*ack transmit buffer*/
	int curread;	/*current buffer for reading, if any, or -1*/
	int transidcounter;	/*used for forming transaction ids, one per data send, for this queue pair*/
	int idcounter;	/*used for forming session unique id's for this queue pair*/
	int readsposted;/*non-ack reads*/
	int sendsposted;/*non-ack sends*/
}; 

typedef struct CRIBBuffer {
	unsigned int          magic;
	CRIBBufferKind     kind;
	unsigned int          len;
	unsigned int          allocated;
	unsigned int		ibmagic;
	VAPI_mr_hndl_t		hndl;
	VAPI_lkey_t		lkey;
	VAPI_rkey_t		rkey;
	unsigned int          pad; /*This must be last, it is tromped on from above. */
} CRIBBuffer;

int crIBErrno( void )
{
	int err = errno;
	errno = 0;
	return err;
}

char *crIBErrorString( int err )
{
	static char buf[512], *temp;
	
	temp = strerror( err );
	if ( temp )
	{
		crStrncpy( buf, temp, sizeof(buf)-1 );
		buf[sizeof(buf)-1] = 0;
	}
	else
	{
		sprintf( buf, "err=%d", err );
	}

	return buf;
}

void crIBCloseSocket( CRSocket sock )
{
	int fail;

	if (sock <= 0)
		return;

	shutdown( sock, 2 /*  RDWR */ );
	fail = ( close( sock ) != 0 );
	if ( fail )
	{
		int err = crIBErrno( );
		crWarning( "crIBCloseSocket( sock=%d ): %s",
					   sock, crIBErrorString( err ) );
	}
}

static unsigned short last_port = 0;

struct {
	int                  initialized;
	int                  num_conns;
	CRConnection         **conns;
	CRBufferPool         *bufpool;
#ifdef CHROMIUM_THREADSAFE
	CRmutex              mutex;
	CRmutex              recvmutex;
#endif
	CRNetReceiveFuncList *recv_list;
	CRNetCloseFuncList *close_list;
	CRSocket             server_sock;
} cr_ib;

static int getid(CRConnection *conn){
    /* get a unique identifier. Read and write requests are 
     * given session-unique identifiers.*/
    return conn->ib->idcounter++;
}

static int gettransid(CRConnection *conn){
    /*Transaction id. The same id is used on the return, with 0x10000
     * or-ed in. This way we know who is being acked, for future use
     * with RDMA if need be.*/
    int ret = 0xffff & conn->ib->transidcounter++;
    if (ret == 0) /*so we don't collide with items which have been reset*/
	ret = 0xffff & conn->ib->transidcounter++;
    return ret;
}

static double usecs(){
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv,&tz);
    return tv.tv_sec + tv.tv_usec/1000000.;
}

static int csum(unsigned char *x, int len){
/*inspired by Numerical Algorithms in C page 900*/
    int i,j,crc = 0;
    return 0;
    if (len <= 0) return -1;
    for (i = 0; i < len; i++){
	unsigned short ans = (crc ^ x[i] << 8);
	for (j = 0; j < 8; j++){
	    if (ans & 0x8000) {
		ans <<= 1;
		ans ^= 4129;
	    } else 
		ans <<= 1;
	}
	crc = ans;
    }
    return crc;
}

static void dumptrans(CRConnection *conn,int i, struct bufib *b,char *msg){
    int j;
    if (IBDEBUGLEVEL == 0) return;
    crWarning("Socket %d Transmission dump at %f, buf(%d) len(x:%d/r:%d/s:%d) csum(%d),%s",
	    conn->tcp_socket,usecs(),i,b->len,b->rr.sg_lst_p->len,b->sr.sg_lst_p->len,csum(b->buf,b->len),msg);
    crWarning("Transid %d(s:%d) id %d rid %d sid %d",
	    b->transid,(int)b->sr.imm_data,b->id,(int)b->rr.id,(int)b->sr.id);
    if (0){
	for (j = 0; j < b->len; j+=25){
	    int k;
	    char buf[100];
	    for (k = 0; (k < 25) && (k < (b->len-j));k++){
		sprintf(buf+k*2,"%2x",b->buf[j+k]);
	    }
	    crWarning(buf);
	    for (k = 0; (k < 25) && (k < (b->len-j));k++){
		if (isprint(b->buf[j+k])){
		    sprintf(buf+k,"%c",b->buf[j+k]);
		} else {
		    sprintf(buf+k,"%c",'.');
		}
	    }
	    crWarning(buf);
	}
    } else {
	crWarning("data dump suppressed");
    }
}

static void initrr(CRConnection *conn, VAPI_rr_desc_t *x, VAPI_sg_lst_entry_t *s){
    x->id = getid(conn);
    x->opcode = VAPI_RECEIVE;
    x->comp_type = VAPI_SIGNALED;
    x->sg_lst_p = s;
    x->sg_lst_len = 1;
}

static void initsr(CRConnection *conn, VAPI_sr_desc_t *x, VAPI_sg_lst_entry_t *s){
    x->id = getid(conn);
    x->opcode = VAPI_SEND_WITH_IMM;
    x->comp_type = VAPI_SIGNALED;
    x->sg_lst_p = s;
    x->sg_lst_len = 1;
    x->imm_data = 299;
    x->set_se = 1;
}

static void initsgl(VAPI_sg_lst_entry_t *x, char *b, VAPI_lkey_t lkey){
    x->addr = (VAPI_virt_addr_t)(unsigned)(b);
    x->len = 0;
    x->lkey = lkey;
}

static void initbufib(CRConnection *conn,struct bufib *b, VAPI_lkey_t lkey){
    b->len = 0;
    b->pos = 0;
    b->id = 0;
    b->transid = 0;
    b->next = -1;
    b->state = ibnull;
    initrr(conn,&b->rr,&b->sgl);
    initsr(conn,&b->sr,&b->sgl);
    initsgl(&b->sgl,b->buf,lkey);
}

#if 0
static int completeread(CRConnection *conn){
    VAPI_ret_t ret;
    if ((ret = EVAPI_peek_cq(conn->ib->hndl,conn->ib->rcqhndl,1)) != VAPI_OK){
	crDebug("completeread IB socket(%d) nothing is waiting (%s) at (%f)",conn->tcp_socket, VAPI_strerror(ret),usecs());
        return 0;
    }
    crDebug("completeread IB socket(%d) something is waiting",conn->tcp_socket);
    return 1;
}

static int cyclicinc(int i){
    return (i+1) % K;
}
#endif
#if 0
static int completesend(CRConnection *conn){
    if (EVAPI_peek_cq(conn->ib->hndl,conn->ib->scqhndl,1) != VAPI_OK) {
	int t = (int)time(NULL);
	crDebug("completesend IB socket(%d) nothing is waiting at (%d)",conn->tcp_socket, t);
        return 0;
    }
    crDebug("completesend IB socket(%d) something is waiting",conn->tcp_socket);
    return 1;
}
#endif
#if 0
static void sleep100ms(){
    struct timespec req, rem;
    req.tv_sec = 0;
    req.tv_nsec = 100000000; /*100 ms*/
    (void)nanosleep(&req,&rem);
}
#endif
#if 0
static void sleep10ms(){
    struct timespec req, rem;
    req.tv_sec = 0;
    req.tv_nsec = 10000000; /*10 ms*/
    (void)nanosleep(&req,&rem);
}
#endif
#if 0
static void sleep1ms(){
    struct timespec req, rem;
    req.tv_sec = 0;
    req.tv_nsec = 1000000; /*1 ms*/
    (void)nanosleep(&req,&rem);
}
#endif
static void sleepusec(int howmany){
    struct timespec req, rem;
    req.tv_sec = howmany/1000000;
    req.tv_nsec = (howmany%1000000)*1000; /*1 ms*/
    if (IBDEBUGLEVEL)
	crWarning("sleeping for %5.2f milliseconds...",howmany/1000.0);
    /*nanosleep does not work as desired under Linux 2.4, it comes back
in 50ms regardless*/
    if (0) {
	    (void)nanosleep(&req,&rem);
    } else {
        int i;
        static double f;
        sched_yield();
        /*this works on Mike Houston's Xeon system, not elsewhere*/
        for (i = 0; i < 10*howmany; i++){
            f = sin(0.22333);
        }
        sched_yield();
    }
}
#if 0
static void waitforsend(CRConnection *conn){
    struct timespec req, rem;
    int count = 0;
    req.tv_sec = 0;
    req.tv_nsec = 10000000; /*10 ms*/
    while (!completesend(conn)){
	(void)nanosleep(&req,&rem);
	count++;
	if ((count % 100) == 0)
	    crWarning("Have slept %d seconds waiting for a send",count/100);
	if (count > 3000)
	    crError("ERROR: timeout waiting for Infiniband Send (30 sec)");
    }
}
#endif
static void initibmem(CRConnection *conn){
    int i; 
    VAPI_ret_t ret;
    conn->ib->curread = -1;
    for (i = 0; i < K; i++){
	initbufib(conn,&conn->ib->r[i],conn->ib->lkey); /*initialize read buffers*/
	initbufib(conn,&conn->ib->w[i],conn->ib->lkey); /*initialize write buffers*/
	conn->ib->r[i].rr.id = conn->ib->r[i].id = getid(conn);
	ret = VAPI_post_rr(conn->ib->hndl,conn->ib->qphndl,&(conn->ib->r[i].rr));
	if (ret != VAPI_OK)
	    crError("initibmem r VAPI_post_rr: %s",VAPI_strerror(ret));
	if (IBDEBUGLEVEL)
	    crWarning("socket(%d) read buf(%d) state init to ibrdposted in initibmem",
		conn->tcp_socket,i);
	conn->ib->r[i].state = ibrdposted;
    }
}
static int 
ibhca ( CRConnection *conn )
{
    /*find an hca and set conn->hndl */
#define MAXHCA 3
    /*Multiple HCAs are supported by Mellanox, but not here.*/
    /*Return the first HCA, that is what we will work with*/
    /*(Of course the one HCA has two ports)*/
    VAPI_ret_t ret;
    VAPI_hca_hndl_t hndl;
    VAPI_hca_id_t hcaidbuf[MAXHCA]; /*HCA id buffers, should be at least 2*/
    u_int32_t numhcas;
    int i;

    ret = EVAPI_list_hcas(MAXHCA,&numhcas,hcaidbuf);
    if (ret != VAPI_OK){
	    crError("Chromium crIBInit EVAPI_list_hcas: %s",VAPI_strerror(ret));
	    return 1;
    }
    if (numhcas != 1)
	    crWarning("There are %d hcas, but Chromium only supports the first.",numhcas);
    ret = EVAPI_get_hca_hndl(hcaidbuf[0], &hndl);
    if (ret != VAPI_OK) {
	    crError("Chromium crIBInit getting hca handle: (EVAPI_get_hca_hndl): %s",VAPI_strerror(ret));
	    return 1;
    }
    conn->ib->hndl = hndl;
    for (i = 1; i < 3; i++){
	VAPI_hca_port_t prop;
	ret = VAPI_query_hca_port_prop(hndl,i,&prop);
	if ((ret == VAPI_OK) &&
	    (prop.state == PORT_ACTIVE)){
	    conn->ib->ibport = i;
	    conn->ib->lid = prop.lid;
	    crDebug("found active IB port %d",i);
	    return 0;
	} else if (ret != VAPI_OK){
	    crWarning("No port properties for port %d",i);
	}
    }
    crError("Could not find an active port with VAPI_query_hca_port_prop");
    return 1;
}

static int 
ibpd ( CRConnection *conn )
{
    /*get a protection domain and set conn->pdhndl */
    VAPI_ret_t ret;
    if ((ret = VAPI_alloc_pd(conn->ib->hndl,&conn->ib->pdhndl)) != VAPI_OK){
	crError("ibpd (VAPI_alloc_pd): %s",VAPI_strerror(ret));
	return 1;
    }
    crDebug("Protection domain %d allocated.",(int)conn->ib->pdhndl);
    return 0;
}

static int 
ibcqs ( CRConnection *conn )
{
    /*get a couple of completion queues, and set conn->scqhndl/rcqhndl */
#define CQSZ 400
    VAPI_ret_t ret;
    VAPI_cqe_num_t num;
    if ((ret = VAPI_create_cq(conn->ib->hndl,CQSZ,&(conn->ib->rcqhndl),&num)) != VAPI_OK){
	crError("ibcqs (VAPI_create_cq 1): %s",VAPI_strerror(ret));
	return 1;
    }
    crDebug("ibcqs Setting rcqhndl %d",(int)conn->ib->rcqhndl);
    if (num < CQSZ)
	crWarning("ibcqs asked for %d, got %d",CQSZ,num);
    if ((ret = VAPI_create_cq(conn->ib->hndl,CQSZ,&(conn->ib->scqhndl),&num)) != VAPI_OK){
	crError("ibcqs (VAPI_create_cq 2): %s",VAPI_strerror(ret));
	return 1;
    }
    if (num < CQSZ)
	crWarning("ibcqs 2 asked for %d, got %d",CQSZ,num);
    return 0;
}

static void
sendlocalinfo ( CRConnection *conn )
{
    /* a socket has just been opened*/
    /* bundle up our local IB info, and send it*/
    char buf[500];
    sprintf(buf,"%d %d %d",(int)conn->ib->qpn,(int)conn->ib->lid,(int)conn->ib->sqpsn);
    crDebug("sendlocalinfo sending qpn/lid/sqpsn %s",buf);
    int len = write(conn->tcp_socket,buf,strlen(buf));
    if (len <= 0){
	crError("sendlocalinfo, did not put IB information on first write of a new socket.");
	return;
    }
    return;
}

static void
sendlocalinfo2 ( CRConnection *conn )
{
    /*synchronization at startup, IB is now ready to receive*/
    char buf[500];
    sprintf(buf,"1");
    int len = write(conn->tcp_socket,buf,strlen(buf));
    if (len <= 0){
	crError("sendlocalinfo, did not put IB information on second write of a new socket.");
	return;
    }
    return;
}

static void
getremoteinfo ( CRConnection *conn )
{
    /* a socket has just been opened*/
    /* read some remote IB info, and store it in conn */
    char buf[500];
    int qpn,lid,sqpsn;
    /* we don't want ib_readexact, it keeps going until
     * it gets what it wants. We just want some stuff */
    int len = read(conn->tcp_socket,buf,499);
    if (len <= 0){
	crError("getremoteinfo, did not get IB information on first read of a new socket.");
	return;
    }
    buf[len] = 0;
    if (sscanf(buf, "%d %d %d",&qpn,&lid,&sqpsn) < 3){
	/* a little bit risky error, if given total garbage*/
	crError("getremoteinfo, malformed ib info on first read (%s).",buf);
	return;
    }
    crDebug("getremoteinfo got this from peer(%s)",buf);
    conn->ib->remoteqpn = qpn;
    conn->ib->remotelid = lid;
    conn->ib->remotesqpsn = sqpsn;
    crDebug("getremoteinfo set rqpn %d rlid %d rsqpsn %d",
	    (int)conn->ib->remoteqpn, (int)conn->ib->remotelid, (int)conn->ib->remotesqpsn);
}

static void
getremoteinfo2 ( CRConnection *conn )
{
    char buf[500];
    /* we don't want ib_readexact, it keeps going until
     * it gets what it wants. We just want some stuff */
    int len = read(conn->tcp_socket,buf,499);
    if (len <= 0){
	crError("getremoteinfo, did not get IB information on first read of a new socket.");
	return;
    }
    /*else we're done, could care less about what was sent*/
    return;
}

static int
ibconn ( CRConnection *conn)
{
    /*set up rc queue pairs with the system whose lid and qpnum we magically know*/
    /*move it through all states*/
    /*This works for clients, because there will be a port waiting*/
    VAPI_ret_t ret;
    VAPI_qp_init_attr_t initattr;
    VAPI_qp_prop_t prop;
    VAPI_qp_attr_t attr;
    VAPI_qp_attr_mask_t attr_mask;
    VAPI_qp_cap_t cap;
    static int psnstart = 23; /*this needs to vary, 23 is an arbitrary start*/

    /*create the queue pair, with typical values*/
    initattr.sq_cq_hndl = conn->ib->scqhndl;
    initattr.rq_cq_hndl = conn->ib->rcqhndl;
    initattr.cap.max_oust_wr_sq = 1000;
    initattr.cap.max_oust_wr_rq = 1000;
    initattr.cap.max_sg_size_sq = 10;
    initattr.cap.max_sg_size_rq = 10;
    initattr.sq_sig_type = VAPI_SIGNAL_ALL_WR;
    initattr.rq_sig_type = VAPI_SIGNAL_ALL_WR;
    initattr.pd_hndl = conn->ib->pdhndl;
    initattr.ts_type = VAPI_TS_RC;
    if ((ret = VAPI_create_qp(conn->ib->hndl,&initattr,&conn->ib->qphndl,&prop)) != VAPI_OK){
	crError("ibconn (VAPI_create_cq): %s",VAPI_strerror(ret));
	return 1;
    }
    conn->ib->qpn = prop.qp_num;
    conn->ib->sqpsn = psnstart++; 

    /*Convert from reset to init state*/
    QP_ATTR_MASK_CLR_ALL(attr_mask);
    attr.qp_state = VAPI_INIT;
    QP_ATTR_MASK_SET(attr_mask,QP_ATTR_QP_STATE);
    attr.remote_atomic_flags |= VAPI_EN_REM_WRITE;
    attr.remote_atomic_flags |= VAPI_EN_REM_READ;
    QP_ATTR_MASK_SET(attr_mask,QP_ATTR_REMOTE_ATOMIC_FLAGS);
    attr.pkey_ix = 0;   /*MAGIC: is this always valid?*/
    QP_ATTR_MASK_SET(attr_mask,QP_ATTR_PKEY_IX);
    attr.port = 1;	/*MAGIC: how do we get this value?*/
    QP_ATTR_MASK_SET(attr_mask,QP_ATTR_PORT);
    if ((ret = VAPI_modify_qp(conn->ib->hndl,conn->ib->qphndl,&attr,&attr_mask,&cap)) != VAPI_OK){
	crError("ibconn (VAPI_modify_qp RST-to-INIT): %s",VAPI_strerror(ret));
	return 1;
    }

    return 0;
}

static void
ibclose ( CRConnection *conn )
{
    VAPI_ret_t ret;
    if (!conn->tcp_socket) return;
    if (!conn->ib->hndl) return; /*apparently these don't start at 0*/
    crDebug("IBCLOSE has been called %d", conn->tcp_socket);
    conn->ib->lkey = 0;
    crDebug("IBCLOSE deregistering memory %d", (int)conn->ib->mhndl);
    if ((ret = VAPI_deregister_mr(conn->ib->hndl,conn->ib->mhndl)) != VAPI_OK)
	crError("ibclose (VAPI_deregister_mr): %s",VAPI_strerror(ret));
    crDebug("IBCLOSE destroying qp %d", (int)conn->ib->qphndl);
    if ((ret = VAPI_destroy_qp(conn->ib->hndl, conn->ib->qphndl)) != VAPI_OK)
	crError("ibclose (VAPI_destroy_qp): %s",VAPI_strerror(ret));
    crDebug("IBCLOSE destroying rcq %d", (int)conn->ib->rcqhndl);
    if ((ret = VAPI_destroy_cq(conn->ib->hndl, conn->ib->rcqhndl)) != VAPI_OK)
	crError("ibclose (VAPI_destroy_cq rcq): %s",VAPI_strerror(ret));
    crDebug("IBCLOSE destroying scq %d", (int)conn->ib->scqhndl);
    if ((ret = VAPI_destroy_cq(conn->ib->hndl, conn->ib->scqhndl)) != VAPI_OK)
	crError("ibclose (VAPI_destroy_cq scq): %s",VAPI_strerror(ret));
    /* old order, don't close these, leave them around for next*/
    crDebug("IBCLOSE deallocing pdhndl %d", (int)conn->ib->pdhndl);
    if ((ret = VAPI_dealloc_pd(conn->ib->hndl, conn->ib->pdhndl)) != VAPI_OK)
	crError("ibclose (VAPI_dealloc_pd): %s",VAPI_strerror(ret));
    crDebug("IBCLOSE releasing hca_hndl %d", (int)conn->ib->hndl);
    if ((ret = EVAPI_release_hca_hndl(conn->ib->hndl)) != VAPI_OK)
	crError("ibclose (VAPI_release_hca_hndl): %s",VAPI_strerror(ret));
    /*Close an IB connection */
}

static void setupmb(CRConnection *conn)
{
    VAPI_ret_t ret;
    VAPI_mrw_t mrw;
    VAPI_mr_hndl_t mhndl;
    VAPI_mrw_t mrwout;
    if (conn->ib->lkey != 0) return;
    crDebug("setupmb socket %d", (int)conn->tcp_socket);
    if ((conn->tcp_socket < 0) || (conn->tcp_socket >= MK))
	crError("Too many IB connections, or bad socket number in IB layer.");
    mrw.type = VAPI_MR;
    mrw.start = (VAPI_virt_addr_t)(unsigned)(conn->ib);
    mrw.size = sizeof(struct ibmem);
    crDebug("pd handle in setupmb is %d; asking for %d bytes starting at %x sbrk(%x)",(int)conn->ib->pdhndl, (int)mrw.size, (int)mrw.start, (int)sbrk(0));
    mrw.pd_hndl = conn->ib->pdhndl;
    mrw.acl = VAPI_EN_LOCAL_WRITE|VAPI_EN_REMOTE_READ|VAPI_EN_REMOTE_WRITE;

    /*check to see that they are really in memory*/
    {int i,j;
	for (i = 0; i < K; i++){
	    for (j = 0; j < BUFMAX; j += 100){
		conn->ib->r[i].buf[j] = 0;
		conn->ib->w[i].buf[j] = 0;
	    }
	}
    }

    if ((ret = VAPI_register_mr(conn->ib->hndl,&mrw,&mhndl,&mrwout)) != VAPI_OK){

	crError("setupmb VAPI_register_mr: %s",VAPI_strerror(ret));
    }
    crDebug("Memory handle for socket %d is %d",conn->tcp_socket,(int)mhndl);
    conn->ib->mhndl = mhndl;
    conn->ib->lkey = mrwout.l_key;
    conn->ib->rkey = mrwout.r_key;
    initibmem(conn);
}

static int 
ibconnect ( CRConnection *conn )
{
    conn->ib->transidcounter = 77;
    conn->ib->idcounter = 100; /*arbitrary starting point for unique ids*/
    conn->ib->readsposted = 0; /*counted when recycled, so it means reads processed*/
    conn->ib->sendsposted = 0;
    if (ibhca(conn) ||
	ibpd(conn) ||
	ibcqs(conn) ||
	ibconn(conn))
	return 1;
    return 0;
}

static int 
ibconnect2 ( CRConnection *conn )
{
    /*do that stuff which can only be done when we have some info 
     * from the other side*/
    /*Convert from INIT to RTR state*/
    VAPI_ret_t ret;
    VAPI_qp_attr_t attr;
    VAPI_qp_attr_mask_t attr_mask;
    VAPI_qp_cap_t cap;

    QP_ATTR_MASK_CLR_ALL(attr_mask);
    attr.qp_state = VAPI_RTR;
    QP_ATTR_MASK_SET(attr_mask,QP_ATTR_QP_STATE);
    attr.av.sl = 0;	/*CHECK*/
    attr.av.grh_flag = 0;
    attr.av.dlid = conn->ib->remotelid;
    attr.av.static_rate = 1;	/*CHECK*/
    attr.av.src_path_bits = 1;	/*CHECK*/
    attr.av.port = 1;		/*CHECK; I don't think this is used per .h*/
    QP_ATTR_MASK_SET(attr_mask,QP_ATTR_AV);
    attr.rq_psn = conn->ib->remotesqpsn;	/* which should be set by now*/
    QP_ATTR_MASK_SET(attr_mask,QP_ATTR_RQ_PSN);
    attr.dest_qp_num = conn->ib->remoteqpn;	/* also set*/
    QP_ATTR_MASK_SET(attr_mask,QP_ATTR_DEST_QP_NUM);

    /*Conflicting reports on Path MTU. But Mellanox layer complains
     * if it is not set. */
    attr.path_mtu = MTU256;
    QP_ATTR_MASK_SET(attr_mask,QP_ATTR_PATH_MTU);

    attr.qp_ous_rd_atom = 1; /*no particular idea what a good value would be*/
    QP_ATTR_MASK_SET(attr_mask,QP_ATTR_QP_OUS_RD_ATOM);
    attr.min_rnr_timer = IB_RNR_NAK_TIMER_0_08;
    QP_ATTR_MASK_SET(attr_mask,QP_ATTR_MIN_RNR_TIMER);
    /*the following were thought to be required but aren't*/
    /*attr.cap.max_oust_wr_sq = 10;
    attr.cap.max_oust_wr_rq = 10;
    attr.cap.max_sg_size_sq = 10;
    attr.cap.max_sg_size_rq = 10;
    attr.cap.max_inline_data_sq = 10;
    QP_ATTR_MASK_SET(attr_mask,QP_ATTR_CAP);*/
    if ((ret = VAPI_modify_qp(conn->ib->hndl,conn->ib->qphndl,&attr,&attr_mask,&cap)) != VAPI_OK){
	crError("ibconn (VAPI_modify_qp INIT-to-RTR): %s",VAPI_strerror(ret));
	return 1;
    }

    /*Convert from RTR to RTS state*/
    /*Here we will set timeout, retry_count, and rnr_retry.
     * One or more of these is sensitive. Values of 200 for retrys,
     * and either 0x20 or 3 for timeout, did not work at all.*/
    QP_ATTR_MASK_CLR_ALL(attr_mask);
    attr.qp_state = VAPI_RTS;
    QP_ATTR_MASK_SET(attr_mask,QP_ATTR_QP_STATE);
    attr.timeout = 18;	/*CHECKED*/
    QP_ATTR_MASK_SET(attr_mask,QP_ATTR_TIMEOUT);
    attr.sq_psn = conn->ib->sqpsn;	
    QP_ATTR_MASK_SET(attr_mask,QP_ATTR_SQ_PSN);
    attr.retry_count = 200;	/*CHECKED*/ /*and then changed from 6*/
    QP_ATTR_MASK_SET(attr_mask,QP_ATTR_RETRY_COUNT);
    attr.rnr_retry = 6;	/*CHECKED*/
    QP_ATTR_MASK_SET(attr_mask,QP_ATTR_RNR_RETRY);
    /*say what? is this supposed to be passed back in the protocol of setup?*/
    attr.ous_dst_rd_atom = 1; /*no particular idea what a good value would be*/
    QP_ATTR_MASK_SET(attr_mask,QP_ATTR_OUS_DST_RD_ATOM);
    if ((ret = VAPI_modify_qp(conn->ib->hndl,conn->ib->qphndl,&attr,&attr_mask,&cap)) != VAPI_OK){
	crError("ibconn (VAPI_modify_qp RTR-to-RTS): %s",VAPI_strerror(ret));
	return 1;
    }
    setupmb(conn);
    return 0;
}

static char *istate(enum ibstate x){
    switch (x){
	case ibnull: return "ibnull";
	case ibrdposted: return "ibrdposted";
	case ibrdready: return "ibrdready";
	case ibrdacked: return "ibrdacked";
	case ibsdposted: return "ibsdposted";
	case ibsdsent: return "ibsdsent";
	case ibsdacked: return "ibsdacked";
	case ibasdposted: return "ibasdposted";
	case ibasdsent: return "ibasdsent";
	case ibardposted: return "ibardposted";
	case ibardready: return "ibardready";
    }
    return "INVALIDSTATE";
}

static void dumpstate(CRConnection *conn, char *msg)
{
    int i;
    crWarning("Dump of connection(%d) (%s) reads(%d) sends(%d) curread(%d)",
	    conn->tcp_socket,msg,conn->ib->readsposted,conn->ib->sendsposted,
	    conn->ib->curread);
    if (IBDEBUGLEVEL == 0){
	crWarning("You may want to turn on IBDEBUGLEVEL in the source to find out what is happening.\n");
	return;
    }
    for (i = 0; i < K; i++){
	crWarning("Send buffer(%d) state(%s) next(%d) pos(%d) len(%d) id(%d) transid(%d)",
		i, istate(conn->ib->w[i].state),
		conn->ib->w[i].next, conn->ib->w[i].pos,
		conn->ib->w[i].len, conn->ib->w[i].id,
		conn->ib->w[i].transid);
	crWarning("Read buffer(%d) state(%s) next(%d) pos(%d) len(%d) id(%d) transid(%d)",
		i, istate(conn->ib->r[i].state),
		conn->ib->r[i].next, conn->ib->r[i].pos,
		conn->ib->r[i].len, conn->ib->r[i].id,
		conn->ib->r[i].transid);
    }
}
#if 0
static void waituntilsend(CRConnection *conn)
{
    /*Wait until there is at least one entry in the send queue.*/
    VAPI_ret_t ret;
    while ((ret = EVAPI_peek_cq(conn->ib->hndl,conn->ib->scqhndl,1)) != VAPI_OK)
    {
        crDebug("waituntilsend checking for 1, %s",VAPI_strerror(ret));
        sleep10ms();
    }
    crDebug("waituntilsend found 1, %s",VAPI_strerror(ret));
}

static int emptyq(CRConnection *conn)
{
    VAPI_ret_t ret;
    ret = EVAPI_peek_cq(conn->ib->hndl,conn->ib->rcqhndl,1);
    if (ret == VAPI_OK) return 0;
    return 1;
}

static void waituntil1(CRConnection *conn)
{
    /*Wait until there is at least one entry in the recieve queue.*/
    VAPI_ret_t ret;
    while ((ret = EVAPI_peek_cq(conn->ib->hndl,conn->ib->rcqhndl,1)) != VAPI_OK)
    {
        crDebug("waituntil1 checking for 1, %s",VAPI_strerror(ret));
        /*sleep(1);*/
    }
    crDebug("waituntil1 found 1, %s",VAPI_strerror(ret));
}
#endif
static void dumpsr(char *mes, VAPI_sr_desc_t srd)
{
    crWarning("Dumping VAPI_sr_desc_t %s",mes);
    crWarning("id(%d) opcode(%d) comp_type(%d) fence(%d) set_se(%d)",
	    (int)srd.id,(int)srd.opcode,(int)srd.comp_type,
	    (int)srd.fence,(int)srd.set_se);
    crWarning("gl.addr(%x) gl.len(%d) gl.lkey(%d)",
	    (unsigned)srd.sg_lst_p->addr,(int)srd.sg_lst_p->len,
	    (unsigned)srd.sg_lst_p->lkey);
}

static int getwritebuf(CRConnection *conn){
    /*Is there a write buffer available?*/
    /*Modification: this ran into a problem when using all of them.
     * So only return true if there are at least 3*/
    int i;
    int ret = 0;
    int first = -1;
    IBASSERT(K > 6);
    for (i = 0; i < K; i++){
	if (conn->ib->w[i].state == ibnull){
	    ret++;
	    if (ret == 1) first = i;
	}
    }
    if (ret < 3)return -1;
    return first;
}

static void processwcompletion(CRConnection *conn,int id){
    int i;
    for (i = 0; i < K; i++){
	if (id == conn->ib->w[i].id){
	    if (conn->ib->w[i].state == ibasdposted){
		if (IBDEBUGLEVEL)
		    crWarning("socket(%d) write buf(%d) id(%d) state change from ibasdposted to ibnull on wcompletion %f",
			conn->tcp_socket,i,id,usecs());
		conn->ib->w[i].state = ibasdsent;
		/*we won't get acked for an ack, so move on*/
		conn->ib->w[i].state = ibnull;
		conn->ib->w[i].transid = 0;
		conn->ib->w[i].id = 0;
	    } else if (conn->ib->w[i].state == ibsdsent){
		if (IBDEBUGLEVEL)
		    crWarning("socket(%d) write buf(%d) id(%d) no state change from ibsdsent to ibsdsent on wcompletion %f",
			conn->tcp_socket,i,id,usecs());
		/*we got an ack before getting here, so it was moved up*/
		/*nothing to do*/
	    } else {
		if (IBDEBUGLEVEL)
		    crWarning("socket(%d) write buf(%d) id(%d) state change from ibsdposted to ibsdsent on wcompletion %f",
			conn->tcp_socket,i,id,usecs());
		IBASSERT(conn->ib->w[i].state == ibsdposted);
		conn->ib->w[i].state = ibsdsent;
	    }
	    return;
	}
    }
    crError("socket %d id %d not found in processwcompletion", conn->tcp_socket, id);
}

static void processrcompletion(CRConnection *conn,int id, int immdata, int len){
    int i,wi;
    VAPI_ret_t ret;
    if (len == ACKSZ)
	if (IBDEBUGLEVEL)
	    crWarning("GOT AN ACK trans(%d) id(%d), WHAT WILL WE DO WITH IT",immdata,id);
    if (immdata & 0x10000){ /* it is an ack*/
	if (IBDEBUGLEVEL)
	    crWarning("confirmed ACK");
	for (i = 0; i < K; i++){
	    if (IBDEBUGLEVEL)
		crWarning("testing to see if buffer(%d) with id(%d) was the read in question",i,conn->ib->r[i].id);
	    if (id == conn->ib->r[i].id){
		if (IBDEBUGLEVEL)
		    crWarning("socket(%d) read buf(%d) state change from ibrdposted to ibardready on rcompletion %f",
			conn->tcp_socket,i,usecs());
		IBASSERT(conn->ib->r[i].state == ibrdposted);
		conn->ib->r[i].state = ibardready;
		conn->ib->r[i].len = len; /*shouldn't matter*/
		conn->ib->r[i].transid = immdata;
		conn->ib->r[i].pos = 0;
		dumptrans(conn,i,&(conn->ib->r[i]),"ack read");
		/*find out who was just acknowledged, and move their state from
		 * sdsent to sdacked, and reopen a read request*/
		for (wi = 0; wi < K; wi++){
		    if (((conn->ib->w[wi].transid|0x10000) == immdata) &&
		        (conn->ib->w[wi].transid < 0x10000)){
			/*We want to make sure it is an ack to a real send, not
			coincidentally an ack with the same transid. Transids are unique
			in each direction, but not across both directions. Which perhaps
			should be fixed: maybe the transid should contain info about
			which half it is from*/
			if (IBDEBUGLEVEL)
			    crWarning("state %s wi %d immdata(%d)",istate(conn->ib->w[wi].state),wi,immdata);
			if (conn->ib->w[wi].state == ibsdposted){
			    if (IBDEBUGLEVEL)
				crWarning("Race: state is ibsdposted not ibsdsent; sleep 100 usec and checking");
			    sleepusec(100);
			    checksends(conn);
			    if (conn->ib->w[wi].state == ibsdposted){
				crError("Socket(%d) write buf(%d) in processrcompletion, "
					"we appear to have an ack for a send, but the send "
					"was not viewed as complete even after waiting 100 "
					"usec. Report to Graphstream.",conn->tcp_socket,wi);
			    }
			    IBASSERT(conn->ib->w[wi].state == ibsdsent);
			    /*crWarning("socket(%d) write buf(%d) state change from ibsdposted to ibsdent on rcompletion %f",
				    conn->tcp_socket,wi,usecs());
			    conn->ib->w[wi].state = ibsdsent;*/
			}
			IBASSERT(conn->ib->w[wi].state == ibsdsent);
			if (IBDEBUGLEVEL)
			    crWarning("socket(%d) write buf(%d) state change from ibsdsent to ibsdacked then ibnull on rcompletion %f",
				conn->tcp_socket,wi,usecs());
			conn->ib->w[wi].state = ibsdacked;
			conn->ib->w[wi].state = ibnull;
			conn->ib->w[wi].transid = 0;
			conn->ib->w[wi].id = 0;

			if (IBDEBUGLEVEL)
			    crWarning("socket(%d) read buf(%d) state change from ibardready to ibnull then ibrdposted on rcompletion %f",
				conn->tcp_socket,i,usecs());
			/*and now for the remainder of the work, and return*/
			conn->ib->r[i].state = ibnull;
			conn->ib->r[i].transid = 0;
			conn->ib->r[i].id = 0;
			/*and now, get it back to read posted*/
			conn->ib->r[i].rr.id = conn->ib->r[i].id = getid(conn);
			ret = VAPI_post_rr(conn->ib->hndl,conn->ib->qphndl,&(conn->ib->r[i].rr));
			crDebug("Posting read request on ack socket(%d)",(int)conn->tcp_socket);
			if (ret != VAPI_OK)
			    crError("processrcompletion ra VAPI_post_rr: %s",VAPI_strerror(ret));
			conn->ib->r[i].state = ibrdposted;
			return;
		    }
		}
		crError("Socket %d recieved an acknowledgement(%d), but for what?",
		conn->tcp_socket, immdata);
	    }
	}
	crError("socket %d id %d acktransid %d not found in processrcompletion", conn->tcp_socket, id, immdata);
    }
    for (i = 0; i < K; i++){
	if (id == conn->ib->r[i].id){
	    IBASSERT(conn->ib->r[i].state == ibrdposted);
	    IBASSERT(conn->ib->r[i].next == -1);
	    conn->ib->r[i].len = len;
	    conn->ib->r[i].pos = 0;
	    conn->ib->r[i].transid = immdata;
	    dumptrans(conn,i,&(conn->ib->r[i]),"normal data read");
	    if (conn->ib->curread == -1){
		conn->ib->curread = i; 
	    } else {
		int k = conn->ib->curread;
		int emergencypull = 0;
		if (0)dumpstate(conn,"before adding to curread");
		IBASSERT(conn->ib->r[k].state == ibrdready);
		while (conn->ib->r[k].next != -1) {
		    if (IBDEBUGLEVEL)
			crWarning("Socket %d adding buffer %d to list (%d) next(%d)",
			       conn->tcp_socket, i, k, conn->ib->r[k].next);
		    IBASSERT(conn->ib->r[k].state == ibrdready);
		    IBASSERT(k != conn->ib->r[k].next);
		    k = conn->ib->r[k].next;
		    IBASSERT(k >= -1 && k < K);
		    if (emergencypull++ > K){
			/*if you ever get here, it means that somehow
			 * a loop was formed among the available (filled)
			 * read buffers. Really. Unless the code has changed,
			 * you shouldn't ever get here*/
			crError("Socket %d: infinite loop of read buffers in processrcompletion, impossible",conn->tcp_socket);
		    }
		}
		IBASSERT(conn->ib->r[k].next == -1);
		IBASSERT(k != i);
		if (IBDEBUGLEVEL)
		    crWarning("SETTING socket(%d) readbuf(%d) next to %d",
			conn->tcp_socket,k,i);
		conn->ib->r[k].next = i;
	    }
	    if (IBDEBUGLEVEL)
		crWarning("socket(%d) read buf(%d) state change from ibrdposted to ibrdready on rcompletion %f",
				conn->tcp_socket,i,usecs());
	    conn->ib->r[i].state = ibrdready;
	    IBASSERT(conn->ib->curread != -1);
	    IBASSERT(conn->ib->r[i].next == -1);
	    /*next step will be ibnull, when the data has been read and then
	     * an ack sent*/
	    return;
	}
    }
    crError("socket %d id %d transid %d not found in processrcompletion", conn->tcp_socket, id, immdata);
}
#if 0
static void mempage(CRConnection *conn){
    /*DELETE THIS*/
    int i,j;
    char c=0;
    /*much as everything SHOULD be locked in, why not just read a bit from
     * everything*/
    for (i = 0; i < K; i++){
	for (j = 0; j < BUFMAX; j+=511){
	    c += conn->ib->r[i].buf[j];
	    c += conn->ib->w[i].buf[j];
	}
    }
    if ((c & 0x23) == 0x13){/*short out optimizer*/
	if (IBDEBUGLEVEL)
	    crWarning("shouldn't get here often but doesn't matter");
    }
}
#endif
static void checksends(CRConnection *conn){
    VAPI_ret_t ret;
    VAPI_wc_desc_t cd;
    /*TODO: CONFIRM WORKS WITHOUT FOLLOWING LINE*/
    /*mempage(conn);*/
    while ((ret = EVAPI_peek_cq(conn->ib->hndl,conn->ib->scqhndl,1)) == VAPI_OK){
	if (IBDEBUGLEVEL)
	    crWarning("WOW. DIDN'T USE TO GET SEND QUEUE COMPLETIONS");
	ret = VAPI_poll_cq(conn->ib->hndl,conn->ib->scqhndl,&cd);
	if (ret != VAPI_OK)
	    crError("checksends VAPI_poll_cq: %s",VAPI_strerror(ret));
	if (cd.status != VAPI_SUCCESS)
	    crError("checksends VAPI_poll_cq not SUCCESS: %d %d hndl(%d)",(int)cd.status, (int)cd.byte_len, (int)conn->ib->scqhndl);
	crDebug("write id %d opcode %d byte_len %d pkey_ix %d status %d",
		(int)cd.id,(int)cd.opcode,(int)cd.byte_len,(int)cd.pkey_ix,(int)cd.status);
	processwcompletion(conn,(int)cd.id);
    }
}

static void checkreads(CRConnection *conn){
    VAPI_ret_t ret;
    VAPI_wc_desc_t cd;
    /*TODO CONFIRM WORKS WITHOUT FOLLOWING LINE*/
    /*mempage(conn);*/
    while ((ret = EVAPI_peek_cq(conn->ib->hndl,conn->ib->rcqhndl,1)) == VAPI_OK){
	ret = VAPI_poll_cq(conn->ib->hndl,conn->ib->rcqhndl,&cd);
	if (ret != VAPI_OK)
	    crError("checkreads VAPI_poll_cq: %s",VAPI_strerror(ret));
	if (cd.status != VAPI_SUCCESS)
	    crError("checkreads VAPI_poll_cq not SUCCESS: %d %d hndl(%d)",(int)cd.status, (int)cd.byte_len, (int)conn->ib->scqhndl);
	crDebug("read id %d opcode %d byte_len %d pkey_ix %d status %d",
	    (int)cd.id,(int)cd.opcode,(int)cd.byte_len,(int)cd.pkey_ix,(int)cd.status);
	IBASSERT(cd.imm_data_valid);
	processrcompletion(conn,(int)cd.id,(int)cd.imm_data,(int)cd.byte_len);
    }
    /*Check to see if there are any readqueue entries.*/
    /*These could be acks, which will move a send buffer from ibsdsent to 
     * ibsdsent to ibsdacked and then ibsdnull*/
    /*an ack comes with a number, move that number the appropriate state*/
}

static void ibwriteexact(CRConnection *conn, void *buf, unsigned int len)
{
    /*check completion queues, clear out any*/
    int i;
    int basesleep = 100; /*usec*/
    struct bufib *x;
    VAPI_ret_t ret;
    double inittime = usecs();
    if (IBDEBUGLEVEL)
	crWarning("Top level write requested sock(%d), %d bytes, time %f csum(%d) csum4(%d)",conn->tcp_socket,len,usecs(),
	    csum((char*)buf,len),
	    csum(((char*)buf)+4,len-4));
    while (len > BUFMAX){
	if (IBDEBUGLEVEL)
	    crWarning("IB%d: Given %d bytes to transmit, bufmax %d, therefore recursing",conn->tcp_socket,len,BUFMAX);
	ibwriteexact(conn,buf,BUFMAX);
	buf = (void*)((char*)buf + BUFMAX);
	len -= BUFMAX;
    }
    while ((i = getwritebuf(conn)) == -1){
	checksends(conn);
	checkreads(conn);
	if ((i = getwritebuf(conn)) == -1){
	    if (IBDEBUGLEVEL)
		crWarning("IB%d: No transmit buffers available. If sporadic, K may need to be increased. Else error.",conn->tcp_socket);
	    if (basesleep == 100)
		dumpstate(conn,"no transmit buffers");
	    sleepusec(basesleep); basesleep *= 1.5;
	}
    }
    IBASSERT(i >=0 && i < K);
    x = &(conn->ib->w[i]);
    IBASSERT(x->state == ibnull);
    memcpy(x->buf,buf,len);
    x->len = len;
    (int)x->sr.id = x->id = getid(conn);
    x->sr.imm_data = x->transid = gettransid(conn); 
    x->sr.sg_lst_p->len = x->len; 
    dumptrans(conn,i,x,"normal data post");
    ret = VAPI_post_sr(conn->ib->hndl,conn->ib->qphndl,&x->sr);
    conn->ib->sendsposted++;
    if (ret != VAPI_OK)
	crError("postsr VAPI_post_sr: %s",VAPI_strerror(ret));
    if (IBDEBUGLEVEL)
	crWarning("socket(%d) write buf(%d) state change from ibnull to ibsdposted on ibwriteexact %f",
				conn->tcp_socket,i,usecs());
    x->state = ibsdposted;
    if (0 || IBDEBUGLEVEL) {
	double timenow = usecs();
	crWarning("Top level write posted sock(%d), %d bytes, time %f, timeused %f",conn->tcp_socket,len,timenow, timenow-inittime);
    }
    return;
}

static void sendack(CRConnection *conn, struct bufib *x)
{
    /*we are finished with a buffer. Grab the acknowledge buffer,
     * insert the transaction id, send it, move our state to ibrdacked and
     * then ibnull*/
    VAPI_ret_t ret;
    int i;
    int z;
    int basesleep = 100; /*usec*/
    IBASSERT(x->state == ibrdready);
    checksends(conn);
    checkreads(conn);
    while ((i = getwritebuf(conn)) == -1){
	checksends(conn);
	checkreads(conn);
	if ((i = getwritebuf(conn)) == -1){
	    crWarning("IB%d: No transmit buffers available for ack. If sporadic, K may need to be increased. Else error.",conn->tcp_socket);
	    if (0)dumpstate(conn,"no transmit buffers");
	    sleepusec(basesleep); basesleep *= 1.5;
	}
    }
    IBASSERT(i >= 0 && i < K);
    IBASSERT(conn->ib->w[i].state == ibnull);
    if (IBDEBUGLEVEL)
	crWarning("socket(%d) write buf(%d) state change from ibnull to ibasdposted on sendack %f",
				conn->tcp_socket,i,usecs());
    conn->ib->w[i].state = ibasdposted;
    (int)conn->ib->w[i].sr.id = conn->ib->w[i].id = getid(conn);
    conn->ib->w[i].sr.imm_data = conn->ib->w[i].transid = x->transid|0x10000; 
    conn->ib->w[i].sr.sg_lst_p->len = conn->ib->w[i].len = ACKSZ; 
    for (z = 0 ; z < ACKSZ; z++) {conn->ib->w[i].buf[z] = 0xff;}
    dumptrans(conn,i,&(conn->ib->w[i]),"normal ack post");
    if (0 && IBDEBUGLEVEL)
	dumpsr("DUMPING ACK SR",conn->ib->w[i].sr);
    ret = VAPI_post_sr(conn->ib->hndl,conn->ib->qphndl,&(conn->ib->w[i].sr));
    if (ret != VAPI_OK)
	crError("sendack VAPI_post_sr: %s",VAPI_strerror(ret));
    if (IBDEBUGLEVEL)
    crWarning("socket(%d) read buf(%d)(==%d) state change from ibrdready to ibrdacked to ibnull to ibrdposted on sendack %f",
				conn->tcp_socket,x-conn->ib->r,conn->ib->curread,usecs());
    x->state = ibrdacked; 
    x->id = 0;
    x->transid = 0;
    IBASSERT(conn->ib->curread != -1);
    IBASSERT(&(conn->ib->r[conn->ib->curread]) == x);
    conn->ib->curread = x->next;
    x->next = -1;
    x->state = ibnull;
    x->transid = 0;
    x->pos = 0;
    x->len = 0;
    x->rr.id = x->id = getid(conn);
    ret = VAPI_post_rr(conn->ib->hndl,conn->ib->qphndl,&x->rr);
    conn->ib->readsposted++;
    if (ret != VAPI_OK)
	crError("initibmem r VAPI_post_rr: %s",VAPI_strerror(ret));
    CRASSERT((conn->ib->curread == -1) ||
	     (conn->ib->r[conn->ib->curread].state == ibrdready));
    x->state = ibrdposted;
}

static int findcurrent(CRConnection *conn){
    int basesleep = 100;
    while (conn->ib->curread == -1){
	checksends(conn);
	checkreads(conn);
	if (conn->ib->curread == -1){
	    sleepusec(basesleep); basesleep *= 1.5;
	    if (IBDEBUGLEVEL)
		crWarning("Socket %d don't think we should get here often; waiting for read",conn->tcp_socket);
	}
    }
    return conn->ib->curread;
}

static void pureibreadexact(CRConnection *conn, void *buf, unsigned int len)
{
    /*this is a higher level routine. Normally, being chromium, it will 
     * ask for amounts that are a part of a single buffer. But we don't know
     * that. So find a buffer which is read ready and pos > 0, the active one.
     * Or else find the one with the lowest transaction id which is ready,
     * and begin serving that. */
    /*copy from buffer and read as needed */
    /*setupmb(conn);*/
    /*shamelessly wait if there is nothing there*/
    /*at the end of each buffer, send an ack.*/
    struct bufib *x;
    int i;
    int offset = 0;
    unsigned int origlen = len;
    double inittime = usecs();
    if (IBDEBUGLEVEL)
	crWarning("Top level read requested, %d bytes, time %f",len,usecs());
    while (len > 0){
	int remaining;
	checksends(conn);
	checkreads(conn);
	i = findcurrent(conn);
	x = &(conn->ib->r[i]);
	IBASSERT(x->state == ibrdready);
	remaining = x->len - x->pos;
	if (remaining <= 0){
	    crWarning("Prepare to crash: socket(%d) buf(%d) len(%d) pos(%d)",
		    conn->tcp_socket,i,x->len,x->pos);
	}
	IBASSERT(remaining > 0);
	if (remaining > len){
	    memcpy((char*)buf+offset,x->buf+x->pos,len);
	    x->pos += len;
	    offset += remaining;
	    /*no state changes needed*/
	    if (0 || IBDEBUGLEVEL) {
		double timenow = usecs();
		crWarning("Top level read satisfied by partial read, %d bytes, time %f timeused %f csum(%d) csum4(%d)",origlen,timenow,timenow-inittime,
		csum((char*)buf,origlen),
		csum(((char*)buf)+4,origlen-4));
	    }
	    return;
	}
	IBASSERT(len >= remaining);
	memcpy((char*)buf+offset,x->buf+x->pos,remaining);
	x->pos += remaining;
	offset += remaining;
	IBASSERT(x->pos  == x->len);
	len -= remaining;
	/*send an acknowledgement, and change state*/
	sendack(conn,x);
    }
    if (IBDEBUGLEVEL) {
	double timenow = usecs();
	crWarning("Top level read satisfied by full read, %d bytes, time %f elapsed %f csum(%d) csum4(%d)",origlen,timenow,timenow-inittime,
	    csum((char*)buf,origlen),
	    csum(((char*)buf)+4,origlen-4));
    }
    return;
}

int
__ib_read_exact( CRSocket sock, CRConnection *conn, void *buf, unsigned int len )
{
	unsigned int iblen = len;
	/* 
	 * Shouldn't write to a non-existent socket, ie when 
	 * crIBDoDisconnect has removed it from the pool
	 */
	if ( sock <= 0 )
		return 1;

	crDebug("ib_read_exact socket %d buf %x len %d",conn->tcp_socket, (int)buf, len);
	pureibreadexact(conn,buf,iblen);

	return 1;
}

void
crIBReadExact( CRConnection *conn, void *buf, unsigned int len )
{
	if ( __ib_read_exact( conn->tcp_socket, conn, buf, len ) <= 0 )
	{
		__ib_dead_connection( conn );
	}
}

int
__ib_write_exact(CRConnection *conn, CRSocket sock, void *buf, unsigned int len )
{
	unsigned int iblen = len;

	/* 
	 * Shouldn't write to a non-existent socket, ie when 
	 * crIBDoDisconnect has removed it from the pool
	 */
	if ( sock <= 0 )
		return 1;
	crDebug("ib_write_exact socket %d buf %x len %d",conn->tcp_socket, (int)buf, len);
	ibwriteexact(conn,buf,iblen);
	return 1;
}

void
crIBWriteExact( CRConnection *conn, void *buf, unsigned int len )
{
	if ( __ib_write_exact(conn, conn->tcp_socket, buf, len) <= 0 )
	{
		__ib_dead_connection( conn );
	}

}

/* 
 * Make sockets do what we want: 
 * 
 * 1) Change the size of the send/receive buffers to 64K 
 * 2) Turn off Nagle's algorithm */

static void
__crSpankSocket( CRSocket sock )
{
	/* why do we do 1) ? things work much better for me to push the
	 * the buffer size way up -- karl
	 */
#ifdef LINUX
	int sndbuf = 1*1024*1024;
#else
	int sndbuf = 64*1024;
#endif	

	int rcvbuf = sndbuf;
	int so_reuseaddr = 1;
	int tcp_nodelay = 1;

	if ( setsockopt( sock, SOL_SOCKET, SO_SNDBUF, 
				(char *) &sndbuf, sizeof(sndbuf) ) )
	{
		int err = crIBErrno( );
		crWarning( "setsockopt( SO_SNDBUF=%d ) : %s",
				sndbuf, crIBErrorString( err ) );
	}

	if ( setsockopt( sock, SOL_SOCKET, SO_RCVBUF,
				(char *) &rcvbuf, sizeof(rcvbuf) ) )
	{
		int err = crIBErrno( );
		crWarning( "setsockopt( SO_RCVBUF=%d ) : %s",
				rcvbuf, crIBErrorString( err ) );
	}

	
	if ( setsockopt( sock, SOL_SOCKET, SO_REUSEADDR,
				(char *) &so_reuseaddr, sizeof(so_reuseaddr) ) )
	{
		int err = crIBErrno( );
		crWarning( "setsockopt( SO_REUSEADDR=%d ) : %s",
				so_reuseaddr, crIBErrorString( err ) );
	}

	if ( setsockopt( sock, IPPROTO_TCP, TCP_NODELAY,
				(char *) &tcp_nodelay, sizeof(tcp_nodelay) ) )
	{
		int err = crIBErrno( );
		crWarning( "setsockopt( TCP_NODELAY=%d )"
				" : %s", tcp_nodelay, crIBErrorString( err ) );
	}
}


void
crIBAccept( CRConnection *conn, char *hostname, unsigned short port )
{
	int err;
	socklen_t		addr_length;
#ifndef ADDRINFO
	struct sockaddr_in	servaddr;
	struct sockaddr		addr;
	struct hostent		*host;
	struct in_addr		sin_addr;
#else
	struct sockaddr_storage	addr;
	char			host[NI_MAXHOST];
#endif
	if (port != last_port)
	{
		/* with the new OOB stuff, we can have multiple ports being 
		 * accepted on, so we need to redo the server socket every time.
		 */
#ifndef ADDRINFO
		cr_ib.server_sock = socket( AF_INET, SOCK_STREAM, 0 );
		if ( cr_ib.server_sock == -1 )
		{
			err = crIBErrno( );
			crError( "Couldn't create socket: %s", crIBErrorString( err ) );
		}
		__crSpankSocket( cr_ib.server_sock );

		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = INADDR_ANY;
		servaddr.sin_port = htons( port );

		if ( bind( cr_ib.server_sock, (struct sockaddr *) &servaddr, sizeof(servaddr) ) )
		{
			err = crIBErrno( );
			crError( "Couldn't bind to socket (port=%d): %s", port, crIBErrorString( err ) );
		}
		last_port = port;

		if ( listen( cr_ib.server_sock, 100 /* max pending connections */ ) )
		{
			err = crIBErrno( );
			crError( "Couldn't listen on socket: %s", crIBErrorString( err ) );
		}
#else
		char port_s[NI_MAXSERV];
		struct addrinfo *res,*cur;
		struct addrinfo hints;

		sprintf(port_s, "%u", (short unsigned) port);

		crMemset(&hints, 0, sizeof(hints));
		hints.ai_flags = AI_PASSIVE;
		hints.ai_family = PF;
		hints.ai_socktype = SOCK_STREAM;

		err = getaddrinfo( NULL, port_s, &hints, &res );
		if ( err )
			crError( "Couldn't find local TCP port %s: %s", port_s, gai_strerror(err) );

		for (cur=res;cur;cur=cur->ai_next)
		{
			cr_ib.server_sock = socket( cur->ai_family, cur->ai_socktype, cur->ai_protocol );
			if ( cr_ib.server_sock == -1 )
			{
				err = crIBErrno( );
				if (err != EAFNOSUPPORT)
					crWarning( "Couldn't create socket of family %i: %s, trying another", cur->ai_family, crIBErrorString( err ) );
				continue;
			}
			__crSpankSocket( cr_ib.server_sock );

			if ( bind( cr_ib.server_sock, cur->ai_addr, cur->ai_addrlen ) )
			{
				err = crIBErrno( );
				crWarning( "Couldn't bind to socket (port=%d): %s", port, crIBErrorString( err ) );
				crIBCloseSocket( cr_ib.server_sock );
				continue;
			}
			last_port = port;

			if ( listen( cr_ib.server_sock, 100 /* max pending connections */ ) )
			{
				err = crIBErrno( );
				crWarning( "Couldn't listen on socket: %s", crIBErrorString( err ) );
				crIBCloseSocket( cr_ib.server_sock );
				continue;
			}
			break;
		}
		freeaddrinfo(res);
		if (!cur)
			crError( "Couldn't find local TCP port %s", port_s);
#endif
	}

	if (conn->broker)
	{
		CRConnection *mother;
		char response[8096];
		char my_hostname[256];
		char *temp;
		mother = __copy_of_crMothershipConnect( );

		if (!hostname)
		{
			if ( crIBGetHostname( my_hostname, sizeof( my_hostname ) ) )
			{
				crError( "Couldn't determine my own hostname in crIBAccept!" );
			}
		}
		else
			crStrcpy(my_hostname, hostname);

		temp = crStrchr( my_hostname, '.' );
		if (temp) *temp='\0';

		if (!__copy_of_crMothershipSendString( mother, response, "acceptrequest ib %s %d %d", my_hostname, conn->port, conn->endianness ) )
			/*CHECK THIS*/
		{
			crError( "Mothership didn't like my accept request request" );
		}

		__copy_of_crMothershipDisconnect( mother );

		sscanf( response, "%u", &(conn->id) );
	}

	addr_length =	sizeof( addr );
	conn->tcp_socket = accept( cr_ib.server_sock, (struct sockaddr *) &addr, &addr_length );
	if (conn->tcp_socket == -1)
	{
		err = crIBErrno( );
		crError( "Couldn't accept client: %s", crIBErrorString( err ) );
	}

#ifndef ADDRINFO
	sin_addr = ((struct sockaddr_in *) &addr)->sin_addr;
	host = gethostbyaddr( (char *) &sin_addr, sizeof( sin_addr), AF_INET );
	if (host == NULL )
	{
		char *temp = inet_ntoa( sin_addr );
		conn->hostname = crStrdup( temp );
	}
#else
	err = getnameinfo ( (struct sockaddr *) &addr, addr_length,
			host, sizeof( host),
			NULL, 0, NI_NAMEREQD);
	if ( err )
	{
		err = getnameinfo ( (struct sockaddr *) &addr, addr_length,
			host, sizeof( host),
			NULL, 0, NI_NUMERICHOST);
		if ( err )	/* shouldn't ever happen */
			conn->hostname = "unknown ?!";
		else
			conn->hostname = crStrdup( host );
	}
#endif
	else
	{
		char *temp;
#ifndef ADDRINFO
		conn->hostname = crStrdup( host->h_name );
#else
		conn->hostname = crStrdup( host );
#endif

		temp = conn->hostname;
		while (*temp && *temp != '.' )
			temp++;
		*temp = '\0';
	}

#ifdef RECV_BAIL_OUT 
	err = sizeof(unsigned int);
	if ( getsockopt( conn->tcp_socket, SOL_SOCKET, SO_RCVBUF,
			(char *) &conn->krecv_buf_size, &err ) )
	{
		conn->krecv_buf_size = 0;	
	}
#endif
	crDebug( "Accepted connection from \"%s\".", conn->hostname );
	if (ibconnect(conn)){
		crWarning( "IB connection error.");
		cr_ib.conns[conn->index] = NULL; /* remove from table */
		return;
	};
	getremoteinfo(conn);
	sendlocalinfo(conn);
	if (ibconnect2(conn)){
		crWarning( "IB connection error, phase 2.");
		cr_ib.conns[conn->index] = NULL; /* remove from table */
		return;
	};
	getremoteinfo2(conn); /*now that ibconnect2 is done, and read requests are waiting, synchronize*/
	sendlocalinfo2(conn);
	crWarning("Initializing IB(%d)",conn->tcp_socket);
}

void *
crIBAlloc( CRConnection *conn )
{
	CRIBBuffer *buf;

#ifdef CHROMIUM_THREADSAFE
	crLockMutex(&cr_ib.mutex);
#endif

	buf = (CRIBBuffer *) crBufferPoolPop( cr_ib.bufpool, conn->buffer_size );

	if ( buf == NULL )
	{
		crDebug( "Buffer pool %p was empty, so I allocated %d bytes.\n\tI did so from the buffer: %p", 
						 cr_ib.bufpool,
			(unsigned int)sizeof(CRIBBuffer) + conn->buffer_size, &cr_ib.bufpool );
		crDebug("sizeof(CRIBBuffer): %d", (unsigned int)sizeof(CRIBBuffer));
		crDebug("sizeof(conn->buffer_size): %d", conn->buffer_size);
		buf = (CRIBBuffer *) 
			crAlloc( sizeof(CRIBBuffer) + conn->buffer_size );
		buf->magic = CR_IB_BUFFER_MAGIC;
		buf->kind  = CRIBMemory;
		buf->pad   = 0;
		buf->allocated = conn->buffer_size;
		buf->ibmagic = 1;
	}
	else
	{
	   crDebug( "I asked for a message of size %d, and I got one from the pool, which was of size %d", conn->buffer_size, buf->allocated );
	}

#ifdef CHROMIUM_THREADSAFE
	crUnlockMutex(&cr_ib.mutex);
#endif

	return (void *)( buf + 1 );
}

static void
crIBSingleRecv( CRConnection *conn, void *buf, unsigned int len )
{
	crIBReadExact( conn, buf, len );
}

static void
crIBSend( CRConnection *conn, void **bufp,
				 void *start, unsigned int len )
{
	CRIBBuffer *ib_buffer;
	unsigned int      *lenp;

	if ( !conn || conn->type == CR_NO_CONNECTION )
		return;

	if ( bufp == NULL )
	{
		/* we are doing synchronous sends from user memory, so no need
		 * to get fancy.  Simply write the length & the payload and
		 * return. */
		int sendable_len=len;/* can't swap len then use it for length*/
		if (conn->swap)
		{
			sendable_len = SWAP32(len);
		}
		crIBWriteExact( conn, &sendable_len, sizeof(len) );
		if ( !conn || conn->type == CR_NO_CONNECTION) return;
		crIBWriteExact( conn, start, len );
		return;
	}

	ib_buffer = (CRIBBuffer *)(*bufp) - 1;

	IBASSERT( ib_buffer->magic == CR_IB_BUFFER_MAGIC );

	/* All of the buffers passed to the send function were allocated
	 * with crIBAlloc(), which includes a header with a 4 byte
	 * length field, to insure that we always have a place to write
	 * the length field, even when start == *bufp. */
	lenp = (unsigned int *) start - 1;
	if (conn->swap)
	{
		*lenp = SWAP32(len);
	}
	else
	{
		*lenp = len;
	}

	if ( __ib_write_exact(conn, conn->tcp_socket, lenp, len + sizeof(int) ) < 0 )
	{
		__ib_dead_connection( conn );
	}

	/* reclaim this pointer for reuse and try to keep the client from
		 accidentally reusing it directly */
#ifdef CHROMIUM_THREADSAFE
	crLockMutex(&cr_ib.mutex);
#endif
	crBufferPoolPush( cr_ib.bufpool, ib_buffer, conn->buffer_size );
#ifdef CHROMIUM_THREADSAFE
	crUnlockMutex(&cr_ib.mutex);
#endif
}

void
__ib_dead_connection( CRConnection *conn )
{
	crDebug( "Dead connection (sock=%d, host=%s), removing from pool",
  				   conn->tcp_socket, conn->hostname );
  
	/* remove from connection pool */
	crIBDoDisconnect( conn );
}

int
__crIBSelect( int n, fd_set *readfds, int sec, int usec )
{
	for ( ; ; ) 
	{ 
		int err, num_ready;

		if (sec || usec)
		{
			/* We re-init everytime for Linux, as it corrupts
			 * the timeout structure, but other OS's
			 * don't have a problem with it.
			 */
			struct timeval timeout;
			timeout.tv_sec = sec;
			timeout.tv_usec = usec;
			num_ready = select( n, readfds, NULL, NULL, &timeout );
		} 
		else
			num_ready = select( n, readfds, NULL, NULL, NULL );


		if ( num_ready >= 0 )
		{
			return num_ready;
		}

		err = crIBErrno( );
		if ( err == EINTR )
		{
			crWarning( "select interruped by an unblocked signal, trying again" );
		}
		else
		{
			crError( "select failed: %s", crIBErrorString( err ) );
		}
	}
}

void crIBFree( CRConnection *conn, void *buf )
{
	CRIBBuffer *ib_buffer = (CRIBBuffer *) buf - 1;

	IBASSERT( ib_buffer->magic == CR_IB_BUFFER_MAGIC );
	conn->recv_credits += ib_buffer->len;

	switch ( ib_buffer->kind )
	{
		case CRIBMemory:
#ifdef CHROMIUM_THREADSAFE
			crLockMutex(&cr_ib.mutex);
#endif
			crBufferPoolPush( cr_ib.bufpool, ib_buffer, conn->buffer_size );
#ifdef CHROMIUM_THREADSAFE
			crUnlockMutex(&cr_ib.mutex);
#endif
			break;

		case CRIBMemoryBig:
			crFree( ib_buffer );
			break;

		default:
			crError( "Weird buffer kind trying to free in crIBFree: %d", ib_buffer->kind );
	}
}

/* returns the amt of pending data which was handled */ 
static int
crIBUserbufRecv(CRConnection *conn, CRMessage *msg)
{
	unsigned int buf[2];
	int len;

	switch (msg->header.type)
	{
		case CR_MESSAGE_GATHER:
			/* grab the offset and the length */
			len = 2*sizeof(unsigned long);
			if (__ib_read_exact(conn->tcp_socket, conn, buf, len) <= 0)
			{
				__ib_dead_connection( conn );
			}
			msg->gather.offset = buf[0];
			msg->gather.len = buf[1];

			/* read the rest into the userbuf */
			if (buf[0]+buf[1] > (unsigned int)conn->userbuf_len)
			{
				crDebug("userbuf for Gather Message is too small!");
				return len;
			}

			if (__ib_read_exact(conn->tcp_socket, conn, conn->userbuf+buf[0], buf[1]) <= 0)
			{
				__ib_dead_connection( conn );
			}
			return len+buf[1];

		default:
			return 0;
	}
}

static int checkforibreads(int howmany, int microsec){
    /*Sort of like a select. Check to see if we already picked something
     * up, then check all ports for anything to read,
     * if nothing, wait for microsec and do this again.
     * Do this howmany times.*/
    int ret = 0;
    while (ret == 0 && howmany-- >= 0){
	int i;
	int foundib = 0;
	for ( i = 0; i < cr_ib.num_conns; i++ )
	{
	    CRConnection  *conn = cr_ib.conns[i];
	    if ((conn==NULL) || (conn->type != CR_IB)) continue;
	    foundib = 1;
	    checkreads(conn);
	    checksends(conn); /*may as well get rid of any send completions etc.*/
	    if (conn->ib->curread != -1)
		ret++;
	    /*if ((conn->ib->curread != -1) ||completeread(conn))
		ret++;*/
	}
	if (!foundib)
	    return 0;
	if (ret) return ret;
	{
           sleepusec(microsec);
	    /*struct timespec req, rem;
	    req.tv_sec = 0;
	    req.tv_nsec = microsec * 1000;
	    (void)nanosleep(&req,&rem);*/
	}
    }
    return ret;
}

int
crIBRecv( void )
{
    /* go through all connections.  If there are no connections, wait
     * for one.  otherwise, if anything shows up in 500 microseconds
     * on any connection, process it.  if not, return 0.  if so, do
     * the reads, dispatch it, and return 1.  return 1 generally means
     * we'll be right back here.*/
	CRMessage *msg;
	CRMessageType cached_type;
	int    num_ready;
	int i;
	/* ensure we don't get caught with a new thread connecting */
	int num_conns = cr_ib.num_conns;
	if (num_conns <= 0) return 0;
#ifdef CHROMIUM_THREADSAFE
	crLockMutex(&cr_ib.recvmutex);
#endif
	num_ready = checkforibreads(1,500); /*check, then once more in 500usec */
	if ( num_ready == 0 ) {
#ifdef CHROMIUM_THREADSAFE
		crUnlockMutex(&cr_ib.recvmutex);
#endif
		if (IBDEBUGLEVEL)
		    crWarning("IBRecv found nothing socket %f",usecs());
		return 0;
	}
	if (IBDEBUGLEVEL)
	    crWarning("IBRecv found %d ready socket %f",num_ready,usecs());
	for ( i = 0; i < num_conns; i++ )
	{
		CRIBBuffer *ib_buffer;
		unsigned int   len, total, handled, leftover;
		CRConnection  *conn = cr_ib.conns[i];
		CRSocket       sock = conn->tcp_socket;
		if ( !conn || conn->type == CR_NO_CONNECTION ) continue;
		if (conn->ib->curread == -1) continue;
		/*wouldn't be here if there wasn't anything*/
		/*don't want to use completeread, could be an ack
		 * which showed up*/
		/*if (conn->ib->curread == -1 && !completeread(conn))
		    continue;*/
		/* this reads the length of the message */
		if ( __ib_read_exact( sock, conn, &len, sizeof(len)) <= 0 )
		{
			__ib_dead_connection( conn );
			i--;
			continue;
		}

		if (conn->swap)
		{
			len = SWAP32(len);
		}

		IBASSERT( len > 0 );

		crDebug("===Length of read buffer %d", len);
		if ( len <= conn->buffer_size )
		{
			ib_buffer = (CRIBBuffer *) crIBAlloc( conn ) - 1;
		}
		else
		{
			ib_buffer = (CRIBBuffer *) 
				crAlloc( sizeof(*ib_buffer) + len );

			ib_buffer->magic = CR_IB_BUFFER_MAGIC;
			ib_buffer->kind  = CRIBMemoryBig;
			ib_buffer->pad   = 0;
			ib_buffer->ibmagic = 0;
		}

		ib_buffer->len = len;

		/* if we have set a userbuf, and there is room in it, we probably 
		 * want to stick the message into that, instead of our allocated
		 * buffer.  */
		leftover = 0;
		total = len;
		if ((conn->userbuf != NULL) && (conn->userbuf_len >= sizeof(CRMessageHeader)))
		{
			leftover = len - sizeof(CRMessageHeader);
			total = sizeof(CRMessageHeader);
		}
		if ( __ib_read_exact( sock, conn, ib_buffer + 1, total) <= 0 )
		{
			crWarning( "Bad juju: %d %d on sock %x", ib_buffer->allocated, total, sock );
			crFree( ib_buffer );
			__ib_dead_connection( conn );
			i--;
			continue;
		}
		
		conn->recv_credits -= total;
		conn->total_bytes_recv +=  total;

		msg = (CRMessage *) (ib_buffer + 1);
		cached_type = msg->header.type;
		if (conn->swap)
		{
			msg->header.type = (CRMessageType) SWAP32( msg->header.type );
			msg->header.conn_id = (CRMessageType) SWAP32( msg->header.conn_id );
		}
	
		/* if there is still data pending, it should go into the user buffer */
		if (leftover)
		{
			handled = crIBUserbufRecv(conn, msg);

			crDebug("User buffer");
			/* if there is anything left, plop it into the recv_buffer */
			if (leftover-handled)
			{
				if ( __ib_read_exact( sock, conn, ib_buffer + 1 + total, leftover-handled) <= 0 )
				{
					crWarning( "Bad juju: %d %d", ib_buffer->allocated, leftover-handled);
					crFree( ib_buffer );
					__ib_dead_connection( conn );
					i--;
					continue;
				}
			}
			
			conn->recv_credits -= handled;
			conn->total_bytes_recv +=  handled;
		}

		
		crNetDispatchMessage( cr_ib.recv_list, conn, ib_buffer + 1, len );
#if 0
		crLogRead( len );
#endif


		/* CR_MESSAGE_OPCODES is freed in
		 * crserverlib/server_stream.c 
		 *
		 * OOB messages are the programmer's problem.  -- Humper 12/17/01 */
		if (cached_type != CR_MESSAGE_OPCODES && cached_type != CR_MESSAGE_OOB
		    &&	cached_type != CR_MESSAGE_GATHER) 
		{
			crIBFree( conn, ib_buffer + 1 );
		}
		
	}
#ifdef CHROMIUM_THREADSAFE
	crUnlockMutex(&cr_ib.recvmutex);
#endif
	return 1;
}

static void
crIBHandleNewMessage( CRConnection *conn, CRMessage *msg,
		unsigned int len )
{
	CRIBBuffer *buf = ((CRIBBuffer *) msg) - 1;

	crError( "crIBHandleNewMessage called, not expected to ever be called." );
	/* build a header so we can delete the message later */
	buf->magic = CR_IB_BUFFER_MAGIC;
	buf->kind  = CRIBMemory;
	buf->len   = len;
	buf->pad   = 0;
	buf->ibmagic = 0;

	crNetDispatchMessage( cr_ib.recv_list, conn, msg, len );
}

static void
crIBInstantReclaim( CRConnection *conn, CRMessage *mess )
{
	crError( "crIBInstantReclaim called, not expected to ever be called." );
	crIBFree( conn, mess );
}

void
crIBInit( CRNetReceiveFuncList *rfl, CRNetCloseFuncList *cfl, unsigned int mtu )
{
	(void) mtu;

	cr_ib.recv_list = rfl;
	cr_ib.close_list = cfl;
	crDebug("zap crIBInit rfl(%x) cfl(%x) mtu(%d)",(int)rfl,(int)cfl,(int)mtu);
	if ( cr_ib.initialized )
	{
		return;
	}

	crDebug("Initializing IB\n");

	cr_ib.num_conns = 0;
	cr_ib.conns     = NULL;
	
	cr_ib.server_sock    = -1;

#ifdef CHROMIUM_THREADSAFE
	crInitMutex(&cr_ib.mutex);
	crInitMutex(&cr_ib.recvmutex);
#endif
	cr_ib.bufpool = crBufferPoolInit(16);

	cr_ib.initialized = 1;
}

/* The function that actually connects.  This should only be called by clients 
 * Servers have another way to set up the socket. */

int
crIBDoConnect( CRConnection *conn )
{
	int err;
#ifndef ADDRINFO
	struct sockaddr_in servaddr;
	struct hostent *hp;
	int i;

	conn->tcp_socket = socket( AF_INET, SOCK_STREAM, 0 );
	if ( conn->tcp_socket < 0 )
	{
		int err = crIBErrno( );
		crWarning( "socket error: %s", crIBErrorString( err ) );
		cr_ib.conns[conn->index] = NULL; /* remove from table */
		return 0;
	}

	/* Set up the socket the way *we* want. */
	__crSpankSocket( conn->tcp_socket );

	/* Standard Berkeley sockets mumbo jumbo */
	hp = gethostbyname( conn->hostname );
	if ( !hp )
	{
		crWarning( "Unknown host: \"%s\"", conn->hostname );
		cr_ib.conns[conn->index] = NULL; /* remove from table */
		return 0;
	}

	crMemset( &servaddr, 0, sizeof(servaddr) );
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons( (short) conn->port );

	crMemcpy( (char *) &servaddr.sin_addr, hp->h_addr,
			sizeof(servaddr.sin_addr) );
#else
	char port_s[NI_MAXSERV];
	struct addrinfo *res,*cur;
	struct addrinfo hints;

	sprintf(port_s, "%u", (short unsigned) conn->port);

	crMemset(&hints, 0, sizeof(hints));
	hints.ai_family = PF;
	hints.ai_socktype = SOCK_STREAM;

	err = getaddrinfo( conn->hostname, port_s, &hints, &res);
	if ( err )
	{
		crWarning( "Unknown host: \"%s\": %s", conn->hostname, gai_strerror(err) );
		cr_ib.conns[conn->index] = NULL; /* remove from table */
		return 0;
	}
#endif

	if (conn->broker)
	{
		CRConnection *mother;
		char response[8096];
		int remote_endianness;
		mother = __copy_of_crMothershipConnect( );

		if (!__copy_of_crMothershipSendString( mother, response, "connectrequest ib %s %d %d", conn->hostname, conn->port, conn->endianness) )
		{
#ifdef ADDRINFO
			freeaddrinfo(res);
#endif
			crError( "Mothership didn't like my connect request request" );
		}

		__copy_of_crMothershipDisconnect( mother );

		sscanf( response, "%u %d", &(conn->id), &(remote_endianness) );

		if (conn->endianness != remote_endianness)
		{
			conn->swap = 1;
		}
	}
#ifndef ADDRINFO
	for (i=1;i;)
#else
	for (cur=res;cur;)
#endif
	{
#ifndef ADDRINFO

#ifdef RECV_BAIL_OUT		
		err = sizeof(unsigned int);
		if ( getsockopt( conn->tcp_socket, SOL_SOCKET, SO_RCVBUF,
				(char *) &conn->krecv_buf_size, &err ) )
		{
			conn->krecv_buf_size = 0;	
		}
#endif
		if ( !connect( conn->tcp_socket, (struct sockaddr *) &servaddr,
					sizeof(servaddr) ) ){
			if (ibconnect(conn)){
				crWarning( "IB connection error.");
				cr_ib.conns[conn->index] = NULL; /* remove from table */
				return 0;
			};
		        sendlocalinfo(conn);
			getremoteinfo(conn);
			if (ibconnect2(conn)){
				crWarning( "IB connection error, phase 2.");
				cr_ib.conns[conn->index] = NULL; /* remove from table */
				return 0;
			};
			sendlocalinfo2(conn);
			getremoteinfo2(conn); /*now that ibconnect2 is done, and read requests are waiting, synchronize*/
			crWarning("Initializing IB(%d) from connect side",conn->tcp_socket);
			return 1;
		}
#else

		conn->tcp_socket = socket( cur->ai_family, cur->ai_socktype, cur->ai_protocol );
		if ( conn->tcp_socket < 0 )
		{
			int err = crIBErrno( );
			if (err != EAFNOSUPPORT)
				crWarning( "socket error: %s, trying another way", crIBErrorString( err ) );
			cur=cur->ai_next;
			continue;
		}

		err = 1;
		setsockopt(conn->tcp_socket, SOL_SOCKET, SO_REUSEADDR,  &err, sizeof(int));

		/* Set up the socket the way *we* want. */
		__crSpankSocket( conn->tcp_socket );

#if RECV_BAIL_OUT		
		err = sizeof(unsigned int);
		if ( getsockopt( conn->tcp_socket, SOL_SOCKET, SO_RCVBUF,
				(char *) &conn->krecv_buf_size, &err ) )
		{
			conn->krecv_buf_size = 0;	
		}
#endif

		if ( !connect( conn->tcp_socket, cur->ai_addr, cur->ai_addrlen ) ) {
			if (ibconnect(conn)){
				crWarning( "IB connection error.");
				cr_ib.conns[conn->index] = NULL; /* remove from table */
				return 0;
			};
		        sendlocalinfo(conn);
			getremoteinfo(conn);
			if (ibconnect2(conn)){
				crWarning( "IB connection error, phase 2.");
				cr_ib.conns[conn->index] = NULL; /* remove from table */
				return 0;
			};
			freeaddrinfo(res);
			return 1;
		}
		crIBCloseSocket( conn->tcp_socket );
#endif

		err = crIBErrno( );
		if ( err == EADDRINUSE || err == ECONNREFUSED )
			crWarning( "Couldn't connect to %s:%d, %s",
					conn->hostname, conn->port, crIBErrorString( err ) );

		else if ( err == EINTR )
		{
			crWarning( "connection to %s:%d "
					"interruped, trying again", conn->hostname, conn->port );
			continue;
		}
		else
			crWarning( "Couldn't connect to %s:%d, %s",
					conn->hostname, conn->port, crIBErrorString( err ) );
#ifndef ADDRINFO
		i=0;
#else
		cur=cur->ai_next;
#endif
	}
#ifdef ADDRINFO
	freeaddrinfo(res);
	crWarning( "Couln't find any suitable way to connect to %s", conn->hostname );
#endif
	cr_ib.conns[conn->index] = NULL; /* remove from table */
	return 0;
}

void
crIBDoDisconnect( CRConnection *conn )
{
	int num_conns = cr_ib.num_conns;
	int none_left = 1;
	int i;

	ibclose(conn);
	crIBCloseSocket( conn->tcp_socket );
	conn->tcp_socket = 0;
	conn->type = CR_NO_CONNECTION;
	cr_ib.conns[conn->index] = NULL;

	for (i = 0; i < num_conns; i++) 
	{
		if ( cr_ib.conns[i] && cr_ib.conns[i]->type != CR_NO_CONNECTION )
			none_left = 0; /* found a live connection */
	}

	if (none_left && cr_ib.server_sock != -1)
	{
		crDebug("Closing master socket (probably quitting).");
		crIBCloseSocket( cr_ib.server_sock );
#ifdef CHROMIUM_THREADSAFE
		crFreeMutex(&cr_ib.mutex);
		crFreeMutex(&cr_ib.recvmutex);
#endif
		crBufferPoolFree( cr_ib.bufpool );
		cr_ib.bufpool = NULL;
		last_port = 0;
		cr_ib.initialized = 0;
	}
}

void
crIBConnection( CRConnection *conn )
{
	int i, found = 0;
	int n_bytes;

	IBASSERT( cr_ib.initialized );

	conn->type  = CR_IB;
	conn->Alloc = crIBAlloc;
	conn->Send  = crIBSend;
	conn->SendExact  = crIBWriteExact;
	conn->Recv  = crIBSingleRecv;
	conn->Free  = crIBFree;
	conn->Accept = crIBAccept;
	conn->Connect = crIBDoConnect;
	conn->Disconnect = crIBDoDisconnect;
	conn->InstantReclaim = crIBInstantReclaim;
	conn->HandleNewMessage = crIBHandleNewMessage;
	conn->index = cr_ib.num_conns;
	conn->sizeof_buffer_header = sizeof( CRIBBuffer );
	conn->actual_network = 1;

	conn->krecv_buf_size = 0;

	conn->ib = (struct ibmem*)malloc(sizeof(struct ibmem));
	conn->ib->hndl			= 0;	/*HCA handle*/
	conn->ib->pdhndl		= 0;	/*Protection domain handle*/
	conn->ib->handleset		= 0;	/*have the above been set?*/
	conn->ib->rcqhndl		= 0;	/*Receive queue handle*/
	conn->ib->scqhndl		= 0;	/*Send queue handle*/
	conn->ib->ibport		= 0;	/*physical port*/
	conn->ib->qphndl		= 0;	/*queue pair handle*/
	conn->ib->qpn		= 0;	/*set when qp created by VAPI*/
	conn->ib->remoteqpn		= 0;	/*set after initial TCP contact*/
	conn->ib->lid		= 0;	/*set when qp created*/
	conn->ib->remotelid		= 0;	/*set after initial TCP*/
	conn->ib->sqpsn		= 0;	/*my send queue psn*/
	conn->ib->remotesqpsn	= 0;	/*other sqpsn, my read queue psn*/
	conn->ib->readsposted = 0;
	conn->ib->sendsposted = 0;

	/* Find a free slot */
	for (i = 0; i < cr_ib.num_conns; i++) {
		if (cr_ib.conns[i] == NULL) {
			conn->index = i;
			cr_ib.conns[i] = conn;
			found = 1;
			break;
		}
	}
	
	/* Realloc connection stack if we couldn't find a free slot */
	if (found == 0) {
		n_bytes = ( cr_ib.num_conns + 1 ) * sizeof(*cr_ib.conns);
		crRealloc( (void **) &cr_ib.conns, n_bytes );
		cr_ib.conns[cr_ib.num_conns++] = conn;
	}
}

int crIBGetHostname( char *buf, unsigned int len )
{
	char *override = NULL;
	int ret;

 	override = crGetenv("CR_HOSTNAME");
	if (override)
	{
		crStrncpy(buf, override, len);
		ret = 0;	
	}
	else
		ret = gethostname( buf, len );

	return ret;
}

CRConnection** crIBDump( int *num )
{
	*num = cr_ib.num_conns;

	return cr_ib.conns;
}
