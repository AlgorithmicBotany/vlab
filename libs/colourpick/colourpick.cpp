/* ColourPick - A Custom-Built Widget for your Application
                ( this is the best colour-picker ever! )

   Implementation of Classes: ColourPick
                              ColourWheel

   Last Modified by: Joanne
   On Date: 30-08-01
*/

#include "colourpick.h"

// ==================== Class: ColourPick
// ------------------- Construction/Destruction
ColourPick::ColourPick(QWidget *parent, const char *name, bool fixsize,
                       WFlags f)
    : QWidget(parent, name, f) {

  setCaption("Colour Pick : " + parentWidget()->caption());
  slidermode = "no";

  QBoxLayout *top = new QVBoxLayout(this);
  top->setMargin(3);

  QBoxLayout *upper = new QHBoxLayout(top, 5);
  upper->setMargin(3);

  // sliders
  QGridLayout *slides = new QGridLayout(upper, 6, 4, 3);
  slides->setMargin(3);
  slides->addColSpacing(1, 150);
  slides->addColSpacing(2, 75);

  QLabel *hL1 = new QLabel("H", this, "hL1");
  QLabel *hL2 = new QLabel("H", this, "hL2");
  hSl =
      new QSlider(UNDEFINED, 36000, 4500, 0, QSlider::Horizontal, this, "hSl");
  hSl->setTickmarks(QSlider::Above);
  hLe = new QLineEdit("UNDEF", this, "hLe");
  hLe->setMaximumWidth(85);
  slides->addWidget(hL1, 0, 0);
  slides->addWidget(hSl, 0, 1);
  slides->addWidget(hLe, 0, 2);
  slides->addWidget(hL2, 0, 3);

  QLabel *sL1 = new QLabel("S", this, "sL1");
  QLabel *sL2 = new QLabel("S", this, "sL2");
  sSl = new QSlider(0, 1000, 125, 0, QSlider::Horizontal, this, "sSl");
  sSl->setTickmarks(QSlider::Above);
  sLe = new QLineEdit("0.000", this, "sLe");
  sLe->setMaximumWidth(95);
  slides->addWidget(sL1, 1, 0);
  slides->addWidget(sSl, 1, 1);
  slides->addWidget(sLe, 1, 2);
  slides->addWidget(sL2, 1, 3);

  QLabel *vL1 = new QLabel("V", this, "vL1");
  QLabel *vL2 = new QLabel("V", this, "vL2");
  vSl = new QSlider(0, 1000, 125, 0, QSlider::Horizontal, this, "vSl");
  vSl->setTickmarks(QSlider::Above);
  vLe = new QLineEdit("0.000", this, "vLe");
  vLe->setMaximumWidth(95);
  slides->addWidget(vL1, 2, 0);
  slides->addWidget(vSl, 2, 1);
  slides->addWidget(vLe, 2, 2);
  slides->addWidget(vL2, 2, 3);

  QLabel *rL1 = new QLabel("R", this, "rL1");
  QLabel *rL2 = new QLabel("R", this, "rL2");
  rSl = new QSlider(0, 1000, 125, 0, QSlider::Horizontal, this, "rSl");
  rSl->setTickmarks(QSlider::Above);
  rLe = new QLineEdit("0.000", this, "rLe");
  rLe->setMaximumWidth(95);
  slides->addWidget(rL1, 3, 0);
  slides->addWidget(rSl, 3, 1);
  slides->addWidget(rLe, 3, 2);
  slides->addWidget(rL2, 3, 3);

  QLabel *gL1 = new QLabel("G", this, "gL1");
  QLabel *gL2 = new QLabel("G", this, "gL2");
  gSl = new QSlider(0, 1000, 125, 0, QSlider::Horizontal, this, "gSl");
  gSl->setTickmarks(QSlider::Above);
  gLe = new QLineEdit("0.000", this, "gLe");
  gLe->setMaximumWidth(95);
  slides->addWidget(gL1, 4, 0);
  slides->addWidget(gSl, 4, 1);
  slides->addWidget(gLe, 4, 2);
  slides->addWidget(gL2, 4, 3);

  QLabel *bL1 = new QLabel("B", this, "bL1");
  QLabel *bL2 = new QLabel("B", this, "bL2");
  bSl = new QSlider(0, 1000, 125, 0, QSlider::Horizontal, this, "bSl");
  bSl->setTickmarks(QSlider::Above);
  bLe = new QLineEdit("0.000", this, "bLe");
  bLe->setMaximumWidth(95);
  slides->addWidget(bL1, 5, 0);
  slides->addWidget(bSl, 5, 1);
  slides->addWidget(bLe, 5, 2);
  slides->addWidget(bL2, 5, 3);

  // colour stuff
  QBoxLayout *display = new QVBoxLayout(upper, 1);
  display->setMargin(3);
  colourWheel = new ColourWheel(this, "colourwheel");

  status = new QStatusBar(this, "s");
  status->setSizeGripEnabled(false);
  closebutton = new QPushButton("Close", status, "close");
  display->addWidget(colourWheel, 10);
  status->addWidget(closebutton, 0, true);
  //  display->addWidget(close,1);
  display->addWidget(status, 1);

  // connections
  connectSliders();

  connect(hLe, SIGNAL(returnPressed()), this, SLOT(editHue()));
  connect(sLe, SIGNAL(returnPressed()), this, SLOT(editSat()));
  connect(vLe, SIGNAL(returnPressed()), this, SLOT(editVal()));
  connect(rLe, SIGNAL(returnPressed()), this, SLOT(editRed()));
  connect(gLe, SIGNAL(returnPressed()), this, SLOT(editGreen()));
  connect(bLe, SIGNAL(returnPressed()), this, SLOT(editBlue()));

  connect(colourWheel, SIGNAL(select(Colour *)), this,
          SLOT(setSelect(Colour *)));
  connect(colourWheel, SIGNAL(revert()), this, SLOT(revertColour()));
  connect(colourWheel, SIGNAL(apply()), this, SLOT(apply()));

  connect(closebutton, SIGNAL(clicked()), this, SLOT(close()));

  // other construction stuff
  selectColour = new Colour();
  init = new Colour();

  if (fixsize)
    setFixedSize(10, 10);
  else {
    resize(10, 10);
  }
}

