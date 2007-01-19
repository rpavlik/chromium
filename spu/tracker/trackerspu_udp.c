/* 
 * Copyright (c) 2006  Michael Duerig  
 * Bern University of Applied Sciences
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 */

#ifdef WINDOWS
#  include <winsock2.h>
#else
#  include <errno.h>
#  include <string.h>
#  include <sys/socket.h>
#  include <arpa/inet.h>
#  include <unistd.h>
#  include <pthread.h>
#endif

#include <stdio.h>

#include "trackerspu.h"
#include "trackerspu_udp.h"
#include "cr_error.h"

#ifdef WINDOWS
#  define CR_THREAD_PROC_DECL static DWORD WINAPI 
#  define crThreadProc LPTHREAD_START_ROUTINE
#  define CR_THREAD_EXIT(arg) return (arg);
#  define EINTR WSAEINTR
#else
#  define CR_THREAD_PROC_DECL static void*
  typedef void *(*crThreadProc) (void *);
#  define CR_THREAD_EXIT(arg) pthread_exit(arg);
#  define INVALID_SOCKET -1
#  define SOCKET_ERROR -1
#endif

static int crGetLastError() {
#ifdef WINDOWS
  return WSAGetLastError();
#else
  return errno;
#endif
}

static void* crInterlockedExchangePointer(void** target, void* value) {
#ifdef WINDOWS
  return InterlockedExchangePointer(target, value);
#else
  asm ( 
    "lock; xchgl %0, (%1) \n\t"
    : "=a" (value), "=d" (*target)
    : "0" (value), "1" (*target) 
  );

  return value;
#endif
}

static void crClose(crSocket sock) {
#ifdef WINDOWS
  closesocket(sock);
#else
  close(sock);
#endif
}

static int __crCreateThread(crThread* ThreadHandle, crThreadProc ThreadProc, void* arg) {
#ifdef WINDOWS
  return (*ThreadHandle = CreateThread(NULL, 0, ThreadProc, arg, 0, NULL)) != NULL;
#else
  return pthread_create(ThreadHandle, NULL, ThreadProc, NULL) == 0;
#endif
}

static void crThreadJoin(crThread thread) {
#ifdef WINDOWS
  WaitForSingleObject(thread, INFINITE);
#else
  pthread_join(thread, NULL);
#endif
}


static void SocketError(char *msg, int LastError) {
  crWarning("Tracker SPU: %s", msg);
  crWarning("Tracker SPU: WSAGetLastError returned %d", LastError);
}

CR_THREAD_PROC_DECL SocketThreadProc(void* arg) {
  enum {BUF_SIZE = 256 };

  int LastError;
  int RecvLen;
  char buf[BUF_SIZE]; 
  TrackerPose pose;

  for (;;) {
    if( (RecvLen = recvfrom(tracker_spu.listen_sock, buf, BUF_SIZE - 1, 0, NULL, NULL)) == SOCKET_ERROR ) {  

      LastError = crGetLastError();

      switch(LastError) {

#ifdef WINDOWS        
        case WSAESHUTDOWN:
          CR_THREAD_EXIT(0);
#endif

        case EINTR:
          continue;

        default:
          SocketError("Error on server socket of tracker SPU", LastError);
          CR_THREAD_EXIT(0);
      }
    }

    else {
      buf[RecvLen] = 0; 

      if (!unpackTrackerPose(&pose, buf))
        crWarning("Tracker SPU: Invalid datagram for tracker SPU");

      else {
        GLvectorf *l = &tracker_spu.pos[tracker_spu.currentIndex].left;
        GLvectorf *r = &tracker_spu.pos[tracker_spu.currentIndex].right;

        // Converting pose.left into world coordinates yields the position of the left eye.
        *l = pose.left;
        crMatrixTransformPointf(&tracker_spu.tracker2OpenGL, l);
        crDebug("Tracker SPU: Tracker left  (%f, %f, %f)", l->x, l->y, l->z);
        
        // For the right eye rotate rightEyeOffset by the rotation matrix received.
        // This yields the difference to the left eye. Add the left eye position 
        // and convert to world coordinates. 
        *r = tracker_spu.rightEyeOffset;
        crMatrixTransformPointf(&pose.rot, r);
        r->x += pose.left.x;   r->y += pose.left.y;  r->z += pose.left.z;
        crMatrixTransformPointf(&tracker_spu.tracker2OpenGL, r);
        crDebug("Tracker SPU: Tracker right (%f, %f, %f)", r->x, r->y, r->z);

        // Switch pointer to the current pose and flag its availibility.
        crInterlockedExchangePointer(
          (void*)&tracker_spu.currentPos, 
          &tracker_spu.pos[tracker_spu.currentIndex]);

        tracker_spu.currentIndex = (tracker_spu.currentIndex + 1) % 2;
        tracker_spu.hasNewPos = 1;
      }
    }
  }

  CR_THREAD_EXIT(0);
}

