#ifndef DELETECONFIRM_H
#define DELETECONFIRM_H

#include <QDialog>
#include <QPoint>

class Timeline;

class QLabel;
class QPushButton;

class deleteConfirm : public QDialog {
  Q_OBJECT

public:
  deleteConfirm(QPoint pos, QWidget *parent = 0);
  deleteConfirm(int index, QWidget *parent = 0);

  Timeline *timeline;

private slots:
  void confirmClicked();

private:
  QLabel *message;
  QPushButton *confirm;
  QPushButton *cancel;
  QPoint _pos;
  int _index;
};

#endif // DELETECONFIRM_H
