/* VNC Reflector
 * Copyright (C) 2001-2003 HorizonLive.com, Inc.  All rights reserved.
 *
 * This software is released under the terms specified in the file LICENSE,
 * included.  HorizonLive provides e-Learning and collaborative synchronous
 * presentation solutions in a totally Web-based environment.  For more
 * information about HorizonLive, please see our website at
 * http://www.horizonlive.com.
 *
 * This software was authored by Constantin Kaplinsky <const@ce.cctpu.edu.ru>
 * and sponsored by HorizonLive.com, Inc.
 *
 * $Id: async_io.c,v 1.11 2006-10-30 22:43:51 brianp Exp $
 * Asynchronous file/socket I/O
 */

#include "vncspu.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#ifdef NETLOGGER
#include "nl_log.h"
#endif

#ifdef USE_POLL
#include <sys/poll.h>
#define FD_ARRAY_MAXSIZE  10000
#endif

#include "async_io.h"

extern int opt_write_coalescing;

/*
 * Global variables
 */

AIO_SLOT *cur_slot;

/*
 * Static variables
 */

struct in_addr s_bind_address;

#ifdef USE_POLL
static struct pollfd s_fd_array[FD_ARRAY_MAXSIZE];
static unsigned int s_fd_array_size;
#else
static fd_set s_fdset_read;
static fd_set s_fdset_write;
static int s_max_fd;
#endif

static AIO_FUNCPTR s_idle_func;
static AIO_SLOT *s_first_slot;
static AIO_SLOT *s_last_slot;

static volatile int s_sig_func_set;
static AIO_FUNCPTR s_sig_func[10];

static int s_close_f;

/*
 * Prototypes for static functions
 */

static AIO_SLOT *aio_new_slot(int fd, const char *name, size_t slot_size);
static void aio_process_input(AIO_SLOT *slot);
static void aio_process_output(AIO_SLOT *slot);
static void aio_process_func_list(void);
static void aio_accept_connection(AIO_SLOT *slot);
static void aio_process_closed(void);
static void aio_destroy_slot(AIO_SLOT *slot, int fatal);

static void sh_interrupt(int signo);


/*
 * Implementation
 */

/*
 * Initialize I/O sybsystem. This function should be called prior to
 * any other function herein and should NOT be called from within
 * event loop, from callback functions.
 */

void aio_init(void)
{
  int i;

#ifdef USE_POLL
  s_fd_array_size = 0;
#else
  FD_ZERO(&s_fdset_read);
  FD_ZERO(&s_fdset_write);
  s_max_fd = 0;
#endif
  s_idle_func = NULL;
  s_first_slot = NULL;
  s_last_slot = NULL;
  s_close_f = 0;

  s_sig_func_set = 0;
  for (i = 0; i < 10; i++)
    s_sig_func[i] = NULL;

  s_bind_address.s_addr = htonl(INADDR_ANY);
}

/*
 * Bind all listening sockets to specific interface specified by an IP
 * adress.
 */

int aio_set_bind_address(char *bind_ip)
{
  struct in_addr tmp_bind_address;

  if (!inet_aton(bind_ip, &tmp_bind_address))
    return 0;

  s_bind_address = tmp_bind_address;
  return 1;
}

/*
 * Create I/O slot for existing connection (open file). After new slot
 * has been created, initfunc would be called with cur_slot pointing
 * to that slot. To allow reading from provided descriptor, initfunc
 * should set some input handler using aio_setread() function.
 */

AIO_SLOT *
aio_add_slot(int fd, const char *name, AIO_FUNCPTR initfunc, size_t slot_size)
{
  AIO_SLOT *slot, *saved_slot;

  if (initfunc == NULL)
    return NULL;

  slot = aio_new_slot(fd, name, slot_size);
  if (slot == NULL)
    return NULL;

  /* Saving cur_slot value, calling initfunc with different cur_slot */
  saved_slot = cur_slot;
  cur_slot = slot;
  (*initfunc)();
  cur_slot = saved_slot;

  return slot;
}

