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



#include "FinderWidget.h"
#include "RA.h"
#include "main.h"
#include "openNode.h"
#include <QKeyEvent>
#include <QProgressDialog>
#include <iostream>
#include <QDateTime>

FinderWidget::FinderWidget(QWidget *parent, const char *)
    : QLineEdit(parent) {
  QColor backgroundColor(QColor(255, 222, 187));
  QPalette pal(palette());
  pal.setColor(QPalette::Background, backgroundColor);
  setAutoFillBackground(true);
  setPalette(pal);


  QColor foregroundColor(QColor(0, 0, 0));
  pal = palette();
  pal.setColor(QPalette::Foreground, foregroundColor);
  setPalette(pal);
  setFrame(true);
  setText("Search text");
  selectAll();
  in_progress = false;

  connect(this, SIGNAL(textChanged(const QString &)), SLOT(textChanged_cb()));
}

void FinderWidget::activate() {
  QColor backgroundColor(QColor(255, 222, 187));
  QPalette pal(palette());
  pal.setColor(QPalette::Background, backgroundColor);
  setAutoFillBackground(true);
  setPalette(pal);


  QColor foregroundColor(QColor(0, 0, 0));
  pal = palette();
  pal.setColor(QPalette::Foreground, foregroundColor);
  setPalette(pal);
  setEnabled(true);
  show();
  setFocus();
  selectAll();

  in_progress = false;
}

void FinderWidget::doSearching() {
  if (in_progress == false) {
    in_progress = true;
    QColor backgroundColor(QColor(255, 222, 187));
    QPalette pal(palette());
    pal.setColor(QPalette::Background, backgroundColor);
    setAutoFillBackground(true);
    setPalette(pal);


    QColor foregroundColor(QColor(0, 0, 0));
    pal = palette();
    pal.setColor(QPalette::Foreground, foregroundColor);
    setPalette(pal);
    RA::searchBegin(sysInfo.connection,
                    sysInfo.oofs_dir_rp,  // oofs directory
                    sysInfo.oofs_dir_rp,  // search path
                    text().toStdString(), // pattern
                    false,                // case sensitive
                    false                 // exact match
    );
  }

  QTime lastProgressUpdate;
  lastProgressUpdate.restart();
  QProgressDialog progress("Searching", "Cancel", 0, 1000, this);
  progress.setModal(true);
  int count = 0;
  while (1) {
    count++;
    if (lastProgressUpdate.elapsed() > 100) {
      lastProgressUpdate.restart();
      progress.setValue(1);
      progress.setLabelText(QString("Processed %1 objects").arg(count));
    }
    qApp->processEvents();
    if (progress.wasCanceled())
      break;
    // get the next match
    std::string path = RA::searchContinue(sysInfo.connection, false);
    if (path == "*") {
      // no more matches
      QApplication::beep();
      QColor backgroundColor(QColor(255, 0, 0));
      QPalette pal(palette());
      pal.setColor(QPalette::Background, backgroundColor);
      setAutoFillBackground(true);
      setPalette(pal);


      QColor foregroundColor(QColor(255, 255, 255));
      pal = palette();
      pal.setColor(QPalette::Foreground, foregroundColor);
      setAutoFillBackground(true);
      setPalette(pal);
      return;
    } else if (path == "**") {
      // more matches could be possibly found
      continue;
    } else {
      // match found, show it
      openNode(path.c_str());
      return;
    }
  }
}

// intercept key events to deal with Escape and Enter
// - on escape hide the widget
// - on return/enter start or continue searching
// - any other key --> pass up the event to QLineEdit's event handler
void FinderWidget::keyPressEvent(QKeyEvent *e) {
  if (e->key() == Qt::Key_Escape) {
    RA::searchEnd(sysInfo.connection);
    hide();
    setEnabled(false);
    e->accept();
  } else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
    doSearching();
    e->accept();
  } else {
    RA::searchEnd(sysInfo.connection);
    QLineEdit::keyPressEvent(e);
  }
}

// on losing focus we hide the widget
void FinderWidget::focusOutEvent(QFocusEvent *) {
  hide();
  setEnabled(false);
  RA::searchEnd(sysInfo.connection);
}

// callback on text changed
void FinderWidget::textChanged_cb() {
  QColor backgroundColor(QColor(255, 222, 187));
  QPalette pal(palette());
  pal.setColor(QPalette::Background, backgroundColor);
  setAutoFillBackground(true);
  setPalette(pal);


  QColor foregroundColor(QColor(0, 0, 0));
  pal = palette();
  pal.setColor(QPalette::Foreground, foregroundColor);
  in_progress = false;
  RA::searchEnd(sysInfo.connection);
}
