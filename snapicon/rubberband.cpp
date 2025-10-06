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



#include "rubberband.h"
#include <QPainter>
#include <iostream>

/////////////////////
// Window draw routine
void RubberBand::paintEvent(QPaintEvent *) {
  QPixmap pm(size());
  QPainter p;
  pm.fill(rubberBandBackGround);

  p.begin(&pm);

  QPen pen(rubberBandColor);
  pen.setWidth(rubberBandBorderSize);
  pen.setJoinStyle(Qt::MiterJoin);
  p.setPen(pen);
  p.drawRect(rubberBandBorderSize / 2, rubberBandBorderSize / 2,
             width - rubberBandBorderSize, height - rubberBandBorderSize);

  pen.setColor(Qt::black);
  pen.setWidth(1);
  p.setPen(pen);

  if ((aspectSize || freeSize)) {
    QString text = QString("%1,%2")
                       .arg(parentWidget()->size().width()-2*rubberBandBorderSize)
                       .arg(parentWidget()->size().height()-2*rubberBandBorderSize);
    QFont font("Arial", 8);
    QFontMetrics fm(font);
    int pixelsWide = fm.width(text);
    int pixelsHigh = fm.height();
    QPoint textPos(parentWidget()->size().width() - pixelsWide -
                       rubberBandBorderSize,
                   parentWidget()->size().height() - pixelsHigh);
    p.setBrush(Qt::NoBrush);
    if (drawtext) {
  
      pen.setColor(Qt::black);
      pen.setWidth(1);
      p.setPen(pen);

      p.setFont(font);
      p.drawText(textPos, text);
    }
  }

  pen.setColor(Qt::black);
  pen.setWidth(1);
  p.setPen(pen);

  p.drawRect(-1, -1, rubberBandBorderSize, rubberBandBorderSize);
  p.drawRect(width - rubberBandBorderSize, -1, width, rubberBandBorderSize);
  p.drawRect(width - rubberBandBorderSize, height - rubberBandBorderSize, width,
             height);
  p.drawRect(-1, height - rubberBandBorderSize, rubberBandBorderSize, height);
  pen.setColor(rubberBandColor);
  p.setPen(pen);

  p.end();

  QPainter p1(this);
  p1.drawPixmap(0, 0, pm);
}

bool RubberBand::isTopLeftCorner(QPoint p) {

  if (p.x() > rubberBandBorderSize + 10)
    return false;
  if (p.y() > rubberBandBorderSize + 10)
    return false;

  return true;
}

bool RubberBand::isTopRightCorner(QPoint p) {
  QPoint topRight = rect().topRight();

  if (p.x() < topRight.x() - rubberBandBorderSize - 10)
    return false;
  if (p.y() > topRight.y() + rubberBandBorderSize + 10)
    return false;

  return true;
}

bool RubberBand::isBottomLeftCorner(QPoint p) {
  QPoint bottomLeft = rect().bottomLeft();

  if (p.x() > bottomLeft.x() + rubberBandBorderSize + 10)
    return false;
  if (p.y() < bottomLeft.y() - rubberBandBorderSize - 10)
    return false;

  return true;
}

bool RubberBand::isBottomRightCorner(QPoint p) {
  QPoint bottomRight = rect().bottomRight();

  if (p.x() < bottomRight.x() - rubberBandBorderSize - 10)
    return false;
  if (p.y() < bottomRight.y() - rubberBandBorderSize - 10)
    return false;

  return true;
}

bool RubberBand::isLeftEdge(QPoint p) {
  if (isTopLeftCorner(p) || isBottomLeftCorner(p))
    return false;

  if (p.x() > rubberBandBorderSize + 10)
    return false;

  return true;
}

bool RubberBand::isRightEdge(QPoint p) {
  int right = rect().right();
  if (isBottomRightCorner(p) || isTopRightCorner(p))
    return false;

  if (p.x() < right - rubberBandBorderSize - 10)
    return false;

  return true;
}

bool RubberBand::isTopEdge(QPoint p) {
  if (isTopLeftCorner(p) || isTopRightCorner(p))
    return false;

  if (p.y() > rubberBandBorderSize + 10)
    return false;

  return true;
}

bool RubberBand::isBottomEdge(QPoint p) {
  int bottom = rect().bottom();

  if (isBottomLeftCorner(p) || isBottomRightCorner(p))
    return false;

  if (p.y() < bottom - rubberBandBorderSize - 10)
    return false;

  return true;
}