/*
 * Create listening socket. All connections would be accepted
 * automatically. initfunc would be called after listening I/O slot is
 * created, and acceptfunc would be called for each new slot created
 * on newly accepted connection.
 */

AIO_SLOT *aio_listen(int port, AIO_FUNCPTR initfunc, AIO_FUNCPTR acceptfunc,
               size_t slot_size)
{
  AIO_SLOT *slot, *saved_slot;
  int listen_fd;
  struct sockaddr_in listen_addr;
  int optval = 1;

  errno = 0;

  /* initfunc is optional but acceptfunc should be provided. */
  if (acceptfunc == NULL)
    return 0;
  
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd < 0)
    return 0;

  if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR,
                 &optval, sizeof(int)) != 0) {
    close(listen_fd);
    return 0;
  }

  listen_addr.sin_family = AF_INET;
  listen_addr.sin_addr.s_addr = s_bind_address.s_addr;
  listen_addr.sin_port = htons((unsigned short)port);

  if ( bind(listen_fd, (struct sockaddr *)&listen_addr,
            sizeof(listen_addr)) != 0 ||
       fcntl(listen_fd, F_SETFL, O_NONBLOCK) != 0 ||
       listen(listen_fd, 5) != 0 ) {
    close(listen_fd);
    return 0;
  }

  slot = aio_new_slot(listen_fd, "[listening slot]", sizeof(AIO_SLOT));
  if (slot == NULL)
    return 0;

  slot->listening_f = 1;
  slot->bytes_to_read = slot_size;
  slot->readfunc = acceptfunc;

  if (initfunc != NULL) {
    saved_slot = cur_slot;
    cur_slot = slot;
    (*initfunc)();
    cur_slot = saved_slot;
  }

  return slot;
}

/*
 * Iterate over a list of connection slots with specified type.
 * Returns number of matching slots.
 */

int aio_walk_slots(AIO_FUNCPTR fn, int type)
{
  AIO_SLOT *slot, *next_slot;
  int count = 0;

  slot = s_first_slot;
  while (slot != NULL && !s_close_f) {
    next_slot = slot->next;
    if (slot->type == type && !slot->close_f) {
      (*fn)(slot);
      count++;
    }
    slot = next_slot;
  }

  return count;
}


int aio_walk_slots2(AIO_FUNCPTR fn, int type, void *data)
{
  AIO_SLOT *slot, *next_slot, *saved_cur;
  int count = 0;

  saved_cur = cur_slot;

  slot = s_first_slot;
  while (slot != NULL && !s_close_f) {
    next_slot = slot->next;
    if (slot->type == type && !slot->close_f) {
      cur_slot = slot;
      (*fn)(data);
      count++;
    }
    slot = next_slot;
  }

  cur_slot = saved_cur;

  return count;
}


/*
 * This function should be called if we have to execute a function
 * when I/O state is consistent, but currently we are not sure if it's
 * safe (e.g. to be called from signal handlers). fn_type should be a
 * number in the range of 0..9 and if there are two or more functions
 * of the same fn_type set, only one of them would be called
 * (probably, the latest set).
 */

void aio_call_func(AIO_FUNCPTR fn, int fn_type)
{
  if (fn_type >= 0 && fn_type < 10) {
    s_sig_func[fn_type] = fn;
    s_sig_func_set = 1;
  }
}

/*
 * Function to close connection slot. Operates on *cur_slot.
 * If fatal_f is not 0 then close all other slots and quit
 * event loop. Note that a slot would not be destroyed right
 * on this function call, this would be done later, at the end
 * of main loop cycle (however, listening sockets stop listening
 * immediately if fatal_f is 0).
 */

void aio_close(int fatal_f)
{
  aio_close_other(cur_slot, fatal_f);
}

/*
 * A function similar to aio_close, but operates on specified slot.
 */

void aio_close_other(AIO_SLOT *slot, int fatal_f)
{
  slot->close_f = 1;

  if (fatal_f) {
    s_close_f = 1;
  } else if (slot->listening_f) {
    close(slot->fd);
    slot->fd_closed_f = 1;
#ifndef USE_POLL
    FD_CLR(slot->fd, &s_fdset_read);
    if (slot->fd == s_max_fd) {
      /* NOTE: Better way is to find _existing_ max fd */
      s_max_fd--;
    }
#endif
  }
}

