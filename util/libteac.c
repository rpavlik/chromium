#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <string.h>
#include <time.h>

#include "teac.h"

#ifdef CHROMIUM
#include <cr_string.h>
#include <cr_mem.h>
#else
#define crAlloc(sz) malloc(sz)
#define crStrncpy(out,in,sz) strncpy(out,in,sz)
#define crStrchr(instr,inchr) strchr(instr,inchr)
#define crFree(ptr) free(ptr)
#define crRealloc( pp, size ) { \
  if (!(*pp=realloc(*pp,size))) { \
    fprintf(stderr,"failed to reallocate %d bytes!\n",size); \
    abort(); \
  } \
}
#endif

#define EADDR_ALLOC_ELAN            0x200000
#define ALLOC_ELAN_SIZE	            0x200000
#define EADDR_ALLOC_MAIN            0x400000
#define ALLOC_MAIN_SIZE	            0x2000000

/* Notes-
 * -I really need a better user key in the capability.
 * -Do I need an event to act as a spin lock after initialization?
 * -Why is elan3_fini dropping core?
 */

/* In the future we will use the bitwise AND of the rail masks */
#define RAIL 0

#ifndef HOST_TABLE_FILENAME
#define HOST_TABLE_FILENAME "/usr/users/8/welling/elanstuff/teac/teac_host_map.t"
#endif

#define INITIAL_HOST_TABLE_SIZE 256

static host_t* hosts= NULL;
static sdramaddr_t* sdramAddrBase= NULL;
static E3_Addr* elanAddrBase= NULL;

static int nodeTablesInitialized= 0;
static int nodeTableSize= 0;
static int nodeCount= 0;

static int read_node_map()
{
  FILE* f= NULL;
  int i;
  int iLine;
  char buf[256];

  if (hosts) crFree(hosts);
  if (!(hosts= (host_t*)crAlloc(INITIAL_HOST_TABLE_SIZE*sizeof(host_t)))) {
    fprintf(stderr,"libteac: read_node_map: unable to allocate %d bytes!\n",
	    INITIAL_HOST_TABLE_SIZE*sizeof(host_t));
    abort();
  }
  nodeTableSize= INITIAL_HOST_TABLE_SIZE;

  if (!(f=fopen(HOST_TABLE_FILENAME,"r"))) {
    fprintf(stderr,"libteac: read_node_map: cannot open <%s> for reading!\n",
	    HOST_TABLE_FILENAME);
    abort();
  }

  i= 0;
  iLine= 0;
  buf[sizeof(buf)-1]= '\0';
  while (!feof(f) && !ferror(f)) {
    char* tmp;
    char* string;
    if (!fgets(buf, sizeof(buf)-1, f)) break;
    iLine++;
    if (buf[0]=='#' || buf[0]=='\n' || buf[0]=='\0') continue;
    if (i>=nodeTableSize) {
      /* We need to grow the table */
      int newSize= 2*nodeTableSize;
      crRealloc((void**)&hosts,newSize*sizeof(host_t));
      nodeTableSize= newSize;      
    }
    if (!(string=strtok_r(buf," ,;\t\n",&tmp))) {
      fprintf(stderr,"libteac: read_node_map: Bad machine name at line %d of %s!\n",
	      iLine, HOST_TABLE_FILENAME);
      abort();
    }
    if (strlen(string)==0) continue; /* blank line */
    hosts[i].name= strdup(string);

    if (!(string=strtok_r(NULL," ,;\t\n",&tmp))) {
      fprintf(stderr,"libteac: read_node_map: bad integer string at line %d of %s!\n",
	      iLine, HOST_TABLE_FILENAME);
      abort();
    }
    errno= 0;
    hosts[i].railMask= atoi(string);
    if (errno != 0) {
      fprintf(stderr,"libteac: read_node_map: bad integer %s at %s line %d!\n",
	      string, HOST_TABLE_FILENAME, iLine);
      abort();
    }

    if (!(string=strtok_r(NULL," ,;\t\n",&tmp))) {
      fprintf(stderr,"libteac: read_node_map: bad integer string at line %d of %s!\n",
	      iLine, HOST_TABLE_FILENAME);
      abort();
    }
    errno= 0;
    hosts[i].id= atoi(string);
    if (errno != 0) {
      fprintf(stderr,"libteac: read_node_map: bad integer %s at %s line %d!\n",
	      string, HOST_TABLE_FILENAME, iLine);
      abort();
    }

    if (!(string=strtok_r(NULL," ,;\t\n",&tmp))) {
      fprintf(stderr,"libteac: read_node_map: bad hex value at line %d of %s!\n",
	      iLine, HOST_TABLE_FILENAME);
      abort();
    }
    errno= 0;
    hosts[i].sdramAddrBase= (sdramaddr_t)strtol(string, (char**)NULL, 0);
    if (errno != 0) {
      fprintf(stderr,"libteac: read_node_map: bad hex address %s at %s line %d!\n",
	      string, HOST_TABLE_FILENAME, iLine);
      abort();
    }

    if (!(string=strtok_r(NULL," ,;\t\n",&tmp))) {
      fprintf(stderr,"libteac: read_node_map: bad hex value at line %d of %s!\n",
	      iLine, HOST_TABLE_FILENAME);
      abort();
    }
    errno= 0;
    hosts[i].elanAddrBase= (E3_Addr)strtol(string, (char**)NULL, 0);
    if (errno != 0) {
      fprintf(stderr,"libteac: read_node_map: bad hex address %s at %s line %d!\n",
	      string, HOST_TABLE_FILENAME, iLine);
      abort();
    }

#if 0
    fprintf(stderr,"line %d: %d: got <%s> %d %d 0x%x 0x%x\n",
	    iLine, i, hosts[i].name, hosts[i].railMask, hosts[i].id, 
	    hosts[i].sdramAddrBase, hosts[i].elanAddrBase);
#endif

    i++;
  }
  if (ferror(f)) {
    perror("Error reading host table");
    fprintf(stderr,"libteac: read_node_map: error reading <%s>!\n",
	    HOST_TABLE_FILENAME);
    abort();
  }

  (void)fclose(f);
  return i;
}

