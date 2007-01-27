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
using System.Collections;
using System.Collections.Generic;
using System.Text;

namespace FlockOfBirds {

  public interface IMatrix<T>: IEnumerable<double> {
    double this[int i, int j] { get; }
    T clone();
  }

  public class Matrix3: IMatrix<Matrix3> {
    private double[,] _m;

    public Matrix3() {
      _m = new double[,] { 
        {1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {0.0, 0.0, 1.0}  
      };
    }

    public Matrix3(Matrix3 m) {
      _m = (double[,])m._m.Clone();
    }

    public double this[int i, int j] {
      get { return _m[i, j]; }
      set { _m[i, j] = value; }
    }

    public Matrix3 clone() {
      return new Matrix3(this);
    }

    public IEnumerator<double> GetEnumerator() {
      foreach (double d in _m)
        yield return d;
    }

    IEnumerator IEnumerable.GetEnumerator() {
      return GetEnumerator();
    }

    public override bool Equals(object o) {
      if (o == null)
        return false;

      if (GetType() != o.GetType())
        return false;

      Matrix3 m1 = (Matrix3)o;
      return _m.Equals(m1._m);
    }

    public static bool operator==(Matrix3 m1, Matrix3 m2) {
      return m1.Equals(m2);
    }

    public static bool operator!=(Matrix3 m1, Matrix3 m2) {
      return !m1.Equals(m2);
    }

    public override int GetHashCode() {
      return _m.GetHashCode();
    }

    public Matrix3 scale(double factor) {
      for (int i = 0 ; i < 3 ; i++)
        for (int j = 0 ; j < 3 ; j++)
          _m[i, j] *= factor;

      return this;
    }

  }

  public interface IVector<T>: IEnumerable<double>{
    double this[int i] { get; }
    T clone();
  }

  public class Vector3: IVector<Vector3> {
    private double _v0, _v1, _v2;

    public Vector3() {
      _v0 = _v1 = _v2 = 0.0;
    }

    public Vector3(Vector3 v) {
      _v0 = v._v0;
      _v1 = v._v1;
      _v2 = v._v2;
    }

    public Vector3(double v0, double v1, double v2) {
      _v0 = v0;
      _v1 = v1;
      _v2 = v2;
    }

    public double this[int i] {
      get { 
        switch(i) {
          case 0:  return _v0;
          case 1:  return _v1;
          case 2:  return _v2;
          default: throw new System.IndexOutOfRangeException();
        }
      }
      set {
        switch (i) {
          case 0: _v0 = value; break;
          case 1: _v1 = value; break;
          case 2: _v2 = value; break;
          default: throw new System.IndexOutOfRangeException();
        }
      }
    }

    public Vector3 clone() {
      return new Vector3(this);
    }

    public IEnumerator<double> GetEnumerator() {
      yield return _v0;
      yield return _v1;
      yield return _v2;
    }

    IEnumerator IEnumerable.GetEnumerator() {
      return GetEnumerator();
    }

    public override bool Equals(object o) {
      if (o == null)
        return false;

      if (GetType() != o.GetType())
        return false;

      Vector3 v1 = (Vector3)o;
      return _v0 == v1._v0 && _v1 == v1._v1 && _v2 == v1._v2;
    }

    public static bool operator==(Vector3 v1, Vector3 v2) {
      return v1.Equals(v2);
    }

    public static bool operator!=(Vector3 v1, Vector3 v2) {
      return !v1.Equals(v2);
    }

    public override int GetHashCode() {
      return (_v0 + _v1 + _v2).GetHashCode();
    }

  }

  public class Tracker {
    private Vector3 _Position = new Vector3();
    public IVector<Vector3> Position {
      get { return _Position; }
    }

    public void setPosition(Vector3 v) {
      if (_Position == v)
        return;

      _Position = v;
      OnPoseChanged(EventArgs.Empty);
    }

    private Matrix3 _Orientation = new Matrix3();
    public IMatrix<Matrix3> Orientation {
      get { return _Orientation; }
    }

    public void setOrientation(Matrix3 m) {
      if (_Orientation == m)
        return;

      _Orientation = m;
      OnPoseChanged(EventArgs.Empty);
    }

    public IVector<Vector3> Angles {
      get {
        Vector3 v = new Vector3();
        v[0] = Math.Atan(_Orientation[1, 2] / _Orientation[2, 2]);
        v[1] = Math.Asin(-_Orientation[0, 2]);
        v[2] = Math.Atan(_Orientation[0, 1] / _Orientation[0, 0]);
        return v;
      }
    }

    private bool setAnglesInternal(Vector3 v) {
      double r = v[0];
      double e = v[1];
      double a = v[2];
      Matrix3 o = new Matrix3();

      // Calculate rotation matrix from euler angles
      o[0, 0] = Math.Cos(e)*Math.Cos(a);
      o[0, 1] = Math.Cos(e)*Math.Sin(a);
      o[0, 2] = -Math.Sin(e);

      o[1, 0] = -Math.Cos(r)*Math.Sin(a) + Math.Sin(r)*Math.Sin(e)*Math.Cos(a);
      o[1, 1] = Math.Cos(r)*Math.Cos(a) + Math.Sin(r)*Math.Sin(e)*Math.Sin(a);
      o[1, 2] = Math.Sin(r)*Math.Cos(e);

      o[2, 0] = Math.Sin(r)*Math.Sin(a) + Math.Cos(r)*Math.Sin(e)*Math.Cos(a);
      o[2, 1] = -Math.Sin(r)*Math.Cos(a) + Math.Cos(r)*Math.Sin(e)*Math.Sin(a);
      o[2, 2] = Math.Cos(r)*Math.Cos(e);

      if (o != _Orientation) {
        _Orientation = o;
        return true;
      } else
        return false;
    }

    public void setAngles(Vector3 v) {
      if (setAnglesInternal(v))
        OnPoseChanged(EventArgs.Empty);
    }

    private uint _TimeStamp;
    public uint TimeStamp {
      get { return _TimeStamp; }
    }

    public void setPose(Vector3 Position, Vector3 Angles, uint TimeStamp) {
      bool dirty = false;

      if (_TimeStamp != TimeStamp) {
        dirty = true;
        _TimeStamp = TimeStamp;
      }        

      if (_Position != Position) {
        _Position = Position;
        dirty = true;
      }

      if (setAnglesInternal(Angles))
        dirty = true;

      if (dirty)
        OnPoseChanged(EventArgs.Empty);
    }

    public void setPose(Vector3 Position, Matrix3 Orientation, uint TimeStamp) {
      bool dirty = false;

      if (_TimeStamp != TimeStamp) {
        dirty = true;
        _TimeStamp = TimeStamp;
      }        

      if (_Position != Position) {
        _Position = Position;
        dirty = true;
      }

      if (_Orientation != Orientation) {
        _Orientation = Orientation;
        dirty = true;
      }

      if (dirty)
        OnPoseChanged(EventArgs.Empty);
    }

    private bool _paused;
    public bool paused {
      get { return _paused; }
      set {
        if (_paused != value) {
          _paused = value;
          OnPaused(EventArgs.Empty);
        }
      }
    }

    public delegate void PoseChangedEventHandler(object Sender, EventArgs e);
    public event PoseChangedEventHandler PoseChanged;
    protected virtual void OnPoseChanged(EventArgs e) {
      if (PoseChanged != null)
        PoseChanged(this, e);
    }

    public delegate void PausedEventHandler(object Sender, EventArgs e);
    public event PausedEventHandler Paused;
    protected virtual void OnPaused(EventArgs e) {
      if (Paused != null)
        Paused(this, e);
    }

  }


}