void ColourPick::connectSliders() {
  connect(rSl, SIGNAL(valueChanged(int)), this, SLOT(updateRed(int)));
  connect(gSl, SIGNAL(valueChanged(int)), this, SLOT(updateGreen(int)));
  connect(bSl, SIGNAL(valueChanged(int)), this, SLOT(updateBlue(int)));
  connect(hSl, SIGNAL(valueChanged(int)), this, SLOT(updateHue(int)));
  connect(sSl, SIGNAL(valueChanged(int)), this, SLOT(updateSat(int)));
  connect(vSl, SIGNAL(valueChanged(int)), this, SLOT(updateValue(int)));
}

void ColourPick::disconnectSliders() {
  disconnect(rSl, SIGNAL(valueChanged(int)), this, SLOT(updateRed(int)));
  disconnect(gSl, SIGNAL(valueChanged(int)), this, SLOT(updateGreen(int)));
  disconnect(bSl, SIGNAL(valueChanged(int)), this, SLOT(updateBlue(int)));
  disconnect(hSl, SIGNAL(valueChanged(int)), this, SLOT(updateHue(int)));
  disconnect(sSl, SIGNAL(valueChanged(int)), this, SLOT(updateSat(int)));
  disconnect(vSl, SIGNAL(valueChanged(int)), this, SLOT(updateValue(int)));
}

// -------------------- Slots ----------------------
// this is signalled when a slider, edit box, or the colour wheel is modified
// manually by the user
void ColourPick::notify(bool M) {
  if (M) {
    emit applyEvent(selectColour->cv());
  }
}

