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



#include <iostream>

#include <QCursor>
#include <QLayout>
#include <QSizePolicy>
#include <QApplication>
#include <QDir>
#include <QMenu>
#include <QMouseEvent>
#include <QTimer>
#include <QVBoxLayout>

#include "config.h"
#include "gallery.h"
#include "item.h"
#include "set.h"
#include "glwidget.h"
#include <directorywatcher.h>


Item::Item(Gallery *pGal, std::string fileName, QWidget *parent,
           const char *, Qt::WindowFlags f)
    : QWidget(parent, f), pGallery(pGal), filename(fileName),
      pEditor(0),
      pNameTxt(new QLabel(pGal)),
      pSet(0), _offSet(0) {
  
  bgClr = palette().color(backgroundRole());
  setPalette(QPalette(bgClr));
  dragClr = QColor(Config::getDragRed(), Config::getDragGreen(),
                   Config::getDragBlue());

 }

Item::~Item() {
  // delete pNameTxt;
}

void Item::edit() {
  SavingMode savingMode = pGallery->getSavingMode();
  int posx = pGallery->posx();
  int posy = pGallery->posy();
  if (pEditor)
    pEditor->launch(filename, savingMode, posx, posy);
  pGallery->setPosx(posx + 40);
  pGallery->setPosy(posy + 40);
}

void Item::mousePressEvent(QMouseEvent *pEv) {
  pNameTxt->setStyleSheet("QLabel { background-color : none; color : red}");
  _mousePos = this->mapFromGlobal(QCursor::pos());
  switch (pEv->button()) {
  case Qt::RightButton:
    pNameTxt->setStyleSheet(
        "QLabel { background-color : none; color : black; }");
    emit openMenu(this);
    break;
  default:
    setPalette(QPalette(dragClr));
  }
  /*
  //  QWidget *topWidget = QApplication::topLevelAt(this->mapToGlobal(QPoint()));
  std::cerr<<"item mouspress event: "<<this->getName()->text().toStdString()<<std::endl;
  // topWidget->raise();
  std::cerr<<"Ok"<<std::endl;

  pNameTxt->setStyleSheet("QLabel { background-color : none; color : red}");
  std::cerr<<pNameTxt->text().toStdString()<<std::endl;
  pGallery->setSelectedItem(this);
  std::cerr<<"Selected Item set to Gallery"<<std::endl;

  switch (pEv->button()) {
  case RightButton:
    pNameTxt->setStyleSheet(
        "QLabel { background-color : none; color : black; }");
    pGallery->activateMenu();
    break;
  default:
    setPalette(QPalette(dragClr));
  }
  */
}

void Item::mouseReleaseEvent(QMouseEvent * evt) {
  pNameTxt->setStyleSheet("QLabel { background-color : none; color : black; }");
  //pGallery->unselectItems();
  pGallery->mouseReleaseEvent(evt);

  setPalette(QPalette(bgClr));
}

void Item::mouseDoubleClickEvent(QMouseEvent *) {
  edit();
  pNameTxt->setStyleSheet("QLabel { background-color : none; color : black; }");
}

void Item::mouseMoveEvent(QMouseEvent *pEv) {
  setPalette(QPalette(dragClr));
  if ( (pEv->modifiers() & Qt::ShiftModifier)) {
    int direction = _mousePos.x() - this->mapFromGlobal(QCursor::pos()).x();
    if (direction > 0) {
      pGallery->moveItemLeft(this);
      //setPalette(QPalette(bgClr));
    }
    if (direction < 0) { 
      
      pGallery->moveItemRight(this);
      //setPalette(QPalette(bgClr));
    }
    _mousePos = this->mapFromGlobal(QCursor::pos());
  }
 }

void Item::mouseGrabEvent(QMouseEvent *) {
  pNameTxt->setStyleSheet("QLabel { background-color : none; color : red}");
}


bool Item::reload() {
  return load();
}
