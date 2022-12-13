#include "deleteconfirm.h"

#include "timeline.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStyle>

deleteConfirm::deleteConfirm(QPoint pos, QWidget *parent) : QDialog(parent) {
  // Construct the dialog box and layout the widgets in the window
  setWindowFlags(Qt::WindowStaysOnTopHint);
  message = new QLabel(
      tr("Are you sure?"));
  QStyle *style =  QApplication::style();
  QIcon icon =style->standardIcon(QStyle::SP_MessageBoxQuestion);
  QPixmap pixmap = icon.pixmap(QSize(60, 60));
  QLabel *iconLabel = new QLabel;
  iconLabel->setPixmap(pixmap);


  confirm = new QPushButton(tr("Delete"));
  cancel = new QPushButton(tr("Cancel"));

  connect(confirm, SIGNAL(clicked()), this, SLOT(confirmClicked()));
  connect(cancel, SIGNAL(clicked()), this, SLOT(close()));

  QVBoxLayout *mainLayout = new QVBoxLayout;
  QHBoxLayout *messageLayout = new QHBoxLayout;

  messageLayout->addWidget(iconLabel);
  messageLayout->addWidget(message);
  QHBoxLayout *buttonLayout = new QHBoxLayout;

  buttonLayout->addWidget(cancel);
  buttonLayout->addWidget(confirm);
  mainLayout->addLayout(messageLayout);
  mainLayout->addLayout(buttonLayout);
  setLayout(mainLayout);

  setWindowTitle(tr("Delete"));
  setFixedHeight(sizeHint().height());
  _pos = pos;
  _index = -1;
}

deleteConfirm::deleteConfirm(int index, QWidget *parent) : QDialog(parent) {
  // Construct the dialog box and layout the widgets in the window
  message =
      new QLabel(tr("Are you sure?"));

  QStyle *style =  QApplication::style();
  QIcon icon =style->standardIcon(QStyle::SP_MessageBoxQuestion);
  QPixmap pixmap = icon.pixmap(QSize(60, 60));
  QLabel *iconLabel = new QLabel;
  iconLabel->setPixmap(pixmap);



  confirm = new QPushButton(tr("Delete"));
  cancel = new QPushButton(tr("Cancel"));

  connect(confirm, SIGNAL(clicked()), this, SLOT(confirmClicked()));
  connect(cancel, SIGNAL(clicked()), this, SLOT(close()));

  QVBoxLayout *mainLayout = new QVBoxLayout;
  QHBoxLayout *messageLayout = new QHBoxLayout;

  messageLayout->addWidget(iconLabel);
  messageLayout->addWidget(message);
  QHBoxLayout *buttonLayout = new QHBoxLayout;
  buttonLayout->addWidget(cancel);
  buttonLayout->addWidget(confirm);
  mainLayout->addLayout(messageLayout);
  mainLayout->addLayout(buttonLayout);
  setLayout(mainLayout);

  setWindowTitle(tr("Delete"));
  setFixedHeight(sizeHint().height());

  _index = index;
}

void deleteConfirm::confirmClicked() {
  // Delete the selected events when the delete button is clicked
  if (_index == -1)
    _index = timeline->getEventIndex(_pos);
  if (_index == -1)
    timeline->deleteSelected();
  else
    timeline->deleteEvent(_index);
  emit timeline->change() ;
  this->close();
}
