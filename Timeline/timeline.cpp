#include "timeline.h"

#include <QProcess>
#include <QDir>
#include <QApplication>
#include <QFileDialog>

#include <fstream>
#include <sstream>
#include <stdio.h>
#include <cmath>

#include "colors.h"
#include <platform.h>

#define POW 100.

Timeline::Timeline(QWidget *parent) : QWidget(parent) {
  // This constructor is only called on creation of a New timeline
  // Offsets for making everything visible within the timeline

  _xOffset = 0.0;
  _yOffset = 0.0;
  _epsilon = 10.0;
  // By default, the timeline goes from t=0 to t=10
  xMin = 0.0;
  xMax = 10.0;
  // Loads user configurations set in config.txt
  loadConfig();
}

Timeline::Timeline(const std::string &filename, Preferences *prefs, QWidget *parent)
    : QWidget(parent) {
  // Constructor called on program startup, works similar to above but loads a
  // startup file if specified
  _prefs = prefs;
  _xOffset = 0.0;
  _yOffset = 0.0;
  _epsilon = 5.0;
  xMin = 0.0;
  xMax = 10.0;
  _mode = 0;
  // Loads user configurations set in config.txt
  loadConfig();

  char tempdir[] = "gal.XXXXXX";
  mkdtemp(tempdir);
  _tmpDir = std::string(tempdir);
  std::string path = (QDir::currentPath()).toStdString() + "/" + _tmpDir + "/";
  _fileName = filename;

  if (!_fileName.empty()) {
    // Values to be read in from the file
    std::vector<double> startValues;
    std::vector<double> endValues;
    std::vector<std::string> names;
    std::vector<std::string> colors;
    std::vector<std::string> startLabels;
    std::vector<std::string> endLabels;
    std::string line;
    int items = -1;
    std::ifstream myfile(_fileName);
    //_fileName = argv[1];
    if (myfile.is_open()) {
      // Look for the number of events in the file using a regex
      while (getline(myfile, line)) {
        char bf[40]="";
        char buffer[40]="";
        sscanf(line.c_str(), "%[^':']:%s\n", bf, buffer);
        if ((!strcmp(bf, "items"))) {
          items = std::atoi(buffer);
          break;
        }
      }
      if (items < 0) {
        std::cerr << "Unable to parse file, missing or corrupt item number"
                  << std::endl;
        return;
      }
      // Look for each of the event parameters in order
      for (int i = 0; i < items; i++) {
        bool endOfItems = false;
        while (!endOfItems) {
          getline(myfile, line);
          char bf[40]="";
          char buffer[40] = "";
          sscanf(line.c_str(), "%[^':']:%s\n", bf, buffer);
          if (!strcmp(bf, "start")) {
            startValues.push_back(std::atof(buffer));
          } else if (!strcmp(bf, "end")) {
            endValues.push_back(std::atof(buffer));
          } else if (!strcmp(bf, "name")) {
            names.push_back(buffer);
	    colors.push_back("");
	  } else if (!strcmp(bf, "color")) {
	    colors[colors.size()-1] = buffer;
          } else if (!strcmp(bf, "startLabel")) {
            startLabels.push_back(buffer);
          } else if (!strcmp(bf, "endLabel")) {
            endLabels.push_back(buffer);
          } else if (!strcmp(bf, "timelineFunceditFileStart")) {
            std::string filePath = path + names.back();
            std::ofstream funcFile(filePath.c_str());
            if (funcFile.is_open()) {
              while (strcmp(bf, "timelineFunceditFileEnd")) {
                getline(myfile, line);
                sscanf(line.c_str(), "%[^':']:%s\n", bf, buffer);
                if (strcmp(bf, "timelineFunceditFileEnd"))
                  // Puts lines into temporary funcedit files until the funcedit
                  // end delimiter is matched
                  funcFile << line << "\n";
              }
              endOfItems = true;
              funcFile.close();
            } else {
              // This should never happen
              std::cerr << "Unable to open" + names.back() + "file"
                        << std::endl;
              endOfItems = true;
            }
          } else {
            std::cerr << "Unable to parse file, wrong label : " << line
                      << std::endl;
            return;
          }
        }
      }
      myfile.close();
      // Create the timeline from the input file
      if (startValues.size() != 0 && startValues.size() == endValues.size() &&
          startValues.size() == names.size() &&
          startValues.size() == startLabels.size() &&
          startValues.size() == endLabels.size()) {
        for (unsigned int i = 0; i < startValues.size(); i++) {
          createEvent(startValues[i], endValues[i],
                      QString::fromStdString(startLabels[i]),
                      QString::fromStdString(endLabels[i]),
                      QString::fromStdString(names[i]),
		      QString::fromStdString(colors[i]));
          if (endValues[i] >= xMax) {
            xMax = endValues[i] + (100 / _fontSize);
          }
          if (startValues[i] < xMin) {
            xMin = startValues[i] - (100 / _fontSize);
          }
        }
      }
    } else {
      std::cerr << "Unable to open file" << std::endl;
    }
  }
  yMin = 0;
  float heightLinePos = -(float)(events.size()) * _yOffset;
  yMax = heightLinePos;
  _epsilon = _fontSize;
}

Timeline::~Timeline() {
}

void Timeline::cleanUp() {
  //kill all the processes
  for (unsigned int i = 0; i < processList.size(); ++i){
    if (processList[i] != NULL){
      processList[i]->close();
      //delete processList[i];
    }
  }

  std::string path = (QDir::currentPath()).toStdString() + "/" + _tmpDir + "/";  
  for (unsigned int i = 0; i < events.size(); ++i) {
    Event *e = events[i];
    // Removes the temporary funcedit files when the timeline is closed
    std::string name = path + e->name.text().toStdString();
    remove(name.c_str());
    delete e;
  }
  // clear std::vector of events because they were deleted in the loop
  events.clear();
  rmdir(path.c_str());
}

