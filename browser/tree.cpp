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



#include <cassert>
#include <dirent.h>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <limits.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "archive.h"
#include "buildTree.h"
#include "debug.h"
#include "delete_recursive.h"
#include "dsprintf.h"
#include "font.h"
#include "graphics.h"
#include "hash.h"
#include "isLink.h"
#include "lists_same.h"
#include "log.h"
#include "main.h"
#include "nodeinfo.h"
#include "tree.h"
#include "utilities.h"
#include "xmemory.h"
#include "xstring.h"
#include "xutils.h"

void link_same_files(char *rootName);

/******************************************************************************
 *
 * tree_sort() will sort the nodes in the tree (all of them)
 *
 */

int compNodes(const void *node1, const void *node2) {
  char *comp1;
  char *comp2;
  comp1 = (*((NODE **)node1))->baseName;
  comp2 = (*((NODE **)node2))->baseName;
  if ((*((NODE **)node1))->isHObj)
    if ((*((NODE **)node1))->screenName != NULL)
      comp1 = (*((NODE **)node1))->screenName;
  if ((*((NODE **)node2))->isHObj)
    if ((*((NODE **)node2))->screenName != NULL)
      comp2 = (*((NODE **)node2))->screenName;
  return strcasecmp(comp1, comp2);
}

void tree_sort(NODE *root, bool recursive) {
  int i;

  if (root->nChildren > 1)
    qsort((void *)(root->child), root->nChildren, sizeof(NODE **), compNodes);

  for (i = 0; i < root->nChildren; i++)
    tree_sort(root->child[i], recursive);
}

/******************************************************************************
 *
 * returns the total height of the tree
 */

int get_tree_height(NODE *root) {
  set_heights_in_tree(root);
  return root->treeHeight;
}

/******************************************************************************
 *
 * returns the total width of the tree
 */

int get_tree_width(NODE *root) {
  int max, width;
  int i;

  if (root->nChildren > 0) {
    int width;

    /*** this root is opened and has some children
     ***  so to figure out the width of this subtree
     ***  we have to find the width of the widest
     ***  child ***/
    max = get_tree_width(root->child[0]);
    for (i = 1; i < root->nChildren; i++) {
      width = get_tree_width(root->child[i]);
      if (width > max)
        max = width;
    }

    width = box_width + box_to_text_distance + root->strWidth +
            text_to_line_distance + horiz_distance + max;

    if (root->iconShow)
      if (root->iconWidth > box_width + box_to_text_distance + root->strWidth) {
        width = root->iconWidth + text_to_line_distance + horiz_distance + max;
      }

    return width;
  } else {
    width = box_width + box_to_text_distance + root->strWidth;

    if (root->iconShow)
      if (root->iconWidth > box_width + box_to_text_distance + root->strWidth)
        width = root->iconWidth;

    return width;
  }
}

/******************************************************************************
 *
 * - compute the height for this tree and every subtree of this node
 *   (highly dependent on the algorithm of "draw_tree" routine in the
 *    graphics.c)
 */

void set_heights_in_tree(NODE *root) {
  int i;

  // calculate the height of all of the children of this root into compHeight
  root->childrenHeight = 0;
  if (root->nChildren > 0) {
    // otherwise the height of this subtree has to be calculated from the
    //  children of this subNODE
    root->childrenHeight = -vert_distance;
    for (i = 0; i < root->nChildren; i++) {
      set_heights_in_tree(root->child[i]);
      root->childrenHeight += vert_distance + root->child[i]->treeHeight;
    }
  }
  // compute the height of the single node in nodeHeight
  if (root->iconShow)
    root->nodeHeight =
        selectionHeight + text_to_icon_distance + root->iconHeight;
  else
    root->nodeHeight = selectionHeight;

  if (root->nodeHeight > root->childrenHeight)
    root->treeHeight = root->nodeHeight;
  else
    root->treeHeight = root->childrenHeight;
}

void tree_set_dimensions()
// calculates
{
  // recalculate the hights in the NODE
  sysInfo.treeHeight = get_tree_height(sysInfo.beginTree);
  sysInfo.treeWidth = get_tree_width(sysInfo.beginTree);
}

/******************************************************************************
 *
 * this routine will hide the extension of a node and then recalculate the
 * dimensions of the tree
 *
 */

void hide_extensions(NODE *node) {

  int i;

  if (node->nChildren == 0) /* if it is already closed, don't do */
    return;                 /* anything */

  /*** free the extensions of the node ***/
  if (node->nChildren > 0) {
    for (i = 0; i < node->nChildren; i++)
      tree_free(node->child[i]);
    node->nChildren = 0;
    xfree(node->child);
    node->child = NULL;
  }

  /*** rebuild the tree ***/
  build_tree();
}

/******************************************************************************
 *
 * this routine will show the extension of a node and then it will recalculate
 * the dimensions of the entire tree
 */

int show_extensions(NODE *node) {
  if (sysInfo.connection->reconnect_and_remain_open()) {
    vlabxutils::infoBox(
        sysInfo.mainForm,
        "Extensions can't be shown : connection to server is down.", "Warning");

    return -1;
  }

  // if it is already openeded, don't do anything
  if (node->nChildren > 0) {
    sysInfo.connection->Disconnect_remain_open_connection();
    return 0;
  }

  // read in the extensions
  get_file_tree(node);
  sysInfo.connection->Disconnect_remain_open_connection();

  // build the tree
  build_tree();
  return 0;
}

/******************************************************************************
 *
 * this routine will show all extensions of a node (recursively),
 * and then recalculate the dimensions of the entire tree
 *
 * it will not expand the 'links' though
 *
 */

