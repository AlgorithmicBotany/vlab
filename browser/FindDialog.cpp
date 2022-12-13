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



#include <QComboBox>
#include <QCursor>
#include <QLabel>
#include <iostream>
#include <stdio.h>
using namespace Qt;
#include "FindDialog.h"
#include "main.h"
#include "openNode.h"
#include "xstring.h"
#include "xutils.h"

// Constructor
FindDialog::FindDialog(QWidget *parent, const char *, bool )
    : QDialog(parent) {
  setupUi(this);
  findLine->setText("");
  setWindowTitle("Find");
  // set the 'find next' button to inactive
 
  startButton->setDefault(false);
  nextButton->setEnabled(false);
  previousButton->setEnabled(false);

  caseCheckBox->setChecked(false);
  exactCheckBox->setChecked(false);

  QObject::connect(startButton, &QPushButton::pressed, this,
                   &FindDialog::startSearchCB);

  QObject::connect(nextButton, &QPushButton::pressed, this,
                   &FindDialog::nextSearchCB);

  QObject::connect(previousButton, &QPushButton::pressed, this,
                   &FindDialog::previousSearchCB);

  QObject::connect(quitButton, &QPushButton::pressed, this,
                   &FindDialog::quitSearchCB);

  QObject::connect(findLine, SIGNAL(textChanged(const QString &)), this,
                   SLOT(resetSearch()));
  // startButton->setDefault(false);
  setUpdatesEnabled(true);
  setWindowFlag(Qt::WindowStaysOnTopHint);

}

/******************************************************************************
 *
 * startSearchCB - callback for the 'Start Search' button of the
 * find-dialog_box
 *
 */
void FindDialog::startSearchCB() {
  if (!sysInfo.connection->check_connection()) {
    vlabxutils::infoBox(sysInfo.mainForm,
                        "Search can't begin\n"
                        "Connection with raserver is down\n"
                        "Check your network connection\n",
                        "Error");
    return;
  }
  
  startButton->setDefault(false);
  previousButton->setDefault(false);
  nextButton->setDefault(false);
  quitButton->setDefault(false);

  nextButton->setEnabled(true);
  previousButton->setEnabled(true);

  startButton->setDown(false);
  quitButton->setDown(false);
  nextButton->setDown(false);
  previousButton->setDown(false);

  /* find the string in the tree */
  RA::searchBegin(sysInfo.connection, sysInfo.oofs_dir_rp, sysInfo.oofs_dir_rp,
                  findLine->text().toStdString(), caseCheckBox->isChecked(),
                  exactCheckBox->isChecked());
  // nextButton->setDefault(true);
  /* set the 'find next' button to active */

  std::string str1;
  response_list.clear();
  do {
    str1 = RA::searchContinue(sysInfo.connection, true);
    if (str1 != "*")
      response_list.push_back(str1);
  } while (str1 != "*");
  std::sort(response_list.begin(), response_list.end());
  response_list.push_back("*");
  current_object = response_list.begin();

  nextSearch(FirstSearch);
}

// Search next occuration of string
void FindDialog::nextSearchCB() {
  startButton->setDefault(false);
  previousButton->setDefault(false);
  quitButton->setDefault(false);
  nextButton->setEnabled(true);
  previousButton->setEnabled(true);
  nextButton->setDefault(false);

  startButton->setDown(false);
  quitButton->setDown(false);
  nextButton->setDown(false);
  previousButton->setDown(false);

  if (*current_object != "*")
    current_object++;
  nextSearch(SubsequentSearch);
}

// Search next occuration of string
void FindDialog::nextSearch(SearchState state) {
  sysInfo.mainForm->setCursor(WaitCursor);
  setCursor(WaitCursor);

  if (*current_object != "*") {
    std::string str = *current_object;
    sysInfo.mainForm->setCursor(ArrowCursor);
    setCursor(ArrowCursor);
    if (!openNode(str.c_str())) {
      current_object++;
      nextSearch(SubsequentSearch);
    }
  } else {
    sysInfo.mainForm->setCursor(ArrowCursor);
    setCursor(ArrowCursor);
    RA::searchEnd(sysInfo.connection);
    if (state == FirstSearch) {
      const char *prompt_text = "Could not find any matches.\n\nWould you like "
                                "to start a new search?\n";
      if (vlabxutils::askYesNo(sysInfo.mainForm, prompt_text) == 1)
        resetSearch();
      else
        quitSearchCB();
    } else {
      const char *prompt_text = "Could not find any more matches.\n\nWould you "
                                "like to restart the search?\n";
      if (vlabxutils::askYesNo(sysInfo.mainForm, prompt_text) == 1)
        startSearchCB();
      else
        quitSearchCB();
    }
  }
}

// Search next occuration of string
void FindDialog::previousSearchCB() {
  startButton->setDefault(false);
  previousButton->setDefault(false);
  nextButton->setDefault(false);
  quitButton->setDefault(false);

  nextButton->setEnabled(true);
  previousButton->setEnabled(true);

  startButton->setDown(false);
  quitButton->setDown(false);
  nextButton->setDown(false);
  previousButton->setDown(false);
  previousSearch(SubsequentSearch);
}

// Search previous occuration of string
void FindDialog::previousSearch(SearchState state) {
  sysInfo.mainForm->setCursor(WaitCursor);
  setCursor(WaitCursor);

  if (current_object != response_list.begin()) {
    current_object--;

    std::string str = *current_object;
    sysInfo.mainForm->setCursor(ArrowCursor);
    setCursor(ArrowCursor);
    if (!openNode(str.c_str())) {
      current_object--;
      previousSearch(state);
    }
  } else {
    sysInfo.mainForm->setCursor(ArrowCursor);
    setCursor(ArrowCursor);
    RA::searchEnd(sysInfo.connection);
    if (state == FirstSearch) {
      const char *prompt_text = "Could not find any matches.\n\nWould you like "
                                "to restart the search?\n";
      if (vlabxutils::askYesNo(sysInfo.mainForm, prompt_text) == 1)
        resetSearch();
      else
        quitSearchCB();
    } else {
      const char *prompt_text = "Could not find any more matches.\n\nWould you "
                                "like to restart the search?\n";
      if (vlabxutils::askYesNo(sysInfo.mainForm, prompt_text) == 1)
        startSearchCB();
      else
        quitSearchCB();
    }
  }
}

// Quit search
void FindDialog::quitSearchCB() {
  startButton->setDefault(false);
  previousButton->setDefault(false);
  nextButton->setDefault(false);
  quitButton->setDefault(false);

 
  nextButton->setEnabled(false);
  previousButton->setEnabled(false);

  startButton->setDown(false);
  quitButton->setDown(false);
  nextButton->setDown(false);
  previousButton->setDown(false);
  RA::searchEnd(sysInfo.connection);
  close();
}

// Reset serch
void FindDialog::resetSearch() {
  startButton->setDefault(false);
  previousButton->setDefault(false);
  nextButton->setDefault(false);
  quitButton->setDefault(false);

  startButton->setDown(false);
  quitButton->setDown(false);
  nextButton->setDown(false);
  previousButton->setDown(false);
}

void FindDialog::show(){
   startButton->setDefault(false);
  previousButton->setDefault(false);
  nextButton->setDefault(false);
  quitButton->setDefault(false);
  nextButton->setEnabled(false);
  previousButton->setEnabled(false);
  QDialog::show();
}
