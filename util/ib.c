/* Copyright (c) 2004, Stanford University
 * Copyright (c) 2004, GraphStream
 * All rights reserved
 *
 * Author: Mike Houston (mhouston@graphics.stanford.edu)
 *
 * See the file LICENSE.txt for information on redistributing this software.
 *
 *
 * Notes: 
 *
 * 1)Memory is *slightly* misaligned.  We need to fix the alignment
 * to deal with the IBBuffer space attached to the front of the
 * registered sections.  3/26/04 should be fixed, but double check virt code.
 *
 * 2)We don't yet make sure there are enough buffers for a SendMulti.
 * We probably need to send special control messages to cause the recv
 * side to increase it's buffers or use RDMA reads for large buffers.
 *
 * 3)Slow multi is really broken, we somehow end up corrupting net.c's
 * allocated buffer after awhile...
 *
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <memory.h>

#include <ctype.h> /* for tolower */

#include "cr_error.h"
#include "cr_mem.h"
#include "cr_string.h"
#include "cr_bufpool.h"
#include "cr_net.h"
#include "cr_endian.h"
#include "cr_threads.h"
#include "net_internals.h"
#include "cr_environment.h"
#include "cr_protocol.h"
#include "cr_string.h"

/* added */
#include <mosal.h>  /* for MOSAL_get_counts_per_sec() */
#include <stdio.h>
#include <unistd.h> /* for sleep */
#include <sys/time.h>
#include <sys/resource.h>
#include <vapi.h>
#include <evapi.h>
#include <vapi_common.h>

typedef enum {
    RET_OK  = 0,
    RET_ERR = -1
} ret_val_t;

typedef struct params_tx_s params_tx;

typedef struct {
	VAPI_lkey_t            send_lkey;
	u_int8_t               *send_buf;
	u_int8_t               *recv_buf;
	u_int8_t               *real_send_buf;
	u_int8_t               *real_recv_buf;
	VAPI_lkey_t            recv_lkey;
	VAPI_rkey_t            recv_rkey;
	VAPI_mr_hndl_t         mr_hndl_s;
	VAPI_mr_hndl_t         mr_hndl_r;
} buffer_params_t;

#define MAX_BUFS 8

struct params_tx_s {
	VAPI_pd_hndl_t         pd_hndl;
	int                    max_inline_size;
	int                    max_cap_inline_size;
	u_int8_t               qp_ous_rd_atom;  /* Maximum number of oust. RDMA read/atomic as target */ 
	buffer_params_t        r_buffers[MAX_BUFS];
	buffer_params_t        s_buffers[MAX_BUFS];
	EVAPI_async_handler_hndl_t async_hndl;
	EVAPI_compl_handler_hndl_t comp_hndl;

	int               port_num;
	IB_lid_t          lid;
	IB_lid_t          d_lid;
	VAPI_cq_hndl_t    s_cq_hndl;
	VAPI_cq_hndl_t    r_cq_hndl;
	u_int32_t         num_of_qps;
	int		  size;

	VAPI_qp_hndl_t qp_hndl;
	VAPI_qp_num_t  qp_num;
	VAPI_qp_num_t  d_qpnum;

	int total_sends;
};

void crIBSend( CRConnection *conn, void **bufp, 
		const void *start, unsigned int len );
void
crIBSendExact( CRConnection *conn, const void *buf, unsigned int len );

typedef enum {
	CRIBMemory,
	CRIBMemoryBig
} CRIBBufferKind;

#define CR_IB_BUFFER_MAGIC 0x8716abcd

typedef struct CRIBBuffer {
	unsigned int   magic;
	CRIBBufferKind kind;
	unsigned int   len;
	unsigned int   id;
} CRIBBuffer;


typedef struct CRIBConnection {
	unsigned int          node_id;
	unsigned int          port_num;
	CRConnection   	      *conn;
	struct CRIBConnection *hash_next;
	params_tx params;
	CRBufferPool         *send_pool;
} CRIBConnection;

#define CR_IB_CONN_HASH_SIZE	1024
static struct {
	VAPI_hca_hndl_t        hca_hndl;
	VAPI_hca_vendor_t      hca_vendor;
	VAPI_hca_cap_t	       hca_cap;
	VAPI_hca_id_t          hca_id;

	int		num_ah;
	int             size;
	IB_mtu_t        mtu;
	int             alloc_aligned;
	u_int32_t       num_cqe;
	u_int32_t       num_r_wqe;
	u_int32_t       num_s_wqe;
	u_int32_t       num_rsge;
	u_int32_t       num_ssge;
	
	int                  initialized;
	unsigned int         node_id;
	unsigned int         num_nodes;
	int                  num_conns;
	CRIBConnection        *ib_conn_hash[CR_IB_CONN_HASH_SIZE];
	CRConnection         **conns;
#ifdef CHROMIUM_THREADSAFE
	CRmutex              mutex;
#endif
	CRNetReceiveFuncList *recv_list;
	CRNetCloseFuncList   *close_list;
} cr_ib;


/***************************** VAPI MAGIC *******************************/

static inline u_int64_t gettime()
{
    u_int64_t ticks;
    struct timeval TsNow;

    gettimeofday(&TsNow,NULL);
    ticks = TsNow.tv_usec + TsNow.tv_sec * 1000000; 

    return ticks;
}

static inline u_int64_t get_cpu_clocks_per_seconds ()
{
  FILE* fp;
  char buffer[1024];
  char *match, *match1;
  float clock_speed = 1000.0;
  int found;

  match1 = match = NULL;

  fp = fopen ("/proc/cpuinfo", "r");
  while(NULL != fgets(buffer, 100, fp)){
      match = strstr (buffer, "cpu MHz");
      if(match) break;
  }
  fclose (fp);
  
  if (match == NULL){
      return 1000000000;
  }
  
  /* Parse the line to extrace the clock speed.  */
  match1 = strchr (match, ':');
  if(match1){
      found = sscanf (match1+1, "%f", &clock_speed);
      if(found == 0){
          clock_speed =  1000.0;
      }
  }


  return (u_int64_t) (clock_speed*1000000);
}

#define get_clk64_timeofday(u64)  u64 = gettime()
#define CLK_TO_USEC_GETTIMEOFDAY(u64)  u64

/* Use gettimeofday by default */
#define update_CPU_CLCKS_PER_SECS() \
        cpu_CLCKS_PER_SECS = 1000000.0
#define get_clk64(u64) get_clk64_timeofday(u64)

#define ADJUST_TYPE		u_int64_t
#define STR_2_ULL(str)	strtoull(str,(char **) NULL,0)

u_int64_t cpu_CLCKS_PER_SECS = 1000000.0;

#define DEBUG_LEVEL 1 
/*#define DEBUG_PRINT*/

#define MIN2(a,b) ((a)>(b) ? (b) : (a))
#ifndef MAX
#define MAX(a,b) ((a)>(b) ? (a) : (b))
#endif

#ifdef DEBUG_PRINT
#define COND_PRINTF(aa) printf aa
#else
#define COND_PRINTF(aa)
#endif

#if DEBUG_LEVEL >= 0
#define COND_PRINTF0(aa) printf aa
#else
#define COND_PRINTF0(aa)
#endif
#if DEBUG_LEVEL >= 1
#define COND_PRINTF1(aa) printf aa
#else
#define COND_PRINTF1(aa)
#endif
#if DEBUG_LEVEL >= 2
#define COND_PRINTF2(aa) printf aa
#else
#define COND_PRINTF2(aa)
#endif
#if DEBUG_LEVEL >= 3
#define COND_PRINTF3(aa) printf aa
#else
#define COND_PRINTF3(aa)
#endif
#if DEBUG_LEVEL >= 7
#define COND_PRINTF7(aa) printf aa
#else
#define COND_PRINTF7(aa)
#endif

