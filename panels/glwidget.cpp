
#define  GL_SILENCE_DEPRECATION
#include "glwidget.h"
#include <cmath>
#include <QPainter>
#include <QPaintEngine>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QCoreApplication>

#include "paneledit.h"

GLWidget::GLWidget(Panel *mw, const QColor &background)
    : m_mainWindow(mw), m_background(background) {
  setFocusPolicy(Qt::StrongFocus);
  if (QCoreApplication::arguments().contains(QStringLiteral("--srgb")))
    setTextureFormat(GL_SRGB8_ALPHA8);
  _init = false;
}

GLWidget::~GLWidget() {}

void GLWidget::initializeGL() {
  initializeOpenGLFunctions();

  setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);

  glClearColor(m_background.red(), m_background.green(), m_background.blue(),
               1.0);
  glShadeModel(GL_FLAT);
  glClear(GL_COLOR_BUFFER_BIT);
  glFinish();
}

void GLWidget::resizeGL(int w, int h) {

  makeCurrent();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluOrtho2D(0, (GLdouble)w, 0, (GLdouble)h);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glViewport(0, 0, (GLint)w, (GLint)h);

  Page *currpage = m_mainWindow->getCurrentPage();
  currpage->setSize(w, h);
  m_mainWindow->resize(w, h);
  QString mess;

  m_mainWindow->getPanelEditor()->updatePanel();
  m_mainWindow->updateOpenDialogs();
  m_mainWindow->setEdited(true);
}

void GLWidget::paintGL() {
  GLfloat *bgCol = m_mainWindow->getBGColour();
  m_background = QColor(bgCol[0] * 255.0, bgCol[1] * 255.0, bgCol[2] * 255.0);
  glClearColor(bgCol[0], bgCol[1], bgCol[2], 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  int gridsize = m_mainWindow->getDefaultGridSize();
  GLfloat *gridvec = m_mainWindow->getGridVec();
  Page *currpage = m_mainWindow->getCurrentPage();
  QString mode = m_mainWindow->getMode();

  if (!mode.compare("EDIT") && (gridsize > 1)) {
    makeCurrent();
    glColor3fv(gridvec);
    glBegin(GL_LINES);
    for (int i = 0; i <= width(); i += gridsize) {
      glVertex2i(i, 0);
      glVertex2i(i, height());
    }
    for (int j = height(); j >= 0; j -= gridsize) {
      glVertex2i(0, j);
      glVertex2i(width(), j);
    }
    glEnd();
  }

  if (currpage) {
    currpage->draw(this);
  }
  glFlush();
}

void GLWidget::keyPressEvent(QKeyEvent *pEv) { m_mainWindow->keyPressed(pEv); }

void GLWidget::mousePressEvent(QMouseEvent *pEv) {
  m_mainWindow->mouseClicked(pEv);
}

void GLWidget::mouseDoubleClickEvent(QMouseEvent *pEv) {
  m_mainWindow->mouseDoubleClicked(pEv);
}

void GLWidget::mouseMoveEvent(QMouseEvent *pEv) {
  m_mainWindow->mouseMoved(pEv);
}

void GLWidget::mouseReleaseEvent(QMouseEvent *pEv) {
  m_mainWindow->mouseReleased(pEv);
}

// Routine to render text
inline GLint GLWidget::project(GLdouble objx, GLdouble objy, GLdouble objz,
                               const GLdouble model[16],
                               const GLdouble proj[16], const GLint viewport[4],
                               GLdouble *winx, GLdouble *winy, GLdouble *winz) {
  GLdouble in[4], out[4];
  in[0] = objx;
  in[1] = objy;
  in[2] = objz;
  in[3] = 1.0;
  transformPoint(out, model, in);
  transformPoint(in, proj, out);

  if (in[3] == 0.0)
    return GL_FALSE;

  in[0] /= in[3];
  in[1] /= in[3];
  in[2] /= in[3];

  *winx = (double)viewport[0] + ((1 + in[0]) * (double)viewport[2]) / 2.;
  *winy = (double)viewport[1] + ((1 + in[1]) * (double)viewport[3]) / 2.;

  *winz = (1 + in[2]) / 2.;
  return GL_TRUE;
}

inline void GLWidget::transformPoint(GLdouble out[4], const GLdouble m[16],
                                     const GLdouble in[4]) {
#define M(row, col) m[col * 4 + row]
  out[0] =
      M(0, 0) * in[0] + M(0, 1) * in[1] + M(0, 2) * in[2] + M(0, 3) * in[3];
  out[1] =
      M(1, 0) * in[0] + M(1, 1) * in[1] + M(1, 2) * in[2] + M(1, 3) * in[3];
  out[2] =
      M(2, 0) * in[0] + M(2, 1) * in[1] + M(2, 2) * in[2] + M(2, 3) * in[3];
  out[3] =
      M(3, 0) * in[0] + M(3, 1) * in[1] + M(3, 2) * in[2] + M(3, 3) * in[3];
#undef M
}

void GLWidget::renderText(double x, double y, double z, const QString &str,
                          const QColor color, const QFont &font) {

  // save GL attributes before calling QPainter
  if (str.isEmpty())
    return;
  glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  
  int height = this->height();

  GLdouble model[4][4], proj[4][4];
  GLint view[4];
  glGetDoublev(GL_MODELVIEW_MATRIX, &model[0][0]);
  glGetDoublev(GL_PROJECTION_MATRIX, &proj[0][0]);
  glGetIntegerv(GL_VIEWPORT, &view[0]);
  GLdouble textPosX = 0, textPosY = 0, textPosZ = 0;

  project(x, y, z, &model[0][0], &proj[0][0], &view[0], &textPosX, &textPosY,
          &textPosZ);

  textPosZ = 0;
  textPosX /= devicePixelRatio();
  textPosY /= devicePixelRatio();
  textPosY = height - textPosY; // y is inverted

  //int fontSize = font.pointSize();
  QFontMetrics metrics(font);
  int text_width = metrics.width(QString(str));

  //  int text_height = fontSize;
  int text_height = metrics.height();
  QPixmap textimg(text_width, text_height + 5);
  textimg.fill(Qt::transparent);

  QPainter painter(&textimg);
  painter.setRenderHints(QPainter::HighQualityAntialiasing |
                         QPainter::TextAntialiasing |
                         QPainter::NonCosmeticDefaultPen);
  painter.setBrush(color);
  painter.setPen(color);
  painter.setFont(font);
  painter.drawText(0, text_height, str);
  painter.end();
  QPainter p1(this);
  p1.drawPixmap(static_cast<int>(std::round(textPosX)),
                static_cast<int>(std::round(textPosY))-2, textimg);

  p1.end();

  glPopAttrib();
  glPopClientAttrib();
}
