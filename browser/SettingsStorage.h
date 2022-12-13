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




#ifndef __VLAB_SETTINGS_STORAGE__
#define __VLAB_SETTINGS_STORAGE__

#include <qstring.h>
#include <qcolor.h>
#include <qapplication.h>
#include <QSettings>
#include <QFont>

class SettingsStorage
{
public:
    SettingsStorage( const QString & filename );
    QStringList subkeyList( const QString & key );
    // removes recursively too...
    void removeKey( const QString & key );
    // reader methods:
    QString readEntry( const QString & key, const QString & def = QString::null );
    int     readNumEntry( const QString & key, int def = 0 );
    bool    readBoolEntry( const QString & key, bool def = false );
    double  readDoubleEntry( const QString & key, double def = 0.0 );
    QColor  readColorEntry( const QString & key, const QColor & def = QColor( 0, 0, 0 ) );
    QFont   readFontEntry( const QString & key, const QFont & def = QApplication::font() );
    QStringList readStringListEntry( const QString & key, const QStringList & def = QStringList() );

    // writer methods
    bool writeEntry( const QString & key, const QString & val );
    bool writeEntry( const QString & key, const QStringList & val );
    bool writeEntry( const QString & key, int value );
    bool writeEntry( const QString & key, bool value );
    bool writeEntry( const QString & key, double value );
    bool writeEntry( const QString & key, const QColor & val );
    bool writeEntry( const QString & key, const QFont & val );
private:
    // returns initialized qsettings
    QSettings & qset();
    QSettings _qset;
    // make copy consturctor unavailable
    SettingsStorage( const SettingsStorage & s );
};


#endif