static int hostnameCompare(const void* h1, const void* h2)
{
  return strcmp(((host_t*)h1)->name, ((host_t*)h2)->name);
}

static int hostnameLookup(const void* h1, const void* h2)
{
  return strcmp((const char*)h1, ((host_t*)h2)->name);
}

static void initialize_node_tables()
{
  if (!nodeTablesInitialized) {
    int nodeMin;
    int nodeMax;
    int nodeRange;
    int i;

    fprintf(stderr,"Loading Quadrics network map from <%s>\n",HOST_TABLE_FILENAME);

    /* Load information about Quadrics network */
    nodeCount= read_node_map();
    if (nodeCount==0) {
      fprintf(stderr,
	      "libteac: initialize_node_tables: no valid nodes in %s!\n",
	      HOST_TABLE_FILENAME);
    }

    /* 
     * Build the offset tables.  Offsets are looked up by node ID, so we
     * have to avoid redundant IDs and order the tables correctly.
     */
    nodeMin= nodeMax= hosts[0].id;
    for (i=1; i<nodeCount; i++) {
      if (hosts[i].id<nodeMin) nodeMin= hosts[i].id;
      if (hosts[i].id>nodeMax) nodeMax= hosts[i].id;
    }
    nodeRange= (nodeMax-nodeMin) + 1;
    if (!(sdramAddrBase=(sdramaddr_t*)crAlloc(nodeRange*sizeof(sdramaddr_t)))) {
      fprintf(stderr,
	    "libteac: initialize_node_tables: unable to allocate %d bytes!\n",
	      nodeRange*sizeof(sdramaddr_t));
      abort();
    }
    if (!(elanAddrBase=(E3_Addr*)crAlloc(nodeRange*sizeof(E3_Addr)))) {
      fprintf(stderr,
	    "libteac: initialize_node_tables: unable to allocate %d bytes!\n",
	      nodeRange*sizeof(E3_Addr));
      abort();
    }
    for (i=0; i<nodeRange; i++) {
      sdramAddrBase[i]= (sdramaddr_t)0;
      elanAddrBase[i]= (E3_Addr)0;
    }
    for (i=0; i<nodeCount; i++) {
      sdramAddrBase[hosts[i].id]= hosts[i].sdramAddrBase;
      elanAddrBase[hosts[i].id]= hosts[i].elanAddrBase;
    }

    /* Sort the host table alphabetically by host name for faster lookup */
    qsort( hosts, nodeCount, sizeof(host_t), hostnameCompare );

    nodeTablesInitialized= 1;
  }
}

static int trans_host(const char *hn)  {
  host_t* h= NULL;

  if (!nodeTablesInitialized) initialize_node_tables();

#if 0
  char buf[128];
  int i;
  i= 0;
  while (i<sizeof(buf)-1 && hn[i] && hn[i]!='.') { buf[i]= hn[i]; i++; };
  buf[i]= '\0';
#endif

  h=(host_t*)bsearch(hn, hosts, nodeCount, sizeof(host_t), hostnameLookup);
#if 0
  if (h) fprintf(stderr,"Lookup <%s> got <%s> <%d> <%x> <%x>\n",
		 hn,h->name,h->id,(int)h->sdramAddrBase,(int)h->elanAddrBase);
  else fprintf(stderr,"Lookup <%s> returned NULL!\n",hn);
#endif
  if (h) return h->id;
  else return -1;
}

/*
 * A version of address translation with some safety checks
 */
static E3_Addr teac_main2elan( ELAN3_CTX *ctx, void* main_addr )
{
  E3_Addr result= elan3_main2elan(ctx,main_addr);
  /*
  fprintf(stderr,"mapping %0lx -> %d; addressable %d\n",
	  main_addr,result,elan3_addressable(ctx,main_addr,64));
  */
  if (result==ELAN_BAD_ADDR) {
    fprintf(stderr,"Address translation error: %0x has no elan equivalent\n",
	    (int)main_addr);
    exit(-1);
  }
  return result;
}