#define BW_MBSEC(__clk,__bytes) (((ADJUST_TYPE)(__clk))) > 0 ? (1.0*((ADJUST_TYPE)cpu_CLCKS_PER_SECS)*(1.0*((ADJUST_TYPE)(__bytes))))/(1024.0*1024*(1.0*((ADJUST_TYPE)(__clk)))): 0

#define INVAL_HNDL     0xFFFFFFFF
#define PAGE_SIZE_ALIGN      4096
#define LOG_PAGE_SIZE_ALIGN   12

/* literals */
#define DFLT_QP_OUS_RD_ATOM	8
ret_val_t  open_hca(void);
ret_val_t  ib_post_recv_buffers(params_tx *params_p);
ret_val_t  create_common_resources(params_tx *params);
ret_val_t  qp_create(params_tx *params);
ret_val_t  poll_cq(VAPI_hca_hndl_t hca_hndl, 
		   VAPI_cq_hndl_t  cq_hndl,
		   VAPI_wc_desc_t  *sc_p);
VAPI_ret_t poll_cq_nowait(VAPI_hca_hndl_t hca_hndl, 
			  VAPI_cq_hndl_t  cq_hndl,
			  VAPI_wc_desc_t  *sc_p);
ret_val_t cr_ib_send( CRConnection *conn, const void *buf, 
		      unsigned int len, int id);
void* cr_ib_recv( CRConnection *conn, unsigned int* len);

/* forward declarations */
static void async_events_handler( VAPI_hca_hndl_t hca_hndl,
				  VAPI_event_record_t *event_p, 
				  void* priv_data);

static ret_val_t qp_init2rts_one_qp(params_tx *params);


ret_val_t qp_create(params_tx *params)
{
     VAPI_qp_init_attr_t    qp_init_attr;
     VAPI_qp_prop_t         qp_prop;
     VAPI_ret_t             res;
     VAPI_qp_hndl_t         qp_hndl =  INVAL_HNDL;
     
     qp_init_attr.cap.max_oust_wr_rq = cr_ib.num_r_wqe;
     qp_init_attr.cap.max_oust_wr_sq = cr_ib.num_s_wqe;
     qp_init_attr.cap.max_sg_size_rq = cr_ib.num_rsge;
     qp_init_attr.cap.max_sg_size_sq = cr_ib.num_ssge;
     qp_init_attr.pd_hndl            = params->pd_hndl;
     qp_init_attr.rdd_hndl           = 0;
     qp_init_attr.sq_sig_type        = VAPI_SIGNAL_ALL_WR;
     qp_init_attr.rq_sig_type        = VAPI_SIGNAL_ALL_WR;
     qp_init_attr.ts_type            = IB_TS_RC;
     qp_init_attr.rq_cq_hndl         = params->r_cq_hndl;
     qp_init_attr.sq_cq_hndl         = params->s_cq_hndl;
          
     params->qp_hndl = INVAL_HNDL;
     res= VAPI_create_qp(cr_ib.hca_hndl, 
			 &qp_init_attr, 
			 &qp_hndl, 
			 &qp_prop);
     if (res != VAPI_OK) {
	     crWarning("IB: Error: Creating QP : %s", VAPI_strerror(res));
	     return(RET_ERR);
     }
     params->qp_hndl = qp_hndl;
     params->qp_num = qp_prop.qp_num;
     params->max_cap_inline_size = qp_prop.cap.max_inline_data_sq;
     return(RET_OK);
}

ret_val_t open_hca(void)
{
	VAPI_ret_t             res;
	VAPI_hca_hndl_t        hca_hndl;
	
	/********************** HCA handle **********************/
	res = EVAPI_get_hca_hndl(cr_ib.hca_id, &hca_hndl); 
	if (res != VAPI_OK) {
		crWarning("IB: Error: Opening HCA : %s", VAPI_strerror(res));
		return(RET_ERR);
	}
	cr_ib.hca_hndl = hca_hndl;
	crWarning("IB: open hca handle = %d:%d",cr_ib.hca_hndl,hca_hndl );
	
	/************* get some configuration info *************/
	res = VAPI_query_hca_cap( hca_hndl, 
				  &cr_ib.hca_vendor, 
				  &cr_ib.hca_cap);
	if (res != VAPI_OK) {
		crWarning("IB: Error on VAPI_query_hca_cap : %s", VAPI_strerror(res));
		return(RET_ERR);
	}

	return res;
}

