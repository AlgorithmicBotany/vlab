#ifndef GLOBJWIN_H
#define GLOBJWIN_H
#define GL_SILENCE_DEPRECATION

#include <QWidget>
#include "gldisplay.h"

class GLObjectWindow : public QWidget {
  Q_OBJECT

public:
  GLObjectWindow(void);
  void emitUpdateWindow(void) { emit updateWindow(); };
  void emitUpdateVisualization(void) { emit updateVisualization(); };
  bool isOpen(void);

protected:
  void closeEvent(QCloseEvent *event);

signals:
  void updateWindow(void);
  void updateVisualization();

public slots:
  void about();
  void pdfHelp();

private:
  bool window_open;
};

#endif
