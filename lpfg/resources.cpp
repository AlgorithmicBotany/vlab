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



#include "resources.h"

#include <QString>
#include <QTextStream>
#include <QDir>

#ifdef __APPLE__
#include <Carbon/Carbon.h>

static QTextStream cerr(stderr);

QString urlToQString(CFURLRef url) {
  CFStringRef macPath = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
  const char *pathPtr =
      CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());
  QString result(pathPtr);
  CFRelease(macPath);
  return result;
}

QString getRessourceDirectory() {
  CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
  CFURLRef resUrlRef =
      CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());

  QString result =
      QDir(urlToQString(appUrlRef)).filePath(urlToQString(resUrlRef));
  CFRelease(appUrlRef);
  CFRelease(resUrlRef);
  return result;
}

QString getHelpDirectory() {
  QDir resource(getRessourceDirectory());
  resource.cd("LPFG.help");
  resource.cd("Contents");
  resource.cd("Resources");
  return resource.filePath("English.lproj");
}

void OpenHelpBook() {
  CFBundleRef myApplicationBundle = NULL;
  CFTypeRef myBookName = NULL;
  OSStatus err = noErr;

  myApplicationBundle = CFBundleGetMainBundle();
  if (myApplicationBundle == NULL) {
    err = fnfErr;
    cerr << "Cannot find application bundle" << endl;
    return;
  }

  myBookName = CFBundleGetValueForInfoDictionaryKey(
      myApplicationBundle, CFSTR("CFBundleHelpBookName"));
  if (myBookName == NULL) {
    err = fnfErr;
    cerr << "Cannot find help book name" << endl;
    return;
  }

  if (CFGetTypeID(myBookName) != CFStringGetTypeID()) {
    err = paramErr;
    cerr << "Bookname doesn't contain a string" << endl;
    return;
  }
  cerr << "go to page" << endl;
  cerr << myBookName << endl;
  AHGotoPage((CFStringRef)myBookName, NULL, NULL);
}

#else

#include <QProcess>
#include <QDesktopServices>
#include <QUrl>

QString getRessourceDirectory() {
  QStringList env = QProcess::systemEnvironment();
  QStringList lpfg_dir_lst = env.filter("ObjectRESOURCES=");
  if (!lpfg_dir_lst.empty()) {
    QString lpfg_dir = lpfg_dir_lst[0];
    if (lpfg_dir.startsWith("ObjectRESOURCES=")) {
      lpfg_dir.remove("LObjectSOURCES=");
      if (QFile::exists(lpfg_dir))
        return lpfg_dir;
    }
  }
  return "";
}

QString getHelpDirectory() {
  QStringList env = QProcess::systemEnvironment();
  QStringList lpfg_dir_lst = env.filter("VLABDOCDIR=");
  if (!lpfg_dir_lst.empty()) {
    QString lpfg_dir = lpfg_dir_lst[0];
    if (lpfg_dir.startsWith("VLABDOCDIR=")) {
      lpfg_dir.remove("VLABDOCDIR=");
      if (QFile::exists(lpfg_dir)) {
        //lpfg_dir = lpfg_dir + "/../docs/lpfg/";
        return lpfg_dir;
      }
    }
  }
  return "";
}

void OpenHelpBook() {
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(QDir(getHelpDirectory()).filePath("LPFGManual.pdf")));
}

#endif