// this is signalled by the user parent widget to set initial colour
// if this slot is not used, default init/revert colour is black
void ColourPick::initColour(GLfloat *col) {
  init->setrgb(col);
  selectColour->setto(init);
  colourWheel->initColour(init);
  notify(false);
  updateRGB();
  updateHSV();
}

// this is signalled by the user parent widget to execute the colour pick dialog
void ColourPick::getColour() {
  selectColour->setto(init);
  colourWheel->setSelect(init);
  //  emit applyEvent(selectColour->cv());
  notify(false);
  updateRGB();
  updateHSV();
  //  revertColour();
  //  notify(false);
  show();
}

// this is signalled by the colour wheel when selection is made
void ColourPick::setSelect(Colour *col) {
  selectColour->setto(col);
  notify(true);
  updateRGB();
  updateHSV();
}

// i don't think this is used by anything...
void ColourPick::resetInit() {
  init->setto(selectColour);
  notify(false);
}

// this is signalled when revert button is pushed
void ColourPick::revertColour() {
  selectColour->setto(init);
  colourWheel->setSelect(init);
  emit applyEvent(selectColour->cv());
  notify(false);
  updateRGB();
  updateHSV();
}

// this is signalled when the "Apply" button is clicked
void ColourPick::apply() {
  emit applyEvent(selectColour->cv());
  notify(false);
}

// this is signalled when the dialog is closed
void ColourPick::closeEvent(QCloseEvent *ce) {
  ce->ignore();
  hide();
}

void ColourPick::updateIntensity(int val) {
  selectColour->v((GLfloat)val / (GLfloat)1000);
  colourWheel->setSelect(selectColour);
  //  notify(true);
  updateRGB();
  vSl->setValue(val);
  vLe->setText(QString::number(selectColour->v(), 'f', 3));
}

// SLIDER SIGNALS
// signalled when a value is changed on a slider
// if the user moves the slider, then set select and we must calculate
// otherwise, we are just updating the value because we already did the
// calculation
void ColourPick::updateRed(int component) {
  selectColour->r(GLfloat(component) / (GLfloat)1000);
  colourWheel->setSelect(selectColour);
  updateHSV();
  notify(true);
  rLe->setText(QString::number(selectColour->r(), 'f', 3));
}

void ColourPick::updateGreen(int component) {
  selectColour->g((GLfloat)component / (GLfloat)1000);
  colourWheel->setSelect(selectColour);
  notify(true);
  gLe->setText(QString::number(selectColour->g(), 'f', 3));
}

void ColourPick::updateBlue(int component) {
  selectColour->b((GLfloat)component / (GLfloat)1000);
  updateHSV();
  colourWheel->setSelect(selectColour);
  notify(true);
  bLe->setText(QString::number(selectColour->b(), 'f', 3));
}

void ColourPick::updateHue(int degree) {
  if (degree == UNDEFINED)
    selectColour->h(UNDEFINED);
  else
    selectColour->h((GLfloat)degree / (GLfloat)100);
  colourWheel->setSelect(selectColour);
  notify(true);
  updateRGB();
  if (selectColour->h() == UNDEFINED)
    hLe->setText("UNDEF");
  else
    hLe->setText(QString::number(selectColour->h(), 'f', 3));
}

void ColourPick::updateSat(int sat) {
  selectColour->s((GLfloat)sat / (GLfloat)1000);
  colourWheel->setSelect(selectColour);
  notify(true);
  updateRGB();
  sLe->setText(QString::number(selectColour->s(), 'f', 3));
}

void ColourPick::updateValue(int val) {
  selectColour->v((GLfloat)val / (GLfloat)1000);
  colourWheel->setSelect(selectColour);
  notify(true);
  updateRGB();
  vLe->setText(QString::number(selectColour->v(), 'f', 3));
}

void ColourPick::rgbmode() { slidermode = "rgb"; }

