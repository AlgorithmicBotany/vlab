#ifndef ADDPOINTDIALOG_H
#define ADDPOINTDIALOG_H

#include <QDialog>

class Timeline;

class QLabel;
class QDoubleSpinBox;
class QLineEdit;
class QPushButton;

class addPointDialog : public QDialog {
  Q_OBJECT

public:
  addPointDialog(QWidget *parent = 0);

  Timeline *timeline;
  void setColor(QColor c);

private slots:
  void confirmClicked();
  void colorButtonClicked();
  void colorSelectedFromDialog(QColor ret);
  void colorRejectedFromDialog();


private:
  QLabel *startTimeLabel;
  QLabel *endTimeLabel;
  QLabel *startLabel;
  QLabel *endLabel;
  QLabel *nameLabel;
  QDoubleSpinBox *startTime;
  QDoubleSpinBox *endTime;
  QLineEdit *start;
  QLineEdit *end;
  QLineEdit *name;
  QPushButton *confirm;
  QPushButton *cancel;
  QColor _color;
  QLabel *colorButtonLabel;
  QPushButton *colorButton;

  QColor _previousColor1;
};

#endif // ADDPOINTDIALOG_H