/*
 * Main event loop. It watches for possibility to perform I/O
 * operations on descriptors and dispatches results to custom
 * callback functions.
 *
 * Here are two versions, one uses poll(2) syscall, another uses
 * select(2) instead. Note that select(2) is more portable while
 * poll(2) is less limited.
 */

/* FIXME: Implement configurable network timeout. */

#ifdef USE_POLL

void aio_mainloop(void)
{
  AIO_SLOT *slot, *next_slot;

  signal(SIGPIPE, SIG_IGN);
  signal(SIGTERM, sh_interrupt);
  signal(SIGINT, sh_interrupt);

  if (s_sig_func_set)
    aio_process_func_list();

  while (!s_close_f) {
    if (poll(s_fd_array, s_fd_array_size, 1000) > 0) {
      slot = s_first_slot;
      while (slot != NULL && !s_close_f) {
        next_slot = slot->next;
        if (s_fd_array[slot->idx].revents & (POLLERR | POLLHUP | POLLNVAL)) {
          slot->errio_f = 1;
          slot->close_f = 1;
        } else {
          if (s_fd_array[slot->idx].revents & POLLOUT)
            aio_process_output(slot);
          if ((s_fd_array[slot->idx].revents & POLLIN) && !slot->close_f) {
            if (slot->listening_f)
              aio_accept_connection(slot);
            else
              aio_process_input(slot);
          }
        }
        slot = next_slot;
      }
      aio_process_closed();
      if (s_sig_func_set && !s_close_f)
        aio_process_func_list();
    } else {
      if (s_sig_func_set)
        aio_process_func_list();
      else if (s_idle_func != NULL)
        (*s_idle_func)();       /* Do something in idle periods */
    }
  }
  /* Close all slots and exit */
  slot = s_first_slot;
  while (slot != NULL) {
    next_slot = slot->next;
    aio_destroy_slot(slot, 1);
    slot = next_slot;
  }
}

#else

void aio_mainloop(void)
{
  fd_set fdset_r, fdset_w;
  struct timeval timeout;
  AIO_SLOT *slot, *next_slot;

  signal(SIGPIPE, SIG_IGN);
  signal(SIGTERM, sh_interrupt);
  signal(SIGINT, sh_interrupt);

  if (s_sig_func_set)
    aio_process_func_list();

  while (!s_close_f) {
    memcpy(&fdset_r, &s_fdset_read, sizeof(fd_set));
    memcpy(&fdset_w, &s_fdset_write, sizeof(fd_set));
    timeout.tv_sec = 1;         /* One second timeout */
    timeout.tv_usec = 0;
    if (select(s_max_fd + 1, &fdset_r, &fdset_w, NULL, &timeout) > 0) {
      /* loop over all slots */
      slot = s_first_slot;
      while (slot != NULL && !s_close_f) {
        next_slot = slot->next;
        if (FD_ISSET(slot->fd, &fdset_w)) {
          if ((slot->suspended & SUSPEND_WRITE) == 0) {
            aio_process_output(slot);
          }
          else {
            /*printf("output blocked\n");*/
          }
        }
        if (FD_ISSET(slot->fd, &fdset_r) && !slot->close_f) {
          if (slot->listening_f)
            aio_accept_connection(slot);
          else
            aio_process_input(slot);
        }
        slot = next_slot;
      }
      aio_process_closed();
      if (s_sig_func_set && !s_close_f)
        aio_process_func_list();
    } else {
      if (s_sig_func_set)
        aio_process_func_list();
      else if (s_idle_func != NULL)
        (*s_idle_func)();       /* Do something in idle periods */
    }
  }
  /* Stop listening, close all slots and exit */
  slot = s_first_slot;
  while (slot != NULL) {
    next_slot = slot->next;
    aio_destroy_slot(slot, 1);
    slot = next_slot;
  }
}

#endif /* USE_POLL */

