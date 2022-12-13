/* ********************************************************************
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 * University of Calgary. All rights reserved.
 * ********************************************************************/

#include <QtWidgets>
#include <iostream>
#include "newVersion.h"

NewVersion::NewVersion(QWidget *parent,std::string msg, std::string longMsg, std::string new_name, bool bool_pos, bool details)
    : QDialog(parent,Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
{
    _answer = QMessageBox::Cancel;

    QString wtitle = QString("New version: ") + QString(new_name.c_str());
    std::cerr<<wtitle.toStdString()<<std::endl;
    setWindowTitle(wtitle);


    // short Message Widgets
    shortMsg = new QLabel(msg.c_str());
    QHBoxLayout *labelLayout = new QHBoxLayout;
    labelLayout->setContentsMargins( 0, 0, 0, 0 );
    QIcon icon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
    QPixmap pixmap = icon.pixmap(QSize(64, 64));
    QLabel* iconLabel = new QLabel();
    iconLabel->setPixmap(pixmap);
    labelLayout->addWidget(iconLabel,20);
    labelLayout->addWidget(shortMsg,80);
    labelLayout->setAlignment(iconLabel,Qt::AlignTop);
   // Label and box widgets
    label = new QLabel(tr("New version name: "));
    label->setAlignment(Qt::AlignLeft);
    QFont font("Arial",16);
    font.setBold(true);
    label->setFont(font);
    lineEdit = new QLineEdit;
    lineEdit->setText(QString(new_name.c_str()));

    label->setBuddy(lineEdit);
    QVBoxLayout *lineEditLayout = new QVBoxLayout;
    lineEditLayout->addWidget(label);
    lineEditLayout->addWidget(lineEdit);

    pointToPosition = new QCheckBox(tr("Point to new version"));

    //QTextEdit
    textEdit = new QTextEdit;
    textEdit->setFixedHeight(100);
    textEdit->setFocusPolicy(Qt::NoFocus);
    textEdit->setReadOnly(true);
    textEdit->setPlainText(longMsg.c_str());
    textEdit->setVisible(true);

    _visible = false;
    // Buttons

    QVBoxLayout* detailsLayout = new QVBoxLayout;
    detailsLayout->addWidget(textEdit);

    okButton = new QPushButton("Save");
    okButton->setDefault(true);
    cancelButton = new QPushButton("Cancel");
    showDetailsButton = new QPushButton("Show Details...");
    if (!details)
      showDetailsButton->setEnabled(false);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(showDetailsButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(okButton);

    _newName = new_name;

    // Slots
    connect(lineEdit, SIGNAL(textEdited(QString)),
	    SLOT(setNewName(QString)));
    connect(okButton, SIGNAL(clicked()),SLOT(okCB()));
    connect(cancelButton, SIGNAL(clicked()),SLOT(cancelCB()));
    connect(showDetailsButton, SIGNAL(clicked()),SLOT(showDetails()));
  
    // Layout
    // line widget (to decorate)
    QFrame* line1 = new QFrame();
    line1->setFrameShape(QFrame::HLine);
    line1->setFrameShadow(QFrame::Sunken);
    QFrame* line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);


    // Main layout
    QVBoxLayout *globalLayout = new QVBoxLayout;
    globalLayout->addLayout(labelLayout);
    //globalLayout->addWidget(line1);
    globalLayout->addLayout(lineEditLayout);
    globalLayout->addWidget(pointToPosition);
    globalLayout->addWidget(line2);
    globalLayout->addLayout(detailsLayout);
    globalLayout->addLayout(buttonLayout);
    //    globalLayout->addWidget(msgBox);
    setLayout(globalLayout);

    
    if (bool_pos)
      pointToPosition->setCheckState(Qt::Checked);
     globalLayout->activate();
    int fixedwitdh = globalLayout->totalMinimumSize().width();
    int fixedheight = globalLayout->totalMinimumSize().height();
    _showDetailsSize.setWidth(fixedwitdh);
    _showDetailsSize.setHeight(fixedheight);

    textEdit->setVisible(false);
    globalLayout->activate();
    fixedheight = globalLayout->totalMinimumSize().height();
    _hideDetailsSize.setWidth(fixedwitdh);
    _hideDetailsSize.setHeight(fixedheight);
    setFixedSize(fixedwitdh,fixedheight);

}
//! [5]



void NewVersion::setNewName(QString newName) {
  _newName = newName.toStdString();
}

// Search next occuration of string
void NewVersion::okCB() {
  _answer = QMessageBox::Ok;
  accept();
}

// Quit search
void NewVersion::cancelCB() {
  _answer = QMessageBox::Cancel;
  reject();
}

// Quit search
void NewVersion::newPositionClicked() {}

void NewVersion::showDetails(){
  if (!_visible){
    textEdit->setVisible(true);
    showDetailsButton->setText("Hide Details...");
    _visible = true;
    setFixedSize(_showDetailsSize);
    return;
  }
  textEdit->setVisible(false);
  showDetailsButton->setText("Show Details...");
  _visible = false;
  setFixedSize(_hideDetailsSize);  
  return;
}
