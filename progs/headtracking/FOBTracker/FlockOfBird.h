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

/*
 * Note: The driver for the Ascension Flock of bird tracker is available from 
 * ftp://ftp.ascension-tech.com/DRIVERS/WINDOWS_DRIVER
 */

#ifndef FLOCK_OF_BIRD_H
#define FLOCK_OF_BIRD_H

#include <windows.h>
#include <string>

#include "common.h"
#include "bird.h"     

struct FOBData {
  DWORD	time;
  Vector3 pos;
  Vector3 ang;
  Matrix3 rot;
};

/**
 * Use an instance of a class implementing CFOBListener to receive data from an CFlockOfBird object.
 * Note: onReceive() and onError() might be called on a separate thread. It is YOUR responsibility to
 * do the necessary synchronization.
 */
class CFOBListener {
public:
  virtual ~CFOBListener() {}
  virtual void onReceive(const FOBData &TrackerPos) = 0; 
  virtual void onError(const std::string &Msg) = 0;
};

static const int GROUP_ID = 1;                 // Arbitrary designation for group
static const int READ_TIMEOUT = 2000;          // 2000 ms
static const int WRITE_TIMEOUT = 2000;         // 2000 ms
static const int COM_PORT = 1;                 // Default port for FoB device
static const int BAUD_RATE = 115200;           // Default baud rate for FoB device 
static const double MEASUREMENT_RATE = 103.3;  // Default measurement rate for FoB device

/**
 *  Repesentation of the Ascension Flock of Birds tracker device in standalone mode.
 */
class CFlockOfBird {
private:
  CFOBListener &_FOBListener;
  int _ComPort;
  int _BaudRate;
  bool _StopRequest;
  int _Pos_Scale;
  HANDLE _ThreadHandle;

  static DWORD WINAPI ThreadProc(LPVOID Param);

protected:
  virtual void configure(BIRDSYSTEMCONFIG &SysConfig, BIRDDEVICECONFIG &DevConfig);
  virtual void run();

public:
  CFlockOfBird(CFOBListener &FOBListener, int ComPort = COM_PORT, int BaudRate = BAUD_RATE):
    _FOBListener(FOBListener), 
    _ComPort(ComPort),
    _BaudRate(BaudRate),
    _StopRequest(false),
    _ThreadHandle(NULL) {}

  void start();
  void stop();
  bool isStarted() { return _ThreadHandle != NULL; }
  virtual ~CFlockOfBird();

private:
  CFlockOfBird(const CFlockOfBird&);
  CFlockOfBird& operator=(const CFlockOfBird&);
};

#endif
