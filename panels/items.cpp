/* ******************************************************************** *
   Copyright (C) 1990-2022 University of Calgary
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ******************************************************************** */



#include "items.h"
#include "panel.h"
#include "paneledit.h"
#include <iostream>
#include <cmath>

void Item::processMessage() {

  if (!message.isEmpty()) {
    fprintf(stdout, message.toStdString().c_str(), value);
    fprintf(stdout, "\n");
    fflush(stdout);
  }
}

void Item::snap() {
  if (!itemMoved)
    return;
  int a = (int(((GLfloat(contour.left())) / GLfloat(panel->getGridSize()))) *
           panel->getGridSize()) -
          contour.left();
  int b = (int(((GLfloat(contour.bottom())) / GLfloat(panel->getGridSize()))) *
           panel->getGridSize()) -
          contour.bottom();
  moveBy(a, b);
  panel->getGLWidget()->update();
}

void Item::setSize(int w, int h) {
  width = w;
  height = h;
}

void Item::setValue(int v) { value = v; }

void Item::setMode(QString M) { mode = M; }

void Item::setColour1(GLfloat *c1) {
  for (int i = 0; i < 3; i++) {
    colour1[i] = c1[i];
  }
  panel->getGLWidget()->update();
}

void Item::setColour2(GLfloat *c2) {
  for (int i = 0; i < 3; i++) {
    colour2[i] = c2[i];
  }
  panel->getGLWidget()->update();
}

void Item::setSelectColour(GLfloat *c) {
  for (int i = 0; i < 3; i++) {
    selcol[i] = c[i];
  }
  panel->getGLWidget()->update();
}


// ------------------------------------------------ Page
// ---------------------------------------
Page::Page(QFont f, Panel *pan, QAction *id, QString nm, GLfloat *c, int x,
           int y, QString mess) {
  name = nm;
  value = 0;
  panel = pan;
  font = f;
  font.setHintingPreference(QFont::PreferNoHinting);
#ifndef __APPLE__
  font.setPointSizeF(font.pointSizeF() * 72. / panel->logicalDpiX());
#endif
  menu_id = id;
  mode = "EXEC";
  margin = 3;
  message = mess;
  selcol[0] = 1.0;
  selcol[1] = 0.85;
  selcol[2] = 0.0;
  if (x == -1)
    x = panel->width() / 2;
  if (y == -1)
    y = 5;

  if (c == NULL){
    c = new GLfloat[3];
    c[0] = 1.0;
    c[1] = 1.0;
    c[2] = 1.0;
  }
  colour1[0] = c[0];
  colour1[1] = c[1];
  colour1[2] = c[2];
  itemWithActiveDialogue = NULL;
  items.push_back(new Label(this, panel, name, c, x, y));
  type = "PAGE";
  _isDragSelecting = false;
    
}

Page::Page(Page *I) {
  name = I->getName();
  GLfloat *col = I->getSelColour();
  setSelectColour(col);
  GLfloat* c = I->getColour1();
  if (c == NULL){
    c = new GLfloat[3];
    c[0] = 1.0;
    c[1] = 1.0;
    c[2] = 1.0;
  }
  colour1[0] = c[0];
  colour1[1] = c[1];
  colour1[2] = c[2];

}

void Page::resetToDefaultValue() {
  for (unsigned int i = 0; i < items.size(); i++) {
    items[i]->resetToDefaultValue();
  }
  panel->getGLWidget()->update();
}

void Page::saveDefaultValue() {
  for (unsigned int i = 0; i < items.size(); i++) {
    items[i]->saveDefaultValue();
  }
  panel->getGLWidget()->update();
}

void Page::setSelectColour(GLfloat *c) {
  for (int i = 0; i < 3; i++) {
    selcol[i] = c[i];
  }
  for (unsigned int i = 0; i < items.size(); i++) {
    items[i]->setSelectColour(selcol);
  }
}

void Page::getSelectColour(GLfloat *col) {
  for (int i = 0; i < 3; i++) {
    col[i] = selcol[i];
  }
}
void Page::setColour1(GLfloat *c1){
  if (items[0] != NULL) {
    items[0]->setColour1(c1);
  }

  for (int i = 0; i < 3; i++) {
    colour1[i] = c1[i];
  }
}


void Page::setName(QString nm) {
  name = nm;
  items[0]->setName(nm);
}

void Page::activate() {
  if (mode == "EXEC")
    processMessage();
}

void Page::setFont(QFont f) {
  font = f;
  font.setHintingPreference(QFont::PreferNoHinting);
#ifndef __APPLE__
  font.setPointSizeF(font.pointSizeF() * 72. / panel->logicalDpiX());
#endif
  for (unsigned int i = 0; i < items.size(); i++) {
    if ((items[i]->getType() == "LABEL") || (items[i]->getType() == "SLIDER") ||
        (items[i]->getType() == "BUTTON")) {
      items[i]->aligntext();
    }
  }
}

void Page::addItem(Item *I) { items.push_back(I); }

void Page::deleteItem(Item *I) {


  for (unsigned int i = 0; i < selectedItems.size(); i++) {
    if (selectedItems[i] == I) {
      selectedItems.erase(selectedItems.begin() + i);
    }
  }
  for (unsigned int i = 0; i < items.size(); i++) {
    if (items[i] == I) {
      items.erase(items.begin() + i);
    }
  }
  // delete I;
}

void Page::setMode(QString M) {
  mode = M;
  for (unsigned int i = 0; i < items.size(); i++) {
    items[i]->setMode(M);
  }
}

void Page::draw(GLWidget *w) {
  w->makeCurrent();
  glLineWidth(1);
  if (!items[0]->getName().isEmpty()) {
    items[0]->draw(w);
  }

  for (unsigned int i = 1; i < items.size(); i++) {
    items[i]->draw(w);
  }

  if (mode == "EDIT") {
    if (_isDragSelecting) {
      drawDragSelectionBox();
    }

    
    if (!selectedItems.empty()) {
      drawSelectedItemsBox();
    }

    if (itemWithActiveDialogue != NULL && panel->thereAreOpenDialogs()) {
      drawItemWithActiveDialogBox();
    }

  }

  //glFlush();
}