ret_val_t create_common_resources(params_tx *params)   
{
     VAPI_ret_t             res;
     VAPI_hca_hndl_t        hca_hndl;
     VAPI_hca_port_t        hca_port;
     VAPI_mrw_t             mr_in, mr_out;
     VAPI_pd_hndl_t         pd_hndl   =  INVAL_HNDL; 
     VAPI_cq_hndl_t         s_cq_hndl =  INVAL_HNDL;
     VAPI_cq_hndl_t         r_cq_hndl =  INVAL_HNDL;
     VAPI_mr_hndl_t         mr_hndl   =  INVAL_HNDL;

     u_int8_t               *send_buf      = NULL;
     u_int8_t               *recv_buf      = NULL;
     u_int8_t               *real_send_buf = NULL;
     u_int8_t               *real_recv_buf = NULL;

     int       act_num_cqe;
     int       size         = cr_ib.size;
     u_int32_t num_cqe      = cr_ib.num_cqe;

     int i;
     
     hca_hndl = cr_ib.hca_hndl;
     crWarning("HCA handle = %d",cr_ib.hca_hndl);
      
     /********************** PD **********************/
     res = EVAPI_alloc_pd(hca_hndl, cr_ib.num_ah, &pd_hndl);
     if (res != VAPI_OK) {
	     crWarning("IB: Error: Allocating PD : %s", VAPI_strerror(res));
	     return(RET_ERR);
     }
     params->pd_hndl = pd_hndl;
     
     /********************** Port Dependent Resources **********************/
     
    
     /********************** get port properties  **********************/
     crWarning("Trying port %d", params->port_num);
     res = VAPI_query_hca_port_prop(hca_hndl,
				    (IB_port_t)params->port_num,
				    ( VAPI_hca_port_t *)&hca_port);
     if (res != VAPI_OK) {
	     crWarning("IB: Error on VAPI_query_hca_port_prop: %s", 
			VAPI_strerror(res));
	     return(RET_ERR);
     }
     params->lid = hca_port.lid;
     
     /********************** Send CQ **********************/
     res = VAPI_create_cq(hca_hndl, num_cqe, &s_cq_hndl, &act_num_cqe);
     if (res != VAPI_OK) {
	     crWarning("IB: Error: Creating CQ : %s", VAPI_strerror(res));
	     return(RET_ERR);
     }
     params->s_cq_hndl = s_cq_hndl;
     
     /********************** RECV CQ **********************/
     res = VAPI_create_cq(hca_hndl, num_cqe, &r_cq_hndl, &act_num_cqe);
     if (res != VAPI_OK) {
	     crWarning("IB: Error: Creating CQ : %s", VAPI_strerror(res));
	     return(RET_ERR);
     }
     params->r_cq_hndl = r_cq_hndl;
	  
     /********************** async event handler **********************/
     res= EVAPI_set_async_event_handler(hca_hndl,async_events_handler,
					params,&(params->async_hndl));
     if (res != VAPI_OK) {
	     crWarning("IB: Error: Creating in EVAPI_set_async_eventh (send) : %s", 
		       VAPI_strerror(res));
	     return(RET_ERR);
     }
     
     /********************** SEND MR **********************/
     for(i=0; i<MAX_BUFS; i++){
	     real_send_buf = VMALLOC(size+2*sizeof(CRIBBuffer)+PAGE_SIZE_ALIGN);
	     send_buf = (void *) MT_UP_ALIGNX_VIRT((long int)real_send_buf+sizeof(CRIBBuffer), LOG_PAGE_SIZE_ALIGN);
	     
	     COND_PRINTF3(("IB: real=%p after=%p\n",real_send_buf,send_buf));
	     
	     if (send_buf == NULL) {
		     crWarning("IB: Try again..alloc memory failed");
		     return(RET_ERR);
	     }
	     
	     mr_in.start = (VAPI_virt_addr_t)(MT_virt_addr_t)send_buf;
	     mr_in.type  = VAPI_MR;
	     
	     memset(send_buf,i,size);
	     
	     params->s_buffers[i].send_buf = send_buf - sizeof(CRIBBuffer);
	     params->s_buffers[i].real_send_buf = real_send_buf;
	     
	     mr_in.acl =  VAPI_EN_LOCAL_WRITE | VAPI_EN_REMOTE_WRITE |  
		     VAPI_EN_REMOTE_READ | VAPI_EN_REMOTE_ATOM;/*Noam + Vladz(MCH: magic?)*/
	     
	     mr_in.l_key = 0;
	     mr_in.pd_hndl = pd_hndl;
	     mr_in.r_key = 0;
	     mr_in.size = size;
	     
	     res = VAPI_register_mr(hca_hndl, &mr_in, &mr_hndl, &mr_out);
	     if (res != VAPI_OK) {
		     crWarning("IB: Error: Registering MR : %s", VAPI_strerror(res));
		     return(RET_ERR);
	     }
	     params->s_buffers[i].send_lkey = mr_out.l_key;
	     params->s_buffers[i].mr_hndl_s = mr_hndl;
     }
     /**********************  RECV MR  **********************/
     for(i=0; i<MAX_BUFS; i++){
	     real_recv_buf = VMALLOC(size+2*sizeof(CRIBBuffer)+PAGE_SIZE_ALIGN);
	     recv_buf = (void *) MT_UP_ALIGNX_VIRT((long int)real_recv_buf+sizeof(CRIBBuffer), LOG_PAGE_SIZE_ALIGN);

	     COND_PRINTF(("IB: real=%p after=%p\n",real_recv_buf,recv_buf));
	     
	     if (recv_buf == NULL) {
		     crWarning("IB: Try again..alloc memory failed");
		     return(RET_ERR);
	     }
	     
	     mr_in.start = (VAPI_virt_addr_t)(MT_virt_addr_t)recv_buf;
	     mr_in.type = VAPI_MR;

	     memset(recv_buf,0,size);
	     
	     params->r_buffers[i].recv_buf = recv_buf - sizeof(CRIBBuffer);
	     params->r_buffers[i].real_recv_buf = real_recv_buf;
	     
	     mr_in.acl =  VAPI_EN_LOCAL_WRITE | VAPI_EN_REMOTE_WRITE | 
		     VAPI_EN_REMOTE_READ | VAPI_EN_REMOTE_ATOM;/*Noam + Vladz(MCH: magic?)*/
	     
	     mr_in.l_key = 0;
	     mr_in.pd_hndl = pd_hndl;
	     mr_in.r_key = 0;
	     mr_in.size  = size;            
	     res = VAPI_register_mr(hca_hndl, &mr_in, &mr_hndl, &mr_out);
	     if (res != VAPI_OK) {
		     crWarning("IB: Error: Registering MR : %s", VAPI_strerror(res));
		     return(RET_ERR);
	     }
	     params->r_buffers[i].recv_lkey = mr_out.l_key;
	     params->r_buffers[i].recv_rkey = mr_out.r_key;
	     params->r_buffers[i].mr_hndl_r = mr_hndl;
     }
     COND_PRINTF(("IB: registerd mem\n"));
     
     return(RET_OK);
}

static void show_qp_state(VAPI_hca_hndl_t  hca_hndl, VAPI_qp_hndl_t qp_hndl, VAPI_qp_num_t  qp_num)
{
     VAPI_qp_attr_t       qp_attr;
     VAPI_qp_attr_mask_t  qp_attr_mask;
     VAPI_qp_init_attr_t  qp_init_attr;
     VAPI_ret_t           res;
     
     res = VAPI_query_qp(hca_hndl, qp_hndl, &qp_attr, &qp_attr_mask, &qp_init_attr );
     if (res == VAPI_OK) {
	  crWarning("IB: QP handle 0x%x, QP num 0x%x: state %d", (unsigned)qp_hndl, (unsigned)qp_num, qp_attr.qp_state);
     }
     else {
	  crWarning("IB: Error on VAPI_query_qp: %s", VAPI_strerror(res));
     }
}

