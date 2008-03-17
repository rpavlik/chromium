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
using System;
using System.Collections.Generic;
using System.Text;
using System.Net.Sockets;

namespace FlockOfBirds {
  public class UDP {

    private UdpClient _sock;

    public UDP() {
      _sock = new UdpClient();
    }

    public UDP(string IP, int Port) {
      _sock = new UdpClient(IP, Port);
    }

    /**
     * <summary>
     * Let n be the sum of the bytes in s. Then n%256 + calcChecksum(s) == 0.
     * </summary>
     */
    private byte calcChecksum(byte[] s) {
      byte cs = 0;

      foreach (byte b in s)
        cs -= b;

      return cs;
    }

    /**
     * <summary>
     * Send pose to tracker SPU over UDP.
     * </summar>
     */
    public void sendPose(IVector<Vector3> pos, IMatrix<Matrix3> rot) {
      StringBuilder s = new StringBuilder();

      // Send pose as homogenous 4x4 matrix in row major order
      for (int i = 0 ; i < 3 ; i++) {
        for (int j = 0 ; j < 3 ; j++)
          s.AppendFormat("{0} ", rot[i, j]);

        s.AppendFormat("{0} ", pos[i]);
      }

			s.Replace(',', '.');
      s.Append("0 0 0 1 ");
      s.Append("c");

      byte[] b = Encoding.ASCII.GetBytes(s.ToString());
      int l = b.Length;
      b[l - 1] = 0;
      b[l - 1] = calcChecksum(b);

      _sock.Send(b, l);
    }

    /**
     * <summary>
     * Rebind to a different IP address and UPD port.
     * </summary>
     */
    public void rebind(string IP, int Port) {
      _sock.Close();
      try {
        _sock = new UdpClient();
        _sock.Connect(IP, Port);
      }
      catch (Exception e) {
        string msg = string.Format("Error binding to {0}:{1}", IP, Port);
        System.Windows.Forms.MessageBox.Show(e.ToString(), msg);
      }
    }
  }

}