void Timeline::loadConfig() {
  // Loads user configurations set in config.txt
  // Set default configurations. If a config file can be found, overwrite them
  // with the user specified configurations

  _fontColor = _prefs->get_gridLabel_color().name().toStdString();
  _buttonFontColor = _prefs->get_eventTitle_color().name().toStdString();
  _buttonLabelFontColor = _prefs->get_eventLabel_color().name().toStdString();
  if (_mode == 0){
    _labelBackgroundColor = _prefs->get_background_color().name().toStdString();
    _buttonBackgroundColor = _prefs->get_background_color().name().toStdString();
  }
  if (_mode == 1){
    _labelBackgroundColor = _prefs->get_backgroundEdit_color().name().toStdString();
    _buttonBackgroundColor = _prefs->get_backgroundEdit_color().name().toStdString();
  }
  _fontSize = _prefs->get_fontSize();
  _font = _prefs->get_font();
  _segmentWidth = _prefs->get_segmentWidth();
  _pointSize = _prefs->get_pointSize();
  _yOffset = -_prefs->get_yOffset();
  _xOffset = 10;
  _yOffsetLabel = _prefs->get_labelYoffset();
  _yOffsetStartEnd = _prefs->get_startEndYOffset();

  for (unsigned int i = 0; i < events.size(); ++i) {
    Event *e = events[i];

    // Style sheet for the event button and labels
    std::string fontSizeString; // string which will contain the result
    std::ostringstream convert; // stream used for the conversion
    convert << _fontSize; // insert the textual representation of 'Number' in
                          // the characters in the stream

    fontSizeString =
        convert.str(); // set 'Result' to the contents of the stream

    std::string eventLabelStyle =
        "QLabel { background-color : " + _labelBackgroundColor +
        "; color : " + _buttonLabelFontColor + "; font : " + fontSizeString +
        "pt; font-family : \"" + _font + "background-position: bottom center" +
        "\";}";
    if (e->startLabel.text().isEmpty())
      eventLabelStyle =
          "QLabel { background-color : ; color : " + _buttonLabelFontColor +
          "; font : " + fontSizeString + "pt; font-family : \"" + _font +
          "background-position: bottom center" + "\";}";
    /*
      std::string buttonStyle =
      "QPushButton { background-color : " + _buttonBackgroundColor +
      "; color : " + _buttonFontColor +
      " ; border: none; ; font : " + fontSizeString + "pt; font-family : \"" +
      _font + "\";}";
    */
    std::string style =
      "QPushButton { background-color : " + _buttonBackgroundColor +
      "; border: dotted; " + 
      "border-width: 1px; "+
      "border-radius: 5px; "+
      "color : " + _buttonFontColor +
      "; font : " + fontSizeString + "pt;  font-family : \"" + _font + "\";" + 
      " padding: 3px; ";
    

    std::string unselectedStyle = style + " border-color:  " +  _buttonBackgroundColor + "; }";
    std::string selectedStyle = style + " border-color: beige ; }";

     
    e->startLabel.setStyleSheet(QString::fromStdString(eventLabelStyle));
    e->startLabel.adjustSize();
    e->endLabel.setStyleSheet(QString::fromStdString(eventLabelStyle));
    e->endLabel.adjustSize();
    //e->name.setStyleSheet(QString::fromStdString(buttonStyle));
    e->name.setSelectedStyle(selectedStyle);
    e->name.setUnselectedStyle(unselectedStyle);

    e->name.adjustSize();
    e->yOffsetLabel = _yOffsetLabel;
    e->yOffsetStartEnd = _yOffsetStartEnd;
    e->update();
  }
  createAxis();

  update();
}

void Timeline::createAxis() {

  // Creates the axis labels
  // Clear any previous axis
  for (unsigned int i = 0; i < axis.size(); ++i) {
    delete axis[i];
  }
  axis.clear();
  // Use the range to calculate how the labels should be spaced and the counting
  // interval
  float range = xMax - xMin;
  int counter = 0;
  // Change counting interval at every power of 20
  while (range > 100) {
    range /= 10.0;
    counter++;
  }
  range = xMax - xMin;
  float width = 1. * std::pow(POW, counter) / range * this->size().width();
  int step = 1;
  if (width < 20)
    step = 2 * 20 / width;
  // Create the axis labels
  for (int i = round(xMin); i < xMax; i += step * (int)std::pow(POW, counter)) {

    QLabel *axisLabel = new QLabel();

    std::string axisNumber;      // string which will contain the result
    std::ostringstream convert1; // stream used for the conversion
    convert1 << (int)i; // insert the textual representation of 'Number' in the
                        // characters in the stream

    std::string iString =
        convert1.str(); // set 'Result' to the contents of the stream

    axisLabel->setText(iString.c_str());
    axisLabel->setParent(this);
    std::string fontSizeString; // string which will contain the result
    std::ostringstream convert; // stream used for the conversion
    convert << _fontSize; // insert the textual representation of 'Number' in
                          // the characters in the stream

    fontSizeString =
        convert.str(); // set 'Result' to the contents of the stream

    std::string text = "QLabel { background-color : " + _labelBackgroundColor +
                       "; color : " + _fontColor +
                       "; font : " + fontSizeString + "pt; font-family : \"" +
                       _font + "\";}";
    axisLabel->setStyleSheet(QString::fromStdString(text));
    axisLabel->show();
    // Move the label to the correct screen coordinates
    //axisLabel->move(((i - xMin) / range) * this->size().width() + _xOffset,
    //              this->size().height() - axisLabel->size().height());
    int xpos = ((i - xMin) / range) * this->size().width() + _xOffset;
    int ypos = this->size().height() - axisLabel->size().height();
    axisLabel->move(xpos,ypos);
    axisLabel->show();
    
    axis.push_back(axisLabel);
  }
  update();
}

