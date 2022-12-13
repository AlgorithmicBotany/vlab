/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "ResizeImageDialog.h"

#include <QSizePolicy>
#include <QLayout>
#include <QLabel>
ResizeImageDialog::ResizeImageDialog(int width, int height, QWidget *parent)
    : QDialog(parent) {

  wbox = createSpinBox();
  wbox->setValue(width);
  hbox = createSpinBox();
  hbox->setValue(height);

  okButton = new QPushButton("&OK", this);
  cancelButton = new QPushButton("&Cancel", this);
  connect(okButton, SIGNAL(clicked()), this, SLOT(confirmed()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

  // Create the layouts
  QVBoxLayout *mainLayout = new QVBoxLayout;
  QHBoxLayout *buttonLayout = new QHBoxLayout;
  QGridLayout *spinBoxLayout = new QGridLayout;

  mainLayout->addLayout(spinBoxLayout);
  mainLayout->addLayout(buttonLayout);

  QLabel *xlbl = new QLabel("Width");
  QLabel *ylbl = new QLabel("Height");
  xlbl->setMaximumSize(QSize(50, 10));
  ylbl->setMaximumSize(QSize(50, 10));

  spinBoxLayout->addWidget(xlbl);
  spinBoxLayout->addWidget(wbox, 0, 1);
  spinBoxLayout->addWidget(ylbl);
  spinBoxLayout->addWidget(hbox, 1, 1);

  buttonLayout->addWidget(okButton);
  buttonLayout->addWidget(cancelButton);

  setLayout(mainLayout);
  setWindowTitle(tr("Resize Image"));
}

ResizeImageDialog::~ResizeImageDialog() {}

QSize ResizeImageDialog::sizeHint() const { return QSize(10, 10); }

// Slots
void ResizeImageDialog::confirmed() {
  emit(result(wbox->value(), hbox->value()));
  accept();
}

// Auxilliary functions

// Creates a QDoubleSpinBox with steps of 0.1 and a range from -10 000 to 10 000
QSpinBox *ResizeImageDialog::createSpinBox() {
  QSpinBox *box = new QSpinBox();
  box->setSingleStep(1);
  box->setRange(1, 10000);
  return box;
}
