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




#ifndef __QTGLBROWSER_H
#define __QTGLBROWSER_H

#include <QScrollArea>
#include <QEvent>
#include <QCursor>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QPixmap>
#include <QLabel>
#include "tree.h"

#define MOVE_FACTOR_H 1.499  //Scroll movement factor
#define MOVE_FACTOR_V 1.499  //Scroll movement factor

//Forward class declaration
class QTbrowser;

class QTGLbrowser : public QWidget
{
    Q_OBJECT

public:
  QTGLbrowser(QWidget * parent=0, QScrollArea* s=0,const char* name=0);
    ~QTGLbrowser();

    void adjustChild(int isUp);
    void redrawBackBuffer(void);
    void resetTopLeft(void);
    void ensureVisible ( int x, int y, int xmargin = 50, int ymargin = 50 ){
      scrollArea->ensureVisible(x,y,xmargin,ymargin);
    }


protected:
    // this is called when the widget needs to be repainted
    virtual void drawContents( QPainter *, int, int, int, int );
    // callback for mouse clicks
    virtual void mousePressEvent(QMouseEvent* ev);
    // callback for mouse release
    virtual void mouseReleaseEvent(QMouseEvent* ev);
    // callback for mouse double click
    virtual void mouseDoubleClickEvent(QMouseEvent* ev);
    // callback for mouse move events
    virtual void mouseMoveEvent(QMouseEvent* ev);
    // callback for drag enter event
    virtual void dragEnterEvent(QDragEnterEvent* ev);
    virtual void dragMoveEvent(QDragMoveEvent* ev);
    virtual void dragLeaveEvent(QDragLeaveEvent* ev);
    virtual void dropEvent(QDropEvent* ev);
    virtual void windowActivationChange( bool ) {}
    virtual void resizeEvent(QResizeEvent* ev);
    virtual void paintEvent(QPaintEvent *e);

private:
    // translate mouse event's coordinate from contents to viewport
    QMouseEvent translateMouseEvent( const QMouseEvent & ev );

    QTbrowser* _pWin;            //Parent widget
    bool isPanView;              //View panning flag
    QPoint panStart;             // the position of the mouse when mouse started

    QPoint topLeft;              // the top left corner of the tree
    QPixmap backbuffer;

    // node being dragged
    NODE * drag_node;
    // position of the drag when started - to delay drag until the mouse moves 'far enough'
    QPoint drag_pos;
    // store the default cursor here
    QCursor _defaultCursor;
    QScrollArea *scrollArea;

};

#endif