static ret_val_t qp_init2rts_one_qp(params_tx *params)
{
     VAPI_qp_attr_mask_t qp_attr_mask;
     VAPI_qp_attr_t      qp_attr;
     VAPI_qp_cap_t       qp_cap;
     VAPI_ret_t          res;     
     VAPI_hca_hndl_t     hca_hndl  = cr_ib.hca_hndl;
     VAPI_qp_hndl_t      qp_hndl   = params->qp_hndl;

     IB_mtu_t            mtu       = cr_ib.mtu;

     int                 d_qpnum   = params->d_qpnum;
     int                 dlid      = params->d_lid;
     int                 port_num  = params->port_num;
     
     /************************************************************************/
     
     /*************************** INIT ***************************************/
     QP_ATTR_MASK_CLR_ALL(qp_attr_mask);
     
     qp_attr.qp_state = VAPI_INIT;
     QP_ATTR_MASK_SET(qp_attr_mask,QP_ATTR_QP_STATE);
     
     qp_attr.pkey_ix  = 0;
     QP_ATTR_MASK_SET(qp_attr_mask,QP_ATTR_PKEY_IX);
     
     qp_attr.port     = port_num;
     QP_ATTR_MASK_SET(qp_attr_mask,QP_ATTR_PORT);
     
     qp_attr.remote_atomic_flags = VAPI_EN_REM_WRITE | VAPI_EN_REM_READ | VAPI_EN_REM_ATOMIC_OP;
     QP_ATTR_MASK_SET(qp_attr_mask,QP_ATTR_REMOTE_ATOMIC_FLAGS);
     
     res = VAPI_modify_qp(hca_hndl, qp_hndl, &qp_attr, &qp_attr_mask, &qp_cap);
     if (res != VAPI_OK) {
	     crWarning("IB: Error: Modifying  QP to INIT: %s", VAPI_strerror(res));
	     return(RET_ERR);
     }
     COND_PRINTF(("IB: Modified QP to INIT\n"));
     
     /**************************** RTR *************************************/   
     QP_ATTR_MASK_CLR_ALL(qp_attr_mask);
     
     qp_attr.qp_state = VAPI_RTR;
     QP_ATTR_MASK_SET(qp_attr_mask,QP_ATTR_QP_STATE);
     
     qp_attr.qp_ous_rd_atom  = MIN2(DFLT_QP_OUS_RD_ATOM, 
				    cr_ib.hca_cap.max_qp_ous_rd_atom);
     
     QP_ATTR_MASK_SET(qp_attr_mask,QP_ATTR_QP_OUS_RD_ATOM);
     
     qp_attr.dest_qp_num = d_qpnum; 
     QP_ATTR_MASK_SET(qp_attr_mask, QP_ATTR_DEST_QP_NUM);
     
     qp_attr.av.sl            = 0;
     qp_attr.av.grh_flag      = FALSE;
     qp_attr.av.dlid          = dlid;
     qp_attr.av.static_rate   = 2;
     qp_attr.av.src_path_bits = 0;
     QP_ATTR_MASK_SET(qp_attr_mask,QP_ATTR_AV);
     
     qp_attr.path_mtu      = mtu;
     QP_ATTR_MASK_SET(qp_attr_mask,QP_ATTR_PATH_MTU);
     
     qp_attr.rq_psn           = 0;
     QP_ATTR_MASK_SET(qp_attr_mask,QP_ATTR_RQ_PSN);
     
     /*qp_attr.pkey_ix = 0;
       QP_ATTR_MASK_SET(qp_attr_mask,QP_ATTR_PKEY_IX);*/
     
     qp_attr.min_rnr_timer = 0;
     QP_ATTR_MASK_SET(qp_attr_mask,QP_ATTR_MIN_RNR_TIMER);
     
     res = VAPI_modify_qp(hca_hndl, qp_hndl, &qp_attr, &qp_attr_mask, &qp_cap);
     if (res != VAPI_OK) {
	  crWarning("IB: Error: Modifying  QP to RTR: %s", VAPI_strerror(res));      
	  return(RET_ERR);
     }
     COND_PRINTF(("IB: Moodified QP to RTR\n"));
     
     /***************************** RTS **********************************/   
     
     QP_ATTR_MASK_CLR_ALL(qp_attr_mask);
     
     qp_attr.qp_state   = VAPI_RTS;
     QP_ATTR_MASK_SET(qp_attr_mask,QP_ATTR_QP_STATE);
     
     qp_attr.sq_psn   = 0;
     QP_ATTR_MASK_SET(qp_attr_mask,QP_ATTR_SQ_PSN);
     
     qp_attr.timeout   = 18; /* ~1 sec */
     QP_ATTR_MASK_SET(qp_attr_mask,QP_ATTR_TIMEOUT);
     
     qp_attr.retry_count   = 6;
     QP_ATTR_MASK_SET(qp_attr_mask,QP_ATTR_RETRY_COUNT);
     
     qp_attr.rnr_retry     = 6;
     QP_ATTR_MASK_SET(qp_attr_mask,QP_ATTR_RNR_RETRY);
     
     qp_attr.ous_dst_rd_atom  = params->qp_ous_rd_atom;
     QP_ATTR_MASK_SET(qp_attr_mask,QP_ATTR_OUS_DST_RD_ATOM);
     
     res = VAPI_modify_qp(hca_hndl,qp_hndl,&qp_attr,&qp_attr_mask,&qp_cap);
     if (res != VAPI_OK) {
	  crWarning("IB: Error: Modifying  QP to RTS: %s", VAPI_strerror(res));
	  return(RET_ERR);
     }
     COND_PRINTF(("IB: Modified QP to RTS\n"));
     return(RET_OK);
}

ret_val_t poll_cq(VAPI_hca_hndl_t hca_hndl, 
		  VAPI_cq_hndl_t  cq_hndl,
		  VAPI_wc_desc_t  *sc_p)
{
     VAPI_ret_t res;
     
     res = VAPI_poll_cq(hca_hndl, cq_hndl, sc_p);
     
     while (res == VAPI_CQ_EMPTY) {
	  res = VAPI_poll_cq(hca_hndl, cq_hndl, sc_p);
     }
     
     if (res != VAPI_OK) {
	  crWarning("IB: Error: Polling CQ : %s", VAPI_strerror(res));
	  return(RET_ERR);
     }
     if (sc_p->status != VAPI_SUCCESS) {
	  crWarning("IB: Completion with error on queue (syndrome=0x%x=%s , opcode=%d=%s)",
		 sc_p->vendor_err_syndrome,VAPI_wc_status_sym(sc_p->status),sc_p->opcode,VAPI_cqe_opcode_sym(sc_p->opcode));
	  crWarning("IB: %s: completion with error (%d) detected", __func__, sc_p->status);
	  return(RET_ERR);
     }
     return(RET_OK);
}

/*
  Purpose: performs one poll
  Returns:
  VAPI_OK - on success
  VAPI_CQ_EMPTY - while no completion event
  VAPI_EGEN - on error
*/
VAPI_ret_t poll_cq_nowait(VAPI_hca_hndl_t hca_hndl, 
			  VAPI_cq_hndl_t  cq_hndl,
			  VAPI_wc_desc_t  *sc_p)
{
     VAPI_ret_t  res;
     
     res = VAPI_poll_cq(hca_hndl, cq_hndl, sc_p);
     
     if (res == VAPI_OK) {
	     if (sc_p->status != VAPI_SUCCESS) {
		     crWarning("IB: Completion with error on queue (syndrome=0x%x=%s , opcode=%d=%s)",
			    sc_p->vendor_err_syndrome,VAPI_wc_status_sym(sc_p->status),sc_p->opcode,VAPI_cqe_opcode_sym(sc_p->opcode));
		     crWarning("IB: %s: completion with error (%d) detected", __func__, sc_p->status);
		     return(VAPI_EGEN);
	     }
     }
     
     return(res);
}


static void
async_events_handler( VAPI_hca_hndl_t hca_hndl, 
		      VAPI_event_record_t *event_p, 
		      void* priv_data)
{    
     crWarning("IB: Got async. event: %s (0x%x - %s)",
	    VAPI_event_record_sym(event_p->type),
	    event_p->syndrome,VAPI_event_syndrome_sym(event_p->syndrome));
}

/*************************************************************************
 * Function: clean_up
 *
 * Description:
 *   Clean all IB resources and memory allocations
 *
 *************************************************************************/ 
static void
clean_up(params_tx *params)
{
	int i;
	crWarning("IB: Cleaning up VAPI resources");
	if (params->qp_hndl != VAPI_INVAL_HNDL) {
		VAPI_destroy_qp(cr_ib.hca_hndl,
				params->qp_hndl);
	}

	if (params->r_cq_hndl != VAPI_INVAL_HNDL) {
	     VAPI_destroy_cq(cr_ib.hca_hndl,params->r_cq_hndl);
	}
	if (params->s_cq_hndl != VAPI_INVAL_HNDL) {
	     VAPI_destroy_cq(cr_ib.hca_hndl,params->s_cq_hndl);
	}
	
	/* cleanup MRs */
	for(i=0; i<MAX_BUFS; i++){
		if (params->s_buffers[i].mr_hndl_s != VAPI_INVAL_HNDL) {
			VAPI_deregister_mr(cr_ib.hca_hndl,params->s_buffers[i].mr_hndl_s);
		}
		
		if (params->s_buffers[i].real_send_buf != NULL) {
			free(params->s_buffers[i].real_send_buf);
		}
	}

	for(i=0; i<MAX_BUFS; i++){
		if  (params->r_buffers[i].mr_hndl_r != VAPI_INVAL_HNDL) {
			VAPI_deregister_mr(cr_ib.hca_hndl,params->r_buffers[i].mr_hndl_r);
		}
		if (params->r_buffers[i].real_recv_buf != NULL) {
			free(params->r_buffers[i].real_recv_buf);
		}
	}
	
	if (params->pd_hndl != VAPI_INVAL_HNDL) {
		VAPI_dealloc_pd(cr_ib.hca_hndl,params->pd_hndl);
	}
	if (cr_ib.hca_hndl != VAPI_INVAL_HNDL) {
		EVAPI_release_hca_hndl(cr_ib.hca_hndl);  
	}
	
	exit(0);
}

