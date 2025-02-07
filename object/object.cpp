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



/*
  The object is found in 'objdir'.  Its files are copied to
  TEMPDIR/VLxxxxxx *(the lab table), where they can be manipulated without
  affecting the original.

  The tool list is given in the object's 'specifications' file, and
  contains actual tool descriptions(command lines) and references to tools
  in the generic 'tools' file. The 'tools' file is in
  TEMPDIR/VLnnntools(where 'nnn' * is the uid) - it is linked to there by
  the lab manager.
*/

/* includes */
#include <iostream>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QPixmap>
#include <QSettings>
#include <QTimer>
#include <cassert>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <QLabel>
#include <QMessageBox>
#include <QTimer>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

/* Vlab includes */
#include "Loader.h"
#include "MessageBox.h"
#include "QTask_login.h"
#include "delete_recursive.h"
#include "dsprintf.h"
#include "labutil.h"
#include "object.h"
#include "parse_object_location.h"
#include "platform.h"
#include "sgiFormat.h"
#include "util.h"
#include "utilities.h"
#include "xmemory.h"
#include "xstring.h"

#include "mainwindow.h"
#ifdef __APPLE__
#include <errno.h>
#include <sys/sysctl.h>
#endif

#include <QtPlugin>

/* global variables */

typedef struct def_struct {
  char *name;
  char *val;
} Def;

struct object obj;
static struct gtools gtlist[MAXTOOLS];
static struct spec Specs[MAXTOOLS];
static int NextSpec;
static int nDefs; // number of definitions
static Def *def;  // the definitions
const char *objShell = "xterm";
const char *objEd = "jot";
static std::string object;   // the full object location
static std::string password; // password

ObjectLocation objectLocation;

QApplication *qapp = NULL;
QTGLObject *iconForm = NULL;

MainWindow *wnd;

// static QLabel        * waitLabel = NULL;

static char *line; /* current line in specification file */

VlabD *vlabd = 0; // connection to VLAB daemon

// this is to tell g++ to shut up about some string literal warnings
void qqWarning(const QString &s) { qWarning("%s", qPrintable(s)); }

// Print out usage and exit with status code
static void usage(int argc, char **argv, int status = -1)
// ======================================================================
{
  std::cerr << "Usage: object [-p password] [-rootdir root_dirname] [-posx "
               "posx] [-posy posy] <object location>\n"
            << "  where <object location> is either [[login@]host:]objdir\n"
            << "        or '.' for an object in the current directory\n"
            << "  - the offending command line was:\n"
            << "       ";
  for (int i = 0; i < argc; i++)
    std::cerr << std::string(argv[i]) << " ";
  std::cerr << "\n";
  exit(status);
}

void fatalError(const std::string &str) {
  QMessageBox::critical(iconForm, "Fatal error!!!", str.c_str());
  if (obj.tmpDir != "")
    RemoveTemp();
  kill(0, SIGKILL);
  ::exit(-1);
}

void silentKill() {
  kill(0, SIGKILL);
  ::exit(-1);
}

