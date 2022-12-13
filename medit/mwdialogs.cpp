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



/* MaterialWorld

   Implementation of Assorted Dialog Classes

 */

#include "mwdialogs.h"
#include <QCloseEvent>
#include <QGridLayout>
#include <QLabel>
using namespace Qt;

SelectRangeDialog::SelectRangeDialog(QWidget *parent, const char *,
                                     Qt::WindowFlags f)
    : QWidget(parent, f) {

  QGridLayout *lay = new QGridLayout(this);
  lay->setMargin(3);
  begin = new QSpinBox(this);
  begin->setMinimum(0);
  begin->setMaximum(255);
  begin->setSingleStep(1);

  end = new QSpinBox(this);
  end->setMinimum(0);
  end->setMaximum(255);
  end->setSingleStep(1);

  offset = new QSpinBox(this);
  offset->setMinimum(1);
  offset->setMaximum(256);
  offset->setSingleStep(1);

  QLabel *l1 = new QLabel("Start Index ", this);
  QLabel *l2 = new QLabel(" End Index ", this);
  QLabel *l3 = new QLabel("# Selected ", this);

  lay->addWidget(l1, 0, 0);
  lay->addWidget(begin, 0, 1);
  lay->addWidget(l2, 0, 2);
  lay->addWidget(end, 0, 3);
  lay->addWidget(l3, 1, 0);
  lay->addWidget(offset, 1, 1);

  okay = new QPushButton("OK", this);
  cancel = new QPushButton("Cancel", this);

  lay->addWidget(okay, 2, 0);
  lay->addWidget(cancel, 2, 1);

  QObject::connect(offset, SIGNAL(valueChanged(int)), this,
                   SLOT(newOffset(int)));
  QObject::connect(end, SIGNAL(valueChanged(int)), this, SLOT(newEnd(int)));
  QObject::connect(begin, SIGNAL(valueChanged(int)), this, SLOT(newBegin(int)));
  QObject::connect(okay, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
}

void SelectRangeDialog::getRange(int start, int off) {
  begin->setValue(start);
  offset->setValue(off);
  show();
}

void SelectRangeDialog::newOffset(int o) {
  end->setValue((begin->value() + o) - 1);
}

void SelectRangeDialog::newEnd(int e) {
  offset->setValue((e - begin->value()) + 1);
}

void SelectRangeDialog::newBegin(int b) {
  offset->setMaximum(256 - b);
  end->setMinimum(b);
  if (end->value() < b)
    end->setValue(b);
  else
    offset->setValue((end->value() - b) + 1);
}

void SelectRangeDialog::accept() {
  emit rangeEvent(begin->value(), offset->value());
}

void SelectRangeDialog::reject() { close(); }

void SelectRangeDialog::closeEvent(QCloseEvent*) { hide(); }

ImageDialog::ImageDialog(QWidget *parent, const char*, Qt::WindowFlags f)
    : QWidget(parent, f) {

  QGridLayout *G = new QGridLayout(this);
  G->setMargin(3);

  QLabel *l1 = new QLabel("Render Speed", this);
  l1->setAlignment(AlignRight);
  QLabel *l2 = new QLabel("Image Quality", this);

  slide = new QSlider(Qt::Horizontal, this);
  slide->setObjectName("slide");
  slide->setMinimum(0);
  slide->setMaximum(80);
  slide->setPageStep(8);
  slide->setValue(8);
  slide->setTickPosition(QSlider::TicksAbove);

  butt = new QPushButton("Close", this);
  savebutt = new QPushButton("Save", this);
  bg = new QGroupBox("Auto-adjust smoothness on:", this);
  bgButtonGroup = new QButtonGroup();

  rs = new QRadioButton("resize window", bg);
  rs->setChecked(false);
  bgButtonGroup->addButton(rs, 0);
  pc = new QRadioButton("change page size", bg);
  pc->setChecked(true);
  bgButtonGroup->addButton(pc, 1);

  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->addWidget(rs);
  vbox->addWidget(pc);
  vbox->addStretch(2);
  bg->setLayout(vbox);
  G->addWidget(l1, 0, 0);
  G->addWidget(slide, 0, 1);
  G->addWidget(l2, 0, 3);
  G->addItem(new QSpacerItem(0, 10), 1, 0);
  G->addWidget(bg, 2, 0);
  G->addWidget(savebutt, 4, 2);
  G->addWidget(butt, 4, 3);

  resize(350, 10);

  QObject::connect(slide, SIGNAL(valueChanged(int)), this,
                   SLOT(editQuality(int)));
  QObject::connect(butt, SIGNAL(clicked()), this, SLOT(close()));
  QObject::connect(bgButtonGroup, SIGNAL(buttonClicked(int)), this,
                   SLOT(setAdjust(int)));
  QObject::connect(savebutt, SIGNAL(clicked()), this, SLOT(save()));
}

void ImageDialog::getImageQuality(int Q, bool r, bool p) {
  slide->setValue(Q);
  rs->setChecked(r);
  pc->setChecked(p);
  show();
}

void ImageDialog::save() { emit saveconfig(); }

void ImageDialog::setAdjust(int A) {
  switch (A) {
  case 0:
    emit adjustResize(rs->isChecked());
    break;
  case 1:
    emit adjustPageSize(pc->isChecked());
    break;
  }
}

void ImageDialog::editQuality(int Q) { emit setSmoothness(Q); }

void ImageDialog::closeEvent(QCloseEvent*) { hide(); }

// eof: mwdialogs.cc
