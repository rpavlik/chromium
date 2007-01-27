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

#include "flockofbird.h"

CFlockOfBird::~CFlockOfBird() {
  stop();
}

DWORD CFlockOfBird::ThreadProc(LPVOID Param) {
  CFlockOfBird* fob = (CFlockOfBird*)Param;
  fob->run();  
  return 0;   
}

/** 
 * Start receiving data from the Flock of birds device.
 * Data and errors are communicated to the CFOBListener passed to the ctor.
 */
void CFlockOfBird::start() {

  // Start by sending the wake up command to the Flock
  WORD Port = _ComPort;
  if (!birdRS232WakeUp(GROUP_ID, true, 1, &Port, _BaudRate, READ_TIMEOUT, WRITE_TIMEOUT)) {
    _FOBListener.onError("Bird wake up failed");
    return;
  }

  // Configure the device
  BIRDSYSTEMCONFIG SysConfig;
  BIRDDEVICECONFIG DevConfig;

  if (!birdGetSystemConfig(GROUP_ID, &SysConfig)) {
    _FOBListener.onError("Bird system configuration failed");
    return;
  }

  if (!birdGetDeviceConfig(GROUP_ID, 1, &DevConfig)) {
    _FOBListener.onError("Bird device configuration failed");
    return;
  }

  configure(SysConfig, DevConfig);

  if (!birdSetSystemConfig(GROUP_ID, &SysConfig)) {
    _FOBListener.onError("Bird system configuration failed");
    return;
  }

  if (!birdSetDeviceConfig(GROUP_ID, 1, &DevConfig)) {
    _FOBListener.onError("Bird device configuration failed");
    return;
  }

  // Start acquisition 
  if (!birdStartFrameStream(GROUP_ID)) {
    _FOBListener.onError("Bird failed to start streaming");
    return;
  }

  _Pos_Scale = DevConfig.wScaling;

  DWORD ThreadID;
  if (!(_ThreadHandle = CreateThread(0, 0, ThreadProc, this, 0, &ThreadID))) {
    _FOBListener.onError("Error starting bird thread");
    return;
  }

  _FOBListener.onError("Ok");
}

/** 
 * Stop receiving data from the Flock of birds device
 */
void CFlockOfBird::stop() {
  if (isStarted()) {
    _StopRequest = true;
    WaitForSingleObject(_ThreadHandle, INFINITE);
    CloseHandle(_ThreadHandle);
    _ThreadHandle = NULL;
  }

  // Stop acqusition
  birdStopFrameStream(GROUP_ID);	
  birdShutDown(GROUP_ID);  
}

/**
 *  This method is called before data acquisition starts. Override it to specify your own
 *  configuration options. See bird.h for details on the arguments.
 */
void CFlockOfBird::configure(BIRDSYSTEMCONFIG &SysConfig, BIRDDEVICECONFIG &DevConfig) {
  SysConfig.dMeasurementRate = MEASUREMENT_RATE;
}

/**
 * Here we collect the data from the FoB device and pass it to the CFOBListener instance.
 */
void CFlockOfBird::run() { 
  while (!_StopRequest) 
    if(birdFrameReady(GROUP_ID)) {

      // Get data frame from bird
      BIRDFRAME frame;

      if(!birdGetFrame(GROUP_ID, &frame)) { 
        std::string err(birdGetErrorMessage());
        _FOBListener.onError(err);
      }

      BIRDREADING *preading = &frame.reading[0];

      // convert position and angle data
      FOBData d;
      d.time = frame.dwTime;
      d.pos.v[0] = (preading->position.nX * _Pos_Scale / 32767.)*2.54;
      d.pos.v[1] = (preading->position.nY * _Pos_Scale / 32767.)*2.54;
      d.pos.v[2] = (preading->position.nZ * _Pos_Scale / 32767.)*2.54;
      d.ang.v[0] = preading->angles.nAzimuth * 180. / 32767.;      // Zang 
      d.ang.v[1] = preading->angles.nElevation * 180. / 32767.;    // Yang
      d.ang.v[2] = preading->angles.nRoll * 180. / 32767.;         // Xang

      for(int i = 0; i < 3; i++)
        for(int j = 0; j < 3; j++)
          d.rot.m[i][j] = preading->matrix.n[i][j] / 32767.;

      _FOBListener.onReceive(d);
      _FOBListener.onError("Ok");
    }			
    else {
      Sleep(1); // Yield the rest of our time slice. (Note: Sleep(0) doesn't seem to work)
      // _FOBListener.onError("No Data");
    }
}
