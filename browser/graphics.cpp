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



#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <vector>

#include <QPainter>
#include "QTGLbrowser.h"
#include "buildTree.h"
#include "buttons.h"
#include "dsprintf.h"
#include "error.h"
#include "eventHandle.h"
#include "font.h"
#include "graphics.h"
#include "main.h"
#include "nodeinfo.h"
#include "readPixmap.h"
#include "rgbToAbgr.h"
#include "scale.h"
#include "tree.h"
#include "xmemory.h"
#include "xutils.h"
#include <QPixmap>
#include <QPolygon>

// the height and width of the small icon for objects
const int box_height = 15;
const int box_width = 20;
// horizontal distance between parent and child
const int horiz_distance = 30;
// horizontal distance between the miniicon and the text
const int box_to_text_distance = 3;
// distance between the text and the line
const int text_to_line_distance = 1;
// vertical distace between two children
const int vert_distance = 2;
// vertical distance from text to icon
const int text_to_icon_distance = 3;
// the height of a box that is drawn in the background of the node when the node
// is selected.
int selectionHeight;

extern SystemInfo sysInfo; /* Global Variables for the browser */

// static int parentPositionType = PARENT_TOP;
static int box_y_off;

enum DrawOpE {
  EXPNOLINK,
  EXPLINK,
  SNOLINK,
  SLINK,
  HEXPNOLINK,
  HEXPLINK,
  HSNOLINK,
  HSLINK,
  HERROR
};

void centre_node(NODE *node)
// centers the node
{
  if (node == NULL)
    return;

  // try to make the whole tree visible if node has children
  if (node->nChildren > 0) {
    QRect r = node->getTreeRect();
    QPoint c = r.center();

    sysInfo.qgl->ensureVisible(c.x(), c.y(), r.width() / 2, r.height() / 2);
  }
  // now make sure the node itself is visible
  QRect r = node->getNodeRect();
  QPoint c = r.center();
  sysInfo.qgl->ensureVisible(c.x(), c.y(), r.width() / 2, r.height() / 2);
  return;

  if (node->nChildren == 0) {
    QRect r = node->getNodeRect();
    QPoint c = r.center();
    sysInfo.qgl->ensureVisible(c.x(), c.y(), 300, 300);
  } else {
    QRect r = node->getTreeRect();
    QPoint c = r.center();
    sysInfo.qgl->ensureVisible(c.x(), c.y(), 300, 300);
  }
}

void loadIcon(NODE *node, int iconSize) {
  // icon is no longer loading
  node->_isIconLoading = false;
  // try to read in the icon from object's directory
  if (node->isHObj != 1) {
    node->_icon =
        readPixmap(sysInfo.connection, QString("%1/icon").arg(node->name),
                   iconSize, sysInfo.tmpDir);
    // if the icon could not be read in from the object's directory, try the
    // root object
    if (node->_icon.isNull())
      node->_icon = readPixmap(sysInfo.connection,
                               QString("%1/icon").arg(sysInfo.oofs_dir),
                               iconSize, sysInfo.tmpDir);
  } else {
    if (node->object_name)
      node->_icon = readPixmap(sysInfo.connection,
                               QString("%1/icon").arg(node->object_name),
                               iconSize, sysInfo.tmpDir);
    // if the icon could not be read in from the object's
    // directory, try the root object
    if (node->object_name == 0 || node->_icon.isNull())
      node->_icon = readPixmap(sysInfo.connection,
                               QString("%1/h_icon").arg(sysInfo.oofs_dir),
                               iconSize, sysInfo.tmpDir);
  }
}

void hideIcon(NODE *node) {
  node->iconShow = false;
  node->_isIconLoading = false;
  node->_icon = QPixmap();
}

bool showIcon(NODE *node)
// loads an icon for the node
{
  // avoid core dumps... :)
  if (node == NULL)
    return false;
  // if the icon is already there, do nothing
  if (node->iconShow)
    return false;

  // obtain icon size from browser settings
  int iconSize = sysInfo.mainForm->browserSettings()
                     .get(BrowserSettings::IconSize)
                     .toInt();

  // the icon will be loaded on demand or in the idle loop
  node->_icon = QPixmap();
  node->iconShow = true;
  node->_isIconLoading = true;
  node->iconWidth = iconSize;
  node->iconHeight = iconSize + 1;

  // fire up the icon-loading idle loop
  sysInfo.mainForm->scheduleIdleIconLoader(true);
  return true;
}