void Timeline::createEvent(float startTime, float endTime,
                           const QString &startLabel, const QString &endLabel,
                           const QString &name, const QString &color) {
  Event *e = new Event;
  emit change();
  if (color.compare("") != 0){
    QRegExp rx("[, ]");// match a comma or a space
    QStringList list = color.split(rx, QString::SkipEmptyParts);
    int r = list.at(0).toInt();
    int g = list.at(1).toInt();
    int b = list.at(2).toInt();
    e->setColor(r,g,b);
  }
    
  // Style sheet for the event button and labels
  std::string starttext, endtext;
  std::string fontSizeString; // string which will contain the result
  std::ostringstream convert; // stream used for the conversion
  convert << _fontSize; // insert the textual representation of 'Number' in the
                        // characters in the stream

  fontSizeString = convert.str(); // set 'Result' to the contents of the stream

  if (startLabel.isEmpty()) {
    starttext =
        "QLabel { background-color : ; color : " + _buttonLabelFontColor +
        "; font : " + fontSizeString + "pt; font-family : \"" + _font + "\";}";
  } else {
    starttext = "QLabel { background-color : " + _buttonBackgroundColor +
                "; color : " + _buttonLabelFontColor +
                "; font : " + fontSizeString + "pt; font-family : \"" + _font +
                "\";}";
  }
  if (endLabel.isEmpty()) {
    endtext = "QLabel { background-color : ; color : " + _buttonLabelFontColor +
              "; font : " + fontSizeString + "pt; font-family : \"" + _font +
              "\";}";
  } else {
    endtext = "QLabel { background-color : " + _buttonBackgroundColor +
              "; color : " + _buttonLabelFontColor +
              "; font : " + fontSizeString + "pt; font-family : \"" + _font +
              "\";}";
  }

  std::string style =
    "QPushButton { background-color : " + _buttonBackgroundColor +
    "; border: dotted; " + 
    "border-width: 1px; "+
    "border-radius: 5px; "+
    "color : " + _buttonFontColor +
    "; font : " + fontSizeString + "pt;  font-family : \"" + _font + "\";" + 
    " padding: 3px; ";
    

  std::string unselectedStyle = style + " border-color:  " +  _buttonBackgroundColor + "; }";
  std::string selectedStyle = style + " border-color: beige ; }";

  
  // Set the event parameters
  e->startTime = startTime;
  e->endTime = endTime;
  e->yOffsetLabel = _yOffsetLabel;
  e->yOffsetStartEnd = _yOffsetStartEnd;

  e->startLabel.setText(startLabel);
  e->startLabel.setParent(this);
  e->startLabel.setStyleSheet(QString::fromStdString(starttext));
  e->startLabel.show();

  e->endLabel.setText(endLabel);
  e->endLabel.setParent(this);
  e->endLabel.setStyleSheet(QString::fromStdString(endtext));
  e->endLabel.show();

  e->name.setText(name);
  e->name.setParent(this);
  e->name.setStyleSheet(QString::fromStdString(unselectedStyle));
  e->name.setSelectedStyle(selectedStyle);
  e->name.setUnselectedStyle(unselectedStyle);
  e->name.adjustSize();
  connect(&e->name, SIGNAL(leftClicked()), this, SLOT(nameLeftClicked()));
  connect(&e->name, SIGNAL(leftDoubleClicked()), this, SLOT(nameClicked()));
  connect(&e->name, SIGNAL(rightClicked(QMouseEvent*)), this, SLOT(nameRightClicked(QMouseEvent*)));
  e->name.show();
  e->setSelected(false);
  events.push_back(e);

  // If the function file does not exist, create it
  std::string path = (QDir::currentPath()).toStdString() + "/" + _tmpDir + "/";
  std::string newName = path + e->name.text().toStdString();

  std::ifstream myfile(newName.c_str());
  if (!myfile.good()) {
    std::ofstream outfile(newName.c_str());
    if (outfile.is_open()) {
      outfile << "fver 1 1\n";
      outfile << "name: " << e->name.text().toStdString() << "\n";
      outfile << "samples: 20\n";
      outfile << "flip: off\n";
      outfile << "points: 4\n";
      outfile << "0.000000 0.000000\n";
      outfile << "0.333333 0.000000\n";
      outfile << "0.666667 0.000000\n";
      outfile << "1.000000 0.000000\n";
    } else {
      std::cerr << "Unable to create funcedit file" << std::endl;
    }
    //createFunceditProcess(e->name.text());
  }

  float heightLinePos = -(float)(events.size()) * _yOffset;
  yMax = heightLinePos + 80;
  // we place the screen in the middle
  //std::cerr<<events.size()<<std::endl;
  if (events.size() > 1)
    yMin = -heightLinePos / 2;
  
  update();
}

void Timeline::deleteEvent(int index) {
  emit change();
  // Remove the funcedit file associated with the event
  QString oldname = events[index]->name.text();
  delete (events[index]);
  events.erase(events.begin() + index);
  bool otherExists = false;
  for (unsigned int i = 0; i < events.size(); i++) {
    if (events[i]->name.text() == oldname) {
      otherExists = true;
      break;
    }
  }
  if (!otherExists) {
    std::string path =
        (QDir::currentPath()).toStdString() + "/" + _tmpDir + "/";
    std::string name = path + oldname.toStdString();
    remove(name.c_str());
  }
  float heightLinePos = -(float)(events.size()) * _yOffset;
  yMax = heightLinePos;

  update();
}

