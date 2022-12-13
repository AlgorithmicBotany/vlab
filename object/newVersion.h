

#ifndef NEWVERSION_H
#define NEWVERSION_H

#include <QDialog>
#include <QCheckBox>
#include <QDialog>
#include <QLineEdit>
#include <QString>
#include <QPushButton>
#include <QCheckBox>
#include <QMessageBox>
#include <QTextEdit>
using namespace Qt;

//! [0]
class NewVersion : public QDialog
{
    Q_OBJECT

public:
  NewVersion(QWidget *parent ,std::string msg, std::string longMsg, std::string new_name, bool bool_pos, bool details = true);
  ~NewVersion() {}

  std::string getNewName(){
    return _newName;
  }

  QMessageBox::StandardButton getAnswer(){
    return _answer;
  }

  bool getNewPosition(){
    return pointToPosition->checkState()==Qt::Checked;
  }
  

public slots:
    void newPositionClicked();
    void cancelCB();
    void okCB();
    void setNewName(QString newName);
    void showDetails();

protected:

private:
    QLabel *label;
    QLineEdit *lineEdit;
    QLabel *shortMsg;
    QCheckBox *pointToPosition;
    QTextEdit *textEdit;

    QPushButton* okButton;
    QPushButton* cancelButton;
    QPushButton* showDetailsButton;
  
    bool _newPosition;
    std::string _newName;

    QMessageBox::StandardButton _answer;
    bool _visible;
    QSize _showDetailsSize;
    QSize _hideDetailsSize;

  //QGridLayout *mainLayout;
  //QSize *originalSize;


};
//! [0]

#endif
