#include <QFileInfo>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "Preferences.h"
#include "colors.h"
#include <QSettings>
#include <QTextStream>
#include <QColorDialog>
#include <QFontDialog>
#include <fstream>

Preferences::Preferences(QWidget *parent, QString fileName)
    : QDialog(parent), _fileName(fileName) {
  QString fName = "./timeline.cfg";
  QFile file(fName);
  if (file.open(QFile::ReadOnly | QFile::Text))
    _fileName = fName;

  setupUi(this);
  if (!_fileName.isEmpty()) {
    _fileNametmp = _fileName + QString("~");
    QFile::remove(_fileNametmp);

    QFile file(_fileName);
    if (file.open(QFile::ReadOnly | QFile::Text))
      plainTextEdit->setPlainText(file.readAll());

    // copy file to a temporary file so we can reload in case we cancel changes
    QFile::copy(_fileName, _fileNametmp);
  }
  loadConfig();

  connect(buttonBox, SIGNAL(rejected()), SLOT(cancel()));

  connect(tabWidget, SIGNAL(currentChanged(int)), SLOT(Save(int)));
  connect(buttonBox, SIGNAL(clicked(QAbstractButton *)),
          SLOT(Save(QAbstractButton *)));
  connect(button_selected_color, SIGNAL(clicked()),
          SLOT(pick_selected_color()));
  connect(button_unselected_color, SIGNAL(clicked()),
          SLOT(pick_unselected_color()));
  connect(button_background_color, SIGNAL(clicked()),
          SLOT(pick_background_color()));
  connect(button_backgroundEdit_color, SIGNAL(clicked()),
          SLOT(pick_backgroundEdit_color()));
  connect(button_segment_color, SIGNAL(clicked()), SLOT(pick_segment_color()));
  connect(button_grid_color, SIGNAL(clicked()), SLOT(pick_grid_color()));
  connect(button_eventTitle_color, SIGNAL(clicked()),
          SLOT(pick_eventTitle_color()));
  connect(button_eventLabel_color, SIGNAL(clicked()),
          SLOT(pick_eventLabel_color()));
  connect(button_gridLabel_color, SIGNAL(clicked()),
          SLOT(pick_gridLabel_color()));

  // set spin boxes
  connect(segmentWidth_spinBox, SIGNAL(valueChanged(int)),
          SLOT(set_SegmentWidth(int)));
  connect(labelYOffset_spinBox, SIGNAL(valueChanged(int)),
          SLOT(set_labelYoffset(int)));
  connect(startEndYOffset_spinBox, SIGNAL(valueChanged(int)),
          SLOT(set_startEndYOffset(int)));
  connect(yOffset_spinBox, SIGNAL(valueChanged(int)), SLOT(set_yOffset(int)));
  connect(pointSize_spinBox, SIGNAL(valueChanged(int)),
          SLOT(set_pointSize(int)));

  connect(font_button, SIGNAL(clicked()), SLOT(font_cb()));
  gridLayout_3->setColumnStretch(0, 132);
  gridLayout_2->setColumnStretch(0, 132);
  gridLayout->setColumnStretch(0, 132);

  this->setWindowTitle("Preferences");
}

Preferences::~Preferences() {
  // delete temporary file ?
}

void Preferences::font_cb() {
  QFont currentFont = QFont(QString(_font.c_str()), _fontSize);
  _previousFont = _font;
  _previousPointSize = _fontSize;
  QFontDialog *qfd = new QFontDialog(this);
  qfd->setModal(false);

  qfd->setOption(QFontDialog::DontUseNativeDialog);
  qfd->setWindowTitle("Select font");
  qfd->setCurrentFont(currentFont);
  connect(qfd, SIGNAL(currentFontChanged(QFont)), SLOT(setFont(QFont)));
  connect(qfd, SIGNAL(rejected()), this, SLOT(resetFont()));
  qfd->show();
}

void Preferences::Save(QAbstractButton *button) {
  if ((QPushButton *)button == buttonBox->button(QDialogButtonBox::Cancel))
    return;
  if (!_fileName.isEmpty()) {
    QString preferences = plainTextEdit->toPlainText();
    QFile file(_fileName);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
      QTextStream out(&file);
      out << preferences;
      file.close();
    }
    loadConfig();
    emit configChanged();
  }
}
void Preferences::Save(int index) {
  if (index == 1)
    return;
  if (!_fileName.isEmpty()) {
    QString preferences = plainTextEdit->toPlainText();
    QFile file(_fileName);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
      QTextStream out(&file);
      out << preferences;
      file.close();
    }
    loadConfig();
    emit configChanged();
  }
}

void Preferences::cancel() {
  QFile::remove(_fileName);
  QFile::copy(_fileNametmp, _fileName);
  loadConfig();
  emit configChanged();
}