int _show_all_extensions(NODE *node) {
  int i;

  // this node is not open yet, so open it
  if (node->nChildren == 0) {
    // open this node only if it is not a link
    if (node->isLink == 0)
      get_file_tree(node);
  }

  // if the node has children, then open every child
  if (node->nChildren > 0) {
    for (i = 0; i < node->nChildren; i++)
      _show_all_extensions(node->child[i]);
  }
  return 0;
}

int show_all_extensions(NODE *node) {
  if (sysInfo.connection->reconnect_and_remain_open()) {
    vlabxutils::infoBox(
        sysInfo.mainForm,
        "Extensions can't be shown : connection to server is down.", "Warning");

    return -1;
  }

  /*    HASH_TABLE * hashTable;	 hash table of node names that are already
   * the tree */
  /*     hashTable = hash_new( 40, strHash, strCompare, strDestroy); */

  int res = _show_all_extensions(node);

  sysInfo.connection->Disconnect_remain_open_connection();

  return res;

}

/******************************************************************************
 *
 * finds out whether a given operation ("op") is allowed on this node
 *
 * For renaming, deleting & cutting - we check the parent directory for
 * RWX access.
 *
 * For pasting and dropping we check the node's 'ext' directory for
 * RWX access (if the 'ext' directory does not exist, we check the
 * node's directory for the same access, since 'ext' would have to be
 * created).
 *
 *
 */

bool node_operation_allowed(NODE *node, Operation op) {
  bool result;
  char *parentDir;
  char *extDir;
  int len;

  // if the operation is 'delete' or 'rename', we check the parent directory
  if ((op == OP_DELETE) || (op == OP_RENAME) || (op == OP_CUT)) {
    // calculate the length of the path of the parent directory
    len = xstrlen(node->name) - xstrlen(node->baseName) - 1;

    if (len < 0)
      parentDir = xstrdup("/");
    else {
      parentDir = xstrdup(node->name);
      parentDir[len] = '\0';
    }
    result =
        (RA::Access(sysInfo.connection, parentDir, R_OK | W_OK | X_OK) == 0);
    xfree(parentDir);

    return result;
  }

  // for paste or drop, we check the permissions of 'ext' directory
  if ((op == OP_PASTE) || (op == OP_DROP)) {
    extDir = dsprintf("%s/ext", node->name);
    /* first we have to check whether the 'ext' directory is actually
     * there */
    if (RA::Access(sysInfo.connection, extDir, F_OK) == 0) {
      result =
          (RA::Access(sysInfo.connection, extDir, R_OK | W_OK | X_OK) == 0);
    } else {
      result = (RA::Access(sysInfo.connection, node->realPath,
                           R_OK | W_OK | X_OK) == 0);
    }
    xfree(extDir);

    return result;
  }

  return true;
}

/******************************************************************************
 *
 * this function will find out whether a node is expandable or not
 *
 */

bool is_expandable(NODE *tree) {
  char tmpStr[4096];

  sprintf(tmpStr, "%s/ext", tree->name);
  if (0 >= RA::Get_dir(sysInfo.connection, tmpStr, NULL))
    return false;
  else
    return true;
}
/******************************************************************************
 *
 * this function will find out whether an object corresponding to the
 * name is expandable or not
 *
 */

static bool is_expandable2(char *name) {
  char tmpstr[4096];

  sprintf(tmpstr, "%s/ext/.", name);
  return (0 < RA::Get_dir(sysInfo.connection, tmpstr, NULL));
}

// re-read the node file
// refresh the object we point to
// update node-> object_name and node-> screenName
static void hyperobject_update_link(NODE *node) {
  // sanity check
  if (!node)
    return;
  // get rid of the old values
  xfree(node->screenName);
  xfree(node->object_name);
  // reread the node file
  std::string nodeFname = std::string(node->name) + "/node";
  node->node_info = NodeInfo(sysInfo.connection, nodeFname.c_str());
  // lookup the corresponding object
  if (node->node_info.uuid().isNull()) {
    node->object_name = 0;
  } else {
    std::string path = RA::lookupUUID(sysInfo.connection, sysInfo.oofs_dir_rp,
                                      node->node_info.uuid());

    if (path == "*")
      node->object_name = 0; // lookup failed
    else
      node->object_name = xstrdup(path.c_str());
  }
 
  // determine screen name
  if (node->object_name) {
    // valid object
    if (node->node_info.name() == "") {
      // use name specified in node file if it exists
      node->screenName = xstrdup(getBaseName(node->object_name));
    } else {
      // if no name specified in node file, use the filesystem name :(
      node->screenName = xstrdup(node->node_info.name().c_str());
    }
  } else {
    // invalid object, but make a special case for hyperobjects with null id
    // since we use them as place-holders
    if (node->node_info.uuid().isNull() && node->node_info.name() != "") {
      node->screenName = xstrdup(node->node_info.name().c_str());
    } else {
      node->screenName = xstrdup("?????");
    }
  }
}

/******************************************************************************
 *
 * this function creates one node of a tree and assigns it the given
 * name (if the pointer is not NULL). It also calculates the pointer to the
 * baseName for this node.
 *
 * the new node realPath will be also determined, and the flag 'isLink' will
 * be set if the name is an actual link.
 */

NODE *tree_create_node_fl(char flag, const char *name, const char *rp) {

  // allocate room for the node
  NODE *node = new NODE;

  // assign the name
  node->name = xstrdup(name);
  // determine base name
  node->baseName = getBaseName(node->name);
  // assigne the real path
  node->realPath = xstrdup(rp);

  node->nChildren = 0;
  node->child = NULL;
  node->parent = NULL;
  node->childrenHeight = 0;
  node->nodeHeight = 0;
  node->treeHeight = 0;
  node->iconShow = false;
  node->iconWidth = 0;
  node->iconHeight = 0;
  node->expandable = flag & 0x02;
  node->isLink = flag & 0x01;
  node->isHObj = 0;
  node->rx = 0;
  node->ry = 0;
  node->x = 0;
  node->y = 0;
  node->upperContour = NULL;
  node->lowerContour = NULL;
  node->object_name = 0;
  node->screenName = 0;

  // if this is a hyperobject, initialize hyperobject specific information
  // otherwise it is an object and we are done
  std::string nodeFname = std::string(node->name) + "/node";
  if (RA::Access(sysInfo.connection, nodeFname.c_str(), R_OK) == 0) {
    node->isHObj = 1;

    hyperobject_update_link(node);
   }

  return node;
}

