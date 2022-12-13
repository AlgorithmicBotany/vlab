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



#include "QTGLbrowser.h"
#include "QTbrowser.h"
#include <QMimeData>
#include <iostream>
#include <QApplication>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMouseEvent>
#include <QResizeEvent>
using namespace Qt;
#include "buttons.h"
#include "comm.h"
#include "dragDrop.h"
#include "font.h"
#include "graphics.h"
#include "main.h"
#include "nodeinfo.h"
#include "qtsupport.h"
#include "xutils.h"
#include <qpainter.h>

// Constructor
QTGLbrowser::QTGLbrowser(QWidget *parent, QScrollArea *s, const char *)
    : QWidget(parent) {

  // a hack from the C days - to give external functions access to this class
  sysInfo.qgl = this;
  // set this so that scrollview asks always to redraw the whole widget,
  // not just the stuff that was uncovered
  setAutoFillBackground(true);

  // cache obtain window size
  sysInfo.winWidth = width();
  sysInfo.winHeight = height();

  // allow drag/drop
  setAcceptDrops(true);

  // ask the scrollview to automatically pan the widget when drag happens
  // [Pascal port to Qt4, is this need in Qt5 ?] setDragAutoScroll( true );

  // Apparently, WNoAutoErase isn't always enough, so we also have to set the
  // widget's background mode manually....
  // try without updating widget

  // initialy we are not in the pan mode
  isPanView = false;

  // currently we are not in drag mode
  drag_node = NULL;
  QColor backgroundColor(Qt::black);
  QPalette pal(palette());
  pal.setColor(QPalette::Background, backgroundColor);
  setAutoFillBackground(true);
  setPalette(pal);

  scrollArea = s;
}

// Destructor
QTGLbrowser::~QTGLbrowser() {}

// static bool hack = false;

void QTGLbrowser::paintEvent(QPaintEvent *) {
  QPainter qp(this);
  drawContents(&qp, 0, 0, width(), height());
  sysInfo.winWidth = width();
  sysInfo.winHeight = height();
}

void QTGLbrowser::drawContents(QPainter *p, int x, int y, int w, int h)
// redraw the tree
{
  QRect viewRect(x, y, w, h), contentsRect(x, y, w, h),
      backRect(x - topLeft.x(), y - topLeft.y(), w, h);
  viewRect.moveTopLeft(mapToGlobal(viewRect.topLeft()));

  QColor backgroundColor(sysInfo.mainForm->browserSettings().getColor(
      BrowserSettings::BackgroundColor));
  QPalette pal(palette());
  pal.setColor(QPalette::Background, backgroundColor);
  setAutoFillBackground(true);
  setPalette(pal);

  if (!topLeft.isNull())
    p->fillRect(contentsRect, backgroundColor);
  resetTopLeft();
  p->drawPixmap(topLeft.x(), topLeft.y(), backbuffer);
  // resetTopLeft();
}

void QTGLbrowser::redrawBackBuffer(void) {
  QRect treeRect = sysInfo.beginTree->getTreeRect();
  backbuffer = QPixmap(treeRect.width(), treeRect.height());
  if (backbuffer.width() != treeRect.width() ||
      backbuffer.height() != treeRect.height()) {
    int size = treeRect.width() * treeRect.height() * 4 / 1024 / 1024;
    if (size > 500) {
      std::cerr << "Warning: increasing backbuffer to " << treeRect.width()
                << "x" << treeRect.height() << " = ~" << size << "MB\n";
      static bool warned = false;
      if (!warned) {
        std::cerr << "Pavol recommends re-enabling clipping :)\n";
        warned = true;
      }
    }

    backbuffer.copy(QRect(QPoint(0, 0), treeRect.size()));
    resetTopLeft();
  }

  QPainter bb(&backbuffer);
  treeRect.setWidth(width());
  treeRect.setHeight(height());
  redraw(bb, sysInfo.mainForm->browserSettings(), treeRect);

   update(treeRect);
}

void QTGLbrowser::resetTopLeft(void) {
  // if the view is actually smaller than the widget area, center it
  topLeft.setX((width() - backbuffer.width()) / 2);
  topLeft.setY((height() - backbuffer.height()) / 2);
  if (topLeft.x() < 0)
    topLeft.setX(0);
  if (topLeft.y() < 0)
    topLeft.setY(0);
}

// handles resize events
void QTGLbrowser::resizeEvent(QResizeEvent *) {
  sysInfo.winWidth = width();
  sysInfo.winHeight = height();

  resetTopLeft();
}

QMouseEvent QTGLbrowser::translateMouseEvent(const QMouseEvent &ev) {
  return QMouseEvent(ev.type(), ev.pos() - topLeft, ev.globalPos(), ev.button(),
                     ev.buttons() & Qt::MouseButtonMask, Qt::NoModifier);
}

void QTGLbrowser::mousePressEvent(QMouseEvent *ev)
// Handle mouse press event
{

  QPoint pos = ev->pos() - topLeft;
  if (ev->button() == LeftButton) {
    if (ev->buttons() & Qt::ShiftModifier) {
      // shift left click starts panning
      isPanView = true;
      panStart = ev->globalPos();
    } else {
      // regular click just selects a node
      checkMouseClickButton(&sysInfo.buttons, pos.x(), pos.y(), B1DOWN);
      // attempt to start a drag
      drag_node =
          (NODE *)getUserDataFromButtonAt(&sysInfo.buttons, pos.x(), pos.y());
      drag_pos = ev->globalPos();
      isPanView = false;
      if (drag_node == NULL) {
        // drag was not started so start a pan
        isPanView = true;
        panStart = ev->globalPos();
        if (sysInfo.selNode) {
          sysInfo.selNode = 0;
          sysInfo.mainForm->update_menus();
          sysInfo.mainForm->updateDisplay();
        }
      }
    }
  } else if (ev->button() == MidButton) {
    checkMouseClickButton(&sysInfo.buttons, pos.x(), pos.y(), B2DOWN);
  } else if (ev->button() == RightButton) {
    checkMouseClickButton(&sysInfo.buttons, pos.x(), pos.y(), B3DOWN);
  }
  if (isPanView)
    setCursor(Qt::PointingHandCursor);
  else
    unsetCursor();
}

