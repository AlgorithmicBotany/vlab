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
#include <QUrl>
#include <QFile>
 #include <QTextStream>
#include "FixOofsDialog.h"
#include "ui_FixOofsDialog.h"
#include "main.h"
#include "RA.h"
#include "xutils.h"
#include "openNode.h"
#include "buildTree.h"
#include "graphics.h"
#include "qtsupport.h"

FixOofsDialog::FixOofsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FixOofsDialog)
{
    ui->setupUi(this);
    QString msg;
    for( double o = 0.9 ; o > 0.2 ; o *= 0.9 )
	msg += QString("<span style=\"opacity: %1;\">Results will be displayed here.</span><br>")
	    .arg( o );
    ui-> webView-> setHtml( msg );
    ui-> gridLayout_2-> setSpacing( 6 );
    connect( ui-> goButton, SIGNAL( clicked()),
	     this, SLOT( goCB()));
    connect( ui-> saveButton, SIGNAL( clicked()),
	     this, SLOT( saveCB()));
}

FixOofsDialog::~FixOofsDialog()
{
    delete ui;
}

void FixOofsDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void FixOofsDialog::linkClickedCB( const QUrl & url )
{

    QString s = url.toString();

    if( ! url.toString().startsWith( "pos: ")) {
	return;
    }
    QString path = url.toString().mid( 5);

    QString msg = QString( "%1@%2:%3" ).arg( sysInfo.login_name).arg( sysInfo.host_name)
	.arg( path);
    node_position( msg.toStdString().c_str());
    build_tree();
    sysInfo.mainForm-> update_menus();
    centre_node( sysInfo.selNode);
    sysInfo.mainForm-> updateDisplay();
}

void FixOofsDialog::goCB()
{
    bool renumber = ui-> renumberCheckBox-> isChecked();
    std::string log = RA::fixOofs(
	sysInfo.connection,
	sysInfo.oofs_dir_rp,
	renumber );
    std::string log_mess = log + "\n";
    _html = QString(log_mess.c_str());
    ui-> webView-> setHtml( _html );

    // tell browsers we changed UUID table
    if (sysInfo.connection->reconnect())
      return;

    QByteArray loginData = sysInfo.login_name.toLatin1();
    const char *loginChar = loginData.constData();

    sysInfo.vlabd-> va_send_message(
	UUIDTABLECHANGED,
	"%s@%s:%s",
	loginChar,
	sysInfo.host_name,
	sysInfo.oofs_dir);
  sysInfo.connection->Disconnect();

}

void FixOofsDialog::saveCB()
{
    QString fname = QString( sysInfo.database ) + "/.fixoofs_log.html";
    QFile f( fname );
    if( f.open( QFile::WriteOnly ))
    {
	QTextStream out ( & f );
	out << _html;
    }
    vlabxutils::infoBox( this, "Saved log to<br>" + fname.toStdString() );
}

