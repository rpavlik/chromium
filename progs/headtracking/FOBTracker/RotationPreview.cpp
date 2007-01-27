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

#include <QPainter>
#include <QColor>
#include <QWidget>
#include <QRect>

#include "RotationPreview.h"

void CRotationPreview::paintEvent(QPaintEvent *event) {
  QWidget::paintEvent(event);

  QPainter painter(this);

  int h = geometry().height()/2;
  int w = geometry().width()/2;

  painter.setPen(Qt::red);
  double l = sqrt(_rot.m[0][0]*_rot.m[0][0] + _rot.m[1][0]*_rot.m[1][0] + _rot.m[2][0]*_rot.m[2][0]);
  int x = w + _rot.m[0][0]/l * w;
  int y = h - _rot.m[1][0]/l * h;
  painter.drawLine(w, h, x, y);
  painter.drawText(x + 1, y + 1, "x");

  painter.setPen(Qt::green);
  l = sqrt(_rot.m[0][1]*_rot.m[0][1] + _rot.m[1][1]*_rot.m[1][1] + _rot.m[2][1]*_rot.m[2][1]);
  x = w + _rot.m[0][1]/l * w;
  y = h - _rot.m[1][1]/l * h;
  painter.drawLine(w, h, x, y);
  painter.drawText(x + 1, y + 1, "y");

  painter.setPen(Qt::cyan);
  l = sqrt(_rot.m[0][2]*_rot.m[0][2] + _rot.m[1][2]*_rot.m[1][2] + _rot.m[2][2]*_rot.m[2][2]);
  x = w + _rot.m[0][2]/l * w;
  y = h - _rot.m[1][2]/l * h;
  painter.drawLine(w, h, x, y);
  painter.drawText(x + 1, y + 1, "z");
}

void CRotationPreview::setRotation(const Matrix3 &rot) {
  _rot = rot;
  update();
}

