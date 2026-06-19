/* ******************************************************************** *
   Copyright (C) 1990-2022 University of Calgary
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ******************************************************************** */



#ifndef RUBBERBAND_H
#define RUBBERBAND_H

#include <QMainWindow>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QActionGroup>
#include <QDebug>
#include <QColorDialog>
#include <iostream>
#include "common.h"

class QCloseEvent;

class RubberBand : public QWidget {
  Q_OBJECT
public:
  RubberBand(QWidget *parent, QColor defaultColor,
             QColor defaultBackgroundColor, int defaultBorderSize) {
    // Create Label
    width = parent->size().width();
    height = parent->size().height();
    setMovingToFalse();
    aspectSize = false;
    freeSize = false;
    setAttribute(Qt::WA_Hover);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoFillBackground(false);
    rubberBandColor = defaultColor;
    rubberBandBorderSize = defaultBorderSize;
    rubberBandBackGround = defaultBackgroundColor;
    setFocusPolicy(Qt::StrongFocus);
    drawtext = false;
  }

  ~RubberBand() {}

  void setToActualSize();
  void setToDoubleSize();
  void setToQuadrupleSize();
  void setToFreeSize() {
    aspectSize = false;
    freeSize = true;
  }
  void setToAspectSize();

  int getBorderSize(){
    return rubberBandBorderSize;
  }
  
signals:
  void open_menu();

private slots:

  void resetColor() {
    rubberBandColor = previousRubberBandcolor;
    update();
  }

public slots:
  void changeColor();
  void setColor(QColor color) {
    rubberBandColor = color;
    update();
  }
  void setBackgroundColor(QColor color) {
    rubberBandBackGround = color;
    update();
  }
  void setWidth(int value) {
    width = width - 2 * (rubberBandBorderSize - value);
    height = height - 2 * (rubberBandBorderSize - value);
    rubberBandBorderSize = value;
    update();
    parentWidget()->resize(width, height);
  }

protected:
  void keyPressEvent(QKeyEvent *ev);
  void mousePressEvent(QMouseEvent *ev);
  void mouseMoveEvent(QMouseEvent *ev);
  void mouseReleaseEvent(QMouseEvent *ev);
  void paintEvent(QPaintEvent *ev);
  void enterEvent(QEvent *ev);
  void leave(QEvent *ev);
  void focusOutEvent(QFocusEvent *event);
  void focusInEvent(QFocusEvent *event);
  void resizeEvent(QResizeEvent *);

  void hoverMove(QHoverEvent *event) {
    QPoint mousePosition = mapFromGlobal(QCursor::pos());
    // so the mouse pointer is normal inside the window and change only on
    // borders
    if (window_is_moving) {
      setCursor(Qt::PointingHandCursor);
      return;
    }
    if (mousePressed) {
      return;
    }

    if (isInside(mousePosition)) {
      setCursor(Qt::ArrowCursor);
      return;
    }

    if (!isInside(mousePosition)) {
      changeCursor(event);
      return;
    }
  }

  bool event(QEvent *e) {
    switch (e->type()) {
    case QEvent::HoverMove:
      hoverMove(static_cast<QHoverEvent *>(e));
      return true;
      break;
    default:
      break;
    }
    return QWidget::event(e);
  }

private:
  void setMovingToFalse() {
    window_is_moving = false;
    moveTopLeftCorner = false;
    moveTopRightCorner = false;
    moveBottomLeftCorner = false;
    moveBottomRightCorner = false;
    moveTop = false;
    moveRight = false;
    moveBottom = false;
    moveLeft = false;
  }

  void changeCursor(QEvent *ev);

  bool isTopLeftCorner(QPoint);
  bool isTopRightCorner(QPoint);
  bool isBottomLeftCorner(QPoint);
  bool isBottomRightCorner(QPoint);
  bool isInside(QPoint);

  bool isTopEdge(QPoint);
  bool isBottomEdge(QPoint);
  bool isRightEdge(QPoint);
  bool isLeftEdge(QPoint);

  QPoint offset; // Mouse offsets
  QMenu *menu;
  bool window_is_moving;
  int width;
  int height;
  bool moveTopLeftCorner, moveTopRightCorner, moveBottomLeftCorner,
      moveBottomRightCorner;
  bool moveTop, moveRight, moveBottom, moveLeft;
  bool freeSize;
  bool aspectSize;
  QCursor backup_cursor;
  QColor rubberBandColor;
  QColor previousRubberBandcolor;

  QColor rubberBandBackGround;
  int rubberBandBorderSize;
  int previousRubberBandBorderSize;
  bool drawtext;
  bool mousePressed;
};

#endif // RUBBERBAND_H
