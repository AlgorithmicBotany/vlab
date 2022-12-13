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



#include <QMimeData>
#include <iostream>
#include <QBitmap>
#include <QPoint>
#include <QString>
#include <sstream>
#include <string>
#include "buildTree.h"
#include "dragDrop.h"
#include "dsprintf.h"
#include "main.h"
#include "parse_object_location.h"
#include "utilities.h"
#include "xstring.h"
#include "xutils.h"
#include <QDragMoveEvent>
#include <QPixmap>
#include <QString>
#include <cassert>
#include <qpainter.h>
#include "QTGLbrowser.h"
#include "graphics.h"
#include "nodeinfo.h"
#include "qtsupport.h"
#include <QDrag>
#include <QWidget>

#include <iostream>

static NODE *sourceNode = NULL; /* to make sure we don't drop on ourselves */
static DRAGDATA *dataList[8] = {NULL};
static int dataListLen = 0;

void startDrag(NODE *node)
/******************************************************************************
 *
 * this function will start the drag operation (when the middle mouse button
 * is pressed)
 *
 */
{
  sourceNode = node;

  QString str = ConvertProc(node);
  if (str.isEmpty())
    return;

  QDrag *drag = new QDrag(sysInfo.qgl);
  QMimeData *dragObj = new QMimeData();
  dragObj->setText(str);

  // set the pixmap
  QString txt = node->baseName;
  QFont font = sysInfo.mainForm->browserSettings()
    .get(BrowserSettings::TextFont)
    .value<QFont>();
  QFontMetrics fm(font);
  int w = fm.width(txt);
  int h = fm.height();
  if (node->iconShow) {
    h += text_to_icon_distance + node->iconHeight;
    w = qMax(w, node->iconWidth);
  }
  QPixmap pix(w, h);
  QPainter p(&pix);
  p.setBackground(QBrush(QColor("#000000")));
  p.eraseRect(0, 0, w, h);
  p.setPen(QColor("#ffffff"));
  p.setFont(font);
  p.drawText(0, fm.ascent(), txt);
  if (node->iconShow) {
    p.drawPixmap(0, fm.height() + text_to_icon_distance, node->icon());
  }
  
  drag->setMimeData(dragObj);
  drag->setPixmap(pix);
  

  // start the copy (never delete dragObj, it will be deleted automatically by
  // qt)
  drag->exec();

  DragDropFinish();
}

void DragDropFinish()
/*-----------------------------------------------------------.
| DragDropFinish() - clean up after a drag and drop transfer |
`-----------------------------------------------------------*/
{

  sourceNode = NULL;
}

QString ConvertProc(NODE *node)
/******************************************************************************
 * ConverProc() -- convert the file data to the format requested
 * by the drop site.
 */
{
  char str1[8096];
  char str2[8096];
  char node_path[4096];

  // Sanity check
  if (node == NULL) {
    sourceNode = NULL;
    return QString("");
  }

  if (node->isHObj == 0) {
    // processes RA filename and VLAB object
    node_get_relative_path(node, node_path);
    QByteArray passwordData = sysInfo.password.toLatin1();
    const char *passwordChar = passwordData.constData();
    QByteArray userNameData = sysInfo.login_name.toLatin1();
    const char *userNameChar = userNameData.constData();

    sprintf(str1, "$RA_FILE_NAME %s %s@%s:%s %s", passwordChar, userNameChar,
            sysInfo.host_name, sysInfo.oofs_dir_rp, node_path);

    // format the drop 'message'
    sprintf(str2, "$VLAB_OBJECT %s %s@%s:%s %s", passwordChar, userNameChar,
            sysInfo.host_name, sysInfo.oofs_dir_rp, node->name);

    return QString(str1) + QString("|") + QString(str2);
  } else if (node->isHObj == 1) {
    QByteArray passwordData = sysInfo.password.toLatin1();
    const char *passwordChar = passwordData.constData();
    QByteArray userNameData = sysInfo.login_name.toLatin1();
    const char *userNameChar = userNameData.constData();

    sprintf(str1, "$VLAB_MOBJECT %s %s@%s:%s", passwordChar, userNameChar,
            sysInfo.host_name, node->name);

    return QString(str1);
  }

  sourceNode = NULL;
  return QString("");
}

