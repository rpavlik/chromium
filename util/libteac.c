#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <string.h>

#include "teac.h"
#include "time.h"

/* Notes-
 * -Am I really using all the parts of Tcomm struct?
 * -Am I really using all the consts in teac.h?
 * -Should some of the constants move to libteac.c from teac.h?
 * -I really need a better user key in the capability.
 * -Do I need an event to act as a spin lock after initialization?
 * -Why is elan3_fini dropping core?
 */

#define RAIL 0

#ifdef never
static char junkString[256];
#endif

static host_t hosts[] = { 
  { "mini-t0", 0, 0 }, { "mini-t1", 0, 1 },
  { "mini-t2", 0, 2 }, { "mini-t3", 0, 3 }, 
  { "dummy4", 0, 4 }, { "dummy5", 0, 5 }, { "dummy6", 0, 6 },
  { "dummy7", 0, 7 }, { "dummy8", 0, 8 }, { "dummy9", 0, 9 },
  { "dummy10", 0, 10 }, { "dummy11", 0, 11 }, { "dummy12", 0, 12 },
  { "dummy13", 0, 13 }, { "dummy14", 0, 14 }, { "dummy15", 0, 15 },
  { "vis0", 0, 16 }, { "vis1", 0, 17 }, { "vis2", 0, 18 },
};

#ifdef never
static host_t hosts[] = { 
  { "mini-t0", 0, 0 }, { "mini-t1", 0, 1 },
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
  { "tcsini64", 0, 64 }, { "vis0", 0, 65 }, { "vis1", 0, 66 },
  { "vis2", 0, 67 } };
#endif

#define MAXHOSTS (sizeof(hosts)/sizeof(host_t))

static sdramaddr_t sdramAddrBase[]= { /* indexed by node number */
  0x14d00, /* mini-t0 */
  0x14d00, /* mini-t1 */
  0x14d00, /* mini-t2 */
  0x14d00, /* mini-t3 */
  0x00000, /* dummy4 */
  0x00000, /* dummy5 */
  0x00000, /* dummy6 */
  0x00000, /* dummy7 */
  0x00000, /* dummy8 */
  0x00000, /* dummy9 */
  0x00000, /* dummy10 */
  0x00000, /* dummy11 */
  0x00000, /* dummy12 */
  0x00000, /* dummy13 */
  0x00000, /* dummy14 */
  0x00000, /* dummy15 */
  0x13d00, /* vis0 */
  0x13d00, /* vis1 */
  0x13d00, /* vis2 */
};

static E3_Addr elanAddrBase[]= { /* indexed by node number */
  0x405400, /* mini-t0 */
  0x405400, /* mini-t1 */
  0x405400, /* mini-t2 */
  0x405400, /* mini-t3 */
  0x000000, /* dummy4 */
  0x000000, /* dummy5 */
  0x000000, /* dummy6 */
  0x000000, /* dummy7 */
  0x000000, /* dummy8 */
  0x000000, /* dummy9 */
  0x000000, /* dummy10 */
  0x000000, /* dummy11 */
  0x000000, /* dummy12 */
  0x000000, /* dummy13 */
  0x000000, /* dummy14 */
  0x000000, /* dummy15 */
  0x403400, /* vis0 */
  0x403400, /* vis1 */
  0x403400, /* vis2 */
};

#define MAXPORTS (sizeof(sdramAddrBase)/sizeof(sdramaddr_t))

