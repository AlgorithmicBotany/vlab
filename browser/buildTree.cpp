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



#include "buildTree.h"
#include "QTGLbrowser.h"
#include "graphics.h"
#include "layout.h"
#include "main.h"
#include <stack>

static void _reallign_nodes(NODE *root);

static bool compacting = true;
static bool center_parent = true;

void build_set_compacting(bool val) { compacting = val; }

void build_set_center_parent(bool val) { center_parent = val; }

/******************************************************************************
 *
 * set the dimensions for every single node in the tree
 * ( nodeWidth, nodeHeight)
 *
 */

void set_node_dimensions(NODE *node) {
  int i;

  /* set the dimensions for the children of this node */
  for (i = 0; i < node->nChildren; i++)
    set_node_dimensions(node->child[i]);

  /* set the dimensions for this node */

  /* nodeWidth */
  node->nodeWidth = box_width + box_to_text_distance + node->strWidth;
  if (node->iconShow)
    if (node->iconWidth > node->nodeWidth)
      node->nodeWidth = node->iconWidth;

  /* nodeHeight */
  if (node->iconShow)
    node->nodeHeight =
        selectionHeight + text_to_icon_distance + node->iconHeight;
  else
    node->nodeHeight = selectionHeight;
}

/******************************************************************************
 *
 * computes the x,y for every node in the tree given the rx, ry
 *
 */

void set_relative_xy(NODE *root) {
  int x = root->x;
  int y = root->y;
  for (int i = 0; i < root->nChildren; i++) {
    x += root->child[i]->rx;
    y += root->child[i]->ry;
    root->child[i]->x = x;
    root->child[i]->y = y;
    set_relative_xy(root->child[i]);
  }
}

static void find_extremes(NODE *root)
// recursively calculates _treeRect for the whole subtree:
{
  // basic rectangle only contains the node itself
  QPoint tl = QPoint(root->x, root->y);
  QPoint br = QPoint(root->x + root->nodeWidth, root->y + root->nodeHeight);
  root->_treeRect = QRect(tl, br);

  // recursively find the extremes for all children and unite the results with
  // this rectangle
  for (int i = 0; i < root->nChildren; i++) {
    find_extremes(root->child[i]);
    root->_treeRect = root->_treeRect.united(root->child[i]->_treeRect);
  }

  root->treeWidth = root->_treeRect.width();
  root->treeHeight = root->_treeRect.height();
}

void _build_tree(NODE *root, int x, int y) {
  int i;
  int nodeWidth;
  int childY;

  /* we don't want to do anything with a null node */
  if (root == NULL)
    return;

  /* assign the coordinates to this node: */
  root->x = x;
  /* now, we have to calculate the y position of the node,
   * which depends on the graphicsType */
  if (center_parent) {
    if (root->nChildren > 0)
      if (root->childrenHeight < root->nodeHeight)
        root->y = y;
      else
        root->y = y - (root->childrenHeight - root->nodeHeight) / 2;
    else
      root->y = y;
  } else { /* PARENT_TOP */
    root->y = y;
  }

  /* compute the dimensions of this node: */

  /* first compute the width of the node without considering the icon,
   * i.e. only considering the text   */

  nodeWidth = box_width + box_to_text_distance + root->strWidth;

  /* now, adjust the width of the node to the width of the icon, if the
   * icon is displayed, and is wider than the text only     */
  if (root->iconShow)
    if (nodeWidth < root->iconWidth)
      nodeWidth = root->iconWidth;

  /* now, recursively build the children: */
  childY = y; /* childY will contain the upper left
               * corner's Y of the child*/
  for (i = 0; i < root->nChildren; i++) {
    _build_tree(root->child[i],
                x + nodeWidth + text_to_line_distance + horiz_distance, childY);
    /* update the childY for the next child */
    childY -= root->child[i]->treeHeight + vert_distance;
  }
}

static void build_tree_orig()
// calculates x,y position (top left corner) for each node in sysInfo.beginTree
// - the root node will have x,y = 0
// - the nodes are laid out according to sysInfo.graphicsType
// - the coordinates are in OpengStyle (origin in lower left corner)
{
  if (compacting == 0) {

    /* first, we should calculate the actual heights in the tree, because
     * _build_tree relies on this information */
    tree_set_dimensions();

    /* do the actual work in _build_tree() */
    _build_tree(sysInfo.beginTree, 0, 0);

    /* realligns the nodes, so that a first child of a parent does not
     * start below the parent */
    _reallign_nodes(sysInfo.beginTree);

    /* first we have to set the dimensions for every single node */
    set_node_dimensions(sysInfo.beginTree);

  } else {

    /* first we have to set the dimensions for every single node */
    set_node_dimensions(sysInfo.beginTree);

    /* now, compute the relative offsets for every node in the tree
     * into rx, ry (rx, ry = relative offset to the immediate parent) */

    layout(sysInfo.beginTree, center_parent);

    /* and compute the x,y (relative offset from the root of the entire
     * tree) for every node when the root is at 0,0 */
    sysInfo.beginTree->x = 0;
    sysInfo.beginTree->y = 0;
    set_relative_xy(sysInfo.beginTree);
  }
}

// will build the tree (based on the values in global variables defined in
// graphics.h)
// - the x,y of every node will be such that the min. value of either will be
// 0,0
void build_tree(void) {
  // adjust the selection height
  if (sysInfo.fontHeight < box_height)
    selectionHeight = box_height;
  else
    selectionHeight = sysInfo.fontHeight;

  tree_set_str_widths(sysInfo.wholeTree);

  // build the tree upside down:
  // - this is due to the fact that original drawing was done using OpenGL where
  // the
  //   coordinate system was upside down w.r.t. Qt and build_tree_orig() was not
  //   rewritten
  build_tree_orig();
  // flip the coordinates upside down, so that
  // x,y denotes top left corner of each node
  std::stack<NODE *> todo;
  todo.push(sysInfo.beginTree);
  while (!todo.empty()) {
    NODE *n = todo.top();
    todo.pop();
    n->y = -n->y;
    for (int i = 0; i < n->nChildren; i++)
      todo.push(n->child[i]);
  }
  // calculate bounding rectangles for all nodes in the tree
  find_extremes(sysInfo.beginTree);
  // translate the x,y coordinates so that the minimum x,y in the tree is at 0,0
  int dx = -sysInfo.beginTree->getTreeRect().x();
  int dy = -sysInfo.beginTree->getTreeRect().y();
  todo.push(sysInfo.beginTree);
  while (!todo.empty()) {
    NODE *n = todo.top();
    todo.pop();
    n->x += dx;
    n->y += dy;
    for (int i = 0; i < n->nChildren; i++)
      todo.push(n->child[i]);
  }
  // recalculate the extremes
  find_extremes(sysInfo.beginTree);

  sysInfo.treeWidth = sysInfo.beginTree->treeWidth;
  sysInfo.treeHeight = sysInfo.beginTree->treeHeight;

  // resize the drawing area to fit the tree
  QRect rect = sysInfo.beginTree->getTreeRect();
  // sysInfo.qgl-> resizeContents( rect.width(), rect.height() );
  sysInfo.qgl->resize(rect.width(), rect.height());
}

void _reallign_nodes(NODE *root) {
  int offset;
  int i;

  if (root == NULL)
    return;

  if (root->nChildren > 0) {
    offset = root->y - root->child[0]->y;
    if (offset > 0) {
      root->child[0]->y = root->y;
    }
    for (i = 0; i < root->nChildren; i++)
      _reallign_nodes(root->child[i]);
  }
}