void trackerspuStartUDPServer() {
  struct sockaddr_in sock_addr;

#ifdef WINDOWS
  WSADATA wsaData;
  WORD wVersionRequested = MAKEWORD(2, 0);

  if (WSAStartup(wVersionRequested, &wsaData) != 0) {
    SocketError("Couldn't initialize winsock for tracker SPU", crGetLastError());
    return;
  }
#endif
 
  if ( (tracker_spu.listen_sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET ) {
    SocketError("Could not create server socket for tracker SPU", crGetLastError());
    return;
  }

  memset(&sock_addr, 0, sizeof(sock_addr));
  sock_addr.sin_family = AF_INET;

  if (!tracker_spu.listenIP)
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
  else
    sock_addr.sin_addr.s_addr = inet_addr(tracker_spu.listenIP);

  sock_addr.sin_port = htons(tracker_spu.listenPort); 

  if ( bind(tracker_spu.listen_sock, (struct sockaddr*) &sock_addr, sizeof(sock_addr)) != 0 ) {
    SocketError("Could not bind server socket for tracker SPU", crGetLastError());
    crClose(tracker_spu.listen_sock);
    return;
  }

  if (!__crCreateThread(&tracker_spu.hSocketThread, SocketThreadProc, NULL)) {
    SocketError("Could not create socket thread for tracker SPU", crGetLastError());
    crClose(tracker_spu.listen_sock);
    return;
  }
}

void trackerspuStopUDPServer() {
  crClose(tracker_spu.listen_sock);
  crThreadJoin(tracker_spu.hSocketThread);

#ifdef WINDOWS
  CloseHandle(tracker_spu.hSocketThread);
  WSACleanup();
#endif
}

static char calcChecksum(const char *buf) {
  char cs = 0;
  const char *p;

  for(p = buf; *p != 0; p++)
    cs -= *p;

  return cs;
}

int packTrackerPose(const TrackerPose* pose, void *buf, int len) {
  int l;
  char *b = (char *)buf;

  l = snprintf(b, len, 
    "%f %f %f"
    "%f %f %f"
    "%f %f %f"
    "%f %f %f cs", 
    pose->left.x, pose->left.y, pose->left.z,
    pose->rot.m00, pose->rot.m01, pose->rot.m02,
    pose->rot.m10, pose->rot.m11, pose->rot.m12,
    pose->rot.m20, pose->rot.m21, pose->rot.m22);

  // Patch in checksum
  if (l <= len) {
    b[l - 2] = 0;
    b[l - 2] = calcChecksum(b);
    b[l - 1] = 0;
    return l;
  }
  else
    return 0;
}

int unpackTrackerPose(TrackerPose *pose, const void* buf) {
  if (calcChecksum(buf) !=  0)
    return 0;

  pose->left.w = 1;
  pose->rot.m03 = pose->rot.m13 = pose->rot.m23 = 0;
  pose->rot.m30 = pose->rot.m31 = pose->rot.m32 = 0;
  pose->rot.m33 = 1;

  return sscanf((char *)buf, 
    "%f %f %f"
    "%f %f %f"
    "%f %f %f"
    "%f %f %f",
    &pose->left.x, &pose->left.y, &pose->left.z,
    &pose->rot.m00, &pose->rot.m10, &pose->rot.m20,
    &pose->rot.m01, &pose->rot.m11, &pose->rot.m21,
    &pose->rot.m02, &pose->rot.m12, &pose->rot.m22) == 12;
}

