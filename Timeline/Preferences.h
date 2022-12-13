#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>
#include <QDir>
#include <QFileDialog>
#include <QMenu>
#include "ui_Preferences.h"

class Preferences : public QDialog, public Ui_Preferences {
  Q_OBJECT

protected:

public:
  Preferences(QWidget *parent = 0, QString objName = NULL);
  ~Preferences();
  void WriteColors(std::string line);
  void loadConfig();

signals:
  void configChanged();

public slots:
  void Save(QAbstractButton *);
  void Save(int);
  void cancel();
  void paste();
  void keyPressEvent(QKeyEvent *e) {
    if (e->key() != Qt::Key_Escape)
      QDialog::keyPressEvent(e);
    else { /* minimize */
    }
  } // this is to avoid escape key to close the dialog

  void pick_selected_color();
  void pick_unselected_color();
  void pick_background_color();
  void pick_backgroundEdit_color();
  void pick_segment_color();
  void pick_grid_color();
  void pick_eventTitle_color();
  void pick_eventLabel_color();
  void pick_gridLabel_color();

  void setButtonColor(QPushButton *, const QColor &);
  void setSpinBox(QSpinBox *spinBox, int value);

  void setButton_selected_color(const QColor &color);
  void setButton_unselected_color(const QColor &color);
  void setButton_background_color(const QColor &color);
  void setButton_backgroundEdit_color(const QColor &color);
  void setButton_segment_color(const QColor &color);
  void setButton_grid_color(const QColor &color);
  void setButton_eventTitle_color(const QColor &color);
  void setButton_eventLabel_color(const QColor &color);
  void setButton_gridLabel_color(const QColor &color);

  void resetButton_selected_color();
  void resetButton_unselected_color();
  void resetButton_background_color();
  void resetButton_backgroundEdit_color();
  void resetButton_segment_color();
  void resetButton_grid_color();
  void resetButton_eventTitle_color();
  void resetButton_eventLabel_color();
  void resetButton_gridLabel_color();

  void set_SegmentWidth(int value);
  void set_pointSize(int value);
  void set_yOffset(int value);
  void set_startEndYOffset(int value);
  void set_labelYoffset(int value);

  void setFont(QFont font);
  void resetFont();

  void font_cb();

  QColor get_background_color() { return _background_color; }
  QColor get_backgroundEdit_color() { return _backgroundEdit_color; }

  QColor get_unselected_color() { return _unselected_color; }
  QColor get_segment_color() { return _segment_color; }
  QColor get_grid_color() { return _grid_color; }

  QColor get_selected_color() { return _selected_color; }
  QColor get_eventTitle_color() { return _eventTitle_color; }
  QColor get_eventLabel_color() { return _eventLabel_color; }
  QColor get_gridLabel_color() { return _gridLabel_color; }
  int get_labelYoffset() { return _labelYoffset; }
  int get_startEndYOffset() { return _startEndYOffset; }

  int get_yOffset() { return _yOffset; }
  int get_segmentWidth() { return _segmentWidth; }
  std::string get_font() { return _font; }
  int get_fontSize() { return _fontSize; }
  int get_pointSize() { return _pointSize; }

private:
  QString _fileName;
  QString _fileNametmp;

  QMenu *_editMenu;
  std::string _font;
  int _fontSize;
  int _pointSize;
  QColor _background_color;
  QColor _unselected_color;
  QColor _segment_color;
  QColor _grid_color;
  QColor _selected_color;
  QColor _eventTitle_color;
  QColor _eventLabel_color;
  QColor _gridLabel_color;
  QColor _backgroundEdit_color;
  int _labelYoffset;
  int _startEndYOffset;
  int _yOffset;
  int _segmentWidth;

  std::string _previousFont;
  int _previousFontSize;
  int _previousPointSize;
  QColor _previousBackground_color;
  QColor _previousUnselected_color;
  QColor _previousSegment_color;
  QColor _previousGrid_color;
  QColor _previousSelected_color;
  QColor _previousEventTitle_color;
  QColor _previousEventLabel_color;
  QColor _previousGridLabel_color;
  QColor _previousBackgroundEdit_color;

};

#endif // PREFERENCES_H