Tcomm *teac_Init(char *lh, char *hh, int lctx, int hctx, int myrank)
{
  ELAN3_DEVINFO	info;
  ELAN_CAPABILITY *cap;
  Tcomm* result= NULL;
  int i;
  int j;
  int a;
  int b;
  char buf[256];
  char* here;
#ifdef __linux__
#ifdef ELAN_PRE_KITE
  void* control;
#endif
#endif
#if 0
static char junkString[256];
#endif

  if (!nodeTablesInitialized) initialize_node_tables();

  if (!(result= (Tcomm*)crAlloc(sizeof(Tcomm)))) {
    fprintf(stderr,"teac_Init: unable to allocate %d bytes!\n",
	    sizeof(Tcomm));
    return(NULL);
  }
  result->ctx = NULL;
  result->dma = NULL;
  result->e_dma = 0;
  result->s_event = 0;
  result->r_event = NULL;
  for (i=0; i<NUM_SEND_BUFFERS; i++) result->sbuf_pull_event[i] = 0;
  result->rbuf_pull_event = 0;
  result->m_snd = NULL;
  result->m_rcv = NULL;
  for (i=0; i<NUM_SEND_BUFFERS; i++) result->sbuf_ready[i]= NULL;
  result->rbuf_ready = NULL;
  result->mbuff= NULL;
  for (i=0; i<NUM_SEND_BUFFERS; i++) result->sendWrappers[i]= NULL;
  result->vp = -1;
  result->hhost = result->lhost = -1;
  result->hctx = result->lctx = -1;
  result->msgnum = 0;
  result->poll_shift = 0;
      
  a = trans_host(lh);
  b = trans_host(hh);
  result->lhost = (a < b)? a : b;
  result->hhost = (a > b) ? a : b;
  
  result->lctx = (lctx<hctx) ? lctx : hctx;
  result->hctx = (hctx>lctx) ? hctx : lctx;
  
  cap = &(result->cap);
  elan3_nullcap(cap);
  
  /* Initialize the UserKey to a random number */
  crStrncpy((char*)&(cap->UserKey), "This is pretty random!", 
	  sizeof(ELAN_USERKEY));
  cap->LowContext = lctx;
  cap->MyContext = lctx + (myrank%4);
  cap->HighContext = hctx;
  cap->LowNode = result->lhost;
  cap->HighNode = result->hhost;
  cap->Entries = (hctx - lctx + 1) * (cap->HighNode - cap->LowNode + 1);
  cap->Type = 
    ELAN_CAP_TYPE_BLOCK | ELAN_CAP_TYPE_NO_BITMAP | ELAN_CAP_TYPE_MULTI_RAIL;
  
  if ((result->ctx = elan3_init( 0, EADDR_ALLOC_MAIN, ALLOC_MAIN_SIZE, 
				 EADDR_ALLOC_ELAN, ALLOC_ELAN_SIZE))
      == NULL)  {
    fprintf(stderr, "teac_Init: elan3_init returned NULL context\n");
    return(NULL);
  }
  elan3_block_inputter (result->ctx, 1);
  
#ifdef __linux__
#ifdef ELAN_PRE_KITE
  if ((control = elan3_control_open (RAIL)) != NULL)  {
    if (elan3_create(control, &(result->cap)))  {
      fprintf(stderr, "elan3_create failed with <%s>, but that's OK!\n",
	      strerror(errno));
      errno= 0;
    }
  }  
  else  {
    perror("elan3_control_open failed");
    teac_Close(result);
    return NULL;
  }                                                                       
#else
  if (elan3_create(result->ctx, &(result->cap)))  {
    fprintf(stderr, "elan3_create failed with <%s>, but that's OK!\n",
	    strerror(errno));
    errno= 0;
  }
#endif
#else
  if (elan3_create(result->ctx, &(result->cap)))  {
    fprintf(stderr, "elan3_create failed with <%s>, but that's OK!\n",
	    strerror(errno));
    errno= 0;
  }
#endif

  elan3_devinfo(0, &info);
  /*
    The above call for rail '0' yields the following info:
    - info.NodeId
    - info.NumLevels
    - info.NodeLevel
  */

#if 0
#ifdef __linux__
#ifdef ELAN_PRE_KITE
  fprintf(stdout, "NodeId: %d, NumLevels: %d, NodeLevel: %d\n",
	  info.NodeId, info.NumLevels, info.NodeLevel);
#else
  fprintf(stdout, "NodeId: %d, NumLevels: %d, NodeLevel: %d\n",
	  info.Position.NodeId, info.Position.NumLevels, 
	  info.Position.NodeLevel);
#endif
#else
  fprintf(stdout, "NodeId: %d, NumLevels: %d, NodeLevel: %d\n",
	  info.NodeId, info.NumLevels, info.NodeLevel);
#endif

  fprintf(stderr,"Capability: <%s>\n",
          elan3_capability_string(&(result->cap),junkString));
  fprintf(stderr,"railmask is %d\n",result->cap.RailMask);
  fprintf(stderr,"bitmap is %x\n",result->cap.Bitmap[0]);

#endif

  /* Reality check. */
  if (gethostname(buf,sizeof(buf)-1)) {
    perror("Can't get my own host name");
    teac_Close(result);
    return NULL;
  }
  if ((here= crStrchr(buf,'.')) != NULL) *here= '\0';
#ifdef __linux__
#ifdef ELAN_PRE_KITE
  if (trans_host(buf) != info.NodeId) {
    fprintf(stderr,
 "teac_Init: compiled-in Quadrics port id %d does not match real value %d!\n",
	    trans_host(buf), info.NodeId);
    teac_Close(result);
    return NULL;
  }
#else
  if (trans_host(buf) != info.Position.NodeId) {
    fprintf(stderr,
 "teac_Init: compiled-in Quadrics port id %d does not match real value %d!\n",
	    trans_host(buf), info.Position.NodeId);
    teac_Close(result);
    return NULL;
  }
#endif
#else
  if (trans_host(buf) != info.NodeId) {
    fprintf(stderr,
 "teac_Init: compiled-in Quadrics port id %d does not match real value %d!\n",
	    trans_host(buf), info.NodeId);
    teac_Close(result);
    return NULL;
  }
#endif

  if (elan3_attach(result->ctx, &(result->cap)))  {
    fprintf(stderr, "teac_Init: elan3_attach failed\n");
    teac_Close(result);
    return(NULL);
  }

  if (elan3_addvp(result->ctx, 0, &(result->cap)))  {
    fprintf(stderr, "teac_Init: elan3_addvp failed\n");
    teac_Close(result);
    return(NULL);
  }
  
  result->vp = elan3_process(result->ctx);
  
  if (! elan3_set_required_mappings (result->ctx))  {
    fprintf(stderr, "teac_Init: elan3_set_required_mappings failed\n");
    teac_Close(result);
    return(NULL);
  }
  
  if (!(result->r_event= 
	(sdramaddr_t**)crAlloc( cap->Entries*sizeof(sdramaddr_t*) ))) {
    fprintf(stderr,"teac_Init: unable to allocate %d bytes!\n",
	    cap->Entries*sizeof(sdramaddr_t*));
    teac_Close(result);
    return(NULL);
  }
  if (!(result->r_event[0]=
	(sdramaddr_t*)crAlloc( cap->Entries*NUM_SEND_BUFFERS
			      * sizeof(sdramaddr_t) ))) {
    fprintf(stderr,"teac_Init: unable to allocate %d bytes!\n",
	    cap->Entries*NUM_SEND_BUFFERS*sizeof(sdramaddr_t));
    teac_Close(result);
    return(NULL);
  }
  if (!(result->r_event[0][0]=
	elan3_allocElan(result->ctx, E3_EVENT_ALIGN, 
		cap->Entries*NUM_SEND_BUFFERS*sizeof(E3_BlockCopyEvent)))) {
    perror("teac_Init: elan3_allocElan failed for r_event block");
    teac_Close(result);
    return(NULL);
  }
  for (j=1; j<cap->Entries; j++) {
    result->r_event[j]= result->r_event[0] + j*NUM_SEND_BUFFERS;
    result->r_event[j][0]= 
      result->r_event[0][0]+j*NUM_SEND_BUFFERS*sizeof(E3_BlockCopyEvent);
  }
  for (j=0; j<cap->Entries; j++) 
    for (i=1; i<NUM_SEND_BUFFERS; i++) {
      result->r_event[j][i]= 
	result->r_event[j][0] + i*sizeof(E3_BlockCopyEvent);
    }
#if 0
  fprintf(stderr,"r_event[0][0] is %x\n",(int)result->r_event[0][0]);
#endif

  if (!(result->m_rcv= 
	(volatile E3_uint32**)crAlloc( cap->Entries*sizeof(E3_uint32*) ))) {
    fprintf(stderr,"teac_Init: unable to allocate %d bytes!\n",
	    cap->Entries*sizeof(E3_uint32*));
    teac_Close(result);
    return(NULL);
  }
  if (!(result->m_rcv[0]= (volatile E3_uint32*)
	elan3_allocMain(result->ctx, 0, 
			cap->Entries*NUM_SEND_BUFFERS*sizeof(E3_uint32)))) {
    perror("teac_Init: elan3_allocMain failed for m_rcv block");
    teac_Close(result);
    return(NULL);
  }
  for (i=1; i<cap->Entries; i++)
    result->m_rcv[i]= result->m_rcv[0] + i*NUM_SEND_BUFFERS;
#if 0
  fprintf(stderr,"Base of m_rcv is %lx -> %lx\n",
	  (long)(result->m_rcv[0]),
	  (long)teac_main2elan(result->ctx,(void*)(result->m_rcv[0])));
#endif
  
  if (!(result->mbuff= (teacMsg**)crAlloc( cap->Entries*sizeof(teacMsg*) ))) {
    fprintf(stderr,"teac_Init: unable to allocate %d bytes!\n",
	    cap->Entries*sizeof(teacMsg*));
    teac_Close(result);
    return(NULL);
  }
  if (!(result->mbuff[0]= (teacMsg*)
	elan3_allocMain(result->ctx, 8, 
			cap->Entries*NUM_SEND_BUFFERS*sizeof(teacMsg)))) {
    perror("teac_Init: elan3_allocMain failed for mbuff block");
    teac_Close(result);
    return(NULL);
  }
  for (i=1; i<cap->Entries; i++)
    result->mbuff[i]= result->mbuff[0] + i*NUM_SEND_BUFFERS;
#if 0
  fprintf(stderr,"Base of mbuff is %lx -> %lx\n",
	  (long)(result->mbuff[0]), 
	  (long)teac_main2elan(result->ctx,result->mbuff[0]));
#endif
  
  if (!(result->dma= (E3_DMA_MAIN *)
	elan3_allocMain(result->ctx, 
			E3_DMA_ALIGN, sizeof(E3_DMA_MAIN)))) {
    perror("teac_Init: elan3_allocMain failed for dma");
    teac_Close(result);
    return(NULL);
  }
  
  if (!(result->e_dma= 
	elan3_allocElan(result->ctx, E3_DMA_ALIGN, sizeof(E3_DMA)))) {
    perror("teac_Init: elan3_allocElan failed for e_dma");
    teac_Close(result);
    return(NULL);
  }
  
  if (!(result->s_event= 
	elan3_allocElan(result->ctx, E3_EVENT_ALIGN, 
			sizeof(E3_BlockCopyEvent)))) {
    perror("teac_Init: elan3_allocElan failed for s_event");
    teac_Close(result);
    return(NULL);
  }
#if 0
  fprintf(stderr,"s_event is %x\n",(int)result->s_event);
#endif
  
  if (!(result->sbuf_pull_event[0]= 
	elan3_allocElan(result->ctx, E3_EVENT_ALIGN, 
			NUM_SEND_BUFFERS*sizeof(E3_BlockCopyEvent)))) {
    perror("teac_Init: elan3_allocElan failed for sbuf_pull_event block");
    teac_Close(result);
    return(NULL);
  }
  for (i=1; i<NUM_SEND_BUFFERS; i++) 
    result->sbuf_pull_event[i]= 
      result->sbuf_pull_event[0]+i*sizeof(E3_BlockCopyEvent);
  
  if (!(result->rbuf_pull_event= 
	elan3_allocElan(result->ctx, E3_EVENT_ALIGN, 
			sizeof(E3_BlockCopyEvent)))) {
    perror("teac_Init: elan3_allocElan failed for rbuf_pull_event");
    teac_Close(result);
    return(NULL);
  }
  
  if (!(result->m_snd= (E3_uint32 *)
	elan3_allocMain(result->ctx, 0, 
			sizeof(E3_uint32)))) {
    perror("teac_Init: elan3_allocMain failed for m_snd");
    teac_Close(result);
    return(NULL);
  }

  if (!(result->sbuf_ready[0]= (E3_uint32 *)
	elan3_allocMain(result->ctx, 0, 
			NUM_SEND_BUFFERS*sizeof(E3_uint32)))) {
    perror("teac_Init: elan3_allocMain failed for sbuf_ready block");
    teac_Close(result);
    return(NULL);
  }
  for (i=1; i<NUM_SEND_BUFFERS; i++)
    result->sbuf_ready[i]= result->sbuf_ready[0] + i;
  
  if (!(result->sendWrappers[0]= 
	(SBuffer*)crAlloc(NUM_SEND_BUFFERS*sizeof(SBuffer)))) {
    fprintf(stderr,"teac_Init: unable to allocate %d bytes!\n",
	    NUM_SEND_BUFFERS*sizeof(SBuffer));
    teac_Close(result);
    return(NULL);
  }
  for (i=1; i<NUM_SEND_BUFFERS; i++)
    result->sendWrappers[i]= result->sendWrappers[0] + i;

  if (!(result->rbuf_ready= (E3_uint32 *)
	elan3_allocMain(result->ctx, 0, 
			sizeof(E3_uint32)))) {
    perror("teac_Init: elan3_allocMain failed for rbuf_ready");
    teac_Close(result);
    return(NULL);
  }

  for (i=0; i<NUM_SEND_BUFFERS; i++) {
    char* buf;
    if (!(buf= (char*)elan3_allocMain(result->ctx, 8, 
				      E_BUFFER_INITIAL_SIZE))) {
      perror("teac_Init: elan3_allocMain failed for buffer block");
      teac_Close(result);
      return(NULL);
    }
    result->sendWrappers[i]->bufId= i;
    result->sendWrappers[i]->totSize= E_BUFFER_INITIAL_SIZE;
    result->sendWrappers[i]->validSize= 0;
    result->sendWrappers[i]->buf= buf;
  }
  
  elan3_initevent_word (result->ctx, 
			result->s_event, result->m_snd);
  elan3_initevent_word (result->ctx, 
			result->rbuf_pull_event, result->rbuf_ready);
  
  for (i=0; i<NUM_SEND_BUFFERS; i++) {
    elan3_initevent_word (result->ctx, 
			  result->sbuf_pull_event[i], result->sbuf_ready[i]);
  }

  for (j=0; j<cap->Entries; j++) 
    for (i=0; i<NUM_SEND_BUFFERS; i++) {
      elan3_initevent_word (result->ctx, 
			    result->r_event[j][i], &(result->m_rcv[j][i]));
    }
  
  /* Get the message receive events ready to fire, in case something
   * comes in before receive gets called.
   */
  for (j=0; j<cap->Entries; j++)
    for (i=0; i<NUM_SEND_BUFFERS; i++) {
      elan3_primeevent(result->ctx, result->r_event[j][i], 1);
    }

  /* Fire the sbuffer free events, so that the buffers look free when 
   * the first call to send happens.
   */
  for (i=0; i<NUM_SEND_BUFFERS; i++) {
    elan3_primeevent(result->ctx, result->sbuf_pull_event[i], 1);
    elan3_setevent( result->ctx, result->sbuf_pull_event[i] );
  }
  
  /* And now we're ready to face the world. */
  elan3_block_inputter (result->ctx, 0);
  
  return(result);
}
  
