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
using System.Runtime.InteropServices;
using System.Threading;
using System.ComponentModel;

namespace FlockOfBirds {

  public class Bird {

    // Maximum allowable bird number
    public const uint MAX_DEVICE_NUM = 126;
    
    // Bird hemisphere codes
    public const byte BHC_FRONT = 0; // front hemisphere
    public const byte BHC_REAR = 1;  // rear hemisphere
    public const byte BHC_UPPER = 2; // upper hemisphere
    public const byte BHC_LOWER = 3; // lower hemisphere
    public const byte BHC_LEFT = 4;  // left hemisphere
    public const byte BHC_RIGHT = 5; // right hemisphere
    
    // Bird data formats
    public const byte BDF_NOBIRDDATA = 0;         // no data (NOTE: RS232 and ISA modes have no way of specifying this format)
    public const byte BDF_POSITION = 1;           // position only
    public const byte BDF_ANGLES = 2;             // angles only
    public const byte BDF_MATRIX = 3;             // matrix only
    public const byte BDF_POSITIONANGLES = 4;     // position and angles
    public const byte BDF_POSITIONMATRIX = 5;     // position and matrix
    public const byte BDF_QUATERNION = 7;         // quaternion only
    public const byte BDF_POSITIONQUATERNION = 8; // position and quaternion

