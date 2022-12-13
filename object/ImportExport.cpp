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



#include "ImportExport.h"
#include "dirList.h"
#include "object.h"
#include "ui_ImportExport.h"
#include <QDesktopServices>
#include <QFileInfo>
#include <QProcess>
#include <QSettings>
#include <algorithm>
#include <iostream>
#include <sstream>

ImportExport::ImportExport(QWidget *parent, QString objName, QString basePath,
                           int baseArchiveType)
    : QDialog(parent), ui(new Ui::ImportExport) {
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
  ui->directory_2->setDuplicatesEnabled(false);
  ui->directory_2->setInsertPolicy(QComboBox::InsertAtTop);
  ui->directory_2->setEditText(exportPath);
  setLineEdit();
  settingsFile = QString(QDir::homePath()) + "/.vlab/lpfgsettings.ini";
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

void ImportExport::selectingPath(const QString &text) {
 exportPath = text;
}

void ImportExport::loadSettings() {
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

void ImportExport::writeSettings() {
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

void ImportExport::closeEvent(QCloseEvent *) {
  writeSettings();
}

ImportExport::~ImportExport() { delete ui; }

void ImportExport::changeEvent(QEvent *e) {
  QDialog::changeEvent(e);
  switch (e->type()) {
  case QEvent::LanguageChange:
    ui->retranslateUi(this);
    break;
  default:
    break;
  }
}

void ImportExport::setNodeName() {
  int ext_size = 4;
  if (previousOutputFormat == 0)
    ext_size = 0;
  nodeName = ui->textEdit->toPlainText();
  nodeName.chop(ext_size);
}

void ImportExport::preserveFormat() {
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

void ImportExport::preserveExtension() {
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

void ImportExport::changeExtension(int inValue) {
  setNodeName();
  this->setLineEdit();
  previousOutputFormat = inValue;
}

int ImportExport::getType() {
  return ui->comboBox->currentIndex();
}

int ImportExport::getFormat() { return ui->comboBox_4->currentIndex(); }

void ImportExport::setFormat(int inValue) {
  setNodeName();
  ui->comboBox_4->setCurrentIndex(inValue);
}

void ImportExport::setRecursive(bool ) {}

void ImportExport::setType(int inValue) {
  ui->comboBox->setCurrentIndex(inValue);
}

QString ImportExport::getPath() { return ui->directory_2->currentText(); }

void ImportExport::setPaths(QStringList inPaths) {
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

void ImportExport::setLineEdit() {
  QString formatedNodeName =
      QString("<span style= color:#000000;> %1</span>").arg(nodeName);
  QString ext =
      QString("<span style= color:#999999;>%1</span>").arg(getExtension());
  ui->textEdit->setHtml("<span style= color:#000000;>" + formatedNodeName +
                        "</span>" + ext);

}

QString ImportExport::getExtension() {
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

void ImportExport::browse() {
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
    // changeExtension();
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

void ImportExport::ok() {
  // we don't need to test the result() attribute of this object since
  // closeEvent() is only called when accepted but if you like you can test: if
  // (result() == QDialog::accepted)
  int ret;
  ret = this->Export();
  if (ret == 0)
    accept();
}

int ImportExport::Export() {

  setNodeName();
  saveName = exportPath + "/" + nodeName;
  if (ui->comboBox->currentIndex() == 1)
    saveName = saveName + QString(".tgz");
  if (ui->comboBox->currentIndex() == 2)
    saveName = saveName + QString(".zip");

  to = new TransferObject();
  // get the list of files on the table
  FileList tableList = dirList2(obj.tmpDir);
  for (unsigned int i = 0; i < tableList.size(); i++) {
    to->remoteDirs << nodeName;
    to->remoteFiles << QString(tableList[i].c_str());
  }

  QString tmpDir = "/tmp";
  tmpDir.append("/exportTemp");
  QByteArray tmpDirBytes = tmpDir.toLatin1();
  if (access(tmpDirBytes.data(), F_OK) == 0)
    // delete_recursive(tmpDir);
    delete_recursive(tmpDirBytes);
  if (mkdir(tmpDirBytes.data(), 0755)) {
    QMessageBox::critical(this, tr("Error creating directory"),
                          tr("Unable to create directory:\n") + tmpDir,
                          QMessageBox::Ok, QMessageBox::Ok);
    return 0;
  }
  if (access(tmpDirBytes.data(), W_OK)) {
    QMessageBox::critical(this, tr("Error writing to directory directory"),
                          tr("Unable to write to directory:\n") + tmpDir,
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
    localName = to->remoteDirs.at(i);
    localName.remove(0, nodeName.length());
    localName.prepend(tmpDir);
    localNameBytes = QDir::toNativeSeparators(localName).toLatin1();
    if (access(localNameBytes.data(), F_OK)) // if the directory does not exist
      if (mkdir(localNameBytes.data(), 0755)) { // then make it
        QMessageBox::critical(this, tr("Error creating directory"),
                              tr("Unable to create directory:\n") + localName,
                              QMessageBox::Abort, QMessageBox::Abort);
        delete_recursive(tmpDirBytes.data());
        return 0;
      }
    if (access(localNameBytes.data(), W_OK)) {
      QMessageBox::critical(this, tr("Error writing to directory"),
                            tr("Unable to write to directory:\n") + localName,
                            QMessageBox::Abort, QMessageBox::Abort);
      delete_recursive(tmpDirBytes.data());
      return 0;
    }
  }
  for (int i = 0; i < to->remoteFiles.size(); i++) {
    localName = to->remoteFiles.at(i);
    localName.remove(0, nodeName.length());
    localName = QDir::fromNativeSeparators(localName);
    if (this->getFormat() == 0) {
      localName.replace("/ext", "");
    }
    localName.prepend(tmpDir);
    localNameBytes = QDir::toNativeSeparators(localName).toLatin1();
    QString sysCommand = QString("cp -R ")
                             .append(QString(obj.tmpDir.c_str()))
                             .append("/")
                             .append(to->remoteFiles.at(i))
                             .append(" ")
                             .append(tmpDir);
    fprintf(stderr, "Executing system call: %s\n",
            sysCommand.toStdString().c_str());
    QProcess process;
    int status = process.execute(sysCommand);
    if (status < 0) {
      QMessageBox::critical(
          this, tr("Error copying package"),
          tr("Unable to copy package to Temporary directory\n"),
          QMessageBox::Abort, QMessageBox::Abort);
      delete_recursive(tmpDirBytes.data());
      return 0;
    }

    // the else in this case is success... yes, it is kind of a backwards way of
    // writing this function
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
          QMessageBox::critical(this, tr("Error converting file"),
                                tr("Unable to convert file:\n") + localName +
                                    "\nTo DOS format",
                                QMessageBox::Abort, QMessageBox::Abort);
          delete_recursive(tmpDirBytes.data());
          return 0;
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
  QProcess process;
  switch (this->getType()) {
  case 0: // directory
    fprintf(stderr, "Copying object to target directory: %s\n",
            this->getPath().append(target).toStdString().c_str());
    target.append("/");
    localNameBytes = QDir::toNativeSeparators(target).toLatin1();
    if (access(localNameBytes.data(), F_OK))
      mkdir(target.toStdString().c_str(), 0755);
    else {
      // some kind of popup telling the user they are writing into an existing
      // directory
      ret = QMessageBox::warning(
          this, tr("Target directory exists"),
          tr("The target directory: ") + target +
              "\nAlready exists, all contents will be lost if you continue",
          QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
      if (ret == QMessageBox::Cancel) {
        delete_recursive(tmpDirBytes.data());
        return -1;
      } else {
        QByteArray targetBytes = target.toLocal8Bit();
        delete_recursive(targetBytes);
        mkdir(target.toStdString().c_str(), 0755);
      }
    }
    sysCommand = QString("cp -R ./* \"")
                     .append(tmpDir)
                     .append("/\" \"")
                     .append(target)
                     .append("\"");
    fprintf(stderr, "Executing system call: %s\n",
            sysCommand.toStdString().c_str());
    status = process.execute(sysCommand);
    if (status < 0) {
      QMessageBox::critical(
          this, tr("Error copying package"),
          tr("Unable to copy package to destination directory:\n") + target,
          QMessageBox::Abort, QMessageBox::Abort);
      delete_recursive(tmpDirBytes.data());
      return 0;
    }
    break;
  case 1: // tarred gzip
    fprintf(stderr, "Creating archive %s\n", target.toStdString().c_str());
    localNameBytes = QDir::toNativeSeparators(target).toLatin1();
    fprintf(stderr, "Executing system call %s from directory %s\n",
            sysCommand.toStdString().c_str(), tmpDir.toStdString().c_str());
    if (!access(localNameBytes.data(),
                F_OK)) { // we want an error here.. no error means it exists
      ret = QMessageBox::warning(
          this, tr("Target file exists"),
          tr("The target file: ") + target +
              "\nAlready exists, it will be overwritten if you continue",
          QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
      if (ret == QMessageBox::Cancel) {
        delete_recursive(tmpDirBytes.data());
        return -1;
      } else {
        unlink(target.toStdString().c_str());
      }
    }
    target.prepend("\"").append("\" ");
    sysCommand = QString("tar -czf ")
                     .append(target)
                     .append(" -C ")
                     .append(tmpDir)
                     .append(" .");
    status = process.execute(sysCommand);

    if (status < 0) {
      QMessageBox::critical(this, tr("Error creating archive"),
                            tr("Unable to create archive:\n") +
                                target.left(target.length() - 2),
                            QMessageBox::Abort, QMessageBox::Abort);
      delete_recursive(tmpDirBytes.data());
      return 0;
    }
    break;
  case 2: // zip archive
    fprintf(stderr, "Creating archive %s.zip\n",
            this->getPath().append(target).toStdString().c_str());
    localNameBytes = QDir::toNativeSeparators(target).toLatin1();
    fprintf(stderr, "Executing system call %s from directory %s\n",
            sysCommand.toStdString().c_str(), tmpDir.toStdString().c_str());
    if (!access(localNameBytes.data(), F_OK)) {
      ret = QMessageBox::warning(
          this, tr("Target file exists"),
          tr("The target file: ") + target +
              "\nAlready exists, it will be overwritten if you continue",
          QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
      if (ret == QMessageBox::Cancel) {
        delete_recursive(tmpDirBytes.data());
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
      QMessageBox::critical(this, tr("Error creating archive"),
                            tr("Unable to create archive:\n") +
                                target.left(target.length() - 2),
                            QMessageBox::Abort, QMessageBox::Abort);
      delete_recursive(tmpDirBytes.data());
      return 0;
    }
    break;
  }
  chdir("/");
  delete_recursive(tmpDirBytes.data()); // clean up now that we are done
  delete to;
  return 0;
}

void ImportExport::receiveTransferObject(TransferObject *inObject) {
  to = inObject;
  fprintf(stderr, "received the transfer object\n");
}
