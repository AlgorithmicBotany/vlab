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



#include "panel.h"
#include "itemdialogs.h"
#include <QCloseEvent>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>

// =======================================   SLIDER
// ==============================================
SliderDialog::SliderDialog(QWidget *parent, Slider *I, const char *,
                           Qt::WindowFlags f)
    : QWidget(parent, f) {

  G = new QGridLayout(this);
  QLabel *label[8];
  label[0] = new QLabel("Type:", this);
  label[1] = new QLabel("SLIDER", this);
  label[2] = new QLabel("Name:", this);
  Name = new QLineEdit("One Cool Slider", this);
  label[3] = new QLabel("Colours:", this);

  label[4] = new QLabel("Origin:", this);
  originX = new QSpinBox(this);
  originX->setMinimum(0);
  originX->setMaximum(10000);
  originX->setSingleStep(1);
  originY = new QSpinBox(this);
  originY->setMinimum(0);
  originY->setMaximum(10000);
  originY->setSingleStep(1);
  label[5] = new QLabel("Min/Max:", this);
  min = new QSpinBox(this);
  min->setMinimum(-100000);
  min->setMaximum(100000);
  min->setSingleStep(1);

  max = new QSpinBox(this);
  max->setMinimum(-100000);
  max->setMaximum(100000);
  max->setSingleStep(1);

  label[6] = new QLabel("Default:", this);
  val = new QSpinBox(this);
  val->setMinimum(-100000);
  val->setMaximum(100000);
  val->setSingleStep(1);

  label[7] = new QLabel("Message:", this);
  message = new QLineEdit("Joanne is swell!", this);

  messageTemplate1 = new QRadioButton("d <name> %d <scale>", this);
  messageTemplate2 = new QRadioButton("n <line> <field> <scale exp> %d", this);
  QGroupBox *templateBox = new QGroupBox(tr("Select a template Message:"));
  messageTemplate1->setChecked(true);
  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->addWidget(messageTemplate1);
  vbox->addWidget(messageTemplate2);
  templateBox->setLayout(vbox);

  closebutton = new QPushButton("OK", this);

  G->addWidget(label[0], 0, 0);
  G->addWidget(label[1], 0, 1, 1, 2);

  G->addWidget(label[2], 1, 0);
  G->addWidget(Name, 1, 1, 1, 4);

  colour1Button = new QPushButton("", this);
  colour2Button = new QPushButton("", this);

  G->addWidget(label[3], 2, 0);
  G->addWidget(colour1Button, 2, 2);
  G->addWidget(colour2Button, 2, 4);

  G->addWidget(label[4], 3, 0);
  G->addWidget(originX, 3, 1, 1, 2);
  G->addWidget(originY, 3, 3, 1, 2);

  G->addWidget(label[5], 4, 0);
  G->addWidget(min, 4, 1, 1, 2);
  G->addWidget(max, 4, 3, 1, 2);

  G->addWidget(label[6], 5, 0);
  G->addWidget(val, 5, 1, 1, 2);

  G->addWidget(label[7], 6, 0);
  G->addWidget(message, 6, 1, 1, 4);

  G->addWidget(templateBox, 7, 0,4,5);

  G->addWidget(closebutton, 11, 0, 1, 5);

  setFixedSize(320, 350);

  hide();
  connected = false;
  slider = I;
  open = false;
  if (slider) {
    connectUp();
    update();
   }

  this->setWindowTitle("Slider Editor");
}

void SliderDialog::disconnectUp() {
  disconnect(Name, SIGNAL(textChanged(const QString &)), this, SLOT(setName()));
  disconnect(message, SIGNAL(textChanged(const QString &)), this,
             SLOT(setMessage()));
  //disconnect(val, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
  //disconnect(min, SIGNAL(valueChanged(int)), this, SLOT(setMinValue(int)));
  //disconnect(max, SIGNAL(valueChanged(int)), this, SLOT(setMaxValue(int)));
  disconnect(originX, SIGNAL(valueChanged(int)), this, SLOT(setOriginX(int)));
  disconnect(originY, SIGNAL(valueChanged(int)), this, SLOT(setOriginY(int)));
  disconnect(closebutton, SIGNAL(clicked()), this, SLOT(close()));
  disconnect(colour1Button, SIGNAL(clicked()), this,
             SLOT(colour1ButtonClicked()));
  disconnect(colour2Button, SIGNAL(clicked()), this,
             SLOT(colour2ButtonClicked()));
  disconnect(messageTemplate1, SIGNAL(toggled(bool)), this, SLOT(setTemplateMessage1(bool)));
  disconnect(messageTemplate2, SIGNAL(toggled(bool)), this, SLOT(setTemplateMessage2(bool)));

  connected = false;
}

void SliderDialog::connectUp() {
  connect(Name, SIGNAL(textChanged(const QString &)), this, SLOT(setName()));
  connect(message, SIGNAL(textChanged(const QString &)), this,
          SLOT(setMessage()));
  //connect(val, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
  //connect(min, SIGNAL(valueChanged(int)), this, SLOT(setMinValue(int)));
  //connect(max, SIGNAL(valueChanged(int)), this, SLOT(setMaxValue(int)));
  connect(originX, SIGNAL(valueChanged(int)), this, SLOT(setOriginX(int)));
  connect(originY, SIGNAL(valueChanged(int)), this, SLOT(setOriginY(int)));
  connect(closebutton, SIGNAL(clicked()), this, SLOT(close()));
  connect(colour1Button, SIGNAL(clicked()), this, SLOT(colour1ButtonClicked()));
  connect(colour2Button, SIGNAL(clicked()), this, SLOT(colour2ButtonClicked()));
  connect(messageTemplate1, SIGNAL(toggled(bool)), this, SLOT(setTemplateMessage1(bool)));
  connect(messageTemplate2, SIGNAL(toggled(bool)), this, SLOT(setTemplateMessage2(bool)));

  connected = true;
}

void SliderDialog::setSlider(Slider *I) {
  if (connected)
    disconnectUp();
  slider = I;
  connectUp();
  update();
  setWindowModality(Qt::NonModal);
  show();
}

void SliderDialog::update() {
  if (slider) {

    if (connected)
      disconnectUp();
    Name->setText(slider->getName());
    message->setText(slider->getMessage());
    // I don't know why this open variable is here ?
    //if (!open){
      min->setValue(slider->getMinValue());
      max->setValue(slider->getMaxValue());
    //val->setMinimum(min->value());
    //val->setMaximum(max->value());
      val->setValue(slider->getDefaultValue());

      open = true;
      //}
    
    originX->setValue(slider->getOrigin().x());
    originY->setValue(slider->getOrigin().y());

    QColor c = QColor((int)(slider->getColour1()[0] * 255.0),
                      (int)(slider->getColour1()[1] * 255.0),
                      (int)(slider->getColour1()[2] * 255.0));
    if (c.isValid()) {
      QString qss = QString("background-color: %1").arg(c.name());
      colour1Button->setStyleSheet(qss);
    }

    c = QColor((int)(slider->getColour2()[0] * 255.0),
               (int)(slider->getColour2()[1] * 255.0),
               (int)(slider->getColour2()[2] * 255.0));
    if (c.isValid()) {
      QString qss = QString("background-color: %1").arg(c.name());
      colour2Button->setStyleSheet(qss);
    }

    connectUp();
  }
}

