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



#include "Export.h"
#include "buildTree.h"
#include "delete_recursive.h"
#include "dirList.h"
#include "tree.h"
#include "ui_Export.h"
#include <QDesktopServices>
#include <QFileInfo>
#include <QProcess>
#include <QSettings>
#include <algorithm>
#include <iostream>
#include <sstream>

Export::Export(QWidget *parent, NODE *root, QString objName, QString basePath,
               int baseArchiveType, int hyperLinks)
    : QDialog(parent), ui(new Ui::Export), _root(root) {
  changeFormat = false;
  nodeName = objName;
  exportPath = basePath;
  saveName = exportPath + nodeName;

  if (baseArchiveType == 1)
    saveName = saveName + QString(".tgz");
  if (baseArchiveType == 2)
    saveName = saveName + QString(".zip");

  ui->setupUi(this);
  // we want explicit control of the order of the items in the combobox so that
  // we know from the code what each index is
  archiveTypes << "Directory"
               << "/";
  archiveTypes << "Tarred gzip"
               << ".tgz";
  archiveTypes << "Zip Archive"
               << ".zip";
  formats << "Windows"
          << "Mac/Linux";
  for (int i = 0; i < archiveTypes.size(); i += 2) {
    ui->comboBox->addItem(archiveTypes.at(i) + " (" + archiveTypes.at(i + 1) +
                          ")");
  }
  for (int i = 0; i < formats.size(); i++) {
    ui->comboBox_4->addItem(formats.at(i));
  }
  previousOutputFormat = baseArchiveType;

  ui->checkBox_2->setChecked(hyperLinks);

  ui->comboBox->setCurrentIndex(baseArchiveType);
  ui->comboBox_4->setCurrentIndex(1);
  ui->directory_2->setEditText(exportPath);
  setLineEdit();
  settingsFile = QString(QDir::homePath()) + "/.vlab/lpfgsettings.ini";
  ui->directory_2->setDuplicatesEnabled(false);
  ui->directory_2->setInsertPolicy(QComboBox::InsertAtTop);
  loadSettings();
  ui->directory_2->setCurrentIndex(0);
  exportPath = ui->directory_2->currentText();

  connect(ui->textEdit, SIGNAL(textChanged()), SLOT(preserveExtension()));
  connect(ui->textEdit, SIGNAL(textChanged()), SLOT(preserveFormat()));
  connect(ui->textEdit, SIGNAL(cursorPositionChanged()),
          SLOT(preserveExtension()));

  connect(ui->textEdit, SIGNAL(cursorPositionChanged()),
          SLOT(preserveExtension()));

  connect(ui->comboBox, SIGNAL(currentIndexChanged(int)),
          SLOT(changeExtension(int)));
  connect(ui->directory_2, SIGNAL(activated(QString)),
          SLOT(selectingPath(QString)));

  this->setWindowTitle("Export");
}

void Export::selectingPath(const QString &text) {
  exportPath = text;
}

void Export::loadSettings() {
  QSettings settings(settingsFile, QSettings::NativeFormat);
  QStringList keyList = settings.allKeys();
  QStringList pathList;

  if (!exportPath.isEmpty()) {
    ui->directory_2->addItem(exportPath);
    pathList << exportPath;
  }
  for (QStringList::Iterator it = keyList.begin(); it != keyList.end(); ++it) {
    QString key = *it;
    QString sText = settings.value(key, "").toString();
    if (!pathList.contains(sText)) {
      ui->directory_2->addItem(sText);
      pathList << sText;
    }
  }
  if ((keyList.size() == 0))
    ui->directory_2->addItem(
        QStandardPaths::standardLocations(QStandardPaths::DesktopLocation)[0]);
}

void Export::writeSettings() {
  QSettings settings(settingsFile, QSettings::NativeFormat);
  int currentIndex = ui->directory_2->currentIndex();
  std::stringstream st;
  st << 0;
  std::string key = "path" + st.str();
  settings.setValue(QString::fromStdString(key),
                    ui->directory_2->itemText(currentIndex));

  for (int i = 0; i < std::min(10, ui->directory_2->count()); i++) {
    std::stringstream st;
    st << i + 1;
    std::string key = "path" + st.str();
    if (i != currentIndex)
      settings.setValue(QString::fromStdString(key),
                        ui->directory_2->itemText(i));
  }
}

