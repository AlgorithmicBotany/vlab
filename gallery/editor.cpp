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

#include "editor.h"
#include <QByteArray>
#include <QString>
#include <iostream>

using std::string;

void Editor::launch(string filename, SavingMode savingMode, int posx,
                    int posy) {

  QString prog = QString::fromStdString(command).trimmed();
  QStringList args;

  if (command == "funcedit " || command == "cuspy ") {
    args << "-wp" << QString::number(posx) << QString::number(posy);
  }

  if (savingMode == CONTINUOUS) {
    args << "-rmode" << "cont";
  } else if (savingMode != OFF) {
    args << "-rmode" << "trig";
  }

  args << QString::fromStdString(filename);

  process = new QProcess();
  process->start(prog, args);
  if (!process->waitForStarted()) {
    std::cerr << "Failed to start process: "
              << process->errorString().toStdString() << "\n";
  }
  return;
  /*
  pid_t pid = vfork();

  if (pid == -1) {
    QString message =  QString("object: Cannot execute '%1' because fork()
  failed.").arg(cmd); qWarning("%s",message.toStdString().c_str()); } else if
  (pid == 0) { QByteArray ba = cmd.toLatin1();

    execlp("/bin/bash", "/bin/bash", "-c", ba.data(), (char *)NULL);

    qWarning("object:execlp failed.");
    _exit(0);
  } else {
    // fork was created...
    _pid = pid+1;

  }
  //system(new_command.c_str());
  */
}

FuncEditor::FuncEditor() {
  command = "funcedit ";
  process = NULL;
}

ConEditor::ConEditor() {
  command = "cuspy ";
  process = NULL;
}
