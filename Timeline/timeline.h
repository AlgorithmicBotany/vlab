#ifndef TIMELINE_H
#define TIMELINE_H

#include <iostream>
#include <float.h>
#include <string>
#include <string.h>

#include <vector>

#include <QMouseEvent>
#include <QPainter>
#include <QWidget>
#include <QGridLayout>
#include <QWheelEvent>
#include <QProcess>

#include <unistd.h>
#include "Preferences.h"
#include "event.h"

enum SavingMode { CONTINUOUS, TRIGGERED, OFF };

class Timeline : public QWidget {
  Q_OBJECT

public:
  explicit Timeline(QWidget *parent = NULL);
  explicit Timeline(const std::string &filename, Preferences *prefs,
                    QWidget *parent = NULL);
  ~Timeline();
  void cleanUp();
  
  void outputFile();
  void saveas();
  void inputFile();

  // min and max are the maximum and minimum time displayed by the timeline
  float xMin;
  float xMax;
  float yMin;
  float yMax;

  std::vector<Event *> events;

  int getFontSize() { return _fontSize; }

  void setPreferences(Preferences *prefs) { _prefs = prefs; }
  int nbSelected();
  void setMode(int mode){
    _mode = mode;
    loadConfig();
    for (int i = 0; i<events.size(); ++i)
      events[i]->update();
  }



public slots:
  void loadConfig();
  void createAxis();
  void createEvent(float startTime, float endTime, const QString &startLabel,
                   const QString &endLabel, const QString &name, const QString &color);
  bool isPointSelected();
  int getEventIndex(QPoint);
  bool isPoint();
  void deleteSelected();
  void deleteEvent(int index);
  bool selected();

  void selectPoint();
  void selectAll();
  void deselectPoint();
  void moveSelected(float dt);
  void releaseSelected();
  void swapEvents(int flag);
  void editEvent(int index, float startTime, float endTime,
                 const QString &startLabel, const QString &endLabel,
                 const QString &name, const QString& color);

  void nameClicked();
  void nameLeftClicked();
  void nameRightClicked(QMouseEvent* );
  void createFunceditProcess(QString fileName);
  std::string getTmpDir() { return _tmpDir; }
  void setSavingMode(SavingMode savingMode){ _savingMode = savingMode;}
  void resizeEvent(QResizeEvent *event) {
    // Handle window resize
    createAxis();
  }
  

signals:
  void nameRightClick(QMouseEvent*);
  void nameLeftClick(int index);
  void eventDoubleClicked();
  void change();
  
private:
  // Offsets within the timeline window from its edge to make sure everything is
  // visible
  float _xOffset;
  float _yOffset;
  float _epsilon;

  // Store the user preferences from the config file
  std::string _font;
  int _fontSize;
  int _segmentWidth;
  int _pointSize;
  int _yOffsetLabel;
  int _yOffsetStartEnd;

  std::string _fontColor;
  std::string _buttonFontColor;
  std::string _buttonLabelFontColor;

  std::string _labelBackgroundColor;
  std::string _buttonBackgroundColor;
  std::string _buttonBackgroundEditColor;

  void paintEvent(QPaintEvent *event);
  std::vector<QLabel *> axis;
  std::string _fileName;
  std::string _tmpDir;
  int _nameClicked;
  Preferences *_prefs;
  std::vector<QProcess*> processList;
  SavingMode _savingMode;
  QString lastStartMessage;
  QString lastEndMessage;
  int _mode;
};

#endif // TIMELINE_H