void Page::mousePress(QPoint P) {
  if (_selectedItemsRect.contains(P)) {
    for (unsigned int i = 0; i < selectedItems.size(); i++) {
      selectedItems[i]->mousePress(P);
    }
    return;
  } else {
    clearSelectedItems();
  }

  for (unsigned int i = 0; i < items.size(); i++) {
    if (items[i]->getContour()->contains(P)) {
      if (selectedItems.empty()) {
        addItemToSelectedItems(items[i]);
        selectedItems.back()->mousePress(P);
	setItemWithActiveDialogue(selectedItems.back());
      }
    } else {
      items[i]->setSelected(false);
    }
  }

  if (selectedItems.empty() && mode == "EDIT") {
    _isDragSelecting = true;
    _dragSelectOrigin = _dragSelectDestination = P;
  }
}

void Page::shiftMousePress(QPoint P) {
  for (unsigned int i = 0; i < items.size(); i++) {
    if (items[i]->getContour()->contains(P)) {
      if (std::find(selectedItems.begin(), selectedItems.end(), items[i]) !=
          selectedItems.end()) {
        continue;
      } else {
        addItemToSelectedItems(items[i]);
        for (unsigned int i = 0; i < selectedItems.size(); i++) {
          selectedItems[i]->mousePress(P);
        }
        break;
      }
    }
  }
}

void Page::mouseDoublePress(QPoint P) {
  clearSelectedItems();

  for (unsigned int i = 0; i < items.size(); i++) {
    if (items[i]->getContour()->contains(P)) {
      if (selectedItems.empty()) {
        addItemToSelectedItems(items[i]);
        selectedItems.back()->mousePress(P);
        setItemWithActiveDialogue(selectedItems.back());
      }
    } else {
      items[i]->setSelected(false);
    }
  }
}

void Page::mouseMove(QPoint P) {
  if (!selectedItems.empty()) {
    for (unsigned int i = 0; i < selectedItems.size(); i++) {
      selectedItems[i]->mouseMove(P);
    }
  }
  if (_isDragSelecting) {
    _dragSelectDestination = P;
  }
}

void Page::mouseRelease(QPoint P) {
  if (!selectedItems.empty()) {
    for (unsigned int i = 0; i < selectedItems.size(); i++) {
      selectedItems[i]->mouseRelease(P);
    }
  }

  if (_isDragSelecting) {
    _dragSelectDestination = P;
    _isDragSelecting = false;
    setSelectedItemsFromArea(QRect(_dragSelectOrigin, _dragSelectDestination));
  }
}

void Page::setSelectedItemsFromArea(const QRect area) {
  clearSelectedItems();

  for (unsigned int i = 0; i < items.size(); i++) {
    if (area.contains(*items[i]->getContour())) {
      addItemToSelectedItems(items[i]);
    }
  }
}

void Page::clearSelectedItems() {
  for (unsigned int i = 0; i < selectedItems.size(); i++) {
    selectedItems[i]->setSelected(false);
  }
  selectedItems.clear();

  updateAlignmentButtonsEnabled();
  _selectedItemsRect = QRect(QPoint(0.f, 0.f), QPoint(0.f, 0.f));
}

void Page::addItemToSelectedItems(Item *I) {
  selectedItems.push_back(I);
  selectedItems.back()->setSelected(true);

  updateAlignmentButtonsEnabled();
  updateSelectedItemsRect();
}

void Page::removeItemFromSelectedItems(Item *I) {
  for (unsigned int i = 0; i < selectedItems.size(); i++) {
    if (selectedItems[i] == I) {
      selectedItems.erase(selectedItems.begin() + i);
      I->setSelected(false);
    }
  }

  updateAlignmentButtonsEnabled();
  updateSelectedItemsRect();
  panel->getGLWidget()->update();
}

void Page::updateAlignmentButtonsEnabled() {

  panel->getPanelEditor()->setAlignmentButttonsEnabled(selectedItems.size() >=
                                                       2);
  panel->getPanelEditor()->setDistributionButtonsEnabled(selectedItems.size() >=
                                                         3);

  int buttonSelectionCount = 0;
  for (unsigned int i = 0; i < selectedItems.size(); i++) {
    if (selectedItems[i]->getType() == "BUTTON") {
      buttonSelectionCount++;
    }
  }

  panel->getPanelEditor()->setAddButtonGroupEnabled(buttonSelectionCount >= 2);
}

void Page::drawDragSelectionBox() {
  int h = getPanel()->getPanelHeight();

  glEnable(GL_LINE_STIPPLE);
  glLineStipple(1, 0x00FF); /*  dotted  */
  glColor3fv(selcol);

  glColor3fv(selcol);
  glBegin(GL_LINE_LOOP);
  {
    glVertex2i(_dragSelectOrigin.x(), h - _dragSelectOrigin.y());
    glVertex2i(_dragSelectOrigin.x(), h - _dragSelectDestination.y());
    glVertex2i(_dragSelectDestination.x(), h - _dragSelectDestination.y());
    glVertex2i(_dragSelectDestination.x(), h - _dragSelectOrigin.y());
  }
  glEnd();
  glDisable(GL_LINE_STIPPLE);
}

void Page::drawSelectedItemsBox() {
  int h = getPanel()->getPanelHeight();

  updateSelectedItemsRect();

  glEnable(GL_LINE_STIPPLE);
  glLineStipple(1, 0x00FF); /*  dotted  */
  glColor3fv(selcol);

  glColor3fv(selcol);
  glBegin(GL_LINE_LOOP);
  {
    glVertex2i(_selectedItemsRect.left() - 4, h - _selectedItemsRect.top() + 4);
    glVertex2i(_selectedItemsRect.left() - 4,
               h - _selectedItemsRect.bottom() - 4);
    glVertex2i(_selectedItemsRect.right() + 4,
               h - _selectedItemsRect.bottom() - 4);
    glVertex2i(_selectedItemsRect.right() + 4,
               h - _selectedItemsRect.top() + 4);
  }
  glEnd();
  glDisable(GL_LINE_STIPPLE);
}

void Page::updateSelectedItemsRect() {
  float minX, minY, maxX, maxY;

  if (selectedItems.size() > 0) {
    minX = selectedItems[0]->getContour()->left();
    maxX = selectedItems[0]->getContour()->right();
    maxY = selectedItems[0]->getContour()->bottom();
    minY = selectedItems[0]->getContour()->top();
  }

  for (unsigned i = 1; i < selectedItems.size(); i++) {
    minX = fmin(minX, selectedItems[i]->getContour()->left());
    maxX = fmax(maxX, selectedItems[i]->getContour()->right());
    maxY = fmax(maxY, selectedItems[i]->getContour()->bottom());
    minY = fmin(minY, selectedItems[i]->getContour()->top());
  }

  _selectedItemsRect = QRect(QPoint(minX, minY), QPoint(maxX, maxY));
}