/******************************************************************************
 *
 * convenience function for tree_create_node_fl()
 */

NODE *tree_create_node(char *name) {
  // create parameters so that we can call tree_create_node_fl()
  char *realPath = NULL;
  if (RA::Realpath(sysInfo.connection, name, realPath)) {
    realPath = xstrdup(name);
  }
  // construct flag
  int flag = 0;
  if (RA::Is_link(sysInfo.connection, name))
    flag |= 0x01;
  if (is_expandable2(name))
    flag |= 0x02;
  // call the tree_create_node_fl() version
  NODE *node = tree_create_node_fl(flag, name, realPath);
  xfree(realPath); // we should switch to C++ to make this a one-liner :)
  return node;
}

/******************************************************************************
 *
 * reads in all children of this node
 *
 */

int get_file_tree(NODE *root) {
  if (root->isHObj == 2)
    return -1;

  LOG("get_file_tree( '%s')\n", root->name);
  root->nChildren = 0; // so far no children
  root->child = NULL;  // ||

  // get extensions into 'list' and their number into 'n'
  char **list = NULL;
  int n = RA::Get_extensions(sysInfo.connection, root->name, list);

  LOG("    - Get_extensions = %d\n", n);
  if (n == -2)
    return -2; // connection error
  if (n < 0)
    return -1;
  if (n == 0)
    return 0;

 
  bool hasOrdering = false;
  for (int i = 0; i < n; i++)
    if (!strncmp(list[i], ".ordering", 10)) {
      hasOrdering = true;
      break;
    }

  // allocate room for extensions
  root->nChildren = n;
  if (hasOrdering)
    root->nChildren--;
  root->child = (NODE **)xmalloc(sizeof(NODE *) * n);

  // now we add the children for object
  for (int i = 0, idx = 0; i < n; i++, idx++) {
    if (hasOrdering && !strncmp(list[i], ".ordering", 10)) {
      idx--;
      continue;
    }

    // prepare the name into name
    char name[4096];
    sprintf(name, "%s/ext/%s", root->name, list[i]);
    int len1 = xstrlen(list[i]) + 1;

    // prepare the real path into rp
    char rp[4096];
    strcpy(rp, list[i] + len1);
    int len2 = xstrlen(list[i] + len1) + 1;

    // prepare the flags
    char flag = list[i][len1 + len2];

    // create a child
    root->child[idx] = tree_create_node_fl(flag, name, rp);
    root->child[idx]->parent = root;

    // free the name from the directory 'list'
    xfree(list[i]);
  }
  // free the directory 'list'
  xfree(list);

  // sort all children alphabetically
  tree_sort(root, false);

  // Now we resort based on the .ordering file, if present
  if (hasOrdering) {
    std::string ordName = std::string(root->name) + "/ext/.ordering";
    char tmpfile[17] = "/tmp/orderXXXXXX";
    int fd;
    if (-1 != (fd = mkstemp(tmpfile))) {
      if (!RA::Fetch_file(sysInfo.connection, ordName.c_str(), tmpfile)) {
        std::vector<NODE *> redirect(root->nChildren);
        int idx = 0;
        // First, everything in the .ordering file, in order
        std::ifstream ordFile(tmpfile);
        while (!ordFile.eof()) {
          std::string name;
          getline(ordFile, name);
          if (name.empty())
            continue;
          for (int i = 0; i < root->nChildren; i++) {
            if (NULL == root->child[i])
              continue;
            else if (name == root->child[i]->baseName) {
              redirect[idx++] = root->child[i];
              root->child[i] = NULL;
              break;
            }
          }
        }
        ordFile.close();

        // Then everything that wasn't already placed.
        for (int rdx = 0; idx < root->nChildren;) {
          while (root->child[rdx] == NULL)
            rdx++;
          redirect[idx++] = root->child[rdx++];
        }

        for (idx = 0; idx < root->nChildren; idx++)
          root->child[idx] = redirect[idx];
      }
      unlink(tmpfile);
      close(fd);
    }
  }

  return 0;
}

/******************************************************************************
 *
 * this function will create a tree of the file structure under the filename
 * passed to it as argument (and recursively read in the nodes, if the
 * boolean recursive is set)
 */

NODE *tree_read_in(char *name) {
  NODE *resultTree;
  int res;

  resultTree = tree_create_node(name);
  res = get_file_tree(resultTree);

  return resultTree;
}

/******************************************************************************
 *
 * read in the new tree, set the necessarey information about it and display it
 */

void get_new_tree(char *name) {
  // when we read in a new tree, no object is selected
  sysInfo.selNode = NULL;

  // *** first empty the existing tree (if it exists) ***
  if (sysInfo.wholeTree != NULL)
    tree_free(sysInfo.wholeTree);

  if (name[0] == '\0') { // there is no TREE
    sysInfo.wholeTree = NULL;
    sysInfo.beginTree = NULL;
  } else {
    if (name != sysInfo.oofs_dir_rp)
      strcpy(sysInfo.oofs_dir_rp, name);
    // *** read in the actual TREE ***
    sysInfo.wholeTree = tree_read_in(name);
    sysInfo.beginTree = sysInfo.wholeTree;
  }

  // now enable all buttons
  enableButtons(&sysInfo.buttons);
}

/******************************************************************************
 *
 * this function will dellocate all the memory that the tree occupies
 */