bool refreshIcon(NODE *node) {
  if (node == NULL)
    return false;
  node->_isIconLoading = false;
  node->_icon = QPixmap();
  node->iconShow = true;
  int iconSize = sysInfo.mainForm->browserSettings()
                     .get(BrowserSettings::IconSize)
                     .toInt();
  node->iconWidth = iconSize;
  node->iconHeight = iconSize + 1;
  loadIcon(node, iconSize);
  sysInfo.mainForm->updateDisplay();
  return true;
}

/******************************************************************************
 *
 * show all icons in the tree
 */

void showAllIcons_in_tree(NODE *root) {
  int i;

  if (!root->iconShow)
    showIcon(root);
  if (root->nChildren > 0)
    for (i = 0; i < root->nChildren; i++)
      showAllIcons_in_tree(root->child[i]);
}

/******************************************************************************
 *
 * set every node in the tree not to display the icon
 */

void hideAllIcons_in_tree(NODE *root) {
  int i;

  if (root->iconShow)
    hideIcon(root);
  if (root->nChildren > 0)
    for (i = 0; i < root->nChildren; i++)
      hideAllIcons_in_tree(root->child[i]);
}

/******************************************************************************
 *
 * graphics init
 *
 * - will initialize the graphics by setting up the
 *   initial viewport
 * - create the gl_lists for various node icons
 *
 */

void graphics_init() {
  QTGLbrowser *qgl = sysInfo.qgl;
  sysInfo.winWidth = qgl->width();
  sysInfo.winHeight = qgl->height();
}

static void draw_box(DrawOpE op, QPainter &p, const BrowserSettings &bset) {
  QPolygon tri(3);
  tri.setPoint(0, 0, 0);
  tri.setPoint(1, box_width - 6, box_height / 2);
  tri.setPoint(2, 0, box_height);

  switch (op) {
  case EXPNOLINK:
    p.setPen(bset.get(BrowserSettings::BoxBorderColor).value<QColor>());
    p.setBrush(bset.get(BrowserSettings::BackgroundColor).value<QColor>());
    p.drawRect(3, 0, box_width - 3, box_height - 3);
    p.setBrush(bset.get(BrowserSettings::BoxFillColor).value<QColor>());
    p.drawRect(0, 3, box_width - 3, box_height - 3);
    break;
  case HEXPNOLINK:
    p.setPen(bset.get(BrowserSettings::HBoxBorderColor).value<QColor>());
    p.setBrush(bset.get(BrowserSettings::BackgroundColor).value<QColor>());
    p.save();
    p.translate(6, 0);
    p.drawConvexPolygon(tri);
    p.restore();
    p.setBrush(bset.get(BrowserSettings::HBoxFillColor).value<QColor>());
    p.drawConvexPolygon(tri);
    break;
  case EXPLINK:
    p.setPen(bset.get(BrowserSettings::BoxBorderColor).value<QColor>());
    p.setBrush(bset.get(BrowserSettings::BackgroundColor).value<QColor>());
    p.drawRect(3, 0, box_width - 3, box_height - 3);
    p.setBrush(bset.get(BrowserSettings::BoxFillColor).value<QColor>());
    p.drawRect(0, 3, box_width - 3, box_height - 3);
    /* the 'L' */
    p.setPen(bset.get(BrowserSettings::LinkColor).value<QColor>());
    p.drawLine(5, 10, 5, 5);
    p.drawLine(5, 5, 10, 5);
    break;
  case HEXPLINK:
    p.setPen(bset.get(BrowserSettings::HBoxBorderColor).value<QColor>());
    p.setBrush(bset.get(BrowserSettings::BackgroundColor).value<QColor>());
    p.save();
    p.translate(6, 0);
    p.drawConvexPolygon(tri);
    p.restore();
    p.setBrush(bset.get(BrowserSettings::HBoxFillColor).value<QColor>());
    p.drawConvexPolygon(tri);
    /* the 'L' */
    p.setPen(bset.get(BrowserSettings::LinkColor).value<QColor>());
    p.drawLine(5, 10, 5, 5);
    p.drawLine(5, 5, 10, 5);
    break;
  case SNOLINK:
    p.setPen(bset.get(BrowserSettings::BoxBorderColor).value<QColor>());
    p.setBrush(bset.get(BrowserSettings::BackgroundColor).value<QColor>());
    p.drawRect(0, 3, box_width - 3, box_height - 3);
    break;
  case HSNOLINK:
    p.setPen(bset.get(BrowserSettings::HBoxBorderColor).value<QColor>());
    p.setBrush(bset.get(BrowserSettings::BackgroundColor).value<QColor>());
    p.drawConvexPolygon(tri);
    break;
  case SLINK:
    p.setPen(bset.get(BrowserSettings::BoxBorderColor).value<QColor>());
    p.setBrush(bset.get(BrowserSettings::BackgroundColor).value<QColor>());
    p.drawRect(0, 3, box_width - 3, box_height - 3);
    /* the 'L' */
    p.setPen(bset.get(BrowserSettings::LinkColor).value<QColor>());
    p.drawLine(5, 10, 5, 5);
    p.drawLine(5, 5, 10, 5);
    break;
  case HSLINK:
    p.setPen(bset.get(BrowserSettings::HBoxBorderColor).value<QColor>());
    p.setBrush(bset.get(BrowserSettings::BackgroundColor).value<QColor>());
    p.drawConvexPolygon(tri);
    /* the 'L' */
    p.setPen(bset.get(BrowserSettings::LinkColor).value<QColor>());
    p.drawLine(5, 10, 5, 5);
    p.drawLine(5, 5, 10, 5);
    break;
  case HERROR:
    p.setPen(QColor(255, 0, 0));
    p.drawLine(0, 0, box_width, box_height);
    p.drawLine(0, box_height, box_width, 0);
    break;
  }
}