void Timeline::deleteSelected() {
  // Iterate through event and delete all that are selected
  std::vector<Event *>::iterator it;
  for (it = events.begin(); it != events.end();) {
    if ((*it)->selected()) {
      // Remove the funcedit file associated with the event
      QString oldname = (*it)->name.text();
      (*it)->setSelected(false);
      delete (*it);
      it = events.erase(it);
      bool otherExists = false;
      for (unsigned int i = 0; i < events.size(); i++) {
        if (events[i]->name.text() == oldname) {
          otherExists = true;
          break;
        }
      }
      if (!otherExists) {
        std::string path =
            (QDir::currentPath()).toStdString() + "/" + _tmpDir + "/";
        std::string name = path + oldname.toStdString();
        remove(name.c_str());
      }
    } else {
      it++;
    }
  }
  float heightLinePos = -(float)(events.size()) * _yOffset;
  yMax = heightLinePos;
  emit change();

  update();
}

void Timeline::selectPoint() {
  // Select the nearest point to the cursor within a threshold distance _epsilon
  bool *candidate = NULL;
  // Get mouse coordinates
  QPointF mousePos = mapFromGlobal(QCursor::pos());
  float currentDistance = FLT_MAX;
  float range = xMax - xMin;

  // Iterate through events looking for the point closest to the mouse pointer
  for (unsigned int i = 0; i < events.size(); i++) {
    float heightLinePos = -(float)(i + 1) * _yOffset + yMin;
    QLineF line = QLineF(
        ((events[i]->startTime - xMin) / range) * this->size().width() +
            _xOffset,
        heightLinePos,
        ((events[i]->endTime - xMin) / range) * this->size().width() + _xOffset,
        heightLinePos);
    // Check each endpoint of the event
    QPointF cursorToP1 = mousePos - line.p1();
    // Distance from mouse to p1
    float distp1 = std::sqrt((cursorToP1.x() * cursorToP1.x()) +
                             (cursorToP1.y() * cursorToP1.y()));
    if (distp1 < currentDistance && distp1 < _epsilon) {
      candidate = &events[i]->startSelected;
      currentDistance = distp1;
    }
    QPointF cursorToP2 = mousePos - line.p2();
    // Distance from mouse to p2
    float distp2 = std::sqrt((cursorToP2.x() * cursorToP2.x()) +
                             (cursorToP2.y() * cursorToP2.y()));
    if (distp2 < currentDistance && distp2 < _epsilon) {
      candidate = &events[i]->endSelected;
      currentDistance = distp2;
    }
  }
  // If a point is near enough, toggle it selected
  if (candidate != NULL) {
    *candidate = true;
  }
  update();
}

void Timeline::selectAll() {
  // Select all points to the right of the mouse cursor
  // Get the mouse position
  QPointF mousePos = mapFromGlobal(QCursor::pos());
  float range = xMax - xMin;
  // Iterate through points to find ones with greater x coordinate than that of
  // the mouse and select them
  for (unsigned int i = 0; i < events.size(); i++) {
    float heightLinePos = -(float)(i + 1) * _yOffset + yMin;

    QLineF line = QLineF(
        ((events[i]->startTime - xMin) / range) * this->size().width() +
            _xOffset,
        heightLinePos,
        ((events[i]->endTime - xMin) / range) * this->size().width() + _xOffset,
        heightLinePos);
    if (line.p1().x() > mousePos.x()) {
      events[i]->startSelected = true;
      //events[i]->selected = true;
    }
    if (line.p2().x() > mousePos.x()) {
      events[i]->endSelected = true;
      //events[i]->selected = true;
    }
  }
  update();
}

bool Timeline::isPoint() {
  // decide if the point closest to the mouse within a threshold distance
  // _epsilon Works very similar to select above
  bool candidate = false;

  QPoint pos = QCursor::pos();
  QPointF mousePos = mapFromGlobal(pos);
  float currentDistance = FLT_MAX;
  float range = xMax - xMin;
  for (unsigned int i = 0; i < events.size(); i++) {
    float heightLinePos = -(float)(i + 1) * _yOffset + yMin;

    QLineF line = QLineF(
        ((events[i]->startTime - xMin) / range) * this->size().width() +
            _xOffset,
        heightLinePos,
        ((events[i]->endTime - xMin) / range) * this->size().width() + _xOffset,
        heightLinePos);
    QPointF cursorToP1 = mousePos - line.p1();
    float distp1 = std::sqrt((cursorToP1.x() * cursorToP1.x()) +
                             (cursorToP1.y() * cursorToP1.y()));
    if (distp1 < currentDistance && distp1 < _epsilon) {
      candidate = true;
      currentDistance = distp1;
    }
    QPointF cursorToP2 = mousePos - line.p2();
    float distp2 = std::sqrt((cursorToP2.x() * cursorToP2.x()) +
                             (cursorToP2.y() * cursorToP2.y()));
    if (distp2 < currentDistance && distp2 < _epsilon) {
      candidate = true;
      currentDistance = distp2;
    }
  }
  return candidate;
}

