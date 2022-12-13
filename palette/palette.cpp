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



/* Palette

   Implementation of Class: Palette

   Last Modified by: Joanne
   On Date: 14-06-01
*/

#include "palette.h"
#include <QBoxLayout>
#include <QCloseEvent>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
using namespace Qt;
// ==================== Class: Palette
// -------------------- Construction / Destruction
Palette::Palette(QString *, page *pages, SavingMode savingMode, QWidget *parent,
                 const char *)
    : QWidget(parent) {
  _savingMode = savingMode;

  QPixmap icon(":icon.png");
  setWindowIcon(icon.scaled(icon.width() / 2, icon.height() / 2,
                            Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
  init();
  connect();

  emit INIT(pages);
  // this displays a message box, it may be annoying so I commented it out...
  //     it may be useful, so I left it here...
  //  if(mess) NOTICE(*mess,0);
}

Palette::~Palette() { delete colourmap; }

void Palette::init() {
  QString text;
  colourmap = new GLColourMap(this, _savingMode);
  setFocusProxy(colourmap);

  QBoxLayout *top = new QVBoxLayout(this);
  top->setSpacing(4);

  pager = new QGroupBox("Page", this);
  pagerButtonGroup = new QButtonGroup();
  pagebuttons = new QRadioButton *[16];
  QGridLayout *vbox = new QGridLayout();
  for (int i = 0; i < 16; i++) {
    text = QString::number(i + 1);
    pagebuttons[i] = new QRadioButton(text, pager);
    pagebuttons[i]->setObjectName(QString("p") + QString::number(i));
    pagerButtonGroup->addButton(pagebuttons[i], i);
    if (i < 8)
      vbox->addWidget(pagebuttons[i], 0, i);
    else
      vbox->addWidget(pagebuttons[i], 1, i - 8);
  }

  pager->setLayout(vbox);
  pagebuttons[0]->toggle();

  top->addWidget(pager);

  QGridLayout *theslides = new QGridLayout();
  top->addLayout(theslides);

  QLabel *redlab = new QLabel("Red", this);
  redlab->setAlignment(AlignCenter);
  redlab->setPalette(QPalette(QColor(0, 0, 0), QColor(255, 125, 125)));

  redslide = new QSlider(Qt::Horizontal, this);
  redslide->setObjectName(QStringLiteral("redslide"));

  redslide->setMinimum(0);
  redslide->setMaximum(255);
  redslide->setPageStep(31);
  redslide->setValue(0);

  redslide->setTickPosition(QSlider::TicksAbove);
  redslide->setMaximumWidth(670);

  redspin = new QSpinBox(this);
  redspin->setObjectName("redspin");
  redspin->setMinimum(0);
  redspin->setMaximum(255);
  redspin->setSingleStep(1);
  redspin->setValue(0);

  redspin->setPalette(QPalette(QColor(255, 125, 125), QColor(255, 125, 125)));
  theslides->addWidget(redlab, 0, 0);
  theslides->addWidget(redslide, 0, 1);
  theslides->addWidget(redspin, 0, 2, AlignLeft);

  QLabel *greenlab = new QLabel("Green", this);
  greenlab->setAlignment(AlignCenter);
  greenlab->setPalette(QPalette(QColor(0, 0, 0), QColor(112, 255, 112)));
  greenslide = new QSlider(Qt::Horizontal, this);
  greenslide->setObjectName("greenslide");
  greenslide->setMinimum(0);
  greenslide->setMaximum(255);
  greenslide->setPageStep(31);
  greenslide->setValue(0);

  greenslide->setTickPosition(QSlider::TicksAbove);
  greenslide->setMaximumWidth(670);
  greenspin = new QSpinBox(this);
  greenspin->setObjectName("greenspin");
  greenspin->setMinimum(0);
  greenspin->setMaximum(255);
  greenspin->setSingleStep(1);
  greenspin->setValue(0);

  greenspin->setPalette(QPalette(QColor(125, 255, 125), QColor(125, 255, 125)));
  theslides->addWidget(greenlab, 1, 0);
  theslides->addWidget(greenslide, 1, 1);
  theslides->addWidget(greenspin, 1, 2, AlignLeft);

  QLabel *bluelab = new QLabel("Blue", this);
  bluelab->setAlignment(AlignCenter);
  bluelab->setPalette(QPalette(QColor(0, 0, 0), QColor(125, 125, 255)));
  blueslide = new QSlider(Qt::Horizontal, this);
  blueslide->setObjectName("blueslide");
  blueslide->setMinimum(0);
  blueslide->setMaximum(255);
  blueslide->setPageStep(31);
  blueslide->setValue(0);

  blueslide->setTickPosition(QSlider::TicksAbove);
  blueslide->setMaximumWidth(670);
  bluespin = new QSpinBox(this);
  bluespin->setObjectName("bluespin");
  bluespin->setMinimum(0);
  bluespin->setMaximum(255);
  bluespin->setSingleStep(1);
  bluespin->setValue(0);

  bluespin->setPalette(QPalette(QColor(125, 125, 255), QColor(125, 125, 255)));
  theslides->addWidget(bluelab, 2, 0);
  theslides->addWidget(blueslide, 2, 1);
  theslides->addWidget(bluespin, 2, 2, AlignLeft);

  top->addWidget(colourmap, 3);

  status = new QStatusBar(this);
  jumper = new QSpinBox(this);
  jumper->setObjectName("jumper");
  jumper->setMinimum(0);
  jumper->setMaximum(255);
  jumper->setSingleStep(1);
  jumper->setValue(0);

  status->addPermanentWidget(jumper, 0);

  top->addWidget(status);

  top->activate();

  modified = false;
}

void Palette::connect() {
  QObject::connect(redslide, SIGNAL(valueChanged(int)), redspin,
                   SLOT(setValue(int)));
  QObject::connect(redspin, SIGNAL(valueChanged(int)), redslide,
                   SLOT(setValue(int)));

  QObject::connect(greenslide, SIGNAL(valueChanged(int)), greenspin,
                   SLOT(setValue(int)));
  QObject::connect(greenspin, SIGNAL(valueChanged(int)), greenslide,
                   SLOT(setValue(int)));

  QObject::connect(blueslide, SIGNAL(valueChanged(int)), bluespin,
                   SLOT(setValue(int)));
  QObject::connect(bluespin, SIGNAL(valueChanged(int)), blueslide,
                   SLOT(setValue(int)));

  QObject::connect(this, SIGNAL(INIT(page *)), colourmap, SLOT(INIT(page *)));
  QObject::connect(pagerButtonGroup, SIGNAL(buttonClicked(int)), colourmap,
                   SLOT(PAGE(int)));

  QObject::connect(this, SIGNAL(SAVE()), colourmap, SLOT(SAVE_ALL()));

  QObject::connect(colourmap, SIGNAL(CONFIRM(const QString &)), this,
                   SLOT(CONFIRM(const QString &)));
  QObject::connect(colourmap, SIGNAL(NOTICE(const QString &, int)), this,
                   SLOT(NOTICE(const QString &, int)));

  QObject::connect(colourmap, SIGNAL(MODIFIED(bool)), this,
                   SLOT(MODIFIED(bool)));

  QObject::connect(redslide, SIGNAL(valueChanged(int)), colourmap,
                   SLOT(updateRED(int)));
  QObject::connect(redslide, SIGNAL(sliderReleased()), colourmap,
                   SLOT(sliderTriggered()));
  QObject::connect(redslide, SIGNAL(sliderMoved(int)), colourmap,
                   SLOT(sliderMoving()));
  QObject::connect(colourmap, SIGNAL(myRED(int)), redslide,
                   SLOT(setValue(int)));

  QObject::connect(greenslide, SIGNAL(valueChanged(int)), colourmap,
                   SLOT(updateGREEN(int)));
  QObject::connect(greenslide, SIGNAL(sliderReleased()), colourmap,
                   SLOT(sliderTriggered()));
  QObject::connect(greenslide, SIGNAL(sliderMoved(int)), colourmap,
                   SLOT(sliderMoving()));
  QObject::connect(colourmap, SIGNAL(myGREEN(int)), greenslide,
                   SLOT(setValue(int)));

  QObject::connect(blueslide, SIGNAL(valueChanged(int)), colourmap,
                   SLOT(updateBLUE(int)));
  QObject::connect(blueslide, SIGNAL(sliderReleased()), colourmap,
                   SLOT(sliderTriggered()));
  QObject::connect(blueslide, SIGNAL(sliderMoved(int)), colourmap,
                   SLOT(sliderMoving()));
  QObject::connect(colourmap, SIGNAL(myBLUE(int)), blueslide,
                   SLOT(setValue(int)));

  QObject::connect(colourmap, SIGNAL(SLIDERS_ON(bool)), this,
                   SLOT(SLIDERS_ON(bool)));
  QObject::connect(colourmap, SIGNAL(SELECT_ON(bool)), this,
                   SLOT(SELECT_ON(bool)));

  QObject::connect(colourmap->tune()->r(), SIGNAL(valueChanged(int)), colourmap,
                   SLOT(finetuneRED(int)));
  QObject::connect(colourmap->tune()->g(), SIGNAL(valueChanged(int)), colourmap,
                   SLOT(finetuneGREEN(int)));
  QObject::connect(colourmap->tune()->b(), SIGNAL(valueChanged(int)), colourmap,
                   SLOT(finetuneBLUE(int)));
  QObject::connect(colourmap->tune()->br(), SIGNAL(valueChanged(int)),
                   colourmap, SLOT(BRIGHT(int)));

  QObject::connect(colourmap->tune()->r(), SIGNAL(sliderPressed()), colourmap,
                   SLOT(modeR()));
  QObject::connect(colourmap->tune()->g(), SIGNAL(sliderPressed()), colourmap,
                   SLOT(modeG()));
  QObject::connect(colourmap->tune()->b(), SIGNAL(sliderPressed()), colourmap,
                   SLOT(modeB()));
  QObject::connect(colourmap->tune()->br(), SIGNAL(sliderPressed()), colourmap,
                   SLOT(modeD()));

  QObject::connect(colourmap->tune(), SIGNAL(SELECTABLE()), colourmap,
                   SLOT(SELECTABLE()));

  QObject::connect(colourmap->tune()->r(), SIGNAL(valueChanged(int)),
                   colourmap->tune(), SLOT(DISRED(int)));
  QObject::connect(colourmap->tune()->g(), SIGNAL(valueChanged(int)),
                   colourmap->tune(), SLOT(DISGRE(int)));
  QObject::connect(colourmap->tune()->b(), SIGNAL(valueChanged(int)),
                   colourmap->tune(), SLOT(DISBLU(int)));
  QObject::connect(colourmap->tune()->br(), SIGNAL(valueChanged(int)),
                   colourmap->tune(), SLOT(DISBRI(int)));

  QObject::connect(colourmap->pick(), SIGNAL(applyEvent(GLfloat *)), colourmap,
                   SLOT(SHOWPICK(GLfloat *)));

  QObject::connect(colourmap, SIGNAL(MAKEPICK()), colourmap->pick(),
                   SLOT(getColour()));

  QObject::connect(jumper, SIGNAL(valueChanged(int)), colourmap,
                   SLOT(SELECTINDEX(int)));
  QObject::connect(colourmap, SIGNAL(INDEX(int)), jumper, SLOT(setValue(int)));
}

void Palette::closeEvent(QCloseEvent *ce) {
  if (modified) {
    QMessageBox msgBox;
    msgBox.setText("Some of the colour maps have been modified.");
    msgBox.setInformativeText("Save modified files?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard |
                              QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);

    int ret = msgBox.exec();

    switch (ret) {
    case QMessageBox::Save:
      emit SAVE();

      if (modified) {
        QMessageBox msgBox;
        msgBox.setText("Modified palettes exist.");
        msgBox.setInformativeText("Exit anyway ?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Ok);
        int ret1 = msgBox.exec();
        switch (ret1) {

        case QMessageBox::Ok:
          ce->accept();
          emit quit();
          break;

        case QMessageBox::Cancel:
        default:
          ce->ignore();
          return;
          break;
        }
      }

      else {
        ce->accept();
        emit quit();
        break;
      }

    case QMessageBox::Discard:
      ce->accept();
      emit quit();
      break;

    case QMessageBox::Cancel:
    default:
      ce->ignore();
      return;
      break;
    }
  } else {
    ce->accept();
    emit quit();
  }
}

// -------------------- Slots
void Palette::CONFIRM(const QString &mess) { status->showMessage(mess, 2500); }

void Palette::NOTICE(const QString &mess, int code) {
  switch (code) {
  case -1:
    QMessageBox::critical(this, "File Error!", mess, "EXIT");
    QApplication::exit(-1);
    break;

  case 0:
    QMessageBox::information(this, "File Warning", mess, "Continue");
    break;

  case 1:
  default:
    QMessageBox::information(this, "Notice", mess, "Continue");
    break;
  }
}

void Palette::MODIFIED(bool m) { modified = m; }

void Palette::SLIDERS_ON(bool enable) {
  redslide->setEnabled(enable);
  redspin->setEnabled(enable);
  greenslide->setEnabled(enable);
  greenspin->setEnabled(enable);
  blueslide->setEnabled(enable);
  bluespin->setEnabled(enable);
}

void Palette::SELECT_ON(bool enable) { jumper->setEnabled(enable); }

// EOF: palette.cc