void Page::drawItemWithActiveDialogBox() {
  int h = getPanel()->getPanelHeight();
  QRect *itemContour = itemWithActiveDialogue->getContour();
  if (itemContour != NULL) {
    glColor3fv(selcol);
    glBegin(GL_LINE_LOOP);
    {
      glVertex2i(itemContour->left(), h - itemContour->top());
      glVertex2i(itemContour->left(), h - itemContour->bottom());
      glVertex2i(itemContour->right(), h - itemContour->bottom());
      glVertex2i(itemContour->right(), h - itemContour->top());
    }
    glEnd();
  }
}

void Page::alignSelected(int alignment, QString orientation) {
  if (selectedItems.size() >= 2) {
    if (orientation == "Horizontal") {
      switch (alignment) {
      case 0: // left
        horizontalAlignLeft();
        break;
      case 1: // center
        horizontalAlignCenter();
        break;
      case 2: // right
        horizontalAlignRight();
        break;
      }
    } else if (orientation == "Vertical") {
      switch (alignment) {
      case 0: // bottom
        verticalAlignBottom();
        break;
      case 1: // center
        verticalAlignCenter();
        break;
      case 2: // top
        verticalAlignTop();
        break;
      }
    }
  }
}

void Page::horizontalAlignLeft() {
  removeGroupedButtonsFromSelected();

  int selectedItemsContourLeft = _selectedItemsRect.left();

  for (unsigned int i = 0; i < selectedItems.size(); i++) {
    int itemContourLeft = selectedItems[i]->getContour()->left();
    int dx = selectedItemsContourLeft - itemContourLeft;

    selectedItems[i]->moveBy(dx, 0);
  }
  panel->getGLWidget()->update();
}

void Page::horizontalAlignCenter() {
  removeGroupedButtonsFromSelected();

  removeGroupedButtonsFromSelected();

  int selectedItemsContourCenter = _selectedItemsRect.center().x();

  for (unsigned int i = 0; i < selectedItems.size(); i++) {
    int itemContourCenter = selectedItems[i]->getContour()->center().x();
    int dx = selectedItemsContourCenter - itemContourCenter;

    selectedItems[i]->moveBy(dx, 0);
  }
  panel->getGLWidget()->update();
}

void Page::horizontalAlignRight() {
  removeGroupedButtonsFromSelected();

  int selectedItemsContourRight = _selectedItemsRect.right();

  for (unsigned int i = 0; i < selectedItems.size(); i++) {
    int itemContourRight = selectedItems[i]->getContour()->right();
    int dx = selectedItemsContourRight - itemContourRight;

    selectedItems[i]->moveBy(dx, 0);
  }
  panel->getGLWidget()->update();
}

void Page::distributeHorizontally() {
  removeGroupedButtonsFromSelected();

  int selectedItemsContourWidth = _selectedItemsRect.width();

  float selectedItemsTotalWidth = 0;
  for (unsigned int i = 0; i < selectedItems.size(); i++) {
    selectedItemsTotalWidth += selectedItems[i]->getContour()->width();
  }

  // no spacing at the end
  float numSpacesBetweenItems = selectedItems.size() - 1;
  float spacingBetweenItems =
      (selectedItemsContourWidth - selectedItemsTotalWidth) /
      numSpacesBetweenItems;

  if (spacingBetweenItems > 10000)
    return;

  int currX = _selectedItemsRect.left();

  // sort items to distribute by horizontal position
  std::sort(selectedItems.begin(), selectedItems.end(), [](Item *I1, Item *I2) {
    return I1->getContour()->left() < I2->getContour()->left();
  });

  for (unsigned int i = 0; i < selectedItems.size(); i++) {
    int moveByX = currX - selectedItems[i]->getContour()->left();
    selectedItems[i]->moveBy(moveByX, 0);
    currX += selectedItems[i]->getContour()->width() + (int)spacingBetweenItems;
  }
  panel->getGLWidget()->update();
}

void Page::verticalAlignBottom() {
  removeGroupedButtonsFromSelected();

  int selectedItemsContourBottom = _selectedItemsRect.bottom();

  for (unsigned int i = 0; i < selectedItems.size(); i++) {
    int itemContourBottom = selectedItems[i]->getContour()->bottom();
    int dy = selectedItemsContourBottom - itemContourBottom;

    selectedItems[i]->moveBy(0, dy);
  }
  panel->getGLWidget()->update();
}

void Page::verticalAlignCenter() {
  removeGroupedButtonsFromSelected();

  int selectedItemsContourCenter = _selectedItemsRect.center().y();

  for (unsigned int i = 0; i < selectedItems.size(); i++) {
    int itemContourCenter = selectedItems[i]->getContour()->center().y();
    int dy = selectedItemsContourCenter - itemContourCenter;

    selectedItems[i]->moveBy(0, dy);
  }
  panel->getGLWidget()->update();
}

void Page::verticalAlignTop() {
  removeGroupedButtonsFromSelected();

  int selectedItemsContourTop = _selectedItemsRect.top();

  for (unsigned int i = 0; i < selectedItems.size(); i++) {
    int itemContourTop = selectedItems[i]->getContour()->top();
    int dy = selectedItemsContourTop - itemContourTop;

    selectedItems[i]->moveBy(0, dy);
  }
  panel->getGLWidget()->update();
}

void Page::distributeVertically() {
  removeGroupedButtonsFromSelected();

  float selectedItemsContourHeight = _selectedItemsRect.height();

  float selectedItemsTotalHeight = 0;
  for (unsigned int i = 0; i < selectedItems.size(); i++) {
    selectedItemsTotalHeight += selectedItems[i]->getContour()->height();
  }

  // no spacing at the end
  float numSpacesBetweenItems = selectedItems.size() - 1;
  float spacingBetweenItems =
      (selectedItemsContourHeight - selectedItemsTotalHeight) /
      numSpacesBetweenItems;

  int currY = _selectedItemsRect.bottom();

  // sort items to distribute by vertical position
  std::sort(selectedItems.begin(), selectedItems.end(), [](Item *I1, Item *I2) {
    return I1->getContour()->bottom() > I2->getContour()->bottom();
  });

  for (unsigned int i = 0; i < selectedItems.size(); i++) {
    int moveByY = currY - selectedItems[i]->getContour()->bottom();
    selectedItems[i]->moveBy(0, moveByY);
    currY -=
        selectedItems[i]->getContour()->height() + (int)spacingBetweenItems;
  }
  panel->getGLWidget()->update();
}