void tree_free(NODE *tree) {
  int i;

  /* free the names */
  if (tree->name)
    xfree(tree->name);
  if (tree->realPath)
    xfree(tree->realPath);

  /* free the icon - if there is one */
  if (tree->iconShow)
    hideIcon(tree);

  /* free all the children if there are any */
  if (tree->nChildren > 0) {

    for (i = 0; i < tree->nChildren; i++)
      tree_free(tree->child[i]);

    xfree(tree->child);
  } else { /* if there are no children, just error check */
    assert(tree->nChildren == 0);
    assert(tree->child == NULL);
  }

  if (tree->isHObj == 1) {
    if (tree->screenName)
      xfree(tree->screenName);
    if (tree->object_name)
      xfree(tree->object_name);
  }

  delete tree;
}

/******************************************************************************
 *
 * this function will recursively change the prefix of the root, and all its
 * children (the path prefix)
 *
 */

void set_new_prefix(NODE *root, char *name) {
  char *newName;
  int i;

  newName = (char *)xmalloc(strlen(name) + strlen("/ext/") +
                            strlen(root->baseName) + 1);
  sprintf(newName, "%s/ext/%s", name, root->baseName);
  xfree(root->name);
  root->name = newName;
  root->baseName = getBaseName(root->name);

  // get a new realpath
  xfree(root->realPath);
  if (RA::Realpath(sysInfo.connection, root->name, root->realPath))
    root->realPath = xstrdup(root->name);

  for (i = 0; i < root->nChildren; i++)
    set_new_prefix(root->child[i], root->name);
}

/******************************************************************************
 *
 * this function will delete the tree and all its children
 *
 * Returns: true - on success
 *          false - on failure
 */

bool tree_delete(NODE *root) {
  int index, i;
  NODE *parent;

  parent = root->parent;

  if (root->isHObj == 0) {
    /* find the index of this root in the parent */
    for (index = 0; index < parent->nChildren; index++)
      if (parent->child[index] == root)
        break;
    assert(index < parent->nChildren);

    /*** update the data-structure of the NODE ***/

    /* free the node */
    tree_free(parent->child[index]);
    /* shift all the rest of the children upwards */
    for (i = index + 1; i < parent->nChildren; i++)
      parent->child[i - 1] = parent->child[i];
    /* reallocate memory for the parent's children */
    parent->nChildren -= 1;
    if (parent->nChildren == 0) {
      /* if the parent has no more children, make it unexpandable,
       * and free the memory for the children pointers */
      xfree(parent->child);
      parent->child = NULL;
      parent->expandable = false;
    } else {
      /* otherwise reallocate the memory for children */
      parent->child = (NODE **)xrealloc(parent->child,
                                        sizeof(NODE *) * (parent->nChildren));
    }
    /* rebuild the tree */
    build_tree();
  } else if (root->isHObj == 1) {
    // delete the whole tree
    if (RA::Deltree(sysInfo.connection, root->name)) {
      vlabxutils::infoBox(sysInfo.mainForm, "The 'delete' operation failed.",
                          "Warning");
      return false;
    }

    // update the parent node
    node_update(parent);
  }

  return true;
}

/******************************************************************************
 *
 * this function will cut the tree and all its children from the disk,
 * and then update the tree
 *
 * - implemented as recursive 'copy' & then 'delete'
 *
 */

bool tree_cut(NODE *root) {
  int index;
  char nameName[4096];
  char archiveName[4096];
  FILE *fp;

  if (root->isHObj == 1) {
    if (tree_copy(root))
      if (tree_delete(root))
        return true;
    return false;
  }

  NODE *parent = root->parent;
  // find the index of this root in the parent
  for (index = 0; index < parent->nChildren; index++)
    if (parent->child[index] == root)
      break;
  assert(index < parent->nChildren);

  // write the name of the object into a file '---FILENAME---'
  sprintf(nameName, "%s/---FILENAME---", sysInfo.paste_dir);
  fp = fopen(nameName, "w");
  fprintf(fp, "%s\n", root->baseName);
  fclose(fp);

  // recursivly copy the node into the archive in the temporary directory
  sprintf(archiveName, "%s/data.ar", sysInfo.paste_dir);

  int res = RA::Archive_object(sysInfo.connection, sysInfo.oofs_dir_rp,
                               root->name, archiveName, 1);
  if (res)
    return false;

  // now, recursively delete the node
  res = RA::Delete_object(sysInfo.connection, sysInfo.oofs_dir_rp, root->name);
  if (res)
    return false;

  // update the data-structure of the NODE
  node_update(parent);

  // update the menus
  sysInfo.pasteReady = true;
  sysInfo.pasteLinkReady = false;

  return true;
}

/******************************************************************************
 *
 * this function will copy the tree and all its children from the disk,
 * and then update the tree (it will copy recursively
 * to a temporary directory in /tmp)
 *
 */

#include "ProgressReporter.h"

