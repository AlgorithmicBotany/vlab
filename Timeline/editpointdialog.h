#ifndef EDITPOINTDIALOG_H
#define EDITPOINTDIALOG_H

#include <QDialog>

class Timeline;

class QLabel;
class QDoubleSpinBox;
class QLineEdit;
class QPushButton;

class editpointdialog : public QDialog {

  Q_OBJECT

public:
  editpointdialog(int index = -1, QWidget *parent = 0);

  Timeline *timeline;
  void setColor(QColor c);
  void setEventColor(const QColor &c);


  QDoubleSpinBox *startTime;
  QDoubleSpinBox *endTime;
  QLineEdit *start;
  QLineEdit *end;
  QLineEdit *name;

private slots:
  void confirmClicked();
  void cancelClicked();
  void colorButtonClicked();
  void colorSelectedFromDialog(QColor ret);
  void colorRejectedFromDialog();


private:
  QLabel *startTimeLabel;
  QLabel *endTimeLabel;
  QLabel *startLabel;
  QLabel *endLabel;
  QLabel *nameLabel;
  QPushButton *confirm;
  QPushButton *cancel;
  int _index;
  QColor _color;
  QLabel *colorButtonLabel;
  QPushButton *colorButton;

  QColor _previousColor1;
  QColor _initialColor;

};

#endif // EDITPOINTDIALOG_H