void SliderDialog::setName() {
  if (slider) {
    slider->setName(Name->text());
    slider->getPanel()->setEdited(true);
    slider->getPanel()->update();
    Panel *panel = slider->getPanel();
    panel->getGLWidget()->update();
  }
}

void SliderDialog::setMessage() {
  if (slider) {
    slider->setMessage(message->text());
    slider->getPanel()->setEdited(true);
    //slider->getPanel()->update();
  }
}
void SliderDialog::setValue(int V) {
  if (slider) {
    slider->setDefaultValue(V);
    slider->setValue(V);
    slider->getPanel()->setEdited(true);
    //slider->getPanel()->update();
  }
}
void SliderDialog::setMinValue(int V) {
  if (slider) {
    slider->setMinValue(V);
    //val->setMinimum(min->value());
    /*
    if (val->value() < min->value())
      val->setValue(min->value());
    */
    slider->getPanel()->setEdited(true);
    //slider->getPanel()->update();
  }
}
void SliderDialog::setMaxValue(int V) {
  if (slider) {
    slider->setMaxValue(V);
    //val->setMaximum(max->value());
    /*
      if (val->value() > max->value())
      val->setValue(max->value());
    */
    slider->getPanel()->setEdited(true);
    // slider->getPanel()->update();
  }
}

void SliderDialog::setOriginX(int V) {
  if (slider) {
    slider->moveBy(V - slider->getOrigin().x(), 0);
    slider->getPanel()->setEdited(true);
    //slider->getPanel()->update();
  }
}

void SliderDialog::setOriginY(int V) {
  if (slider) {
    slider->moveBy(0, V - slider->getOrigin().y());
    slider->getPanel()->setEdited(true);
    slider->getPanel()->update();
  }
}

void SliderDialog::setColour1(GLfloat *c1) {
  if (slider) {
    QColor c = QColor((int)(c1[0] * 255.0), (int)(c1[1] * 255.0),
                      (int)(c1[2] * 255.0));
    if (c.isValid()) {
      QString qss = QString("background-color: %1").arg(c.name());
      colour1Button->setStyleSheet(qss);
    }

    slider->setColour1(c1);
    slider->getPanel()->setEdited(true);
    slider->getPanel()->update();
  }
}

void SliderDialog::setColour2(GLfloat *c2) {
  if (slider) {
    QColor c = QColor((int)(c2[0] * 255.0), (int)(c2[1] * 255.0),
                      (int)(c2[2] * 255.0));
    if (c.isValid()) {
      QString qss = QString("background-color: %1").arg(c.name());
      colour2Button->setStyleSheet(qss);
    }

    slider->setColour2(c2);
    slider->getPanel()->setEdited(true);
    slider->getPanel()->update();
  }
}

void SliderDialog::setTemplateMessage1(bool ) {
  if (slider) {
    message->setText(tr("d <name> %d <scale>"));
    setMessage();
    slider->getPanel()->setEdited(true);
  }
}
void SliderDialog::setTemplateMessage2(bool ) {
  if (slider) {
    message->setText(tr("n <line> <field> <scale exp> %d"));
    setMessage();
    slider->getPanel()->setEdited(true);
  }
}


void SliderDialog::colour1ButtonClicked() {
  int currRed = int(slider->getColour1()[0] * 255.0);
  int currGreen = int(slider->getColour1()[1] * 255.0);
  int currBlue = int(slider->getColour1()[2] * 255.0);

  previousColour1 = QColor(currRed, currGreen, currBlue);

  QColorDialog *dialog = new QColorDialog(QColor(currRed, currGreen, currBlue));
  connect(dialog, SIGNAL(currentColorChanged(QColor)), this,
          SLOT(colour1SelectedFromDialog(QColor)));
  connect(dialog, SIGNAL(rejected()), this, SLOT(colour1RejectedFromDialog()));

  dialog->show();
}

void SliderDialog::colour2ButtonClicked() {
  int currRed = int(slider->getColour2()[0] * 255.0);
  int currGreen = int(slider->getColour2()[1] * 255.0);
  int currBlue = int(slider->getColour2()[2] * 255.0);

  previousColour2 = QColor(currRed, currGreen, currBlue);

  QColorDialog *dialog = new QColorDialog(QColor(currRed, currGreen, currBlue));
  connect(dialog, SIGNAL(currentColorChanged(QColor)), this,
          SLOT(colour2SelectedFromDialog(QColor)));
  connect(dialog, SIGNAL(rejected()), this, SLOT(colour2RejectedFromDialog()));

  dialog->show();
}

void SliderDialog::closeEvent(QCloseEvent *) {
  if (slider) {
    slider->setMinValue(min->value());
    //val->setMinimum(min->value());
    if (val->value() < min->value())
      val->setValue(min->value());
    if (max->value() < min->value())
      max->setValue(min->value());
    slider->getPanel()->setEdited(true);
    slider->getPanel()->update();
    slider->setMaxValue(max->value());
    //val->setMaximum(max->value());
    if (val->value() > max->value())
      val->setValue(max->value());
    slider->setDefaultValue(val->value());
    slider->setValue(val->value());
    slider->getPanel()->setEdited(true);
    slider->getPanel()->update();
  }
  hide();
}

void SliderDialog::colour1SelectedFromDialog(QColor ret) {
  GLfloat c[3];

  if (ret.isValid()) {
    c[0] = float(ret.red()) / 255.0;
    c[1] = float(ret.green()) / 255.0;
    c[2] = float(ret.blue()) / 255.0;

    setColour1(c);
  }
}

void SliderDialog::colour2SelectedFromDialog(QColor ret) {
  GLfloat c[3];

  if (ret.isValid()) {
    c[0] = float(ret.red()) / 255.0;
    c[1] = float(ret.green()) / 255.0;
    c[2] = float(ret.blue()) / 255.0;

    setColour2(c);
  }
}

void SliderDialog::colour1RejectedFromDialog() {
  GLfloat c[3];

  if (previousColour1.isValid()) {
    c[0] = float(previousColour1.red()) / 255.0;
    c[1] = float(previousColour1.green()) / 255.0;
    c[2] = float(previousColour1.blue()) / 255.0;

    setColour1(c);
  }
}

void SliderDialog::colour2RejectedFromDialog() {
  GLfloat c[3];

  if (previousColour2.isValid()) {
    c[0] = float(previousColour2.red()) / 255.0;
    c[1] = float(previousColour2.green()) / 255.0;
    c[2] = float(previousColour2.blue()) / 255.0;

    setColour2(c);
  }
}



