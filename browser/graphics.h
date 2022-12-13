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




#ifndef __graphics_h__
#define __graphics_h__

class QPainter;
class BrowserSettings;
#include "tree.h"

#define MAX_ZOOM 500

extern const int box_width, box_height, box_to_text_distance, text_to_line_distance
    , horiz_distance, vert_distance, text_to_icon_distance;

extern int selectionHeight;	/* the height of a box that is drawn in
				 * the background of the node when the node
				 * is selected. */

void redraw( QPainter & p, const BrowserSettings & bset, const QRect & view );
void hideIcon ( NODE * node );
void hideAllIcons_in_tree ( NODE * root );
void loadIcon( NODE * node, int iconSize );
bool showIcon ( NODE * node );
bool refreshIcon( NODE * node );
void showAllIcons_in_tree ( NODE * root );
void centre_node ( NODE * root );
void graphics_init( );

#endif /* ifndef __graphics_h__ */