void ColourPick::hsvmode() { slidermode = "hsv"; }

void ColourPick::nomode() {
  slidermode = "no";
  notify(true);
}

// signalled to update the values on the sliders after calculation
void ColourPick::updateRGB() {
  disconnectSliders();
  rSl->setValue((int)(selectColour->r() * 1000));
  gSl->setValue((int)(selectColour->g() * 1000));
  bSl->setValue((int)(selectColour->b() * 1000));
  rLe->setText(QString::number(selectColour->r(), 'f', 3));
  gLe->setText(QString::number(selectColour->g(), 'f', 3));
  bLe->setText(QString::number(selectColour->b(), 'f', 3));
  connectSliders();
}

void ColourPick::updateHSV() {
  disconnectSliders();
  if (selectColour->h() == UNDEFINED)
    hSl->setValue(UNDEFINED);
  else
    hSl->setValue((int)(selectColour->h() * 100));
  sSl->setValue((int)(selectColour->s() * 1000));
  vSl->setValue((int)(selectColour->v() * 1000));
  sLe->setText(QString::number(selectColour->s(), 'f', 3));
  vLe->setText(QString::number(selectColour->v(), 'f', 3));
  if (selectColour->h() == UNDEFINED)
    hLe->setText("UNDEF");
  else
    hLe->setText(QString::number(selectColour->h(), 'f', 3));
  connectSliders();
}

// signalled when user presses return in the line edit boxes
// parses input - must be floating point number
// hue : >=0 or "undef"
// sat,val : e[0,1]
// r,g,b : e[0,1]
void ColourPick::editHue() {
  bool ok = false;
  float h;
  if ((hLe->text() == "UNDEF") || (hLe->text() == "undef"))
    h = UNDEFINED;
  else {
    h = hLe->text().toFloat(&ok);
    if (ok) {
      if (h < 0)
        h = UNDEFINED;
      while (h > 360)
        h -= 360.00;
    } else {
      if (selectColour->h() == UNDEFINED)
        hLe->setText("UNDEF");
      else
        hLe->setText(QString::number(selectColour->h(), 'f', 3));
      status->message("H >= 0 or \"undef\"", 2500);
      return;
    }
  }

  if (h == UNDEFINED) {
    selectColour->h(UNDEFINED);
    hSl->setValue(UNDEFINED);
  } else {
    selectColour->h(h);
    hSl->setValue((int)(h * 100));
  }
  colourWheel->setSelect(selectColour);
  notify(true);
  updateRGB();
}

void ColourPick::editSat() {
  bool ok = false;
  float s;
  s = sLe->text().toFloat(&ok);
  if ((ok) && ((s < 0) || (s > 1)))
    ok = false;
  if (!ok) {
    status->message("0 <= S <= 1", 2500);
    sLe->setText(QString::number(selectColour->s(), 'f', 3));
    return;
  }
  selectColour->s(s);
  sSl->setValue((int)(s * 1000));
  colourWheel->setSelect(selectColour);
  notify(true);
  updateRGB();
}

void ColourPick::editVal() {
  bool ok = false;
  float v;
  v = vLe->text().toFloat(&ok);
  if ((ok) && ((v < 0) || (v > 1)))
    ok = false;
  if (!ok) {
    status->message("0 <= V <= 1", 2500);
    vLe->setText(QString::number(selectColour->v(), 'f', 3));
    return;
  }
  selectColour->v(v);
  vSl->setValue((int)(v * 1000));
  colourWheel->setSelect(selectColour);
  notify(true);
  updateRGB();
}

void ColourPick::editRed() {
  bool ok = false;
  float r;
  r = rLe->text().toFloat(&ok);
  if ((ok) && ((r < 0) || (r > 1)))
    ok = false;
  if (!ok) {
    status->message("0 <= R <= 1", 2500);
    rLe->setText(QString::number(selectColour->r(), 'f', 3));
    return;
  }
  selectColour->r(r);
  rSl->setValue((int)(r * 1000));
  colourWheel->setSelect(selectColour);
  notify(true);
  updateRGB();
}

