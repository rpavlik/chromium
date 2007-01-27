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

#ifndef FOBTRACKER_H
#define FOBTRACKER_H

#include <memory>

#include <QtGui/QMainWindow>
#include <QObject>
#include <QString>

#include "ui_fobtracker.h"
#include "FlockOfBird.h"
#include "UDP.h"
#include "RotationPreview.h" 

class CTrackerFOB: public CFlockOfBird {
private:
  int _MeasurementRate;
  int _Hemisphere;

public:
  CTrackerFOB(
    CFOBListener &FOBListener, 
    int ComPort = COM_PORT, 
    int BaudRate = BAUD_RATE,
    int MeasurementRate = MEASUREMENT_RATE, 
    int Hemisphere = BHC_FRONT):
      CFlockOfBird(FOBListener, ComPort, BaudRate), 
      _MeasurementRate(MeasurementRate), 
      _Hemisphere(Hemisphere) {}

protected:
  virtual void configure(BIRDSYSTEMCONFIG &SysConfig, BIRDDEVICECONFIG &DevConfig) {
    SysConfig.dMeasurementRate = _MeasurementRate;
    DevConfig.byHemisphere = _Hemisphere;
    DevConfig.byDataFormat = BDF_POSITIONMATRIX;
  }
};

// Well QT... I wanted this as a private inner class of FOBTracker but then moc'ing fails.
// Also putting this declaration into the implementation section causes QT to fail. So now it is 
// here for everyone to see...
class CTrackerFOBListener: public QObject, public CFOBListener {
Q_OBJECT

public:
  CTrackerFOBListener(); 
  virtual void onReceive(const FOBData &TrackerPos);
  virtual void onError(const std::string &Msg);

signals:
  void gotPosition(const FOBData &TrackerPos);
  void Error(const QString &Msg);

private:
  CTrackerFOBListener(const CTrackerFOBListener&);
  const CTrackerFOBListener& operator=(const CTrackerFOBListener&);
};

static const char *IP = "127.0.0.1";
static const unsigned int PORT = 1234;

class CTrackerMainWindow: public QMainWindow {
  Q_OBJECT

public:
  CTrackerMainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);

private:
  Ui::FOBTrackerClass ui;
  CRotationPreview *_RotationPreview;
  bool _paused;
  bool _muted;
  CUDP _UDP;
  std::auto_ptr<CTrackerFOBListener> _Listener;
  std::auto_ptr<CTrackerFOB> _FOB;

  void pause(bool state);
  void mute(bool state);
  void applySettings();
  void updateControls();
  void updateRotation();
  void setupFOB();
  void setupUDP();
  void sendPose(const Vector3 &pos, const Matrix3 &rot);
  bool initialized() { return _FOB.get() != 0; }

private slots:
  virtual void setVisible(bool visible);
  void on_btnSend_clicked();
  void on_btnApply_clicked();
  void on_btnPause_clicked();
  void on_btnMute_clicked();
  void on_eAng1_editingFinished();
  void on_eAng2_editingFinished();
  void on_eAng3_editingFinished();

  void onReceive(const FOBData& TrackerPos);
  void onError(const QString& Msg);
};

#endif // FOBTRACKER_H
