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



#include "newbrowser.h"
#include "BrowserSettings.h"
#include "main.h"
#include "parse_object_location.h"
#include "qtsupport.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QUrl>
#include <iostream>

NewBrowserDlg::NewBrowserDlg(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f), default_oofs(), default_password(), _f(f) {
  BrowserSettings settings;
  ui.setupUi(this);
  setModal(true);
  recent_oofs = settings.recentOofsPath();
  recent_passwords = settings.recentOofsPasswords();
  if (!recent_oofs.empty()) {
    default_oofs = recent_oofs.front();
    default_password = recent_passwords.front();
  }
  resetContent();
  foreach (QString oofs, recent_oofs) { ui.recentOofs->addItem(oofs); }
  setAcceptDrops(true);
  activateWindow();
  ui.directory->setFocus();

#ifndef __APPLE__
  setWindowIcon(QIcon(":/linux-icon.png"));
#endif
}

NewBrowserDlg::NewBrowserDlg(const QString &oofs, const QString &password,
                             QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f), default_oofs(oofs), default_password(password) {
  BrowserSettings settings;
  ui.setupUi(this);
  recent_oofs = settings.recentOofsPath();
  recent_passwords = settings.recentOofsPasswords();
  foreach (QString oofs, recent_oofs) { ui.recentOofs->addItem(oofs); }
  resetContent();
  setAcceptDrops(true);
}

void NewBrowserDlg::on_recentOofs_activated(int idx) {
  setContent(recent_oofs[idx], recent_passwords[idx]);
}

void NewBrowserDlg::resetContent() {
  char *login;
  char *host;
  char *object_name;
  bool first_is_local;
  for (int i = 0; i < recent_oofs.size(); ++i) {
    QByteArray recentOofsData = recent_oofs[i].toLatin1();
    const char *recentOofsChar = recentOofsData.constData();

    parse_object_location(recentOofsChar, &login, &host, &object_name);
    bool is_local = std::string(host) == LOCAL_HOST_STR;
    if (i == 0) {
      first_is_local = is_local;
    } else {
      if (is_local != first_is_local) {
        setContent(recent_oofs[i], recent_passwords[i]);
        break;
      }
    }
  }
  setContent(default_oofs, default_password);
}

void NewBrowserDlg::setContent(const QString &oofs, const QString &password) {
  char *login;
  char *host;
  char *object_name;

  parse_object_location(oofs.toStdString(), &login, &host, &object_name);

  if (std::string(host) == LOCAL_HOST_STR) {
    // i.e. local
    ui.oofsType->setCurrentIndex(0);
    ui.directory->setText(object_name);
  } else {
    // i.e. remote
    ui.oofsType->setCurrentIndex(1);
    ui.hostName->setText(host);
    ui.remoteDirectory->setText(object_name);
    if (strlen(login) > 0) {
      ui.hasLogin->setChecked(true);
      ui.loginName->setText(login);
    } else {
      ui.hasLogin->setChecked(false);
      ui.loginName->setText("");
    }
    if (password.isEmpty()) {
      ui.hasPassword->setChecked(false);
      ui.password->setText("");
    } else {
      ui.hasPassword->setChecked(true);
      ui.password->setText(password);
    }
  }
  free(login);
  free(host);
  free(object_name);
}

#include <QTextStream>

QString NewBrowserDlg::oofs() {
  QString oofs;
  switch (ui.oofsType->currentIndex()) {
  case 0:
    oofs = ui.directory->text();
    break;
  case 1:
    oofs =
        QString("%1:%2").arg(ui.hostName->text(), ui.remoteDirectory->text());
    if (ui.hasLogin->isChecked()) {
      oofs = QString("%1@%2").arg(ui.loginName->text()).arg(oofs);
    }
    if (ui.hasPassword->isChecked()) {
      oofs = QString("-p %1 %2").arg(ui.password->text()).arg(oofs);
    }
    break;
  }
  return oofs;
}

void NewBrowserDlg::on_chooseDirectory_clicked() {
  QString temp = ui.directory->text();
  QString dir = QFileDialog::getExistingDirectory(
      this, "Select the OOFS directory", temp,
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  QString path = QDir::toNativeSeparators(dir);
  if (!dir.isEmpty()) {
    ui.directory->setText(dir);
  }
}

void NewBrowserDlg::on_buttonBox_clicked(QAbstractButton *btn) {
  if (ui.buttonBox->buttonRole(btn) == QDialogButtonBox::ResetRole) {
    resetContent();
  }
}

void NewBrowserDlg::dragEnterEvent(QDragEnterEvent *ev) {
  if (ev->mimeData()->hasUrls())
    ev->acceptProposedAction();
}

void NewBrowserDlg::dropEvent(QDropEvent *ev) {
  if (ev->mimeData()->hasUrls()) {
    QString oofs, password;
    QUrl url = ev->mimeData()->urls()[0];
    QString path = url.path();
    QFileInfo fi(path);
    if (fi.isDir()) {
      oofs = path;
    } else {
      QFile f(path);
      if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(
            this, "Error while opening OOFS",
            QString("Cannot open OOFS description file '%1'").arg(path));
        return;
      }
      oofs = f.readAll().trimmed();
      QStringList oofs_parts = oofs.split(' ');
      if (oofs_parts[0] == "-p") {
        if (oofs_parts.size() > 2) {
          password = oofs_parts[1];
          oofs_parts.pop_front();
          oofs_parts.pop_front();
          oofs = oofs_parts.join(" ");
          printf("oofs = '%s'\n", oofs.toLatin1().data());
        } else {
          QMessageBox::critical(this, "Error while opening OOFS",
                                "File content is invalid. Format: [-p "
                                "password] [[login@]machine:]oofs_path");
          return;
        }
      }
    }
    setContent(oofs, password);
    accept();
  }
  ev->acceptProposedAction();
}

void NewBrowserDlg::showEvent(QShowEvent *ev) { QDialog::showEvent(ev); }