static void draw_mini_icon(NODE *root, QPainter &p, const BrowserSettings &bset)
// draws a mini-icon for the node based on the type of the node
{
  // draw a small icon for the node
  p.save();
  p.translate(root->x, root->y + box_y_off);
  if (root->isHObj == 0) {
    if (root->expandable)
      if (root->isLink)
        draw_box(EXPLINK, p, bset);
      else
        draw_box(EXPNOLINK, p, bset);
    else if (root->isLink)
      draw_box(SLINK, p, bset);
    else
      draw_box(SNOLINK, p, bset);
  } else if (root->isHObj == 1) {
    if (root->expandable)
      if (root->isLink)
        draw_box(HEXPLINK, p, bset);
      else
        draw_box(HEXPNOLINK, p, bset);
    else if (root->isLink)
      draw_box(HSLINK, p, bset);
    else
      draw_box(HSNOLINK, p, bset);
  } else {
    draw_box(HERROR, p, bset);
  }
  p.restore();
}

static void addClickableArea(const QPainter &p, int x1, int y1, int x2, int y2,
                             BUTTON_DATA_PTR ptr, BUTTON_CALLBACK_PROC proc) {
 
  QPoint p1 = QPoint(x1, y1) * p.combinedTransform();
  QPoint p2 = QPoint(x2, y2) * p.combinedTransform();

  addButton(&sysInfo.buttons, p1.x(), p1.y(), p2.x(), p2.y(), ptr, proc);
}

/******************************************************************************
 *
 * this routine will draw the entire tree from the root, and the lines
 * connecting the parent with the children will be drawn as a comb:
 *
 *
 *                      +---- child 1
 *                      |
 *        parent    ----+---- child 2
 *                      |
 *                      +---- child 3
 *
 */

// static long nodeDrawCount;