void SliderDialog::resizeEvent(QResizeEvent *event) {
  slider->getPanel()->resizeItemDialogs(event->size());
}

void SliderDialog::moveEvent(QMoveEvent *) {
  if (slider != NULL)
    slider->getPanel()->moveItemDialogs(frameGeometry().topLeft());
}

// =======================================   BUTTON
// ==============================================
ButtonDialog::ButtonDialog(QWidget *parent, Button *I, const char *,
                           Qt::WindowFlags f)
    : QWidget(parent, f) {
  G = new QGridLayout(this);
  QLabel *label[8];
  label[0] = new QLabel("Type:", this);
  label[1] = new QLabel("BUTTON", this);
  label[2] = new QLabel("Name:", this);
  Name = new QLineEdit("", this);
  label[3] = new QLabel("Colours:", this);

  colour1Button = new QPushButton("", this);
  colour2Button = new QPushButton("", this);
  colour3Button = new QPushButton("", this);
  clear = new QRadioButton("Transparent", this);
  label[4] = new QLabel("Origin:", this);
  originX = new QSpinBox(this);
  originX->setMinimum(0);
  originX->setMaximum(10000);
  originX->setSingleStep(1);
  originY = new QSpinBox(this);
  originY->setMinimum(0);
  originY->setMaximum(10000);
  originY->setSingleStep(1);

  label[5] = new QLabel("Value:", this);
  val = new QListWidget(this);
  val->addItem("OFF");
  val->addItem("ON");
  val->addItem("MONOSTABLE");
  label[6] = new QLabel("Message:", this);
  message = new QLineEdit("", this);

  messageTemplate1 = new QRadioButton("d <name> %d <scale>", this);
  messageTemplate2 = new QRadioButton("n <line> <field> <scale exp> %d", this);
  QGroupBox *templateBox = new QGroupBox(tr("Select a template Message:"));
  messageTemplate1->setChecked(true);
  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->addWidget(messageTemplate1);
  vbox->addWidget(messageTemplate2);
  templateBox->setLayout(vbox);

  
  closebutton = new QPushButton("OK", this);

  G->addWidget(label[0], 0, 0);
  G->addWidget(label[1], 0, 1, 1, 2);

  G->addWidget(label[2], 1, 0);
  G->addWidget(Name, 1, 1, 1, 2);

  G->addWidget(label[3], 2, 0);
  QGridLayout *layout = new QGridLayout;
  /*
  G->addWidget(colour1Button, 2, 1);
  G->addWidget(colour2Button, 2, 3);
  G->addWidget(colour3Button, 2, 4);
  */
  layout->addWidget(colour1Button,0,0);
  layout->addWidget(colour2Button,0,1);
  layout->addWidget(colour3Button,0,2);
  QLabel *colourLabel1 = new QLabel("Outline",this);
  colourLabel1->setAlignment(Qt::AlignCenter);
  QLabel *colourLabel2 = new QLabel("On",this);
  colourLabel2->setAlignment(Qt::AlignCenter);
  QLabel *colourLabel3 = new QLabel("Off",this);
  colourLabel3->setAlignment(Qt::AlignCenter);
  layout->addWidget(colourLabel1,1,0);
  layout->addWidget(colourLabel2,1,1);
  layout->addWidget(colourLabel3,1,2);
  layout->addWidget(clear, 2, 2, 1, 2);

  G->addLayout(layout,2,1,3,5);

  G->addWidget(label[4], 5, 0);
  G->addWidget(originX, 5, 1, 1, 2);
  G->addWidget(originY, 5, 3, 1, 2);

  G->addWidget(label[5], 6, 0);
  G->addWidget(val, 6, 1, 4, 4);

  G->addWidget(label[6], 10, 0);
  G->addWidget(message, 10, 1, 1, 4);

  G->addWidget(templateBox, 11, 0,3,5);

  G->addWidget(closebutton, 14, 0, 1, 5);

  hide();
  setFixedSize(350, 428);

  button = I;
  if (button) {
    connectUp();
    update();
  }

  this->setWindowTitle("Button Editor");
}

void ButtonDialog::disconnectUp() {
  disconnect(Name, SIGNAL(textChanged(const QString &)), this, SLOT(setName()));
  disconnect(message, SIGNAL(textChanged(const QString &)), this,
             SLOT(setMessage()));
  disconnect(val, SIGNAL(itemClicked(QListWidgetItem *)), this,
             SLOT(setValue(QListWidgetItem *)));
  disconnect(originX, SIGNAL(valueChanged(int)), this, SLOT(setOriginX(int)));
  disconnect(originY, SIGNAL(valueChanged(int)), this, SLOT(setOriginY(int)));
  disconnect(clear, SIGNAL(toggled(bool)), this, SLOT(setClear(bool)));
  disconnect(messageTemplate1, SIGNAL(toggled(bool)), this, SLOT(setTemplateMessage1(bool)));
  disconnect(messageTemplate2, SIGNAL(toggled(bool)), this, SLOT(setTemplateMessage2(bool)));
  disconnect(closebutton, SIGNAL(clicked()), this, SLOT(close()));
  disconnect(colour1Button, SIGNAL(clicked()), this,
             SLOT(colour1ButtonClicked()));
  disconnect(colour2Button, SIGNAL(clicked()), this,
             SLOT(colour2ButtonClicked()));
  disconnect(colour3Button, SIGNAL(clicked()), this,
             SLOT(colour3ButtonClicked()));

  connected = false;
}

void ButtonDialog::connectUp() {
  connect(Name, SIGNAL(textChanged(const QString &)), this, SLOT(setName()));
  connect(message, SIGNAL(textChanged(const QString &)), this,
          SLOT(setMessage()));
  connect(val, SIGNAL(itemClicked(QListWidgetItem *)), this,
          SLOT(setValue(QListWidgetItem *)));
  connect(originX, SIGNAL(valueChanged(int)), this, SLOT(setOriginX(int)));
  connect(originY, SIGNAL(valueChanged(int)), this, SLOT(setOriginY(int)));
  connect(clear, SIGNAL(toggled(bool)), this, SLOT(setClear(bool)));
  connect(messageTemplate1, SIGNAL(toggled(bool)), this, SLOT(setTemplateMessage1(bool)));
  connect(messageTemplate2, SIGNAL(toggled(bool)), this, SLOT(setTemplateMessage2(bool)));
  connect(closebutton, SIGNAL(clicked()), this, SLOT(close()));
  connect(colour1Button, SIGNAL(clicked()), this, SLOT(colour1ButtonClicked()));
  connect(colour2Button, SIGNAL(clicked()), this, SLOT(colour2ButtonClicked()));
  connect(colour3Button, SIGNAL(clicked()), this, SLOT(colour3ButtonClicked()));

  connected = true;
}

void ButtonDialog::setButton(Button *I) {
  if (connected)
    disconnectUp();
  button = I;
  connectUp();
  update();
  show();
}