bool HandleDropFileName(QString data)
/******************************************************************************
 *
 * start the data transfer when data is dropped in the drawing area
 *
 */
{
  int file_name = -1;
  int is_mobject = -1;
  int i;

  // if no node is selected, we cannot drop...
  if (sysInfo.selNode == NULL)
    return false;

  ParseDragData(data);

  // retrieve the data targets and search for RA_FILE_NAME
  for (i = 0; i < dataListLen; ++i) {
    if (strcmp(dataList[i]->transType, "$RA_FILE_NAME") == 0) {
      file_name = i;
      break;
    }
    if (strcmp(dataList[i]->transType, "$VLAB_MOBJECT") == 0) {
      is_mobject = i;
    }
  }

  if (is_mobject >= 0) {
    TransferProc(dataList[is_mobject]->transType, dataList[is_mobject]->data);
  } else {
    // make sure one of the targets is RA_FILE_NAME. If not, set the status
    // to failure.
    if (file_name < 0) {
      std::string str("None of these targets are supported for dropping:\n");
      for (i = 0; i < dataListLen; i++) {
        str.append(dataList[i]->transType);
        str.push_back('\n');
      }
      vlabxutils::infoBox(sysInfo.mainForm, str, "Warning");
      return false;
    } else {
      if (sysInfo.selNode->isHObj == 0)
        TransferProc(dataList[file_name]->transType, dataList[file_name]->data);
      else if (sysInfo.selNode->isHObj == 1)
        TransferProc(dataList[file_name + 1]->transType,
                     dataList[file_name + 1]->data);
    }
  }
  return true;
}

void TransferProc(char *type, char *data)
/******************************************************************************
 *
 * the data is here from the drag source, process it
 *
 */
{

  if ((strcmp(type, "$RA_FILE_NAME") != 0) &&
      (strcmp(type, "$VLAB_MOBJECT") != 0) &&
      (strcmp(type, "$VLAB_OBJECT") != 0)) {
    fprintf(stderr,
            "The drag-source application has dropped\n"
            "\n"
            "       '%s'\n"
            "\n"
            "eventhough 'RA_FILE_NAME' was requested!",
            type);
    return;
  }

  if ((sysInfo.selNode != NULL) && (sysInfo.selNode != sourceNode)) {
    if (node_operation_allowed(sysInfo.selNode, OP_DROP)) {
      if (strcmp(type, "$RA_FILE_NAME") == 0)
        copyNameIntoNode(data, sysInfo.selNode);
      else if (strcmp(type, "$VLAB_OBJECT") == 0)
        copy_object_into_node(data, sysInfo.selNode);
      else
        copy_mobject_into_node(data, sysInfo.selNode);
    } else
      vlabxutils::infoBox(
          sysInfo.mainForm,
          "You do not have permissions to 'drop' to this object!", "Warning");
  }

  sysInfo.mainForm->update_menus();
}