void aio_setread(AIO_FUNCPTR fn, void *inbuf, int bytes_to_read)
{
  /* FIXME: Check for close_f before the work? */

  if (cur_slot->alloc_f) {
    free(cur_slot->readbuf);
    cur_slot->alloc_f = 0;
  }

  /* NOTE: readfunc must be real, not NULL */
  cur_slot->readfunc = fn;

  if (inbuf != NULL) {
    cur_slot->readbuf = inbuf;
  } else {
    if (bytes_to_read <= (int) sizeof(cur_slot->buf256)) {
      cur_slot->readbuf = cur_slot->buf256;
    } else {
      cur_slot->readbuf = malloc(bytes_to_read);
      if (cur_slot->readbuf != NULL) {
        cur_slot->alloc_f = 1;
      } else {
        cur_slot->close_f = 1;
      }
    }
  }
  cur_slot->bytes_to_read = bytes_to_read;
  cur_slot->bytes_ready = 0;
}


void aio_write(AIO_FUNCPTR fn, void *outbuf, int bytes_to_write)
{
  AIO_BLOCK *block;

  /* FIXME: Support small static buffer as in reading? */

  if (opt_write_coalescing) {
    unsigned char *dest = aio_alloc_write(bytes_to_write, &block);
    assert(dest);
    memcpy(dest, outbuf, bytes_to_write);
    assert(block);
    assert(!block->func);
    block->func = fn;

#ifdef USE_POLL
    s_fd_array[cur_slot->idx].events |= POLLOUT;
#else
    FD_SET(cur_slot->fd, &s_fdset_write);
#endif
  }
  else
  {
    /* original, non-coalesce path */
    block = calloc(1, sizeof(AIO_BLOCK));
    if (block != NULL) {
      block->buffer_size = bytes_to_write;
      block->data = malloc(block->buffer_size);
      block->data_size = bytes_to_write;
      memcpy(block->data, outbuf, bytes_to_write);
      aio_write_nocopy(fn, block);
    }
  }
}


void aio_write_nocopy(AIO_FUNCPTR fn, AIO_BLOCK *block)
{
  if (block != NULL) {
    /*printf("aio_write_nocopy %d on %d\n", block->data_size, cur_slot->fd);*/
    /* By the way, fn may be NULL */
    block->func = fn;
    block->in_list_flag = 1;
    block->serial_number = cur_slot->serial_number;

    if (cur_slot->outqueue == NULL) {
      /* Output queue was empty - init outqueue list */
      cur_slot->outqueue = block;
      cur_slot->bytes_written = 0;
#ifdef USE_POLL
      s_fd_array[cur_slot->idx].events |= POLLOUT;
#else
      FD_SET(cur_slot->fd, &s_fdset_write);
#endif
    }
    else {
      /* Output queue was not empty - add to end of outqueue list */
      cur_slot->outqueue_last->next = block;
    }

    cur_slot->outqueue_last = block;
    block->next = NULL;
  }
}


void aio_setclose(AIO_FUNCPTR closefunc)
{
  cur_slot->closefunc = closefunc;
}


/***************************
 * Static functions follow
 */

AIO_SLOT *aio_new_slot(int fd, const char *name, size_t slot_size)
{
  size_t size;
  AIO_SLOT *slot;

  /* Allocate memory make sure all fields are zeroed (very important). */
  size = (slot_size > sizeof(AIO_SLOT)) ? slot_size : sizeof(AIO_SLOT);
  slot = calloc(1, size);

  if (slot) {
    slot->fd = fd;
    if (name != NULL) {
      slot->name = strdup(name);
    } else {
      slot->name = strdup("[unknown]");
    }

    if (s_last_slot == NULL) {
      /* This is the first slot */
      s_first_slot = slot;
      slot->prev = NULL;
    } else {
      /* Other slots exist */
      s_last_slot->next = slot;
      slot->prev = s_last_slot;
    }
    s_last_slot = slot;

    /* Put fd into non-blocking mode */
    /* FIXME: check return value? */
    fcntl(fd, F_SETFL, O_NONBLOCK);

#ifdef USE_POLL
    /* FIXME: do something better if s_fd_array_size exceeds max size? */
    if (s_fd_array_size < FD_ARRAY_MAXSIZE) {
      slot->idx = s_fd_array_size++;
      s_fd_array[slot->idx].fd = fd;
      s_fd_array[slot->idx].events = POLLIN;
      s_fd_array[slot->idx].revents = 0;
    }
#else
    FD_SET(fd, &s_fdset_read);
    if (fd > s_max_fd)
      s_max_fd = fd;
#endif
  }
  return slot;
}