/*************************************************************************
 * Function: init_params
 *
 * Description:
 *   Initialize all IB resources
 *
 *************************************************************************/ 
static void
init_params(params_tx *params)
{
	int i;
	params->pd_hndl     = VAPI_INVAL_HNDL;

	params->max_inline_size           = 400; 
	params->port_num     = 1;
	params->lid          = -1;
	params->d_lid        = -1;
	params->num_of_qps   = 1;
	params->r_cq_hndl    = VAPI_INVAL_HNDL;
	params->s_cq_hndl    = VAPI_INVAL_HNDL;
	
	params->num_of_qps = 1;

	/* per buffer */
	for(i=0; i<MAX_BUFS; i++){
		params->r_buffers[i].recv_buf    = NULL;
		params->r_buffers[i].mr_hndl_r   = VAPI_INVAL_HNDL;
	}
	for(i=0; i<MAX_BUFS; i++){
		params->s_buffers[i].send_buf    = NULL;
		params->s_buffers[i].mr_hndl_s   = VAPI_INVAL_HNDL;
	}
}

static void
msg2num(char *msg, char *label, long def, void *out, unsigned int len)
{
	char *startstr;
	u_int64_t tmp_val;
	
	startstr = strstr(msg,label);
	if(NULL == startstr){
		tmp_val = def;
	}
	else 
	{
		tmp_val = strtoull(startstr+strlen(label),(char **) NULL,0);
	}
	
	switch (len) {
        case 1:
		*((char *)out) = (char) tmp_val;
		break;
        case 2:
		*((short *)out) = (short ) tmp_val;
		break;
        case 4:
		*((int *)out) = (int ) tmp_val;
		break;
        case 8:
		*((u_int64_t *)out) = (u_int64_t ) tmp_val;
		break;
        default:
		*((char *)out) = (char) tmp_val;
		break;
	}
}


static void
print_params(params_tx *p_params)
{
	params_tx *p = p_params;
	
	crWarning("IB: ========== parameters ==========");
	
	crWarning("IB: IN: size %d, mtu %d", 
		     cr_ib.size, cr_ib.mtu);
	crWarning("IB: IN: alloc_aligned %d", cr_ib.alloc_aligned);
	crWarning("IB: IN: port_num %d", 
		     p->port_num);
	crWarning("IB: IN: NUMs: cqe %d, r_wqe %d, s_wqe %d, rsge %d, ssge %d",
		     cr_ib.num_cqe, cr_ib.num_r_wqe, cr_ib.num_s_wqe, cr_ib.num_rsge, cr_ib.num_ssge);
	crWarning("IB: HANDLEs: hca 0x%x, pd 0x%x, qp 0x%x",
		     (unsigned int)cr_ib.hca_hndl, (unsigned int)p->pd_hndl, (unsigned int)p->qp_hndl);
	crWarning("IB: max_inline_size %d, max_cap_inline_size %d, ous_src_rd_atom %d, ous_dst_rd_atom %d",
	       p->max_inline_size, p->max_cap_inline_size, cr_ib.hca_cap.max_qp_ous_rd_atom, p->qp_ous_rd_atom );
	crWarning("IB: port %d, lid %d, dlid %d, s_cq 0x%x, r_cq 0x%x, "
	       "\n\t\tqps %d, qp0 0x%x, dqp0 0x%x",
	       p->port_num, p->lid, p->d_lid, 
	       (unsigned int)p->s_cq_hndl, (unsigned int)p->r_cq_hndl, 
	       p->num_of_qps,
	       p->qp_num, p->d_qpnum
	     );
	crWarning("IB: ============================================");
}

/******************* END VAPI JUNK *****************/

ret_val_t
ib_post_recv_buffers( params_tx *params_p)
{
	
	VAPI_rr_desc_t      rr;
	VAPI_sg_lst_entry_t sg_entry_r;
	VAPI_ret_t          res;
	VAPI_hca_hndl_t     hca_hndl    = cr_ib.hca_hndl;
	VAPI_qp_hndl_t      qp_hndl     = params_p->qp_hndl;

	int i;

	rr.opcode     = VAPI_RECEIVE;
	rr.comp_type  = VAPI_SIGNALED;
	rr.sg_lst_len = 1;
	rr.sg_lst_p   = &sg_entry_r;

	/* Pre-post all recv buffers */
	for (i=0;i<MAX_BUFS;i++) {
		rr.id = i;
		sg_entry_r.lkey = params_p->r_buffers[i].recv_lkey;
		sg_entry_r.len  = cr_ib.size;
		sg_entry_r.addr = (VAPI_virt_addr_t)(MT_virt_addr_t)params_p->r_buffers[i].recv_buf+sizeof(CRIBBuffer);
		res = VAPI_post_rr(hca_hndl, qp_hndl, &rr);
		if (res != VAPI_OK) {
			crWarning("IB: Error: Pre-posting Recv buffer (port %d) : %s", params_p->port_num, VAPI_strerror(res));
			return RET_ERR;
		}
	}
	return RET_OK;
	
}

/******************/

void
crIBSendExact( CRConnection *conn, const void *buf, unsigned int len )
{
	crError("crIBSendExact should never be called!");
	(void) conn;
	(void) buf;
	(void) len;
}
/* Use a power of 2 to the lookup is just a mask -- this works fine
 * because we expect the IB node numbering to be dense */
#define CR_IB_HASH(n)	cr_ib.ib_conn_hash[ (n) & (CR_IB_CONN_HASH_SIZE-1) ]


static __inline CRIBConnection *
crIBConnectionLookup( unsigned int id )
{
	CRIBConnection *ib_conn;

	ib_conn = CR_IB_HASH( id );
	while ( ib_conn )
	{
		if ( ib_conn->conn->id == id )
		{
			return ib_conn;
		}
		ib_conn = ib_conn->hash_next;
	}

	crError( "IB: lookup on unknown connection: id=%d", id );

	/* unreached */
	return NULL;
}

static void
crIBConnectionAdd( CRConnection *conn, CRIBConnection* ib_conn_in )
{
	CRIBConnection *ib_conn, **bucket;
	
	crDebug( "IB: Adding a connection with id 0x%x", conn->id );
	bucket = &CR_IB_HASH( conn->id );
	crDebug( "IB: The initial bucket was %p", bucket );
	
	for ( ib_conn = *bucket; ib_conn != NULL; 
	      ib_conn = ib_conn->hash_next )
	{
		if ( ib_conn->node_id  == conn->ib_node_id && 
		     ib_conn->port_num == conn->ib_port_num )
		{
			crWarning( "IB: I've already got a connection from node=%u "
				   "port=%u (\"%s\"), so why is it connecting again?",
				   conn->ib_node_id, conn->ib_port_num, conn->hostname );
		}
	}
	
	ib_conn = ib_conn_in;/*(CRIBConnection*)crAlloc( sizeof(*ib_conn) );*/
	ib_conn->node_id   = conn->ib_node_id;
	ib_conn->port_num  = conn->ib_port_num;
	ib_conn->conn      = conn;
	ib_conn->hash_next = *bucket;
	
	*bucket = ib_conn_in;
}

