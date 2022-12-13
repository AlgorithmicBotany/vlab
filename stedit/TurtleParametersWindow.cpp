/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "TurtleParametersWindow.h"

#include <QSizePolicy>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>

TurtleParametersWindow::TurtleParametersWindow(QWidget *parent)
    : QDialog(parent) {

  contactPointXbox = createSpinBox();
  connect(contactPointXbox, SIGNAL(valueChanged(double)), this,
          SLOT(contactPointEdited()));
  contactPointYbox = createSpinBox();
  connect(contactPointYbox, SIGNAL(valueChanged(double)), this,
          SLOT(contactPointEdited()));
  contactPointZbox = createSpinBox();
  connect(contactPointZbox, SIGNAL(valueChanged(double)), this,
          SLOT(contactPointEdited()));
  endPointXbox = createSpinBox();
  connect(endPointXbox, SIGNAL(valueChanged(double)), this,
          SLOT(endPointEdited()));
  endPointYbox = createSpinBox();
  connect(endPointYbox, SIGNAL(valueChanged(double)), this,
          SLOT(endPointEdited()));
  endPointZbox = createSpinBox();
  connect(endPointZbox, SIGNAL(valueChanged(double)), this,
          SLOT(endPointEdited()));
  headingXbox = createSpinBox();
  connect(headingXbox, SIGNAL(valueChanged(double)), this,
          SLOT(headingEdited()));
  headingYbox = createSpinBox();
  connect(headingYbox, SIGNAL(valueChanged(double)), this,
          SLOT(headingEdited()));
  headingZbox = createSpinBox();
  connect(headingZbox, SIGNAL(valueChanged(double)), this,
          SLOT(headingEdited()));
  upXbox = createSpinBox();
  connect(upXbox, SIGNAL(valueChanged(double)), this, SLOT(upEdited()));
  upYbox = createSpinBox();
  connect(upYbox, SIGNAL(valueChanged(double)), this, SLOT(upEdited()));
  upZbox = createSpinBox();
  connect(upZbox, SIGNAL(valueChanged(double)), this, SLOT(upEdited()));
  sizeBox = createSpinBox();
  connect(sizeBox, SIGNAL(valueChanged(double)), this, SLOT(sizeEdited()));
  sizeBox->setRange(0, 10000);

  okButton = new QPushButton("&OK", this);
  cancelButton = new QPushButton("&Cancel", this);
  resetButton = new QPushButton("&Reset", this);
  connect(okButton, SIGNAL(clicked()), this, SLOT(confirm()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));
  connect(this, SIGNAL(buttonPressed()), this, SLOT(close()));

  previewBox = new QCheckBox("Preview", this);
  previewBox->setChecked(true);
  preview = true;
  connect(previewBox, SIGNAL(toggled(bool)), this, SLOT(setPreview(bool)));

  normalizeBox = new QCheckBox("Normalize", this);
  normalizeBox->setChecked(true);
  normalize = true;
  connect(normalizeBox, SIGNAL(toggled(bool)), this, SLOT(setNormalize(bool)));

  orthagonalizeHeadingBox = new QCheckBox("Orthogonalize", this);
  orthagonalizeHeadingBox->setChecked(false);
  orthagonalizeHeading = false;
  connect(orthagonalizeHeadingBox, SIGNAL(toggled(bool)), this,
          SLOT(setOrthagonalizeHeading(bool)));

  orthagonalizeUpBox = new QCheckBox("Orthogonalize", this);
  orthagonalizeUpBox->setChecked(false);
  orthagonalizeUp = false;
  connect(orthagonalizeUpBox, SIGNAL(toggled(bool)), this,
          SLOT(setOrthagonalizeUp(bool)));

  // Create the layouts
  QVBoxLayout *mainLayout = new QVBoxLayout;
  QHBoxLayout *buttonLayout = new QHBoxLayout;
  QGridLayout *spinBoxLayout = new QGridLayout;

  mainLayout->addLayout(spinBoxLayout);
  mainLayout->addLayout(buttonLayout);

  spinBoxLayout->addWidget(new QLabel("Contact Point"), 0, 0);
  spinBoxLayout->addWidget(contactPointXbox, 0, 1);
  spinBoxLayout->addWidget(contactPointYbox, 0, 2);
  spinBoxLayout->addWidget(contactPointZbox, 0, 3);
  spinBoxLayout->addWidget(new QLabel("End Point"), 1, 0);
  spinBoxLayout->addWidget(endPointXbox, 1, 1);
  spinBoxLayout->addWidget(endPointYbox, 1, 2);
  spinBoxLayout->addWidget(endPointZbox, 1, 3);
  spinBoxLayout->addWidget(new QLabel("Heading Vector"), 2, 0);
  spinBoxLayout->addWidget(headingXbox, 2, 1);
  spinBoxLayout->addWidget(headingYbox, 2, 2);
  spinBoxLayout->addWidget(headingZbox, 2, 3);
  spinBoxLayout->addWidget(orthagonalizeHeadingBox, 2, 4);
  spinBoxLayout->addWidget(new QLabel("Up Vector"), 3, 0);
  spinBoxLayout->addWidget(upXbox, 3, 1);
  spinBoxLayout->addWidget(upYbox, 3, 2);
  spinBoxLayout->addWidget(upZbox, 3, 3);
  spinBoxLayout->addWidget(orthagonalizeUpBox, 3, 4);
  spinBoxLayout->addWidget(new QLabel("Size"), 4, 0);
  spinBoxLayout->addWidget(sizeBox, 4, 1);
  spinBoxLayout->addWidget(previewBox, 4, 2);
  spinBoxLayout->addWidget(normalizeBox, 4, 3);

  buttonLayout->addWidget(okButton);
  buttonLayout->addWidget(cancelButton);
  buttonLayout->addWidget(resetButton);

  setLayout(mainLayout);
  setWindowTitle(tr("Contact Point"));
}

