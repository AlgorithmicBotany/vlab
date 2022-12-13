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



// UNIX includes
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <stack>

// QT includes
//#include <QFile>
//#include <QTextStream>
//#include <QStringList>

// VLAB includes
#include "fixoofs.h"
#include "nodeinfo.h"
#include "uuid.h"

// info we like to store about each hyperlink
struct HyperlinkInfo {
  std::string path;
  std::string optionalName;
  QUuid uuid;
};

typedef std::vector<HyperlinkInfo> HyperlinkInfoList;

// info we like to store about each UUID
struct UUIDinfo {
  std::string objectPath;
  std::vector<HyperlinkInfo> links;
  UUIDinfo() { objectPath = "*"; }
};

// returns a list of hyperlinks in the database
static HyperlinkInfoList scanForHyperlinks(const std::string &oofs_path) {
  // we'll assemble the result in 'list'
  HyperlinkInfoList list;

  // convenience
  std::string qoofs = oofs_path;

  // use a stack to recursively traverse through oofs
  std::stack<std::string> st;
  st.push(qoofs);
  while (!st.empty()) {
    // get the top object off the stack
    std::string qpath = st.top();
    st.pop();
    // see if the object is a hyperlink
    std::string filepathname = qpath + "/node";
    std::ifstream node;
    node.open(filepathname.c_str());
    if (node.good()) {
      // read in the information about this hyperlink (i.e. uuid & optional
      // name)
      std::string line;
      node >> line;
      /*
      char str[512];
      node.getline(str,512);
      line = std::string(str);
      */
      HyperlinkInfo info;
      if (!line.empty())
        info.uuid = QUuid(line);
      /*
      node.getline(str,512);
      line = std::string(str);
      */
      node >> line;
      if (line.empty())
        line = "";
      info.optionalName = line;
      info.path = qpath;
      // add this to the list
      list.push_back(info);
    }
    // this is broken in QT 4.6.2 & under MAC OS X 10.6.3 after fork
    //	QStringList lst = QDir( fullPath + "/ext/" ).entryList(
    //	    QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot );
    // ... we need a workaround, trusty old Unix syscalls come to the rescue
    std::list<std::string> lst =
        QDir_entryList_dirs_nosyms_nodot(qpath + "/ext/");
    std::list<std::string>::iterator it;
    for (it = lst.begin(); it != lst.end(); it++)
      st.push(qpath + "/ext/" + *it);
  } // while

  // return the compiled list of hyperlinks
  return list;
}

// returns a unique UUID (making sure it's not in usedUUIDs yet), and also
// inserting it into the usedUUIDs
static QUuid getUniqueUUID(std::set<QUuid> &usedUUIDs) {
  while (1) {
    QUuid uuid = QcreateUuid();
    if (usedUUIDs.find(uuid) != usedUUIDs.end())
      continue;
    usedUUIDs.insert(uuid);
    return uuid;
  }
}

// - reconcile oofs (getting rid of any UUID duplicates, and making sure the
// UUID table
//   has all object with UUIDs
// - contruct a lookup map: UUID --> object path, hyperlink paths:
//   - for every object/uuid in the UUID table:
//     map[uuid] = absolute object path
//   - for every hyperlink in the oofs (traverse recursively)
//     map[uuid] += hyperlink path
// - go through every entry in the map and:
//   - if entry has object, but no links, this UUID is unnecessary
//     - delete the UUID from the object
//   - if entry has object and links
//     - renumber the UUID (for the object and the links)
//   - if entry has links but no object, the links are broken
//     - report broken links
// - reconcile oofs again

