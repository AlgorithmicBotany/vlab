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
#include <QCursor>

#include "eventHandle.h"
#include "main.h"
#include "buildTree.h"
#include "comm.h"
#include "dragDrop.h"
#include "dsprintf.h"
#include "graphics.h"
#include "nodeinfo.h"
#include "openNode.h"
#include "tree.h"
#include "xstring.h"
#include "xutils.h"

/******************************************************************************
 *
 *  This callback is called when the mouse action occurs on the
 *  the name
 *
 *  Left down: select the node
 *  Left double click: hide/expand children of the node
 *  Mid down: start drag and drop
 *  Right down: hide/show icon
 */
static int nb_of_objectDiagonals = 0;

void selectNodeNameCB(BUTTON_DATA_PTR cbs, BUTTON_ACTION action) {
  NODE *node;

  node = (NODE *)cbs;
  switch (action) {
  case B1DOWN: /* left mouse button down */
    sysInfo.selNode = node;
    /* update the menus */
    sysInfo.mainForm->update_menus();
    break;
  case B1CLICK2: /* left mouse button double click */
    sysInfo.selNode = node;
    if (node->nChildren > 0) /* close all the subnodes */
      hide_extensions(node);
    else {
      // checking if connection is still alive
      if (!sysInfo.connection->check_connection()) {
        vlabxutils::infoBox(sysInfo.mainForm,
                            "Can't extend\n"
                            "Connection with raserver is down\n"
                            "Check your network connection\n",
                            "Error");
        return;
      }

      sysInfo.mainForm->setCursor(Qt::WaitCursor);
      show_extensions(node);
      sysInfo.mainForm->setCursor(Qt::ArrowCursor);
      /* centre the node on the screen, build the tree and redraw */
      centre_node(node);
    }
    /* update the menu */
    sysInfo.mainForm->update_menus();
    break;
  case B2DOWN: /* middle mouse down*/
    if (!sysInfo.connection->check_connection()) {
      vlabxutils::infoBox(sysInfo.mainForm,
                          "Can't hide/show icon\n"
                          "Connection with raserver is down\n"
                          "Check your network connection\n",
                          "Error");
      return;
    }

    sysInfo.selNode = node;
    sysInfo.mainForm->update_menus();
    sysInfo.mainForm->updateDisplay();
    startDrag(node);
    break;
  case B3DOWN:   /* right mouse button down */
  case B3CLICK2: /* right mouse double click */
    sysInfo.selNode = node;
    // checking if connection is still alive
    if (!sysInfo.connection->check_connection()) {
      vlabxutils::infoBox(sysInfo.mainForm,
                          "Can't hide/show icon\n"
                          "Connection with raserver is down\n"
                          "Check your network connection\n",
                          "Error");
      return;
    }

    if (sysInfo.selNode->iconShow == false) {
      showIcon(sysInfo.selNode);
    } else {
      /*** hide the icon ***/
      hideIcon(sysInfo.selNode);
    }
    sysInfo.mainForm->update_menus();
    build_tree();
    break;
  default:;
  }
  sysInfo.mainForm->updateDisplay();
  sysInfo.mainForm->show_status(node->name);
}

/******************************************************************************
 *
 *  This callback is called when the mouse action occurs on the
 *  box beside the textural name
 *
 *  Left down: select node
 *  Left double click: invoke object manager for vlabd
 *  Mid down: drag and drop
 *  Right click: show/hide icon
 */
