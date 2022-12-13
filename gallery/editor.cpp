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

using std::string;

void Editor::launch(string filename, SavingMode savingMode, int posx,
                    int posy) {
  string new_command = command;
  if ((new_command.compare("funcedit ") == 0) ||
      (new_command.compare("cuspy ") == 0))
    new_command += " -wp " + std::to_string(posx) + " " + std::to_string(posy);
  if (savingMode == OFF) {
    new_command += " \"" + filename + "\" &" ;
  }
  else if  (savingMode == CONTINUOUS){
    new_command += " -rmode cont \"" +  filename + "\" &" ;
  }
  else  {
    new_command += " -rmode trig \""  +  filename + "\" &";
  }
   // try to vfork
  QString cmd(new_command.c_str());
  process = new QProcess();
  process->start(cmd);
  return;
  /*
  pid_t pid = vfork();

  if (pid == -1) {
    QString message =  QString("object: Cannot execute '%1' because fork() failed.").arg(cmd);
    qWarning("%s",message.toStdString().c_str());
  } else if (pid == 0) {
    QByteArray ba = cmd.toLatin1();

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

FuncEditor::FuncEditor() { command = "funcedit "; process = NULL;}

ConEditor::ConEditor() { command = "cuspy "; process = NULL;}