void teac_Close(Tcomm *tcomm)
{
  int i;
#if 0
  char buf[256];
#endif

  if (tcomm) {
    /* First we have to wait until all pending messages have been
     * pulled (assuming they got initialized in the first place).
     */
    if ((tcomm->sbuf_pull_event[0] != 0) && (tcomm->sbuf_ready[0] != NULL)) {
      for (i=0; i<NUM_SEND_BUFFERS; i++) {
	elan3_waitevent_word(tcomm->ctx, tcomm->sbuf_pull_event[i],
			     tcomm->sbuf_ready[i], 10);
      }
      fprintf(stderr,"All TEAC messages have reported home!\n");
    }
    elan3_block_inputter (tcomm->ctx, 1);

    if (tcomm->e_dma != 0) elan3_freeElan(tcomm->ctx, tcomm->e_dma);
    if (tcomm->s_event != 0) elan3_freeElan(tcomm->ctx, tcomm->s_event);
    if (tcomm->r_event != NULL) {
      if (tcomm->r_event[0] != NULL) {
	if (tcomm->r_event[0][0] != 0) elan3_freeElan(tcomm->ctx, 
						      tcomm->r_event[0][0]);
	crFree(tcomm->r_event[0]);
      }
      crFree(tcomm->r_event);
    }
    if (tcomm->sbuf_pull_event[0] != 0) 
      elan3_freeElan(tcomm->ctx, tcomm->sbuf_pull_event[0]);
    if (tcomm->sendWrappers[0] != NULL) {
      for (i=0; i<NUM_SEND_BUFFERS; i++) {
	if (tcomm->sendWrappers[i]->buf != NULL) 
	  elan3_free(tcomm->ctx, tcomm->sendWrappers[i]->buf);
      }
      crFree(tcomm->sendWrappers[0]);
    }
    if (tcomm->rbuf_pull_event != 0) 
      elan3_freeElan(tcomm->ctx, tcomm->rbuf_pull_event);
    if (tcomm->m_snd != NULL) elan3_free(tcomm->ctx, 
					 (E3_uint32*)tcomm->m_snd);
    if (tcomm->m_rcv != NULL) {
      if (tcomm->m_rcv[0] != NULL) elan3_free(tcomm->ctx,
					      (E3_uint32*)tcomm->m_rcv[0]);
      crFree(tcomm->m_rcv);
    }
    if (tcomm->sbuf_ready[0] != NULL) 
      elan3_free(tcomm->ctx, (E3_uint32*)tcomm->sbuf_ready[0]);
    if (tcomm->rbuf_ready != NULL) elan3_free(tcomm->ctx, 
					    (E3_uint32*)tcomm->rbuf_ready);
    if (tcomm->mbuff != NULL) {
      if (tcomm->mbuff[0] != NULL) elan3_free(tcomm->ctx, tcomm->mbuff[0]);
      crFree(tcomm->mbuff);
    }
#if 0
    elan3_detach(tcomm->ctx);
    fprintf(stderr,"tcomm string: <%s>\n",teac_getTcommString(tcomm,buf,256));
    elan3_fini(tcomm->ctx);
#endif
  }
}

