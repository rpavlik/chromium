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

#include <sstream>

#include "udp.h"

/**
 *  Let n be the sum of the bytes in s. Then n%256 + calcChecksum(s) == 0.
 */
char CUDP::calcChecksum(const std::string &s) const {
  char cs = 0;

  for(std::string::const_iterator i = s.begin(); i != s.end(); i++)  
    cs -= *i;

  return cs;
}

/**
 * Send pose to tracker SPU over UDP.
 */
void CUDP::sendPose(const Vector3 &pos, const Matrix3 &rot) {
  std::stringstream ss;

  // Send pose as homogenous 4x4 matrix in row major order
  for(int i = 0; i < 3; i++) {
    for(int j = 0; j < 3; j++)
      ss << rot.m[i][j] << " ";

    ss << pos.v[i] << " ";
  }

  ss << "0 0 0 1 ";
  ss << "c" << std::ends;

  std::string s = ss.str();
  int l = s.length(); 
  s[l - 2] = 0;
  s[l - 2] = calcChecksum(s);

  _sock.writeDatagram(s.c_str(), l, _IP, _Port); 
}

/**
 *  Rebind to a different IP address and UPD port.
 */
void CUDP::rebind(const std::string &IP, int Port) {
  _IP = QHostAddress(IP.c_str());

  if (Port)
    _Port = Port;
}