void Export::closeEvent(QCloseEvent *) {
  writeSettings();
}

Export::~Export() { delete ui; }

void Export::changeEvent(QEvent *e) {
  QDialog::changeEvent(e);
  switch (e->type()) {
  case QEvent::LanguageChange:
    ui->retranslateUi(this);
    break;
  default:
    break;
  }
}

void Export::setNodeName() {
  int ext_size = 4;
  if (previousOutputFormat == 0)
    ext_size = 0;
  nodeName = ui->textEdit->toPlainText();
  nodeName.chop(ext_size);
}

void Export::preserveFormat() {
  QTextCursor cursor = ui->textEdit->textCursor();
  int cursorPosition = cursor.position();
  if (cursorPosition == 1) {
    if (changeFormat) {
      changeFormat = false;
      setNodeName();
      setLineEdit();
      cursor.setPosition(1);
      ui->textEdit->setTextCursor(cursor);
    }
  } else
    changeFormat = true;
}

void Export::preserveExtension() {
  int ext_size = 4;
  if (previousOutputFormat == 0)
    ext_size = 0;

  QString name = ui->textEdit->toPlainText();

  QTextCursor cursor = ui->textEdit->textCursor();
  int cursorPosition = cursor.position();
  if (cursorPosition > name.size() - ext_size) {
    cursor.setPosition(name.size() - ext_size);
    ui->textEdit->setTextCursor(cursor);
  }

}

void Export::changeExtension(int inValue) {
  setNodeName();
  this->setLineEdit();
  previousOutputFormat = inValue;
}

int Export::getType() {
  return ui->comboBox->currentIndex();
}

int Export::getFormat() { return ui->comboBox_4->currentIndex(); }

void Export::setFormat(int inValue) {
  setNodeName();
  ui->comboBox_4->setCurrentIndex(inValue);
}

bool Export::getRecursive() { return ui->checkBox->isChecked(); }

void Export::setRecursive(bool inValue) { ui->checkBox->setChecked(inValue); }

void Export::setType(int inValue) { ui->comboBox->setCurrentIndex(inValue); }

QString Export::getPath() { return ui->directory_2->currentText(); }

void Export::setPaths(QStringList inPaths) {
  for (int i = 0; i < inPaths.size(); i++) {
    ui->directory_2->addItem(inPaths.at(i));
  }
  // intelligently set the type based on the first item added to the drop-downs,
  // might as well make it easy for the user
  for (int i = 1; i < archiveTypes.size(); i += 2) {
    if (ui->directory_2->currentText().endsWith(archiveTypes.at(i)))
      ui->comboBox->setCurrentIndex((i - 1) / 2);
  }
}

void Export::setLineEdit() {
  QString formatedNodeName =
      QString("<span style= color:#000000;> %1</span>").arg(nodeName);
  QString ext =
      QString("<span style= color:#999999;>%1</span>").arg(getExtension());
  ui->textEdit->setHtml("<span style= color:#000000;>" + formatedNodeName +
                        "</span>" + ext);
}

QString Export::getExtension() {
  QString extension;
  switch (ui->comboBox->currentIndex()) {
  case 0:
    extension = "";
    break;
  case 1:
    extension = ".tgz";
    break;
  case 2:
    extension = ".zip";
    break;
  default:
    extension = "";
    break;
  }
  return extension;
}

