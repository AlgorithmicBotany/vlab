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



#include "Import.h"
#include "dirList.h"
#include "ui_Import.h"
#include <QDesktopServices>
#include <QFileInfo>
#include <QProcess>
#include <QSettings>
#include <algorithm>
#include <iostream>
#include <sstream>

Import::Import(QWidget *parent, QString objName, QString basePath,
               int baseArchiveType)
    : QDialog(parent), ui(new Ui::Import) {
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

  ui->comboBox->setCurrentIndex(baseArchiveType);
  ui->comboBox_4->setCurrentIndex(1);
  //    tab open and disable the second tab ui->tabWidget->setCurrentIndex(1);
  ui->directory_2->setDuplicatesEnabled(false);
  ui->directory_2->setInsertPolicy(QComboBox::InsertAtTop);
  ui->directory_2->setEditText(exportPath);
  settingsFile = QString(QDir::homePath()) + "/.vlab/lpfgsettings.ini";
  loadSettings();
  ui->directory_2->setCurrentIndex(0);
  exportPath = ui->directory_2->currentText();

  connect(ui->directory_2, SIGNAL(activated(QString)),
          SLOT(selectingPath(QString)));

  this->setWindowTitle("Import");
}

void Import::selectingPath(const QString &text) {
  exportPath = text;
}

void Import::loadSettings() {
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
  if (keyList.size() == 0)
    ui->directory_2->addItem(
        QStandardPaths::standardLocations(QStandardPaths::DesktopLocation)[0]);
}