void Page::flipItemsVertically() {
  for (uint i = 0; i < items.size(); i++) {
    Item *item = items[i];
    if (item->getType() == "BUTTON" || item->getType() == "SLIDER" ||
        item->getType() == "LABEL") {
      int currentHeight = item->getContour()->center().y();
      int targetHeight = panel->getPanelHeight() - currentHeight;
      item->moveBy(0, targetHeight - currentHeight);
      ;
    }
  }
}

void Page::moveSelectedItemsRight(int dx) {
  for (unsigned int i = 0; i < selectedItems.size(); i++) {
    selectedItems[i]->moveBy(dx, 0);
  }
}

void Page::moveSelectedItemsUp(int dy) {
  for (unsigned int i = 0; i < selectedItems.size(); i++) {
    selectedItems[i]->moveBy(0, -dy);
  }
}

void Page::removeGroupedButtonsFromSelected() {
  std::vector<Item *> newSelectedItems;
  for (unsigned int i = 0; i < selectedItems.size(); i++) {
    Item *item = selectedItems[i];
    if (item->getType() == "BUTTON") {
      Button *button = (Button *)item;
      if (button->getGroup() != NULL) {
        if (button->getGroup()->isSelected()) {
          continue;
        }
      }
    }
    newSelectedItems.push_back(item);
  }

  clearSelectedItems();
  for (unsigned int i = 0; i < newSelectedItems.size(); i++) {
    addItemToSelectedItems(newSelectedItems[i]);
  }
}

// ----------------------------------------- Slider
// ------------------------------------------------------------
Slider::Slider(Page *p, Panel *pan, SavingMode , QString nm,
               GLfloat *c1, GLfloat *c2, int x, int y, int minval, int maxval,
               int val, QString mess, int w, int h)
    : titlelabel(NULL), valuelabel(NULL) {
  initSlider(p, pan, nm, c1, c2, x, y, minval, maxval, val, mess, w, h);
}

Slider::Slider(Slider *I) : titlelabel(NULL), valuelabel(NULL) {
  initSlider(I->getPage(), I->getPanel(), I->getName(), I->getColour1(),
             I->getColour2(), I->getOrigin().x(), I->getOrigin().y(),
             I->getMinValue(), I->getMaxValue(), I->getValue(),
             I->getMessage());
}

Slider::~Slider() {
  delete titlelabel;
  delete valuelabel;
}

void Slider::initSlider(Page *p, Panel *pan, QString nm, GLfloat *c1,
                        GLfloat *c2, int x, int y, int minval, int maxval,
                        int val, QString mess, int, int) {
  panel = pan;
  name = nm;
  pg = p;
  message = mess;
  stepsize = 1;
  mode = "EXEC";
  iamselect = false;

  stepwidth = (int)((SLIDERHEIGHT * 0.5) + 0.5);

  initValue(val, minval, maxval);
  defaultVal = val;
  defaultValue = defaultVal;

  if (c1) {
    colour1[0] = c1[0];
    colour1[1] = c1[1];
    colour1[2] = c1[2];
  } else {
    colour1[0] = 1.0;
    colour1[1] = 1.0;
    colour1[2] = 1.0;
  }
  if (c2) {
    colour2[0] = c2[0];
    colour2[1] = c2[1];
    colour2[2] = c2[2];
  } else {
    colour2[0] = 1.0;
    colour2[1] = 0.0;
    colour2[2] = 0.0;
  }

  if (x >= 0 && y >= 0) {
    contour = QRect(x, y, SLIDERLENGTH + SLIDERHEIGHT,
                    SLIDERHEIGHT + 2 * SLIDERHEIGHT);
    slidebox =
        QRect(x + stepwidth, y + SLIDERHEIGHT, SLIDERLENGTH, SLIDERHEIGHT);
  }

  QFontMetrics fm(pg->panelfont());

  // calculate center position of the slider's value and title - this position
  // is constant amongst all panels
  QPoint valueCenterPos =
      QPoint(x + SLIDERLENGTH / 2, y - 0 * SLIDERHEIGHT / 2);
  QPoint titleCenterPos =
      QPoint(x + SLIDERLENGTH / 2 + SLIDERHEIGHT / 2, y + SLIDERHEIGHT * 2.75);

  QPoint valuePos = valueCenterPos - QPoint(fm.width(val) / 2, fm.height() / 2);
  QPoint titlePos =
      titleCenterPos - QPoint(fm.width(name) / 2, fm.height() / 2);
  valuelabel = new Label(pg, panel, QString::number(value), c1, valuePos.x(),
                         valuePos.y(), SLIDERLENGTH);
  titlelabel =
      new Label(pg, panel, nm, c1, titlePos.x(), titlePos.y(), SLIDERLENGTH);

  pg->getSelectColour(selcol);
  emptyContour = contour;
  int left = contour.left();
  int right = contour.right();
  int top = contour.top();
  int bottom = contour.bottom();

  if (titlelabel->getContour()->left() < left)
    left = titlelabel->getContour()->left();
  if (titlelabel->getContour()->right() > right)
    right = titlelabel->getContour()->right();
  if (titlelabel->getContour()->top() < top)
    top = titlelabel->getContour()->top();
  if (titlelabel->getContour()->bottom() > bottom)
    bottom = titlelabel->getContour()->bottom();

  if (valuelabel->getContour()->left() < left)
    left = valuelabel->getContour()->left();
  if (valuelabel->getContour()->right() > right)
    right = valuelabel->getContour()->right();
  if (valuelabel->getContour()->top() < top)
    top = valuelabel->getContour()->top();
  if (valuelabel->getContour()->bottom() > bottom)
    bottom = valuelabel->getContour()->bottom();

  contour.setLeft(left);
  contour.setRight(right);
  contour.setTop(top);
  contour.setBottom(bottom);

  type = "SLIDER";
  sliding = false;
}

void Slider::setName(QString nm) {
  name = nm;
  titlelabel->setName(nm);
}

void Slider::moveBy(int dx, int dy) {
  GLWidget *w = panel->getGLWidget();
  w->makeCurrent();

  if ((contour.bottom() + dy >= 0 || dy > 0) &&
      (contour.left() + dx >= 0 || dx > 0)) {
    contour.translate(dx, dy);
    emptyContour.translate(dx, dy);

    slidebox.translate(dx, dy);
    valuelabel->getContour()->translate(dx, dy);
    valuelabel->aligntext();
    titlelabel->getContour()->translate(dx, dy);
    titlelabel->aligntext();
  }
}