void copyNameIntoNode(char *i_name, NODE *node)
/*--------------------------------------------------------------.
| Copy 'name' into the node 'node', and then update 'node'      |
|                                                               |
| Format of i_name:                                             |
|                                                               |
|    <password> [[<username>@]<host_name>:]<prefix> <node_path> |
`--------------------------------------------------------------*/
{
  /*---------------.
    | parse 'i_name' |
    `---------------*/

  // extract the password from 'name'
  char *password = get_token(i_name, ' ');

  // extract full location of the database prefix (login@host:path)
  char *full_name = get_token(NULL, ' ');

  // extract the node_path
  char *node_path = get_token(NULL, ' ');

  // parse 'full_name' into 3 components:
  char *login_name;
  char *host_name;
  char *prefix;
  parse_object_location(full_name, &login_name, &host_name, &prefix);

  // prepare a full path of the source object
  char src_path[4096];
  strcpy(src_path, prefix);
  char *ptr = node_path;
  while (1) {
    if (*ptr == '\0')
      break;
    if (*ptr == '/')
      break;
    ptr++;
  }
  strcat(src_path, ptr);

  /*-------------------------------------------------------------------.
    | open a connection to the host containing the drag/drop source data |
    `-------------------------------------------------------------------*/

  RA_Connection *connection =
      RA::new_connection(host_name, login_name, password);
  if (connection == NULL) {
    char infoStr[4000];
    sprintf(infoStr, "Cannot establish a connection to %s.", host_name);
    vlabxutils::infoBox(sysInfo.mainForm, infoStr, "Error");
    return;
  }

  /*--------------------------------------------------------------------.
    | If move_links is selected, the source of the data to be dropped has |
    | to be the same as the destination, otherwise this operation does    |
    | not make sense.                                                     |
    `--------------------------------------------------------------------*/

  // [PASCAL] bug list from 2016, Jan 25th
  // this option should
  // not have any effect when moving objects between different oofs (since
  // h-links are never moved then). so we just disable temporarily the
  // move_links option.

  bool moveLinks = sysInfo.move_links;

  if (sysInfo.move_links) {

    if (strcmp(host_name, sysInfo.connection->host_name)) {
      moveLinks = false;
    }

    if (strcmp(prefix, sysInfo.oofs_dir_rp)) {
      moveLinks = false;
    }
  }

  /*------------------------.
    | archive the source data |
    `------------------------*/

  char archive_name[4096];
  sprintf(archive_name, "%s/dragdata.ar", sysInfo.paste_dir);
  unlink(archive_name);
  int res = RA::Archive_object(connection, prefix, src_path, archive_name, 0);
  // close the temporary connection
  RA::close_connection(connection);

  // check if archive operation was successful
  if (res) {
    vlabxutils::infoBox(sysInfo.mainForm, "Cannot drag/drop this object.",
                        "Error");
    return;
  }

  /*-----------------------------------.
   | paste the archive into destination |
    `-----------------------------------*/

  res = RA::Paste_object(sysInfo.connection,  // RA connection
                         sysInfo.oofs_dir_rp, // database location
                         node->name,          // destination
                         archive_name,        // name of the local archive file
                         src_path,            // the old name of the object
                         moveLinks // whether hyperlinks should follow
  );                               // the new object

  // remove the archive
  unlink(archive_name);

  if (res) {
    char reason[4096];
    switch (res) {
    case -1:
      sprintf(reason, "Write access denied.");
      break;
    case -2:
      sprintf(reason, "Object already exists.");
      break;
    case -4:
      sprintf(reason, "Cannot open local archive.");
      break;
    case -5:
      sprintf(reason, "RAserver problems (resources?).");
      break;
    case -6:
      sprintf(reason, "Internal bug. Please contact developers.");
      break;
    case -7:
      sprintf(reason, ".dbase could not be properly modified.");
      break;
    default:
      sprintf(reason, "Unknown error.");
    }

    char err_msg[4096];
    sprintf(err_msg,
            "Could not finish drop because:"
            "\n"
            "         %s\n"
            "\n"
            "Reason:\n"
            "\n"
            "         %s\n",
            RA::err_to_str(RA::error_code), reason);
    vlabxutils::infoBox(sysInfo.mainForm, err_msg, "Warning");
    return;
  }

  // send a message to all the browsers that this node has been updated
  if (sysInfo.connection->reconnect())
    return;
  QByteArray passwordData = sysInfo.password.toLatin1();
  QByteArray userNameData = sysInfo.login_name.toLatin1();
  const char *userNameChar = userNameData.constData();

  sysInfo.vlabd->va_send_message(UPDATE, "%s@%s:%s", userNameChar,
                                 sysInfo.host_name, node->name);

  sysInfo.vlabd->va_send_message(UUIDTABLECHANGED, "%s@%s:%s", userNameChar,
                                 sysInfo.host_name, sysInfo.oofs_dir);
  sysInfo.connection->Disconnect();
}

