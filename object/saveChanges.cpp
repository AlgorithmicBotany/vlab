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



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

#include <QCursor>

#include "dirList.h"
#include "dsprintf.h"
#include "globMatch.h"
#include "object.h"
#include "saveChanges.h"
#include "xmemory.h"
#include "xstring.h"
#include "xutils.h"

// prototypes

SaveStatus save_to_storage(void);
SaveStatus save_to_lost_found(void);

// global variables

// finds out whether a string is part of a list
#include <algorithm>
template <class T>
static bool memberOf(const T &val, const std::vector<T> &list) {
  return list.end() != find(list.begin(), list.end(), val);
}

SaveStatus save_changes(void)
// ======================================================================
// - tries to save the changes to the storage
// - if successful, reurns SAVE_OK
// - otherwise, ask user what to do
//       - cancel = return SAVE_CANCEL
//       - save to LOST & FOUND
//            - attempt to save to LOST & FOUND directory
//            - if successful, return SAVE_OK
//            - otherwiser return SAVE_ERROR
// ......................................................................
{
  // try to save to storage
  SaveStatus s = save_to_storage();

  // if save was canceled or successful, return
  if (s == SAVE_OK || s == SAVE_CANCEL)
    return s;

  // otherwise, ask user what to do
  auto button = QMessageBox::warning(nullptr, "Object",
      "<qt>"
      "<b>Saving unsuccessful!</b><p>"
      "Save in lost_found?",
      QMessageBox::Ok | QMessageBox::Cancel,
      QMessageBox::Cancel);

  if (QMessageBox::Ok == button) {
    return save_to_lost_found();
  } else {
    return SAVE_CANCEL;
  }
}

