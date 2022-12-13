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




#ifndef __MSGID_H__
#define __MSGID_H__

enum VlabD_msgid {

    ACK_REQUEST,		// client asks vlabd for ack (to know it is
				// running and to find out vlabd's version)
    ACK_RESPONSE,		// response of vlabd to the client (followed
				// by a version (e.g. 4.0)

    NEWOBJ,			// new object for browser to invoke
    UPDATE,			// update object - for browser
    RENAME,			// rename object - for browser
    DELETE,			// delete object - for browser
    REFRESHICON,                // re-draw the current object's icon, if it is visible - for browser
    OBJCUR,			// current object
    PANELSTART,			// panel start 
    REGISTER,			// list of codes we are interested in receiving
				// separated by commas
    REMOVE,			// remove process from list 
    REORDER,			// this node has just been reordered
    GETBUSY,			// display watch-cursor (suspend user input) 
    GETREADY,			// resume user input 
    ISPASTEREADY,		// does paste contain valid data ? 
    PASTEREADY,			// yes, paste does contain valid data 
    GETOBJECT,			// invoke the object manager 
    POSITIONOBJ,		// position browser on this object 
    MISPASTEREADY,		// same as ISPASTEREADY but for hbrowser 
    MPASTEREADY,		// same as PASTEREADY but for hbrowser 
    MOVELINKS,			// message for changing the flag for
				// move/keep hbrowser links 
    RESENDMOVELINKS,		// message requesting a resend of MOVEHLINKS 
    UUIDTABLECHANGED,		// UUID table has changed
    HUPDATE,			// update object - for hbrowser
    HRENAME,			// rename hyperobject - for browser
    HDELETE,			// delete object - for hbrowser
    OBJECTINFO,			// the info about the object 
    PANELVALUE,			// parameter/value pair from panel->metacard 
    LINKQUIT,
    LASTTOKEN

};

#endif