void selectNodeBoxCB(BUTTON_DATA_PTR cbs, BUTTON_ACTION action) {

  std::string pwdString = sysInfo.password.toStdString();
  const char *pwd = pwdString.c_str();
  std::string loginString = sysInfo.login_name.toStdString();
  const char *login = loginString.c_str();

  NODE *node;
  char *str = NULL;

  node = (NODE *)cbs;
  switch (action) {
  case B1DOWN:
    sysInfo.selNode = node;
    /* update the menus */
    sysInfo.mainForm->update_menus();
    break;
  case B1CLICK2:
    sysInfo.selNode = node;
    if (sysInfo.selNode == NULL) {
      fprintf(stderr, "No object selected\n");
      return;
    }
    // checking if connection is still alive
    if (!sysInfo.connection->check_connection()) {
      vlabxutils::infoBox(sysInfo.mainForm,
                          "Can't open object\n"
                          "Connection with raserver is down\n"
                          "Check your network connection\n",
                          "Error");
      return;
    }
    // At this point if there is no connection anymore an assert wil be raised
    // on RA:: Access
    str = dsprintf("%s/REMOVEME", sysInfo.selNode->name);
    if (RA::Access(sysInfo.connection, str, F_OK) == 0) {
      vlabxutils::infoBox(
          sysInfo.mainForm,
          "This node is newly created and user edit is still needed.", "Error");
      xfree(str);
      return;
    }
    xfree(str);

    // prepare the full name of the object + password
    if (sysInfo.selNode->isHObj == 0) {
      if (sysInfo.connection->reconnect())
        return;

      sysInfo.vlabd->va_send_message(
          GETOBJECT, "-rootdir %s -p '%s' -posx %d -posy %d %s@%s:%s",
          sysInfo.oofs_dir, sysInfo.password == QString::null ? "" : pwd,
          sysInfo.obj_posx, sysInfo.obj_posy, login, sysInfo.host_name,
          sysInfo.selNode->name);
      sysInfo.connection->Disconnect();

    } else {
      // refresh the associated object
      std::string path = RA::lookupUUID(sysInfo.connection, sysInfo.oofs_dir_rp,
                                        node->node_info.uuid());
      if (path == "*")
        node->object_name = 0; // lookup failed
      else
        node->object_name = xstrdup(path.c_str());

      // make sure that this node is associate with an object
      if (sysInfo.selNode->object_name == NULL)
        return;

      // send the request to the vlabd - to invoke an object
      if (sysInfo.connection->reconnect())
        return;

      sysInfo.vlabd->va_send_message(
          GETOBJECT, "-rootdir %s -p '%s' -posx %d -posy %d %s@%s:%s",
          sysInfo.oofs_dir, sysInfo.password == QString::null ? "" : pwd,
          sysInfo.obj_posx, sysInfo.obj_posy, login, sysInfo.host_name,
          sysInfo.selNode->object_name);
      sysInfo.connection->Disconnect();
    }
    if (sysInfo.obj_posx < 400) {
      sysInfo.obj_posx += 20;
      sysInfo.obj_posy += 20;
    } else {
      nb_of_objectDiagonals++;
      sysInfo.obj_posx = 20;
      sysInfo.obj_posy = nb_of_objectDiagonals * 20;
    }
    break;
  case B2DOWN:
    sysInfo.selNode = node;
    sysInfo.mainForm->update_menus();
    sysInfo.mainForm->updateDisplay();
    // checking if connection is still alive
    if (!sysInfo.connection->check_connection()) {
      vlabxutils::infoBox(sysInfo.mainForm,
                          "Can't drag and drop object\n"
                          "Connection with raserver is down\n"
                          "Check your network connection\n",
                          "Error");
      return;
    }

    startDrag(node);
    break;
  case B3DOWN:
  case B3CLICK2:
    sysInfo.selNode = node;
    // checking if connection is still alive
    if (!sysInfo.connection->check_connection()) {
      vlabxutils::infoBox(sysInfo.mainForm,
                          "Can't hide/ show icon\n"
                          "Connection with raserver is down\n"
                          "Check your network connection\n",
                          "Error");
      return;
    }

    if (sysInfo.selNode->iconShow == false) {
      /*** load the picture in ***/
      showIcon(sysInfo.selNode);
    } else {
      /*** hide the icon ***/
      hideIcon(sysInfo.selNode);
    }
    /*** update the menus ***/
    sysInfo.mainForm->update_menus();
    /* rebuild the tree */
    build_tree();
    sysInfo.mainForm->updateDisplay();
    break;
  default:;
  }
  sysInfo.mainForm->updateDisplay();
  sysInfo.mainForm->show_status(node->name);
}

/******************************************************************************
 *
 *  This callback is called when the mouse action occurs on the
 *  the icon picture of a node
 *
 *  Left down: select the node
 *  Left double click: hide/expand children of the node
 *  Mid down: start drag and drop
 *  Right down: hide/show icon
 */
void selectNodeIconCB(BUTTON_DATA_PTR cbs, BUTTON_ACTION action) {
  NODE *node;

  node = (NODE *)cbs;
  switch (action) {
  case B1DOWN: /* left mouse button down */
    sysInfo.selNode = node;
    /* update the menus */
    sysInfo.mainForm->update_menus();
    break;
  case B2DOWN: /* start DRAG */
    sysInfo.selNode = node;
    sysInfo.mainForm->update_menus();
    sysInfo.mainForm->updateDisplay();
    // checking if connection is still alive
    if (!sysInfo.connection->check_connection()) {
      vlabxutils::infoBox(sysInfo.mainForm,
                          "Can't drag and drop object\n"
                          "Connection with raserver is down\n"
                          "Check your network connection\n",
                          "Error");
      return;
    }

    startDrag(node);
    break;
  case B3DOWN:   /* right mouse button down */
  case B3CLICK2: /* right mouse double click */
                 // checking if connection is still alive
    if (!sysInfo.connection->check_connection()) {
      vlabxutils::infoBox(sysInfo.mainForm,
                          "Can't hide/ show icon\n"
                          "Connection with raserver is down\n"
                          "Check your network connection\n",
                          "Error");
      return;
    }

    sysInfo.selNode = node;
    if (sysInfo.selNode->iconShow == false) {
      /*** load the picture in ***/
      showIcon(sysInfo.selNode);
    } else {
      /*** hide the icon ***/
      hideIcon(sysInfo.selNode);
    }
    /*** update the menus ***/
    sysInfo.mainForm->update_menus();
    /* rebuild the tree */
    build_tree();
    sysInfo.mainForm->updateDisplay();
    break;
  default:;
  }
  sysInfo.mainForm->updateDisplay();
  sysInfo.mainForm->show_status(node->name);
}