void ButtonDialog::update() {
  if (button) {
    if (connected)
      disconnectUp();
    Name->setText(button->getName());

    message->setText(button->getMessage());
    val->setCurrentRow(button->getValue());
    originX->setValue(button->getOrigin().x());
    originY->setValue(button->getOrigin().y());
    clear->setChecked(button->isTransparent());

    QColor c = QColor((int)(button->getColour1()[0] * 255.0),
                      (int)(button->getColour1()[1] * 255.0),
                      (int)(button->getColour1()[2] * 255.0));
    if (c.isValid()) {
      QString qss = QString("background-color: %1").arg(c.name());
      colour1Button->setStyleSheet(qss);
    }

    c = QColor((int)(button->getColour2()[0] * 255.0),
               (int)(button->getColour2()[1] * 255.0),
               (int)(button->getColour2()[2] * 255.0));
    if (c.isValid()) {
      QString qss = QString("background-color: %1").arg(c.name());
      colour2Button->setStyleSheet(qss);
    }


    int transparent = 255;
    if (clear->isChecked())
      transparent = 100;

    GLfloat *c3 = button->getColour3();
    std::string rgbaColor = std::to_string((int)(c3[0] * 255.0)) + "," +
      std::to_string((int)(c3[1] * 255.0)) + "," +
      std::to_string((int)(c3[2] * 255.0)) + "," +
      std::to_string(transparent);
    
    QString qss = QString("QPushButton {background-color: rgba(%1) }").arg(QString(rgbaColor.c_str()));
    colour3Button->setStyleSheet(qss);

    

    connectUp();
  }
}

void ButtonDialog::setName() {
  if (button) {
    button->setName(Name->text());
    button->getPanel()->setEdited(true);
    button->getPanel()->update();
  }

 }

void ButtonDialog::setMessage() {
  if (button) {
    button->setMessage(message->text());
    button->getPanel()->setEdited(true);
    button->getPanel()->update();
  }
}

void ButtonDialog::setValue(QListWidgetItem *i) {
  if (button) {
    int type = OFF;
    if (i->text() == "OFF")
      type = OFF;
    else if (i->text() == "ON")
      type = ON;
    else if (i->text() == "MONOSTABLE")
      type = MONOSTABLE;

    button->setValue(type);
    button->setDefaultValue(type);
    button->getPanel()->setEdited(true);
    button->getPanel()->update();
  }
}

void ButtonDialog::setOriginX(int V) {
  if (button) {
    button->moveBy(V - button->getOrigin().x(), 0);
    button->getPanel()->setEdited(true);
    button->getPanel()->update();
  }
}

void ButtonDialog::setOriginY(int V) {
  if (button) {
    button->moveBy(0, V - button->getOrigin().y());
    button->getPanel()->setEdited(true);
    button->getPanel()->update();
  }
}

void ButtonDialog::setClear(bool B) {
  if (button) {
    button->setTransparent(B);
    
    int transparent = 255;
    if (clear->isChecked())
      transparent = 100;

    GLfloat *c3 = button->getColour3();
    std::string rgbaColor = std::to_string((int)(c3[0] * 255.0)) + "," +
      std::to_string((int)(c3[1] * 255.0)) + "," +
      std::to_string((int)(c3[2] * 255.0)) + "," +
      std::to_string(transparent);
    
    QString qss = QString("QPushButton {background-color: rgba(%1) }").arg(QString(rgbaColor.c_str()));
    colour3Button->setStyleSheet(qss);

    button->getPanel()->setEdited(true);
    button->getPanel()->update();
  }
}

void ButtonDialog::setTemplateMessage1(bool ) {
  if (button) {
    message->setText(tr("d <name> %d <scale>"));
    setMessage();
    button->getPanel()->setEdited(true);
  }
}
void ButtonDialog::setTemplateMessage2(bool ) {
  if (button) {
    message->setText(tr("n <line> <field> <scale exp> %d"));
    setMessage();
    button->getPanel()->setEdited(true);
  }
}

void ButtonDialog::setColour1(GLfloat *c1) {
  if (button) {
    QColor c = QColor((int)(c1[0] * 255.0), (int)(c1[1] * 255.0),
                      (int)(c1[2] * 255.0));
    if (c.isValid()) {
      QString qss = QString("background-color: %1").arg(c.name());
      colour1Button->setStyleSheet(qss);
    }
    button->setColour1(c1);
    button->getPanel()->setEdited(true);
    button->getPanel()->update();
  }
}

void ButtonDialog::setColour2(GLfloat *c2) {
  if (button) {
    QColor c = QColor((int)(c2[0] * 255.0), (int)(c2[1] * 255.0),
                      (int)(c2[2] * 255.0));
    if (c.isValid()) {
      QString qss = QString("background-color: %1").arg(c.name());
      colour2Button->setStyleSheet(qss);
    }
    button->setColour2(c2);
    button->getPanel()->setEdited(true);
    button->getPanel()->update();
  }
}

void ButtonDialog::setColour3(GLfloat *c3) {
  if (button) {
    int transparent = 255;
    if (clear->isChecked())
      transparent = 100;

    std::string rgbaColor = std::to_string((int)(c3[0] * 255.0)) + "," +
      std::to_string((int)(c3[1] * 255.0)) + "," +
      std::to_string((int)(c3[2] * 255.0)) + "," +
      std::to_string(transparent);
    
    QString qss = QString("QPushButton {background-color: rgba(%1) }").arg(QString(rgbaColor.c_str()));
    colour3Button->setStyleSheet(qss);

    disconnectUp();
    clear->setChecked(false);
    connectUp();
    button->setTransparent(false);
    button->setColour3(c3);
    button->getPanel()->setEdited(true);
    button->getPanel()->update();
  }
}

void ButtonDialog::colour1ButtonClicked() {
  int currRed = int(button->getColour1()[0] * 255.0);
  int currGreen = int(button->getColour1()[1] * 255.0);
  int currBlue = int(button->getColour1()[2] * 255.0);

  previousColour1 = QColor(currRed, currGreen, currBlue);

  QColorDialog *dialog = new QColorDialog(QColor(currRed, currGreen, currBlue));
  connect(dialog, SIGNAL(currentColorChanged(QColor)), this,
          SLOT(colour1SelectedFromDialog(QColor)));
  connect(dialog, SIGNAL(rejected()), this, SLOT(colour1RejectedFromDialog()));

  dialog->show();
}

void ButtonDialog::colour2ButtonClicked() {
  int currRed = int(button->getColour2()[0] * 255.0);
  int currGreen = int(button->getColour2()[1] * 255.0);
  int currBlue = int(button->getColour2()[2] * 255.0);

  previousColour2 = QColor(currRed, currGreen, currBlue);

  QColorDialog *dialog = new QColorDialog(QColor(currRed, currGreen, currBlue));
  connect(dialog, SIGNAL(currentColorChanged(QColor)), this,
          SLOT(colour2SelectedFromDialog(QColor)));
  connect(dialog, SIGNAL(rejected()), this, SLOT(colour2RejectedFromDialog()));

  dialog->show();
}

