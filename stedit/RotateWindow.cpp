/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "RotateWindow.h"

#include <QSizePolicy>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>

RotateWindow::RotateWindow(QWidget *parent) : QDialog(parent) {

  axisXbox = createSpinBox();
  connect(axisXbox, SIGNAL(valueChanged(double)), this, SLOT(edited()));
  axisYbox = createSpinBox();
  connect(axisYbox, SIGNAL(valueChanged(double)), this, SLOT(edited()));
  axisZbox = createSpinBox();
  connect(axisZbox, SIGNAL(valueChanged(double)), this, SLOT(edited()));
  angleBox = new QDoubleSpinBox();
  angleBox->setSingleStep(1);
  angleBox->setRange(-360, 360);
  connect(angleBox, SIGNAL(valueChanged(double)), this, SLOT(edited()));

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

  spinBoxLayout->addWidget(new QLabel("Axis"), 0, 0);
  spinBoxLayout->addWidget(axisXbox, 1, 0);
  spinBoxLayout->addWidget(axisYbox, 2, 0);
  spinBoxLayout->addWidget(axisZbox, 3, 0);
  spinBoxLayout->addWidget(new QLabel("Angle"), 0, 1);
  spinBoxLayout->addWidget(angleBox, 1, 1);
  spinBoxLayout->addWidget(previewBox, 2, 1);

  buttonLayout->addWidget(okButton);
  buttonLayout->addWidget(cancelButton);

  setLayout(mainLayout);
  setWindowTitle(tr("Rotate"));
}

RotateWindow::~RotateWindow() {}

QSize RotateWindow::sizeHint() const { return QSize(10, 10); }

void RotateWindow::reset() {
  axisXbox->setValue(0);
  axisYbox->setValue(1);
  axisZbox->setValue(0);
  angleBox->setValue(0);
}

// Slots

// Confirm the changes made in the rotation window, updating the patch to the
// modified version
void RotateWindow::confirm() {
  float x = axisXbox->value();
  float y = axisYbox->value();
  float z = axisZbox->value();
  float a = angleBox->value();
  emit(update(Vector3(x,y,z),a));
  emit(accept());
}

// Reject the changes made in the rotation window, restoring them to their
// original values before editing
void RotateWindow::cancel() {
  emit(update(Vector3(0, 1, 0), 0));
  emit(reject());
}

// Indicates whether to preview the parameters in the window, and shows or hides
// the changes when toggled
void RotateWindow::setPreview(bool value) {
  preview = value;
  if (preview)
    emit(
	 update(Vector3(axisXbox->value(), axisYbox->value(), axisZbox->value()),
		angleBox->value()));
}

void RotateWindow::edited() {
  if (preview){
    float x = axisXbox->value();
    float y = axisYbox->value();
    float z = axisZbox->value();
    float a = angleBox->value();
    emit(update(Vector3(x,y,z),a));
  }
}

// Auxilliary functions

// Creates a QDoubleSpinBox with steps of 0.1 and a range from -10 000 to 10 000
QDoubleSpinBox *RotateWindow::createSpinBox() {
  QDoubleSpinBox *box = new QDoubleSpinBox();
  box->setSingleStep(0.1);
  box->setRange(-10000, 10000);
  box->setDecimals(6);
  return box;
}
