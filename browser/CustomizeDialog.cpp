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



#include <iostream>
#include <QPainter>
#include "BrowserSettings.h"
#include "ColorButton.h"
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <QFontDialog>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSlider>
#include <QSpinBox>

#include "CustomizeDialog.h"
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QKeyEvent>
#include <QModelIndexList>
#include <QTreeWidgetItem>
#include <algorithm>

CustomizeDialog::CustomizeDialog(QWidget *parent, const char*, bool modal,
                                 Qt::WindowFlags fl)
    : QDialog(parent, fl) {
  QPoint parentPos = parent->pos();
  this->move(parentPos.x() + parent->width(), parentPos.y());
  setModal(modal);
  ui.setupUi(this);
  setup_color_button(ui.background_cbutton, "Background");
  setup_color_button(ui.line_cbutton, "Line");
  setup_color_button(ui.selection_cbutton, "Selection");
  setup_color_button(ui.text_cbutton, "Text");
  setup_color_button(ui.bborder_cbutton, "Box border");
  setup_color_button(ui.bfill_cbutton, "Box fill");
  setup_color_button(ui.hborder_cbutton, "HBox border");
  setup_color_button(ui.hfill_cbutton, "HBox fill");
  setup_color_button(ui.link_cbutton, "Link");
  ui.iconSize_slider->setRange(2, 500);
  ui.iconSize_spin->setRange(2, 500);

  // read in the current settings
  _bset.loadCurrent();

  renderSettings();

  // connect signals/slots
  QPushButton *okButton = ui.buttonBox->button(QDialogButtonBox::Ok);
  connect(okButton, SIGNAL(clicked()), SLOT(save_cb()));
  QPushButton *cancelButton = ui.buttonBox->button(QDialogButtonBox::Cancel);
  connect(cancelButton, SIGNAL(clicked()), SLOT(loadSettings_cb()));
  QPushButton *defaultButton =
      ui.buttonBox->button(QDialogButtonBox::RestoreDefaults);
  connect(defaultButton, SIGNAL(clicked()), SLOT(loadDefaults_cb()));
  QPushButton *resetButton = ui.buttonBox->button(QDialogButtonBox::Reset);
  connect(resetButton, SIGNAL(clicked()), SLOT(loadSettings_cb()));
  connect(ui.font_button, SIGNAL(clicked()), SLOT(font_cb()));
  connect(ui.compacting_checkbox, SIGNAL(toggled(bool)), SLOT(rereadWidgets()));
  connect(ui.centerParent_checkbox, SIGNAL(toggled(bool)),
          SLOT(rereadWidgets()));
  connect(ui.iconSize_slider, SIGNAL(valueChanged(int)),
          SLOT(iconSize_cb(int)));
  connect(ui.iconSize_spin, SIGNAL(valueChanged(int)), SLOT(iconSize_cb(int)));

  setFocusPolicy(Qt::WheelFocus);
  setFixedSize(sizeHint());
}

// convenience function to attach functionality to all color buttons
// - connects the signal from the button
void CustomizeDialog::setup_color_button(ColorButton *button,
                                         const QString &title) {
  button->setName(title);
  connect(button, SIGNAL(colorChanged()), SLOT(rereadWidgets()));
  connect(button, SIGNAL(rejected()), SLOT(loadSettingsColors_cb()));
}

void CustomizeDialog::set(const BrowserSettings &bset) {
  _bset = bset;
 
  renderSettings();

  emit settingsChanged(_bset);
}

const BrowserSettings &CustomizeDialog::get() { return _bset; }

// readjust the appearance of the dialog based on the current settings
void CustomizeDialog::renderSettings() {
  ui.background_cbutton->setColor(
      _bset.get(BrowserSettings::BackgroundColor).value<QColor>());
  ui.line_cbutton->setColor(
      _bset.get(BrowserSettings::LineColor).value<QColor>());
  ui.selection_cbutton->setColor(
      _bset.get(BrowserSettings::SelectionColor).value<QColor>());
  ui.text_cbutton->setColor(
      _bset.get(BrowserSettings::TextColor).value<QColor>());
  ui.bborder_cbutton->setColor(
      _bset.get(BrowserSettings::BoxBorderColor).value<QColor>());
  ui.bfill_cbutton->setColor(
      _bset.get(BrowserSettings::BoxFillColor).value<QColor>());
  ui.hborder_cbutton->setColor(
      _bset.get(BrowserSettings::HBoxBorderColor).value<QColor>());
  ui.hfill_cbutton->setColor(
      _bset.get(BrowserSettings::HBoxFillColor).value<QColor>());
  ui.link_cbutton->setColor(
      _bset.get(BrowserSettings::LinkColor).value<QColor>());
  ui.font_label->setFont(_bset.get(BrowserSettings::TextFont).value<QFont>());
  ui.compacting_checkbox->setChecked(
      _bset.get(BrowserSettings::Compacting).toBool());
  ui.centerParent_checkbox->setChecked(
      _bset.get(BrowserSettings::CenterParent).toBool());
  ui.iconSize_slider->setValue(_bset.get(BrowserSettings::IconSize).toInt());
  ui.iconSize_spin->setValue(_bset.get(BrowserSettings::IconSize).toInt());

  // prepare the list of available settings
  //    ui.name_box-> clear();
  QStringList list = BrowserSettings::storage().subkeyList("/browser/set");
  list.sort();
   // Load default Oofs
  loadOofsList();

  // Get other preferences
 }