TurtleParametersWindow::~TurtleParametersWindow() {}

QSize TurtleParametersWindow::sizeHint() const { return QSize(10, 10); }

// Remembers the given contact point and sets the values of the boxes to match
void TurtleParametersWindow::setContactPoint(Point point) {
  previousContactPoint = point;
  contactPointXbox->setValue(point.X());
  contactPointYbox->setValue(point.Y());
  contactPointZbox->setValue(point.Z());
}

// Remembers the given end point and sets the values of the boxes to match
void TurtleParametersWindow::setEndPoint(Point point) {
  previousEndPoint = point;
  endPointXbox->setValue(point.X());
  endPointYbox->setValue(point.Y());
  endPointZbox->setValue(point.Z());
}

// Remembers the given heading vector and sets the values of the boxes to match
void TurtleParametersWindow::setHeading(Vector3 vector) {
  previousHeading = vector;
  headingXbox->setValue(vector.X());
  headingYbox->setValue(vector.Y());
  headingZbox->setValue(vector.Z());
}

// Remembers the given up vector and sets the values of the boxes to match
void TurtleParametersWindow::setUp(Vector3 vector) {
  previousUp = vector;
  upXbox->setValue(vector.X());
  upYbox->setValue(vector.Y());
  upZbox->setValue(vector.Z());
}

// Remembers the given size value and sets the values of the boxes to match
void TurtleParametersWindow::setSize(double value) {
  previousSize = value;
  sizeBox->setValue(value);
}

// Slots

// Confirm the changes made in the turtle parameters window, updating all the
// parameters in the bezier editor to the modified versions
void TurtleParametersWindow::confirm() {

  Vector3 heading =
      Vector3(headingXbox->value(), headingYbox->value(), headingZbox->value());
  Vector3 up = Vector3(upXbox->value(), upYbox->value(), upZbox->value());
  if (normalize) {
    heading = heading.normalize();
    up = up.normalize();
  }
  if (orthagonalizeHeading) {
    double headingMag = heading.getMagnitude(); // Save the vector's magnitude
    Vector3 orthagonal = Vector3::cross(
        heading, up); // Get the orthagonal to the up and heading vectors
    orthagonal = orthagonal.normalize();
    heading = Vector3::cross(
        up, orthagonal); // Make the heading vector orthagonal to the up vector
    heading =
        (heading.normalize()) * headingMag; // Preserve the vector's magnitude
  } else if (orthagonalizeUp) {
    double upMag = up.getMagnitude(); // Save the vector's magnitude
    Vector3 orthagonal = Vector3::cross(heading, up);
    orthagonal = orthagonal.normalize();
    up = Vector3::cross(
        orthagonal,
        heading); // Make the up vector orthagonal to the heading vector
    up = (up.normalize()) * upMag; // Preserve the vector's magnitude
  }

  emit(updateContactPoint(Point(contactPointXbox->value(),
                                contactPointYbox->value(),
                                contactPointZbox->value())));
  emit(updateEndPoint(Point(endPointXbox->value(), endPointYbox->value(),
                            endPointZbox->value())));
  emit(updateHeading(heading));
  emit(updateUp(up));
  emit(updateSize(sizeBox->value()));
  emit(accept());
}

// Reject the changes made in the turtle parameters window, restoring them to
// their original values before editing
void TurtleParametersWindow::cancel() {
  emit(updateContactPoint(previousContactPoint));
  emit(updateEndPoint(previousEndPoint));
  emit(updateHeading(previousHeading));
  emit(updateUp(previousUp));
  emit(updateSize(previousSize));
  emit(reject());
}