bool tree_copy(NODE *root) {
  if (root->isHObj == 0) {
    // copying object
    // --------------------------------------------------

    // write the name of the object into a file '---FILENAME---'
    char nameName[4096];
    sprintf(nameName, "%s/---FILENAME---", sysInfo.paste_dir);
    FILE *fp = fopen(nameName, "w");
    fprintf(fp, "%s\n", root->baseName);
    fclose(fp);

    // archive the tree in this temporary directory
    char archiveName[4096];
    sprintf(archiveName, "%s/data.ar", sysInfo.paste_dir);

    ProgressReporter pr(&vlabxutils::setProgress, 0, 1);
    RA::setProgressReporter(&pr);
    int res = RA::Archive_object(sysInfo.connection, sysInfo.oofs_dir_rp,
                                 root->name, archiveName, 1);
    RA::setProgressReporter(NULL);
    if (res) {
      sysInfo.errorLog += RA::getErrorLog();
      return false;
    }

    // update the menus
    sysInfo.pasteReady = true;
    sysInfo.pasteLinkReady = false;
  } else {
    // copying hyperojbect...
    // --------------------------------------------------
    //        char dirName[ 4096];
    char nameName[4096];
    //		char command[ 4096];
    char archiveName[4096];
    FILE *fp;


    // check whether there is already cut/paste directory
    // and if there is, delete it
    if (access(sysInfo.paste_dir, F_OK) >= 0) {
      if (delete_recursive(sysInfo.paste_dir))
        return false;
    }
    // create the directory for cut/copy/paste in temporary
    // directory
    if (mkdir(sysInfo.paste_dir, 0755))
      return false;

    // write the name of the object into a file '---FILENAME---'
    sprintf(nameName, "%s/---FILENAME---", sysInfo.paste_dir);
    fp = fopen(nameName, "w");
    fprintf(fp, "%s\n", root->baseName);
    fclose(fp);
    // archive the tree in this temporary directory
    sprintf(archiveName, "%s/data.ar", sysInfo.paste_dir);
    if (0 != archive(sysInfo.connection, root->name, archiveName, 1)) {
      vlabxutils::infoBox(sysInfo.mainForm, "Could not copy the tree.",
                          "Warning");
      return false;
    }

    // update the menus
    sysInfo.pasteReady = true;
    sysInfo.pasteLinkReady = false;
  }

  // return success
  return true;
}

/******************************************************************************
 *
 * this function will copy the selected node into a temporary directory (but
 * not recursively)
 *
 */

bool tree_copy_node(NODE *root)
// ======================================================================
// copies the node pointed to by 'root', which means:
//   - archives the node (non-recursively) to some temporary space
// ......................................................................
{
  // are we archiving object or hyperobject?
  if (root->isHObj == 0) {
    char archiveName[4096];
    char nameName[4096];
    FILE *fp;

    // write the name of the object into a file '---FILENAME---'
    sprintf(nameName, "%s/---FILENAME---", sysInfo.paste_dir);
    fp = fopen(nameName, "w");
    fprintf(fp, "%s\n", root->baseName);
    fclose(fp);
    // archive the selected node into the temporary directory
    // under data.ar
    sprintf(archiveName, "%s/data.ar", sysInfo.paste_dir);
    int res = RA::Archive_object(sysInfo.connection, sysInfo.oofs_dir_rp,
                                 root->name, archiveName, 0);
    printf("root name = %s - archive name = %s \n", root->name, archiveName);

    // if archive failed, report error
    if (res) {
      debug_printf("tree_copy_node failed\n");
      return false;
    }

    // paste is now ready
    sysInfo.pasteReady = true;
    sysInfo.pasteLinkReady = false;
  } else {
    printf("hypercopy Node\n");
    char archiveName[4096];
    char nameName[4096];
    FILE *fp;

    // write the name of the object into a file '---FILENAME---'
    sprintf(nameName, "%s/---FILENAME---", sysInfo.paste_dir);
    fp = fopen(nameName, "w");
    fprintf(fp, "%s\n", root->baseName);
    fclose(fp);
    // archive the selected node into the temporary directory
    // under data.ar
    sprintf(archiveName, "%s/data.ar", sysInfo.paste_dir);
    // [Pascal] Correction to make sure a hyperlink may be copied from one oofs
    // to another
    int res = RA::Archive_object(sysInfo.connection, sysInfo.oofs_dir_rp,
                                 root->name, archiveName, 0);
    printf("root name = %s - archive name = %s \n", root->name, archiveName);

    if (res) {
      vlabxutils::infoBox(sysInfo.mainForm, "Could not copy the node!\n",
                          "Warning");
      return false;
    }


    // paste is now ready
    sysInfo.pasteReady = true;
    sysInfo.pasteLinkReady = false;
    return true;
  }

  // return 'success'
  return true;
}

/******************************************************************************
 * Basically all this does is filling out the sysInfo.paste_link_info structure:
 * UUID & baseName of the hyperobject represetned by <node>
 */
bool tree_hypercopy_node(NODE *node) {
  // if node is not a hyperobject (should not happen), then just return failure
  if (node->isHObj)
    return false;
  // get the UUID of this object (it will be created if it does not exist)
  sysInfo.paste_link_info.uuid =
      RA::getUUID(sysInfo.connection, sysInfo.oofs_dir_rp, node->name, true);
  // if the UUID is null, that means we were not able to create it
  if (sysInfo.paste_link_info.uuid.isNull())
    return false;
  sysInfo.paste_link_info.dirNamex = std::string(node->baseName) + "_h";
  // not sure if this is necessary but it was here in the old code
  sysInfo.pasteReady = false;
  sysInfo.pasteLinkReady = true;
  return true;
}

bool tree_rename(NODE *root, char *name)
/*-------------------------------------------------------------------.
  | this function will rename the tree (the root), and then adjust the |
  | dimensions of the tree.                                            |
  `-------------------------------------------------------------------*/
{
  if (root->isHObj == 0) {
    // create the new 'full name' in -> 'newName'
    unsigned int keep = xstrlen(root->name) - xstrlen(root->baseName);
    char *newName = (char *)xmalloc(keep + 1 + xstrlen(name));
    memcpy(newName, root->name, keep);
    memcpy(newName + keep, name, xstrlen(name) + 1);

    xfree(root->name);
    root->name = newName;
    root->baseName = getBaseName(root->name);
    root->strWidth = strWidth(root->baseName);

    int j;
    for (j = 0; j < root->nChildren; j++)
      set_new_prefix(root->child[j], root->name);

    /*** sort the tree (the parent) ***/
    tree_sort(root->parent, true);

    // build the tree
    build_tree();

    // get a new realpath
    xfree(root->realPath);
    if (RA::Realpath(sysInfo.connection, root->name, root->realPath)) {
      char msg[4096];
      sprintf(msg, "Cannot determine the location of '%s' !\n", name);
      vlabxutils::infoBox(sysInfo.mainForm, msg, "Warning");
      root->realPath = NULL;
    }

    return true;
  } else {
    node_update(root);
    /*** sort the tree (the parent) ***/
    tree_sort(root->parent, true);
    // build the tree
    build_tree();

    return true;
  }
}