    [StructLayout(LayoutKind.Sequential)]
    public struct SystemConfig {
      public byte SystemStatus;       // current system status (see bird system status bits, above)
      public byte Error;              // error code flagged by server or master bird
      public byte NumDevices;         // number of devices in system
      public byte NumServers;         // number of servers in system
      public byte XmtrNum;            // transmitter number (see transmitter number bits, above)
      public ushort XtalSpeed;        // crystal speed in MHz
      public double MeasurementRate;  // measurement rate in frames per second
      public byte ChassisNum;         // chassis number
      public byte NumChassisDevices;  // number of devices within this chassis
      public byte FirstDeviceNum;     // number of first device in this chassis
      public ushort SoftwareRev;      // software revision of server application or master bird
      public byte[] FlockStatus;      // status of all devices in flock, indexed by bird number (see note in BIRDFRAME definition) - see bird flock status bits, above
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Angles {
      public short Azimuth;   // azimuth angle
      public short Elevation; // elevation angle
      public short Roll;      // roll angle
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct DeviceConfig {
      public byte Status;        // device status (see bird device status bits, above)
      public byte ID;            // device ID code (see bird device ID's, above)
      public ushort SoftwareRev; // software revision of device
      public byte Error;         // error code flagged by device
      public byte Setup;         // setup information (see bird device setup bits, above)
      public byte DataFormat;    // data format (see bird data formats, above)
      public byte ReportRate;    // rate of data reporting, in units of frames
      public ushort Scaling;     // full scale measurement, in inches
      public byte Hemisphere;    // hemisphere of operation (see bird hemisphere codes, above)
      public byte DeviceNum;     // bird number
      public byte XmtrType;      // transmitter type (see bird transmitter type bits, above)
      [MarshalAs(UnmanagedType.ByValArray, SizeConst = 7)]   
      public ushort[] AlphaMin;  // filter constants (see Birdnet3 Protocol pp.26-27 for values)
      [MarshalAs(UnmanagedType.ByValArray, SizeConst = 7)]  
      public ushort[] AlphaMax;  // filter constants (see Birdnet3 Protocol pp.26-27 for values)
      [MarshalAs(UnmanagedType.ByValArray, SizeConst = 7)]  
      public ushort[] VM;        // filter constants (see Birdnet3 Protocol pp.26-27 for values) 
      public Angles ReferenceFrame; // reference frame of bird readings
      public Angles AngleAlign;     // alignment of bird readings
    }

    public enum GroupModeSettings {
      GMS_GROUP_MODE_NEVER,           // RS232 group mode will never be used
      GMS_GROUP_MODE_ALWAYS,          // RS232 group mode will always be used
      NUM_GROUP_MODE_SETTINGS
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct Position {
      public short X;  // x-coordinate
      public short Y;  // y-coordinate
      public short Z;  // z-coordinate
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Matrix { 
      [MarshalAs(UnmanagedType.ByValArray, SizeConst = 9)]  
      private short[] n;  // array of matrix elements

      public short this[int i, int j] {
        get { return n[i*3 + j]; }
        set { n[i*3 + j] = value; }
      }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Quaternion {
      public short Q0;  // q0
      public short Q1;  // q1
      public short Q2;  // q2
      public short Q3;  // q3
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Reading {
      public Position position;     // position of receiver
      public Angles angles;         // orientation of receiver, as angles
      public Matrix matrix;         // orientation of receiver, as matrix
      public Quaternion quaternion; // orientation of receiver, as quaternion
      public uint Buttons;          // button states
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Frame {
      public uint Time;          // time at which readings were taken, in msecs
      [MarshalAs(UnmanagedType.ByValArray, SizeConst = (int)MAX_DEVICE_NUM)]  
      public Reading[] reading;  // reading from each bird
    }

    [DllImport("Bird.dll", EntryPoint="birdRS232WakeUp")]
    public static extern bool RS232WakeUp(int GroupID, bool StandAlone, int NumDevices, ref ushort Comport, 
      uint BaudRate, uint ReadTimeout, uint WriteTimeout, GroupModeSettings GroupMode);

    [DllImport("Bird.dll", EntryPoint="birdGetSystemConfig")]
    public static extern bool GetSystemConfig(int GroupID, ref SystemConfig SysCfg, bool GetDriverCopy);

    [DllImport("Bird.dll", EntryPoint="birdGetDeviceConfig")]
    public static extern bool GetDeviceConfig(int GroupID, int DeviceNum, ref DeviceConfig DevCfg, 
      bool GetDriverCopy);

    [DllImport("Bird.dll", EntryPoint="birdSetSystemConfig")]
    public static extern bool SetSystemConfig(int GroupID, ref SystemConfig SysCfg);

    [DllImport("Bird.dll", EntryPoint="birdSetDeviceConfig")]
    public static extern bool SetDeviceConfig(int GroupID, int DeviceNum, ref DeviceConfig DevCfg);

    [DllImport("Bird.dll", EntryPoint="birdStartFrameStream")]
    public static extern bool StartFrameStream(int GroupID);

    [DllImport("Bird.dll", EntryPoint="birdFrameReady")]
    public static extern bool FrameReady(int GroupID);

    [DllImport("Bird.dll", EntryPoint="birdGetFrame")]
    public static extern bool GetFrame(int GroupID, out Frame frame);

    [DllImport("Bird.dll", EntryPoint="birdGetErrorMessage")]
    public static extern string GetErrorMessage();

    [DllImport("Bird.dll", EntryPoint="birdStopFrameStream")]
    public static extern bool StopFrameStream(int GroupID);

    [DllImport("Bird.dll", EntryPoint="birdShutDown")]
    public static extern void ShutDown(int GroupID);

  }

  public class FlockOfBirds {
    private const uint READ_TIMEOUT = 2000;
    private const uint WRITE_TIMEOUT = 2000;
    private const int GROUP_ID = 1;

    private ushort _PosScale;

    public class StatusEventArgs: EventArgs {
      private string _Message;
      public string Message {
        get { return _Message; }
      }

      public StatusEventArgs(string Message) {
        _Message = Message;
      }
    }

    public delegate void StatusEventHandler(object Sender, StatusEventArgs e);
    public event StatusEventHandler Status;
    protected virtual void OnStatus(StatusEventArgs e) {
      if (Status != null)
        Status(this, e);
    }

    public class PoseEventArgs: EventArgs {
      private uint _TimeStamp;
      public uint TimeStamp {
        get { return _TimeStamp; }
      }

      private Vector3 _Position;
      public Vector3 Position {
        get { return _Position; }
      }

      private Vector3 _Angles;
      public Vector3 Angles {
        get { return _Angles; }
      }

      private Matrix3 _Orientation;
      public Matrix3 Orientation {
        get { return _Orientation; }
      }

      public PoseEventArgs(uint TimeStamp, Vector3 Position, Vector3 Angles, Matrix3 Orientation) {
        _TimeStamp = TimeStamp;
        _Position = Position;
        _Angles = Angles;
        _Orientation = Orientation;
      }
    }

    public delegate void PoseEventHandler(object Sender, PoseEventArgs e);
    public event PoseEventHandler Pose;
    protected virtual void OnPose(PoseEventArgs e) {
      if (Pose != null)
        Pose(this, e);
    }

    public class ConfigureEventArgs: EventArgs {
      public Bird.SystemConfig SystemConfig = new Bird.SystemConfig();
      public Bird.DeviceConfig DeviceConfig = new Bird.DeviceConfig();
    }

    public delegate void ConfigureEventHandler(object Sender, ConfigureEventArgs e);
    public event ConfigureEventHandler Configure;
    protected virtual void OnConfigure(ConfigureEventArgs e) {
      if (Configure != null)
        Configure(this, e);
    }

    AsyncRun _Worker;
    IAsyncResult _WorkerResult;

    public bool isStarted {
      get { return _Worker != null; }
    }

    private bool _paused;
    public bool paused {
      get { return _paused; }
      set { _paused = value; }
    }

    private bool _StopRequest;

    public void stop() {
      if (isStarted) {
        _StopRequest = true;  // Boolean assignment is atomic according to C# Language Specification section 12.5
        _Worker.EndInvoke(_WorkerResult);
        _Worker = null;
      }

      // Stop acqusition
      Bird.StopFrameStream(GROUP_ID);
      Bird.ShutDown(GROUP_ID);
    }

    public void start(ushort ComPort, uint BaudRate) {
      if (isStarted)
        return;

      OnStatus(new StatusEventArgs("Initializing..."));

      // Start by sending the wake up command to the Flock
      if (!Bird.RS232WakeUp(GROUP_ID, true, 1, ref ComPort, BaudRate, READ_TIMEOUT, WRITE_TIMEOUT,
           Bird.GroupModeSettings.GMS_GROUP_MODE_ALWAYS)) 
      {
        OnStatus(new StatusEventArgs("Bird wake up failed"));
        return;
      }

      // Configure the device
      ConfigureEventArgs cfg = new ConfigureEventArgs();
      if (!Bird.GetSystemConfig(GROUP_ID, ref cfg.SystemConfig, false)) {
        OnStatus(new StatusEventArgs("Bird system configuration failed"));
        return;
      }

      if (!Bird.GetDeviceConfig(GROUP_ID, 1, ref cfg.DeviceConfig, false)) {
        OnStatus(new StatusEventArgs("Bird device configuration failed"));
        return;
      }

      OnConfigure(cfg);

      if (!Bird.SetSystemConfig(GROUP_ID, ref cfg.SystemConfig)) {
        OnStatus(new StatusEventArgs("Bird system configuration failed"));
        return;
      }

      if (!Bird.SetDeviceConfig(GROUP_ID, 1, ref cfg.DeviceConfig)) {
        OnStatus(new StatusEventArgs("Bird device configuration failed"));
        return;
      }

      // Start acquisition 
      if (!Bird.StartFrameStream(GROUP_ID)) {
        OnStatus(new StatusEventArgs("Bird failed to start streaming"));
        return;
      }

      _PosScale = cfg.DeviceConfig.Scaling;

      // Start receiving frames asynchronously
      _StopRequest = false;
      _Worker = new AsyncRun(run);
      _WorkerResult = _Worker.BeginInvoke(AsyncOperationManager.CreateOperation(this), 
        new SendOrPostCallback(OnPose), 
        new SendOrPostCallback(OnStatus), null, null);

      OnStatus(new StatusEventArgs("Ok"));
    }

    private void OnPose(object Pose) {
      OnPose((PoseEventArgs)Pose);
    }

    private void OnStatus(object Status) {
      OnStatus((StatusEventArgs)Status);
    }

    private delegate void AsyncRun(AsyncOperation AsyncOp, SendOrPostCallback OnPoseCallback,
      SendOrPostCallback OnStatusCallback);

    private void run(AsyncOperation AsyncOp, SendOrPostCallback OnPoseCallback,
      SendOrPostCallback OnStatusCallback) 
    {
      while (!_StopRequest) 
        if (!_paused && Bird.FrameReady(GROUP_ID)) { 

          // Get data frame from bird
          Bird.Frame frame;
          if(!Bird.GetFrame(GROUP_ID, out frame)) { 
            string err = Bird.GetErrorMessage();
            OnStatus(new StatusEventArgs(err));
          }

          Bird.Reading reading = frame.reading[0];
          
          // convert position and angle data
          Vector3 p = new Vector3(
            (reading.position.X * _PosScale / 32767.0) * 2.54,
            (reading.position.Y * _PosScale / 32767.0) * 2.54,
            (reading.position.Z * _PosScale / 32767.0) * 2.54);

          Vector3 a = new Vector3(
            reading.angles.Azimuth * 180.0 / 32767.0,
            reading.angles.Elevation * 180.0 / 32767.0,
            reading.angles.Roll * 180.0 / 32767.0);

          Matrix3 o = new Matrix3();
          for (int i = 0 ; i < 3 ; i++)
            for (int j = 0 ; j < 3 ; j++)
              o[i, j] = reading.matrix[i, j] / 32767.0;
          
          AsyncOp.Post(OnPoseCallback, (new PoseEventArgs(frame.Time, p, a, o)));
          AsyncOp.Post(OnStatusCallback, new StatusEventArgs("Ok"));
        }			
        else {
          System.Threading.Thread.Sleep(1); // Yield the rest of our time slice. Sleep(0) doesn't seem to work. 
          // AsyncOp.Post(OnStatusCallback, new StatusEventArgs("No Data"));
        }
    }

  }

}
