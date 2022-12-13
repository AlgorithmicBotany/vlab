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




#ifndef VLAB_BROWSER__CUSTOMIZE_DIALOG_H__
#define VLAB_BROWSER__CUSTOMIZE_DIALOG_H__

#include "ui_CustomizeDialog.h"

#include "BrowserSettings.h"

class QKeyEvent;
class QAbstractButton;
class QTreeWidgetItem;
class QShowEvent;

class CustomizeDialog : public QDialog
{
    Q_OBJECT

public:
    CustomizeDialog( QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    // set the settings
    void set( const BrowserSettings & bset );
    // read back the settings
    const BrowserSettings & get();
signals:
    void settingsChanged( const BrowserSettings & );
    void settingsColorsChanged( const BrowserSettings & );
    void fontSettingsChanged( const BrowserSettings & );
private:
    // convenience function to attach functionality to all color buttons
    void setup_color_button( ColorButton * button
                             , const QString & title );
    BrowserSettings _bset;
private slots:
    // re-adjusts the appearance of the dialog to reflect the changed settings
    void renderSettings();
    // re-adjusts the appearance of the dialog to reflect the changed settings
    void renderSettingsFont(QFont);
    // re-adjusts the appearance of the dialog Colors to reflect the changed settings
    void renderSettingsColors();
    // general slot for when any of the edit widgets changed anything
    void rereadWidgets();
    // slot for the <Save As...> button
    void saveAs_cb();
    // slot fot the <Save> button
    void save_cb();
    // slot for the delete button
    void delete_cb();
    // slot for the combo box
    void loadSettings_cb();
    // slot for loading the defaults
    void loadSettingsColors_cb();
    // slot for loading the defaults colors
    void loadDefaults_cb();
    // slot for handling font selection
    void font_cb();
    // slot for changes in icon size
    void iconSize_cb( int );
    void resetToCurrentFont();

    
public slots:
    void on_buttonBox_clicked(QAbstractButton *btn);
     
protected:
    void loadOofsList();

    void apply();

private:
    Ui_CustomizeDialog ui;
    QStringList oofsPath, oofsPwd;
    QFont _currentFont;
};

#endif