int teac_Select(Tcomm* tcomm, int *ids, int num_ids, int timeout)
{
  int id;
  while (timeout-- > 0) {
    if ((id= teac_Poll(tcomm, ids, num_ids)) >= 0) return id;
  }

  return -1;
}

int teac_Poll(Tcomm* tcomm, int *ids, int num_ids)
{
  int i;
  int j;

  for (j=0; j<num_ids; j++) {
    int index= (j+tcomm->poll_shift) % num_ids;
    int thisId= ids[index];
    for (i=0; i<NUM_SEND_BUFFERS; i++) {
      if (elan3_pollevent_word(tcomm->ctx, &(tcomm->m_rcv[thisId][i]), 1)) {
	tcomm->poll_shift= index;
	return thisId;
      }
    }
  }
  
  return -1;
}

int teac_sendBufferAvailable(Tcomm* tcomm)
{
  int i;
  for (i=0; i<NUM_SEND_BUFFERS; i++) {
    if (elan3_pollevent_word(tcomm->ctx, tcomm->sbuf_ready[i], 1))
      return 1;
  }
  return 0;
}

SBuffer* teac_getSendBuffer( Tcomm* tcomm, long size )
{
  /* Find a free send buffer.  We'll busy wait in this poll loop
   * if necessary.
   */
  int i= 0;
  while (1) {
    if (elan3_pollevent_word(tcomm->ctx, tcomm->sbuf_ready[i],
			     1)) break;
    if (++i == NUM_SEND_BUFFERS) i= 0;
  }
  /* We will use this buffer! */
#if 0
  fprintf(stderr,"Allocated message buffer %d\n",i);
#endif
  *(tcomm->sbuf_ready[i])= 0; /* mark it busy */

  /* If the associated chunk of memory is smaller than that requested,
   * replace it with something larger.
   */
  if (tcomm->sendWrappers[i]->totSize < size) {
    elan3_free( tcomm->ctx, tcomm->sendWrappers[i]->buf );
    if (!(tcomm->sendWrappers[i]->buf= 
	  (char*)elan3_allocMain(tcomm->ctx, 8, size))) {
      perror("teac_getSendBuffer: failed to grow send buffer");
      exit(-1);
    }
  }
  tcomm->sendWrappers[i]->totSize= size;
  tcomm->sendWrappers[i]->validSize= 0;
  return tcomm->sendWrappers[i];
}