// Reset the turtle parameters to their original values
void TurtleParametersWindow::reset() {
  QMessageBox box;
  box.setWindowTitle("Reset Parameters");
  box.setIcon(QMessageBox::Question);
  box.setText("Reset all turtle parameters to previous values?");
  box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  box.setDefaultButton(QMessageBox::Cancel);
  box.setEscapeButton(QMessageBox::Cancel);
  int choice = box.exec();
  switch (choice) {
  case QMessageBox::Ok: // Ok was clicked
    emit(updateContactPoint(previousContactPoint));
    emit(updateEndPoint(previousEndPoint));
    emit(updateHeading(previousHeading));
    emit(updateUp(previousUp));
    emit(updateSize(previousSize));
    contactPointXbox->setValue(previousContactPoint.X());
    contactPointYbox->setValue(previousContactPoint.Y());
    contactPointZbox->setValue(previousContactPoint.Z());
    endPointXbox->setValue(previousEndPoint.X());
    endPointYbox->setValue(previousEndPoint.Y());
    endPointZbox->setValue(previousEndPoint.Z());
    headingXbox->setValue(previousHeading.X());
    headingYbox->setValue(previousHeading.Y());
    headingZbox->setValue(previousHeading.Z());
    upXbox->setValue(previousUp.X());
    upYbox->setValue(previousUp.Y());
    upZbox->setValue(previousUp.Z());
    sizeBox->setValue(previousSize);
    break;
  case QMessageBox::Cancel: // Cancel was clicked
    break;
  default: // should never be reached
    break;
  }
}

// Indicates whether to preview the parameters in the window, and shows or hides
// the changes when toggled
void TurtleParametersWindow::setPreview(bool value) {
  preview = value;
  if (!preview) {
    emit(updateContactPoint(previousContactPoint));
    emit(updateEndPoint(previousEndPoint));
    emit(updateHeading(previousHeading));
    emit(updateUp(previousUp));
    emit(updateSize(previousSize));
  } else {
    emit(updateContactPoint(Point(contactPointXbox->value(),
                                  contactPointYbox->value(),
                                  contactPointZbox->value())));
    emit(updateEndPoint(Point(endPointXbox->value(), endPointYbox->value(),
                              endPointZbox->value())));
    emit(updateHeading(Vector3(headingXbox->value(), headingYbox->value(),
                               headingZbox->value())));
    emit(updateUp(Vector3(upXbox->value(), upYbox->value(), upZbox->value())));
    emit(updateSize(sizeBox->value()));
  }
}

// Indicates whether to normalize the heading and up vectors on confirm
void TurtleParametersWindow::setNormalize(bool value) { normalize = value; }

// Indicates whether to orthagonalize the heading vector on confirm
void TurtleParametersWindow::setOrthagonalizeHeading(bool value) {
  orthagonalizeHeading = value;
  orthagonalizeUp =
      orthagonalizeUp && !orthagonalizeHeading; // Can't do both at once
  orthagonalizeUpBox->setChecked(orthagonalizeUp);
}

// Indicates whether to orthagonalize the up vector on confirm
void TurtleParametersWindow::setOrthagonalizeUp(bool value) {
  orthagonalizeUp = value;
  orthagonalizeHeading =
      orthagonalizeHeading && !orthagonalizeUp; // Can't do both at once
  orthagonalizeHeadingBox->setChecked(orthagonalizeHeading);
}

void TurtleParametersWindow::contactPointEdited() {
  if (preview)
    emit(updateContactPoint(Point(contactPointXbox->value(),
                                  contactPointYbox->value(),
                                  contactPointZbox->value())));
}

void TurtleParametersWindow::endPointEdited() {
  if (preview)
    emit(updateEndPoint(Point(endPointXbox->value(), endPointYbox->value(),
                              endPointZbox->value())));
}

void TurtleParametersWindow::headingEdited() {
  if (preview)
    emit(updateHeading(Vector3(headingXbox->value(), headingYbox->value(),
                               headingZbox->value())));
}

void TurtleParametersWindow::upEdited() {
  if (preview)
    emit(updateUp(Vector3(upXbox->value(), upYbox->value(), upZbox->value())));
}

void TurtleParametersWindow::sizeEdited() {
  if (preview)
    emit(updateSize(sizeBox->value()));
}

// Auxilliary functions

// Creates a QDoubleSpinBox with steps of 0.1 and a range from -10 000 to 10 000
QDoubleSpinBox *TurtleParametersWindow::createSpinBox() {
  QDoubleSpinBox *box = new QDoubleSpinBox();
  box->setSingleStep(0.1);
  box->setRange(-10000, 10000);
  return box;
}