static void aio_process_input(AIO_SLOT *slot)
{
  int bytes = 0, error = 0;

  /* FIXME: Do not read anything if readfunc is not set?
     Or maybe skip everything we're receiving?
     Or better destroy the slot? -- I think yes. */

  if (!slot->close_f) {
    if ((slot->suspended & SUSPEND_READ) == 0) {
      int bytes_remaining = slot->bytes_to_read - slot->bytes_ready;
      errno = 0;
      if (bytes_remaining > 0) {
        bytes = read(slot->fd, slot->readbuf + slot->bytes_ready,
                     bytes_remaining);
        slot->total_read += bytes;
      }

      if (bytes > 0)
        slot->bytes_ready += bytes;
      else
        error = 1;
    }

    if (slot->bytes_to_read == 0 || slot->bytes_ready == slot->bytes_to_read) {
      /* call the socket's read function to process the buffered data */
      cur_slot = slot;
      (*slot->readfunc)();
    }
    else if (error || (bytes < 0 && errno != EAGAIN)) {
      /* error while reading - mark slot for close-down */
      slot->close_f = 1;
      slot->errio_f = 1;
      slot->errread_f = 1;
      slot->io_errno = errno;
    }
  }
}

static void aio_process_output(AIO_SLOT *slot)
{
  int bytes = 0;

  /* FIXME: Maybe write all blocks in a loop. */

  if (!slot->close_f) {
    AIO_BLOCK *block = slot->outqueue;
    if (!block)
       return;
    errno = 0;

#ifdef NETLOGGER
    if (slot->bytes_written == 0 && block->serial_number && block->first_block) {
      NL_info("vncspu", "spu.fbupdate.send.begin",
              "NODE=s NUMBER=i", vnc_spu.hostname, block->serial_number);
    }
#endif

#if 0
       printf("process output %d on %d (%d writes)\n",
           block->data_size - slot->bytes_written, slot->fd,
           block->write_count);
#endif
    if (block->data_size - slot->bytes_written > 0) {
      bytes = write(slot->fd, block->data + slot->bytes_written,
                    block->data_size - slot->bytes_written);
      slot->total_written += bytes;
    }
    if (bytes > 0 || block->data_size == 0) {
      slot->bytes_written += bytes;
      if (slot->bytes_written == block->data_size) {
        /* All block data sent, remove block */
        AIO_BLOCK *next = block->next;
        AIO_FUNCPTR func = block->func;

#ifdef NETLOGGER
        if (block->serial_number && block->last_block) {
          /* last block in queue */
          NL_info("vncspu", "spu.fbupdate.send.end",
                  "NODE=s NUMBER=i", vnc_spu.hostname, block->serial_number);
        }
#endif

        /* free this block, unless it's in the cache */
        block->in_list_flag = 0;
        if (!block->in_cache_flag) {
          free(block->data);
          free(block);
        }

        /* fix up linked list ptrs */
        if (next != NULL) {
          /* There are other blocks to send */
          slot->outqueue = next;
          slot->bytes_written = 0;
        } else {
          /* Last block sent */
          slot->outqueue = NULL;
          slot->outqueue_last = NULL;
#ifdef USE_POLL
          s_fd_array[slot->idx].events &= (short)~POLLOUT;
#else
          FD_CLR(slot->fd, &s_fdset_write);
#endif
        }

        /* Call hook function after block was sent */
        if (func) {
          cur_slot = slot;
          (*func)();
        }
      }
    }
    else if (bytes == 0 || (bytes < 0 && errno != EAGAIN)) {
      slot->close_f = 1;
      slot->errio_f = 1;
      slot->errwrite_f = 1;
      slot->io_errno = errno;
    }
  }
}


