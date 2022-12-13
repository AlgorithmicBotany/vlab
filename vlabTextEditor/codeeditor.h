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



// Based on Qt Example

#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QObject>
#include <QMenu>
#include <iostream>

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
QT_END_NAMESPACE

class LineNumberArea;

//![codeeditordefinition]

class CodeEditor : public QPlainTextEdit {
  Q_OBJECT

public:
  CodeEditor( QWidget *parent = 0);
  
  void lineNumberAreaPaintEvent(QPaintEvent *event);
  int lineNumberAreaWidth();

  void mousePressEvent(QMouseEvent *mouse);

  void contextMenuEvent(QContextMenuEvent *event)
  {
    QMenu *menu = createStandardContextMenu();
    
    QList<QAction*> l = menu->actions();
    menu->insertAction(l[0],_saveAct);
    menu->insertSeparator(l[0]);
    menu->exec(event->globalPos());
    delete menu;
  }
  void setEnabled(bool active){
    _saveAct->setEnabled(active);
  }


protected:
  void resizeEvent(QResizeEvent *event);

private slots:
  void updateLineNumberAreaWidth(int newBlockCount);
  void highlightCurrentLine();
  void updateLineNumberArea(const QRect &, int);
  void saveFile(){
    emit save();
  }

signals:
  void save();
  

private:
  QWidget *lineNumberArea;
  QAction *_saveAct;
};

//![codeeditordefinition]
//![extraarea]

class LineNumberArea : public QWidget {
public:
  LineNumberArea(CodeEditor *editor) : QWidget(editor) { codeEditor = editor; }

  QSize sizeHint() const { return QSize(codeEditor->lineNumberAreaWidth(), 0); }

protected:
  void paintEvent(QPaintEvent *event) {
    codeEditor->lineNumberAreaPaintEvent(event);
  }

private:
  CodeEditor *codeEditor;
};

//![extraarea]

#endif