int teac_Send( Tcomm* tcomm, int* ids, int num_ids, SBuffer* buf, void *start )
{
  int	vp = tcomm->vp;
  int	iBuf;
  int   iDest;
  teacMsg *msg;

  /* Reality check: is this one of my buffers? */
  if (buf->bufId<0 || buf->bufId>=NUM_SEND_BUFFERS) {
    fprintf(stderr,"teac_Send: I don't know this buffer!\n");
    return 0;
  }

  /* Reality check: did they write too much into the message? */
  if (buf->validSize > buf->totSize) {
    fprintf(stderr,"teac_Send: message too large! (%ld vs %ld)\n",
	    buf->validSize, buf->totSize);
    abort();
    return 0;
  }

  iBuf= buf->bufId;
  msg = &(tcomm->mbuff[vp][iBuf]);
  msg->msgnum = tcomm->msgnum++;
  msg->size = buf->validSize;
  msg->host = vp;
  if ( start != NULL )
	  msg->mptr = teac_main2elan( tcomm->ctx, start );
  else
  	msg->mptr = teac_main2elan(tcomm->ctx, buf->buf);
  msg->clr_event = elan3_sdram2elan(tcomm->ctx, tcomm->ctx->sdram,
				    tcomm->sbuf_pull_event[iBuf]);
  msg->new= 1;
  
  /* Set up the parts of the DMA that are not specific to destination */
  tcomm->dma->dma_type = E3_DMA_TYPE(DMA_BYTE,DMA_WRITE,DMA_NORMAL,0);
  tcomm->dma->dma_size = sizeof(teacMsg);
  tcomm->dma->dma_srcEvent = 
    elan3_sdram2elan(tcomm->ctx,tcomm->ctx->sdram,tcomm->s_event);
  tcomm->dma->dma_source = teac_main2elan(tcomm->ctx, msg);
  elan3_primeevent(tcomm->ctx, tcomm->sbuf_pull_event[buf->bufId], num_ids);

  /* Send this message off to each destination */
  for (iDest=0; iDest<num_ids; iDest++) {
    tcomm->dma->dma_srcCookieVProc = 
      elan3_local_cookie(tcomm->ctx, vp, ids[iDest]);
    tcomm->dma->dma_destCookieVProc = ids[iDest];
    tcomm->dma->dma_destEvent = 
      elan3_sdram2elan(tcomm->ctx,tcomm->ctx->sdram,
		       tcomm->r_event[vp][iBuf])
      + sdramAddrBase[(ids[iDest]/NUM_SEND_BUFFERS) + tcomm->lhost] 
      - sdramAddrBase[(vp/NUM_SEND_BUFFERS) + tcomm->lhost];
    tcomm->dma->dma_dest = teac_main2elan(tcomm->ctx, msg)
      + elanAddrBase[(ids[iDest]/NUM_SEND_BUFFERS) + tcomm->lhost] 
      - elanAddrBase[(vp/NUM_SEND_BUFFERS) + tcomm->lhost];
    elan3_primeevent(tcomm->ctx, tcomm->s_event, 1);
    *(tcomm->m_snd)= 0;
    elan3_putdma_main(tcomm->ctx, tcomm->dma, tcomm->e_dma);
#if 0
    fprintf(stderr,"DMA dest event %x, dest mem %lx\n",
	    tcomm->dma->dma_destEvent, 
	    (long)tcomm->dma->dma_dest);
    fprintf(stderr,"Mem shifts are %x %x based on %d %d\n",
	    elanAddrBase[(ids[iDest]/NUM_SEND_BUFFERS) + tcomm->lhost],
	    elanAddrBase[(vp/NUM_SEND_BUFFERS) + tcomm->lhost],
	    ids[iDest],vp);
    fprintf(stderr,"Send msg %d in buffer %d to %d (list index %d)...",
	    msg->msgnum,iBuf, ids[iDest],iDest);
#endif
    elan3_waitevent_word(tcomm->ctx,
			 tcomm->s_event, tcomm->m_snd, ELAN_WAIT_EVENT);
#if 0
    fprintf(stderr,"message away!\n");
#endif
  }
  return 1;
}

