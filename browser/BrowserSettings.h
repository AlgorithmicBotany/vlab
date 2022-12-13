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




#ifndef __VLAB_BROSER_BROWSER_SETTINGS_H__
#define __VLAB_BROSER_BROWSER_SETTINGS_H__

#include <qvariant.h>
#include <qcolor.h>
#include <qstring.h>
#include "SettingsStorage.h"

class BrowserSettings
{
    // public types
public:
    enum Role { BackgroundColor,
                LineColor,
                SelectionColor,
                TextColor,
                BoxBorderColor,
                BoxFillColor,
                HBoxBorderColor,
                HBoxFillColor,
                LinkColor,
                IconSize,
                Compacting,
                CenterParent,
                TextFont
    };
public:
    // returns the Settings Storage that BrowserSettings uses to write/load
    static SettingsStorage & storage();
    // initialize everything with default settings
    void setDefault();
    // load current settings
    void loadCurrent();
    // load the settings
    bool load();
    // load the settings
    bool reloadColors();
    // write the settings
    void write();
    // set the name of the settings
    void setName( const QString & name ) { _name = name; }
    // return the name of the settins
    const QString & name() const { return _name; }
    // generic setter/getter
    void set( Role, QVariant v );

    QVariant get( Role ) const;
    QColor getColor( Role ) const;

    // Function handling most recent oofs ... they cannot be kept in memory as 
    // many browsers can be opened at the same time.
    // set the last oofs
    void setLastOofs( const QString& path, const QString& password );
    void setRecentOofsPath(const QStringList& lst);
    void setRecentOofsPasswords(const QStringList& lst);
    void setRecentExportPath(const QString& path);
    void setRecentExportFormat(int format);
    void setRecentExportType(int type);
    void setRecentExportRecursive(bool recursive);
    void setRecentExportSymLinks(bool symLinks);
    QStringList recentOofsPath();
    QStringList recentOofsPasswords();
    QStringList recentExportPath();
    int recentExportFormat();
    int recentExportType();
    bool recentExportRecursive();
    bool recentExportSymLinks();

    QString shellCommandLine() const;
    void setShellCommandLine(QString cmd);
    QString editorCommandLine() const;
    void setEditorCommandLine(QString cmd);

private: // data members
    QString _name;
    QColor _backgroundColor,
        _lineColor,
        _selectionColor,
        _textColor,
        _boxBorderColor,
        _boxFillColor,
        _hboxBorderColor,
        _hboxFillColor,
        _linkColor;
    bool _compacting,
        _centerParent;
    int _iconSize;
    QFont _textFont;

};

#endif