void Slider::setMinValue(int val) { initValue(value, val, max); }

void Slider::setMaxValue(int val) { initValue(value, min, val); }

void Slider::setDefaultValue(int val) {
  defaultVal = val;
  defaultValue = val;
}

void Slider::setValue(int val) {
  initValue(val, min, max);
  valuelabel->setName(QString::number(value));
}
void Slider::resetToDefaultValue() { setValue(defaultVal); }

void Slider::saveDefaultValue() { setDefaultValue(value); }

void Slider::initValue(int v, int mn, int mx, int) {
  min = mn;
  max = mx;
  value = v;

  if (value <= min) {
    steps = 0;
    value = min;
  } else if (value >= max) {
    steps = SLIDERLENGTH - 1 ;
    value = max;
  } else
    steps = (int)(((GLfloat)SLIDERLENGTH *
                   ((GLfloat)(value - min) / (GLfloat)(max - min))) +
                  0.5);
}


void Slider::aligntext() {
  valuelabel->aligntext();
  titlelabel->aligntext();
}

void Slider::setMode(QString M) {
  mode = M;
  valuelabel->setMode(M);
  titlelabel->setMode(M);
}

void Slider::draw(GLWidget *w) {
  w->makeCurrent();
  int h = w->height();
  if (steps > 0) {
    if (steps + slidebox.left()>slidebox.right())
	steps = slidebox.right()- slidebox.left();
    glColor3fv(colour2);
    glBegin(GL_QUADS);
    {
      glVertex2i(slidebox.left() , h - slidebox.bottom());
      glVertex2i(slidebox.left() , h - slidebox.top());
      glVertex2i(slidebox.left() + steps , h - slidebox.top());
      glVertex2i(slidebox.left() + steps , h - slidebox.bottom());
    }
    glEnd();
  }
  
  glColor3fv(colour1);

  glBegin(GL_LINE_LOOP);
  {
    glVertex2i(slidebox.left() , h - slidebox.bottom());
    glVertex2i(slidebox.left() , h - slidebox.top());
    glVertex2i(slidebox.right() , h - slidebox.top());
    glVertex2i(slidebox.right() , h - slidebox.bottom());
  }
  glEnd();

  glBegin(GL_TRIANGLES);
  {
    glVertex2i(slidebox.left() - 3, h - slidebox.bottom());
    glVertex2i(slidebox.left() - 8 , h - (int)((slidebox.bottom() + slidebox.top()) * 0.5));
    glVertex2i(slidebox.left() - 3, h - slidebox.top());

    glVertex2i(slidebox.right() + 2, h - slidebox.bottom() );
    glVertex2i(slidebox.right() + 7, h - (int)((slidebox.bottom() + slidebox.top()) * 0.5));
    glVertex2i(slidebox.right() + 2, h - slidebox.top() );
  }
  glEnd();
  
  titlelabel->draw(w);
  valuelabel->draw(w);
}

void Slider::mousePress(QPoint m) {
  sliding = false;
  itemMoved = false;

  if (mode == "EXEC") {
    // sliding
    if (m.x() >= slidebox.left() && m.x() <= slidebox.right() &&
        m.y() <= slidebox.bottom() && m.y() >= slidebox.top()) { // inside box
      sliding = true;
      if (m.x() == slidebox.left()) { // at left edge
        steps = 0;
        value = min;
      } else if (m.x() == slidebox.right()) { // at right edge
        steps = SLIDERLENGTH - 1;
        value = max;
      } else {
        value = min + (int)(((GLfloat)(max - min) *
                             (GLfloat)(m.x() - slidebox.left()) /
                             (GLfloat)SLIDERLENGTH) +
                            0.5);
        steps = (int)(((GLfloat)SLIDERLENGTH *
                       ((GLfloat)(value - min) / (GLfloat)(max - min))) +
                      0.5);
      }
      valuelabel->setName(QString::number(value));
    }
    // left stepper
    else if (m.x() < slidebox.left() && m.x() >= contour.left() - 3 &&
             m.y() <= slidebox.bottom() && m.y() >= slidebox.top()) {
      value -= stepsize;
      if (value <= min) {
        steps = 0;
        value = min;
      } else {
        steps = (int)(((GLfloat)SLIDERLENGTH *
                       ((GLfloat)(value - min) / (GLfloat)(max - min))) +
                      0.5);
      }
      valuelabel->setName(QString::number(value));
    }
    // right stepper
    else if (m.x() > slidebox.right() && m.x() <= contour.right() + 2 &&
             m.y() <= slidebox.bottom() && m.y() >= slidebox.top()) {
      value += stepsize;
      if (value >= max) {
        steps = SLIDERLENGTH - 1;
        value = max;
      } else
        steps = (int)(((GLfloat)SLIDERLENGTH *
                       ((GLfloat)(value - min) / (GLfloat)(max - min))) +
                      0.5);
      valuelabel->setName(QString::number(value));
    }
  } else {
    lastclick = m;
    iamselect = true;
    preSnapPosition = contour.topLeft();
  }
}

void Slider::mouseMove(QPoint m) {
  if (mode == "EXEC") {
    if (sliding) {
      if (m.x() <= slidebox.left()) {
        steps = 0;
        value = min;
      } else if (m.x() >= slidebox.right()) {
        steps = SLIDERLENGTH - 1;
        value = max;
      } else {
        value = min + (int)(((GLfloat)(max - min) *
                             (GLfloat)(m.x() - slidebox.left()) /
                             (GLfloat)SLIDERLENGTH) +
                            0.5);
        steps = (int)(((GLfloat)SLIDERLENGTH *
                       ((GLfloat)(value - min) / (GLfloat)(max - min))) +
                      0.5);
      }
      valuelabel->setName(QString::number(value));
    }

    if (_savingMode == CONTINUOUS)
      processMessage();
  } else {
    itemMoved = true;
    int dx = m.x() - lastclick.x();
    int dy = m.y() - lastclick.y();

    if (panel->isSnapping()) {
      // recalculate pre-snap position
      preSnapPosition += QPoint(dx, dy);

      // move to pre-snap position and snap
      int moveByX = preSnapPosition.x() - contour.x();
      int moveByY = preSnapPosition.y() - contour.y();
      moveBy(moveByX, moveByY);
      snap();
    } else {

      moveBy(dx, dy);
    }

    lastclick = m;
    this->getPanel()->setEdited(true);
  }
}