int Timeline::getEventIndex(QPoint pos) {
  // decide if the point closest to the mouse within a threshold distance
  // _epsilon Works very similar to select above
  int candidate = -1;

  QPointF mousePos = mapFromGlobal(pos);
  float range = xMax - xMin;
  for (unsigned int i = 0; i < events.size(); i++) {
    float heightLinePos = -(float)(i + 1) * _yOffset + yMin;

    QLineF line = QLineF(
        ((events[i]->startTime - xMin) / range) * this->size().width() +
            _xOffset,
        heightLinePos,
        ((events[i]->endTime - xMin) / range) * this->size().width() + _xOffset,
        heightLinePos);

    if ((mousePos.x() >= line.p1().x() - _epsilon) &&
        (mousePos.x() <= line.p2().x() + _epsilon) &&
        (mousePos.y() <= line.p1().y() + _epsilon) &&
        (mousePos.y() >= line.p1().y() - _epsilon)) {
      candidate = i;
    }

    // otherwise check if the name is clicked
    QPoint N1(((line.x1() + line.x2()) / 2) - (events[i]->name.width() / 2),
              line.y1() - events[i]->yOffsetLabel -
                  (events[i]->name.height() / 2));
    QPoint N2(N1.x() + events[i]->name.width(),
              N1.y() + events[i]->name.height());
    if ((mousePos.x() >= N1.x() - _epsilon) &&
        (mousePos.x() <= N2.x() + _epsilon) &&
        (mousePos.y() >= N1.y() - _epsilon) &&
        (mousePos.y() <= N2.y() + _epsilon)) {
      candidate = i;
    }

  }

  return candidate;
}

bool Timeline::isPointSelected() {
  // decide if the point closest to the mouse within a threshold distance
  // _epsilon Works very similar to select above
  bool candidate = false;
  QPoint pos = QCursor::pos();
  QPointF mousePos = mapFromGlobal(pos);
  float currentDistance = FLT_MAX;
  float range = xMax - xMin;
  for (unsigned int i = 0; i < events.size(); i++) {
    float heightLinePos = -(float)(i + 1) * _yOffset + yMin;

    QLineF line = QLineF(
        ((events[i]->startTime - xMin) / range) * this->size().width() +
            _xOffset,
        heightLinePos,
        ((events[i]->endTime - xMin) / range) * this->size().width() + _xOffset,
        heightLinePos);
    QPointF cursorToP1 = mousePos - line.p1();
    float distp1 = std::sqrt((cursorToP1.x() * cursorToP1.x()) +
                             (cursorToP1.y() * cursorToP1.y()));
    if (distp1 < currentDistance && distp1 < _epsilon) {
      candidate = events[i]->startSelected;
      currentDistance = distp1;
    }
    QPointF cursorToP2 = mousePos - line.p2();
    float distp2 = std::sqrt((cursorToP2.x() * cursorToP2.x()) +
                             (cursorToP2.y() * cursorToP2.y()));
    if (distp2 < currentDistance && distp2 < _epsilon) {
      candidate = events[i]->endSelected;
      currentDistance = distp2;
    }
  }
  return candidate;
}

void Timeline::deselectPoint() {
  // Delselect point closest to the mouse within a threshold distance _epsilon
  // Works very similar to select above
  bool *candidate = NULL;
  QPoint pos = QCursor::pos();
  QPointF mousePos = mapFromGlobal(pos);
  float currentDistance = FLT_MAX;
  float range = xMax - xMin;
  for (unsigned int i = 0; i < events.size(); i++) {
    float heightLinePos = -(float)(i + 1) * _yOffset + yMin;

    QLineF line = QLineF(
        ((events[i]->startTime - xMin) / range) * this->size().width() +
            _xOffset,
        heightLinePos,
        ((events[i]->endTime - xMin) / range) * this->size().width() + _xOffset,
        heightLinePos);
    QPointF cursorToP1 = mousePos - line.p1();
    float distp1 = std::sqrt((cursorToP1.x() * cursorToP1.x()) +
                             (cursorToP1.y() * cursorToP1.y()));
    if (distp1 < currentDistance && distp1 < _epsilon) {
      candidate = &events[i]->startSelected;
      currentDistance = distp1;
    }
    QPointF cursorToP2 = mousePos - line.p2();
    float distp2 = std::sqrt((cursorToP2.x() * cursorToP2.x()) +
                             (cursorToP2.y() * cursorToP2.y()));
    if (distp2 < currentDistance && distp2 < _epsilon) {
      candidate = &events[i]->endSelected;
      currentDistance = distp2;
    }
  }
  if (candidate != NULL) {
    *candidate = false;
  }
  update();
}

void Timeline::moveSelected(float dt) {
  // Move all selected points by distance proportionate to dt
  // Scaling is done so the point moves the same screen distance as the mouse
  float range = xMax - xMin;
  for (unsigned int i = 0; i < events.size(); ++i) {
    Event *e = events[i];
    if (e->startSelected) {
      e->startTime += (dt / this->size().width()) * range;
      if (e->endTime <= e->startTime)
        e->startTime = e->endTime;
      if (_savingMode == CONTINUOUS)
	e->processStartMessage();
      lastStartMessage = e->startMessage();
    }
    if (e->endSelected) {
      e->endTime += (dt / this->size().width()) * range;
      if (e->endTime <= e->startTime)
        e->endTime = e->startTime;

      if (_savingMode == CONTINUOUS)
	e->processEndMessage();
      else
	lastEndMessage = e->endMessage();
    }
  }
  emit change();
  update();
}

void Timeline::releaseSelected() {
  if (_savingMode == TRIGGERED){
   if (!lastStartMessage.isEmpty()) {
    fprintf(stdout, "%s", lastStartMessage.toStdString().c_str());
    fprintf(stdout, "\n");
    fflush(stdout);
    lastStartMessage = QString();
   }
   if (!lastEndMessage.isEmpty()) {
    fprintf(stdout, "%s", lastEndMessage.toStdString().c_str());
    fprintf(stdout, "\n");
    fflush(stdout);
    lastEndMessage = QString();

   }

  }
  update();
}


