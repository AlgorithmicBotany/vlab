/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "TranslateWindow.h"

#include <QSizePolicy>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>

TranslateWindow::TranslateWindow(QWidget *parent) : QDialog(parent) {

  xbox = createSpinBox();
  connect(xbox, SIGNAL(valueChanged(double)), this, SLOT(edited()));
  ybox = createSpinBox();
  connect(ybox, SIGNAL(valueChanged(double)), this, SLOT(edited()));
  zbox = createSpinBox();
  connect(zbox, SIGNAL(valueChanged(double)), this, SLOT(edited()));

  okButton = new QPushButton("&OK", this);
  cancelButton = new QPushButton("&Cancel", this);
  connect(okButton, SIGNAL(clicked()), this, SLOT(confirm()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));

  previewBox = new QCheckBox("Preview", this);
  previewBox->setChecked(true);
  preview = true;
  connect(previewBox, SIGNAL(toggled(bool)), this, SLOT(setPreview(bool)));

  // Create the layouts
  QVBoxLayout *mainLayout = new QVBoxLayout;
  QHBoxLayout *buttonLayout = new QHBoxLayout;
  QGridLayout *spinBoxLayout = new QGridLayout;

  mainLayout->addLayout(spinBoxLayout);
  mainLayout->addLayout(buttonLayout);

  QLabel *xlbl = new QLabel("X");
  QLabel *ylbl = new QLabel("Y");
  QLabel *zlbl = new QLabel("Z");
  xlbl->setMaximumSize(QSize(10, 10));
  ylbl->setMaximumSize(QSize(10, 10));
  zlbl->setMaximumSize(QSize(10, 10));

  spinBoxLayout->addWidget(xlbl);
  spinBoxLayout->addWidget(xbox, 0, 1);
  spinBoxLayout->addWidget(ylbl);
  spinBoxLayout->addWidget(ybox, 1, 1);
  spinBoxLayout->addWidget(zlbl);
  spinBoxLayout->addWidget(zbox, 2, 1);
  spinBoxLayout->addWidget(previewBox, 0, 2);

  buttonLayout->addWidget(okButton);
  buttonLayout->addWidget(cancelButton);

  setLayout(mainLayout);
  setWindowTitle(tr("Translate"));
}

TranslateWindow::~TranslateWindow() {}

QSize TranslateWindow::sizeHint() const { return QSize(10, 10); }

void TranslateWindow::reset() {
  xbox->setValue(0);
  ybox->setValue(0);
  zbox->setValue(0);
}

// Slots

// Confirm the changes made in the translate window, updating the patch to the
// modified version
void TranslateWindow::confirm() {
  emit(update(Vector3(xbox->value(), ybox->value(), zbox->value())));
  emit(accept());
}

// Reject the changes made in the translate window, restoring them to their
// original values before editing
void TranslateWindow::cancel() {
  emit(update(Vector3()));
  emit(reject());
}

// Indicates whether to preview the parameters in the window, and shows or hides
// the changes when toggled
void TranslateWindow::setPreview(bool value) {
  preview = value;
  if (!preview)
    emit(update(Vector3()));
  else
    emit(update(Vector3(xbox->value(), ybox->value(), zbox->value())));
}

// Update the translation if one of the boxes was edited
void TranslateWindow::edited() {
  if (preview)
    emit(update(Vector3(xbox->value(), ybox->value(), zbox->value())));
}

// Auxilliary functions

// Creates a QDoubleSpinBox with steps of 0.1 and a range from -10 000 to 10 000
QDoubleSpinBox *TranslateWindow::createSpinBox() {
  QDoubleSpinBox *box = new QDoubleSpinBox();
  box->setSingleStep(0.1);
  box->setRange(-10000, 10000);
  box->setDecimals(6);
  return box;
}