RBuffer* teac_Recv(Tcomm* tcomm, int id)
{
  int	vp = tcomm->vp;
  RBuffer* result= NULL;
  int iEvent= 0;
  int iBuf= 0;
  int i;
  int lowestMsgnum;
  
  /* poll until we get an incoming message from the given sender */
  while (1) {
    if (elan3_pollevent_word(tcomm->ctx, &(tcomm->m_rcv[id][iEvent]), 1)) 
      break;
    if (++iEvent == NUM_SEND_BUFFERS) iEvent= 0;
  }

  /* This may not be the earliest event, however. */
  lowestMsgnum= -1;
  for (i=0; i<NUM_SEND_BUFFERS; i++) {
#if 0
    fprintf(stderr,"Testing for new msg at %lx -> %lx\n",
	    (long)&(tcomm->mbuff[id][i]), 
	    (long)teac_main2elan(tcomm->ctx,(void*)(&tcomm->mbuff[id][i])));
#endif
    if (tcomm->mbuff[id][i].new) {
      if ((lowestMsgnum < 0) 
	  || (tcomm->mbuff[id][i].msgnum < lowestMsgnum)) {
	lowestMsgnum= tcomm->mbuff[id][i].msgnum;
	iBuf= i;
      }
    }
  }
  if (lowestMsgnum<0) {
    fprintf(stderr,
	    "teac_Recv: internal error: can't find message I just got!\n");
    return NULL;
  }
  tcomm->mbuff[id][iBuf].new= 0; 
  tcomm->m_rcv[id][iBuf]= 0;
  elan3_primeevent(tcomm->ctx, tcomm->r_event[id][iBuf],1);
#if 0
  fprintf(stderr,"got msg %d in buffer %d from %d!\n",
	  tcomm->mbuff[id][iBuf].msgnum, iBuf, id);
#endif

  /* Make some space to put the message when it arrives */
  if (!(result= (RBuffer*)crAlloc(sizeof(RBuffer)))) {
    fprintf(stderr,"teac_Recv: unable to allocate %d bytes!\n",
	    sizeof(RBuffer));
    return NULL;
  }
#if 0
  if (!(result->buf= (void*)elan3_allocMain(tcomm->ctx, 8,E_BUFFER_SIZE))) {
    perror("teac_Recv: elan3_allocMain failed for buffer");
    return(NULL);
  }
  result->totSize= E_BUFFER_SIZE;
#endif
  if (!(result->buf= (void*)elan3_allocMain(tcomm->ctx, 8, 
					    tcomm->mbuff[id][iBuf].size))) {
    perror("teac_Recv: elan3_allocMain failed for buffer");
    return(NULL);
  }
  result->totSize= tcomm->mbuff[id][iBuf].size;
  result->validSize= tcomm->mbuff[id][iBuf].size;
  result->from= tcomm->mbuff[id][iBuf].host;
  result->senderMsgnum= tcomm->mbuff[id][iBuf].msgnum;
		
  /* Set up the DMA to pull the message */
  tcomm->dma->dma_type = E3_DMA_TYPE(DMA_BYTE,DMA_READ,DMA_NORMAL,0);
  tcomm->dma->dma_size = tcomm->mbuff[id][iBuf].size;
  tcomm->dma->dma_source = tcomm->mbuff[id][iBuf].mptr;
  tcomm->dma->dma_dest = teac_main2elan(tcomm->ctx,result->buf);
  tcomm->dma->dma_srcCookieVProc = 
    elan3_remote_cookie(tcomm->ctx, vp, tcomm->mbuff[id][iBuf].host);
  tcomm->dma->dma_destCookieVProc = 
    elan3_local_cookie(tcomm->ctx, vp, tcomm->mbuff[id][iBuf].host);
  tcomm->dma->dma_srcEvent = tcomm->mbuff[id][iBuf].clr_event; 
  tcomm->dma->dma_destEvent = 
    elan3_sdram2elan(tcomm->ctx,tcomm->ctx->sdram,
		     tcomm->rbuf_pull_event);

  
  elan3_primeevent(tcomm->ctx, tcomm->rbuf_pull_event,1);
  *(tcomm->rbuf_ready)= 0;
  elan3_getdma_main(tcomm->ctx, tcomm->dma, tcomm->e_dma);
  elan3_waitevent_word(tcomm->ctx,
		       tcomm->rbuf_pull_event, 
		       tcomm->rbuf_ready, ELAN_WAIT_EVENT);

  return(result);
}