// readjust the appearance of the dialog based on the current settings
void CustomizeDialog::renderSettingsFont(QFont newFont) {
  ui.font_label->setFont(newFont);
  _bset.set(BrowserSettings::TextFont, newFont);
  emit settingsChanged(_bset);
}

// readjust the appearance of the dialog based on the current settings
void CustomizeDialog::renderSettingsColors() {
  ui.background_cbutton->setColor(
      _bset.get(BrowserSettings::BackgroundColor).value<QColor>());
  ui.line_cbutton->setColor(
      _bset.get(BrowserSettings::LineColor).value<QColor>());
  ui.selection_cbutton->setColor(
      _bset.get(BrowserSettings::SelectionColor).value<QColor>());
  ui.text_cbutton->setColor(
      _bset.get(BrowserSettings::TextColor).value<QColor>());
  ui.bborder_cbutton->setColor(
      _bset.get(BrowserSettings::BoxBorderColor).value<QColor>());
  ui.bfill_cbutton->setColor(
      _bset.get(BrowserSettings::BoxFillColor).value<QColor>());
  ui.hborder_cbutton->setColor(
      _bset.get(BrowserSettings::HBoxBorderColor).value<QColor>());
  ui.hfill_cbutton->setColor(
      _bset.get(BrowserSettings::HBoxFillColor).value<QColor>());
  ui.link_cbutton->setColor(
      _bset.get(BrowserSettings::LinkColor).value<QColor>());

  // prepare the list of available settings
  QStringList list = BrowserSettings::storage().subkeyList("/browser/set");
  list.sort();
}

// private slot:
// extract the current value from the widgets
// emit settingsChanged() signal
void CustomizeDialog::rereadWidgets() {
  _bset.set(BrowserSettings::BackgroundColor,
            ui.background_cbutton->getColor());
  _bset.set(BrowserSettings::LineColor, ui.line_cbutton->getColor());
  _bset.set(BrowserSettings::SelectionColor, ui.selection_cbutton->getColor());
  _bset.set(BrowserSettings::TextColor, ui.text_cbutton->getColor());
  _bset.set(BrowserSettings::BoxBorderColor, ui.bborder_cbutton->getColor());
  _bset.set(BrowserSettings::BoxFillColor, ui.bfill_cbutton->getColor());
  _bset.set(BrowserSettings::HBoxBorderColor, ui.hborder_cbutton->getColor());
  _bset.set(BrowserSettings::HBoxFillColor, ui.hfill_cbutton->getColor());
  _bset.set(BrowserSettings::LinkColor, ui.link_cbutton->getColor());
  _bset.set(BrowserSettings::Compacting, ui.compacting_checkbox->isChecked());
  _bset.set(BrowserSettings::CenterParent,
            ui.centerParent_checkbox->isChecked());
  _bset.set(BrowserSettings::IconSize, ui.iconSize_spin->value());
  emit settingsChanged(_bset);
}

// private slot:
// connected to the <Save As> button
// - asks the user for a new name of the settings
// - saves the new setting
// - make the new setting the current setting
void CustomizeDialog::saveAs_cb() {
  QString name = _bset.name();
  while (1) {
    // ask the user for a new name
    bool ok = false;
    name = QInputDialog::getText(this, "Save settings...",
                                 "Enter new name for these settings",
                                 QLineEdit::Normal, name, &ok);
    // if user canceled, return
    if (!ok)
      return;
    // remove leading and trailing spaces
    name = name.simplified();
    // warn for overwrite
    if (BrowserSettings::storage().subkeyList("/browser/set").indexOf(name) !=
        -1) {
      int res = QMessageBox::warning(
          this, "Warning", "This will overwrite existing settings. Coninue?",
          "Yes", "No");
      if (res == 0)
        break;
      else
        continue;
    }
    // reject a bad name
    bool bad_name = false;
    if (name.isEmpty()) {
      bad_name = true;
    } else {
      // ... put more checks here...?
    }
    if (bad_name) {
      QMessageBox::warning(this, "Warning", "That's not a good name.");
      continue;
    } else {
      break;
    }
  }
  // save the settings with the new name
  _bset.setName(name);
  _bset.write();
  BrowserSettings::storage().writeEntry("/browser/current", _bset.name());

  renderSettings();

  emit settingsChanged(_bset);
}

