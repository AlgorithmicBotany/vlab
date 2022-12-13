#ifndef EVENTBUTTON_H
#define EVENTBUTTON_H

#include <QPushButton>
#include <QMouseEvent>
#include <iostream>

class EventButton : public QPushButton {
  Q_OBJECT

public:
  EventButton(QWidget *parent = 0);
  ~EventButton();
  void setUnselectedStyle(const std::string s){
    _unselectedStyle = s;
  }
  void setSelectedStyle(const std::string s){
    _selectedStyle = s;
  }
  void updateStyle();
  void setSelected(const bool s, int mode = 0){
    _selected = s;
    if (mode == 1)
      updateStyle();
  }
  bool selected(){
    return _selected;
  }
  
signals:
  void rightClicked(QMouseEvent *e);
  void leftClicked();
  void leftDoubleClicked();

private slots:
  void mousePressEvent(QMouseEvent *e);
  void mouseDoubleClickEvent(QMouseEvent *e);

public slots:

private:
  std::string _unselectedStyle;
  std::string _selectedStyle;
  bool _selected;
};

#endif
