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

#ifndef UDP_H
#define UDP_H

#include <string>
#include <QUdpSocket>

#include "common.h"

/** 
 * Helper class for sending position data to the tracker SPU over UDP
 */
class CUDP {
private:
  QHostAddress _IP;
  int _Port;
  QUdpSocket _sock;

protected:
  char calcChecksum(const std::string &s) const;

public:
  CUDP(const std::string &IP, int Port):
    _IP(QHostAddress(IP.c_str())), _Port(Port) {};

  void sendPose(const Vector3 &pos, const Matrix3 &rot);
  void rebind(const std::string &IP, int Port = 0);

private:
  CUDP(const CUDP&);
  CUDP& operator=(const CUDP&);
};

#endif
