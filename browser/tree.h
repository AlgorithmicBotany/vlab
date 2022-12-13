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




#ifndef __tree_h__
#define __tree_h__

#include <QPoint>
#include <QSize>
#include <QRect>
#include <QPixmap>
#include "nodeinfo.h"

class RA_Connection;

/* types for the contour - see layout.c for more explanation */

typedef enum { OP_RENAME,
	       OP_DELETE,
	       OP_CUT,
	       OP_COPY,
	       OP_PASTE,
	       OP_DRAG,
	       OP_DROP } Operation;

struct POLYLINE;

/* type for a node */

class NODE
{
public:
    // constructor
    NODE();
    // geometry information
    // --------------------

    // return the top left corner of the node
    QPoint getTopLeft() const;
    // rerurns the size of the node itself (not including children)
    QSize getNodeSize() const;
    // rerurns the bounding rectangle of the node
    QRect getNodeRect() const;
    // returns the bounding rectangle of the tree starting at the node
    const QRect & getTreeRect() const;

    RA_Connection * connection;   // RA connection to the object

    char *     realPath;        // realpath of this node
    char *     name;            // name of the node (this is actually the full path to the node)
    char *     baseName;        // base name of this node (which is actually only a pointer into
                                // the 'name' )
    char *     screenName;      // name appearing on the screen
    int        nChildren;       // number of children
    NODE **    child;           // pointers to all children
    NODE *     parent;          // pointer to the parent
    int        childrenHeight;  // the height of all the children
    int        nodeHeight;      // the height of this node
    int        nodeWidth;       // the width of this node
    int        treeWidth;       // the width of this subtree
    int        treeHeight;      // the height of this subtree
    int        strWidth;        // the width of the string in pixels
    bool       iconShow;        // is the icon visible ?
    int	       iconWidth;       // the dimensions of the icon
    int	       iconHeight;      //
    int	       expandable;      // is this directory expandable?
    int	       isLink;          // is this node a link?
    int        isHObj;          // 0 = reg. object, 1 = hyperobject
    char *     object_name;     // name to object
    NodeInfo   node_info;       // Info on node file
    QPixmap    _icon;           // here we store the icon
    const QPixmap & icon();     // get the node's icon
    bool       isIconLoading(); // returns whether icon is loading or not
    bool       _isIconLoading;  // whether icon is being loaded or not

    // layout of the node
    long		 x, y;	       /* node's left upper corner (x, y)*/
    long                 rx, ry;       /* node's relative (x,y) to parent */
    // bounding rectangle for the whole subtree:
    QRect _treeRect;

    POLYLINE		 * upperContour; /* the upper contour of the subtree */
    POLYLINE             * lowerContour; /* the lower contour of the subtree */
};

void tree_print( NODE * root);
void tree_free( NODE * tree);
int show_extensions( NODE * node);
void hide_extensions( NODE * node);
int show_all_extensions( NODE * node);
int get_tree_width( NODE *);
int get_tree_height( NODE *);
void set_heights_in_tree( NODE * root);
void tree_set_dimensions();
bool tree_rename( NODE * root, char * name);
bool tree_delete( NODE * root);
bool tree_cut( NODE * root);
bool tree_copy( NODE * root);
bool tree_copy_node( NODE * root);
bool tree_hypercopy_node( NODE * root);
bool tree_paste( NODE * root );
void tree_hyperpaste( NODE * root );
int get_file_tree(NODE * root);
NODE * findNode( NODE * node, char * name);
void node_update( NODE * root);
bool tree_hasParent( NODE * node, NODE * parent);
void tree_sort( NODE * root, int recursive);
void get_new_tree( char * name);
void tree_set_str_widths( NODE * root);
void tree_reread_icons( NODE * root);
bool is_expandable( NODE * node);
bool node_operation_allowed( NODE * node, Operation op);
void link_same_files( char * rootName);
void node_get_relative_path( NODE * node, char * path);
void tree_handleUUIDtableChange();

#endif /* ifndef __tree_h__ */