void Preferences::paste() {}

void Preferences::setFont(QFont font) {
  _font = font.family().toStdString();
  _fontSize = font.pointSize();
  WriteColors(_fileName.toStdString());
}

void Preferences::resetFont() {
  _font = _previousFont;
  _fontSize = _previousFontSize;
  WriteColors(_fileName.toStdString());
}

void Preferences::setSpinBox(QSpinBox *spinBox, int value) {
  spinBox->setMaximum(250);
  spinBox->setValue(value);
}

void Preferences::set_SegmentWidth(int value) {
  _segmentWidth = value;
  WriteColors(_fileName.toStdString());
}

void Preferences::set_pointSize(int value) {
  _pointSize = value;
  WriteColors(_fileName.toStdString());
}

void Preferences::set_yOffset(int value) {
  _yOffset = value;
  WriteColors(_fileName.toStdString());
}

void Preferences::set_startEndYOffset(int value) {
  _startEndYOffset = value;
  WriteColors(_fileName.toStdString());
}

void Preferences::set_labelYoffset(int value) {
  _labelYoffset = value;
  WriteColors(_fileName.toStdString());
}

void Preferences::pick_selected_color() {
  QColorDialog *colorDialog = new QColorDialog(_selected_color, this);
  _previousSelected_color = _selected_color;
  colorDialog->setModal(false);
  colorDialog->setOption(QColorDialog::DontUseNativeDialog);
  connect(colorDialog, SIGNAL(currentColorChanged(QColor)),
          SLOT(setButton_selected_color(QColor)));
  connect(colorDialog, SIGNAL(rejected()), SLOT(resetButton_selected_color()));

  colorDialog->show();
  colorDialog->setCurrentColor(_selected_color);
}

void Preferences::setButton_selected_color(const QColor &color) {
  _selected_color = color;
  setButtonColor(button_selected_color, _selected_color);
  setColor(SelectedPoints, _selected_color);
  WriteColors(_fileName.toStdString());
}

void Preferences::resetButton_selected_color() {
  setButton_selected_color(_previousSelected_color);
}

void Preferences::pick_unselected_color() {
  QColorDialog *colorDialog = new QColorDialog(_unselected_color, this);
  _previousUnselected_color = _unselected_color;
  colorDialog->setModal(false);
  colorDialog->setOption(QColorDialog::DontUseNativeDialog);
  connect(colorDialog, SIGNAL(currentColorChanged(QColor)),
          SLOT(setButton_unselected_color(QColor)));
  connect(colorDialog, SIGNAL(rejected()),
          SLOT(resetButton_unselected_color()));

   colorDialog->show();
  colorDialog->setCurrentColor(_unselected_color);
}

void Preferences::setButton_unselected_color(const QColor &color) {
  _unselected_color = color;
  setButtonColor(button_unselected_color, _unselected_color);
  setColor(Points, _unselected_color);
  WriteColors(_fileName.toStdString());
}

void Preferences::resetButton_unselected_color() {
  setButton_unselected_color(_previousUnselected_color);
}

void Preferences::pick_background_color() {
  QColorDialog *colorDialog = new QColorDialog(_background_color, this);
  _previousBackground_color = _background_color;
  colorDialog->setModal(false);
  colorDialog->setOption(QColorDialog::DontUseNativeDialog);
  connect(colorDialog, SIGNAL(currentColorChanged(QColor)),
          SLOT(setButton_background_color(QColor)));
  connect(colorDialog, SIGNAL(rejected()),
          SLOT(resetButton_background_color()));

  colorDialog->show();
  colorDialog->setCurrentColor(_background_color);
}

void Preferences::pick_backgroundEdit_color() {
  QColorDialog *colorDialog = new QColorDialog(_backgroundEdit_color, this);
  _previousBackgroundEdit_color = _backgroundEdit_color;
  colorDialog->setModal(false);
  colorDialog->setOption(QColorDialog::DontUseNativeDialog);
  connect(colorDialog, SIGNAL(currentColorChanged(QColor)),
          SLOT(setButton_backgroundEdit_color(QColor)));
  connect(colorDialog, SIGNAL(rejected()),
          SLOT(resetButton_backgroundEdit_color()));

  colorDialog->show();
  colorDialog->setCurrentColor(_backgroundEdit_color);
}

void Preferences::setButton_background_color(const QColor &color) {
  _background_color = color;
  setButtonColor(button_background_color, _background_color);
  setColor(Background, _background_color);
  WriteColors(_fileName.toStdString());
}

void Preferences::resetButton_background_color() {
  setButton_background_color(_previousBackground_color);
}

void Preferences::setButton_backgroundEdit_color(const QColor &color) {
  _backgroundEdit_color = color;
  setButtonColor(button_backgroundEdit_color, _backgroundEdit_color);
  setColor(BackgroundEdit, _backgroundEdit_color);
  WriteColors(_fileName.toStdString());
}