/******************************************************************************
 *
 * this function will paste the subdirectory in archive
 * $VLABTMPDIR/McutCopyPaste%UID%/data.ar into the 'root'
 *
 * if an object already exists in 'root' with the same name,
 * the user is given an option to rename it
 *
 */

bool tree_paste(NODE *root) {
  // char      dirName[ 4096];
  char baseName[4096];
  char nameName[4096];
  char archiveName[4096];
  char newDest[4096];
  char destDir[4096];
  char tmpStr[4096];
  FILE *fp;
  int i, c;

  // prepare the directory name into dirName
  fprintf(stderr, "tree_paste() node baseName %s\n", root->baseName);
  fflush(stderr);

  // check whether there is  cut/paste directory
  if (access(sysInfo.paste_dir, F_OK) < 0) {
    vlabxutils::infoBox(sysInfo.mainForm, "No data to paste", "Warning");
    return false;
  }

  // find the name of the object from the file '---FILENAME---'
  sprintf(nameName, "%s/---FILENAME---", sysInfo.paste_dir);
  fp = fopen(nameName, "r");

  // this little loop is needed because we need to read in
  // also names with spaces
  //
  i = 0;
  while (1 == 1) {
    c = fgetc(fp);
    assert(c != EOF); // it should never be EOF, because there
    // shoule always be a '\n' at the end...
    if (c == '\n') // exit when read the whole name
      break;
    baseName[i] = c;
    i++;
  }
  baseName[i] = '\0';
  fclose(fp);

  // check whether there is an 'ext' directory in the
  // destination directory, and if there isn't, create one
  sprintf(tmpStr, "%s/ext", root->name);
  if (RA::Access(sysInfo.connection, tmpStr, F_OK)) {
    if (RA::Mkdir(sysInfo.connection, tmpStr, 0755)) {
      sprintf(tmpStr, "Cannot create 'ext' in '%s'", root->name);
      vlabxutils::infoBox(sysInfo.mainForm, tmpStr, "Warning");
      return false;
    }
  }

  // if there is a directory in the destination with the same name,
  // try to come up with an alternative name
  char new_base_name[4096];
  sprintf(new_base_name, "%s", baseName);
  int count = 0;
  while (true) {
    sprintf(newDest, "%s/ext/%s", root->name, new_base_name);
    if (RA::Access(sysInfo.connection, newDest, F_OK) != 0)
      break; // the object doesn't exist, finish drag/drop

    // be creative, and generate a new name
    count++;
    if (count == 1000) { // ridiculous
      vlabxutils::popupInfoBox(sysInfo.mainForm, "Error",
                               "Could not finish paste operation, because\n"
                               "could not create destination dir on try %d.\n",
                               count);
      return false;
    }
    sprintf(new_base_name, "%s_%d%d%d", baseName, count / 100,
            (count / 10) % 10, (count % 10));
  }

  // expand the archive from the temporary directory to a temporary
  // directory in the 'ext' of the destination - this is necessary
  // because if the original baseName is already used, dearchive would
  // expand the files into the object that uses that name. Therefore,
  // we expand the baseName into a temporary directory, and then rename
  // ....../ext/__tmp_dir___/baseName to ....../ext/new_base_name and
  // delete the __tmp_dir___ .
  sprintf(destDir, "%s/ext/___tmp_dir___", root->name);
  if (RA::Mkdir(sysInfo.connection, destDir, 0755)) {
    sprintf(tmpStr, "mkdir('%s:%s') failed\n", sysInfo.connection->host_name,
            destDir);
    vlabxutils::infoBox(sysInfo.mainForm, tmpStr, "Warning");
  }

  sprintf(archiveName, "%s/data.ar", sysInfo.paste_dir);
  // all browsers : become busy!
  if (sysInfo.connection->reconnect())
    return false;
  sysInfo.vlabd->send_message(GETBUSY);
  sysInfo.connection->Disconnect();

  // dearchive into destDir
  fprintf(stderr,
          "tree_paste() calling dearchive on archiveName %s into destDir %s\n",
          archiveName, destDir);
  fflush(stderr);
  if (0 != dearchive(sysInfo.connection, archiveName, destDir)) {
    // status dialog down
    // tell to all browsers that they shouldn't be busy anymore
    if (sysInfo.connection->reconnect())
      return false;
    sysInfo.vlabd->send_message(GETREADY);
    sysInfo.connection->Disconnect();

    vlabxutils::infoBox(sysInfo.mainForm, "Could not finish paste!", "Warning");

    return false;
  }

  // move the pasted file into the new_base_name
  {
    char *src = dsprintf("%s/%s", destDir, baseName);
    char *dst = dsprintf("%s/ext/%s", root->name, new_base_name);
    if (RA::Rename(sysInfo.connection, src, dst)) {
      vlabxutils::infoBox(sysInfo.mainForm, "Could not finish the paste! [2]",
                          "Warning");
      xfree(src);
      xfree(dst);
      return false;
    }
    xfree(src);
    xfree(dst);
  }

  // remove the temporary directory "___tmp_dir___"
  if (RA::Deltree(sysInfo.connection, destDir)) {
    vlabxutils::infoBox(sysInfo.mainForm,
                        "Could not remove the temp. directory\n"
                        "\t ___tmp_dir__",
                        "Warning");
  }


  // tell to all browsers that they should not be busy
  if (sysInfo.connection->reconnect())
    return false;

  sysInfo.vlabd->send_message(GETREADY);
  sysInfo.connection->Disconnect();
  // the next bool is to insure the buffer is empty ...
  sysInfo.pasteLinkReady = false;
  sysInfo.pasteReady = false;
  return true;
}