void Slider::mouseRelease(QPoint) {
  if (mode == "EXEC") {
    processMessage();
  }
  itemMoved = false;

  if (mode == "EDIT" && panel->isSnapping()) {
    snap();
    this->getPanel()->setEdited(true);
  }
}

// ----------------------------------------------------- Button
// ----------------------------------------
Button::Button(Page *p, Panel *pan, QString nm, GLfloat *c1, GLfloat *c2, int x,
               int y, int val, QString mess, GLfloat *c3, bool tricolor, int w,
               int h)
    : group(NULL), label(NULL) {
  initButton(p, pan, nm, c1, c2, x, y, val, mess, c3, tricolor, w, h);
}

Button::Button(Button *I) : group(NULL), label(NULL) {
  initButton(I->getPage(), I->getPanel(), I->getName(), I->getColour1(),
             I->getColour2(), I->getOrigin().x(), I->getOrigin().y(),
             I->getValue(), I->getMessage(), I->getColour3(),
             !I->isTransparent());
}

Button::~Button() { delete label; }

void Button::initButton(Page *p, Panel *pan, QString nm, GLfloat *c1,
                        GLfloat *c2, int x, int y, int val, QString mess,
                        GLfloat *c3, bool tricolor, int w, int h) {
  name = nm;
  pg = p;
  panel = pan;
  value = val;
  defaultValue = value;
  message = mess;
  mode = "EXEC";
  iamselect = false;
  down = false;
  tri = tricolor;

  if (w > 0)
    width = w;
  else
    width = BUTTONLENGTH;

  if (h > 0)
    height = h;
  else
    height = BUTTONHEIGHT;


  if (c1) {
    colour1[0] = c1[0];
    colour1[1] = c1[1];
    colour1[2] = c1[2];
  } else {
    colour1[0] = 1.0;
    colour1[1] = 1.0;
    colour1[2] = 1.0;
  }
  if (c2) {
    colour2[0] = c2[0];
    colour2[1] = c2[1];
    colour2[2] = c2[2];
  } else {
    colour2[0] = 0.0;
    colour2[1] = 0.0;
    colour2[2] = 1.0;
  }
  if (c3) {
    colour3[0] = c3[0];
    colour3[1] = c3[1];
    colour3[2] = c3[2];
  } else {
    colour3[0] = 0.0;
    colour3[1] = 0.0;
    colour3[2] = 0.0;
  }

  pg->getSelectColour(selcol);

  if (x >= 0 && y >= 0) {
    contour = QRect(x - 3, y - 3, width + 6, height + 6);
    buttonBox = QRect(x, y, width, height);
  }

  // calculate center position of the button's title - this position is constant
  // amongst all panels
  QPoint titleCenterPos = QPoint(x + width / 2, y + height / 2);

  QFontMetrics fm(pg->panelfont());
  QString tmpName = name;
  tmpName.remove(getExtension());
  QPoint titlePos =
      titleCenterPos -
      QPoint(fm.width(tmpName) / 2, fm.height() / 2);

  //  label = new Label(pg, panel, nm.remove(getExtension()), c1, titlePos.x(),
  //                 titlePos.y(), width, height);
  label = new Label(pg, panel, nm, c1, titlePos.x(),
                    titlePos.y(), width, height);
  type = "BUTTON";
}

void Button::setColour1(GLfloat *c1) {
  if (label != NULL) {
    label->setColour1(c1);
  }

  for (int i = 0; i < 3; i++) {
    colour1[i] = c1[i];
  }
}

void Button::setColour3(GLfloat *c3) {
  for (int i = 0; i < 3; i++) {
    colour3[i] = c3[i];
  }
}

void Button::setName(QString nm) {
  calculateButtonName(nm);
  //  label->setName(nm.remove(getExtension()));
  label->setName(nm);
}

void Button::setSize(int w, int h) {
  width = w;
  height = h;
}

void Button::aligntext() {
  label->aligntext();
  panel->getGLWidget()->update();
}

void Button::setMode(QString M) {
  mode = M;
  label->setMode(M);
}

void Button::draw(GLWidget *w) {
  int h = w->height();
  if (value == ON || down) {
    glColor3fv(colour2);
    glBegin(GL_QUADS);
    {
      glVertex2i(buttonBox.left(), h - buttonBox.bottom());
      glVertex2i(buttonBox.left(), h - buttonBox.top());
      glVertex2i(buttonBox.right(), h - buttonBox.top());
      glVertex2i(buttonBox.right(), h - buttonBox.bottom());
    }
    glEnd();
  } else {
    if (tri) {
      glColor3fv(colour3);
      glBegin(GL_QUADS);
      {
        glVertex2i(buttonBox.left(), h - buttonBox.bottom());
        glVertex2i(buttonBox.left(), h - buttonBox.top());
        glVertex2i(buttonBox.right(), h - buttonBox.top());
        glVertex2i(buttonBox.right(), h - buttonBox.bottom());
      }
      glEnd();
    }
  }

  glColor3fv(colour1);
  glBegin(GL_LINE_LOOP);
  {
    glVertex2i(buttonBox.left(), h - buttonBox.bottom());
    glVertex2i(buttonBox.left(), h - buttonBox.top());
    glVertex2i(buttonBox.right(), h - buttonBox.top());
    glVertex2i(buttonBox.right(), h - buttonBox.bottom());
  }
  glEnd();

  label->draw(w);
}
void Button::mousePress(QPoint m) {
  itemMoved = false;
  if (mode == "EXEC") {
    if (value == MONOSTABLE) {
      value = ON;
      down = true;
      processMessage();
      value =
          MONOSTABLE; //[PASCAL awful way to handle the MONOSTABLE but didn't
                      //see any better way without recoding too much:()
    } else if (!group) {
      toggle();
    } else {
      group->unset();
      value = ON;
    }
  } else {
    lastclick = m;
    iamselect = true;
    preSnapPosition = contour.topLeft();
  }
}

void Button::mouseMove(QPoint m) {

  if (mode == "EDIT") {
    itemMoved = true;

    int dx = m.x() - lastclick.x();
    int dy = m.y() - lastclick.y();

    // Buttons in groups dont get snapped
    if (panel->isSnapping() && group == NULL) {
      // recalculate pre-snap position
      preSnapPosition += QPoint(dx, dy);

      // move to pre-snap position and snap
      int moveByX = preSnapPosition.x() - contour.x();
      int moveByY = preSnapPosition.y() - contour.y();
      moveBy(moveByX, moveByY);
      snap();
    } else {

      moveBy(dx, dy);
    }

    lastclick = m;
    this->getPanel()->setEdited(true);
  }
}