static int trans_host(const char *hn)  {
  int	i;
  int   j;
  for (i=0; i<MAXHOSTS; i++)  {
    int match= 1;
    for (j=0; (hn[j]!='\0' && hn[j]!='.'); j++) {
      if (hn[j] != hosts[i].name[j]) {
	match= 0;
	break;
      }
    }
    if (match) return(hosts[i].id);
  }
  return(-1);
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
  void* control;
#endif

  if (!(result= (Tcomm*)malloc(sizeof(Tcomm)))) {
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
  
  bzero (&(cap->UserKey), sizeof(ELAN_USERKEY));
  strncpy((char*)&(cap->UserKey), "This is pretty random!", 
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

  elan3_devinfo(0, &info);
  /*
    The above call for rail '0' yields the following info:
    - info.NodeId
    - info.NumLevels
    - info.NodeLevel
  */

#ifdef ELAN_PRE_KITE
  fprintf(stdout, "NodeId: %d, NumLevels: %d, NodeLevel: %d\n",
	  info.NodeId, info.NumLevels, info.NodeLevel);
#else
  fprintf(stdout, "NodeId: %d, NumLevels: %d, NodeLevel: %d\n",
	  info.Position.NodeId, info.Position.NumLevels, 
	  info.Position.NodeLevel);
#endif

#ifdef never
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
  if ((here= strchr(buf,'.')) != NULL) *here= '\0';
#ifdef ELAN_PRE_KITE
  if (trans_host(buf) != info.NodeId) {
    fprintf(stderr,"teac_Init: compiled-in Quadrics port id %d does not match real value %d!\n",
	    trans_host(buf), info.NodeId);
    teac_Close(result);
    return NULL;
  }
#else
#ifdef never
  if (trans_host(buf) != info.Position.NodeId) {
    fprintf(stderr,"teac_Init: compiled-in Quadrics port id %d does not match real value %d!\n",
	    trans_host(buf), info.Position.NodeId);
    teac_Close(result);
    return NULL;
  }
#endif
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
	(sdramaddr_t**)malloc( cap->Entries*sizeof(sdramaddr_t*) ))) {
    fprintf(stderr,"teac_Init: unable to allocate %d bytes!\n",
	    cap->Entries*sizeof(sdramaddr_t*));
    teac_Close(result);
    return(NULL);
  }
  if (!(result->r_event[0]=
	(sdramaddr_t*)malloc( cap->Entries*NUM_SEND_BUFFERS
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
#ifdef never
  fprintf(stderr,"r_event[0][0] is %x\n",result->r_event[0][0]);
#endif

  if (!(result->m_rcv= 
	(volatile E3_uint32**)malloc( cap->Entries*sizeof(E3_uint32*) ))) {
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
  
  if (!(result->mbuff= (teacMsg**)malloc( cap->Entries*sizeof(teacMsg*) ))) {
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
#ifdef never
  fprintf(stderr,"Base of mbuff is %lx\n",(long)(result->mbuff[0]));
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
#ifdef never
  fprintf(stderr,"s_event is %x\n",result->s_event);
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
	(SBuffer*)malloc(NUM_SEND_BUFFERS*sizeof(SBuffer)))) {
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
  
  fprintf(stderr,"#######Teac initialization time: sdram handle is %d!\n",
          (int)result->ctx->sdram);
  return(result);
}
  
void teac_Close(Tcomm *tcomm)
{
  int i;
#ifdef never
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
      fprintf(stderr,"All messages have reported home!\n");
    }
    elan3_block_inputter (tcomm->ctx, 1);

    if (tcomm->e_dma != 0) elan3_freeElan(tcomm->ctx, tcomm->e_dma);
    if (tcomm->s_event != 0) elan3_freeElan(tcomm->ctx, tcomm->s_event);
    if (tcomm->r_event != NULL) {
      if (tcomm->r_event[0] != NULL) {
	if (tcomm->r_event[0][0] != 0) elan3_freeElan(tcomm->ctx, 
						      tcomm->r_event[0][0]);
	free(tcomm->r_event[0]);
      }
      free(tcomm->r_event);
    }
    if (tcomm->sbuf_pull_event[0] != 0) 
      elan3_freeElan(tcomm->ctx, tcomm->sbuf_pull_event[0]);
    if (tcomm->sendWrappers[0] != NULL) {
      for (i=0; i<NUM_SEND_BUFFERS; i++) {
	if (tcomm->sendWrappers[i]->buf != NULL) 
	  elan3_free(tcomm->ctx, tcomm->sendWrappers[i]->buf);
      }
      free(tcomm->sendWrappers[0]);
    }
    if (tcomm->rbuf_pull_event != 0) 
      elan3_freeElan(tcomm->ctx, tcomm->rbuf_pull_event);
    if (tcomm->m_snd != NULL) elan3_free(tcomm->ctx, 
					 (E3_uint32*)tcomm->m_snd);
    if (tcomm->m_rcv != NULL) {
      if (tcomm->m_rcv[0] != NULL) elan3_free(tcomm->ctx,
					      (E3_uint32*)tcomm->m_rcv[0]);
      free(tcomm->m_rcv);
    }
    if (tcomm->sbuf_ready[0] != NULL) 
      elan3_free(tcomm->ctx, (E3_uint32*)tcomm->sbuf_ready[0]);
    if (tcomm->rbuf_ready != NULL) elan3_free(tcomm->ctx, 
					    (E3_uint32*)tcomm->rbuf_ready);
    if (tcomm->mbuff != NULL) {
      if (tcomm->mbuff[0] != NULL) elan3_free(tcomm->ctx, tcomm->mbuff[0]);
      free(tcomm->mbuff);
    }
#ifdef never
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
#ifdef never
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
    fprintf(stderr,"teac_Send: message too large! (%d > %d)\n",
	    buf->validSize, buf->totSize);
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
      + sdramAddrBase[ids[iDest]/NUM_SEND_BUFFERS] 
      - sdramAddrBase[vp/NUM_SEND_BUFFERS];
    tcomm->dma->dma_dest = teac_main2elan(tcomm->ctx, msg)
      + elanAddrBase[ids[iDest]/NUM_SEND_BUFFERS] 
      - elanAddrBase[vp/NUM_SEND_BUFFERS];
    elan3_primeevent(tcomm->ctx, tcomm->s_event, 1);
    *(tcomm->m_snd)= 0;
    elan3_putdma_main(tcomm->ctx, tcomm->dma, tcomm->e_dma);
#ifdef never
    fprintf(stderr,"Send msg %d in buffer %d to %d (list index %d)...",
	    msg->msgnum,iBuf, ids[iDest],iDest);
#endif
    elan3_waitevent_word(tcomm->ctx,
			 tcomm->s_event, tcomm->m_snd, ELAN_WAIT_EVENT);
#ifdef never
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
#ifdef never
  fprintf(stderr,"got msg %d in buffer %d from %d!\n",
	  tcomm->mbuff[id][iBuf].msgnum, iBuf, id);
#endif

  /* Make some space to put the message when it arrives */
  if (!(result= (RBuffer*)malloc(sizeof(RBuffer)))) {
    fprintf(stderr,"teac_Recv: unable to allocate %d bytes!\n",
	    sizeof(RBuffer));
    return NULL;
  }
  if (!(result->buf= (void*)elan3_allocMain(tcomm->ctx, 8, tcomm->mbuff[id][iBuf].size))) {
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
  free(buf);
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
#ifdef never
  fprintf(stderr,"getConnId: <%s> %d %d maps to %d %d\n",
	  host, rank, c->lhost, node, (4*(node - c->lhost) + (rank%4)));
#endif
  return (4*(node - c->lhost) + (rank%4));
}
