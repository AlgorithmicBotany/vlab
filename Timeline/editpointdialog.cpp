#include "editpointdialog.h"

#include "timeline.h"

#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QColorDialog>
#include "colors.h"
#include <limits.h>

editpointdialog::editpointdialog(int index, QWidget *parent) : QDialog(parent) {
  // Construct the dialog box and layout the widgets in the window
  _index = index;
  startTimeLabel = new QLabel(tr("Start time: "));
  startTime = new QDoubleSpinBox;
  startTime->setRange(INT_MIN, INT_MAX);
  startTimeLabel->setBuddy(startTime);

  endTimeLabel = new QLabel(tr("End time: "));
  endTime = new QDoubleSpinBox;
  endTime->setRange(INT_MIN, INT_MAX);
  endTimeLabel->setBuddy(endTime);

  startLabel = new QLabel(tr("Start label: "));
  start = new QLineEdit;
  startLabel->setBuddy(startLabel);

  endLabel = new QLabel(tr("End label: "));
  end = new QLineEdit;
  endLabel->setBuddy(endLabel);

  nameLabel = new QLabel(tr("Timeline name: "));
  name = new QLineEdit;
  nameLabel->setBuddy(name);

  colorButtonLabel = new QLabel(tr("Color: "));

  colorButton = new QPushButton("", this);
  _color =  QColor(GetQColor(Segments));
 
  _initialColor = _color;

  setColor(_color);
  connect(colorButton, SIGNAL(clicked()), this, SLOT(colorButtonClicked()));


  QHBoxLayout *buttonLayout = new QHBoxLayout;

  confirm = new QPushButton(tr("OK"));
  cancel = new QPushButton(tr("Cancel"));

  connect(confirm, SIGNAL(clicked()), this, SLOT(confirmClicked()));
  connect(cancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
  buttonLayout->addWidget(confirm);
  buttonLayout->addWidget(cancel);
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

  setWindowTitle(tr("Edit timeline"));
  setFixedHeight(sizeHint().height());
}

void editpointdialog::confirmClicked() {
  QString color = QString::number(_color.red()) + QString(",") + QString::number(_color.green()) + QString(",") + QString::number(_color.blue());
  // Edit then event when confirm is clicked
  timeline->editEvent(_index,startTime->value(), endTime->value(), start->text(),
                        end->text(), name->text(), color);
  this->close();
}


void editpointdialog::cancelClicked() {
  _color = _initialColor;
  QString color = QString::number(_color.red()) + QString(",") + QString::number(_color.green()) + QString(",") + QString::number(_color.blue());
  // Edit then event when confirm is clicked
  timeline->editEvent(_index,startTime->value(), endTime->value(), start->text(),
                        end->text(), name->text(), color);
  this->close();
}

void editpointdialog::colorButtonClicked() {

  _previousColor1 = _color;

  QColorDialog *dialog = new QColorDialog(_color);
  connect(dialog, SIGNAL(currentColorChanged(QColor)), this,
          SLOT(colorSelectedFromDialog(QColor)));
  connect(dialog, SIGNAL(rejected()), this, SLOT(colorRejectedFromDialog()));

  dialog->exec();
}

void editpointdialog::colorSelectedFromDialog(QColor ret) {

  if (ret.isValid()) {
    _color = ret;
  }
  setColor(_color);
   QString color = QString::number(_color.red()) + QString(",") + QString::number(_color.green()) + QString(",") + QString::number(_color.blue());
  // Edit then event when confirm is clicked
  timeline->editEvent(_index,startTime->value(), endTime->value(), start->text(),
                        end->text(), name->text(), color);

}

void editpointdialog::colorRejectedFromDialog() {

  if (_previousColor1.isValid()) {
    _color = _previousColor1;
  }
  setColor(_color);
    QString color = QString::number(_color.red()) + QString(",") + QString::number(_color.green()) + QString(",") + QString::number(_color.blue());
  // Edit then event when confirm is clicked
  timeline->editEvent(_index,startTime->value(), endTime->value(), start->text(),
                        end->text(), name->text(), color);


}

void editpointdialog::setColor(QColor c) {
  if (c.isValid()) {
    QString qss = QString("background-color: %1").arg(c.name());
    colorButton->setStyleSheet(qss);
  }
}

void editpointdialog::setEventColor(const QColor &c){
  if (c.isValid()){
    _color = c;
    _initialColor = _color;
  }

  setColor(_color);
}