void ColourPick::editGreen() {
  bool ok = false;
  float g;
  g = gLe->text().toFloat(&ok);
  if ((ok) && ((g < 0) || (g > 1)))
    ok = false;
  if (!ok) {
    status->message("0 <= G <= 1", 2500);
    gLe->setText(QString::number(selectColour->g(), 'f', 3));
    return;
  }
  selectColour->g(g);
  gSl->setValue((int)(g * 1000));
  colourWheel->setSelect(selectColour);
  notify(true);
  updateRGB();
}

void ColourPick::editBlue() {
  bool ok = false;
  float b;
  b = bLe->text().toFloat(&ok);
  if ((ok) && ((b < 0) || (b > 1)))
    ok = false;
  if (!ok) {
    status->message("0 <= B <= 1", 2500);
    bLe->setText(QString::number(selectColour->b(), 'f', 3));
    return;
  }
  selectColour->b(b);
  bSl->setValue((int)(b * 1000));
  colourWheel->setSelect(selectColour);
  notify(true);
  updateRGB();
}

// ==================== Class: ColourWheel =========================
// -------------------- Construction/Destruction ------------------------
ColourWheel::ColourWheel(QWidget *parent, const char *name)
    : QGLWidget(parent, name) {
  selectObject = 0;
  selectColour = new Colour();
  revertColour = new Colour();

  setCursor(crossCursor);

  setMinimumHeight(180);
  setMinimumWidth(180);

  range = 100;

  // to mimimize thought...
  GLfloat dim = range * 2.0;
  dim--;
  GLfloat margin = 4;
  R = (((dim - (3 * margin)) / 4) / 2) * 3;
  W = ((dim - (3 * margin)) / 4) * 1;
  sR = R / 20.0;
  sample.x = -range + margin;
  sample.y = -range + margin;
  value.x = range - (margin + W);
  value.y = -range + (margin + W + margin);
  revertBox.x = range - (margin + W);
  revertBox.y = -range + margin;
  wheel.x = -range + (margin + R);
  wheel.y = range - (margin + R);

  setCircumferencePoints(); // called only once for efficiency!!!
}

// -------------------- Slots
// the colour wheel has its own select colour, and copies into it
// when this colour is edited, colour pick is updated
void ColourWheel::setSelect(Colour *col) {
  selectColour->setto(col);
  S = selectColour->s() * R;        // the saturation radius
  dV = selectColour->v() * (R + R); // the value level
  if ((selectColour->s() > 0) && (selectColour->h() != UNDEFINED)) {
    HSpoint.x = wheel.x + S * cos((PI * selectColour->h()) / 180.00);
    HSpoint.y = wheel.y + S * sin((PI * selectColour->h()) / 180.00);
  } else {
    HSpoint.x = wheel.x;
    HSpoint.y = wheel.y;
  }
  updateGL();
}

// initialize colours
void ColourWheel::initColour(Colour *col) {
  revertColour->setto(col);
  setSelect(col);
}

// -------------------- Rendering
void ColourWheel::initializeGL() {
  QColor clear = parentWidget()->backgroundColor();
  glClearColor((GLfloat)clear.red() / (GLfloat)255,
               (GLfloat)clear.green() / (GLfloat)255,
               (GLfloat)clear.blue() / (GLfloat)255, 0);
  glShadeModel(GL_SMOOTH);
}

void ColourWheel::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT);
  drawHSWheel();
  drawVScale();
  drawSample();
  showHSV();
}

void ColourWheel::resizeGL(int, int) {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  GLfloat scale;

  // center at center
  if (width() < height()) {
    scale = (GLfloat)height() / (GLfloat)width();
    left = -range;
    right = range;
    bottom = -range * scale;
    top = range * scale;
  } else {
    scale = (GLfloat)width() / (GLfloat)height();
    left = -range * scale;
    right = range * scale;
    bottom = -range;
    top = range;
  }

  gluOrtho2D(left, right, bottom, top);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glViewport(0, 0, (GLint)width(), (GLint)height());
}

