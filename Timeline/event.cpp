#include "event.h"

#include "timeline.h"

Event::Event() {
  yOffsetStartEnd = 5.;
  yOffsetLabel = 15.;
  startSelected = false;
  endSelected = false;
  name.setSelected(false);
  _customColor = false;
}

void Event::processEndMessage() {
  if (endLabel.text().isEmpty())
    return;
  QString message = QString("d ") + endLabel.text() + QString(" ") +
                    QString::number(endTime) + QString(" 1");

  if (!message.isEmpty()) {
    fprintf(stdout, "%s", message.toStdString().c_str());
    fprintf(stdout, "\n");
    fflush(stdout);
  }
  
}
void Event::processStartMessage() {
  if (startLabel.text().isEmpty())
    return;
  QString message = QString("d ") + startLabel.text() + QString(" ") +
                    QString::number(startTime) + QString(" 1");
  if (!message.isEmpty()) {
    fprintf(stdout, "%s",message.toStdString().c_str());
    fprintf(stdout, "\n");
    fflush(stdout);
  }
}

QString Event::startMessage(){
 if (startLabel.text().isEmpty())
   return QString();
  QString message = QString("d ") + startLabel.text() + QString(" ") +
                    QString::number(startTime) + QString(" 1");
  return message;
}

QString Event::endMessage(){
 if (endLabel.text().isEmpty())
   return QString();
  QString message = QString("d ") + endLabel.text() + QString(" ") +
                    QString::number(endTime) + QString(" 1");
  return message;
}