bool RubberBand::isInside(QPoint p) {
  QPoint bottomRight = rect().bottomRight();
  if (p.x() < rubberBandBorderSize)
    return false;
  if (p.y() < rubberBandBorderSize)
    return false;
  if (p.x() > bottomRight.x() - rubberBandBorderSize)
    return false;
  if (p.y() > bottomRight.y() - rubberBandBorderSize)
    return false;
  return true;
}

void RubberBand::changeCursor(QEvent *) {
  QPoint mousePosition = mapFromGlobal(QCursor::pos());

  if (isTopRightCorner(mousePosition)) {
    if ((freeSize) || (aspectSize))
      setCursor(Qt::SizeBDiagCursor);
    return;
  }
  if (isTopLeftCorner(mousePosition)) {
    if ((freeSize) || (aspectSize))
      setCursor(Qt::SizeFDiagCursor);
    return;
  }

  if (isBottomLeftCorner(mousePosition)) {
    if ((freeSize) || (aspectSize))
      setCursor(Qt::SizeBDiagCursor);
    return;
  }

  if (isBottomRightCorner(mousePosition)) {
    if ((freeSize) || (aspectSize))
      setCursor(Qt::SizeFDiagCursor);
    return;
  }

  if (isTopEdge(mousePosition) && (freeSize)) {
    setCursor(Qt::SizeVerCursor);
    return;
  }

  if (isBottomEdge(mousePosition) && (freeSize)) {
    setCursor(Qt::SizeVerCursor);
    return;
  }

  if (isLeftEdge(mousePosition) && (freeSize)) {
    setCursor(Qt::SizeHorCursor);
    return;
  }

  if (isRightEdge(mousePosition) && (freeSize)) {
    setCursor(Qt::SizeHorCursor);
    return;
  }
  unsetCursor();
}

void RubberBand::enterEvent(QEvent *) {
}

void RubberBand::leave(QEvent *) {
}

void RubberBand::focusOutEvent(QFocusEvent *) {
}

void RubberBand::focusInEvent(QFocusEvent *) {
}

void RubberBand::keyPressEvent(QKeyEvent *ev) {
  if (ev->key() == Qt::Key_Up) {
    parentWidget()->move(QPoint(0, -1) + parentWidget()->pos());
    return;
  }
  if (ev->key() == Qt::Key_Down) {
    parentWidget()->move(QPoint(0, 1) + parentWidget()->pos());
    return;
  }
  if (ev->key() == Qt::Key_Left) {
    parentWidget()->move(QPoint(-1, 0) + parentWidget()->pos());
    return;
  }
  if (ev->key() == Qt::Key_Right) {
    parentWidget()->move(QPoint(1, 0) + parentWidget()->pos());
    return;
  }
}

// Mouse press handler
void RubberBand::mousePressEvent(QMouseEvent *ev) {
  drawtext = true;
  QPoint mousePosition = mapFromGlobal(QCursor::pos());
  // Activate menu
  backup_cursor = cursor();

  if (ev->button() == Qt::RightButton) {
    emit open_menu();
    return;
  }
  if (ev->button() == Qt::LeftButton) {

    if (!aspectSize && !freeSize) {
      setCursor(Qt::PointingHandCursor);
      window_is_moving = true;
      offset = mousePosition;
      return;
    }
    if (aspectSize) {
      if (!isBottomRightCorner(mousePosition) &&
          !isBottomLeftCorner(mousePosition) &&
          !isTopRightCorner(mousePosition) && !isTopLeftCorner(mousePosition)) {
        setCursor(Qt::PointingHandCursor);
        window_is_moving = true;
        offset = mousePosition;
        return;
      }
    }

    if (isTopRightCorner(mousePosition)) {
      moveTopRightCorner = true;
      return;
    }
    if (isTopLeftCorner(mousePosition)) {
      moveTopLeftCorner = true;
      return;
    }
    if (isBottomLeftCorner(mousePosition)) {
      moveBottomLeftCorner = true;
      return;
    }
    if (isBottomRightCorner(mousePosition)) {
      moveBottomRightCorner = true;
      return;
    }
    if (isTopEdge(mousePosition)) {
      moveTop = true;
      return;
    }
    if (isBottomEdge(mousePosition)) {
      moveBottom = true;
      return;
    }
    if (isLeftEdge(mousePosition)) {
      moveLeft = true;
      return;
    }
    if (isRightEdge(mousePosition)) {
      moveRight = true;
      return;
    }
    setCursor(Qt::PointingHandCursor);
    window_is_moving = true;
    offset = mousePosition;
    return;
  }
}

