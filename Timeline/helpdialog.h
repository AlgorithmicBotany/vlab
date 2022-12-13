#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>

class Timeline;

class QLabel;
class QPushButton;

class helpdialog : public QDialog {
  Q_OBJECT

public:
  helpdialog(QWidget *parent = 0);

  Timeline *timeline;

private slots:
  void confirmClicked();

private:
  QLabel *message;
  QPushButton *confirm;
};

#endif // HELPDIALOG_H