void Timeline::swapEvents(int flag) {
  // Reorder events
  // Only one event may be selected and is moved up or down based on the flag
  // param flag = -1 is up, 1 is down
  bool selected = false;
  int index = -1;
  // Find the selected event
  for (unsigned int i = 0; i < events.size(); i++) {
    if (!selected && (events[i]->selected())){
      selected = true;
      index = i;
    } else if (selected && (events[i]->selected())) {
      selected = false;
      break;
    }
  }
  // Swap the events in the list
  if (selected) {
    if ((index + flag) >= 0 && (unsigned int)(index + flag) < events.size()) {
      std::iter_swap(events.begin() + index, events.begin() + index + flag);
      emit change();
      update();
    }
  }
}

void Timeline::editEvent(int index, float startTime, float endTime,
                         const QString &startLabel, const QString &endLabel,
                         const QString &name, const QString &color) {
  // Edits the parameters of an event
  // First find the selected event (only one can be selected)
  bool selected = false;
  if (index >= 0)
    selected = true;

  // Change the event parameters
  if (selected) {
    
  if (color.compare("") != 0){
    
    QRegExp rx("[, ]");// match a comma or a space
    QStringList list = color.split(rx, QString::SkipEmptyParts);
    int r = list.at(0).toInt();
    int g = list.at(1).toInt();
    int b = list.at(2).toInt();
    events[index]->setColor(r,g,b);
  }

    events[index]->startTime = startTime;
    events[index]->endTime = endTime;
    events[index]->startLabel.setText(startLabel);
    events[index]->startLabel.adjustSize();

    events[index]->endLabel.setText(endLabel);
    events[index]->endLabel.adjustSize();
    QString oldName = events[index]->name.text();
    if (oldName != name) {
      QString oldname = events[index]->name.text();
      events[index]->name.setText(name);
      events[index]->name.adjustSize();

      // If the function file does not exist, create it
      std::string path =
          (QDir::currentPath()).toStdString() + "/" + _tmpDir + "/";
      std::string oldNamePath = path + oldName.toStdString();
      std::string newNamePath = path + name.toStdString();
      std::ifstream funcFile(oldNamePath.c_str());
      std::ofstream newFuncFile(newNamePath.c_str());

      if (funcFile.good()) {
        std::string line;
        while (getline(funcFile, line)) {
          char bf[40]="";
          char buffer[40]="";
          sscanf(line.c_str(), "%[^':'].%[^'\n']\n", bf, buffer);
          if (!strcmp(bf, "name")) {
            newFuncFile << "name: " << name.toStdString() << std::endl;
          } else {
            newFuncFile << line << std::endl;
          }
        }
        funcFile.close();
        newFuncFile.close();
      }
    }
  }
  emit change();
  update();
}

void Timeline::paintEvent(QPaintEvent * /*event*/) {
  // Paint the timeline to the screen
  QPainter painter(this);
  painter.save();

  // Creates a black background
  if (_mode == 0)
    painter.fillRect(rect(), GetQColor(Background));
  if (_mode == 1)
    painter.fillRect(rect(), GetQColor(BackgroundEdit));
  // Set pen to green to draw axis lines
  QPen linepen;
  linepen.setColor(GetQColor(Grid));
  linepen.setWidthF(1.);
  painter.setPen(linepen);
  // Calculate spacing of axis line (same as for labels)
  float range = xMax - xMin;
  int counter = 0;
  while (range > 100) {
    range /= 10.0;
    counter++;
  }
  range = xMax - xMin;
  float width = 1. * std::pow(POW, counter) / range * this->size().width();
  int step = 1;
  if (width < 20)
    step = 20 / width;

  //    we want to display the 0 axis if it's visible
  if (xMin <= 0) {
    QLine q = QLine(((0 - xMin) / range) * this->size().width() + _xOffset, 0,
                    ((0 - xMin) / range) * this->size().width() + _xOffset,
                    this->size().height());
    painter.drawLine(q);
  }

  for (int i = round(xMin); i < xMax; i += step * (int)std::pow(POW, counter)) {
    // Construct the lines at the appropriate screen coordinates
    QLine q = QLine(((i - xMin) / range) * this->size().width() + _xOffset, 0,
                    ((i - xMin) / range) * this->size().width() + _xOffset,
                    this->size().height());
    painter.drawLine(q);
  }

  // Draw the event lines, labels and buttons
  for (unsigned int i = 0; i < events.size(); i++) {
    Event *e = events[i];
    // Lines are drawn in yellow
    QColor color = GetQColor(Segments);
    if (e->customColor()){
      color = e->color();
    }
    
    linepen.setColor(color);
    linepen.setCapStyle(Qt::RoundCap);
    linepen.setWidth(_segmentWidth);
    painter.setPen(linepen);
    float heightLinePos = -1 * (float)(i + 1) * _yOffset + yMin;
    // Construct the line at appropriate screen coordinates
    QLineF line = QLineF(
        ((e->startTime - xMin) / range) * this->size().width() + _xOffset,
        heightLinePos,
        ((e->endTime - xMin) / range) * this->size().width() + _xOffset,
        heightLinePos);
    painter.drawLine(line);

    // Draw points red or green depending on if selected
    // linepen.setWidth(10);
    QColor pointColor;
    if (e->startSelected) {
      pointColor = GetQColor(SelectedPoints);
    } else {
      pointColor = GetQColor(Points);
    }
     painter.fillRect(line.p1().x() - _pointSize / 2,
                     line.p1().y() - _pointSize / 2, _pointSize, _pointSize,
                     pointColor);
    if (e->endSelected) {
      pointColor = GetQColor(SelectedPoints);
    } else {
      pointColor = GetQColor(Points);
    }
    painter.fillRect(line.p2().x() - _pointSize / 2,
                     line.p2().y() - _pointSize / 2, _pointSize, _pointSize,
                     pointColor);

    // Move the axis labels and button relative to the event line
    QSize labelWith = e->startLabel.size();
    e->startLabel.move(line.x1() - labelWith.width() / 2,
                       line.y1() + e->yOffsetStartEnd);

    std::string startTimeString; // string which will contain the result
    std::ostringstream convert;  // stream used for the conversion
    convert << e->startTime; // insert the textual representation of 'Number' in
                             // the characters in the stream

    startTimeString =
        convert.str(); // set 'Result' to the contents of the stream

    e->startLabel.setToolTip(QString::fromStdString(startTimeString));
    labelWith = e->endLabel.size();
    e->endLabel.move(line.x2() - labelWith.width() / 2,
                     line.y2() + e->yOffsetStartEnd);

    std::string endTimeString;   // string which will contain the result
    std::ostringstream convert1; // stream used for the conversion
    convert << e->endTime; // insert the textual representation of 'Number' in
                           // the characters in the stream
    endTimeString =
        convert1.str(); // set 'Result' to the contents of the stream

    e->endLabel.setToolTip(QString::fromStdString(endTimeString));
    e->name.move(((line.x1() + line.x2()) / 2) - (e->name.width() / 2),
                 line.y1() - e->yOffsetLabel - (e->name.height() / 2));
  }
  painter.restore();
}