void ColourWheel::showHSV() {
  Colour *vc1 = new Colour();
  Colour *vc2 = new Colour();
  Colour *vc3 = new Colour();
  GLfloat h;

  if (selectColour->h() < 180.00)
    h = 180.00 + selectColour->h();
  else
    h = selectColour->h() - 180.00;
  vc1->sethsv(h, 1.00, 0.75); // a bright complimentary colour
  vc2->sethsv(h, 1.00, 0.25); // a dark complimentary colour
  vc3->sethsv(selectColour->h(), selectColour->s(),
              1.00); // select colour at full value

  GLfloat satscale = S / R;

  // saturation circle
  glBegin(GL_LINE_LOOP);
  {
    for (int theta = 0; theta <= 360; theta++) {
      glColor3fv(Scircumference[theta].colour->cv()); // rainbow ring
      glVertex2f(wheel.x + (Wcircumference[theta].x * satscale),
                 wheel.y + (Wcircumference[theta].y * satscale));
    }
  }
  glEnd();

  // hue line
  if (selectColour->h() != UNDEFINED) {
    glBegin(GL_LINES);
    {
      glColor3fv(vc2->cv());
      glVertex2f(wheel.x, wheel.y);
      glColor3fv(vc1->cv());
      glVertex2f(wheel.x + R * cos((PI * selectColour->h()) / 180.00),
                 wheel.y + R * sin((PI * selectColour->h()) / 180.00));
    }
    glEnd();
  }

  // selected colour on wheel
  glColor3fv(vc3->cv());
  glBegin(GL_TRIANGLE_FAN);
  {
    glVertex2f(HSpoint.x, HSpoint.y);
    for (int theta = 0; theta <= 72; theta++)
      glVertex2f(HSpoint.x + HSPcircumference[theta].x,
                 HSpoint.y + HSPcircumference[theta].y);
  }
  glEnd();

  // ring around select
  if (selectColour->h() == UNDEFINED)
    glColor3f(0, 0, 0);
  else
    glColor3fv(vc1->cv());
  glBegin(GL_LINE_LOOP);
  {
    for (int theta = 0; theta <= 72; theta++)
      glVertex2f(HSpoint.x + HSPcircumference[theta].x,
                 HSpoint.y + HSPcircumference[theta].y);
  }
  glEnd();

  // value level
  if ((selectColour->h() == UNDEFINED) || (eq(selectColour->s(), 0)))
    glColor3f(1.0, 0.0, 0.0); // red slider for greyscale
  else
    glColor3fv(vc1->cv());
  GLfloat l = sR / 2.2;
  glBegin(GL_LINES);
  {
    glVertex2f(value.x, value.y + dV + l);
    glVertex2f(value.x + W, value.y + dV + l);

    glVertex2f(value.x + W, value.y + dV + l);
    glVertex2f(value.x + W, value.y + dV - l);

    glVertex2f(value.x, value.y + dV - l);
    glVertex2f(value.x + W, value.y + dV - l);

    glVertex2f(value.x, value.y + dV + l);
    glVertex2f(value.x, value.y + dV - l);
  }
  glEnd();
  glFlush();
}

void ColourWheel::drawHSWheel() {
  // colour wheel
  glBegin(GL_TRIANGLE_FAN);
  {
    glColor3f(1.0, 1.0, 1.0);
    glVertex2f(wheel.x, wheel.y);
    for (int theta = 0; theta <= 360; theta++) {
      glColor3fv(Wcircumference[theta].colour->cv());
      glVertex2f(wheel.x + Wcircumference[theta].x,
                 wheel.y + Wcircumference[theta].y);
    }
  }
  glEnd();
  glFlush();
}