void warningMessage(const std::string &str) {
  QMessageBox::warning(iconForm, "Warning", str.c_str());
}
int messageBox(QString inMessage, int numFiles, QStringList files) {
  QMessageBox msgBox;
  msgBox.setText(inMessage);
  if (numFiles > 1){
    msgBox.setInformativeText(
        "<big>There are " + QString::number(numFiles) +
        " unloaded files in this object\nDo you want to load all of them?</big>");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
  }
  else if (numFiles == 1){
    msgBox.setInformativeText(
        "There is one unloaded file in this object\nDo you want to load it?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
  }
  else{
    msgBox.setText(
       "An error loading the object has occurred. Please try again.");
    msgBox.setStandardButtons(QMessageBox::Ok);
  }
  if (files.size() > 0) {
    QString detail = "";
    for (int i = 0; i < files.size(); i++) {
      detail += files.at(i) + QString("\n");
    }
    msgBox.setDetailedText(detail);
  }
  return msgBox.exec();
}
static char *read_word(char *line, char *result)
// ======================================================================
// read word from the str(the word has to be at the beginning of 'line')
// and store it in 'result'. If 'result' is NULL, a copy of the word is
// created using xmalloc (), and returned as result.
// ......................................................................
{
  int i;

  /* sanity check */
  if (line == NULL)
    return NULL;

  /* if the string starts with a white space, or is empty, return NULL */
  if (isspace(*line) || (*line == '\0'))
    return NULL;

  /* move the i to the first white space or end of string */
  i = 1;
  while ((!isspace(line[i])) && (line[i] != '\0'))
    i++;

  /* if result is NULL, create buffer for the result */
  if (result == NULL)
    result = (char *)xmalloc(sizeof(char) * (i + 1));

  /* now copy the word from the beginning of the string to the result */
  memcpy(result, line, i);
  result[i] = '\0';

  return result;
}

static char *lookup_def(char *word)
// ======================================================================
// will lookup the definition of the word in the def[] and return its
// value
// ......................................................................
{
  int i;

  for (i = 0; i < nDefs; i++)
    if (strcmp(word, def[i].name) == 0)
      return def[i].val;

  return 0;
}

static char *replace_defs(char *line)
// ======================================================================
// will replace all occurences of 'OBJED', 'OBJDEMOED', 'OBJSHELL' with the
// strings defined in $VLABCONFIGDIR/object(or .../tools)
// ......................................................................
{
  char result[4096];
  int resultPos;
  int i;
  char oldWord[4096];
  char *newWord;

  // basic insanity check
  if (line == NULL)
    return NULL;

  // set the positions in result and input
  resultPos = 0;
  i = 0;

  // scan the line character by character
  while (line[i] != '\0') {
    // skip all the white space
    while (isspace(line[i]))
      result[resultPos++] = line[i++];
    // do we have a 'word'
    if (line[i] != '\0') {
      // scan in the word
      if (NULL == read_word(line + i, oldWord))
        result[resultPos++] = line[i++];
      else {
        // skip the old word in the 'line'
        i += strlen(oldWord);
        // look up its definition from the lookup table
        newWord = lookup_def(oldWord);
        // if it was not found, replace the newWord
        // = oldWord
        if (newWord == NULL)
          newWord = oldWord;
        // append the 'newWord' to the result
        memcpy(result + resultPos, newWord, strlen(newWord));
        // update the position in result
        resultPos += strlen(newWord);
      }
    }
  }

  result[resultPos] = '\0';

  return (strdup(result));
}

static void process_def(char *line)
// ======================================================================
// will process one line of '#define'
//
// the definition will be stored in the def[], and if the variable is
// 'OBJED' or 'OBJSHELL', then this value will be also put into the variables
// 'objEd' & 'objShell'...
// ......................................................................
{
  int i;
  char defName[4096];
  char *defVal;
  char *message;

  i = 7;
  while (!isspace(line[i])) {
    if (line[i] == '\0') {
      message = dsprintf("Line\n\n'%s'\n\nis not a valid definition "
                         "in 'object' file!\n",
                         line);
      //			xInfoBox (iconForm, message, "Warning(object)");
      warningMessage(message);
      xfree(message);
      return;
    }
    i++;
  }

  while (isspace(line[i]))
    i++;

  // now we have 'i' positioned to the definition variable, so read
  // it in
  if (1 != sscanf(&line[i], "%s", defName)) {
    message = dsprintf("Line\n\n'%s'\n\nis not a valid definition "
                       "in 'object' file!\n",
                       line);
    warningMessage(message);
    xfree(message);
    return;
  }

  i += strlen(defName);
  while (isspace(line[i]))
    i++;

  defVal = &line[i];

  /* add the definition to the def[] */
  nDefs += 1;
  def = (Def *)xrealloc(def, sizeof(Def) * nDefs);
  def[nDefs - 1].name = strdup(defName);
  def[nDefs - 1].val = strdup(defVal);

  /* check if it wasn't one of the 'OBJED' of 'OBJSHELL' */
  if (strcmp("OBJED", defName) == 0)
    objEd = strdup(defVal);
  else if (strcmp("OBJSHELL", defName) == 0) {
    objShell = strdup(defVal);
  }
}

void InitGenericTools(void)
// ======================================================================
// InitGenericTools () - inputs data about the generic tools and stores it
// in the array gtlist[] - this info is in TEMPDIR/VLnnntools(where 'nnn'
// is the uid)
// ......................................................................
{
  char *gtfile;
  FILE *fp;
  char *line, *word;
  struct gtools *g;
  char *lastchar;
  int i;

  gtfile = dsprintf("%s/object", qPrintable(Vlab::getUserConfigDir()));
  fp = fopen(gtfile, "r");
  if (!fp) {
    xfree(gtfile);
    std::cerr << "object: Can't open object configuration file.\n";
    exit(-1);
  }

  // read in the '#define' statements
  nDefs = 0;
  def = NULL;
  while ((line = getlineNC(fp)) != NULL) {
    // if the statement is not '#define', do not continue
    // processing the defines
    if (strncasecmp("#define", line, 7) != 0)
      break;

    process_def(line);
    xfree(line);
  }

  line = replace_defs(line);
  for (g = gtlist; (g->tname = line) != NULL; g++) {
    i = 0;
    while (true) {
      // get the next line
      line = replace_defs(getlineNC(fp));

      // stop executing at the end of a file
      if (line == NULL)
        break;

      word = strtok(line, " \t");
      lastchar = &word[strlen(word) - 1];
      if (*lastchar == ':') {
        *lastchar = (char)NULL; // remove ':'
        g->menu_item[i] = word;
        line = replace_defs(getlineNC(fp));
        g->cmdline[i] = strtok(line, "\t");
      } else {
        g->menu_item[i] = NULL;
        break;
      }

      // next item
      i++;
    }
  }
  fclose(fp);
}

static std::string get_line(FILE *fp)
// ======================================================================
// reads in a line from the file, strips the last '\n' if any
// ......................................................................
{
  bool one_read_success = false;

  std::string result;
  while (1) {
    // get a character
    int c = std::fgetc(fp);
    // if end of file, we are done
    if (c == EOF) {
      // if we have read in something at least, we are ok
      if (one_read_success)
        break;
      // otherwise we throw an exception
      throw "End of file reached";
    }
    // if end of line, we are done
    if (c == '\n')
      break;
    // otherwise, append the character to the result
    result.push_back(c);
  }
  return result;
}

static std::string removeTrailingSpaces(const std::string &str)
// ======================================================================
// removes trailing white spaces from a string
// ......................................................................
{
  // get the length of the string
  long len = str.length();
  // count how many spaces are at the end
  long count = 0;
  while (count < len && isspace(str[len - count - 1])) {
    count++;
  }
  // create a result by cutting of 'count' characters
  if (count == 0) {
    return str;
  } else {
    return std::string(str, 0, len - count);
  }
}

static std::string removeLeadingSpaces(const std::string &str)
// ======================================================================
// removes leading white spaces from a string
// ......................................................................
{
  // get the length of the string
  long len = str.length();
  // count how many spaces are at the begininng
  long count = 0;
  while (count < len && isspace(str[count])) {
    count++;
  }
  // create a result by cutting of 'count' characters
  if (count == 0) {
    return str;
  } else {
    return std::string(str, count, len);
  }
}

static bool GetFiles(FILE *fp)
// ======================================================================
// GetFiles () - reads the list of files given in the 'specifications' file
//(opened by ReadSpecs ()) into obj.fnames - the specifications file is
// always included as the first entry - a warning is given for duplicate
// file names
// A special line 'ignore:' will initialize a list of files to be ignored.
// ......................................................................
{
  long n_empty_lines = 0;
  FileList multiply_defined_files;

  // initialize the list of files with icon and specifications
  obj.fnames.clear();
  obj.fnames.push_back("specifications");
  obj.fnames.push_back("icon");
  //[PASCAL] Uncomment to load icon.png
  //  obj.fnames.push_back("icon.png");
  obj.fnamesIgnore.clear();
  // extract all entries until a line starting with '*' is discovered
  bool ignore = false;
  obj.savingModeCommand = "";
  while (1) {
    // get a line from the file
    std::string line;
    try {
      line = get_line(fp);
    } catch (...) {
      return false;
      // end of file reached
      fatalError(
          "Unexpected end of specifications encountered in the list of files.\n"
          "The terminating star (*) is probably missing.");
    }
    // remove trailing spaces
    line = removeTrailingSpaces(line);
    // remove leading spaces
    line = removeLeadingSpaces(line);
    // if line is empty, just make a warning and continue
    if (line == "") {
      n_empty_lines++;
      continue;
    }
    // if the line contains just a '*', we are done
    if (line == "*")
      break;
    // if the line starts with a ';', it is a comment, so skip it
    if (line.find(';') == 0)
      continue;

    // if the line contains savingMode: we check next line to get the mode
    if ((line.find("rmode:") == 0) || (line.find("refresh mode:") == 0) ||
        (line.find("Refresh mode:") == 0) ||
        (line.find("refresh Mode:") == 0) ||
        (line.find("Refresh Mode:") == 0) || (line.find("refreshMode:") == 0)) {
      int pos = line.find(':');
      line.replace(0, pos + 1, "");
      obj.savingModeCommand = std::string("-rmode ") + line;
      continue;
    }

    // if the line is 'ignore:', switch on the ignore mode

    if (line == "ignore:") {
      ignore = true;
      continue;
    }
    // are we populating the regular list of files or the
    // ignore files?
    if (ignore) {

      obj.fnamesIgnore.push_back(line);
    } else {
      // what remains in the line is the filename, so
      // check for duplicates first

      size_t ind = 0;
      for (ind = 0; ind < obj.fnames.size(); ind++)
        if (obj.fnames[ind] == line)
          break;
      if (ind < obj.fnames.size()) {
        multiply_defined_files.push_back(line);
        continue;
      }
      // add the object to the list
      obj.fnames.push_back(line);
    }
  }
  std::string warnings;
  if (n_empty_lines > 0)
    warnings += "Empty line(s) encountered in the list of files in the "
                "specifications.\n";
  if (!multiply_defined_files.empty()) {
    warnings += "There are multiply defined file(s) in the specifications:\n";
    for (size_t i = 0; i < multiply_defined_files.size(); i++) {
      if (i > 5) {
        warnings += "    ...";
        break;
      } else {
        warnings += "    " + multiply_defined_files[i] + "\n";
      }
    }
  }
  if (warnings != "")
    warningMessage(warnings);
  return true;
}

static struct spec *gtEntries(int gt)
// ======================================================================
// ......................................................................
{
  char *args, buf[2 * STRLEN];
  struct spec *sp = NULL;
  struct spec *first = NULL;
  struct spec *next = NULL;
  int i;

  first = NULL;
  args = strchr(line, ' ');
  for (i = 0; gtlist[gt].menu_item[i] != NULL; i++) {
    next = &Specs[NextSpec++];
    if (first == NULL) {
      first = sp = next;
    } else {
      sp->nextspec = next;
      sp = next;
    }
    sp->sname = gtlist[gt].menu_item[i];
    sp->subitems = sp->nextspec = NULL;
    if (args != NULL) {
      sprintf(buf, "%s %s", gtlist[gt].cmdline[i], args);
      sp->cmdline = (char *)strdup(buf);
    } else {
      sp->cmdline = gtlist[gt].cmdline[i];
    }
  }

  // add continuous option if necessary
  std::string new_command = sp->cmdline;
  std::vector<std::string> commandList;
  commandList.push_back("funcedit");
  commandList.push_back("cpfg");
  commandList.push_back("cuspy");
  commandList.push_back("gallery");
  commandList.push_back("palette");
  commandList.push_back("lpfg");
  commandList.push_back("panel");
  commandList.push_back("medit");
  commandList.push_back("stedit");
  commandList.push_back("timeline");

  std::istringstream iss(new_command);
  std::vector<std::string> results((std::istream_iterator<std::string>(iss)),
                                   std::istream_iterator<std::string>());
  for (size_t i = 0; i < commandList.size(); ++i) {
    std::string command = commandList[i];
    int found = results[0].compare(command);
    if (found == 0) {
      new_command = command + std::string(" ") + obj.savingModeCommand;
      for (size_t k = 1; k < results.size(); ++k)
        new_command += std::string(" ") + results[k];
      sp->cmdline = (char *)strdup(new_command.c_str());
    }
  }
  return (first);
}

static struct spec *GetEntry(FILE *fp)
// ======================================================================
// ......................................................................
{
  struct spec *sp, *subentry, *next;
  int level;
  int gt;

  sp = &Specs[NextSpec++];
  sp->sname = strtok(line, ":\t");
  sp->subitems = sp->nextspec = NULL;
  sp->cmdline = NULL;
  next = NULL;

  line = replace_defs(getlineNC(fp));
  level = ntabs(line);
  while (ntabs(line) == level) {
    if (strchr(line, ':') != NULL) {
      subentry = GetEntry(fp);
      if (next == NULL)
        sp->subitems = subentry;
      else
        next->nextspec = subentry;
      next = subentry;
      if (line == NULL)
        break;
    } else if ((gt = gtsearch(gtlist, line)) >= 0) {
      subentry = gtEntries(gt);
      if (next == NULL)
        sp->subitems = next = subentry;
      else
        next->nextspec = subentry;
      while (next->nextspec != NULL)
        next = next->nextspec;
      if ((line = replace_defs(getlineNC(fp))) == NULL)
        break;
    } else { /* command */
      sp->cmdline = line;

      std::string new_command = line;
      std::vector<std::string> commandList;
      commandList.push_back("funcedit");
      commandList.push_back("cpfg");
      commandList.push_back("cuspy");
      commandList.push_back("gallery");
      commandList.push_back("palette");
      commandList.push_back("lpfg");
      commandList.push_back("panel");
      commandList.push_back("medit");
      commandList.push_back("stedit");
      commandList.push_back("timeline");
      std::istringstream iss(new_command);
      std::vector<std::string> results(
          (std::istream_iterator<std::string>(iss)),
          std::istream_iterator<std::string>());
      if (results.size()<=0)
	return sp;
      for (size_t i = 0; i < commandList.size(); ++i) {
        std::string command = commandList[i];
        int found = command.compare(results[0]);
        if (found == 0) {
          new_command = command + std::string(" ") + obj.savingModeCommand;
		
          for (size_t k = 1; k < results.size(); ++k)
            new_command += std::string(" ") + results[k];
          sp->cmdline = (char *)strdup(new_command.c_str());
	  break;
        }
      }


      line = replace_defs(getlineNC(fp));
      break;
    }
  }
  return (sp);
}

static void GetSpecs(FILE *fp)
// ======================================================================
// ......................................................................
{
  struct spec *sp;

  NextSpec = 0;
  line = replace_defs(getlineNC(fp));
  sp = obj.sp = GetEntry(fp);
  while (line != NULL) {
    sp->nextspec = GetEntry(fp);
    sp = sp->nextspec;
  }
}

bool ReadSpecs(RA_Connection *connection, const std::string &specDir)
// ======================================================================
// ReadSpecs () - processes the object's 'specifications' file: - creates a
// list of the object's files - creates the hierarchical menus of tools -
// originally read from object's permanent directory but may be re-read
// from temp directory
// ......................................................................
{
  std::string specFile = specDir + "/specifications";
  bool grabAll = false;

  // first get the file from the remote/local host to a tmp_file
  QString tmpPath =
      QString(QDir::homePath() + QString("/.vlab/tmp/temp-specs-XXXXXX"));

  char *tmp_file = xstrdup(tmpPath.toLatin1());
  mkstemp(tmp_file);
  std::string tmpFile = tmp_file;
  xfree(tmp_file);
  if (RA::Fetch_file(connection, specFile.c_str(), tmpFile.c_str()) != 0) {
    // if there is no specifications file then list the directory to get the
    // number of files and verify with the user if they would like to copy
    // across the entire object
    char **dir_list;
    RA_Stat_Struc stat;
    int load;
    QStringList list;
    int ret = RA::Get_dir(obj.connection, obj.objDir.c_str(), &dir_list);
    if (ret != 0) {
      for (int i = 0; i < ret; i++) {
        std::string specFile = obj.objDir + "/" + dir_list[i];
        RA::Stat(obj.connection, specFile.c_str(), &stat);
        if (stat.type == RA_REG_TYPE)
          list << dir_list[i];
      }
      load = messageBox("No specification file found", ret, list);
      if (load == QMessageBox::Yes) {
        obj.fnames.clear();
        for (int i = 0; i < list.size(); i++) {
          obj.fnames.push_back(list.at(i).toStdString());
        }
        grabAll = true;
      } else {
        silentKill();
      }
    } else {
      fatalError("No files in object");
    }
  }
  // open the local copy
  FILE *fp = fopen(tmpFile.c_str(), "r");
  if (fp == NULL) {
    fatalError("can't open local copy of specification:" + tmpFile);
  }
  bool success = true;
  // process the specification file
  if (!grabAll) {
    success = GetFiles(fp);
    if (success)
      GetSpecs(fp); // can't get specs if we don't have a specifications file!
  }
  fclose(fp);
  unlink(tmpFile.c_str());
  return success;
}

static void MakeTemp()
// ======================================================================
// MakeTemp () - creates a temporary copy of the object in TEMPDIR - a
// subdirectory is created with the name: VLxxxxxx - all files in obj.files
//(the object's specifications file and all files listed there) are copied
// to the temp directory.
// ......................................................................
{
  // create a name for the temporary directory for this object
  QString tmpdir = Vlab::getTmpDir();
  char *buff = dsprintf("%s/VLXXXXXX", tmpdir.toStdString().c_str());
  if (mkdtemp(buff) == 0) {
    qqWarning(QString("object: could not create temporary directory:"));
    qqWarning(QString("        %1").arg(buff));
    qqWarning(QString("        error = %1").arg(strerror(errno)));
    qqWarning(QString("        Aborting."));
    exit(-1);
  }
  if (chdir(buff)) {
    qqWarning(QString("object: could not chdir into vlab table: %1").arg(buff));
    exit(-1);
  }

  obj.tmpDir = std::string(buff);

  std::string msg;
  for (size_t i = 0; i < obj.fnames.size(); i++) {
    std::string origFile = obj.objDir + "/" + obj.fnames[i];
    std::string labFile = obj.tmpDir + "/" + obj.fnames[i];
    if (0 !=
        RA::Fetch_file(obj.connection, origFile.c_str(), labFile.c_str())) {
      msg += "Could not copy file " + obj.fnames[i] + "\n";
      msg += "   src: " + origFile + "\n";
      msg += "   dst: " + labFile + "\n";
    }

    if (obj.connection->connection_type == RA_REMOTE_CONNECTION) {
      RA_Stat_Struc statStruct;
      RA::Stat(obj.connection, origFile.c_str(), &statStruct);
      // change permission on lab table according to the original files
      mode_t permissions = 0;
      // temporary hack to have permissions in any files
      permissions |= S_IRUSR | S_IWUSR | S_IXUSR | S_IROTH | S_IWOTH | S_IXOTH |
                     S_IRGRP | S_IWGRP | S_IXGRP;

      chmod(labFile.c_str(), permissions);
      chown(labFile.c_str(), getuid(), getgid());
    }    
    iconForm->progress().advance();
  }
  return;
  if (msg != "") {
    QMessageBox resizableMessageBox;
    resizableMessageBox.setText("Some files couldn't be loaded");
    resizableMessageBox.setDetailedText(QString::fromStdString(msg));
    resizableMessageBox.setIcon(QMessageBox::Warning);
    resizableMessageBox.setStandardButtons(QMessageBox::Cancel |
                                           QMessageBox::Ok);
    resizableMessageBox.setDefaultButton(QMessageBox::Cancel);
    int answer = resizableMessageBox.exec();
    switch (answer) {
    case QMessageBox::Cancel:
      qqWarning(QString("object is not opened, cancel by user"));
      exit(-1);
      return;
    default:
      return;
    }
  }
}

void RemoveTemp(void)
// ======================================================================
// RemoveTemp () - removes the object from TEMPDIR - all files in the
// subdirectory are unlinked, and the subdirectory is removed.
// ......................................................................
{
  chdir("/");
  delete_recursive(obj.tmpDir.c_str());
}

static void initialize_object()
// - initialize the object database
// - make a local directory
// - put the object into a local directory
// - initialize connection to vlabd
{
  // disconnect main window's signal "readyToLoad"
  // this will prevent the object from loading the files again 
  QObject::disconnect(wnd,SIGNAL(readyToLoad()),nullptr,nullptr);

  iconForm->progress().show(true,obj.getSaveAlways);
  // establish an RA connection
  // this connection will remain open until the end of initialization
  obj.connection =
      RA::new_connection(objectLocation.hostname().c_str(),
                         objectLocation.username().c_str(), password.c_str());
  if (obj.connection == NULL) {
    std::string errorMessage =
        std::string("Could not establish connection with raserver on ") +
        objectLocation.hostname() + std::string(" with login ") +
        objectLocation.username() + std::string(" and password ") + password;
    RA::Error(errorMessage.c_str());
    exit(-1);
  }

  // read the object configuration file
  InitGenericTools();
  // read the specifications file
  ReadSpecs(obj.connection, obj.objDir);
  // now we know how many files there are, so we can set the progress bar
  // appropriately
  iconForm->progress().setup(obj.fnames.size(), 1, 0);

  // also prepare a local connection
  obj.local_connection = RA::new_connection("localhost", NULL, NULL);
  assert(obj.local_connection != NULL);

  // check whether the location of the object is valid
  if (RA::Access(obj.connection, objectLocation.path().c_str(),
                 R_OK | X_OK | F_OK) != 0) {
    fprintf(stderr, "Cannot access object '%s'\n", object.c_str());
    ::exit(-1);
  }

  // open a connection to VLAB daemon
  vlabd = new VlabD;
  vlabd->init(false); // false = don't try to start vlabd if it's not running
  if (!vlabd->valid()) {
    std::cerr
        << "object: Warning! I cannot talk to vlabd, maybe it's not running.\n"
        << "        Some functionality may be disabled.\n"
        << "        libvlabd returned this error:\n"
        << "        " << vlabd->get_errstr() << "\n";
  }

  // register with VLAB daemon
  if (vlabd->valid())
    vlabd->va_send_message(REGISTER, "%ld,Object,%s", long(getpid()),
                           obj.objDir.c_str());

  // copy files from storage onto the table
  MakeTemp();

  obj.connection->Disconnect_remain_open_connection();
  // connection was kept open since its creation

  // files have been put onto vlab table, so finish up the
  // object interface initialization

  //iconForm->loadIcon();
  //call NewIconCb instead to overwrite the default icon
  iconForm->NewIconCb();
  iconForm->MakeMenu();
  iconForm->connectSocket(); // this does nothing right now (Pavol)
  iconForm->update();
  iconForm->progress().show(false,obj.getSaveAlways);
}

/* // to test, uncomment these two functions and the call in main()
static void test(const std::string &s) {
  std::cerr << "Testing: " << s << "\n";
  ObjectLocation loc(s);
  if (loc.valid())
    std::cerr << "  username: " << loc.username() << "\n"
              << "  hostname: " << loc.hostname() << "\n"
              << "  path:     " << loc.path() << "\n";
  else
    std::cerr << "  Error - invalid object location.\n";
}

static void tester() {
  test("pwp@csg:/usr/u/oofs");
  test("csg:/usr/u/oofs");
  test("/usr/u/oofs");
  test("a@b:c");
  test("@b:c");
  test(":c");
  test("pavol@blarg");
  test("a:b@c");
  test("pavol@federl@calgary");
  test("pavol:federl:calgary");
  test("@:");
  test("@");
  test(":");
  test("::");
  exit(0);
}
*/

void readConfiguration(const QString & fileName) {
  
  QSettings qs(fileName, QSettings::IniFormat);
  // default window size
  obj.winSize = qs.value("windowSize", QSize(140, 140)).toSize();
  // enforce some reasonable minimums (20x20)
  obj.winSize.setWidth(std::max(20, obj.winSize.width()));
  obj.winSize.setHeight(std::max(20, obj.winSize.height()));
  qs.setValue("windowSize", obj.winSize);
  // fitIcon toggle
  obj.fitIcon = qs.value("fitIcon", false).toBool();
  qs.setValue("fitIcon", obj.fitIcon);
  // background color (if icon does not fit)
  obj.bgColor =
      QColor(qs.value("bgColor", QColor(0, 0, 0).name()).toString());
  qs.setValue("bgColor", obj.bgColor.name());
  // color of the progress "pie" and the circle behind it
  obj.pieColor =
      QColor(qs.value("pieColor", QColor(70, 131, 0).name()).toString());
  qs.setValue("pieColor", obj.pieColor.name());
  obj.circleColor =
      QColor(qs.value("circleColor", QColor(0, 51, 0).name()).toString());
  qs.setValue("circleColor", obj.circleColor.name());
  // font color of the progress indicator
  obj.fontColor =
      QColor(qs.value("fontColor", QColor(255, 255, 255).name()).toString());
  qs.setValue("fontColor", obj.fontColor.name());
  
  // progress min. duration and fade time
  double minDuration = qs.value("progressMinDuration", 1.0).toDouble();
  qs.setValue("progressMinDuration", minDuration);
  double fadeTime = qs.value("progressFadeTime", 0.6).toDouble();
  qs.setValue("progressFadeTime", fadeTime);
  
  obj.progressMinDuration = int(minDuration * 1000);
  if (obj.progressMinDuration < 0)
    obj.progressMinDuration = 0;
  obj.progressFadeTime = int(fadeTime * 1000);
  if (obj.progressFadeTime < 0)
    obj.progressFadeTime = 0;
  
  // progress time threshold for displaying pie chart + boolean flag for always displaying
  double operationTimeThreshold = qs.value("operationTimeThreshold", 0.5).toDouble();
  qs.setValue("operationTimeThreshold", operationTimeThreshold);
  obj.operationTimeThreshold = std::max(0,int(operationTimeThreshold * 1000));
  obj.getSaveAlways = qs.value("getSaveAlways", true).toBool();
  qs.setValue("getSaveAlways", obj.getSaveAlways);

}

int main(int argc, char **argv) {
    /*
#ifdef __APPLE__
  char str[256];
  size_t size = sizeof(str);
  sysctlbyname("kern.osrelease", str, &size, NULL, 0);
  int version, x1, x2;
  sscanf(str, "%d.%d.%d", &version, &x1, &x2);
  if (version > 12) {
    // fix Mac OS X 10.9 (mavericks) font issue
    // https://bugreports.qt-project.org/browse/QTBUG-32789
    QFont::insertSubstitution(".Helvetica Neue DeskInterface", "Lucida Grande");
    QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
  }
#endif
  */

  // create user interface
  qapp = new QApplication(argc, argv);
  // set up vlab environment
  Vlab::setupVlabEnvironment();

  //if (0)
  //  tester();

  // retreive a user configurable parameters
  readConfiguration(Vlab::getUserConfigDir() + "/object.ini");

  // make myself a group process leader.  All processes spawned by me
  // will be members of this process group.  I can then send a signal
  // to all members of the group. This enables me to send QUIT and
  // KILL signals to all process I create.
  setsid();

  // parse command line arguments
  obj.rootDir = "";

  // parse the arguments
  for (int i = 1; i < argc; i++) {
    if (xstrcmp(argv[i], "-p") == 0) {
      i++;
      // password specification
      if (i == argc || password != "") {
        // no more arguments after '-p', or
        // password has already been specified
        usage(argc, argv);
      }
      password = xstrdup(argv[i]);
      continue;
    }

    if (xstrcmp(argv[i], "-posx") == 0) {
      i++;
      // password specification
      if (i == argc) {
        // no more arguments after '-p', or
        // password has already been specified
        usage(argc, argv);
      }
      obj.obj_posx = atoi(argv[i]);
      continue;
    }
    if (xstrcmp(argv[i], "-posy") == 0) {
      i++;
      // password specification
      if (i == argc) {
        // no more arguments after '-p', or
        // password has already been specified
        usage(argc, argv);
      }
      obj.obj_posy = atoi(argv[i]);
      continue;
    }

    // root_dir directory ?
    if (xstrcmp(argv[i], "-rootdir") == 0) {
      i++;
      // root_dir specified
      if (i == argc || obj.rootDir != "") {
        // either no more arguments present after
        // '-root_dir', or root directory already
        // specified. In either case it is a usage
        // error
        usage(argc, argv);
      }
      obj.rootDir = argv[i];
      continue;
    }

    // is this a name specification?
    if (argv[i][0] != '-') {
      // only one object specification allowed
      if (object == "") {
        object = argv[i];
        continue;
      } else {
        usage(argc, argv);
      }
    }

    fprintf(stderr, "object: Ignoring parameter '%s'\n", argv[i]);
  }
  // object location must be specified
  if (object == "")
    usage(argc, argv);
  // if object's location is '.', convert it to current directory
  if (object == ".") {
    object = QDir::currentPath().toStdString();
  }

  // parse the object location
  objectLocation = ObjectLocation(object);
  // if the host is localhost and the path is relative, make it absolute
  if (objectLocation.hostname() == "localhost") {
    QFileInfo info(objectLocation.path().c_str());
    objectLocation.path() = info.absoluteFilePath().toStdString();
  }
   obj.objDir = objectLocation.path();

  // determine the relative path to the object
  obj.nodePath = obj.objDir;
  obj.nodePath.erase(0, obj.rootDir.length());
  obj.nodePath = getBaseName2(obj.rootDir) + obj.nodePath;

  // remember the name of the object
  obj.objName = getBaseName2(obj.objDir);

  // set the environment variable to tell all applications we invoke
  // the name of the object
  setenv("VLAB_OBJECT_NAME", obj.objName.c_str(), 1);

  // create main Qt window and its OpenGL-based widget
  wnd = new MainWindow;

  iconForm = new QTGLObject(wnd, Qt::CustomizeWindowHint | Qt::WindowTitleHint |
                                     Qt::WindowMinimizeButtonHint |
                                     Qt::WindowSystemMenuHint |
                                     Qt::WindowCloseButtonHint);
  iconForm->resize(obj.winSize);
  iconForm->setMinimumSize(obj.winSize);
  iconForm->setMaximumSize(obj.winSize);
  iconForm->setWindowTitle(obj.objName.c_str());

  // connect main window's signal "ReadyToLoad" to Loader's slot "load"
  // must do this before wnd->show is called.
  Loader *loader = new Loader();
  loader->set_cb(initialize_object);
  QObject::connect(wnd,SIGNAL(readyToLoad(void)),loader,SLOT(load()),Qt::QueuedConnection);

  wnd->setCentralWidget(iconForm);
  wnd->setMinimumSize(iconForm->size());
  wnd->setMaximumSize(iconForm->size());
  wnd->resize(iconForm->size());
  wnd->setWindowTitle(obj.objName.c_str());
  wnd->move(obj.obj_posx, obj.obj_posy);
  wnd->show(); 
  wnd->raise();  
  wnd->activateWindow();

  // in previous versions, a QTimer was used for the Loader: 
  // QTimer::singleShot(0, loader, SLOT(load()));
  // But Loader had to be delayed by some time (>100 ms), so that
  // the OpenGL widget, QtGLObject, could be initialized.
  // The problem is that object wants to draw in the widget while it
  // is loading files (there are draw calls in initialize_object()),
  // but QtGLObject is not ready to draw anything.
  // Instead, in this version, a signal is emitted from the main window
  // upon activation, which is connected to the Loader's slot.
  // Then the connection is destroyed when the initialize_object
  // function is first called to prevent further attempts at
  // signalling Loader to "load".
  
  qapp->setWindowIcon(QIcon(":/hourglass.png"));
  qapp->exec();
  return 0;
}
