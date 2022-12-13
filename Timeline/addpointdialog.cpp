#include "addpointdialog.h"

#include "timeline.h"

#include <QLabel>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QColorDialog>

#include <limits.h>
#include "colors.h"

addPointDialog::addPointDialog(QWidget *parent) : QDialog(parent) {
  // Construct the dialog box and layout the widgets in the window
  setWindowFlags(Qt::WindowStaysOnTopHint);
  nameLabel = new QLabel(tr("Timeline name: "));
  name = new QLineEdit;
  name->setText("noname");
  nameLabel->setBuddy(name);
  _color =  QColor(GetQColor(Segments));

  startTimeLabel = new QLabel(tr("Start time: "));
  startTime = new QDoubleSpinBox;
  startTime->setRange(INT_MIN, INT_MAX);
  startTimeLabel->setBuddy(startTime);

  endTimeLabel = new QLabel(tr("End time: "));
  endTime = new QDoubleSpinBox;
  endTime->setValue(1.0);
  endTime->setRange(INT_MIN, INT_MAX);
  endTimeLabel->setBuddy(endTime);

  startLabel = new QLabel(tr("Start label: "));
  start = new QLineEdit;
  startLabel->setBuddy(startLabel);

  endLabel = new QLabel(tr("End label: "));
  end = new QLineEdit;
  endLabel->setBuddy(endLabel);

  colorButtonLabel = new QLabel(tr("Color: "));

  colorButton = new QPushButton("", this);
  setColor(_color);
  connect(colorButton, SIGNAL(clicked()), this, SLOT(colorButtonClicked()));


  QHBoxLayout *buttonLayout = new QHBoxLayout;

   cancel = new QPushButton(tr("Cancel"));

  connect(cancel, SIGNAL(clicked()), this, SLOT(close()));
   confirm = new QPushButton(tr("OK"));

  connect(confirm, SIGNAL(clicked()), this, SLOT(confirmClicked()));
  buttonLayout->addWidget(cancel);
  buttonLayout->addWidget(confirm);
  QGroupBox *formGroupBox = new QGroupBox();
  QFormLayout *formLayout = new QFormLayout;
  formLayout->setLabelAlignment(Qt::AlignLeft);
  formLayout->setFormAlignment(Qt::AlignLeft);
  formLayout->addRow(nameLabel, name);
  formLayout->addRow(startTimeLabel, startTime);
  formLayout->addRow(endTimeLabel, endTime);
  formLayout->addRow(startLabel, start);
  formLayout->addRow(endLabel, end);
  formLayout->addRow(colorButtonLabel, colorButton);

  formGroupBox->setLayout(formLayout);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  formLayout->setAlignment(Qt::AlignLeft);
  mainLayout->addWidget(formGroupBox);
  mainLayout->addLayout(buttonLayout);

  setLayout(mainLayout);

  setWindowTitle(tr("Add timeline"));
  setFixedHeight(sizeHint().height());
}

void addPointDialog::confirmClicked() {
  QString color = QString::number(_color.red()) + QString(",") + QString::number(_color.green()) + QString(",") + QString::number(_color.blue());
  // Create an event when confirm is clicked
  timeline->createEvent(startTime->value(), endTime->value(), start->text(),
                        end->text(), name->text(), color);
  this->close();
}

void addPointDialog::colorButtonClicked() {

  _previousColor1 = _color;

  QColorDialog *dialog = new QColorDialog(_color);
  connect(dialog, SIGNAL(currentColorChanged(QColor)), this,
          SLOT(colorSelectedFromDialog(QColor)));
  connect(dialog, SIGNAL(rejected()), this, SLOT(colorRejectedFromDialog()));

  dialog->exec();
}

void addPointDialog::colorSelectedFromDialog(QColor ret) {

  if (ret.isValid()) {
    _color = ret;
  }
  setColor(_color);
}

void addPointDialog::colorRejectedFromDialog() {

  if (_previousColor1.isValid()) {
    _color = _previousColor1;
  }
  setColor(_color);

}

void addPointDialog::setColor(QColor c) {
  if (c.isValid()) {
    QString qss = QString("background-color: %1").arg(c.name());
    colorButton->setStyleSheet(qss);
  }
}