// Mouse release event handling
void QTGLbrowser::mouseReleaseEvent(QMouseEvent *ev) {
  if (ev->button() == LeftButton) {
    isPanView = false;
    unsetCursor();
  }
  drag_node = NULL;
}

// Mouse double-click handling
void QTGLbrowser::mouseDoubleClickEvent(QMouseEvent *ev) {
  QPoint pos = ev->pos() - topLeft;

  if (ev->button() == LeftButton) {
    checkMouseClickButton(&sysInfo.buttons, pos.x(), pos.y(), B1CLICK2);

  } else if (ev->button() == RightButton) {
    checkMouseClickButton(&sysInfo.buttons, pos.x(), pos.y(), B3CLICK2);
  }
}

// Mouse move handling
void QTGLbrowser::mouseMoveEvent(QMouseEvent *ev) {
  if (isPanView) {
    QPoint pos = ev->globalPos();
    int dx = panStart.x() - pos.x();
    int dy = panStart.y() - pos.y();
    // scrollBy( dx, dy );
    scroll(dx, dy);
    panStart = pos;
    return;
  }

  // if some node was just selected and the mouse moved far enough
  // away, initialize drag
  if (drag_node != NULL &&
      (drag_pos - ev->globalPos()).manhattanLength() > 15) {
    startDrag(drag_node);
  }
}

void QTGLbrowser::dragEnterEvent(QDragEnterEvent *ev)
// this is called when a drag enters the window
{
  if (ev->mimeData()->hasText())
    ev->acceptProposedAction();

}

void QTGLbrowser::dragMoveEvent(QDragMoveEvent *ev)
// this is called when a drag is moved in the window
{
  //    this does not seem to work once the area is scrolled, apparently
  //    it uses the shifted coordinate system or something...
  QPoint pos = ev->pos() - topLeft;
  NODE *node =
      (NODE *)getUserDataFromButtonAt(&sysInfo.buttons, pos.x(), pos.y());

  // invalid drop location if no object under the cursor, or if the
  // source is the same as destination
  if (node == 0 || node == drag_node) {
    ev->ignore();
    sysInfo.selNode = 0;
    sysInfo.mainForm->update_menus();
    sysInfo.mainForm->updateDisplay();
    return;
  }
  // select the node under the drag cursor
  if (sysInfo.selNode != node) {
    sysInfo.selNode = node;
    sysInfo.mainForm->update_menus();
    sysInfo.mainForm->updateDisplay();
  }
  // accept the event
  ev->accept();
}

void QTGLbrowser::dragLeaveEvent(QDragLeaveEvent *)
// this is called when the drag leaves the window
{}

void QTGLbrowser::dropEvent(QDropEvent *ev)
// this is called when the drag is released in the window
{

  QString data;

  if (ev->mimeData()->hasText())
    data = ev->mimeData()->text();

  if (HandleDropFileName(data))
    ev->accept();
}

void QTGLbrowser::adjustChild(int isUp) {

  int index, index2;

  // Error checking
  if (sysInfo.selNode == NULL)
    return;
  if (sysInfo.selNode->parent == NULL)
    return;

  // Find the index of the selected node
  NODE *pNode = sysInfo.selNode->parent;
  for (index = 0; index < pNode->nChildren; index++)
    if (pNode->child[index] == sysInfo.selNode)
      break;

  // Nil Operation
  if ((index == 0) && isUp)
    return;
  if ((index >= pNode->nChildren - 1) && !isUp)
    return;

  if (isUp)
    index2 = index - 1;
  else
    index2 = index + 1;


  NODE *tmpNode = pNode->child[index];
  pNode->child[index] = pNode->child[index2];
  pNode->child[index2] = tmpNode;

  // save the .ordering file
  bool success = false;
  char ordLocal[1024];
  sprintf(ordLocal, "%s/ordXXXXXX", sysInfo.tmpDir);
  int fd;
  if (-1 != (fd = mkstemp(ordLocal))) {
    FILE *idfile = fdopen(fd, "w");
    for (int i = 0; i < pNode->nChildren; i++)
      fprintf(idfile, "%s\n", pNode->child[i]->baseName);
    fclose(idfile);

    std::string ordRemote(pNode->name);
    ordRemote += "/ext/.ordering";
    if (!RA::Put_file(ordLocal, sysInfo.connection, ordRemote.c_str()))
      success = true;

    unlink(ordLocal);
  }
  if (!success) {
    std::string warning("Could not create .ordering file.\n"
                        "Ordering will not be persistent.");
    fprintf(stderr, "%s\n", warning.c_str());
  }

  // send a message to vlabd about an object having been updated
  if (sysInfo.connection->reconnect()) {
    vlabxutils::infoBox(sysInfo.mainForm,
                        "Can't adjust child\n"
                        "Connection with raserver is down\n"
                        "Check your network connection\n",
                        "Error");
    return;
  }
  QByteArray userNameData = sysInfo.login_name.toLatin1();
  const char *userNameChar = userNameData.constData();

  sysInfo.vlabd->va_send_message(UPDATE, "%s@%s:%s", userNameChar,
                                 sysInfo.host_name, pNode->name);
  sysInfo.connection->Disconnect();

  redrawBackBuffer();
}