/******************************************************************************
 * dropping an object onto a hyper object...
 * this means we'll be creating a hyper object
 * This operation only works on the same oofs.
 *
 * message format:
 *
 *    <password> [[<username>@]<host_name>:]<prefix> <node_path>
 *
 */
void copy_object_into_node(char *name, NODE *node) {
  QStringList lst = QString(name).split(' ');
  if (lst.size() < 3)
    return;
  std::string password = lst[0].toStdString();
  std::string full_name = lst[1].toStdString();
  std::string node_path = lst[2].toStdString();
  QUuid uuid;

  // parse 'full name' to obtain login-, host- and object- name
  char *login_name;
  char *host_name;
  char *oofs_dir;
  parse_object_location(full_name.c_str(), &login_name, &host_name, &oofs_dir);

  // open a connection to the database of the pasted object
  RA_Connection *connection =
      RA::new_connection(host_name, login_name, password.c_str());
  if (connection == NULL) {
    fprintf(stderr,
            "copy_object_into_node(): cannot establish RA_Connection\n"
            "                         for drag/drop %s\n",
            full_name.c_str());
    return;
  }
  // make sure the object is coming from the same oofs as the destination
  if (connection->same_as(sysInfo.connection) != 0) {
    vlabxutils::infoBox(sysInfo.mainForm,
                        "Could not finish drag/drop,\n"
                        "because the browser and the hbrowser seem to\n"
                        "have different connection.");
    RA::close_connection(connection);
    return;
  }
  // now compare the realpath of both oofs
  char *rp1 = 0;
  (void)RA::Realpath(connection, oofs_dir, rp1);
  if (QString(rp1) != QString(sysInfo.oofs_dir_rp)) {
    xfree(rp1);
    RA::close_connection(connection);
    vlabxutils::infoBox(sysInfo.mainForm,
                        "Could not finish drag/drop,\n"
                        "because we don't support cross-oofs links yet.");
    return;
  }
  xfree(rp1);
  // get the UUID of the object that we are trying to paste
  uuid = RA::getUUID(connection, oofs_dir, node_path, true);

  // check whether there is a directory called 'ext' in the
  // destination directory, and if there isn't, create one
  std::string ext_dir = node->name;
  ext_dir += "/ext";
  RA::Mkdir(sysInfo.connection, ext_dir.c_str(), 0755);
  if (RA::Access(sysInfo.connection, ext_dir.c_str(), F_OK) != 0) {
    std::ostringstream out;
    out << "Cannot create 'ext' subdirectory in '" << node->name << "'\n"
        << "Drag/drop failed.";
    vlabxutils::infoBox(sysInfo.mainForm, out.str().c_str(), "Warning");
    return;
  }

  // find out whether there isn't already a directory in the destination
  // object with the same name, and if there is, determine the course of
  // action to take
  std::string baseName = getBaseName2(node_path).substr(0, 10);
  std::string newDest;
  bool success = false;
  {
    static bool first = true;
    if (first) {
      first = false;
      srand(time(0));
    }
  }
  for (int count = 0; count < 1000; count++) {
    std::ostringstream out;
    out << node->name << "/ext/" << baseName;
    if (count > 0)
      out << "_" << count;
    if (count > 5)
      out << "_" << rand(); // :) if this fails...
    newDest = out.str();
    if (RA::Access(sysInfo.connection, newDest.c_str(), F_OK) != 0) {
      success = true;
      break;
    }
  }
  if (!success) {
    vlabxutils::infoBox(sysInfo.mainForm,
                        "Object with the same name already exists!\n",
                        "Warning");
    return;
  }

  // now we can finish dropping

  // create the destination directory
  if (0 != RA::Mkdir(sysInfo.connection, newDest.c_str(), 0755)) {
    std::ostringstream out;
    out << "Cannot mkdir " << newDest;
    vlabxutils::infoBox(sysInfo.mainForm, out.str().c_str(), "Warning");
    return;
  }

  // create the 'node' file for the dropped object
  {
    std::string fname = newDest + "/node";
    NodeInfo node(sysInfo.connection, fname, uuid, "");
    if (!node.write()) {
      vlabxutils::infoBox(sysInfo.mainForm,
                          "Drop: Could not write 'node' file\n"
                          "for the dropped object.");
      return;
    }
  }

  // send a message to all hbrowsers that this node has been updated
  if (sysInfo.connection->reconnect())
    return;
  QByteArray userNameData = sysInfo.login_name.toLatin1();
  const char *userNameChar = userNameData.constData();

  sysInfo.vlabd->va_send_message(UPDATE, "%s@%s:%s", userNameChar,
                                 sysInfo.host_name, node->name);
  sysInfo.connection->Disconnect();

  // rebuild the tree
  build_tree();
  // redraw
  sysInfo.mainForm->update_menus();
  sysInfo.mainForm->updateDisplay();
}

