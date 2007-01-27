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
#include <math.h>

#include <QVariant>
#include <QTableWidgetItem>
#include <QTableWidget>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QPalette>

#include "fobtracker.h"

CTrackerMainWindow::CTrackerMainWindow(QWidget *parent, Qt::WFlags flags): 
  QMainWindow(parent, flags), 
  _paused(false),  
  _muted(false),  
  _UDP(IP, PORT),
  _Listener(new CTrackerFOBListener())
{
	ui.setupUi(this);

  QHBoxLayout *hLayout = new QHBoxLayout(ui.fRotation);
  hLayout->setSpacing(0);
  hLayout->setMargin(0);
  _RotationPreview = new CRotationPreview();
  QPalette palette;
  palette.setColor(_RotationPreview->backgroundRole(), Qt::black);
  _RotationPreview->setAutoFillBackground(true);
  _RotationPreview->setPalette(palette);
  hLayout->addWidget(_RotationPreview);  

  ui.cbHemisphere->clear();
  ui.cbHemisphere->addItem("Front", QVariant(BHC_FRONT));
  ui.cbHemisphere->addItem("Rear", QVariant(BHC_REAR));
  ui.cbHemisphere->addItem("Upper", QVariant(BHC_UPPER));
  ui.cbHemisphere->addItem("Lower", QVariant(BHC_LOWER));
  ui.cbHemisphere->addItem("Left", QVariant(BHC_LEFT));
  ui.cbHemisphere->addItem("Right", QVariant(BHC_RIGHT));
  ui.cbHemisphere->setCurrentIndex(3);
  ui.cbBaudRate->setCurrentIndex(ui.cbBaudRate->count() - 1);

  ui.tRotation->horizontalHeader()->hide();
  ui.tRotation->verticalHeader()->hide();
  ui.tRotation->setColumnWidth(0, 75);
  ui.tRotation->setColumnWidth(1, 75);
  ui.tRotation->setColumnWidth(2, 75);
  ui.tRotation->setRowHeight(0, 20);
  ui.tRotation->setRowHeight(1, 20);
  ui.tRotation->setRowHeight(2, 20);

  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      ui.tRotation->setItem(i, j, new QTableWidgetItem(QString::number(i == j? 1.0 : 0.0), 0));

  updateControls();

  QObject::connect(_Listener.get(), SIGNAL(gotPosition(FOBData)), SLOT(onReceive(FOBData)));
  QObject::connect(_Listener.get(), SIGNAL(Error(QString)), SLOT(onError(QString)));
}

void CTrackerMainWindow::updateControls() {
  if (_paused) {
    ui.ePosX->setEnabled(true);
    ui.ePosY->setEnabled(true);
    ui.ePosZ->setEnabled(true);
    ui.eAng1->setEnabled(true);    
    ui.eAng2->setEnabled(true);    
    ui.eAng3->setEnabled(true);    
    ui.btnSend->setEnabled(true);
  }
  else {
    ui.ePosX->setEnabled(false);
    ui.ePosY->setEnabled(false);
    ui.ePosZ->setEnabled(false);
    ui.eAng1->setEnabled(false);    
    ui.eAng2->setEnabled(false);    
    ui.eAng3->setEnabled(false);    
    ui.btnSend->setEnabled(false);
  }
}

void CTrackerMainWindow::pause(bool state) {
  _paused = state;

  if(_paused) 
    QObject::disconnect(_Listener.get(), SIGNAL(gotPosition(FOBData)), this, SLOT(onReceive(FOBData)));
  else 
    QObject::connect(_Listener.get(), SIGNAL(gotPosition(FOBData)), SLOT(onReceive(FOBData)));

  updateControls();
}

void CTrackerMainWindow::mute(bool state) {
  _muted = state;
}

void CTrackerMainWindow::sendPose(const Vector3 &pos, const Matrix3 &rot) {
  _UDP.sendPose(pos, rot);
}

void CTrackerMainWindow::setupFOB() {
  int ComPort = ui.cbComPort->currentText().toInt();
  int BaudRate = ui.cbBaudRate->currentText().toInt();
  int MRate = ui.sbMeasurementRate->value();
  int HSphere = ui.cbHemisphere->itemData(ui.cbHemisphere->currentIndex()).toInt();  // sigh...

  _FOB.reset(0);  // Explicitly release current FOB instance BEFORE a new instance is created!
  _FOB.reset(new CTrackerFOB(*_Listener, ComPort, BaudRate, MRate, HSphere));
  _FOB->start();
}

void CTrackerMainWindow::setupUDP() {
  std::string IP = ui.eIPAddress->text().toStdString();

  int Port = ui.sbPort->value();
  _UDP.rebind(IP, Port);
}