static void draw_tree(NODE *root, QPainter &p, const QRect &view,
                      const BrowserSettings &bset) {
  // avoid NULL pointer
  if (root == NULL) {
    qWarning("draw_tree in graphics.cpp was called with NULL pointer!");
    return;
  }

  // top left corner of the node:
  QPoint tl = root->getTopLeft();
  int x = tl.x();
  int y = tl.y();
  // where to render text --> x,y
  int text_x = x + box_width + box_to_text_distance;
  int text_y = y;

  /* check if any part of the tree is actually visible
   * (if not, then we don't have to bother draw the tree)
   */

  // basic clipping
  if (!view.intersects(root->getTreeRect())) {
    return;
  }

  p.save();
  /*** calculate the new X coordinate after the text is displayed (the
   ***  end X coordinate for this node) ***/
  int newX = x + box_width + box_to_text_distance + root->strWidth +
             text_to_line_distance;

  int textEndX = newX;

  // add parts of this node to the buttons
  addClickableArea(p, x + box_width, y, newX + 1, y + selectionHeight, root,
                   selectNodeNameCB);
  addClickableArea(p, x, y + box_y_off, x + box_width,
                   y + box_y_off + box_height, root, selectNodeBoxCB);

  // give the node select background if it is selected
  if (root == sysInfo.selNode) {
    p.fillRect(
        QRect(QPoint(text_x - 1, y - 1), QPoint(newX + 1, y + selectionHeight)),
        bset.getColor(BrowserSettings::SelectionColor));
    p.setPen(bset.getColor(BrowserSettings::LineColor));
    p.drawLine(text_x - 1, y + box_y_off + box_height, newX,
               y + box_y_off + box_height);
  }

  // draw the mini-icon for the node
  draw_mini_icon(root, p, bset);

  // draw the label
  std::string label;
  if (root->isHObj == 0)
    label = root->baseName;
  else
    label = root->screenName;
  // render the text
  p.setPen(bset.getColor(BrowserSettings::TextColor));
  QRect bbox = p.boundingRect(QRect(text_x, text_y, 1, 1), Qt::AlignLeft,
                              QString(label.c_str()));
  p.drawText(bbox, Qt::AlignLeft, QString(label.c_str()));

  /*** draw the icon - if there is one ***/
  if (root->iconShow) {
    if (x + root->iconWidth + text_to_line_distance > newX)
      newX = x + root->iconWidth;

    addClickableArea(
        p, x, y + selectionHeight + text_to_icon_distance, x + root->iconWidth,
        y + selectionHeight + text_to_icon_distance + root->iconHeight, root,
        selectNodeIconCB);

 
    p.drawPixmap(x, y + selectionHeight + text_to_icon_distance, root->icon());
  }

  // draw connecting lines to children (the comb)
  if (root->nChildren > 0) {
    // draw a horizontal line from the end of the text to half-way to the
    // children
    p.setPen(bset.getColor(BrowserSettings::LineColor));
    /* the horizontal line from the parent to the middle of the
     * distance to the children */
    p.drawLine(textEndX, y + box_y_off + box_height / 2,
               newX + horiz_distance / 2, y + box_y_off + box_height / 2);
    for (int i = 0; i < root->nChildren; i++) {
      p.drawLine(newX + horiz_distance / 2,
                 root->child[i]->y + box_y_off + box_height / 2,
                 newX + horiz_distance,
                 root->child[i]->y + box_y_off + box_height / 2);
    }

    // the vertical line (from the first child to the last one
    p.drawLine(newX + horiz_distance / 2,
               root->child[0]->y + box_y_off + box_height / 2,
               newX + horiz_distance / 2,
               root->child[root->nChildren - 1]->y + +box_y_off +
                   box_height / 2);

    /* vertical line from the parent to the first child */
    p.drawLine(newX + horiz_distance / 2, y + box_y_off + box_height / 2,
               newX + horiz_distance / 2,
               root->child[0]->y + box_y_off + box_height / 2);
  }
  p.restore();

  /*** now draw the children ***/
  for (int i = 0; i < root->nChildren; i++)
    draw_tree(root->child[i], p, view, bset);
}


/******************************************************************************
 *
 * this routine redraws the entire tree
 *
 */
void redraw(QPainter &p, const BrowserSettings &bset, const QRect &view) {
  // update the size of the viewing area

  p.save();

  // clear the screen with background color
  if (1) {
    p.fillRect(view, bset.getColor(BrowserSettings::BackgroundColor));
  } else {
    static int r = 0;
    r += 5;
    r = r % 100;
    p.fillRect(view, QColor(r, r, r));
  }

  // figure out font related stuff
  QFont font = bset.get(BrowserSettings::TextFont).value<QFont>();
  p.setFont(font);
  QFontMetrics fm(font);
  box_y_off = selectionHeight - box_height - fm.descent();
  if (box_y_off < 0)
    box_y_off = 0;

  // delete all the button definitions
  delAllButtons(&sysInfo.buttons);

  if (0) {
    int w = sysInfo.qgl->size().width();
    int h = sysInfo.qgl->size().height();
    p.setWindow(-w / 10, -h / 10, w * 12 / 10, h * 12 / 10);
  }
  //    nodeDrawCount = 0;
  draw_tree(sysInfo.beginTree, p, view, bset);

  if (0) {
    // draw rectangle around the tree
    p.setPen(QColor(255, 255, 255));
    p.setBrush(Qt::NoBrush);
    p.drawRect(sysInfo.beginTree->getTreeRect());
  }
  p.restore();

  // draw rectangle around the view
  if (0) {
    p.setPen(QColor(255, 0, 0));
    p.drawRect(view);
  }

  // debuggin stuff - draw bounding rectangle for the selected node
  if (0) {
    if (sysInfo.selNode != NULL) {
      p.save();
      p.setPen(
          QPen(bset.getColor(BrowserSettings::SelectionColor), 0, Qt::DotLine));
      p.setBrush(Qt::NoBrush);
      p.drawRect(sysInfo.selNode->getNodeRect());
      p.drawRect(sysInfo.selNode->getTreeRect());
      p.restore();
    }
  }
}
