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



#include "readPixmap.h"

#include "RA.h"
#include <QDir>
#include <QImage>
#include <cassert>
#include <qpainter.h>
#include <qpixmap.h>
#include <qstring.h>

#include <QImageReader>
#include <iostream>

#include "qtsupport.h"
#include "sgiFormat.h"
#include <QDebug>
#include <cstring>

// this class makes it simple to use RA::Fetch_file function
// - if the connection is local, it does nothing
// - if the connection is remote, the file is fetched to a temporary file
// (located in tmpdir)
// - at the destruction time the temporary file is automatically removed
class RALocalFile {
public:
  RALocalFile(){}
  RALocalFile(RA_Connection *con, const QString &fname, const QString &tmpdir)
      : _con(con), _remoteFname(fname) {
    _valid = false;
    if (_con->isLocal()) {
      _valid = true;
      return;
    }
    // generate a temporary file
    char buff[4096];
    QByteArray tmpDirData = tmpdir.toLatin1();
    const char *tmpDirChar = tmpDirData.constData();

    sprintf(buff, "%s/vlab_local_pixmapXXXXXXX", tmpDirChar);
    int _fd = mkstemp(buff);
    if (_fd < 0) {
      qWarning("RALocalFile::Cannot create temporary file.");
      return;
    }
    _localFname = QString::fromLatin1(buff);
    // fetch the file
    QByteArray array = fname.toLocal8Bit();
    const char *tmpfname_c = array.constData();
    char fname_c[array.size()];
    strcpy(fname_c, tmpfname_c);

    array = _localFname.toLocal8Bit();
    const char *tmplocalFname_c = array.constData();
    char localFname_c[array.size()];
    strcpy(localFname_c, tmplocalFname_c);

    if (RA::Fetch_file(_con, (const char *)fname_c,
                       (const char *)localFname_c)) {
      // std::cerr << "Warning : Cannot fetch "<<fname_c<<" from raserver."
      //          << std::endl;
      unlink(buff);
      close(_fd);
      return;
    }

    close(_fd);
    _valid = true;
  }
  ~RALocalFile() {
    if (!_valid)
      return;
    if (_con->isLocal())
      return;
    QByteArray localFnameData = _localFname.toLatin1();
    const char *localFNameChar = localFnameData.constData();

    unlink(localFNameChar);
  }
  bool valid() { return _valid; }
  const QString &fname() {
    assert(_valid);
    if (_con->isLocal())
      return _remoteFname;
    else
      return _localFname;
  }

private:
  bool _valid; // constructor succeeded?
  RA_Connection *_con;
  QString _localFname;
  QString _remoteFname;
};

// reads in a pixmap from a file 'fname' through RA 'connection'
// - if max_size is negative, no scaling is performed
// - if max_size is non-negative, the pixmap is scaled so that its maximum
// dimension
//   is at most max_size
// - tmpdir is where a remote file will be stored
QPixmap readPixmap(RA_Connection *con, const QString &fname, int max_size,
                   const QString &tmpdir) {

  // 1st try with icon.png
  QString namePNG = fname+ QString(".png");
  // [PASCAL] uncomment the following line to proirize png icons
  //RALocalFile localPNG(con, namePNG, tmpdir);
  //RALocalFile local = RALocalFile(con, fname, tmpdir);;
  //if ((!localPNG.valid()) && (!local.valid())){
  RALocalFile local = RALocalFile(con, fname, tmpdir);;
  if (!local.valid()){

    QImage img;
    img.load(":/default-icon.png");
    // re-scale if requested
    if (!img.isNull()){
      if (max_size > -1)
	img = img.scaled(max_size, max_size, Qt::KeepAspectRatio,
			 Qt::SmoothTransformation);
      
    //  convert back to pixmap
      return QPixmap::fromImage(img);
    }
  }
  // [PASCAL] uncomment the following line to proirize png icons
  
  if (local.valid()){
    QImage img;
    QString fileName;
    fileName = QString(local.fname());
    img.load(fileName);
    // re-scale if requested
    if (!img.isNull()){
      if (max_size > -1)
	img = img.scaled(max_size, max_size, Qt::KeepAspectRatio,
			 Qt::SmoothTransformation);
    
   
      return QPixmap::fromImage(img);
    }
  }
  

  if (local.valid()){
    QString fileName = QString(local.fname());
    //QImage img(local.fname());
    QImage img = readSGI(fileName.toLatin1());
    if (!img.isNull()){
      if (max_size > -1)
	img = img.scaled(max_size, max_size, Qt::KeepAspectRatio,
		       Qt::SmoothTransformation);
    
      return QPixmap::fromImage(img);
    }

  }
  QImage img;
  img.load(":/default-icon.png");
 
  // re-scale if requested
  if (!img.isNull()){
    if (max_size > -1)
      img = img.scaled(max_size, max_size, Qt::KeepAspectRatio,
                     Qt::SmoothTransformation);
  }
  //  convert back to pixmap
  return QPixmap::fromImage(img);
}