void CTrackerMainWindow::setVisible(bool visible) {
  QMainWindow::setVisible(visible);

  if (!initialized())
    applySettings();
}

void CTrackerMainWindow::on_btnPause_clicked() {
  pause(!_paused);
}

void CTrackerMainWindow::on_btnMute_clicked() {
  mute(!_muted);
}

void CTrackerMainWindow::updateRotation() {
  Matrix3 rot;
  const double PI = 3.1415926535897932384626433832795;
  double r = ui.eAng1->text().toDouble()/180 * PI;
  double e = ui.eAng2->text().toDouble()/180 * PI;
  double a = ui.eAng3->text().toDouble()/180 * PI;

  // Calculate rotation matrix from euler angles
  rot.m[0][0] =  cos(e)*cos(a);
  rot.m[0][1] =  cos(e)*sin(a);
  rot.m[0][2] = -sin(e);

  rot.m[1][0] = -cos(r)*sin(a) + sin(r)*sin(e)*cos(a);
  rot.m[1][1] =  cos(r)*cos(a) + sin(r)*sin(e)*sin(a);
  rot.m[1][2] =  sin(r)*cos(e);

  rot.m[2][0] =  sin(r)*sin(a) + cos(r)*sin(e)*cos(a);
  rot.m[2][1] = -sin(r)*cos(a) + cos(r)*sin(e)*sin(a);
  rot.m[2][2] =  cos(r)*cos(e);

  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++) 
      ui.tRotation->item(i, j)->setText(QString::number(rot.m[i][j]));

  _RotationPreview->setRotation(rot);
}

void CTrackerMainWindow::applySettings() {
  setCursor(Qt::WaitCursor);
  setEnabled(false);

  ui.lStatus->setText("Initializing...");
  setupUDP();
  setupFOB(); 
  updateRotation();

  ui.btnApply->setEnabled(true);
  setEnabled(true);
  unsetCursor();
}

void CTrackerMainWindow::on_eAng1_editingFinished() {
  updateRotation();
}

void CTrackerMainWindow::on_eAng2_editingFinished() {
  updateRotation();
}

void CTrackerMainWindow::on_eAng3_editingFinished() {
  updateRotation();
}

void CTrackerMainWindow::on_btnApply_clicked() {
  applySettings();
}

void CTrackerMainWindow::on_btnSend_clicked() {
  Vector3 pos;
  Matrix3 rot;

  pos.v[0] = ui.ePosX->text().toDouble();
  pos.v[1] = ui.ePosY->text().toDouble();
  pos.v[2] = ui.ePosZ->text().toDouble();

  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      rot.m[i][j] = ui.tRotation->item(i, j)->text().toDouble();

  sendPose(pos, rot);
}


// todo: Using QTs signal/slot mechanism to send the position data to the main thread is
// inefficient. We might need to roll our own here!
void CTrackerMainWindow::onReceive(const FOBData &TrackerPos) {
  const double PI = 3.1415926535897932384626433832795;

  if (!_muted) {
    ui.eTime->setText(QString::number(TrackerPos.time)); 
    ui.ePosX->setText(QString::number(TrackerPos.pos.v[0]));
    ui.ePosY->setText(QString::number(TrackerPos.pos.v[1]));
    ui.ePosZ->setText(QString::number(TrackerPos.pos.v[2])); 

    double x = atan(TrackerPos.rot.m[1][2] / TrackerPos.rot.m[2][2]);
    double y = asin(-TrackerPos.rot.m[0][2]);
    double z = atan(TrackerPos.rot.m[0][1] / TrackerPos.rot.m[0][0]);

    ui.eAng1->setText(QString::number(x/PI * 180.0)); 
    ui.eAng2->setText(QString::number(y/PI * 180.0));
    ui.eAng3->setText(QString::number(z/PI * 180.0));

    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        ui.tRotation->item(i, j)->setText(QString::number(TrackerPos.rot.m[i][j]));

    _RotationPreview->setRotation(TrackerPos.rot);
  }

  sendPose(TrackerPos.pos, TrackerPos.rot);
}

void CTrackerMainWindow::onError(const QString& Msg) {
  Ui::FOBTrackerClass &u = ui;
  ui.lStatus->setText(Msg);
}

CTrackerFOBListener::CTrackerFOBListener() { 
  qRegisterMetaType<FOBData>("FOBData"); 
}

void CTrackerFOBListener::onReceive(const FOBData& TrackerPos) {
  emit gotPosition(TrackerPos);
}

void CTrackerFOBListener::onError(const std::string& Msg) {
  QString s(Msg.c_str());
  emit Error(s);
}