void Preferences::resetButton_backgroundEdit_color() {
  setButton_backgroundEdit_color(_previousBackgroundEdit_color);
}

void Preferences::pick_segment_color() {
  QColorDialog *colorDialog = new QColorDialog(_segment_color, this);
  _previousSegment_color = _segment_color;
  colorDialog->setModal(false);
  colorDialog->setOption(QColorDialog::DontUseNativeDialog);
  connect(colorDialog, SIGNAL(currentColorChanged(QColor)),
          SLOT(setButton_segment_color(QColor)));
  colorDialog->show();
  colorDialog->setCurrentColor(_segment_color);
}

void Preferences::setButton_segment_color(const QColor &color) {
  _segment_color = color;
  setButtonColor(button_segment_color, _segment_color);
  setColor(Segments, _segment_color);
  WriteColors(_fileName.toStdString());
}

void Preferences::resetButton_segment_color() {
  setButton_segment_color(_previousSegment_color);
}

void Preferences::pick_grid_color() {
  QColorDialog *colorDialog = new QColorDialog(_grid_color, this);
  _previousGrid_color = _grid_color;
  colorDialog->setModal(false);
  colorDialog->setOption(QColorDialog::DontUseNativeDialog);
  connect(colorDialog, SIGNAL(currentColorChanged(QColor)),
          SLOT(setButton_grid_color(QColor)));
  connect(colorDialog, SIGNAL(rejected()), SLOT(resetButton_grid_color()));

   colorDialog->show();
  colorDialog->setCurrentColor(_grid_color);
}

void Preferences::setButton_grid_color(const QColor &color) {
  _grid_color = color;
  setButtonColor(button_grid_color, _grid_color);
  setColor(Grid, _grid_color);
  WriteColors(_fileName.toStdString());
}

void Preferences::resetButton_grid_color() {
  setButton_grid_color(_previousGrid_color);
}

void Preferences::pick_eventTitle_color() {
  QColorDialog *colorDialog = new QColorDialog(_eventTitle_color, this);
  _previousEventTitle_color = _eventTitle_color;
  colorDialog->setModal(false);
  colorDialog->setOption(QColorDialog::DontUseNativeDialog);
  connect(colorDialog, SIGNAL(currentColorChanged(QColor)),
          SLOT(setButton_eventTitle_color(QColor)));
  connect(colorDialog, SIGNAL(rejected()),
          SLOT(resetButton_eventTitle_color()));
  colorDialog->show();
  colorDialog->setCurrentColor(_eventTitle_color);
}

void Preferences::setButton_eventTitle_color(const QColor &color) {
  _eventTitle_color = color;
  setButtonColor(button_eventTitle_color, _eventTitle_color);
  setColor(ButtonTitles, _eventTitle_color);
  WriteColors(_fileName.toStdString());
}

void Preferences::resetButton_eventTitle_color() {
  setButton_eventTitle_color(_previousEventTitle_color);
}

void Preferences::pick_eventLabel_color() {
  QColorDialog *colorDialog = new QColorDialog(_eventLabel_color, this);
  _previousEventLabel_color = _eventLabel_color;
  colorDialog->setModal(false);
  colorDialog->setOption(QColorDialog::DontUseNativeDialog);
  connect(colorDialog, SIGNAL(currentColorChanged(QColor)),
          SLOT(setButton_eventLabel_color(QColor)));
  connect(colorDialog, SIGNAL(rejected()),
          SLOT(resetButton_eventLabel_color()));

  colorDialog->show();
  colorDialog->setCurrentColor(_eventLabel_color);
}

void Preferences::setButton_eventLabel_color(const QColor &color) {
  _eventLabel_color = color;
  setButtonColor(button_eventLabel_color, _eventLabel_color);
  setColor(ButtonLabels, _eventLabel_color);
  WriteColors(_fileName.toStdString());
}

void Preferences::resetButton_eventLabel_color() {
  setButton_eventLabel_color(_previousEventLabel_color);
}

void Preferences::pick_gridLabel_color() {
  QColorDialog *colorDialog = new QColorDialog(_gridLabel_color, this);
  _previousGridLabel_color = _gridLabel_color;
  colorDialog->setModal(false);
  colorDialog->setOption(QColorDialog::DontUseNativeDialog);
  connect(colorDialog, SIGNAL(currentColorChanged(QColor)),
          SLOT(setButton_gridLabel_color(QColor)));
  connect(colorDialog, SIGNAL(rejected()), SLOT(resetButton_gridLabel_color()));

  colorDialog->show();
  colorDialog->setCurrentColor(_gridLabel_color);
}

