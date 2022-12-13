/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "ScaleWindow.h"

#include <QSizePolicy>
#include <QCheckBox>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QLayout>
#include <QLabel>
ScaleWindow::ScaleWindow(QWidget *parent) : QDialog(parent) {

  xbox = createSpinBox();
  connect(xbox, SIGNAL(valueChanged(double)), this, SLOT(editedX()));
  ybox = createSpinBox();
  connect(ybox, SIGNAL(valueChanged(double)), this, SLOT(editedY()));
  zbox = createSpinBox();
  connect(zbox, SIGNAL(valueChanged(double)), this, SLOT(editedZ()));

  okButton = new QPushButton("&OK", this);
  cancelButton = new QPushButton("&Cancel", this);
  connect(okButton, SIGNAL(clicked()), this, SLOT(confirm()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));

  previewBox = new QCheckBox("Preview", this);
  previewBox->setChecked(true);
  preview = true;
  connect(previewBox, SIGNAL(toggled(bool)), this, SLOT(setPreview(bool)));

  uniformBox = new QCheckBox("Uniform", this);
  uniformBox->setChecked(false);
  uniform = false;
  connect(uniformBox, SIGNAL(toggled(bool)), this, SLOT(setUniform(bool)));

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
  spinBoxLayout->addWidget(uniformBox, 1, 2);

  buttonLayout->addWidget(okButton);
  buttonLayout->addWidget(cancelButton);

  setLayout(mainLayout);
  setWindowTitle(tr("Scale"));
}

ScaleWindow::~ScaleWindow() {}

QSize ScaleWindow::sizeHint() const { return QSize(10, 10); }

void ScaleWindow::reset() {
  xbox->setValue(1);
  ybox->setValue(1);
  zbox->setValue(1);
}

// Slots

// Confirm the changes made in the scale window, updating the patch to the
// modified versions
void ScaleWindow::confirm() {
  emit(update(Vector3(xbox->value(), ybox->value(), zbox->value())));
  emit(accept());
}

// Reject the changes made in the scale window, restoring them to their original
// values before editing
void ScaleWindow::cancel() {
  emit(update(Vector3(1, 1, 1)));
  emit(reject());
}

// Indicates whether to preview the parameters in the window, and shows or hides
// the changes when toggled
void ScaleWindow::setPreview(bool value) {
  preview = value;
  if (!preview)
    emit(update(Vector3(1, 1, 1)));
  else
    emit(update(Vector3(xbox->value(), ybox->value(), zbox->value())));
}

// Indicates whether to preview the parameters in the window, and shows or hides
// the changes when toggled
void ScaleWindow::setUniform(bool value) { uniform = value; }

void ScaleWindow::editedX() {
  if (uniform) { // If uniform scaling is on, update the other boxes to match
                 // the X box
    ybox->setValue(xbox->value());
    zbox->setValue(xbox->value());
  }
  if (preview)
    emit(update(Vector3(xbox->value(), ybox->value(),
                        zbox->value()))); // If preview is enabled, update
}

void ScaleWindow::editedY() {
  if (uniform) { // If uniform scaling is on, update the other boxes to match
                 // the Y box
    xbox->setValue(ybox->value());
    zbox->setValue(ybox->value());
  }
  if (preview)
    emit(update(Vector3(xbox->value(), ybox->value(),
                        zbox->value()))); // If preview is enabled, update
}

void ScaleWindow::editedZ() {
  if (uniform) { // If uniform scaling is on, update the other boxes to match
                 // the Z box
    xbox->setValue(zbox->value());
    ybox->setValue(zbox->value());
  }
  if (preview)
    emit(update(Vector3(xbox->value(), ybox->value(),
                        zbox->value()))); // If preview is enabled, update
}

// Auxilliary functions

// Creates a QDoubleSpinBox with steps of 0.1 and a range from -10 000 to 10 000
QDoubleSpinBox *ScaleWindow::createSpinBox() {
  QDoubleSpinBox *box = new QDoubleSpinBox();
  box->setSingleStep(0.01);
  box->setRange(0.01, 1000);
  box->setValue(1);
  return box;
}
