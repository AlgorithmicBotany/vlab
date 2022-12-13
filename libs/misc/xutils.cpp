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



#include <QMessageBox>
#include <qcursor.h>

#include <assert.h>
#include <iostream>
#include <qapplication.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "QTtextDialog.h"
#include <QProgressDialog>

#include "xmemory.h"

#include "xutils.h"

namespace vlabxutils {

// ---------------------------------------------------------------------------
//
// pops up a modal window with the question, and returns the answer
// the user selects (1=YES, 0=NO)
//
// ---------------------------------------------------------------------------
bool askYesNo(QWidget *parent, const std::string &question,
              const std::string &title) {
  while (1) {
    int res = QMessageBox::question(parent, title.c_str(), question.c_str(),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::NoButton);
    if (res == QMessageBox::Yes)
      return true;
    if (res == QMessageBox::No)
      return false;
  }
}

// ---------------------------------------------------------------------------
//
// pops up an information box with message 'message' and title 'title',
// and then waits for the user to press 'OK' button. Then returns.
//
// ---------------------------------------------------------------------------
void infoBox(QWidget *parent, const std::string &message,
             const std::string &title) {
  QMessageBox::information(parent, QString(title.c_str()),
                           QString(message.c_str()),
                           QMessageBox::Ok | QMessageBox::Default);
}

// ---------------------------------------------------------------------------
//
// this is a convenience function for infoBox()
//
// pops up an information box with message with title 'title', and message
// 'message' (printf formatting standard is used, e.g.
//
//    popupInfoBox( top_shell, "Warning", "Invalid number %d.", num);
//
//
// ---------------------------------------------------------------------------
void popupInfoBox(QWidget *parent, const char *title, const char *message,
                  ...) {
  // preconditions
  if (title == NULL)
    title = "Info:";
  assert(message != NULL);

  // *** prepare the arguments for the dialog **************

  // prepare the message
  char buf[4096];
  va_list ap;

  va_start(ap, message);
  vsprintf(buf, message, ap);
  va_end(ap);

  // create the dialog
  infoBox(parent, buf, title);
}

static QProgressDialog *progressDialog = NULL;

// ---------------------------------------------------------------------------
// pops up a working dialog, and changes the cursor to busy
// ---------------------------------------------------------------------------
void tempBoxPopUp(int, int // position
                  ,
                  QWidget *parent // parent
                  ,
                  const char *message // message
                  ,
                  const char *title) // title
{
  if (progressDialog != NULL)
    delete progressDialog;
  if (title == NULL)
    title = "Please wait";
  progressDialog = new QProgressDialog(title, "no cancel", 0, 100, parent);
  if (progressDialog == NULL) {
    std::cerr << "Cannot create temporary dialog:\n"
              << "\t- title = " << title << "\n"
              << "\t- message = " << message << "\n";
    return;
  }
  // disable cancel button
  progressDialog->setCancelButton(NULL);
  progressDialog->setWindowTitle(title);
  progressDialog->setLabelText(message);
  progressDialog->show();
  progressDialog->setValue(0);
  qApp->processEvents();
  usleep(100000);
  qApp->processEvents();
}

void setProgress(double val) {
  if (progressDialog) {
    int newVal = int(val * 100);
    progressDialog->setValue(newVal);
  }
}

void tempBoxPopDown(QWidget *)
// ======================================================================
// pops down the working dialog, and changes the cursor to normal
// ......................................................................
{
  progressDialog->hide();
  delete progressDialog;
  progressDialog = NULL;
}

} // namespace