void Timeline::nameRightClicked(QMouseEvent* e) { emit nameRightClick(e); }

void Timeline::nameLeftClicked(){
  /*
  EventButton* button = qobject_cast<EventButton*>(sender());
  int index = -1;
  for (unsigned int i = 0; i < events.size(); i++) {
    if (&(events[i]->name) != button){
      events[i]->setSelected(false);
    }
    else{
      index = i;
    }
  }
  emit nameLeftClick(index);
  */
  emit nameLeftClick(0);
}

bool Timeline::selected(){
  for (unsigned int i = 0; i < events.size(); i++) {
    if (events[i]->selected()){
      return true;
    } 
  }
  return false;

}

int Timeline::nbSelected(){
  int count = 0;
  for (unsigned int i = 0; i < events.size(); i++) {
    if (events[i]->selected()){
      ++count;
    } 
  }
  return count;
}


void Timeline::nameClicked() {
  // check if we are in edit or execute mode:
  if (_mode == 1) { // edit mode send signal to mainwindow
    emit eventDoubleClicked();
    return;
  }
  // Execute mode => Open function editor

  // Handle calling funcedit when a button is clicked
  QPushButton *button = qobject_cast<QPushButton *>(sender());
  if (button != NULL) {
    // If the function file does not exist, create it (this should never happen)
    std::string path =
        (QDir::currentPath()).toStdString() + "/" + _tmpDir + "/";
    std::string name = path + button->text().toStdString();

    std::ifstream myfile(name.c_str());
    if (!myfile.good()) {
      std::ofstream outfile(name.c_str());
      if (outfile.is_open()) {
        outfile << "fver 1 1\n";
        outfile << "name: " << button->text().toStdString() << "\n";
        outfile << "samples: 20\n";
        outfile << "flip: off\n";
        outfile << "points: 4\n";
        outfile << "0.000000 0.000000\n";
        outfile << "0.333333 0.000000\n";
        outfile << "0.666667 0.000000\n";
        outfile << "1.000000 0.000000\n";
      } else {
        std::cerr << "Unable to open funcedit file" << std::endl;
      }
    }

    createFunceditProcess(button->text());
  }
}



void Timeline::inputFile() {
  // Open a file
  // Create standard file dialog window
  char tempdir[] = "gal.XXXXXX";
  mkdtemp(tempdir);
  _tmpDir = std::string(tempdir);
  std::string path = (QDir::currentPath()).toStdString() + "/" + _tmpDir + "/";

  QString fileName = QFileDialog::getOpenFileName(
      this, tr("Open Timeline"), "", tr("Timeline Files (*.tset)"));
  if (fileName.isEmpty()) {
    return;
  }

  _fileName = fileName.toStdString();
  // Values to be read in from the file
  std::vector<double> startValues;
  std::vector<double> endValues;
  std::vector<std::string> names;
  std::vector<std::string> colors;
  std::vector<std::string> startLabels;
  std::vector<std::string> endLabels;
  std::string line;
  int items = -1;
  std::ifstream myfile(fileName.toStdString().c_str());
  if (myfile.is_open()) {
    // Look for the number of events in the file using a regex
    while (getline(myfile, line)) {
      char bf[40]="";
      char buffer[40]="";
      sscanf(line.c_str(), "%[^':']:%s\n", bf, buffer);
      if ((!strcmp(bf, "items"))) {
        items = std::atoi(buffer);
        break;
      }
    }
    if (items < 0) {
      std::cerr << "Unable to parse file, missing item number" << std::endl;
      return;
    }
    for (int i = 0; i < items; i++) {
      bool endOfItems = false;
      while (!endOfItems) {
        getline(myfile, line);
        char bf[40]="";
        char buffer[40]="";
        sscanf(line.c_str(), "%[^':']:%s\n", bf, buffer);
        if (!strcmp(bf, "start")) {
          startValues.push_back(std::atof(buffer));
        } else if (!strcmp(bf, "end")) {
          endValues.push_back(std::atof(buffer));
        } else if (!strcmp(bf, "name: ")) {
          names.push_back(buffer);
	  colors.push_back("");
	}
	else if (!strcmp(bf, "color: ")) {
          colors[colors.size()-1] = buffer;
        } else if (!strcmp(bf, "startLabel")) {
          startLabels.push_back(buffer);
        } else if (!strcmp(bf, "endLabel")) {
          endLabels.push_back(buffer);
        } else if (!strcmp(bf, "timelineFunceditFileStart")) {
          std::string filePath = path + names.back();
          std::ofstream funcFile(filePath.c_str());
          if (funcFile.is_open()) {
            while (strcmp(bf, "timelineFunceditFileEnd")) {
              getline(myfile, line);
              sscanf(line.c_str(), "%[^':']:%s\n", bf, buffer);
              // Puts lines into temporary funcedit files until the funcedit end
              // delimiter is matched
              if (strcmp(bf, "timelineFunceditFileEnd"))
                funcFile << line << "\n";
            }
            funcFile.close();
          } else {
            // This should never happen
            std::cerr << "Unable to open" + names.back() + "file" << std::endl;
          }
          endOfItems = true;
        } else {
          std::cerr << "Unable to parse file, wrong label" << std::endl;
          return;
        }
      }
    }

    myfile.close();
    // Create the timeline from the input file
    if (startValues.size() != 0 && startValues.size() == endValues.size() &&
        startValues.size() == names.size() &&
        startValues.size() == startLabels.size() &&
        startValues.size() == endLabels.size()) {
      for (unsigned int i = 0; i < startValues.size(); i++) {
        createEvent(startValues[i], endValues[i],
                    QString::fromStdString(startLabels[i]),
                    QString::fromStdString(endLabels[i]),
                    QString::fromStdString(names[i]),
		    QString::fromStdString(colors[i]));
      }
    }
  } else {
    std::cerr << "Unable to open file" << std::endl;
  }
}