void Export::browse() {
  // based on the choice in the format drop-down we either need to pick a
  // directory to save in, or pick a filename to save as....
  // QString temp = QDir::fromNativeSeparators(formats.at(inIndex)); // put all
  // the slashes the way we like them
  QComboBox *target;
  QString temp, dir;
  int type;
  // Export tab
  target = ui->directory_2;
  type = ui->comboBox->currentIndex();
  temp = target->currentText();
  temp = QDir::fromNativeSeparators(
      temp); // QFileDialog likes these strings to be in the "/" format, so
             // let's give it to them that way
  dir = QFileDialog::getExistingDirectory(
      this, tr("Select Directory"), exportPath,
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  // then put the file/directory name back into native format (what the user
  // will be used to) and set the edit text to read this
  if (dir.length() > 0) { // only if they actually picked something
    exportPath = QDir::toNativeSeparators(dir);
    ui->directory_2->setEditText(exportPath);
    bool present = false;
    for (int i = 0; i < ui->directory_2->count(); i++) {
      QString sText = ui->directory_2->itemText(i);
      if (!exportPath.compare(sText)) {
        present = true;
      }
    }
    if (!present)
      ui->directory_2->insertItem(0, exportPath);
  }

}

void Export::ok() {
  // we don't need to test the result() attribute of this object since
  // closeEvent() is only called when accepted but if you like you can test: if
  // (result() == QDialog::accepted)
  int ret;
  ret = this->exportObject();
  if (ret == 0)
    accept();
}

void Export::nodeList(RA_Connection *conn, NODE *root, QString path,
                      QString dirPath, bool recursive = false) {
  char *realPath;
  char **dir_list;
  RA_Stat_Struc stat;
  QStringList dirlist;
  QString baseDir = root->name;
  if (root->isHObj) {
    // we get the real path
    RA::Realpath(conn, root->object_name, realPath);
    path = QString(realPath);
    baseDir = dirPath + "/ext/" + (QFileInfo(QString(realPath)).fileName());
  }

  to->remoteDirs << path; // push the current directory on to the list, this way
                          // if we have to go 2 levels deep without files we
                          // still keep the names to create them later
  to->localDirs << baseDir;

  int ret = RA::Get_dir(conn, path.toStdString().c_str(), &dir_list);
  if (ret != 0) {
    for (int i = 0; i < ret; i++) {
      QString specFile = path;
      specFile.append("/").append(dir_list[i]);
      RA::Stat(conn, specFile.toStdString().c_str(), &stat);
      if (stat.type == RA_REG_TYPE) {
        to->remoteFiles
            << specFile; // if we found a file push it on the list too!
        specFile = baseDir;
        specFile.append("/").append(dir_list[i]);
        to->localFiles << specFile;
      } else if (stat.type == RA_DIR_TYPE) {
        dirlist << specFile;
        to->remoteDirs << specFile;
        specFile = baseDir;
        specFile.append("/").append(dir_list[i]);
        to->localDirs << specFile;
      } else if (stat.type == RA_OTHER_TYPE)
        fprintf(stderr, "Unrecognized file: %s\n",
                specFile.toStdString().c_str());
    }
    if (recursive)
      if (root->nChildren == 0) {
        get_file_tree(root);
        build_tree();
        sysInfo.mainForm->updateDisplay();
      }

    for (int i = 0; i < root->nChildren; i++) {
      this->nodeList(conn, root->child[i], root->child[i]->name, baseDir,
                     recursive);
    }
  }
  if (dir_list != NULL)
    delete[] dir_list;
}

void Export::nodeList(RA_Connection *conn, QString path,
                      bool recursive = false) {
  to->remoteDirs << path; // push the current directory on to the list, this way
                          // if we have to go 2 levels deep without files we
                          // still keep the names to create them later
  char **dir_list;
  RA_Stat_Struc stat;
  QStringList dirlist;
  int ret = RA::Get_dir(conn, path.toStdString().c_str(), &dir_list);
  if (ret != 0) {
    for (int i = 0; i < ret; i++) {
      QString specFile = path;
      specFile.append("/").append(dir_list[i]);
      RA::Stat(conn, specFile.toStdString().c_str(), &stat);
      if (stat.type == RA_REG_TYPE) {
        to->remoteFiles
            << specFile; // if we found a file push it on the list too!
      } else if (stat.type == RA_DIR_TYPE)
        dirlist << specFile;
      else if (stat.type == RA_OTHER_TYPE)
        fprintf(stderr, "Unrecognized file: %s\n",
                specFile.toStdString().c_str());
    }
    if (recursive)
      for (int i = 0; i < dirlist.size(); i++) {
        this->nodeList(conn, dirlist.at(i), recursive);
      }
  }
  if (dir_list != NULL)
    delete[] dir_list;
}

int Export::exportObject() {

  setNodeName();
  saveName = exportPath + "/" + nodeName;
  if (ui->comboBox->currentIndex() == 1)
    saveName = saveName + QString(".tgz");
  if (ui->comboBox->currentIndex() == 2)
    saveName = saveName + QString(".zip");

  nodeName = sysInfo.selNode->realPath;
  to = new TransferObject();
  if (ui->checkBox_2->isChecked())
    this->nodeList(sysInfo.connection, _root, sysInfo.selNode->realPath,
                   sysInfo.selNode->realPath, this->getRecursive());
  else
    this->nodeList(sysInfo.connection, sysInfo.selNode->realPath,
                   this->getRecursive());

  QString tmpDir = sysInfo.tmpDir;
  tmpDir.append("/exportTemp");
  QByteArray tmpDirBytes = tmpDir.toLatin1();
  if (access(tmpDirBytes.data(), F_OK) == 0)
    delete_recursive(tmpDir.toStdString().c_str());
  if (mkdir(tmpDirBytes.data(), 0755)) {
    QMessageBox::critical(this, "Error creating directory",
                          QString("Unable to create directory:\n") + tmpDir,
                          QMessageBox::Ok, QMessageBox::Ok);
    return 0;
  }
  if (access(tmpDirBytes.data(), W_OK)) {
    QMessageBox::critical(this, "Error writing to directory directory",
                          QString("Unable to write to directory:\n") + tmpDir,
                          QMessageBox::Ok, QMessageBox::Ok);
    return 0;
  }
  // if we are going to put files into the windows format then we shall strip
  // all "/ext" directories from the paths...
  QString localName;
  QByteArray localNameBytes;
  if (this->getFormat() == 0) { // windows format
    QStringList tempDirs;
    for (int i = 0; i < to->remoteDirs.size(); i++) {
      if (!(to->remoteDirs.at(i).endsWith(
              "/ext"))) { // only push on directories which do not end in /ext,
                          // they will not be needed
        // strip all of the /ext's from the path (since they can still be at a
        // higher level) then push the fixed names on tempDirs
        if (ui->checkBox_2->isChecked())
          localName = to->localDirs.at(i);
        else
          localName = to->remoteDirs.at(i);
        localName.remove(
            0, nodeName.length()); // if the prefix has "/ext" in it, we do not
                                   // wish to strip that part
        localName.replace("/ext", "");
        localName.prepend(
            nodeName); // put it back the way we found it for later
        tempDirs.push_back(localName);
      }
    }
    to->remoteDirs = tempDirs;
  }
  for (int i = 0; i < to->remoteDirs.size(); i++) {
    if (ui->checkBox_2->isChecked())
      localName = to->localDirs.at(i);
    else
      localName = to->remoteDirs.at(i);
    localName.remove(0, nodeName.length());
    localName.prepend(tmpDir);
    localNameBytes = QDir::toNativeSeparators(localName).toLatin1();
    if (access(localNameBytes.data(), F_OK)) // if the directory does not exist
      if (mkdir(localNameBytes.data(), 0755)) { // then make it
        QMessageBox::critical(this, "Error creating directory",
                              QString("Unable to create directory:\n") +
                                  localName,
                              QMessageBox::Abort, QMessageBox::Abort);
        return 0;
      }
    if (access(localNameBytes.data(), W_OK)) {
      QMessageBox::critical(this, "Error writing to directory",
                            QString("Unable to write to directory:\n") +
                                localName,
                            QMessageBox::Abort, QMessageBox::Abort);
      return 0;
    }
    if ((this->getFormat() == 1) &&
        (ui->checkBox_2->isChecked())) { // Linux format
      // create /ext directory
      if (!(to->remoteDirs.at(i).endsWith("/ext"))) {
        localName = to->localDirs.at(i);
        localName.append("/ext");
        localName.remove(0, nodeName.length());
        localName.prepend(tmpDir);
        localNameBytes = QDir::toNativeSeparators(localName).toLatin1();
        if (access(localNameBytes.data(),
                   F_OK)) // if the directory does not exist
          if (mkdir(localNameBytes.data(), 0755)) { // then make it
            QMessageBox::critical(this, "Error creating directory",
                                  QString("Unable to create directory:\n") +
                                      localName,
                                  QMessageBox::Abort, QMessageBox::Abort);
            return 0;
          }
        if (access(localNameBytes.data(), W_OK)) {
          QMessageBox::critical(this, "Error writing to directory",
                                QString("Unable to write to directory:\n") +
                                    localName,
                                QMessageBox::Abort, QMessageBox::Abort);
          return 0;
        }
      }
    }
  }
  for (int i = 0; i < to->remoteFiles.size(); i++) {
    if (ui->checkBox_2->isChecked())
      localName = to->localFiles.at(i);
    else
      localName = to->remoteFiles.at(i);
    localName.remove(0, nodeName.length());
    localName = QDir::fromNativeSeparators(localName);
    if (this->getFormat() == 0) {
      localName.replace("/ext", "");
    }
    localName.prepend(tmpDir);
    localNameBytes = QDir::toNativeSeparators(localName).toLatin1();
    if (RA::Fetch_file(sysInfo.connection,
                       to->remoteFiles.at(i).toStdString().c_str(),
                       localNameBytes.data()) != 0) {
      QMessageBox::critical(this, "Error fetching file",
                            QString("Unable to copy remote file:\n") +
                                to->remoteFiles.at(i) + QString("\n") +
                                QString("To local file:\n") + localName,
                            QMessageBox::Abort, QMessageBox::Abort);
      return 0;
    } else {
      // the else in this case is success... yes, it is kind of a backwards way
      // of writing this function
      if (this->getFormat() ==
          0) { // then we need to run unix2dos on these files as well
        // files that we should _NOT_ run unix2dos on: icon, *.map, *.mat
        if (!localName.endsWith(".mat", Qt::CaseInsensitive) &&
            !localName.endsWith("icon", Qt::CaseInsensitive) &&
            !localName.endsWith(".map", Qt::CaseInsensitive)) {
          // run unix2dos on the file
          QByteArray temp = localName.simplified().toLocal8Bit();
          QList<QByteArray> list;
          list << "unix2dos"
               << "-q" << temp;
          char *argv[3];
          for (int i = 0; i < 3; i++) {
            argv[i] = list[i].data();
          }
          int ret = unix2dos(3, argv); // error check this one!!
          if (ret != 0) {              // we had an error
            QMessageBox::critical(this, "Error converting file",
                                  QString("Unable to convert file:\n") +
                                      localName + QString("\nTo DOS format"),
                                  QMessageBox::Abort, QMessageBox::Abort);
            return 0;
          }
        }
      }
    }
  }
  // okay, now we have /tmp/exportTemp/ filled with what we wanted to export
  // with the correct file formats, we just need to package it then relocate it
  // to the final place it will live
  QString target = saveName;
  QString sysCommand;
  chdir(tmpDir.toStdString().c_str());
  int ret, status;
  QProcess process(this);
  switch (this->getType()) {
  case 0: // directory
    target.append("/");
    localNameBytes = QDir::toNativeSeparators(target).toLatin1();
    if (access(localNameBytes.data(), F_OK))
      mkdir(target.toStdString().c_str(), 0755);
    else {
      // some kind of popup telling the user they are writing into an existing
      // directory
      ret = QMessageBox::warning(this, QString("Target directory exists"),
                                 QString("The target directory: ") + target +
                                     QString("\nalready exists.\n Overwrite?"),
                                 QMessageBox::Ok | QMessageBox::Cancel,
                                 QMessageBox::Ok);
      if (ret == QMessageBox::Cancel) {
        return -1;
      } else {
        delete_recursive(target.toStdString().c_str());
        mkdir(target.toStdString().c_str(), 0755);
      }
    }
    sysCommand = QString("cp -R \"")
                     .append(tmpDir)
                     .append("/\" \"")
                     .append(target)
                     .append("\"");
    fprintf(stderr, "Executing system call: %s\n",
            sysCommand.toStdString().c_str());
    status = process.execute(sysCommand);
    if (status) {
      QMessageBox::critical(
          this, QString("Error copying package"),
          QString("Unable to copy package to destination directory:\n") +
              target,
          QMessageBox::Abort, QMessageBox::Abort);
      return 0;
    }
    break;
  case 1: // tarred gzip
    fprintf(stderr, "Creating archive %s\n", target.toStdString().c_str());
    localNameBytes = QDir::toNativeSeparators(target).toLatin1();
    if (!access(localNameBytes.data(),
                F_OK)) { // we want an error here.. no error means it exists
      ret = QMessageBox::warning(
          this, QString("Target file exists"),
          QString("The target file: ") + target +
              QString(
                  "\nAlready exists, it will be overwritten if you continue"),
          QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
      if (ret == QMessageBox::Cancel) {
        return -1;
      } else {
        unlink(target.toStdString().c_str());
      }
    }
    target.prepend("\"").append("\"");
    sysCommand = QString("tar -cPzf ")
                     .append(target)
                     .append(" -C ")
                     .append(tmpDir)
                     .append(" .");

    status = process.execute(sysCommand);
    if (status < 0) {
      QMessageBox::critical(this, "Error creating archive",
                            QString("Unable to create archive:\n") +
                                target.left(target.length() - 2),
                            QMessageBox::Abort, QMessageBox::Abort);
      return 0;
    }
    break;
  case 2: // zip archive
     localNameBytes = QDir::toNativeSeparators(target).toLatin1();
     if (!access(localNameBytes.data(), F_OK)) {
      ret = QMessageBox::warning(
          this, "Target file exists",
          QString("The target file: ") + target +
              QString(
                  "\nAlready exists, it will be overwritten if you continue"),
          QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
      if (ret == QMessageBox::Cancel) {
        return -1;
      } else {
        unlink(target.toStdString().c_str());
      }
    }
    QString current_path = QDir::currentPath();
    QDir::setCurrent(tmpDir);
    sysCommand = QString("zip -r -q ").append(target).append(" . ");
    status = process.execute(sysCommand);
    QDir::setCurrent(current_path);
    if (status) {
      QMessageBox::critical(this, "Error creating archive",
                            QString("Unable to create archive:\n") + target,
                            QMessageBox::Abort, QMessageBox::Abort);
      return 0;
    }
    break;
  }
  chdir("/");
  delete to;
  return 0;
}

void Export::receiveTransferObject(TransferObject *inObject) {
  to = inObject;
  fprintf(stderr, "received the transfer object\n");
}

void Export::grabConflicts(TransferObject *to) {
  if (sysInfo.connection != NULL) {
    if (sysInfo.connection->connection_type != RA_NO_CONNECTION) {
      RA_Stat_Struc stat_struct;
      int stat_ret;
      for (int i = 0; i < to->remoteDirs.size(); i++) {
        stat_ret =
            RA::Stat(sysInfo.connection,
                     to->remoteDirs.at(i).toStdString().c_str(), &stat_struct);
        QString temp = to->dirFlags.at(i);
        temp.replace("C", ""); // clear any C's that may be in the string first
        if (stat_ret == 0) {
          if (stat_struct.type != RA_NOEXIST_TYPE) {
            temp.append("C");
          } else {
          }
        } else {
        }
        to->dirFlags.replace(i, temp);
      }
      for (int i = 0; i < to->remoteFiles.size(); i++) {
        stat_ret =
            RA::Stat(sysInfo.connection,
                     to->remoteFiles.at(i).toStdString().c_str(), &stat_struct);
        QString temp = to->fileFlags.at(i);
        temp.replace("C", "");
        if (stat_ret == 0) {
          if (stat_struct.type != RA_NOEXIST_TYPE) {
            temp.append("C");
          } else {
          }
        } else {
        }
        to->fileFlags.replace(i, temp);
      }
    }
  }
}