/******************************************************************************
 *
 * - will create a hyperobject object in NODE corresponding to the data in
 *   'name'. 'name' has to come from a hbrowser, and has to contain:
 *
 *     password login_name@host_name:object_name
 *
 */

void copy_mobject_into_node(char *name, NODE *node) {
  char tmpStr[4096];
  char *baseName;
  char newDest[4096];

  // extract the password from 'name'
  char *password = get_token(name, ' ');
  // extract full name from 'name'
  char *full_name = get_token(NULL, ' ');

  // parse 'full name' to obtain login-, host- and object-name
  char *login_name;
  char *host_name;
  char *obj_name;
  parse_object_location(full_name, &login_name, &host_name, &obj_name);

  // if we are trying to drag from a different connection than our
  // current connection, then it is likely that we are trying to drag
  // from a different hofs database, which is currently not allowed
  {
    // open up a connection to the source
    RA_Connection *connection =
        RA::new_connection(host_name, login_name, password);
    if (connection == NULL) {
      vlabxutils::popupInfoBox(sysInfo.mainForm, "Error",
                               "Cannot establish a connection to the source.");
      return;
    }

    // check if the connections match
    if (sysInfo.connection->same_as(connection) != 0) {
      vlabxutils::popupInfoBox(sysInfo.mainForm, "Error",
                               "Cannot drag/drop between two hbrowsers\n"
                               "with different connections.");
      RA::close_connection(connection);
      return;
    }

    // we don't need this connection anymore
    RA::close_connection(connection);

    // get the real path of the remote object
    char *rp1 = NULL;
    (void)RA::Realpath(sysInfo.connection, obj_name, rp1);

    // get the realpath of our hofs directory
    char *rp2 = NULL;
    (void)RA::Realpath(sysInfo.connection, sysInfo.oofs_dir, rp2);

    if (rp1 == NULL || rp2 == NULL) {
      xfree(rp1);
      xfree(rp2);
      vlabxutils::infoBox(sysInfo.mainForm, "Could not finish drag/drop.");
      return;
    }

    // make sure that rp2 is a substring of rp1
    if (strncmp(rp1, rp2, xstrlen(rp2)) != 0) {
      vlabxutils::infoBox(sysInfo.mainForm,
                          "Could not finish drag/drop,\n"
                          "because it seems like you are trying\n"
                          "to drop an object from a different\n"
                          "database.");
      xfree(rp1);
      xfree(rp2);
      return;
    }
  }

  // if there isn't 'ext' directory in the destination, create one
  sprintf(tmpStr, "%s/ext", node->name);
  if (RA::Access(sysInfo.connection, tmpStr, F_OK) != 0) {
    if (RA::Mkdir(sysInfo.connection, tmpStr, 0755) != 0) {
      sprintf(tmpStr,
              "Cannot create 'ext' in '%s'\n"
              "Drag/drop failed.",
              node->name);
      vlabxutils::infoBox(sysInfo.mainForm, tmpStr, "Warning");
      return;
    }
  }

  // if there is a directory in the destination with the same name,
  // try to come up with an alternative name
  baseName = getBaseName(obj_name);
  char new_base_name[4096];
  sprintf(new_base_name, "%s", baseName);
  int count = 0;
  while (true) {
    sprintf(newDest, "%s/ext/%s", node->name, new_base_name);
    if (RA::Access(sysInfo.connection, newDest, F_OK) != 0)
      break; // the object doesn't exist, finish drag/drop

    // be creative, and generate a new name
    count++;
    if (count == 1000) { // ridiculous
      vlabxutils::popupInfoBox(sysInfo.mainForm, "Error",
                               "Could not finish drag/drop operation, because\n"
                               "could not create destination dir on try %d.\n",
                               count);
      return;
    }
    sprintf(new_base_name, "%s_%d%d%d", baseName, count / 100,
            (count / 10) % 10, (count % 10));
  }

  // create the destination directory
  sprintf(tmpStr, "%s/ext/%s", node->name, new_base_name);
  if (0 != RA::Mkdir(sysInfo.connection, tmpStr, 0755)) {
    sprintf(tmpStr, "Cannot create '%s' in '%s'", new_base_name, node->name);
    vlabxutils::infoBox(sysInfo.mainForm, tmpStr, "Warning");
    return;
  }

  // get a list of files in the source directory
  char **list;
  int n = RA::Get_dir(sysInfo.connection, obj_name, &list);
  if (n < 0) {
    vlabxutils::infoBox(sysInfo.mainForm,
                        "Could not finish drag/drop.\n"
                        "Cannot obtain a list of files in the source mobject.",
                        "Error");
    return;
  }

  // copy the files one by one from the source into destination
  for (int i = 0; i < n; i++) {
    // skip the 'ext' file (or rather 'directory')
    if (xstrcmp(list[i], "ext") == 0)
      continue;

    // copy from source to destination
    char *src = dsprintf("%s/%s", obj_name, list[i]);
    char *dst = dsprintf("%s/%s", newDest, list[i]);
    int result =
        RA::Copy_file(sysInfo.connection, src, sysInfo.connection, dst);
    xfree(src);
    xfree(dst);
    if (result != 0) {
      char *msg = dsprintf("Could not drop file %s.", list[i]);
      vlabxutils::infoBox(sysInfo.mainForm, msg, "Warning");
      xfree(msg);
    }
  }

  // create the 'node' file for the dropped object
  {
    char fname[4096];
    sprintf(fname, "%s/node", newDest);
    NodeInfo nodei(sysInfo.connection, fname);
    if (!nodei.write()) {
      vlabxutils::infoBox(sysInfo.mainForm,
                          "Drop: Could not write 'node' file\n"
                          "for the dropped object.");
      return;
    }
  }

  // send a message to all hbrowsers that this node has been updated
  if (sysInfo.connection->reconnect())
    return;
  QByteArray userNameData = sysInfo.login_name.toLatin1();
  const char *userNameChar = userNameData.constData();

  sysInfo.vlabd->va_send_message(UPDATE, "%s@%s:%s", userNameChar,
                                 sysInfo.host_name, node->name);
  sysInfo.connection->Disconnect();

  // rebuild the tree
  build_tree();
  // redraw
  sysInfo.mainForm->update_menus();
  sysInfo.mainForm->updateDisplay();
}

void ParseDragData(QString data) {
  int i, idx;
  QString substr, str;

  for (i = 0; i < 8; ++i)
    if (dataList[i]) {
      delete dataList[i];
      dataList[i] = NULL;
    }

  dataListLen = 0;
  idx = 0;
  str = data;
  while ((idx = str.indexOf('|', idx)) >= 0) {
    substr = str.left(idx);
    ParseDragSubData(dataListLen, substr);
    str = str.right(str.length() - idx - 1);
    ++dataListLen;
  }
  substr = str;
  ParseDragSubData(dataListLen, substr);
  ++dataListLen;
}

void ParseDragSubData(int idx, QString data) {
  int cnt = data.indexOf(' ', 0);

  dataList[idx] = new DRAGDATA;
  strcpy(dataList[idx]->transType, data.left(cnt).toLatin1());
  strcpy(dataList[idx]->data, data.right(data.length() - cnt - 1).toLatin1());
}