void ButtonDialog::colour3ButtonClicked() {
  int currRed = int(button->getColour3()[0] * 255.0);
  int currGreen = int(button->getColour3()[1] * 255.0);
  int currBlue = int(button->getColour3()[2] * 255.0);

  previousColour3 = QColor(currRed, currGreen, currBlue);

  QColorDialog *dialog = new QColorDialog(QColor(currRed, currGreen, currBlue));
  connect(dialog, SIGNAL(currentColorChanged(QColor)), this,
          SLOT(colour3SelectedFromDialog(QColor)));
  connect(dialog, SIGNAL(rejected()), this, SLOT(colour3RejectedFromDialog()));

  dialog->show();
}

void ButtonDialog::closeEvent(QCloseEvent *) { hide(); }

void ButtonDialog::colour1SelectedFromDialog(QColor ret) {
  GLfloat c[3];

  if (ret.isValid()) {
    c[0] = float(ret.red()) / 255.0;
    c[1] = float(ret.green()) / 255.0;
    c[2] = float(ret.blue()) / 255.0;

    setColour1(c);
  }
}

void ButtonDialog::colour2SelectedFromDialog(QColor ret) {
  GLfloat c[3];

  if (ret.isValid()) {
    c[0] = float(ret.red()) / 255.0;
    c[1] = float(ret.green()) / 255.0;
    c[2] = float(ret.blue()) / 255.0;

    setColour2(c);
  }
}

void ButtonDialog::colour3SelectedFromDialog(QColor ret) {
  GLfloat c[3];

  if (ret.isValid()) {
    c[0] = float(ret.red()) / 255.0;
    c[1] = float(ret.green()) / 255.0;
    c[2] = float(ret.blue()) / 255.0;

    setColour3(c);
  }
}

void ButtonDialog::colour1RejectedFromDialog() {
  GLfloat c[3];

  if (previousColour1.isValid()) {
    c[0] = float(previousColour1.red()) / 255.0;
    c[1] = float(previousColour1.green()) / 255.0;
    c[2] = float(previousColour1.blue()) / 255.0;

    setColour1(c);
  }
}

void ButtonDialog::colour2RejectedFromDialog() {
  GLfloat c[3];

  if (previousColour2.isValid()) {
    c[0] = float(previousColour2.red()) / 255.0;
    c[1] = float(previousColour2.green()) / 255.0;
    c[2] = float(previousColour2.blue()) / 255.0;

    setColour2(c);
  }
}

void ButtonDialog::colour3RejectedFromDialog() {
  GLfloat c[3];

  if (previousColour3.isValid()) {
    c[0] = float(previousColour3.red()) / 255.0;
    c[1] = float(previousColour3.green()) / 255.0;
    c[2] = float(previousColour3.blue()) / 255.0;

    setColour3(c);
  }
}

void ButtonDialog::resizeEvent(QResizeEvent *event) {
  button->getPanel()->resizeItemDialogs(event->size());
}

void ButtonDialog::moveEvent(QMoveEvent *) {
  if (button != NULL)
    button->getPanel()->moveItemDialogs(frameGeometry().topLeft());
}

// =======================================   LABEL
// ==============================================
LabelDialog::LabelDialog(QWidget *parent, Label *I, const char *name,
                         Qt::WindowFlags f)
    : QWidget(parent, f) {
  setObjectName(name);
  G = new QGridLayout(this);
  QLabel *labels[8];
  labels[0] = new QLabel("Type:", this);
  labels[1] = new QLabel("LABEL", this);
  labels[2] = new QLabel("Name:", this);
  Name = new QLineEdit("", this);
  labels[3] = new QLabel("Colour:", this);
  colour1Button = new QPushButton("", this);
  labels[4] = new QLabel("Origin:", this);
  originX = new QSpinBox(this);
  originX->setMinimum(0);
  originX->setMaximum(10000);
  originX->setSingleStep(1);

  originY = new QSpinBox(this);
  originY->setMinimum(0);
  originY->setMaximum(10000);
  originY->setSingleStep(1);

  closebutton = new QPushButton("OK", this);

  G->addWidget(labels[0], 0, 0);
  G->addWidget(labels[1], 0, 1, 1, 2);

  G->addWidget(labels[2], 1, 0);
  G->addWidget(Name, 1, 1, 1, 4);

  G->addWidget(labels[3], 2, 0);
  G->addWidget(colour1Button, 2, 2);

  G->addWidget(labels[4], 3, 0);
  G->addWidget(originX, 3, 1, 1, 2);
  G->addWidget(originY, 3, 3, 1, 2);

  G->addWidget(closebutton, 4, 0, 1, 5);
  setFixedSize(276, 181);
  hide();

  label = I;
  if (label) {
    connectUp();
    update();
  }

  this->setWindowTitle("Label Editor");
}

void LabelDialog::disconnectUp() {
  disconnect(Name, SIGNAL(textChanged(const QString &)), this, SLOT(setName()));
  disconnect(originX, SIGNAL(valueChanged(int)), this, SLOT(setOriginX(int)));
  disconnect(originY, SIGNAL(valueChanged(int)), this, SLOT(setOriginY(int)));
  disconnect(closebutton, SIGNAL(clicked()), this, SLOT(close()));
  disconnect(colour1Button, SIGNAL(clicked()), this,
             SLOT(colour1ButtonClicked()));
  connected = false;
}

void LabelDialog::connectUp() {
  connect(Name, SIGNAL(textChanged(const QString &)), this, SLOT(setName()));
  connect(originX, SIGNAL(valueChanged(int)), this, SLOT(setOriginX(int)));
  connect(originY, SIGNAL(valueChanged(int)), this, SLOT(setOriginY(int)));
  connect(closebutton, SIGNAL(clicked()), this, SLOT(close()));
  connect(colour1Button, SIGNAL(clicked()), this, SLOT(colour1ButtonClicked()));
  connected = true;
}

void LabelDialog::setLabel(Label *I) {
  if (connected)
    disconnectUp();
  label = I;
  connectUp();
  update();
  show();
}

void LabelDialog::update() {
  if (label) {
    if (connected)
      disconnectUp();
    Name->setText(label->getName());
    originX->setValue(label->getOrigin().x());
    originY->setValue(label->getOrigin().y());

    QColor c = QColor((int)(label->getColour1()[0] * 255.0),
                      (int)(label->getColour1()[1] * 255.0),
                      (int)(label->getColour1()[2] * 255.0));
    if (c.isValid()) {
      QString qss = QString("background-color: %1").arg(c.name());
      colour1Button->setStyleSheet(qss);
    }

    connectUp();
  }
}

void LabelDialog::setName() {
  if (label) {
    label->setName(Name->text());
    label->getPanel()->setEdited(true);
    label->getPanel()->update();
  }
}