// private slot:
// connected to the <Save> button
void CustomizeDialog::save_cb() {
  _bset.write();
  apply();
}

// private slot:
// connected to the delete button
// - deletes the current settings
// - if no settings are left, inserts a default settings
// - makes one of the other settings default
void CustomizeDialog::delete_cb() {
  BrowserSettings::storage().removeKey(
      QString("/browser/set/%1").arg(_bset.name()));

  // if the number of remaining settings are 0, insert a default setting
  if (BrowserSettings::storage().subkeyList("/browser/set").size() == 0) {
    _bset.setDefault();
    _bset.setName("default");
    _bset.write();
  }
  // set the default to the first setting we find
  QStringList lst = BrowserSettings::storage().subkeyList("/browser/set");
  if (!lst.isEmpty()) {
    BrowserSettings::storage().writeEntry("/browser/current", lst[0]);
    _bset.setName(lst[0]);
    _bset.load();
  }

  renderSettings();

  emit settingsChanged(_bset);
}

// private slot:
// connected to the combo box
// - load the selected settings
// - set it as default
// - rerender the dialog
// - generate signal to indicate settings have changed
void CustomizeDialog::loadSettings_cb() {
  //    _bset.setName( ui.name_box-> currentText() );
  _bset.load();
  BrowserSettings::storage().writeEntry("/browser/current", _bset.name());
  renderSettings();

  emit settingsChanged(_bset);
}

// private slot:
// connected to the combo box
// - load the selected settings
// - set it as default
// - rerender the dialog
// - generate signal to indicate settings have changed
void CustomizeDialog::loadSettingsColors_cb() {
  //    _bset.setName( ui.name_box-> currentText() );
  _bset.reloadColors();
  BrowserSettings::storage().writeEntry("/browser/current", _bset.name());
  renderSettingsColors();

  emit settingsColorsChanged(_bset);
}

// private slot:
// connected to the 'defaults' button
// - loads teh default settings
void CustomizeDialog::loadDefaults_cb() {
  _bset.setDefault();

  renderSettings();

  emit settingsChanged(_bset);
}

// private slot:
// connected to the 'font' button
// - invokes the font dialog
void CustomizeDialog::font_cb() {
  _currentFont = _bset.get(BrowserSettings::TextFont).value<QFont>();
 
  QFontDialog *qfd = new QFontDialog(this);
  qfd->setOption(QFontDialog::DontUseNativeDialog);
  qfd->setWindowTitle("Select font");
  qfd->setCurrentFont(_currentFont);
  connect(qfd, SIGNAL(currentFontChanged(QFont)),
          SLOT(renderSettingsFont(QFont)));
 
  connect(qfd, SIGNAL(rejected()), this, SLOT(resetToCurrentFont()));
  qfd->show();
}

void CustomizeDialog::resetToCurrentFont() {
  ui.font_label->setFont(_currentFont);
  _bset.set(BrowserSettings::TextFont, _currentFont);
  emit settingsChanged(_bset);
}

void CustomizeDialog::iconSize_cb(int val)
// slot for changes in icon size
{
  ui.iconSize_slider->setValue(val);
  ui.iconSize_spin->setValue(val);
  rereadWidgets();
}

void CustomizeDialog::on_buttonBox_clicked(QAbstractButton *btn) {
  switch (ui.buttonBox->buttonRole(btn)) {
  case QDialogButtonBox::ResetRole:
    loadDefaults_cb();
    break;
  case QDialogButtonBox::RejectRole:
    close();
    break;
  case QDialogButtonBox::ApplyRole:
    apply();
    break;
  case QDialogButtonBox::AcceptRole:
    apply();
    accept();
    break;
  default:
    break;
  }
}

void CustomizeDialog::apply() {
  _bset.setRecentOofsPath(oofsPath);
  _bset.setRecentOofsPasswords(oofsPwd);
}

void CustomizeDialog::loadOofsList() {
  oofsPath = _bset.recentOofsPath();
  oofsPwd = _bset.recentOofsPasswords();
}
