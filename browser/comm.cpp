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
#include <netdb.h>
#include <netinet/in.h>
#include <qcursor.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "Mem.h"
#include "buildTree.h"
#include "comm.h"
#include "debug.h"
#include "graphics.h"
#include "main.h"
#include "memory.h"
#include "openNode.h"
#include "parse_object_location.h"
#include "qtsupport.h"
#include "tree.h"
#include "xmemory.h"
#include "xstring.h"
#include "xutils.h"

/******************************************************************************
 *
 * process the message
 *
 */

static void processMessage(int code, char *message) {
  NODE *node;
  unsigned int i = 0;
  char *nodeName = NULL;
  char *newName = NULL;
  char *login_name = NULL; // used for parsing object's name
  char *host_name = NULL;  //    -//-
  char *obj_name = NULL;   //    -//-
  if (code == PASTEREADY) {
    /* paste ready (somebody made some data ready*/
    sysInfo.pasteReady = true;

    // regenerate new paste_info
    delete sysInfo.paste_info;
    sysInfo.paste_info = new PasteInfo;

    // extract information from the message. Message format:
    //
    //    <password> [[<username>@]<host_name>:]<prefix>
    //         <node_path>
    debug_printf("PASTEREADY: message = '%s'\n", message);

    // extract the password from 'message'
    sysInfo.paste_info->user_password = get_token(message, ' ');

    // extract the full location of the database prefix
    // format: login@host:path
    char *full_name = get_token(NULL, ' ');
    // parse 'full_name' into 3 components:
    char *c_login_name = NULL;
    parse_object_location(full_name, &c_login_name,
                          &sysInfo.paste_info->host_name,
                          &sysInfo.paste_info->prefix);
    // [Pascal] I don't understand why we do have to update the login_name (that
    // cannot be changed afterwar I comment but we may have some side effect ...
    // sysInfo.login_name = c_login_name;
    // extract the node_path
    sysInfo.paste_info->node_path = get_token(NULL, ' ');

    sysInfo.mainForm->update_menus();
    sysInfo.mainForm->updateDisplay();
  } else if (code == ISPASTEREADY) {
    /* ask if there is something to paste? */
    if (sysInfo.pasteReady) {
      if (sysInfo.connection->reconnect())
        return;
      QByteArray user_passwordData =
          sysInfo.paste_info->user_password.toLatin1();
      const char *user_passwordChar = user_passwordData.constData();
      QByteArray userNameData = sysInfo.paste_info->user_name.toLatin1();
      const char *userNameChar = userNameData.constData();
      sysInfo.vlabd->va_send_message(
          PASTEREADY, "%s %s@%s:%s %s", user_passwordChar, userNameChar,
          sysInfo.paste_info->host_name, sysInfo.paste_info->prefix,
          sysInfo.paste_info->node_path);
      sysInfo.connection->Disconnect();
    }
  } else if (code == GETBUSY) {
    sysInfo.mainForm->setCursor(Qt::WaitCursor);
  } else if (code == GETREADY) {
    sysInfo.mainForm->setCursor(Qt::ArrowCursor);
  } else if (code == RENAME) {
    for (i = 0; i < strlen(message); i++)
      if (message[i] == ',')
        break;
    if (i == strlen(message)) {
      vlabxutils::infoBox(sysInfo.mainForm,
                          "Received a RENAME(13) message without a comma!\n",
                          "Warning");
      return;
    }
    message[i] = '\0';
    nodeName = message;
    if (*nodeName == '\0') {
      vlabxutils::infoBox(sysInfo.mainForm,
                          "comm.c:Cannot rename this object, "
                          "because it doesn't exist!\n",
                          "Warning");
      return;
    }
    newName = message + i + 1;

    // parse the name of the object
    parse_object_location(nodeName, &login_name, &host_name, &obj_name);

    // make sure we are connected to the same host as the message
    // specifies
    if (xstrcmp(host_name, sysInfo.host_name) != 0)
      return;

    // look for the node
    node = findNode(sysInfo.wholeTree, obj_name);
    if (node == NULL)
      return; /* we are not displaying this node,
               * so don't worry about it */
    /* rename the node without attempting to make the
     * changes to the filesystem
     */
    tree_rename(node, newName);
    sysInfo.mainForm->update_menus();
    sysInfo.mainForm->updateDisplay();
  } else if (code == RESENDMOVELINKS) {
    static int first_time = 1;
    if (first_time)
      first_time = 0;
    else {
      if (sysInfo.connection->reconnect())
        return;

      sysInfo.vlabd->send_message(MOVELINKS,
                                  (char *)(sysInfo.move_links ? "1" : "0"));
      sysInfo.connection->Disconnect();
    }

  } else if (code == MOVELINKS) {
    sysInfo.move_links = (xstrcmp(message, "1") == 0);
    sysInfo.mainForm->update_menus();
  } else if (code == UPDATE) {
    /* update object */
    openNode_realPath(message);
    build_tree();
    sysInfo.mainForm->update_menus();
    sysInfo.mainForm->updateDisplay();
  } else if (code == REFRESHICON) {
    // no_expand_search(message);
    node_position(message);
    build_tree();
    sysInfo.mainForm->update_menus();
    sysInfo.mainForm->updateDisplay();
    if (sysInfo.selNode != NULL) {
      if (sysInfo.selNode->iconShow) {
        // fprintf(stderr,"Refreshing icon on %s\n", sysInfo.selNode->name);
        refreshIcon(sysInfo.selNode);
      }
    }
  } else if (code == POSITIONOBJ) {
    /* position browser on this object */
    node_position(message);
    build_tree();

    sysInfo.mainForm->update_menus();
    if (sysInfo.selNode != NULL) {
      centre_node(sysInfo.selNode);
      sysInfo.mainForm->show_status(sysInfo.selNode->name);
    }
    sysInfo.mainForm->updateDisplay();

    /////////////////////////////////////
  } else if (code == DELETE) {
    // parse the name of the object
    parse_object_location(message, &login_name, &host_name, &obj_name);

    // make sure we are connected to the same host as the message
    // specifies
    if (xstrcmp(host_name, sysInfo.host_name) != 0)
      return;

    node = findNode(sysInfo.wholeTree, obj_name);
    if (node != NULL) {
      // delete the node without attempting to make the
      // changes to the filesystem

      if (tree_hasParent(sysInfo.selNode, node))
        sysInfo.selNode = NULL;
      if (tree_hasParent(sysInfo.beginTree, node))
        sysInfo.beginTree = node->parent;
      if (tree_hasParent(sysInfo.wholeTree, node))
        sysInfo.wholeTree = node->parent;
      if (node->parent != NULL)
        node_update(node->parent);
      // tree_delete( node);
    } else {
      // The node that was deleted is not being displayed, but maybe
      // its parent is. Then it would need to be updated.

      // figure out the name of the parent (by removing everything
      // after the second last slash from the obj_name
      char parent_str[4096];
      strcpy(parent_str, obj_name);
      char *p = strrchr(parent_str, '/');
      if (p == NULL)
        return;
      *p = '\0';
      p = strrchr(parent_str, '/');
      if (p == NULL)
        return;
      *p = '\0';

      // let's see if we are displaying this node
      node = findNode(sysInfo.wholeTree, parent_str);
      if (node == NULL)
        return;

      // we are displaying such parent - update it
      node_update(node);
    }
    sysInfo.mainForm->update_menus();
    sysInfo.mainForm->updateDisplay();
  } else if (code == UUIDTABLECHANGED) {
    tree_handleUUIDtableChange();
    build_tree();
    sysInfo.mainForm->update_menus();
    sysInfo.mainForm->updateDisplay();
  } else if (code == HRENAME) {
    // TBD - implement handling of HRENAME messages (when hyperlinks are
    // renamed)
  } else {
    printf("I cannot handle this message yet\n");
  }
}

void handleMessages(void) {
  long code;
  char *msg;
  long length;

  if (sysInfo.vlabd->has_data()) {
    if (sysInfo.vlabd->get_message(code, msg, length)) {
      // we have a problem with the socket - restart the
      // connection
      fprintf(stderr, "browser: connection to vlabd died. "
                      "Trying to\n"
                      "         restart vlabd ...\n");

      delete sysInfo.vlabd;
      sysInfo.vlabd = NULL;

      // try to reopen the connection
      if (vlab_open())
        fprintf(stderr, "browser: connection to vlabd "
                        "could not be \n"
                        "         restarted. You are now "
                        "without a connection\n");
      else
        fprintf(stderr, "browser: connection successfuly "
                        "restarted\n");
    } else {
      // put a \0 at the end of the message
      Mem tmp(length + 1);
      tmp.append(msg, length);
      if (msg)
        xfree(msg);
      tmp.append("\0", 1);

      // process the message
      processMessage(code, (char *)tmp.data);
    }
  }
}