void Timeline::saveas(){
  QString fileName = QString(_fileName.c_str());

  QFileDialog dialog(this, tr("Save Timeline as"), fileName,
		     tr("Timeline Files (*.tset)"));
  dialog.setOption(QFileDialog::ShowDirsOnly,true);
  dialog.setViewMode(QFileDialog::Detail);
  dialog.setDefaultSuffix(".tset");
  dialog.setNameFilter(tr("Timeline Files (*.tset)"));
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  if (dialog.exec()) {
    fileName = dialog.selectedFiles().front();
  }
  if (fileName.isEmpty()) {
    return;
  }
  _fileName = fileName.toStdString();
  outputFile();



}

void Timeline::outputFile() {
  //first output all labelled endpoints on “Save” if we are on Triggered mode
  if (_savingMode == OFF){
    for (unsigned int i = 0; i < events.size(); ++i) {
      Event *e = events[i];
	e->processStartMessage();
	e->processEndMessage();
    }
  }
  
  // Save a .tset file
  QString fileName = QString(_fileName.c_str());
  if (fileName.isEmpty() || (fileName.compare("noname.tset") == 0)) {
    QFileDialog dialog(this, tr("Save Timeline"), QString("noname.tset"),
		       tr("Timeline Files (*.tset)"));
    dialog.setDefaultSuffix(".tset");
    dialog.setNameFilter(tr("Timeline Files (*.tset)"));
    dialog.selectFile(QString("noname.tset"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec()) {
      fileName = dialog.selectedFiles().front();
      //    fileName = QFileDialog::getSaveFileName(this, tr("Save Timeline"), "",
      //                                      tr("Timeline Files (*.tset)"));
    }
    if (fileName.isEmpty()) {
      return;
    }
    _fileName = fileName.toStdString();
  }

  // Write all of the timeline data to the file
  std::string path = (QDir::currentPath()).toStdString() + "/" + _tmpDir + "/";

  std::ofstream myfile;
  myfile.open(fileName.toStdString().c_str());
  myfile << "timeEdit 1 2\n";
  myfile << "items: " << events.size() << "\n";
  for (unsigned int i = 0; i < events.size(); ++i) {
    Event *e = events[i];
    myfile << "start: " << e->startTime << "\n";
    myfile << "end: " << e->endTime << "\n";
    myfile << "name: " << e->name.text().toStdString() << "\n";
    if (e->customColor()){
      QColor color = e->color();
      myfile << "color: " << color.red()<<","<< color.green()<<","<<color.blue()<<"\n";
    }

    myfile << "startLabel: " << e->startLabel.text().toStdString() << "\n";
    myfile << "endLabel: " << e->endLabel.text().toStdString() << "\n";
    myfile << "timelineFunceditFileStart:\n";
    std::string line;
    std::string filePath = path + e->name.text().toStdString();
    std::ifstream funcFile(filePath.c_str());
    if (funcFile.is_open()) {
      while (getline(funcFile, line)) {
        myfile << line << "\n";
      }
      funcFile.close();
    } else {
      // This should never happen
      std::cerr << "Unable to open " + path + e->name.text().toStdString() +
                       " file"
                << std::endl;
    }
    myfile << "timelineFunceditFileEnd:\n";
  }
  myfile.close();
}

void Timeline::createFunceditProcess(QString fileName) {
  // Call funcedit with the file
  QProcess *process = new QProcess(this);
  processList.push_back(process);
  std::string path = (QDir::currentPath()).toStdString() + "/" + _tmpDir + "/";
  QString fileNamePath = QString(path.c_str()) + fileName;
  std::string command = std::string("funcedit");
  if (_savingMode == OFF) {
    command += " \"" + fileNamePath.toStdString() + "\" &" ;
  }
  else if  (_savingMode == CONTINUOUS){
    command += " -rmode cont \"" +  fileNamePath.toStdString() + "\" &" ;
  }
  else  {
    command += " -rmode trig \""  +  fileNamePath.toStdString() + "\" &";
  }

  //QString file = "funcedit " + QString(path.c_str()) + fileName;
  
  process->start(QString(command.c_str()));
}
