#ifndef EVENT_H
#define EVENT_H

#include <QLabel>
#include <QPushButton>
#include <QMouseEvent>
#include <iostream>
#include "EventButton.h"
class Timeline;

class EventButton;

class Event {

public:
  Event();

  void processEndMessage();
  void processStartMessage();

  QString startMessage();
  QString endMessage();

  void setSelected(bool s, int mode = 0){
    name.setSelected(s, mode);
  }

  void update(){
    name.updateStyle();
  }
  
  bool selected(){
    return name.selected();
  }

  void setCustomColor(bool c){
    _customColor = c;
  }
  
  bool customColor(){
    return _customColor;
  }

  QColor color(){
    return _color;
  }

  void setColor(const int r, const int g, const int b){
    _customColor = true;
    _color = QColor(r,g,b);
  }
  
  // Data for storing timeline events
  float startTime;
  float endTime;
  bool startSelected;
  bool endSelected;
  // These constants handle position of buttons and labels relative to the event
  // line
  float yOffsetStartEnd;
  float yOffsetLabel;

  QLabel startLabel;
  QLabel endLabel;
  EventButton name;

private:
  QColor _color;
  bool _customColor;
 signals:

public slots:
};

#endif // EVENT_H