void Preferences::setButton_gridLabel_color(const QColor &color) {
  _gridLabel_color = color;
  setButtonColor(button_gridLabel_color, _gridLabel_color);
  setColor(GridLabels, _gridLabel_color);
  WriteColors(_fileName.toStdString());
}

void Preferences::resetButton_gridLabel_color() {
  setButton_gridLabel_color(_previousGridLabel_color);
}

void Preferences::setButtonColor(QPushButton *button, const QColor &color) {

  // Style sheet for the event button and labels
  button->setFixedSize(QSize(20, 20));
  std::string colorString = color.name().toStdString();
  std::string style =
      "background-color : " + colorString + ";color: " + colorString + ";";
  button->setStyleSheet(QString::fromStdString(style));
  button->setAutoFillBackground(true);
}

void Preferences::WriteColors(std::string fileName) {
  std::ofstream out;
  out.open(fileName.c_str());
  // 1st write colors
  for (int i = 0; i <= BackgroundEdit; i++) {
    out << GetLabel(i) << ": " << GetColor(i, 0) << " " << GetColor(i, 1) << " "
        << GetColor(i, 2) << "\n";
  }
  // 2nd write other options

  out << "font: " << _font << "\n";
  out << "font size: " << _fontSize << "\n";
  out << "segment width: " << _segmentWidth << "\n";
  out << "point size: " << _pointSize << "\n";
  out << "offset between segments: " << _yOffset << "\n";
  out << "Label Y offset: " << _labelYoffset << "\n";
  out << "Start/End Y offset: " << _startEndYOffset << "\n";
  out.close();

  QFile file(fileName.c_str());
  if (file.open(QFile::ReadOnly | QFile::Text))
    plainTextEdit->setPlainText(file.readAll());

  emit configChanged();
}

void Preferences::loadConfig() {
  // 1s read colors
  //ReadColors(_fileName);

  // 2nd read other options
  std::string line;

  const char *bf = _fileName.toStdString().c_str();

  std::ifstream myfile(bf);
  // Attempt to open and parse the config file

  if (myfile.is_open()) {
    // When matches are found, set the config values

    while (getline(myfile, line)) {
      char bf[40];
      char buffer[40];
      sscanf(line.c_str(), "%[^':']:%[^'\n']\n", bf, buffer);

      if (!strcmp(bf, "font")) {
        _font = buffer;
      } else if (!strcmp(bf, "font size")) {
        _fontSize = std::atoi(buffer);
      } else if (!strcmp(bf, "segment width")) {
        _segmentWidth = std::atoi(buffer);
      } else if (!strcmp(bf, "point size")) {
        _pointSize = std::atoi(buffer);
      } else if (!strcmp(bf, "Start/End Y offset")) {
        _startEndYOffset = std::atoi(buffer);
      } else if (!strcmp(bf, "Label Y offset")) {
        _labelYoffset = std::atoi(buffer);
      } else if (!strcmp(bf, "offset between segments")) {
        _yOffset = std::atoi(buffer);
      }
      ReadColors(line);
    }
    myfile.close();
  } else {
    std::cerr << "Unable to open config file, using default configuration"
              << std::endl;
    _yOffset = 60.0;
    _labelYoffset = 15.0;
    _startEndYOffset = 5.0;
    _fontSize = 12;
    _font = "Lucida Grande";
    _segmentWidth = 2.;
    _pointSize = 10.;
  }

  _background_color = GetQColor(Background);
  _unselected_color = GetQColor(Points);
  _segment_color = GetQColor(Segments);
  _grid_color = GetQColor(Grid);
  _selected_color = GetQColor(SelectedPoints);
  _eventTitle_color = GetQColor(ButtonTitles);
  _gridLabel_color = GetQColor(GridLabels);
  _eventLabel_color = GetQColor(ButtonLabels);
  _backgroundEdit_color = GetQColor(BackgroundEdit);

  // set Buttons
  setButtonColor(button_selected_color, _selected_color);
  setButtonColor(button_unselected_color, _unselected_color);
  setButtonColor(button_background_color, _background_color);
  setButtonColor(button_backgroundEdit_color, _backgroundEdit_color);
  setButtonColor(button_segment_color, _segment_color);
  setButtonColor(button_grid_color, _grid_color);
  setButtonColor(button_eventTitle_color, _eventTitle_color);
  setButtonColor(button_eventLabel_color, _eventLabel_color);
  setButtonColor(button_gridLabel_color, _gridLabel_color);

  // set spinBoxes
  setSpinBox(segmentWidth_spinBox, _segmentWidth);
  setSpinBox(labelYOffset_spinBox, _labelYoffset);
  setSpinBox(startEndYOffset_spinBox, _startEndYOffset);
  setSpinBox(yOffset_spinBox, _yOffset);
  setSpinBox(pointSize_spinBox, _pointSize);
}