// Mouse move handler
void RubberBand::mouseMoveEvent(QMouseEvent *) {
  if (window_is_moving) {
    int x = QCursor::pos().x() - offset.x();
    //if (x<0)
    //  x = 0;
    int y = QCursor::pos().y() - offset.y();
    //if (y<0)
    //  y = 0;
    
    parentWidget()->move(x,y);
    return;
  }
  mousePressed = true;

  if ((!freeSize) && (!aspectSize))
    return;
  QPoint mousePosition = (QCursor::pos());
  QRect r = parentWidget()->geometry();
  if (aspectSize) {
    int a, b;
    int refx1 = r.topLeft().x();
    int refy1 = r.topLeft().y();
    int refx2 = r.bottomRight().x();
    int refy2 = r.bottomRight().y();
    if ((moveBottomLeftCorner) || (moveTopRightCorner)) {
      refx1 = r.topRight().x();
      refy1 = r.topRight().y();
      refx2 = r.bottomLeft().x();
      refy2 = r.bottomLeft().y();
    }

    a = (refy1 - refy2) / (refx1 - refx2);
    b = refy1 - a * refx1;
    int newPosX = mousePosition.x();
    int newPosY = mousePosition.x() * a + b;
    mousePosition = QPoint(newPosX, newPosY);
  }

  if (moveBottomLeftCorner)
    mousePosition += QPoint(1, -1);
  if (moveBottomRightCorner)
    mousePosition += QPoint(-1, -1);
  if (moveTopLeftCorner)
    mousePosition += QPoint(1, 1);
  if (moveTopRightCorner)
    mousePosition += QPoint(-1, 1);
  if (moveTop)
    mousePosition += QPoint(0, 1);
  if (moveBottom)
    mousePosition += QPoint(0, -1);
  if (moveRight)
    mousePosition += QPoint(-1, 0);
  if (moveLeft)
    mousePosition += QPoint(1, 0);

  if ((moveTopLeftCorner)) {
    r.setTopLeft(mousePosition);
  }
  if ((moveTopRightCorner)) {
    r.setTopRight(mousePosition);
  }
  if ((moveBottomLeftCorner)) {
    r.setBottomLeft(mousePosition);
  }
  if (moveBottomRightCorner) {
    r.setBottomRight(mousePosition);
  }

  if (!aspectSize) {
    if (moveBottom) {
      r.setBottom(mousePosition.y());
    }
    if (moveTop) {
      r.setTop(mousePosition.y());
    }
    if (moveRight) {
      r.setRight(mousePosition.x());
    }
    if (moveLeft) {
      r.setLeft(mousePosition.x());
    }
  }
  width = r.width();
  height = r.height();
  resize(width, height);
  parentWidget()->setGeometry(r);

  return;
}

// Mouse release handler
void RubberBand::resizeEvent(QResizeEvent *) {
  setCursor(Qt::PointingHandCursor);
}
// Mouse release handler
void RubberBand::mouseReleaseEvent(QMouseEvent *) {
  mousePressed = false;
  setMovingToFalse();
  setCursor(Qt::ArrowCursor);
  offset = QPoint(-1, -1);
  drawtext = false;
  update();
}

void RubberBand::setToAspectSize() {
  aspectSize = true;
  freeSize = false;
  int best = std::min(width,height);

  width = best;
  height = best;
  resize(width, height);
  parentWidget()->resize(width, height);
}


void RubberBand::setToActualSize() {
  aspectSize = false;
  freeSize = false;

  width = ACTUAL_W + 2 * rubberBandBorderSize;
  height = ACTUAL_H + 2 * rubberBandBorderSize;
  resize(width, height);
  parentWidget()->resize(width, height);
}

void RubberBand::setToDoubleSize() {
  aspectSize = false;
  freeSize = false;

  width = ACTUAL_W * 2 + 2 * rubberBandBorderSize;
  height = ACTUAL_H * 2 + 2 * rubberBandBorderSize;
  resize(width, height);
  parentWidget()->resize(width, height);
}

void RubberBand::setToQuadrupleSize() {
  aspectSize = false;
  freeSize = false;

  width = ACTUAL_W * 4 + 2 * rubberBandBorderSize;
  height = ACTUAL_H * 4 + 2 * rubberBandBorderSize;
  resize(width, height);
  parentWidget()->resize(width, height);
}

void RubberBand::changeColor() {
  QColorDialog *colorDialog = new QColorDialog(rubberBandColor, this);
  previousRubberBandcolor = rubberBandColor;
  colorDialog->setModal(false);
  colorDialog->setOption(QColorDialog::DontUseNativeDialog);
  connect(colorDialog, SIGNAL(currentColorChanged(QColor)),
          SLOT(setColor(QColor)));
  connect(colorDialog, SIGNAL(rejected()), SLOT(resetColor()));

  colorDialog->show();
  colorDialog->setCurrentColor(rubberBandColor);
}