SaveStatus save_to_storage(void)
// ======================================================================
// - this will try to save changes to the storage
// - it is assumed that the user has been already warned about discrepancies,
//   so only files that are on the table are attempted to be copied
// - if the changes will affect the children, the user is warned
// - if he decides to cance, SAVE_CANCEL is returned
// - if he decides to continue, the save is attempted
// - if save is successful, the function returns SAVE_OK, otherwise SAVE_ERROR
// ......................................................................
{
  // would saving affect extensions ?
  std::string extDir = obj.objDir + "/ext/.";
  if (RA::Get_dir(obj.connection, extDir.c_str(), NULL) > 0) {
    // object has extensions - warn the user
    QMessageBox msgBox;
     msgBox.setText("Saving changes could affect extensions.\n"
                   "Save anyway?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    msgBox.setIcon(QMessageBox::Warning);
    int ret = msgBox.exec();
    if (ret == QMessageBox::No)
      return SAVE_CANCEL;
  }
  // start drawing progress in object window
  iconForm->progress().show(true,obj.getSaveAlways);

  // get the list of files on the table
  FileList tableList = dirList2(obj.tmpDir);

  // attempt to copy only files in both specs and table
  std::string errLog;
  iconForm->progress().setup(obj.fnames.size(), 1, 0);
  for (size_t i = 0; i < obj.fnames.size(); i++) {
    // skip the objects that are not on the lab table
    if (!memberOf(obj.fnames[i], tableList))
      continue;

    // prepare full pathnames for source & destination
    std::string src = obj.tmpDir + "/" + obj.fnames[i];
    std::string dst = obj.objDir + "/" + obj.fnames[i];
    if (RA::Copy_file(obj.local_connection, src.c_str(), obj.connection,
                      dst.c_str())) {
      errLog += "File " + obj.fnames[i] + " was not saved.\n";
    }
    iconForm->progress().advance();
  }
  // turn off progress indicator
  iconForm->progress().show(false,obj.getSaveAlways);

  if (!errLog.empty()) {
    return SAVE_ERROR;
  }

  if (RA::Prototype_object(obj.connection, obj.objDir.c_str())) {
    return SAVE_ERROR;
  }
  // everything must have went well
  return SAVE_OK;
}

/******************************************************************************
 *
 * will save the object to the lost/found directory
 *
 *
 */

SaveStatus save_to_lost_found(void) {
  int i;
  char lost_found_dir[4096];
  sprintf(lost_found_dir, "%s/ext/lost_found", obj.rootDir.c_str());

  // first make sure that lost/found directory is ready for us...
  if (RA::Access(obj.connection, lost_found_dir, R_OK | W_OK | X_OK)) {
    // *** try to create lost/found directory ***

    // first make sure 'ext' exists in root directory
    char tmp[4096];
    sprintf(tmp, "%s/ext", obj.rootDir.c_str());
    RA::Mkdir(obj.connection, tmp, 0755);

    // now create lost/found
    RA::Mkdir(obj.connection, lost_found_dir, 0755);

    // and check again if lost/found exists
    if (RA::Access(obj.connection, lost_found_dir, R_OK | W_OK | X_OK)) {
      static char msg[4096];
      sprintf(msg,
              "You don't have access to lost_found "
              "directory:\n"
              "\t%s\n",
              lost_found_dir);
      return SAVE_ERROR;
    }
    // send message telling browsers that lost/found directory
    // was created (so that it can update root)
    vlabd->va_send_message(UPDATE, "%s@%s:%s", obj.connection->login_name,
                           obj.connection->host_name, obj.rootDir.c_str());
  }

  // make sure 'ext' directory is in lost/found
  char *ext = dsprintf("%s/ext", lost_found_dir);
  RA::Mkdir(obj.connection, ext, 0755);
  if (RA::Access(obj.connection, ext, R_OK | W_OK | X_OK)) {
    static char msg[4096];
    sprintf(msg,
            "object: You don't have access to ext in "
            "lost_found directory:\n"
            "\t%s\n",
            lost_found_dir);
    xfree(ext);
    return SAVE_ERROR;
  }

  // make sure we would not be overwriting anything in lost/found
  // directory
  char *dst_dir = dsprintf("%s/%s", ext, obj.objName.c_str());

  // 10 attempts to create unique name
  for (i = 0; i < 10; i++) {
    // check if this file name exists
    if (RA::Access(obj.connection, dst_dir, F_OK)) {
      // this filename doesn't exist, so we can break out
      // of this loop
      break;
    }

    // this filename exists, so try a different filename
    xfree(dst_dir);
    dst_dir = dsprintf("%s/%s%d", ext, obj.objName.c_str(), i);
  }
  // we don't need ext anymore
  xfree(ext);

  // have we successfuly created a unique filename?
  if (i == 10) {
    // no, we could not create a unique filename
    static char msg[4096];
    sprintf(msg,
            "object: 10 attempts failed to create a unique "
            "filename for saving in\n"
            "        the lost_found directory %s/ext/%s\n"
            "\n",
            lost_found_dir, obj.objName.c_str());
    return SAVE_ERROR;
  }

  // now create the direcotory
  if (RA::Mkdir(obj.connection, dst_dir, 0755)) {
    // no, we could not create a directory
    static char msg[4096];
    sprintf(msg,
            "object: Could not create a directory for this object\n"
            "        in the lost_found directory %s/ext/%s",
            lost_found_dir, obj.objName.c_str());
    return SAVE_ERROR;
  }

  // get a list of files on the lab table
  char **list;
  int n = dirList(obj.tmpDir.c_str(), &list);

  // now try to save each of them
  int success = 1;
  for (i = 0; i < n; i++) {
    char *src = dsprintf("%s/%s", obj.tmpDir.c_str(), list[i]);
    char *dst = dsprintf("%s/%s", dst_dir, list[i]);

    if (RA::Put_file(src, obj.connection, dst)) {
      char *msg =
          dsprintf("Could not save '%s' into lost_found directory.", list[i]);
      vlabxutils::infoBox(iconForm, msg, "Warning");
      xfree(msg);
      success = 0;
    }

    xfree(dst);
    xfree(src);
  }

  // return result of saving
  if (success) {
    // send message telling browsers that lost/found directory
    // was updated
    vlabd->va_send_message(UPDATE, "%s@%s:%s", obj.connection->login_name,
                           obj.connection->host_name, lost_found_dir);

    // report to user:
    char *msg = dsprintf("Object saved into:\n\n     %s.\n", dst_dir);
    xfree(msg);

    return SAVE_OK;
  } else {
    return SAVE_ERROR;
  }
}

void getChanges(FileList &modified, FileList &unknown, FileList &missing)
// ======================================================================
// Returns 3 lists:
//   modified = contains files that are both on table and in specs, but
//              have been modified
//   unknown  = contains files that are on the table but not in specs
//              (with files to be ignored removed)
//   missing  = files that are in specs, but not on table
// ......................................................................
{
  iconForm->progress().show(true,false);
  
  // reset the 3 lists
  modified.clear();
  unknown.clear();
  missing.clear();

  // get a list of files on the table
  FileList tableList = dirList2(obj.tmpDir);

  // for convenience, name the list of files in specs
  const FileList &specsList = obj.fnames;

  // and the ignore files
  const FileList &ignoreList = obj.fnamesIgnore;

  // for every file in the specs, find out if it has been modified
  // or if it is missing
  iconForm->progress().setup(specsList.size(), 1, 0);
  for (size_t i = 0; i < specsList.size(); i++) {
    // is this file on the table at all?
    if (!memberOf(specsList[i], tableList)) {
      missing.push_back(specsList[i]);
      continue;
    }
    // compare the files
    std::string storageFname = obj.objDir + "/" + specsList[i];
    std::string tableFname = obj.tmpDir + "/" + specsList[i];

    if (RA::Compare_files(obj.connection, storageFname.c_str(),
                          obj.local_connection, tableFname.c_str())) {
      // the files are different
      modified.push_back(specsList[i]);
    }
    iconForm->progress().advance();
  }
  // if both icon and icon.png are missing we are missing an icon, just keep one
  // if only one is missing keep none
  // [PASCAL] uncomment the following to add icon.png support
  /*
  int missingIconPos = -1;
  int missingIconPNGPos = -1;
  for (size_t i = 0; i < missing.size(); ++i){
    if (missing[i].compare("icon") == 0)
      missingIconPos = i;
    if (missing[i].compare("icon.png") == 0)
      missingIconPNGPos = i;
  }
  if (missingIconPos >=0){
    // icon has been deleted make sure we have iconPNG
    if (missingIconPNGPos < 0){
      // there is an icon.png in the object, delete icon from missing files
      missing.erase(missing.begin() + missingIconPos);
    }
  }
  else{
    if (missingIconPNGPos >= 0){
      // there is an icon.png in the object, delete icon from missing files
        missing.erase(missing.begin() + missingIconPNGPos);
    }
  }
  */

  // for every file on the lab table, make sure it is either in
  // specsList, or it matches something in ignoreList
  for (size_t i = 0; i < tableList.size(); i++) {

    if (memberOf(tableList[i], specsList))
      continue;
    bool ignored = false;
    for (size_t j = 0; j < ignoreList.size(); j++) {
      if (globMatch(ignoreList[j], tableList[i])) {
        ignored = true;
        break;
      }
    }
    if (!ignored)
      unknown.push_back(tableList[i]);
  }
  iconForm->progress().show(false,false);
  return;
}