void LabelDialog::setOriginX(int V) {
  if (label) {
    label->moveBy(V - label->getOrigin().x(), 0);
    label->getPanel()->setEdited(true);
    label->getPanel()->update();
  }
}

void LabelDialog::setOriginY(int V) {
  if (label) {
    label->moveBy(0, V - label->getOrigin().y());
    label->getPanel()->setEdited(true);
    label->getPanel()->update();
  }
}

void LabelDialog::setColour1(GLfloat *c1) {
  if (label) {
    QColor c = QColor((int)(c1[0] * 255.0), (int)(c1[1] * 255.0),
                      (int)(c1[2] * 255.0));
    if (c.isValid()) {
      QString qss = QString("background-color: %1").arg(c.name());
      colour1Button->setStyleSheet(qss);
    }

    label->setColour1(c1);
    label->getPanel()->setEdited(true);
    label->getPanel()->update();
  }
}

void LabelDialog::colour1ButtonClicked() {
  int currRed = int(label->getColour1()[0] * 255.0);
  int currGreen = int(label->getColour1()[1] * 255.0);
  int currBlue = int(label->getColour1()[2] * 255.0);

  previousColour1 = QColor(currRed, currGreen, currBlue);

  QColorDialog *dialog = new QColorDialog(QColor(currRed, currGreen, currBlue));
  connect(dialog, SIGNAL(currentColorChanged(QColor)), this,
          SLOT(colour1SelectedFromDialog(QColor)));
  connect(dialog, SIGNAL(rejected()), this, SLOT(colour1RejectedFromDialog()));

  dialog->show();
}

void LabelDialog::colour1SelectedFromDialog(QColor ret) {
  GLfloat c[3];

  if (ret.isValid()) {
    c[0] = float(ret.red()) / 255.0;
    c[1] = float(ret.green()) / 255.0;
    c[2] = float(ret.blue()) / 255.0;

    setColour1(c);
  }
}

void LabelDialog::colour1RejectedFromDialog() {
  GLfloat c[3];

  if (previousColour1.isValid()) {
    c[0] = float(previousColour1.red()) / 255.0;
    c[1] = float(previousColour1.green()) / 255.0;
    c[2] = float(previousColour1.blue()) / 255.0;

    setColour1(c);
  }
}

void LabelDialog::closeEvent(QCloseEvent *) { hide(); }

void LabelDialog::resizeEvent(QResizeEvent *event) {

  label->getPanel()->resizeItemDialogs(event->size());
}

void LabelDialog::moveEvent(QMoveEvent *) {
  if (label != NULL)
    label->getPanel()->moveItemDialogs(frameGeometry().topLeft());
}

// =======================================   GROUP
// ==============================================
GroupDialog::GroupDialog(QWidget *parent, Group *I, const char *name,
                         Qt::WindowFlags f)
    : QWidget(parent, f) {
  setObjectName(name);
  G = new QGridLayout(this);
  QLabel *label[8];
  label[0] = new QLabel("Type:", this);
  label[1] = new QLabel("GROUP", this);
  label[2] = new QLabel("Colour:", this);
  colour1Button = new QPushButton("", this);
  label[3] = new QLabel("Origin:", this);
  originX = new QSpinBox(this);
  originX->setMinimum(0);
  originX->setMaximum(10000);
  originX->setSingleStep(1);

  originY = new QSpinBox(this);
  originY->setMinimum(0);
  originY->setMaximum(10000);
  originY->setSingleStep(1);

  label[4] = new QLabel("Buttons:", this);
  buttonlist = new QListWidget(this);
  closebutton = new QPushButton("OK", this);
  closebutton->setAutoDefault(false);

  G->addWidget(label[0], 0, 0);
  G->addWidget(label[1], 0, 1, 1, 2);

  G->addWidget(label[2], 1, 0);
  G->addWidget(colour1Button, 1, 2);

  G->addWidget(label[3], 2, 0);
  G->addWidget(originX, 2, 1, 1, 2);
  G->addWidget(originY, 2, 3, 1, 2);

  G->addWidget(label[4], 3, 0);
  G->addWidget(buttonlist, 3, 1, 7, 4);
  G->addWidget(closebutton, 11, 0, 1, 5);

  hide();
  setFixedSize(276, 238);

  group = I;
  if (group) {
    connectUp();
    update();
  }

  this->setWindowTitle("Button Group Editor");
}

void GroupDialog::disconnectUp() {
  disconnect(originX, SIGNAL(valueChanged(int)), this, SLOT(setOriginX(int)));
  disconnect(originY, SIGNAL(valueChanged(int)), this, SLOT(setOriginY(int)));
  disconnect(closebutton, SIGNAL(clicked()), this, SLOT(close()));
  disconnect(colour1Button, SIGNAL(clicked()), this,
             SLOT(colour1ButtonClicked()));

  connected = false;
}

void GroupDialog::connectUp() {
  connect(originX, SIGNAL(valueChanged(int)), this, SLOT(setOriginX(int)));
  connect(originY, SIGNAL(valueChanged(int)), this, SLOT(setOriginY(int)));
  connect(closebutton, SIGNAL(clicked()), this, SLOT(close()));
  connect(colour1Button, SIGNAL(clicked()), this, SLOT(colour1ButtonClicked()));

  connected = true;
}

void GroupDialog::removeSelect() {
  group->getPanel()->removeFromGroup(
      group->getButtonAt(buttonlist->currentRow()));
  buttonlist->removeItemWidget(buttonlist->currentItem());
}

void GroupDialog::setGroup(Group *I) {
  if (connected)
    disconnectUp();
  group = I;
  connectUp();
  update();
  show();
}

void GroupDialog::update() {
  if (group) {
    if (connected)
      disconnectUp();
    originX->setValue(group->getOrigin().x());
    originY->setValue(group->getOrigin().y());
    buttonlist->clear();
    for (int i = 0; i < group->getNumButtons(); i++) {
      buttonlist->addItem(group->getButtonAt(i)->getName());
    }

    QColor c = QColor((int)(group->getColour1()[0] * 255.0),
                      (int)(group->getColour1()[1] * 255.0),
                      (int)(group->getColour1()[2] * 255.0));
    if (c.isValid()) {
      QString qss = QString("background-color: %1").arg(c.name());
      colour1Button->setStyleSheet(qss);
    }
    connectUp();
  }
}

void GroupDialog::setOriginX(int V) {
  if (group) {
    group->moveBy(V - group->getOrigin().x(), 0);
    group->getPanel()->setEdited(true);
    group->getPanel()->update();
  }
}

void GroupDialog::setOriginY(int V) {
  if (group) {
    group->moveBy(0, V - group->getOrigin().y());
    group->getPanel()->setEdited(true);
    group->getPanel()->update();
  }
}

void GroupDialog::setColour1(GLfloat *c1) {
  if (group) {
    QColor c = QColor((int)(c1[0] * 255.0), (int)(c1[1] * 255.0),
                      (int)(c1[2] * 255.0));
    if (c.isValid()) {
      QString qss = QString("background-color: %1").arg(c.name());
      colour1Button->setStyleSheet(qss);
    }

    group->setColour1(c1);
    group->getPanel()->setEdited(true);
    group->getPanel()->update();
  }
}