std::string fixOofs(const std::string &oofs_path, bool renumber) {
  // make a QT string from the argument
  std::string qoofs = oofs_path;

  // Reconcile oofs. This will ensure we have no duplicate UUIDs,
  // and that all UUIDs are in the table
  std::string reconcileLog1 =
      uuidTableReconcile(oofs_path, oofs_path, true, true);

  // Make an empty map:
  typedef std::map<QUuid, UUIDinfo> UUIDmap;
  std::map<QUuid, UUIDinfo> uuidMap;

  // Insert all objects in oofs that have UUIDs into the map (by reading them
  // from the UUID table):
  UUIDtable uuidTable = readUUIDtable(qoofs + "/.uuids");
  for (size_t i = 0; i < uuidTable.size(); i++)
    uuidMap[uuidTable[i].uuid].objectPath = oofs_path + "/" + uuidTable[i].path;

  // Insert all hyperlinks into the map:
  HyperlinkInfoList hlinkList = scanForHyperlinks(oofs_path);
  for (size_t i = 0; i < hlinkList.size(); i++)
    uuidMap[hlinkList[i].uuid].links.push_back(hlinkList[i]);
  // Go through each UUID and decide what to do:
  // a) report broken link
  // b) delete unused UUID from an object
  // c) renumber UUID
  std::string brokenLinks = "";
  std::string deletedFiles = "";
  std::string renumberLog = "";
  std::set<QUuid> usedUUIDs;
  for (UUIDmap::iterator it = uuidMap.begin(); it != uuidMap.end(); it++)
    usedUUIDs.insert(it->first);
  for (UUIDmap::iterator it = uuidMap.begin(); it != uuidMap.end(); it++) {
    const UUIDinfo &info = it->second;
    if (info.objectPath == "*") {
      // broken links, unless this is a null UUId... (which we use for place
      // holders)
      if (!it->first.isNull()) {
        for (size_t i = 0; i < info.links.size(); i++) {
          brokenLinks += "<a href=\"pos: " + info.links[i].path + "\">" +
                         info.links[i].path + "</a>";
        }
      }
      continue;
    }
    // are there any hyperlinks pointing to this object?
    if (info.links.size() == 0) {
      // no hyperlinks pointing to this object, delete the UUID
      std::string msg = "<a href=\"pos:" + info.objectPath + "\">" +
                        info.objectPath + " </a>";
      std::string file_to_remove = info.objectPath + "/.uuid";
      if (!remove(file_to_remove.c_str()))
        msg += " <span class=error>Failed</span><br>";
      deletedFiles += msg;
      continue;
    }
    // renumber this UUID
    if (renumber) {
      QUuid newUUID = getUniqueUUID(usedUUIDs);
      std::ofstream file;
      std::string file_name = info.objectPath + "/.uuid";
      file.open(file_name.c_str(), std::fstream::trunc);
      std::string msg =
          "Assigned new UUID to <a href=\"pos: " + info.objectPath + "\">" +
          info.objectPath + "</a>";
      if (file.good()) {
        file << newUUID.toString() << "\n";
      } else {
        msg += " <span class=error>Failed</span><br>";
      }
      renumberLog += msg;
      for (size_t i = 0; i < info.links.size(); i++) {
        std::string msg =
            "  - adjusted link: <a href=\"pos: " + info.links[i].path + "\">" +
            info.links[i].path + "</a><br>";
        std::ofstream file;
        std::string file_name = info.links[i].path + "/node";
        file.open(file_name.c_str(), std::fstream::trunc);
        if (file.good()) {
          file << newUUID.toString() << "\n"
               << info.links[i].optionalName << "\n";
        } else {
          msg += " <span class=error>Failed</span><br>";
        }
        renumberLog += msg;
      }
    }
  }

  // Reconcile again
  std::string reconcileLog2 =
      uuidTableReconcile(oofs_path, oofs_path, true, true);

  // Assemble the logs:
  std::string log = "";
  log += "<html>\n";
  log += "<head>\n";
  log += "  <style type=\"text/css\">\n";
  log += "  span.error {\n";
  log += "    background-color: #f00; color: white;\n";
  log += "  }\n";
  log += "  a:link,a:hover,a:active,a:visited {\n";
  log += "    color: #00a; text-decoration: none;\n";
  log += "  }\n";
  log += "  a:hover { text-decoration: underline; }\n";
  log += "  </style>\n";
  log += "</head>\n";
  log += "<body>\n";
  log += "<h3>First UUID table sync</h3>\n";
  log += reconcileLog1 + "<br>\n";
  log += "<h3>Broken hyperlinks</h3>\n";
  log += brokenLinks + "<br>\n";
  log += "<h3>Unused UUIDs removed</h3>\n";
  log += deletedFiles + "<br>\n";
  if (renumber) {
    log += "<h3>Renumbered UUIDs</h3>\n";
    log += renumberLog + "<br>\n";
  }
  log += "<h3>Second UUID table sync</h3>\n";
  log += reconcileLog2 + "<br>\n";
  log += "</body>\n";
  log += "</html>";
  return log;
}

std::string fixOofs_string(const std::string &oofs_path, bool renumber) {
  return fixOofs(oofs_path, renumber);
}
