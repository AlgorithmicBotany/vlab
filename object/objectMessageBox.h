

#ifndef OBJECTMESSAGEBOX_H
#define OBJECTMESSAGEBOX_H

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
class ObjectMessageBox : public QDialog
{
    Q_OBJECT

public:
  ObjectMessageBox(QWidget *parent ,std::string msg, std::string longMsg, std::string new_name, bool bool_pos, bool details = true, int type = 0, bool ignoreQuite = false);
  ~ObjectMessageBox() {}

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
    void ignoreCB();
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
    QPushButton* ignoreButton;
  
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