void ColourWheel::drawVScale() {
  Colour *top = new Colour();
  top->sethsv(selectColour->h(), selectColour->s(), 1);

  glBegin(GL_POLYGON);
  {
    glColor3f(0.0, 0.0, 0.0);
    glVertex2f(value.x, value.y);
    glVertex2f(value.x + W, value.y);

    glColor3fv(top->cv());
    glVertex2f(value.x + W, value.y + R + R);
    glVertex2f(value.x, value.y + R + R);
  }
  glEnd();

  glColor3f(0.0, 0.0, 0.0);
  glBegin(GL_LINE_LOOP);
  {
    glVertex2f(value.x, value.y);
    glVertex2f(value.x, value.y + R + R);
    glVertex2f(value.x + W, value.y + R + R);
    glVertex2f(value.x + W, value.y);
  }
  glEnd();
  glFlush();
}

void ColourWheel::drawSample() {
  glColor3fv(selectColour->cv());
  glBegin(GL_POLYGON);
  {
    glVertex2f(sample.x, sample.y);
    glVertex2f(sample.x, sample.y + W);
    glVertex2f(sample.x + R + R, sample.y + W);
    glVertex2f(sample.x + R + R, sample.y);
  }
  glEnd();

  glColor3f(0, 0, 0);
  glBegin(GL_LINE_LOOP);
  {
    glVertex2f(sample.x, sample.y);
    glVertex2f(sample.x, sample.y + W);
    glVertex2f(sample.x + R + R, sample.y + W);
    glVertex2f(sample.x + R + R, sample.y);
  }
  glEnd();

  glColor3fv(revertColour->cv());
  glBegin(GL_POLYGON);
  {
    glVertex2f(revertBox.x, revertBox.y);
    glVertex2f(revertBox.x, revertBox.y + W);
    glVertex2f(revertBox.x + W, revertBox.y + W);
    glVertex2f(revertBox.x + W, revertBox.y);
  }
  glEnd();

  glColor3f(0, 0, 0);
  glBegin(GL_LINE_LOOP);
  {
    glVertex2f(revertBox.x, revertBox.y);
    glVertex2f(revertBox.x, revertBox.y + W);
    glVertex2f(revertBox.x + W, revertBox.y + W);
    glVertex2f(revertBox.x + W, revertBox.y);
  }
  glEnd();

  glFlush();
}

// --------------------- Mouse Event Handling ----------------------------
void ColourWheel::mousePressEvent(QMouseEvent *me) {
  moved = false;
  point.x = (left + ((me->x() / (GLfloat)width()) * (right * 2.0)));
  point.y = (top - ((me->y() / (GLfloat)height()) * (top * 2.0)));

  selectionTest();
  switch (selectObject) {
    // HS Wheel
  case 1:
    HS();
    emit select(selectColour);
    setCursor(blankCursor);
    updateGL();
    break;
    // V Scale
  case 2:
    V();
    emit select(selectColour);
    setCursor(blankCursor);
    updateGL();
    break;
  default:
    break;
  }
}

void ColourWheel::mouseMoveEvent(QMouseEvent *me) {
  moved = true;
  point.x = (left + ((me->x() / (GLfloat)width()) * (right * 2.0)));
  point.y = (top - ((me->y() / (GLfloat)height()) * (top * 2.0)));
  switch (selectObject) {
    // HS Wheel
  case 1:
    HS();
    emit select(selectColour);
    updateGL();
    break;
    // V Scale
  case 2:
    V();
    emit select(selectColour);
    updateGL();
    break;
  default:
    break;
  }
}

