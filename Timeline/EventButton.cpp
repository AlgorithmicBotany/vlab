#include "EventButton.h"
#include <iostream>
EventButton::EventButton(QWidget *parent) : QPushButton(parent) {_selected = false;}

EventButton::~EventButton() {}

void EventButton::updateStyle(){
  if (_selected){
    setStyleSheet(QString::fromStdString(_selectedStyle));
  }else{
    setStyleSheet(QString::fromStdString(_unselectedStyle));

  }
}

void EventButton::mousePressEvent(QMouseEvent *e) {
  _selected = true;
  //updateStyle();
  if (e->button() == Qt::LeftButton)
    emit leftClicked();

  if (e->button() == Qt::RightButton)
    emit rightClicked(e);
}

void EventButton::mouseDoubleClickEvent(QMouseEvent *e) {
  //updateStyle();
  _selected = true;
  //setStyleSheet(QString::fromStdString(_selectedStyle));
    
  if (e->button() == Qt::LeftButton){
    emit leftDoubleClicked();
    emit leftClicked();
  }
}