void Button::mouseRelease(QPoint) {
  itemMoved = false;

  if (mode == "EXEC") {
    if (value == MONOSTABLE) {
      value = OFF;
      down = false;
      processMessage();
      value = MONOSTABLE;
    } else
      processMessage(); // if button turned ON in group, or toggled
  }
}

void Button::moveBy(int dx, int dy) {
  GLWidget *w = panel->getGLWidget();
  w->makeCurrent();
  if ((contour.bottom() + dy >= 0 || dy > 0) &&
      (contour.left() + dx >= 0 || dx > 0)) {

    contour.translate(dx, dy);

    buttonBox.translate(dx, dy);
    label->getContour()->translate(dx, dy);
    label->aligntext();
    if (group)
      group->realign();
  }
}

void Button::toggle() {
  if (value == OFF)
    value = ON;
  else if (value == ON)
    value = OFF;
}

void Button::groupIn(Group *G) { group = G; }

void Button::calculateButtonName(QString name) {
  QString extension = QString("");
  QString nameAndExtension = name + extension;
  
  int i = 1;
  while (!isButtonNameUnique(nameAndExtension)) {
    extension = QString("%Copy") + QString::number(i);

    nameAndExtension = name + extension;
    i++;
  }
  
  this->name = nameAndExtension;
}

bool Button::isButtonNameUnique(QString name) {
  if (pg != NULL) {
    for (int i = 0; i < pg->numItems(); i++) {
      Item *item = pg->getItem(i);
      if (item->getType() == "BUTTON") {
        Button *b = (Button *)item;
        if (b != this) {
          // if this button has same name as another
          if (QString::compare(name, b->getName(), Qt::CaseInsensitive) == 0) {
            return false;
          }
        }
      }
    }
  }
  return true;
}

QString Button::getExtension() {
  int i = name.indexOf("%Copy");
  if (i >= 0) {
    return name.mid(i, name.length() - 1);
  } else {
    return QString("");
  }
}
// ------------------------------------------------------- Group
// --------------------------------------
Group::Group(Page *p, Panel *pan, GLfloat *c1, QString nm, int w, int h) {
  initGroup(p, pan, c1, nm, w, h);
}

Group::Group(Group *I) {
  initGroup(I->getPage(), I->getPanel(), I->getColour1(), I->getName());
}

Group::~Group() {}

void Group::initGroup(Page *p, Panel *pan, GLfloat *c1, QString nm, int, int) {
  name = nm;
  panel = pan;
  margin = 2;
  mode = "EXEC";
  iamselect = false;
  pg = p;
  if (c1) {
    colour1[0] = c1[0];
    colour1[1] = c1[1];
    colour1[2] = c1[2];
  } else {
    colour1[0] = 1.0;
    colour1[1] = 1.0;
    colour1[2] = 1.0;
  }

  pg->getSelectColour(selcol);

  type = "GROUP";
}

bool Group::containsButton(Button *b) {
  for (unsigned int i = 0; i < buttons.size(); i++) {
    if (buttons[i]->getName() == b->getName()) {
      return true;
    }
  }
  return false;
}

void Group::addButtonName(QString nm) { names.push_back(nm); }

void Group::addButton(Button *b) {
  buttons.push_back(b);
  buttons.back()->groupIn(this);
  realign();
}

void Group::setButtons(std::vector<Button *> buttons) {
  for (unsigned int i = 0; i < buttons.size(); i++) {
    Group *g = buttons[i]->getGroup();
    if (g != NULL) {
      g->deleteButton(buttons[i]);
    }

    addButton(buttons[i]);
  }
}

void Group::deleteButton(Button *b) {
  for (unsigned int i = 0; i < buttons.size(); i++) {
    if (buttons[i] == b) {
      buttons.erase(buttons.begin() + i);
      break;
    }
  }

  realign();
  if (buttons.size() <= 1) {
    pg->deleteItem(this);
  }
}

void Group::realign() {
  if (buttons.size()) {
    int left = buttons[0]->getContour()->left();
    int right = buttons[0]->getContour()->right();
    int top = buttons[0]->getContour()->top();
    int bottom = buttons[0]->getContour()->bottom();

    for (unsigned int i = 1; i < buttons.size(); i++) {
      if (buttons[i]->getContour()->left() < left + margin)
        left = buttons[i]->getContour()->left();
      if (buttons[i]->getContour()->right() > right - margin)
        right = buttons[i]->getContour()->right();
      if (buttons[i]->getContour()->top() < top - margin)
        top = buttons[i]->getContour()->top();
      if (buttons[i]->getContour()->bottom() > bottom + margin)
        bottom = buttons[i]->getContour()->bottom();
    }
    contour.setLeft(left - margin);
    contour.setRight(right + margin);
    contour.setTop(top - margin);
    contour.setBottom(bottom + margin);
  }
}

void Group::unset() {
  for (unsigned int i = 0; i < buttons.size(); i++) {
    if (buttons[i]->getValue() == ON)
      buttons[i]->setValue(OFF);
  }
}

void Group::draw(GLWidget *) {
  int h = getPanel()->getPanelHeight();

  if (buttons.size()) {
    if (mode == "EDIT" && iamselect)
      glColor3fv(selcol);
    else
      glColor3fv(colour1);

    glBegin(GL_LINE_LOOP);
    {
      glVertex2i(contour.left(), h - contour.bottom());
      glVertex2i(contour.left(), h - contour.top());
      glVertex2i(contour.right(), h - contour.top());
      glVertex2i(contour.right(), h - contour.bottom());
    }
    glEnd();
  }
}

void Group::mousePress(QPoint m) {
  if (mode == "EDIT") {
    itemMoved = false;

    iamselect = true;
    lastclick = m;
    this->getPanel()->setEdited(true);
    preSnapPosition = contour.topLeft();
  }
}

void Group::mouseRelease(QPoint) {
  itemMoved = false;

  if (mode == "EDIT" && panel->isSnapping()) {
    this->getPanel()->setEdited(true);
  }
}

void Group::mouseMove(QPoint m) {
  if (mode == "EDIT") {
    itemMoved = true;

    int dx = m.x() - lastclick.x();
    int dy = m.y() - lastclick.y();

    if (panel->isSnapping()) {
      // recalculate pre-snap position
      preSnapPosition += QPoint(dx, dy);

      // move to pre-snap position and snap, moving buttons as well
      int moveByX = preSnapPosition.x() - contour.x();
      int moveByY = preSnapPosition.y() - contour.y();

      moveBy(moveByX, moveByY);
      snap();
    } else {

      moveBy(dx, dy);
    }

    lastclick = m;
    this->getPanel()->setEdited(true);
  }
}