/**
 * Check if there's any output pending on any connection.
 */
int aio_any_output_pending(void)
{
   AIO_SLOT *slot;
   for (slot = s_first_slot; slot; slot = slot->next) {
      if (slot->outqueue) {
         return 1;
      }
   }
   return 0;
}


AIO_SLOT *aio_first_slot(void)
{
   return s_first_slot;
}


/**
 * Process signal handling functions.
 */
static void aio_process_func_list(void)
{
  int i;

  s_sig_func_set = 0;
  for (i = 0; i < 10; i++) {
    if (s_sig_func[i] != NULL) {
      (*s_sig_func[i])();
      s_sig_func[i] = NULL;
    }
  }

  aio_process_closed();
}

static void aio_accept_connection(AIO_SLOT *slot)
{
  struct sockaddr_in client_addr;
  unsigned int len;
  int fd;
  AIO_SLOT *new_slot, *saved_slot;

  if (!slot->close_f) {
    len = sizeof(client_addr);
    fd = accept(slot->fd, (struct sockaddr *) &client_addr, &len);
    if (fd < 0)
      return;

    new_slot = aio_new_slot(fd, inet_ntoa(client_addr.sin_addr),
                            slot->bytes_to_read);

    saved_slot = cur_slot;
    cur_slot = new_slot;
    (*slot->readfunc)();
    cur_slot = saved_slot;
  }
}

static void aio_process_closed(void)
{
  AIO_SLOT *slot, *next_slot;

  slot = s_first_slot;
  while (slot != NULL && !s_close_f) {
    next_slot = slot->next;
    if (slot->close_f)
      aio_destroy_slot(slot, 0);
    slot = next_slot;
  }
}

/*
 * Destroy a slot, free all its memory etc. If fatal != 0, assume all
 * slots would be removed one after another so do not care about such
 * things as correctness of slot list links, setfd_* masks etc.
 */

/* FIXME: Dangerous. Changes slot list while we might iterate over it. */

static void aio_destroy_slot(AIO_SLOT *slot, int fatal)
{
  AIO_BLOCK *block, *next_block;
#ifdef USE_POLL
  AIO_SLOT *h_slot;
#endif

  /* Call on-close hook */
  if (slot->closefunc != NULL) {
    cur_slot = slot;
    (*slot->closefunc)();
  }

  if (!fatal) {
    /* Remove from the slot list */
    if (slot->prev == NULL)
      s_first_slot = slot->next;
    else
      slot->prev->next = slot->next;
    if (slot->next == NULL)
      s_last_slot = slot->prev;
    else
      slot->next->prev = slot->prev;

    /* Remove references to descriptor */
#ifdef USE_POLL
    if (s_fd_array_size - 1 > slot->idx) {
      memmove(&s_fd_array[slot->idx],
              &s_fd_array[slot->idx + 1],
              (s_fd_array_size - slot->idx - 1) * sizeof(struct pollfd));
      for (h_slot = slot->next; h_slot != NULL; h_slot = h_slot->next)
        h_slot->idx--;
    }
    s_fd_array_size--;
#else
    if (!slot->fd_closed_f) {
      FD_CLR(slot->fd, &s_fdset_read);
      FD_CLR(slot->fd, &s_fdset_write);
      if (slot->fd == s_max_fd) {
        /* NOTE: Better way is to find _existing_ max fd */
        s_max_fd--;
      }
    }
#endif
  }

  /* Free all memory in slave structures */
  block = slot->outqueue;
  while (block != NULL) {
    next_block = block->next;
    free(block);
    block = next_block;
  }
  if (slot->name)
    free((char *) slot->name);
  if (slot->dpy_name)
    free((char *) slot->dpy_name);
  if (slot->alloc_f)
    free(slot->readbuf);

  /* Close the file and free the slot itself */
  if (!slot->fd_closed_f)
    close(slot->fd);

  free(slot);
}

/*
 * Signal handler catching SIGTERM and SIGINT signals
 */

static void sh_interrupt(int signo)
{
  s_close_f = 1;
  signal(signo, sh_interrupt);
}


