#include "helpdialog.h"

#include "timeline.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

helpdialog::helpdialog(QWidget *parent) : QDialog(parent) {
  // Construct the dialog box and layout the widgets in the window
  setWindowFlags(Qt::WindowStaysOnTopHint);
  message = new QLabel(
      tr("Timeline Editor\n"
         "Usage: timeline <saved file>\n"
         "  - Select Events with the left mouse button\n"
         "  - Deselect Events with the right mouse button\n"
         "  - Use the scroll wheel and shift down to zoom and  in or out\n"
         "  - Click down up and down to zoom in or out\n"
         "  - Use the scroll wheel to  pan up and down\n"
         "  - Use the mouse to pan left and right\n"
         "  - Click and hold shift to select several points\n"
         "  - Click and drag with Events selected to move the points\n"
         "  - Click and drag with no Events selected to pan the view\n"
         "Shortcuts:\n"
         "  - Add Event: A\n"
         "  - Edit Event: E\n"
         "  - Delete Selected: D\n"
         "  - Select All Events to the right of the cursor: S\n"
         "  - Deselect All: X\n"
         "  - Rearrange Events: Up/Down Arrows\n"
         "  - Reload config.txt file: R\n"
         "  - Save: Ctrl+S\n"
         "  - Open: Ctrl+O\n"
         "  - New: Ctrl+N\n"));

  confirm = new QPushButton(tr("Close"));

  connect(confirm, SIGNAL(clicked()), this, SLOT(confirmClicked()));

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(message);
  mainLayout->addWidget(confirm);
  setLayout(mainLayout);

  setWindowTitle(tr("Help"));
  setFixedHeight(sizeHint().height());
}

void helpdialog::confirmClicked() { this->close(); }
