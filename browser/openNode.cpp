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



#include <limits.h>
#include <string.h>
#include <unistd.h>

#include "buildTree.h"
#include "graphics.h"
#include "main.h"
#include "tree.h"
#include "parse_object_location.h"
#include "xmemory.h"
#include "xstring.h"

/******************************************************************************
 *
 * matchName( name1, name2)
 *
 * returns = true, if 'name2'+'/' is at the beginning of 'name1'+'/'
 *         = false, otherwise
 *
 */

int matchName(const char *name1, const char *name2) {
  int l1, l2;
  int i;

  l1 = xstrlen(name1);
  l2 = xstrlen(name2);
  if (l1 < l2)
    return 0;

  i = 0;
  while (i < l2)
    if (name1[i] == name2[i])
      i++;
    else
      return 0;
  if (name1[i] == '\0')
    return 1;
  if (name1[i] == '/')
    return 1;

  return 0;
}

/******************************************************************************
 *
 * expandAndFind() -
 *
 * It will expand all the parents of a node whose fullname is 'name', and
 * then return a pointer to this node.
 *
 */

NODE *expandAndFind(NODE *root, const char *name) {
  int i;
  NODE *result;

  if (xstrcmp(name, root->name) == 0)
    return root;

  if (!matchName(name, root->name))
    return NULL;

  /* expand this tree if it is not already expanded */
  if (root->nChildren == 0) {
    // read in the extensions
    get_file_tree(root);
  }

  for (i = 0; i < root->nChildren; i++) {
    result = expandAndFind(root->child[i], name);
    if (result != NULL)
      return result;
  }

  return NULL;
}

/******************************************************************************
 *
 * will open up a node 'name' in the tree. If the parents of that node
 * are not open, it will open up those as well. The name is a path to the
 * node (absolute path).
 *
 * it also highlights the node and centers is in the window.
 *
 * returns: True = success
 *          False = failure (the node doesn't exist)
 */

int openNode(const char *name) {
  NODE *node;

  node = expandAndFind(sysInfo.beginTree, name);
  sysInfo.selNode = node;
  build_tree();
  sysInfo.mainForm->update_menus();
  centre_node(node);
  sysInfo.mainForm->updateDisplay();

  return (node != NULL);
}

/******************************************************************************
 *
 * if the root's realpath matches the beginning of the name, then this root
 * is expanded, and its children are searched further
 */

void expandFind_realPath(NODE *root, char *name) {
  int i;

  if (!matchName(name, root->realPath))
    return;

  /* check whether this root is the actual match */
  if (xstrlen(name) == xstrlen(root->realPath))
    node_update(root);

  if (root->nChildren == 0) {
    /* we are not going to open up links */
    if (root->isLink)
      return;
    /* the current root must be expandable */
    root->expandable = true;
    /* expand it */
    show_extensions(root);
  }

  /* now we recursively check all the children */
  for (i = 0; i < root->nChildren; i++)
    expandFind_realPath(root->child[i], name);
}

/******************************************************************************
 *
 * this function will try to open up all the parents of the node whose name
 * is 'name'.
 *
 * It will be used by the 'update' message (when a new child is created
 * for an object)
 *
 */

void openNode_realPath(char *name) {
  // parse the name of the object
  char *user_name;
  char *host_name;
  char *obj_name;
  parse_object_location(name, &user_name, &host_name, &obj_name);

  // make sure that host name of the requested object is the same
  // as the host name our browser is connected to
  if (xstrcmp(host_name, sysInfo.host_name) != 0) {
    // host names do not match --> do not do anything
    return;
  }

  // hosts are the same --> proceed

  // determine the real path of the object
  char *realPath;

  if (RA::Realpath(sysInfo.connection, obj_name, realPath)) {
    return; /* bad filename */
  }

  /* now look for a node with the realpath somewhere in the tree,
   * expanding all its parents */

  expandFind_realPath(sysInfo.wholeTree, realPath);
}

/******************************************************************************
 *
 * will find a node that is already open in the tree whose realpath
 * matches the begining of 'name'
 *
 */