void GroupDialog::colour1ButtonClicked() {
  int currRed = int(group->getColour1()[0] * 255.0);
  int currGreen = int(group->getColour1()[1] * 255.0);
  int currBlue = int(group->getColour1()[2] * 255.0);

  previousColour1 = QColor(currRed, currGreen, currBlue);

  QColorDialog *dialog = new QColorDialog(QColor(currRed, currGreen, currBlue));
  connect(dialog, SIGNAL(currentColorChanged(QColor)), this,
          SLOT(colour1SelectedFromDialog(QColor)));
  connect(dialog, SIGNAL(rejected()), this, SLOT(colour1RejectedFromDialog()));

  dialog->show();
}

void GroupDialog::colour1SelectedFromDialog(QColor ret) {
  GLfloat c[3];

  if (ret.isValid()) {
    c[0] = float(ret.red()) / 255.0;
    c[1] = float(ret.green()) / 255.0;
    c[2] = float(ret.blue()) / 255.0;

    setColour1(c);
  }
}

void GroupDialog::colour1RejectedFromDialog() {
  GLfloat c[3];

  if (previousColour1.isValid()) {
    c[0] = float(previousColour1.red()) / 255.0;
    c[1] = float(previousColour1.green()) / 255.0;
    c[2] = float(previousColour1.blue()) / 255.0;

    setColour1(c);
  }
}

void GroupDialog::closeEvent(QCloseEvent *) { hide(); }

void GroupDialog::resizeEvent(QResizeEvent *event) {

  group->getPanel()->resizeItemDialogs(event->size());
}

void GroupDialog::moveEvent(QMoveEvent *) {
  if (group != NULL)
    group->getPanel()->moveItemDialogs(frameGeometry().topLeft());
}

// =======================================   MENU
// ==============================================
MenuDialog::MenuDialog(QWidget *parent, Menu *I, const char *name,
                       Qt::WindowFlags f)
    : QWidget(parent, f) {
  setObjectName(name);
  G = new QGridLayout(this);
  QLabel *label[8];
  label[0] = new QLabel("Type:", this);
  label[1] = new QLabel("MENU", this);
  label[2] = new QLabel("Name:", this);
  Name = new QLineEdit("", this);
  label[3] = new QLabel("Message:", this);
  message = new QLineEdit("", this);
  delbutton = new QPushButton("Delete", this);
  
  messageTemplate1 = new QRadioButton("d <name> %d <scale>", this);
  messageTemplate2 = new QRadioButton("n <line> <field> <scale exp> %d", this);
  QGroupBox *templateBox = new QGroupBox(tr("Select a template Message:"));
  messageTemplate1->setChecked(true);
  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->addWidget(messageTemplate1);
  vbox->addWidget(messageTemplate2);
  templateBox->setLayout(vbox);

  closebutton = new QPushButton("OK", this);

  G->addWidget(label[0], 0, 0);
  G->addWidget(label[1], 0, 1, 1, 2);

  G->addWidget(label[2], 1, 0);
  G->addWidget(Name, 1, 1, 1, 4);

  G->addWidget(label[3], 2, 0);
  G->addWidget(message, 2, 1, 1, 4);
  G->addWidget(templateBox, 3, 0,4,5);

  G->addWidget(delbutton, 7, 0, 1, 2);
  G->addItem(new QSpacerItem(0, 15), 4, 0);

  G->addWidget(closebutton, 8, 0, 1, 5);

  hide();
  setFixedSize(320, 251);

  menu = I;
  if (menu) {
    connectUp();
    update();
  }

  setWindowTitle("Menu Editor");
}

void MenuDialog::disconnectUp() {
  disconnect(Name, SIGNAL(textChanged(const QString &)), this, SLOT(setName()));
  disconnect(message, SIGNAL(textChanged(const QString &)), this,
             SLOT(setMessage()));
  if (menu)
    disconnect(delbutton, SIGNAL(clicked()), menu->getPanel(),
               SLOT(deleteMenuItem()));
  disconnect(closebutton, SIGNAL(clicked()), this, SLOT(close()));
  disconnect(messageTemplate1, SIGNAL(toggled(bool)), this, SLOT(setTemplateMessage1(bool)));
  disconnect(messageTemplate2, SIGNAL(toggled(bool)), this, SLOT(setTemplateMessage2(bool)));
  connected = false;
}

void MenuDialog::connectUp() {
  connect(Name, SIGNAL(textChanged(const QString &)), this, SLOT(setName()));
  connect(message, SIGNAL(textChanged(const QString &)), this,
          SLOT(setMessage()));
  if (menu)
    connect(delbutton, SIGNAL(clicked()), menu->getPanel(),
            SLOT(deleteMenuItem()));
  connect(closebutton, SIGNAL(clicked()), this, SLOT(close()));
  connect(messageTemplate1, SIGNAL(toggled(bool)), this, SLOT(setTemplateMessage1(bool)));
  connect(messageTemplate2, SIGNAL(toggled(bool)), this, SLOT(setTemplateMessage2(bool)));
  connected = true;
}

void MenuDialog::setMenu(Menu *I) {
  if (connected)
    disconnectUp();
  menu = I;
  connectUp();
  update();
  show();
}

void MenuDialog::update() {
  if (menu) {
    if (connected)
      disconnectUp();
    Name->setText(menu->getName());
    message->setText(menu->getMessage());
    connectUp();
  }
}

void MenuDialog::setName() {
  if (menu) {
    menu->setName(Name->text());
    menu->getPanel()->setupMenu();
    menu->getPanel()->setEdited(true);
  }
}

void MenuDialog::setMessage() {
  if (menu) {
    menu->setMessage(message->text());
    menu->getPanel()->setEdited(true);
  }
}
void MenuDialog::setTemplateMessage1(bool ) {
  if (menu) {
    message->setText(tr("d <name> %d <scale>"));
    setMessage();
    menu->getPanel()->setEdited(true);
  }
}
void MenuDialog::setTemplateMessage2(bool ) {
  if (menu) {
    message->setText(tr("n <line> <field> <scale exp> %d"));
    setMessage();
    menu->getPanel()->setEdited(true);
  }
}

void MenuDialog::closeEvent(QCloseEvent *) { hide(); }

void MenuDialog::resizeEvent(QResizeEvent *event) {

  menu->getPanel()->resizeItemDialogs(event->size());
}

void MenuDialog::moveEvent(QMoveEvent *) {
  if (menu != NULL)
    menu->getPanel()->moveItemDialogs(frameGeometry().topLeft());
}