void* cr_ib_recv( CRConnection *conn, 
		  unsigned int* len)
{
	CRIBConnection* ib_conn;
	VAPI_wc_desc_t      rc;
	void* dst;
	VAPI_ret_t          res;
	/*char string[1024];*/
	
	/* Copy all input parameters - don't want to rewrite the loop code */
	VAPI_hca_hndl_t     hca_hndl    = cr_ib.hca_hndl;
	VAPI_cq_hndl_t      r_cq_hndl;
	ib_conn = crIBConnectionLookup(conn->id);

	rc.byte_len = 0;

	r_cq_hndl = ib_conn->params.r_cq_hndl;

	/* sit and wait for a recv completion*/
	res = poll_cq_nowait(hca_hndl, r_cq_hndl, &rc);
	if (res == VAPI_OK) {
		/*crDebug("I got recv buffer id %d, len %d", (int)rc.id, rc.byte_len);*/
		dst = ib_conn->params.r_buffers[rc.id].recv_buf;
		/*crDebug("I'm setting it to 0x%x", (unsigned int)dst);*/
		*len = rc.byte_len;
		
		/*crBytesToString( string, sizeof(string), dst+sizeof(CRIBBuffer), *len );
		  crDebug("I recv buf=[%s]", string);*/
		
		return dst;
	}
	else if( res == VAPI_EGEN){
	     crWarning("IB: RECV error on conn %d", conn->id);
	     show_qp_state(hca_hndl, ib_conn->params.qp_hndl, ib_conn->params.qp_num);
	     if(rc.byte_len > 0){
		  crWarning("IB: recovering");
		  dst = ib_conn->params.r_buffers[rc.id].recv_buf;
		  *len = rc.byte_len;
		  return dst;
	     }
	     else{
		  *len = 0;
		  return NULL;
	     }
	}
	else{
		*len = 0;
		return NULL;
	}
}

ret_val_t
cr_ib_send( CRConnection *conn, const void *buf, unsigned int len, int id)
{
	CRIBConnection* ib_conn;
	VAPI_ret_t          res;
	VAPI_sr_desc_t      sr;
	VAPI_sg_lst_entry_t sg_entry_s;

	/* Copy all input parameters - don't want to rewrite the loop code */
	VAPI_hca_hndl_t     hca_hndl    = cr_ib.hca_hndl;
	VAPI_qp_hndl_t      qp_hndl;
	VAPI_cq_hndl_t      s_cq_hndl;
	/*char string[128];*/

	ib_conn = crIBConnectionLookup(conn->id);	

	qp_hndl   = ib_conn->params.qp_hndl;
	s_cq_hndl = ib_conn->params.s_cq_hndl;

	/***************************************************************************/
	sr.opcode      = VAPI_SEND;
	sr.comp_type   = VAPI_SIGNALED;
	sr.set_se      = FALSE;
	sr.remote_qkey = 0;
	sr.fence = TRUE;   /* In case we are sending a notification after RDMA-R */
	
	sr.sg_lst_len = 1;
	sr.sg_lst_p = &sg_entry_s;

	sr.id = id;
	/*crDebug("Conn: %d, Sending %d, len %d", conn->id, (int)sr.id, len);*/
	
	/* Post send buffer */
	
	sg_entry_s.addr = (VAPI_virt_addr_t)(MT_virt_addr_t)buf;
	sg_entry_s.len  = len;
	sg_entry_s.lkey = ib_conn->params.s_buffers[id].send_lkey;
	res = VAPI_post_sr(hca_hndl, qp_hndl, &sr);
	
	if (res != VAPI_OK) {
		crWarning("IB: sg_list_len %d", sr.sg_lst_len );
		crWarning("IB: Error: Posting send desc (%d): %s", id, VAPI_strerror(res));
		return RET_ERR;
	}
	return RET_OK;
}

static void
crIBAccept( CRConnection *conn, const char *hostname, unsigned short port )
{
     CRConnection *mother;
     char response[8096];
     char tmp_msg[8096];
     char my_hostname[256];
     int i;
     CRIBConnection *ib_conn;

     crWarning( "IB: crIBAccept is being called -- brokering the connection through the mothership!." );
     
     mother = __copy_of_crMothershipConnect( );
     
     if (!hostname)
     {	
	     if ( crGetHostname( my_hostname, sizeof( my_hostname ) ) )
	     {
		     crError( "IB: Couldn't determine my own hostname in crIBAccept!" );
	     }
     }
     else
	     crStrcpy(my_hostname, hostname);  

     ib_conn = (CRIBConnection*)crAlloc( sizeof(*ib_conn) );
     /* initialize ib stuff */
     /* Initialize to default parameters */
     init_params(&ib_conn->params); 	

     ib_conn->params.total_sends = 0;
     
     /* Create IB reasources  */
     if (RET_OK != create_common_resources(&ib_conn->params)) {
	     clean_up(&ib_conn->params);
     }
     
     if (RET_OK != qp_create(&ib_conn->params)) {
	     clean_up(&ib_conn->params);
     } 

     sprintf(tmp_msg, "acceptrequest ib %s %d %d %d lid_p1=%d qp_ous=%d qp=%d", 
	     my_hostname,
	     conn->port, 
	     cr_ib.node_id, 
	     conn->endianness, 
	     ib_conn->params.lid,
	     (int)MIN2(DFLT_QP_OUS_RD_ATOM, 
		       cr_ib.hca_cap.max_qp_ous_rd_atom), 
	     ib_conn->params.qp_num);
     
     /* Tell the mothership I'm willing to receive a client, and what my IB info is */
     if (!__copy_of_crMothershipSendString( mother, response, tmp_msg) )
     {
	  crError( "IB: Mothership didn't like my accept request" );
     }
     sscanf( response, "%d %d %d", 
	     &(conn->id), 
	     &(conn->ib_node_id), 
	     &(conn->ib_port_num) );
     /* The response will contain the IB information for the guy who accepted 
      * this connection.  The mothership will sit on the acceptrequest 
      * until someone connects. */
     msg2num(response, "lid_p1=", -1, 
	     &(ib_conn->params.d_lid), 
	     sizeof(ib_conn->params.d_lid));
     msg2num(response, "qp_ous=", 8, 
	     &(ib_conn->params.qp_ous_rd_atom), 
	     sizeof(ib_conn->params.qp_ous_rd_atom));
     msg2num(response, "qp=", -1, 
	     &(ib_conn->params.d_qpnum), 
	     sizeof(ib_conn->params.d_qpnum));

     /* NOW, we can add the connection, since we have enough information 
      * to uniquely determine the sender when we get a packet! */
     crIBConnectionAdd( conn, ib_conn );
     ib_conn = crIBConnectionLookup(conn->id);	

     if (RET_OK != qp_init2rts_one_qp(&ib_conn->params)) {
	  clean_up(&ib_conn->params);
     }
     
     print_params( &ib_conn->params );

     /* setup all recv buffers */
     for(i=0; i<MAX_BUFS; i++){
	     CRIBBuffer *ib_buffer = (CRIBBuffer*)ib_conn->params.r_buffers[i].recv_buf;
	     ib_buffer->magic = CR_IB_BUFFER_MAGIC;
	     ib_buffer->kind = CRIBMemory;
	     ib_buffer->len = conn->buffer_size;
	     ib_buffer->id = i;
     }

     /* post all recv buffers */
     ib_post_recv_buffers( &ib_conn->params );

     /* and push all of the send buffers */
     ib_conn->send_pool = crBufferPoolInit(MAX_BUFS);
     for(i=0; i<MAX_BUFS; i++){
	     CRIBBuffer *ib_buffer = (CRIBBuffer*)ib_conn->params.s_buffers[i].send_buf;
	     ib_buffer->magic = CR_IB_BUFFER_MAGIC;
	     ib_buffer->kind = CRIBMemory;
	     ib_buffer->len = conn->buffer_size;
	     ib_buffer->id = i;
	     crBufferPoolPush( ib_conn->send_pool, ib_buffer, conn->buffer_size);
     } 
     crWarning("IB: conn: %d buffer count = %d", conn->id, crBufferPoolGetNumBuffers( ib_conn->send_pool));
     
     crDebug("IB: Accept!");
     __copy_of_crMothershipDisconnect( mother );
}