/******************************************************************************
 *
 * this function will paste the current hyperlink into the 'root'
 *
 */

void tree_hyperpaste(NODE *node) {
  char tmpStr[4096];
  char baseName[4096], newBaseName[4096];
  char destName[4096];
  // check whether we're paste-ready
  if (!sysInfo.pasteLinkReady) {
    vlabxutils::infoBox(sysInfo.mainForm, "No hyperlink to paste", "Warning");
    return;
  }

  // check whether there is an 'ext' directory in the
  // destination directory, and if there isn't, create one
  sprintf(tmpStr, "%s/ext", node->name);
  if (RA::Access(sysInfo.connection, tmpStr, F_OK)) {
    if (RA::Mkdir(sysInfo.connection, tmpStr, 0755)) {
      sprintf(tmpStr, "Cannot create 'ext' in '%s'", node->name);
      vlabxutils::infoBox(sysInfo.mainForm, tmpStr, "Warning");
      return;
    }
  }

  sprintf(baseName, "%s", sysInfo.paste_link_info.dirNamex.c_str());
  int count = 0;
  while (true) {
    sprintf(newBaseName, "%s_%d%d%d", baseName, count / 100, (count / 10) % 10,
            (count % 10));
    sprintf(destName, "%s/ext/%s", node->name, newBaseName);
    if (RA::Access(sysInfo.connection, destName, F_OK) != 0)
      break; // the object doesn't exist, finish paste

    // be creative, and generate a new name
    count++;
    if (count >= 1000) { // ridiculous
      vlabxutils::popupInfoBox(sysInfo.mainForm, "Error",
                               "Could not finish paste operation, because\n"
                               "could not create destination dir on try %d.\n",
                               count);
      return;
    }
  }

  // all browsers : become busy!
  if (sysInfo.connection->reconnect())
    return;

  sysInfo.vlabd->send_message(GETBUSY);
  sysInfo.connection->Disconnect();

  // create the required directory
  if (RA::Mkdir(sysInfo.connection, destName, 0755)) {
    sprintf(tmpStr, "mkdir('%s:%s') failed\n", sysInfo.connection->host_name,
            destName);
    vlabxutils::infoBox(sysInfo.mainForm, tmpStr, "Warning");
  }
  // write the node file
  if (!NodeInfo(sysInfo.connection, std::string(destName) + "/node",
                sysInfo.paste_link_info.uuid, "")
           .write()) {
    vlabxutils::infoBox(sysInfo.mainForm, "Could not create node file.",
                        "Warning");
  }

  // tell to all browsers that they should not be busy
  if (sysInfo.connection->reconnect())
    return;
  sysInfo.vlabd->send_message(GETREADY);
  sysInfo.connection->Disconnect();

  // the next bool is to insure the buffer is empty ...
  sysInfo.pasteLinkReady = false;
  sysInfo.pasteReady = false;
  return;
}

/******************************************************************************
 *
 * finds a node in the tree and returns a pointer to it (if it doesn't, it
 * will return NULL)
 *
 * search is performed recursively on all children, and the first matching
 * will be returned
 *
 */

NODE *findNode(NODE *node, char *name) {
  NODE *result;
  int i;

  if (strcmp(node->name, name) == 0)
    return node;

  for (i = 0; i < node->nChildren; i++) {
    result = findNode(node->child[i], name);
    if (result != NULL)
      return result;
  }

  return NULL; /* not found in this NODE */
}

/******************************************************************************
 *
 * this function will update the node 'root', (only works when the node has
 * a new child, or a node has 'lost' a child)
 *
 */