// =======================================   PAGE
// ==============================================
PageDialog::PageDialog(QWidget *parent, Page *I, const char *name,
                       Qt::WindowFlags f)
    : QWidget(parent, f) {
  setObjectName(name);
  G = new QGridLayout(this);
  QLabel *label[8];
  label[0] = new QLabel("Type:", this);
  label[1] = new QLabel("PAGE", this);
  label[2] = new QLabel("Name:", this);
  Name = new QLineEdit("", this);
  label[3] = new QLabel("Label Colour:", this);
  colour1Button = new QPushButton("", this);
  label[4] = new QLabel("Label Origin:", this);
  originX = new QSpinBox(this);
  originX->setMinimum(0);
  originX->setMaximum(10000);
  originX->setSingleStep(1);

  originY = new QSpinBox(this);
  originY->setMinimum(0);
  originY->setMaximum(10000);
  originY->setSingleStep(1);

  label[5] = new QLabel("Message:", this);
  message = new QLineEdit("", this);
  closebutton = new QPushButton("OK", this);

  G->addWidget(label[0], 0, 0);
  G->addWidget(label[1], 0, 1, 1, 2);

  G->addWidget(label[2], 1, 0);
  G->addWidget(Name, 1, 1, 1, 4);

  G->addWidget(label[3], 2, 0);
  G->addWidget(colour1Button, 2, 2);

  G->addWidget(label[4], 3, 0);
  G->addWidget(originX, 3, 1, 1, 2);
  G->addWidget(originY, 3, 3, 1, 2);

  G->addWidget(label[5], 4, 0);
  G->addWidget(message, 4, 1, 1, 4);
  G->addWidget(closebutton, 5, 0, 1, 5);

  hide();
  setFixedSize(284, 216);

  page = I;
 
  if (page) {
    connectUp();
    update();
  }
 
  this->setWindowTitle("Page Editor");
}

void PageDialog::disconnectUp() {
  disconnect(Name, SIGNAL(textChanged(const QString &)), this, SLOT(setName()));
  disconnect(message, SIGNAL(textChanged(const QString &)), this,
             SLOT(setMessage()));
  disconnect(originX, SIGNAL(valueChanged(int)), this, SLOT(setOriginX(int)));
  disconnect(originY, SIGNAL(valueChanged(int)), this, SLOT(setOriginY(int)));
  disconnect(closebutton, SIGNAL(clicked()), this, SLOT(close()));
  disconnect(colour1Button, SIGNAL(clicked()), this,
             SLOT(colour1ButtonClicked()));
  connected = false;
}

void PageDialog::connectUp() {
  connect(Name, SIGNAL(textChanged(const QString &)), this, SLOT(setName()));
  connect(message, SIGNAL(textChanged(const QString &)), this,
          SLOT(setMessage()));
  connect(originX, SIGNAL(valueChanged(int)), this, SLOT(setOriginX(int)));
  connect(originY, SIGNAL(valueChanged(int)), this, SLOT(setOriginY(int)));
  connect(closebutton, SIGNAL(clicked()), this, SLOT(close()));
  connect(colour1Button, SIGNAL(clicked()), this, SLOT(colour1ButtonClicked()));
  connected = true;
}

void PageDialog::setPage(Page *I) {
  if (connected)
    disconnectUp();
  page = I;
  GLfloat *col = I->getColour1();
  page->setColour1(col);
  connectUp();
  update();
  show();
}

void PageDialog::update() {
  if (page) {
    if (connected)
      disconnectUp();
    Name->setText(page->getName());
    message->setText(page->getMessage());
    originX->setValue(page->getItem(0)->getOrigin().x());
    originY->setValue(page->getItem(0)->getOrigin().y());

    QColor c = QColor((int)(page->getColour1()[0] * 255.0),
                      (int)(page->getColour1()[1] * 255.0),
                      (int)(page->getColour1()[2] * 255.0));
    if (c.isValid()) {
      QString qss = QString("QPushButton{background-color: %1}").arg(c.name());
      colour1Button->setStyleSheet(qss);
    }

    connectUp();
  }
}

void PageDialog::setName() {
  if (page) {
    page->setName(Name->text());
    page->getPanel()->setupMenu();
    page->getPanel()->setEdited(true);
    page->getPanel()->update();
  }
}

void PageDialog::setMessage() {
  if (page) {
    page->setMessage(message->text());
    page->getPanel()->setEdited(true);
    page->getPanel()->update();
  }
}

void PageDialog::setOriginX(int V) {
  if (page) {
    page->getItem(0)->moveBy(V - page->getItem(0)->getOrigin().x(), 0);
    page->getPanel()->setEdited(true);
    page->getPanel()->update();
  }
}

void PageDialog::setOriginY(int V) {
  if (page) {
    page->getItem(0)->moveBy(0, V - page->getItem(0)->getOrigin().y());
    page->getPanel()->setEdited(true);
    page->getPanel()->update();
  }
}

void PageDialog::setColour1(GLfloat *c1) {
  if (page) {
    QColor c = QColor((int)(c1[0] * 255.0), (int)(c1[1] * 255.0),
                      (int)(c1[2] * 255.0));

    if (c.isValid()) {
      QString qss = QString("QPushButton{background-color: %1}").arg(c.name());
      colour1Button->setStyleSheet(qss);
    }

    page->setColour1(c1);
    page->getPanel()->setEdited(true);
    page->getPanel()->update();

  }
}

void PageDialog::colour1ButtonClicked() {
  int currRed = int(page->getColour1()[0] * 255.0);
  int currGreen = int(page->getColour1()[1] * 255.0);
  int currBlue = int(page->getColour1()[2] * 255.0);

  previousColour1 = QColor(currRed, currGreen, currBlue);

  QColorDialog *dialog = new QColorDialog(QColor(currRed, currGreen, currBlue));
  connect(dialog, SIGNAL(currentColorChanged(QColor)), this,
          SLOT(colour1SelectedFromDialog(QColor)));
  connect(dialog, SIGNAL(rejected()), this, SLOT(colour1RejectedFromDialog()));

  dialog->show();
}

void PageDialog::colour1SelectedFromDialog(QColor ret) {
  GLfloat c[3];

  if (ret.isValid()) {
    c[0] = float(ret.red()) / 255.0;
    c[1] = float(ret.green()) / 255.0;
    c[2] = float(ret.blue()) / 255.0;

    setColour1(c);
  }
}

void PageDialog::colour1RejectedFromDialog() {
  GLfloat c[3];

  if (previousColour1.isValid()) {
    c[0] = float(previousColour1.red()) / 255.0;
    c[1] = float(previousColour1.green()) / 255.0;
    c[2] = float(previousColour1.blue()) / 255.0;

    setColour1(c);
  }
}

void PageDialog::closeEvent(QCloseEvent *) { hide(); }

void PageDialog::resizeEvent(QResizeEvent *event) {

  page->getPanel()->resizeItemDialogs(event->size());
}

void PageDialog::moveEvent(QMoveEvent *) {
  if (page != NULL)
    page->getPanel()->moveItemDialogs(frameGeometry().topLeft());
}

// EOF: itemdialogs.cc