NODE *findMatchingParent_rp(NODE *node, char *name) {
  NODE *result;
  int i;

  if (matchName(name, node->realPath))
    return node;

  /* we are not opening any closed nodes */
  if (node->nChildren == 0) {
    return NULL;
  }

  /* now we recursively check all the children */
  for (i = 0; i < node->nChildren; i++) {
    result = findMatchingParent_rp(node->child[i], name);
    if (result != NULL)
      return result;
  }

  return NULL;
}

/******************************************************************************
 *
 * will find object in 'node' whose realpath is equal to 'name',
 * and expand necessary directories.
 *
 * This works only if the 'node' has realpath matching the beginning of
 * the 'name'
 *
 */

NODE *findExpand_rp(NODE *node, char *name) {
  int i;
  NODE *result;

  if (!matchName(name, node->realPath))
    return NULL;

  /* check whether this root is the actual match */
  if (xstrlen(name) == xstrlen(node->realPath))
    return node;

  if (node->nChildren == 0) {
    /* we are not going to open up links */
    if (node->isLink)
      return NULL;
    /* the current node must be expandable */
    node->expandable = true;
    /* expand it */
    show_extensions(node);
  }

  /* now we recursively check all the children */
  for (i = 0; i < node->nChildren; i++) {
    result = findExpand_rp(node->child[i], name);
    if (result != NULL)
      return result;
  }

  return NULL;
}

/******************************************************************************
 *
 * will expand and find the node that matches 'name' as realpath
 *
 */

static NODE *xxx(NODE *node, char *name) {
  NODE *parent;

  parent = findMatchingParent_rp(node, name);

  if (parent != NULL)
    return (findExpand_rp(parent, name));
  else
    return NULL;
}

/******************************************************************************
 *
 * position the browser on the given object
 *
 *
 */

void node_position(const char *name) {
  // parse the name of the object
  char *user_name;
  char *host_name;
  char *obj_name;
  parse_object_location(name, &user_name, &host_name, &obj_name);

  // make sure that host name of the requested object is the same
  // as the host name our browser is connected to
  if (xstrcmp(host_name, sysInfo.host_name) != 0) {
    // host names do not match --> do not do anything
    return;
  }

  // hosts are the same --> proceed

  char *realPath;
  NODE *node;

  // find the real path of the name
  if (RA::Realpath(sysInfo.connection, obj_name, realPath)) {
    return;
  }

  // now look for the node with the realpath somewhere in the tree,
  // expanding all its parents

  node = xxx(sysInfo.wholeTree, realPath);

  // we don't need the real path anymore
  xfree(realPath);

  // have we found the node?
  if (node != NULL) {
    // set the current node
    sysInfo.selNode = node;
  }
}

NODE *findClosestPath(NODE *node, char *name) {
  NODE *result;
  int i;

  if (!matchName(name, node->realPath))
    return NULL;

  /* we are not opening any closed nodes */
  if (node->nChildren == 0) {
    return node;
  }

  /* now we recursively check all the children */
  for (i = 0; i < node->nChildren; i++) {
    result = findMatchingParent_rp(node->child[i], name);
    if (result != NULL)
      return result;
  }

  return node;
}

void no_expand_search(const char *name) {
  // parse the name of the object
  char *user_name;
  char *host_name;
  char *obj_name;
  parse_object_location(name, &user_name, &host_name, &obj_name);

  // make sure that host name of the requested object is the same
  // as the host name our browser is connected to
  if (xstrcmp(host_name, sysInfo.host_name) != 0) {
    // host names do not match --> do not do anything
    return;
  }

  // hosts are the same --> proceed

  char *realPath;
  NODE *node;

  // find the real path of the name
  if (RA::Realpath(sysInfo.connection, obj_name, realPath)) {
     return;
  }

  node = findClosestPath(sysInfo.wholeTree, realPath);
  if (node != NULL) {
    if (xstrcmp(realPath, node->name) == 0)
      sysInfo.selNode = node;
  } else
    sysInfo.selNode = NULL;
  xfree(realPath);
}