static void *
crIBAlloc( CRConnection *conn )
{
	CRIBBuffer *ib_buffer;
	VAPI_wc_desc_t      sc;
	CRIBConnection *ib_conn;
	
	/* Copy all input parameters - don't want to rewrite the loop code */
	VAPI_hca_hndl_t     hca_hndl    = cr_ib.hca_hndl;
	VAPI_qp_hndl_t      qp_hndl;
	VAPI_cq_hndl_t      s_cq_hndl;
	int count = 0;

	ib_conn = crIBConnectionLookup(conn->id);	

	qp_hndl   = ib_conn->params.qp_hndl;
	s_cq_hndl = ib_conn->params.s_cq_hndl;

	/*fprintf(stderr, "fast buffer recover: \n");*/
	/* see if other buffers finished but don't block unless it's needed */
	while(poll_cq_nowait(hca_hndl,s_cq_hndl,&sc) == VAPI_OK){
		/*fprintf(stderr, "conn: %d, recovered #%d\n", conn->id, (int)sc.id);*/
		crBufferPoolPush( ib_conn->send_pool, ib_conn->params.s_buffers[sc.id].send_buf, 
											conn->buffer_size );
		count++;
	}  
	//crWarning("conn: %d fast recovered %d, pool = %d", conn->id, count, crBufferPoolGetNumBuffers( ib_conn->send_pool));

	ib_buffer = (CRIBBuffer *) 
		crBufferPoolPop( ib_conn->send_pool, conn->buffer_size );

	if ( ib_buffer == NULL )
	{
		crWarning("IB: Crap: out of IB buffers, blocking to try to get one!");
		poll_cq(hca_hndl,s_cq_hndl,&sc);
		fprintf(stderr, "IB: Got one!: conn: %d, recovered #%d\n", conn->id, (int)sc.id);
		ib_buffer = (CRIBBuffer*)(ib_conn->params.s_buffers[sc.id].send_buf);
	}
	
	/*crWarning("IBAlloc returning 0x%x", (unsigned int)(ib_buffer+1));*/
	/*crWarning("IBAlloc returning buffer %d", ib_buffer->id);*/
	return (void*)(ib_buffer + 1);
}

static void
crIBSendMulti( CRConnection *conn, const void *buf, unsigned int len )
{
	unsigned char *src;
	CRASSERT( buf != NULL && len > 0 );

	if ( len <= conn->buffer_size )
	{
		/* the user is doing a send from memory not allocated by the
		 * network layer, but it does fit within a single message, so
		 * don't bother with fragmentation */
		/*crDebug("fast multi");*/
		void *pack = crIBAlloc( conn );
		crMemcpy( pack, buf, len );
		crIBSend( conn, &pack, pack, len );
		return;
	}
	
	src = (unsigned char *) buf;

	while ( len > 0 )
	{
		CRMessageMulti *msg = (CRMessageMulti *) crIBAlloc( conn );
		int id = ((CRIBBuffer*)msg - 1)->id;
		unsigned int        n_bytes;
		char string[128];
		crDebug("IB: slow multi, len: %d", len);
		
		if ( len + sizeof(*msg) > conn->buffer_size )
		{
			msg->header.type = CR_MESSAGE_MULTI_BODY;
			crDebug("IB: MULTI_BODY");
			msg->header.conn_id = conn->id;
			n_bytes   = conn->buffer_size - sizeof(*msg);
		}
		else
		{
			msg->header.type = CR_MESSAGE_MULTI_TAIL;
			crDebug("IB: MULTI_TAIL");
			msg->header.conn_id = conn->id;
			n_bytes   = len;
		}
		crMemcpy( msg + 1, src, n_bytes );
		crBytesToString( string, sizeof(string), msg, len );
		crDebug( "IB: IBSendMulti: sending message: "
			 "buf=[%s]", string );
		cr_ib_send( conn, msg, n_bytes + sizeof(*msg), id);
		
		src += n_bytes;
		len -= n_bytes;
	}
}

void
crIBSend( CRConnection *conn, void **bufp, 
	  const void *start, unsigned int len )
{
	CRIBBuffer *ib_buffer;
	
	if ( bufp == NULL )
	{
		crIBSendMulti( conn, start, len );
		return;
	}
	
	ib_buffer = (CRIBBuffer *) (*bufp) - 1;
	
	CRASSERT( ib_buffer->magic == CR_IB_BUFFER_MAGIC );
	
	cr_ib_send( conn, start, len, ib_buffer->id );
	
	*bufp = NULL;
}

static void
crIBFree( CRConnection *conn, void *buf )
{
	CRIBBuffer *ib_buffer = (CRIBBuffer *) buf - 1;	
	VAPI_rr_desc_t      rr;
	VAPI_sg_lst_entry_t sg_entry_r;
	VAPI_ret_t          res;
	VAPI_hca_hndl_t     hca_hndl    = cr_ib.hca_hndl;
	VAPI_qp_hndl_t      qp_hndl;
	CRIBConnection *ib_conn = crIBConnectionLookup(conn->id);
	qp_hndl = ib_conn->params.qp_hndl;

	CRASSERT( ib_buffer->magic == CR_IB_BUFFER_MAGIC );

	switch ( ib_buffer->kind )
	{
		case CRIBMemory:
			/* We can repost this buffer, excellent	*/
			rr.opcode     = VAPI_RECEIVE;
			rr.comp_type  = VAPI_SIGNALED;
			rr.sg_lst_len = 1;
			rr.sg_lst_p   = &sg_entry_r;
			rr.id = ib_buffer->id;
			/*crDebug("Freeing %d", ib_buffer->id);*/
			sg_entry_r.lkey = ib_conn->params.r_buffers[rr.id].recv_lkey;
			sg_entry_r.len  = cr_ib.size;
			sg_entry_r.addr = (VAPI_virt_addr_t)(MT_virt_addr_t)(ib_conn->params.r_buffers[rr.id].recv_buf+sizeof(CRIBBuffer));
			res = VAPI_post_rr(hca_hndl, qp_hndl, &rr);
			if (res != VAPI_OK) {
				crWarning("IB: Error: Re-posting Recv buffer: %s", 
				       VAPI_strerror(res));
			}
			break;

		case CRIBMemoryBig:
			crDebug("IB: Big");
			crFree( ib_buffer );
			break;

		default:
			crError( "IB: Weird buffer kind trying to free in crIBFree: %d", ib_buffer->kind );
	}
}

static void
crIBSingleRecv( CRConnection *conn, void *buf, unsigned int len )
{
	crWarning("IB: IBSingleRecv should NEVER be called according to net.c:~683");
}

int
crIBRecv( void )
{
	CRMessage *msg;
	int i;
	int found;
	/*char string[128];*/

	if (!cr_ib.num_conns)
	{
		crError("IB: crIBRecv got called but I don't have any IB connections yet, WTF?");
		return 0;
	}
	
	found = 0;
	for ( i = 0; i < cr_ib.num_conns; i++ )
	{
		CRIBBuffer *ib_buffer = NULL;
		unsigned int   len;
		CRConnection  *conn = cr_ib.conns[i];

		/*fprintf(stderr, ".");*/

		ib_buffer = cr_ib_recv( conn, &len );
		
		if( len > 0 ){
			ib_buffer->len = len;
			
			msg = (CRMessage *) (ib_buffer + 1);	
			
			/* 	crBytesToString( string, sizeof(string), msg, len ); */
/* 			crDebug("RECV buf=[%s]", string); */
			
			crNetDispatchMessage( cr_ib.recv_list, conn, msg, len );	
			
			/* CR_MESSAGE_OPCODES is freed in
			 * crserverlib/server_stream.c 
			 *
			 * OOB messages are the programmer's problem.  -- Humper 12/17/01 */
			if (msg->header.type != CR_MESSAGE_OPCODES && msg->header.type != CR_MESSAGE_OOB)
			{
				crIBFree( conn, ib_buffer + 1 );
			}
			found++;
		}
	}
	/*fprintf(stderr, "\n");*/
	return found;
}