void aio_set_slot_displayname(AIO_SLOT *slot, const char *dpyname)
{
   assert(!slot->dpy_name);
   slot->dpy_name = strdup(dpyname);
}


/**
 * Allocate a new AIO_BLOCK with the given payloadSize
 */
AIO_BLOCK *
aio_new_block(size_t payloadSize)
{
  AIO_BLOCK *block;
  assert(payloadSize > 0);

  block = calloc(1, sizeof(AIO_BLOCK));
  if (block) {
    block->data = malloc(payloadSize);
    if (block->data) {
      block->buffer_size = payloadSize;
    }
    else {
      free(block);
      block = NULL;
    }
  }
  return block;
}


/**
 * Allocate space for 'bytes' in the last AIO_BLOCK on cur_slot, if possible.
 * In some situations we still need to create a new AIO_BLOCK object.
 * The caller can then directly poke data into the buffer instead of
 * calling aio_write() which does a memcpy().
 */
unsigned char *
aio_alloc_write(size_t bytes_to_write, AIO_BLOCK **blockOut)
{
  AIO_BLOCK *block;
  unsigned char *dest;

  /* Check if we can append onto the last block in the outqueue list.
   * We can't if it's a cached block.
   * We can't if the block's callback function pointer is set.
   */
  if (cur_slot->outqueue_last
      && !cur_slot->outqueue_last->in_cache_flag
      && !cur_slot->outqueue_last->func) {
    /* append to last block in list */
    assert(cur_slot->outqueue);
    assert(!cur_slot->outqueue_last->next);
    block = cur_slot->outqueue_last;
  }
  else {
    /* allocate new block and append to tail of list */
    const size_t minSize = 4096;
    const size_t size = (bytes_to_write > minSize) ? bytes_to_write : minSize;
    block = aio_new_block(size);
    if (cur_slot->outqueue_last) {
      /* put after last block */
      cur_slot->outqueue_last->next = block;
      cur_slot->outqueue_last = block;
    }
    else {
      /* this will be the only block in the list */
      cur_slot->outqueue = block;
      cur_slot->outqueue_last = block;
    }
    cur_slot->bytes_written = 0;
  }

  assert(block);
  assert(!block->next);
  assert(!block->in_cache_flag);
  block->serial_number = cur_slot->serial_number;

  if (block->data_size + bytes_to_write > block->buffer_size) {
    /* grow by power of two */
    while (block->data_size + bytes_to_write > block->buffer_size) {
      block->buffer_size *= 2;
    }
    block->data = realloc(block->data, block->buffer_size);
  }

  dest = block->data + block->data_size;

  block->data_size += bytes_to_write;
  assert(block->data_size <= block->buffer_size);

  if (blockOut)
    *blockOut = block;

  return dest;
}


void
aio_set_serial_number(AIO_SLOT *slot, int serno)
{
  slot->serial_number = serno;
  if (serno == 0) {
    /* After an RFB update reply has been completed, fix-up the blocks'
     * first/last_block flags so we can do proper NetLogger reporting.
     */
    AIO_BLOCK *block;
    for (block = slot->outqueue; block; block = block->next) {
       /*
      block->first_block = block->last_block = 0;
       */
      if (block == slot->outqueue) {
        block->first_block = 1;
      }
      if (block->next == NULL) {
        block->last_block = 1;
      }
    }
  }
}



/**
 * Useful for debug.
 */
void
aio_print_slots(void)
{
  const AIO_SLOT *slot;

  for (slot = s_first_slot; slot; slot = slot->next) {
     printf("Slot %p\n", (void*) slot);
     printf("  Name: %s\n", slot->name);
     printf("  Type: %d\n", slot->type);
     printf("  Susp: 0x%x\n", slot->suspended);
     printf("  FD: %d\n", slot->fd);
#if 0
     if (slot->type == TYPE_HOST_ACTIVE_SLOT) {
        HOST_SLOT *hs = (HOST_SLOT *) slot;
        printf("  VNC: %d\n", hs->is_vnc_spu);
     }
#endif
  }
}