int teac_Dispose( Tcomm* tcomm, RBuffer* buf )
{
  elan3_free(tcomm->ctx, buf->buf);
  crFree(buf);
  return 0;
}

char* teac_getTcommString(Tcomm *c, char* buf, int buflen)
{
  snprintf(buf,buflen-1,
	   "<vp= %d, host range %d-%d, ctxnum range %d-%d, msg %d>",
	   c->vp,c->lhost,c->hhost,c->lctx,c->hctx,c->msgnum);
  buf[buflen-1]= '\0';
  return buf;
}

char* teac_getConnString(Tcomm *c, int id, char* buf, int buflen)
{
  int rel_rank= id%4;
  int node= ((id-rel_rank)/4) + c->lhost;
  snprintf(buf,buflen-1,"vp %d, <%s>:%d",id,hosts[node].name,rel_rank);
  buf[buflen-1]= '\0';
  return buf;
}

int teac_getConnId(Tcomm *c, const char* host, int rank)
{
  int node= trans_host(host);
#if 0
  fprintf(stderr,"getConnId: <%s> %d %d maps to %d %d\n",
	  host, rank, c->lhost, node, (4*(node - c->lhost) + (rank%4)));
#endif
  return (4*(node - c->lhost) + (rank%4));
}

int teac_getHostInfo(Tcomm *c, char* host, const int hostLength,
		     int* railMask, int *nodeId, 
		     long* sdramBaseAddr, long* elanBaseAddr)
{
  ELAN3_DEVINFO	info;
  char* here;

  if (gethostname(host,hostLength-1)) {
    perror("Can't get my own host name");
    return 0;
  }
  host[hostLength-1]= '\0';
  if ((here= crStrchr(host,'.')) != NULL) *here= '\0';

  elan3_devinfo(0, &info);
#ifdef __linux__
#ifdef ELAN_PRE_KITE
  *nodeId= info.NodeId;
#else
  *nodeId= info.Position.NodeId;
#endif
#else
  *nodeId= info.NodeId;
#endif
  *railMask= c->cap.RailMask;

  *sdramBaseAddr= (int)c->r_event[0][0];
	  
  *elanBaseAddr= (long)teac_main2elan(c->ctx,(void*)c->m_rcv[0]);
  return 1;
}