static void
crIBHandleNewMessage( CRConnection *conn, CRMessage *msg,
		unsigned int len )
{	
	char string[128];

	CRIBBuffer *ib_buffer = ((CRIBBuffer *) msg) - 1;
	
	/* build a header so we can delete the message later */
	ib_buffer->magic = CR_IB_BUFFER_MAGIC;
	ib_buffer->kind  = CRIBMemoryBig;
	ib_buffer->len   = len;
	
	crBytesToString( string, sizeof(string), msg, len );
	crDebug("IB: HandleNewMessage buf=[%s]", string);

	crNetDispatchMessage( cr_ib.recv_list, conn, msg, len );
}

static void
crIBInstantReclaim( CRConnection *conn, CRMessage *mess )
{
	crIBFree( conn, mess );
}

void
crIBInit( CRNetReceiveFuncList *rfl, CRNetCloseFuncList *cfl, unsigned int mtu )
{
	cr_ib.recv_list = rfl;
	cr_ib.close_list = cfl;

	if (cr_ib.initialized)
	{
		crWarning("#################################");
		crWarning("# IB layer already initialized! #");
		crWarning("#################################");
		return;
	}

	cr_ib.initialized = 1; 
	cr_ib.num_conns = 0;
	cr_ib.conns     = NULL;
	
#ifdef CHROMIUM_THREADSAFE
	crInitMutex(&cr_ib.mutex);
#endif

	/* ignore the wizard behind the curtain */
	strcpy(cr_ib.hca_id,"InfiniHost0");	
	cr_ib.num_ah          = 256;
	cr_ib.size            = mtu;
	cr_ib.mtu             = MTU1024;
	cr_ib.num_ssge        = 10; /* To support inline data */
	cr_ib.num_rsge        = 10; /* To support inline data */
	cr_ib.alloc_aligned   = 1;
	cr_ib.num_cqe         = MAX_BUFS+2;
	cr_ib.num_r_wqe       = MAX_BUFS;  
	cr_ib.num_s_wqe       = MAX_BUFS;
	
	open_hca();
}

static int
crIBDoConnect( CRConnection *conn )
{	
     CRConnection *mother;
     char response[8096];
     char tmp_msg[8096];
     int remote_endianness;
     int i;
     CRIBConnection *ib_conn;

     crWarning( "crIBDoConnect is being called -- brokering the connection through the mothership!." );
     
     mother = __copy_of_crMothershipConnect( );	

     ib_conn = (CRIBConnection*)crAlloc( sizeof(*ib_conn) );

     /* initialize ib stuff */
     /* Initialize to default parameters */
     init_params(&ib_conn->params); 	

     ib_conn->params.total_sends = 0;
     
     /* Create IB reasources */
     if (RET_OK != create_common_resources(&ib_conn->params)) {
	     clean_up(&ib_conn->params);
     }
     
     if (RET_OK != qp_create(&ib_conn->params)) {
	     clean_up(&ib_conn->params);
     }

     sprintf(tmp_msg, "connectrequest ib %s %d %d %d lid_p1=%d qp_ous=%d qp=%d", 
	     conn->hostname,
	     conn->port, 
	     cr_ib.node_id,
	     conn->endianness, 
	     ib_conn->params.lid,
	     (int)MIN2(DFLT_QP_OUS_RD_ATOM, 
		       cr_ib.hca_cap.max_qp_ous_rd_atom), 
	     ib_conn->params.qp_num);
     
     /* Tell the mothership who I want to connect to, and what my IB info is */
     if (!__copy_of_crMothershipSendString( mother, response, tmp_msg ) )
     {
	     crError( "Mothership didn't like my connect request" );
     }
     
     sscanf( response, "%d %d %d", 
	     &(conn->id), 
	     &(conn->ib_node_id), 
	     &(remote_endianness) );
     /* The response will contain the IB information for the guy who accepted 
      * this connection.  The mothership will sit on the connectrequest 
      * until someone accepts. */
     msg2num(response, "lid_p1=", -1, 
	     &(ib_conn->params.d_lid), 
	     sizeof(ib_conn->params.d_lid));
     msg2num(response, "qp_ous=", 8, 
	     &(ib_conn->params.qp_ous_rd_atom), 
	     sizeof(ib_conn->params.qp_ous_rd_atom));
     msg2num(response, "qp=", -1, 
	     &(ib_conn->params.d_qpnum), 
	     sizeof(ib_conn->params.d_qpnum));
     
     /* NOW, we can add the connection, since we have enough information 
      * to uniquely determine the sender when we get a packet! */
     crIBConnectionAdd( conn, ib_conn );
     ib_conn = crIBConnectionLookup(conn->id);

     if (RET_OK != qp_init2rts_one_qp(&ib_conn->params)) {
	  clean_up(&ib_conn->params);
     }
     
     print_params( &ib_conn->params );

     /* setup all recv buffers */
     for(i=0; i<MAX_BUFS; i++){
	     CRIBBuffer *ib_buffer = (CRIBBuffer*)ib_conn->params.r_buffers[i].recv_buf;
	     ib_buffer->magic = CR_IB_BUFFER_MAGIC;
	     ib_buffer->kind = CRIBMemory;
	     ib_buffer->len = conn->buffer_size;
	     ib_buffer->id = i;
     }

     /* post all recv buffers */
     ib_post_recv_buffers( &ib_conn->params );

     /* and push all of the send buffers */
     ib_conn->send_pool = crBufferPoolInit(MAX_BUFS);
     for(i=0; i<MAX_BUFS; i++){
	     CRIBBuffer *ib_buffer = (CRIBBuffer*)ib_conn->params.s_buffers[i].send_buf;
	     ib_buffer->magic = CR_IB_BUFFER_MAGIC;
	     ib_buffer->kind = CRIBMemory;
	     ib_buffer->len = conn->buffer_size;
	     ib_buffer->id = i;
	     crBufferPoolPush( ib_conn->send_pool, ib_buffer, conn->buffer_size);
     } 
     crWarning("IB: conn: %d buffer count = %d", conn->id, crBufferPoolGetNumBuffers( ib_conn->send_pool));

     if (remote_endianness != conn->endianness)
     {
	     crError("IB: Why am I turning on swapping!?");
	     conn->swap = 1;
     }

     __copy_of_crMothershipDisconnect( mother );

     return 1;
}

static void
crIBDoDisconnect( CRConnection *conn )
{
	CRIBConnection* ib_conn;
	ib_conn = crIBConnectionLookup(conn->id);
	crWarning("IB: Disconnect being called!  Bad mojo?");

	crNetCallCloseCallbacks(conn);

	clean_up( &ib_conn->params );
}

void
crIBConnection( CRConnection *conn )
{
	int n_bytes, i, found = 0;

	crDebug("IB: crIBConnection");

	CRASSERT( cr_ib.initialized );

	conn->type  = CR_IB;
	conn->Alloc = crIBAlloc;
	conn->Send  = crIBSend;
	conn->SendExact  = crIBSendExact;
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
	crDebug("IB: num_conns = %d", cr_ib.num_conns);
}

CRConnection**
crIBDump( int* num )
{
	return NULL;
}

