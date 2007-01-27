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
using System.Windows.Forms;

namespace FlockOfBirds {
  static class Programm {

    /// <summary>
    /// The main entry point for the application.
    /// </summary>
    [STAThread]
    static void Main() {
      Application.EnableVisualStyles();
      Application.SetCompatibleTextRenderingDefault(false);

      UDP udp = new UDP();
      Tracker tracker = new Tracker();
      FlockOfBirds fob = new FlockOfBirds();
      GUI gui = new GUI(tracker, udp, fob);

      tracker.Paused += delegate(object Sender, EventArgs e) {
        fob.paused = tracker.paused;
      };

      tracker.PoseChanged += delegate(object Sender, EventArgs e) {
        udp.sendPose(tracker.Position, tracker.Orientation);
      };

      fob.Pose += delegate(object Sender, FlockOfBirds.PoseEventArgs e) {
        tracker.setPose(e.Position, e.Orientation, e.TimeStamp);
      };

      Application.Run(gui);
    }
  }
}