void node_update(NODE *root) {
  NODE *newRoot;
  int i, j;

  if (root == NULL)
    return;

  if (root->isHObj == 0) {
    if (root->nChildren == 0) {
      /* the node is closed, so there
       * will be no need for update, just
       * change it to expandable */
      root->expandable = is_expandable(root);
      /* build the tree */
      build_tree();

      return;
    }

    /* read the most up-to-date tree into newRoot */
    newRoot = tree_read_in(root->name);

    /* compare each child against the new child, and if the child
     * in the new tree exists also in the old tree, copy the child
     * information into the new tree */
    for (i = 0; i < newRoot->nChildren; i++) {
      for (j = 0; j < root->nChildren; j++)
        if (strcmp(root->child[j]->name, newRoot->child[i]->name) == 0)
          break;

      if (j < root->nChildren) {
        /* this node exists */
        tree_free(newRoot->child[i]);
        newRoot->child[i] = root->child[j];
      } else {
        /* node newRoot-> child[ i] doesn't exist in the
         * old tree, so fix its parent
         */
        newRoot->child[i]->parent = root;
      }
    }

    /* now we copy all the information from the new node into the
     * old one, and thusly preserving all the links in the tree
     */
    if (root->name)
      xfree(root->name); /* we won't need this (replace)*/
    root->name = newRoot->name;
    root->nChildren = newRoot->nChildren;
    if (root->child)
      xfree(root->child); /* we won't need this (replace) */
    root->child = newRoot->child;
    root->baseName = newRoot->baseName;
    root->strWidth = strWidth(root->baseName);
    root->expandable = newRoot->expandable;
    root->isLink = newRoot->isLink;
    root->isHObj = newRoot->isHObj;
  } else if (root->isHObj == 1) {
    if (root->nChildren == 0)
      newRoot = tree_create_node(root->name);
    else
      newRoot = tree_read_in(root->name);

    // for each child in the new node, if this child exists in the
    // old node, replace the new child with the old one, because the
    // old one could have been 'expanded' or have its icon shown...
    for (i = 0; i < newRoot->nChildren; i++) {
      // find the equivalent for the new child in the old node list,
      // and put its index into 'j'
      for (j = 0; j < root->nChildren; j++) {
        // skip the old children that have been already matched
        if (root->child[j] == NULL)
          continue;
        // if we found a match, exit out of the loop
        if (strcmp(root->child[j]->name, newRoot->child[i]->name) == 0)
          break;
      }

      // have we found an equivalent?
      if (j < root->nChildren) {
        // free the new child
        tree_free(newRoot->child[i]);
        // put in the old child
        newRoot->child[i] = root->child[j];
        // fix up the pointer in the old node
        root->child[j] = NULL;
      }
    }

    // delete all old children
    for (i = 0; i < root->nChildren; i++) {
      if (root->child[i] != NULL) {
        tree_free(root->child[i]);
        root->child[i] = NULL;
      }
    }
    if (root->nChildren > 0)
      xfree(root->child);

    // now we copy all the information from the new node into the
    // old one, and thusly preserving all the links in the tree

    // replace the 'name'
    if (root->name)
      xfree(root->name); // we won't need this (replace)
    root->name = newRoot->name;
    newRoot->name = NULL;

    // replace the number of children and the children themselves
    root->nChildren = newRoot->nChildren;
    newRoot->nChildren = 0;
    root->child = newRoot->child;
    newRoot->child = NULL;
    // fix the parents of all children
    for (i = 0; i < root->nChildren; i++)
      root->child[i]->parent = root;

    // copy the base name
    root->baseName = newRoot->baseName;
    newRoot->baseName = NULL;

    // copy 'realPath'
    if (root->realPath)
      xfree(root->realPath);
    root->realPath = newRoot->realPath;
    newRoot->realPath = NULL;

    // copy 'screenName'
    if (root->screenName)
      xfree(root->screenName);
    root->screenName = newRoot->screenName;
    newRoot->screenName = NULL;

    // copy 'object_name'
    if (root->object_name)
      xfree(root->object_name);
    root->object_name = newRoot->object_name;
    newRoot->object_name = NULL;

    // copy the 'node_info'
    root->node_info = newRoot->node_info;

    // copy the rest of flags...
    root->isLink = newRoot->isLink;
    root->isHObj = newRoot->isHObj;
    root->expandable = newRoot->expandable;
    root->strWidth = strWidth(root->screenName);

    // now get rid of the new node
    tree_free(newRoot);
  }

  build_tree();
}

/******************************************************************************
 *
 * finds out whether parent is actually a parent of node (distant parent?)
 *
 */

bool tree_hasParent(NODE *node, NODE *parent) {
  NODE *t;

  t = node;

  while (t != NULL) {
    if (t == parent)
      return true;
    t = t->parent;
  }

  return false;
}

/******************************************************************************
 *
 * link_same_files() - will go through all the files (regular files) in
 * the specified directory, and compare them to those of the parent node.
 * If they are the same, the file is destroyed, and replaced by the link
 * to the parent (e.g. the icon of the current object is the same as that
 * of the parent, then the icon will become a link to '../../icon')
 *
 */

void link_same_files(char *object_path) {
  if (RA::Prototype_object(sysInfo.connection, object_path))
    vlabxutils::infoBox(sysInfo.mainForm, "Prototyping failed.", "Warning");
}

/******************************************************************************
 *
 * will recursively recompute the .strWidth for every node of the root
 *
 */

void tree_set_str_widths(NODE *root) {
  int i;

  if (root->isHObj == 0)
    root->strWidth = strWidth(root->baseName);
  else
    root->strWidth = strWidth(root->screenName);

  if (root->nChildren > 0)
    for (i = 0; i < root->nChildren; i++)
      tree_set_str_widths(root->child[i]);
}

/******************************************************************************
 *
 * goes throught the whole tree, and re-reads all icons (that were displayed)
 *
 */

void tree_reread_icons(NODE *root) {
  int i;

  if (root->iconShow) {
    hideIcon(root);
    showIcon(root);
  }

  for (i = 0; i < root->nChildren; i++)
    tree_reread_icons(root->child[i]);
}

/******************************************************************************
 *
 * returns the path to the node starting with the base-name of the root
 *
 * Example: if 'lilac' is a child of 'plants', and 'plants' is a child
 *          of 'distribution', and 'distribution' is a child of 'hofs',
 *          and 'hofs' is the top of the tree, then the result of this
 *          function called on a node 'lilac' would be:
 *
 *              'hofs/ext/distribution/ext/plants/ext/lilac'
 *
 *          and it will be stored in 'buf'. (The caller has to provide
 *          memory for the result in 'buf'.
 */

void node_get_relative_path(NODE *node, char *buf) {
  sprintf(buf, "%s", node->baseName);
  NODE *n = node;

  while (n->parent != NULL) {
    n = n->parent;
    char tmp[4096];
    sprintf(tmp, "%s/ext/%s", n->baseName, buf);
    sprintf(buf, "%s", tmp);
  }
}

// this function is called when the UUID table has changed
// we need to go through all hyperlinks and update them (if necessary)
static void _tree_handleUUIDtableChange(NODE *node) {
  if (node->isHObj) {
    hyperobject_update_link(node);
    // refresh icon
    if (node->iconShow) {
      hideIcon(node);
      showIcon(node);
    }
  }
  for (int i = 0; i < node->nChildren; i++)
    _tree_handleUUIDtableChange(node->child[i]);
}

void tree_handleUUIDtableChange() {
  // read in the UUID table
  // find all hyperlinks and
  //    - if the hyperlink and UUID mismatch, re-read the hyperlink info from
  //    the downloded
  //      uuid table
  // refresh the browser's display
  if (sysInfo.wholeTree)
    _tree_handleUUIDtableChange(sysInfo.wholeTree);
}