void ColourWheel::mouseReleaseEvent(QMouseEvent *me) {
  point.x = (left + ((me->x() / (GLfloat)width()) * (right * 2.0)));
  point.y = (top - ((me->y() / (GLfloat)height()) * (top * 2.0)));
  switch (selectObject) {
    // HS Wheel
  case 1:
    if (moved) {
      HS();
      emit select(selectColour);
      updateGL();
    }
    setCursor(crossCursor);
    break;
    // V Scale
  case 2:
    if (moved) {
      V();
      emit select(selectColour);
      updateGL();
    }
    setCursor(crossCursor);
    break;
    // sample box
  case 3:
    emit apply();
    updateGL();
    break;
    // revert box
  case 4:
    setSelect(revertColour);
    emit revert();
    updateGL();
    break;
  }
}

// home-made selection test
void ColourWheel::selectionTest() {
  if (sqrt((point.x - wheel.x) * (point.x - wheel.x) +
           ((point.y - wheel.y) * (point.y - wheel.y))) < (R + sR))
    selectObject = 1;
  else if ((point.x >= value.x) && (point.y >= value.y) &&
           (point.x <= (value.x + W)) && (point.y <= (value.y + R + R)))
    selectObject = 2;
  else if ((point.x >= sample.x) && (point.y >= sample.y) &&
           (point.x <= (sample.x + R + R)) && ((point.y <= sample.y + W)))
    selectObject = 3;
  else if ((point.x >= revertBox.x) && (point.y >= revertBox.y) &&
           (point.x <= (revertBox.x + R + R)) && ((point.y <= revertBox.y + W)))
    selectObject = 4;
  else
    selectObject = 0;
}

// --------------------- Fancy Calculation Functions ---------------------------
// slow...lookup tables will quicken this up...is it worth it???
void ColourWheel::HS() {
  GLfloat hyp, adj, costheta, dx, dy, h, s;

  if (eq(point.x, wheel.x) && eq(point.y, wheel.y)) {
    h = UNDEFINED;
    s = 0.00;
  } else {
    dx = (point.x - wheel.x);
    dy = (point.y - wheel.y);
    hyp = sqrt((dx * dx) + (dy * dy));
    adj = sqrt(dx * dx);

    costheta = adj / hyp;
    if (dx < 0)
      costheta = -costheta;
    h = (acos(costheta) * 180.00) / PI;
    if (dy < 0)
      h = (360.00 - h);
    s = hyp / R;
    if (s > 1)
      s = 1.00;
  }
  selectColour->sethsv(h, s, selectColour->v());

  S = selectColour->s() * R;
  if ((selectColour->s() > 0) && (selectColour->h() != UNDEFINED)) {
    HSpoint.x = wheel.x + S * cos((PI * selectColour->h()) / 180.00);
    HSpoint.y = wheel.y + S * sin((PI * selectColour->h()) / 180.00);
  } else {
    HSpoint.x = wheel.x;
    HSpoint.y = wheel.y;
  }
}

void ColourWheel::V() {
  selectColour->v((point.y - value.y) / (R + R));
  if (selectColour->v() > 1)
    selectColour->v(1.00);
  else if (selectColour->v() < 0)
    selectColour->v(0.00);
  dV = selectColour->v() * (R + R);
}

// brute force assignment of 360 circle points
void ColourWheel::setCircumferencePoints() {
  GLfloat h;
  for (int theta = 0; theta <= 360; theta++) {
    Wcircumference[theta].colour = new Colour();
    Wcircumference[theta].colour->sethsv((GLfloat)theta, 1.00, 1.00);
    Wcircumference[theta].x = R * cos((PI * (GLfloat)theta) / 180.00);
    Wcircumference[theta].y = R * sin((PI * (GLfloat)theta) / 180.00);

    if (theta < 180.00)
      h = 180.00 + theta;
    else
      h = theta - 180.00;
    Scircumference[theta].colour = new Colour();
    Scircumference[theta].colour->sethsv(h, 1.00, 0.75);
  }
  for (int i = 0; i <= 72; i++) {
    HSPcircumference[i].x = sR * cos((PI * (GLfloat)(i * 5)) / 180.00);
    HSPcircumference[i].y = sR * sin((PI * (GLfloat)(i * 5)) / 180.00);
  }
}

// EOF: colourpick.cc
