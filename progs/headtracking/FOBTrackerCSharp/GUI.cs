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
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace FlockOfBirds {

  public partial class GUI : Form {

    private Tracker _Tracker;
    private UDP _udp;
    private FlockOfBirds _fob;
    private bool _muted;
    
    public GUI(Tracker t, UDP udp, FlockOfBirds fob) {
      InitializeComponent();
      _Tracker = t;
      _udp = udp;
      _fob = fob;
    }

    private class HemisphereItem {
      private string _Name;
      public string Name {
        get { return _Name; }
      }

      private byte _Code;
      public byte Code {
        get { return _Code; }
      }

      public HemisphereItem(string Name, byte Code) {
        _Name = Name;
        _Code = Code;
      }

      public override string ToString() {
        return _Name;
      }
    }

    private void GUI_Load(object sender, EventArgs e) {
      cbComPort.SelectedIndex = 0;
      cbBaudRate.SelectedIndex = cbBaudRate.Items.Count - 1;

      cbHemisphere.Items.Add(new HemisphereItem("Front", Bird.BHC_FRONT));
      cbHemisphere.Items.Add(new HemisphereItem("Rear", Bird.BHC_REAR));
      cbHemisphere.Items.Add(new HemisphereItem("Upper", Bird.BHC_UPPER));
      cbHemisphere.Items.Add(new HemisphereItem("Lower", Bird.BHC_LOWER));
      cbHemisphere.Items.Add(new HemisphereItem("Left", Bird.BHC_LEFT));
      cbHemisphere.Items.Add(new HemisphereItem("Right", Bird.BHC_RIGHT));
      cbHemisphere.SelectedIndex = 3;

      _Tracker.Paused += Tracker_paused;
      _Tracker.PoseChanged += Tracker_PoseChanged;
      _fob.Configure += FOB_Configure;
      _fob.Status += delegate(object Sender, FlockOfBirds.StatusEventArgs ev) {
        toolStripStatusLabel.Text = ev.Message;
      };

      initUDP();
      initFOB();
    }

    private void FOB_Configure(object Sender, FlockOfBirds.ConfigureEventArgs e) {
      e.SystemConfig.MeasurementRate = (double)udMRate.Value;
      e.DeviceConfig.Hemisphere = ((HemisphereItem)cbHemisphere.SelectedItem).Code;
      e.DeviceConfig.DataFormat = Bird.BDF_POSITIONMATRIX;
    }

    private void initUDP() {
      _udp.rebind(eIPAddress.Text, int.Parse(udPort.Text));
    }

    private void initFOB() {
      _fob.stop();
      _fob.start(ushort.Parse(cbComPort.Text), uint.Parse(cbBaudRate.Text));
    }

    private void Tracker_PoseChanged(object Sender, EventArgs e) {
      pPreview.Invalidate();

      if (!_muted) {
        eTimeStamp.Text = _Tracker.TimeStamp.ToString();
        eXPos.Text = _Tracker.Position[0].ToString();
        eYPos.Text = _Tracker.Position[1].ToString();
        eZPos.Text = _Tracker.Position[2].ToString();

        eXang.Text = (_Tracker.Angles[0]/Math.PI * 180).ToString();
        eYang.Text = (_Tracker.Angles[1]/Math.PI * 180).ToString();
        eZang.Text = (_Tracker.Angles[2]/Math.PI * 180).ToString();

        eRot11.Text = _Tracker.Orientation[0, 0].ToString();
        eRot12.Text = _Tracker.Orientation[0, 1].ToString();
        eRot13.Text = _Tracker.Orientation[0, 2].ToString();
        eRot21.Text = _Tracker.Orientation[1, 0].ToString();
        eRot22.Text = _Tracker.Orientation[1, 1].ToString();
        eRot23.Text = _Tracker.Orientation[1, 2].ToString();
        eRot31.Text = _Tracker.Orientation[2, 0].ToString();
        eRot32.Text = _Tracker.Orientation[2, 1].ToString();
        eRot33.Text = _Tracker.Orientation[2, 2].ToString();
      }
    }

    private void Tracker_paused(object Sender, EventArgs e) {
      eXPos.Enabled = 
      eYPos.Enabled = 
      eZPos.Enabled = 
      eXang.Enabled = 
      eYang.Enabled = 
      eZang.Enabled = 
      btnSend.Enabled = _Tracker.paused;
    }

    private void pause(bool state) {
      _Tracker.paused = state;
    }

    private void mute(bool state) {
      _muted = state;
    }
    
    private void exitToolStripMenuItem_Click(object sender, EventArgs e) {
      Close();
    }
    
    private void pPreview_Resize(object sender, EventArgs e) {
      pPreview.Invalidate();
    }

    private void btnPause_Click(object sender, EventArgs e) {
      pause(!_Tracker.paused);
    }

    private void btnMute_Click(object sender, EventArgs e) {
      mute(!_muted);
    }

    private void pPreview_Paint(object sender, PaintEventArgs e) {
      Graphics g = e.Graphics;
      int h = e.ClipRectangle.Height/2;
      int w = e.ClipRectangle.Width/2;
      IMatrix<Matrix3> m = _Tracker.Orientation;
      Font f = new System.Drawing.Font("Arial", 10);

      double l = Math.Sqrt(m[0, 0]*m[0, 0] + m[1, 0]*m[1, 0] + m[2, 0]*m[2, 0]);
      if (l == 0.0)  // Avoid dividing by 0
        return;
      
      double x = w + m[0, 0]/l * w;
      double y = h - m[1, 0]/l * h;
      g.DrawLine(System.Drawing.Pens.Red, w, h, (int)x, (int)y);
      g.DrawString("x", f, System.Drawing.Brushes.Red, (int)x, (int)y);

      l = Math.Sqrt(m[0, 1]*m[0, 1] + m[1, 1]*m[1, 1] + m[2, 1]*m[2, 1]);
      x = w + m[0, 1]/l * w;
      y = h - m[1, 1]/l * h;
      g.DrawLine(System.Drawing.Pens.Green, w, h, (int)x, (int)y);
      g.DrawString("y", f, System.Drawing.Brushes.Green, (int)x, (int)y);

      l = Math.Sqrt(m[0, 2]*m[0, 2] + m[1, 2]*m[1, 2] + m[2, 2]*m[2, 2]);
      x = w + m[0, 2]/l * w;
      y = h - m[1, 2]/l * h;
      g.DrawLine(System.Drawing.Pens.Cyan, w, h, (int)x, (int)y);
      g.DrawString("z", f, System.Drawing.Brushes.Cyan, (int)x, (int)y);
    }

    private void updatePos(int index, double value) {
      Vector3 v = _Tracker.Position.clone();
      v[index] = value;
      _Tracker.setPosition(v);
    }

    private void eXPos_Validated(object sender, EventArgs e) {
      updatePos(0, double.Parse(eXPos.Text));
    }

    private void eYPos_Validated(object sender, EventArgs e) {
      updatePos(1, double.Parse(eYPos.Text));
    }

    private void eZPos_Validated(object sender, EventArgs e) {
      updatePos(2, double.Parse(eZPos.Text));
    }

    private void updateAng(int index, double value) {
      Vector3 v = _Tracker.Angles.clone();
      v[index] = value/180.0 * Math.PI;
      _Tracker.setAngles(v);
    }

    private void eXang_Validated(object sender, EventArgs e) {
      updateAng(0, double.Parse(eXang.Text));
    }

    private void eYang_Validated(object sender, EventArgs e) {
      updateAng(1, double.Parse(eYang.Text));
    }

    private void eZang_Validated(object sender, EventArgs e) {
      updateAng(2, double.Parse(eZang.Text));
    }

    private void btnApply_Click(object sender, EventArgs e) {
      Enabled = false;
      Cursor c = Cursor;
      Cursor = System.Windows.Forms.Cursors.WaitCursor;

      try {
        initUDP();
        initFOB();
      }
      finally {
        Cursor = c;
        Enabled = true;
      }
    }

    private void btnSend_Click(object sender, EventArgs e) {
      _udp.sendPose(_Tracker.Position, _Tracker.Orientation);
    }

    private void eFLoatValidating(object sender, CancelEventArgs e) {
      TextBox t = (TextBox)sender;
      double v;
      e.Cancel = !double.TryParse(t.Text, out v);
    }

    private void aboutToolStripMenuItem_Click(object sender, EventArgs e) {
      MessageBox.Show("Flock of Birds Tracker \n \n" +
        "(c) 2006 Michael Dürig, BFH-TI \n \n" +
        "All rights reserverd", "About...");
    }

  }

}