void Group::moveBy(int dx, int dy) {
  if (contour.bottom() + dy >= 0 && contour.left() + dx >= 0) {

    contour.translate(dx, dy);

    for (unsigned int i = 0; i < buttons.size(); i++) {
      if (!buttons[i]->isSelected()) {
	buttons[i]->moveBy(dx,dy);
	/*
        buttons[i]->getContour()->translate(dx, dy);
        buttons[i]->getLabel()->getContour()->translate(dx, dy);
        buttons[i]->getLabel()->aligntext();
	*/
      }
    }
  }
}

// ------------------------------------------------------ Label
// ------------------------------------------------
Label::Label(Page *p, Panel *pan, QString nm, GLfloat *c1, int x, int y, int w,
             int h) {
  initLabel(p, pan, nm, c1, x, y, w, h);
}

Label::Label(Label *I) {
  initLabel(I->getPage(), I->getPanel(), I->getName(), I->getColour1(),
            I->getOrigin().x(), I->getOrigin().y());
}

void Label::initLabel(Page *p, Panel *pan, QString nm, GLfloat *c1, int x,
                      int y, int w, int h) {
  name = nm;
  pg = p;
  panel = pan;

  width = w;
  height = h;

  mode = "EXEC";
  iamselect = false;

  if (c1) {
    colour1[0] = c1[0];
    colour1[1] = c1[1];
    colour1[2] = c1[2];
  } else {
    colour1[0] = 1.0;
    colour1[1] = 1.0;
    colour1[2] = 1.0;
  }

  pg->getSelectColour(selcol);

  if (x >= 0 && y >= 0) {
    QFontMetrics fm(pg->panelfont());
    int textwidth = fm.width(name);
    int textheight = fm.height();
    contour = QRect(x, y, textwidth, textheight);
  }

  aligntext();
  type = "LABEL";
}

void Label::setName(QString nm) {
  name = nm;
  aligntext();
}

void Label::aligntext() {
  QFontMetrics fm(pg->panelfont());
  int textwidth = fm.width(name);
  int textheight = fm.height();

  QPoint currContourCenter = contour.center();

  xras = currContourCenter.x() - (textwidth / 2.0);
  yras = currContourCenter.y() - (textheight / 2.0);
  contour.setSize(QSize(textwidth, textheight));
  contour.moveCenter(currContourCenter);
}

void Label::setSize(int w, int h) {
  width = w;
  height = h;
}

void Label::draw(GLWidget *w) {
  QFontMetrics fm(pg->panelfont());
  int textwidth = fm.width(name);
  int textheight = fm.height();

  /* quickly reject all the text that would not be displayed at all */
  if (xras > panel->getPanelWidth())
    return;
  if (yras > panel->getPanelHeight())
    return;
  if (xras + textwidth < 0)
    return;
  if (yras + textheight < 0)
    return;

  w->makeCurrent();
  glPushAttrib(GL_LIGHTING_BIT | GL_TEXTURE_BIT);
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  glColor3fv(colour1);

  QColor color(colour1[0] * 255, colour1[1] * 255, colour1[2] * 255);

  int posx = xras;
  int posy = w->height() - yras;
  QString displayedName = QString(name);
  int pos = displayedName.indexOf("%Copy");
  if (pos >0)
    displayedName.truncate(pos);
  w->renderText(posx, posy, 0, displayedName, color, pg->panelfont());
  glPopAttrib();

#ifdef DONT_COMPILE

  /**
   *** if the text begins outside, we fix the viewport, so that opengl will
   *** perform the clipping automatically
   **/
  if (xras < 0 || yras < 0) {
    glColor3fv(colour1);
    /* save current view port */
    glPushAttrib(GL_VIEWPORT_BIT | GL_LIST_BIT);
    /* new viewport */
    glViewport(xras - 2, yras - 2, panel->getPanelWidth(),
               panel->getPanelHeight());
    /* position of the bitmap will be now 0,0 */
    glRasterPos2i(2, 2);
    /* now output the string */
    glListBase(panel->getAllYourBase());
    glCallLists(name.length(), GL_UNSIGNED_BYTE,
                (GLubyte *)((const char *)name));
    /* restore the viewport */
    glPopAttrib();
  } else {
    glColor3fv(colour1);
    glPushAttrib(GL_LIST_BIT);
    /* set the current position for the bitmap */
    glRasterPos2d(xras, yras);
    /* now output the string */
    glListBase(panel->getAllYourBase());
    glCallLists(name.length(), GL_UNSIGNED_BYTE,
                (GLubyte *)((const char *)name));
    glPopAttrib();
  }

#endif
}

void Label::mousePress(QPoint m) {
  if (mode == "EDIT") {
    itemMoved = false;

    iamselect = true;
    lastclick = m;
    this->getPanel()->setEdited(true);
    preSnapPosition = contour.topLeft();
  }
}

void Label::mouseRelease(QPoint) {
}

void Label::mouseMove(QPoint m) {
  if (mode == "EDIT") {
    int dx = m.x() - lastclick.x();
    int dy = m.y() - lastclick.y();

    if (panel->isSnapping()) {
      itemMoved = true;

      // recalculate pre-snap position
      preSnapPosition += QPoint(dx, dy);

      // move to pre-snap position and snap
      int moveByX = preSnapPosition.x() - contour.x();
      int moveByY = preSnapPosition.y() - contour.y();
      moveBy(moveByX, moveByY);
      snap();
    } else {

      moveBy(dx, dy);
    }

    lastclick = m;
    this->getPanel()->setEdited(true);
  }
}

void Label::moveBy(int dx, int dy) {
  if ((contour.bottom() + dy >= 0 || dy > 0) &&
      (contour.left() + dx >= 0 || dx > 0)) {
    contour.translate(dx, dy);
    aligntext();
  }
}

// ------------------------------------------ Menu
// ------------------------------------
Menu::Menu(Panel *pan, QAction *id, QString nm, QString mess) {
  name = nm;
  panel = pan;
  value = 0;
  menu_id = id;
  message = mess;
  type = "MENU";
  mode = "EXEC";
}

void Menu::activate() {
  if (mode == "EXEC")
    processMessage();
}

// EOF: items.cc