void Import::writeSettings() {
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

void Import::closeEvent(QCloseEvent*) {
  writeSettings();
}

Import::~Import() { delete ui; }

void Import::changeEvent(QEvent *e) {
  QDialog::changeEvent(e);
  switch (e->type()) {
  case QEvent::LanguageChange:
    ui->retranslateUi(this);
    break;
  default:
    break;
  }
}

int Import::getType() {
  return ui->comboBox->currentIndex();
}

int Import::getFormat() { return ui->comboBox_4->currentIndex(); }

void Import::setFormat(int inValue) {
  ui->comboBox_4->setCurrentIndex(inValue);
}

void Import::setType(int inValue) { ui->comboBox->setCurrentIndex(inValue); }

QString Import::getPath() { return ui->directory_2->currentText(); }

void Import::setPaths(QStringList inPaths) {
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

QString Import::getExtension() {
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

void Import::browse() {
  // based on the choice in the format drop-down we either need to pick a
  // directory to save in, or pick a filename to save as....
  // QString temp = QDir::fromNativeSeparators(formats.at(inIndex)); // put all
  // the slashes the way we like them
  QComboBox *target;
  QString temp, dir;
  int type;
  // Import tab
  target = ui->directory_2;
  type = ui->comboBox->currentIndex();
  temp = target->currentText();
  temp = QDir::fromNativeSeparators(
      temp); // QFileDialog likes these strings to be in the "/" format, so
             // let's give it to them that way
  if (type == 0)
    dir =
        QFileDialog::getExistingDirectory(this, tr("Select File"), exportPath);
  else
    dir = QFileDialog::getOpenFileName(this, tr("Select File"), exportPath,
                                       archiveTypes.at(type * 2) + "(*" +
                                           archiveTypes.at(type * 2 + 1) +
                                           ");;All Files (*.*)",
                                       0, QFileDialog::DontResolveSymlinks);

  // then put the file/directory name back into native format (what the user
  // will be used to) and set the edit text to read this
  if (dir.length() > 0) { // only if they actually picked something
    exportPath = QDir::toNativeSeparators(dir);
    ui->directory_2->setEditText(exportPath);
    bool present = false;
    QString tempPath = QFileInfo(exportPath).absolutePath();
    for (int i = 0; i < ui->directory_2->count(); i++) {
      QString sText = ui->directory_2->itemText(i);
      if (!tempPath.compare(sText)) {
        present = true;
      }
      //	i++;
    }
    if (!present)
      ui->directory_2->insertItem(0, tempPath);
  }

}

void Import::ok() {
  // we don't need to test the result() attribute of this object since
  // closeEvent() is only called when accepted but if you like you can test: if
  // (result() == QDialog::accepted)
  int ret;
  ret = this->importObject();
  if (ret == 0)
    accept();
}

void Import::nodeList(RA_Connection *conn, QString path,
                      bool recursive = false) {
  fprintf(stderr, "ImportImport::fileList() input path: %s\n",
          path.toStdString().c_str());
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
      if (stat.type == RA_REG_TYPE)
        to->remoteFiles
            << specFile; // if we found a file push it on the list too!
      else if (stat.type == RA_DIR_TYPE)
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

int Import::importObject() {

  saveName = exportPath;
  nodeName = sysInfo.selNode->realPath;

  QString tmpDir = QDir::fromNativeSeparators(sysInfo.tmpDir);
  to = new TransferObject();
  tmpDir.append("/importTemp");
  QString source, targetNode;
  if (this->getType() == 0) {
    // strip the trailing slash then give us the name after the last remaining
    // slash
    if (this->getPath().endsWith("/")) // only strip it if it is there!
      targetNode = QDir::toNativeSeparators(
          QDir::fromNativeSeparators(
              this->getPath().left(this->getPath().length() - 1))
              .split("/")
              .last());
    else
      targetNode = QDir::toNativeSeparators(
          QDir::fromNativeSeparators(this->getPath()).split("/").last());
  } else {
    targetNode = QDir::toNativeSeparators(
        QDir::fromNativeSeparators(this->getPath()).split("/").last());
    // we just want the node
    // name, not the ".tar.gz" or ".zip" part
    if (targetNode.endsWith(".tgz", Qt::CaseInsensitive))
      targetNode = targetNode.left(targetNode.length() - 4);
    else if (targetNode.endsWith(".zip", Qt::CaseInsensitive))
      targetNode = targetNode.left(targetNode.length() - 4);
  }

  // double check target name against the extension we expect, if there is a
  // mis-match then throw an error or warn the user

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

  targetNode.prepend("/").prepend(tmpDir); // set up the target node directory
  tmpDirBytes = targetNode.toLatin1();
  if (mkdir(tmpDirBytes.data(), 0755)) {
    QMessageBox::critical(
        this, "Error creating directory",
        QString("Unable to create temporary directory for object:\n") +
            targetNode,
        QMessageBox::Ok, QMessageBox::Ok);
    return 0;
  }
   chdir(targetNode.toStdString()
            .c_str()); // let's change into our temp directory")
  QString sysCommand;
  source = this->getPath();
  int status;
  QProcess process(this);
  int error;
  switch (this->getType()) {
  case 0: // directory
    // copy this->getPath() into source, then check to make sure it has a
    // trailing slash or else the cp -R can make an extra level of directory
    source = QDir::fromNativeSeparators(source);
    if (!source.endsWith("/"))
      source.append("/");
    source = QDir::toNativeSeparators(source);
    sysCommand = QString("cp -R \"").append(source).append("\" ./");
    error = QProcess::execute(sysCommand);
    if (error)
      QMessageBox::critical(
          this, QString("Error copying source object"),
          QString("Unable to copy object to temporary directory:\n") +
              targetNode,
          QMessageBox::Abort, QMessageBox::Abort);
    break;
  case 1: // tarred gzip
    source = QDir::toNativeSeparators(source);
    if (!source.endsWith(".tgz"))
      fprintf(stderr, "ImportExport::Import() asked to decompress a .tgz file "
                      "which does not end in .tgz\n");
    sysCommand =
        QString("tar -xzf \"").append(source).append("\""); //.append(" ./");
    fprintf(stderr, "Executing system call: %s\n",
            sysCommand.toStdString().c_str());
    status = process.execute(sysCommand);

    if (status < 0)
      QMessageBox::critical(this, QString("Error decompressing archive"),
                            QString("Unable to decompress archive:\n") + source,
                            QMessageBox::Abort, QMessageBox::Abort);
    break;
  case 2: // zip archive
    source = QDir::toNativeSeparators(source);
    if (!source.endsWith(".zip"))
      fprintf(stderr, "ImportExport::Import() asked to decompress a .zip file "
                      "which does not end in .zip\n");
    sysCommand = QString("unzip -q \"")
                     .append(source)
                     .append("\"")
                     .append(" -d \"")
                     .append(targetNode)
                     .append("/\"");
     if (QProcess::execute(sysCommand))
      QMessageBox::critical(this, QString("Error decompressing archive"),
                            QString("Unable to decompress archive:\n") + source,
                            QMessageBox::Abort, QMessageBox::Abort);
    break;
  }
  // now that we have our working files in /tmp/importTemp/ we need to take a
  // look at what we have and build some structures for it all

  RA_Connection *tmpConn = new RA_Connection();
  tmpConn->connection_type =
      RA_LOCAL_CONNECTION; // this is enough to get the RA commands working
                           // locally, it is a small hack but it simplifies the
                           // code I am writing by quite a lot
  this->nodeList(tmpConn, targetNode, true);
  delete tmpConn;
  to->localDirs = to->remoteDirs;
  to->localFiles = to->remoteFiles;
  to->remoteDirs.clear();
  to->remoteFiles.clear();
 
  if ((to->localDirs.size() > 0) && (to->localFiles.size() > 0)) {
    // we need to have extracted something to work on, if there is nothing there
    // then there is no point in proceeding
    QString remoteName;
    for (int i = 0; i < to->localDirs.size(); i++) {
      remoteName = to->localDirs.at(i); // make sure all our slashes are going
                                        // the way we will be searching for
      remoteName.remove(0, tmpDir.length() +
                               1); // strip the tmpDir path from the beginning
      if (this->getFormat() == 0) {
        remoteName.replace("/", "/ext/");
      }
      if (!(QString(sysInfo.selNode->name).endsWith("ext")))
        remoteName.prepend("/ext/").prepend(sysInfo.selNode->name);
      else
        remoteName.prepend("/").prepend(sysInfo.selNode->name);
      to->remoteDirs.push_back(QDir::toNativeSeparators(remoteName));
      if (this->getFormat() == 0) {
        to->remoteDirs.push_back(
            QDir::toNativeSeparators(remoteName.append("/ext")));
      }
    }
    for (int i = 0; i < to->localFiles.size(); i++) {
      QString mid;
      remoteName = to->localFiles.at(i);
      remoteName.remove(0, tmpDir.length() + 1);
      if (this->getFormat() == 0) {
        mid = remoteName.left(remoteName.lastIndexOf(
            "/")); // we don't want to replace the last "/" with "/ext/"
        mid.replace("/", "/ext/");
        remoteName = mid.append(remoteName.right(remoteName.length() -
                                                 remoteName.lastIndexOf("/")));
      }
      if (!(QString(sysInfo.selNode->name).endsWith("ext")))
        remoteName.prepend("/ext/").prepend(sysInfo.selNode->name);
      else
        remoteName.prepend("/").prepend(sysInfo.selNode->name);
      to->remoteFiles.push_back(remoteName);
    }
    for (int i = 0; i < to->remoteDirs.size(); i++) {
      fprintf(stderr, "%s\n", to->remoteDirs.at(i).toStdString().c_str());
    }
    for (int i = 0; i < to->remoteFiles.size(); i++) {
      fprintf(stderr, "%s-->%s\n", to->localFiles.at(i).toStdString().c_str(),
              to->remoteFiles.at(i).toStdString().c_str());
    }

    // test if we have permission to write to the directory we are trying to
    // import into
    remoteName = to->remoteDirs.at(0);
    remoteName = remoteName.left(remoteName.lastIndexOf("/"));
 
    // stat remoteName, if it doesn't even exist then check the parent
    // we need write permissions on remoteName, or if it doesn't exist write
    // permissions on the parent
    RA_Stat_Struc stat_struct;
    int stat_ret = RA::Stat(sysInfo.connection,
                            remoteName.toStdString().c_str(), &stat_struct);
    if (stat_ret == 0) {
      if (!stat_struct.writeable) {
        // throw a fatal error message, unable to write to directory
        QMessageBox::critical(
            this, QString("Unable to write to destination"),
            QString("Unable to write to remote directory:\n") + remoteName,
            QMessageBox::Abort, QMessageBox::Abort);
        delete_recursive(tmpDir.toStdString().c_str());
        return 0;
      }
    } else {
      // the "/ext" directory did not exist where we just tried, let's check out
      // the parent directory
      remoteName = remoteName.left(remoteName.lastIndexOf(
          "/")); // strip the last /abcdefg bit off the string
      stat_ret = RA::Stat(sysInfo.connection, remoteName.toStdString().c_str(),
                          &stat_struct);
      if (stat_ret == 0) {
        if (!stat_struct.writeable) {
          // throw a fatal error message
          QMessageBox::critical(
              this, QString("Unable to write to destination"),
              QString("Unable to write to remote directory:\n") + remoteName,
              QMessageBox::Abort, QMessageBox::Abort);
          delete_recursive(tmpDir.toStdString().c_str());
          return 0;
        } else {
          // make the "/ext" directory for us to write into
          remoteName.append("/ext");
          if (RA::Mkdir(sysInfo.connection, remoteName.toStdString().c_str(),
                        0755) != 0) {
            // throw a fatal error
            QMessageBox::critical(
                this, QString("Unable to create destination directory"),
                QString("Unable to create remote directory:\n" + remoteName),
                QMessageBox::Abort, QMessageBox::Abort);
            delete_recursive(tmpDir.toStdString().c_str());
            return 0;
          }
        }
      } else { // this is very bad, because this directory existed just a second
               // ago when it was picked....
        // also a fatal error
        QMessageBox::critical(this, QString("Unable to access destination"),
                              QString("Remote directory does not exist:\n") +
                                  remoteName,
                              QMessageBox::Abort, QMessageBox::Abort);
        delete_recursive(tmpDir.toStdString().c_str());
        return 0;
      }
    }
    // now we are sure the "<target>/ext" directory exists and that we have
    // write permissions on it we need to check for the "<target>/ext/<imported
    // object>" directory, if it already exists we need to gracefully handle this
    // also this test should be done for all recursive directories and files
    // within the object tree

    // check if there are 0 or more conflicts so we only show this when
    // necessary
    for (int i = 0; i < to->remoteFiles.size(); i++) {
      to->fileFlags.push_back("");
    }
    for (int i = 0; i < to->remoteDirs.size(); i++) {
      to->dirFlags.push_back("");
    }
    this->grabConflicts(to);
    bool conflict = false;
    for (int i = 0; i < to->remoteFiles.size(); i++) {
      if (to->fileFlags.at(i).contains("C"))
        conflict = true;
    }
    if (conflict) {

      // warning dialog
      if (QMessageBox::warning(
              this, QString("The destination object already exists"),
              QString("The destination object :\n") + to->remoteDirs.at(0) +
                  QString(
                      "\nAlready exists.\n\nIf you press \"Ok\" then a merge "
                      "window will be presented to resolve the conflicts"),
              QMessageBox::Cancel | QMessageBox::Ok,
              QMessageBox::Cancel) == QMessageBox::Ok) {

        conflictWindow = new FileConflict(this, to, remoteName);
        conflictWindow->setWindowTitle("File Conflict");
        int ret = conflictWindow->exec();
        if (ret == 0) {
          delete_recursive(tmpDir.toStdString().c_str());
          return 0;
        }
      } else {
        delete_recursive(tmpDir.toStdString().c_str());
        return 0;
      }
    }
     // make all directories
    for (int i = 0; i < to->remoteDirs.size(); i++) {
      // these directories are sorted in order of increasing depth so we can
      // traverse this linearly and not worry about trying to make a directory
      // whose parent does not exist
      int stat_ret =
          RA::Stat(sysInfo.connection,
                   to->remoteDirs.at(i).toStdString().c_str(), &stat_struct);
      // does the directory already exist?
      if ((stat_ret == 0) && (stat_struct.type != RA_NOEXIST_TYPE)) {
        if (!stat_struct.writeable) {
          // throw a fatal error message, unable to write to directory, but ONLY
          // if it exists
          QMessageBox::critical(
              this, QString("Unable to write to destination"),
              QString("Unable to write to remote directory:\n") +
                  to->remoteDirs.at(i),
              QMessageBox::Abort, QMessageBox::Abort);
          delete_recursive(tmpDir.toStdString().c_str());
          return 0;
        }
      } else {
        if (RA::Mkdir(sysInfo.connection,
                      to->remoteDirs.at(i).toStdString().c_str(), 0755) != 0) {
          // throw a fatal error
          QMessageBox::critical(
              this, QString("Unable to create destination"),
              QString("Unable to create remote directory:\n") +
                  to->remoteDirs.at(i),
              QMessageBox::Abort, QMessageBox::Abort);
          delete_recursive(tmpDir.toStdString().c_str());
          return 0;
        }
      }
    }

    // transfer the files (pre-dos2unix them if necessary)
    for (int i = 0; i < to->remoteFiles.size(); i++) {
      if (this->getFormat() == 0) {
        // we need to run dos2unix on these files first before we send them
        // .map, icon and .mat files are excluded from having dos2unix run on
        // them
        if (!to->localFiles.at(i).endsWith(".mat", Qt::CaseInsensitive) &&
            !to->localFiles.at(i).endsWith("icon", Qt::CaseInsensitive) &&
            !to->localFiles.at(i).endsWith(".map", Qt::CaseInsensitive)) {
          // run dos2unix on the file
          QByteArray temp = to->localFiles.at(i).simplified().toLocal8Bit();
          QList<QByteArray> list;
          list << "dos2unix"
               << "-q" << temp;
          char *argv[3];
          for (int j = 0; j < 3; j++) {
            argv[j] = list[j].data();
          }
          int ret = dos2unix(3, argv);
          if (ret != 0) {
            QMessageBox::critical(this, QString("Error converting file"),
                                  QString("Unable to convert file:\n") +
                                      to->localFiles.at(i) +
                                      QString("\n to UNIX format"),
                                  QMessageBox::Abort, QMessageBox::Abort);
            delete_recursive(tmpDir.toStdString().c_str());
            return 0;
          }
        }
      }
      if (RA::Put_file(to->localFiles.at(i).toStdString().c_str(),
                       sysInfo.connection,
                       to->remoteFiles.at(i).toStdString().c_str()) != 0) {
        QMessageBox::critical(this, QString("Error writing file"),
                              QString("Unable to copy local file:\n") +
                                  to->localFiles.at(i) + QString("\n") +
                                  QString("To remote file:\n") +
                                  to->remoteFiles.at(i),
                              QMessageBox::Abort, QMessageBox::Abort);
        delete_recursive(tmpDir.toStdString().c_str());
        return 0;
      }
    }
    // prototype the object recursively
    for (int i = 0; i < to->remoteDirs.size(); i++) {
      if (!to->remoteDirs.at(i).endsWith("ext")) {
        if (RA::Prototype_object(sysInfo.connection,
                                 to->remoteDirs.at(i).toStdString().c_str()) !=
            0) {
          QMessageBox::critical(this, QString("Error prototyping object"),
                                QString("Unable to prototype object:\n") +
                                    to->remoteDirs.at(i) + QString("\n"),
                                QMessageBox::Abort, QMessageBox::Abort);
          delete_recursive(tmpDir.toStdString().c_str());
          return 0;
        }
      }
    }
    delete_recursive(tmpDir.toStdString().c_str());
  }
  delete to;
  return 0;
}

void Import::receiveTransferObject(TransferObject *inObject) {
  to = inObject;
  fprintf(stderr, "received the transfer object\n");
}

void Import::grabConflicts(TransferObject *to) {
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
          } 
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
          }
        } 
        to->fileFlags.replace(i, temp);
      }
    }
  }
}
