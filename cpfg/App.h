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



#ifndef __APP_H__
#define __APP_H__

class App {
public:
  App() : _hInst(NULL) {}
  void Init(HINSTANCE);
  int Run(char *);
  void SetIdleFunc(IdleFun);
  HINSTANCE HInst() const {
    assert(NULL != _hInst);
    return _hInst;
  }
  static BOOL CALLBACK FilesDlgProc(HWND, UINT, WPARAM, LPARAM);
  void InitFileDlg(HWND);
  void FileDlgCommand(HWND, int, HWND, UINT);
  void BrowseLSystem();
  void BrowseView();
  void BrowseMapMat();

private:
  HINSTANCE _hInst;
  HWND _hWnd;
  HWND _hFileDlg;
  HWND _hLSystemTxt;
  HWND _hViewTxt;
  HWND _hMapMatTxt;
};

#else
#error File already included
#endif
