#include <QColorDialog>
#include <QDialog>

#include "CGroupSettingsDialog.h"
#include "utils.h"
#include "CConfigurator.h"
#include "CGroup.h"
#include "mainwindow.h"

CGroupSettingsDialog::CGroupSettingsDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);                        
    connect(pushButton_changeColour, SIGNAL(clicked()), this, SLOT(selectColor()) );
}

CGroupSettingsDialog::~CGroupSettingsDialog()
{
}

void CGroupSettingsDialog::run()
{
	color = conf->getGroupManagerColor();
	lineEdit_charName->setText( conf->getGroupManagerCharName() );
	lineEdit_remoteHost->setText( conf->getGroupManagerHost() );
	lineEdit_localPort->setText( QString("%1").arg( conf->getGroupManagerLocalPort() ) );
	lineEdit_remotePort->setText( QString("%1").arg( conf->getGroupManagerRemotePort() ) );
}

void CGroupSettingsDialog::accept()
{
	CGroup *group = renderer_window->getGroupManager();
	
	QString s;
	s = lineEdit_charName->text();
	if (s != conf->getGroupManagerCharName() ) {
		conf->setGroupManagerCharName(s.toAscii());
		group->resetName();
	}

	if (color != conf->getGroupManagerColor()) {
		conf->setGroupManagerColor(color);
		group->resetColor();
	}
	
	QByteArray remoteHost = lineEdit_remoteHost->text().toAscii();
	int remotePort = lineEdit_remotePort->text().toInt();
	if (remotePort != conf->getGroupManagerRemotePort() || 
		remoteHost != conf->getGroupManagerHost()	) {
		
		conf->setGroupManagerHost(remoteHost);
		conf->setGroupManagerRemotePort(remotePort);
		
		if (group->isConnected()) 
			group->reconnect();
	}

	print_debug(DEBUG_GROUP, "Changing local port");

	int localPort = lineEdit_localPort->text().toInt();
	if (localPort != conf->getGroupManagerLocalPort() ) {
		print_debug(DEBUG_GROUP, "Changing local port 2");
		conf->setGroupManagerLocalPort(localPort);
		print_debug(DEBUG_GROUP, "Changing local port 3");

		if (group->getType() == CGroupCommunicator::Server)
			group->reconnect();
		
		print_debug(DEBUG_GROUP, "Changing local port 4");

	}
	
    done(Accepted);
}


void CGroupSettingsDialog::selectColor() 
{
    QColor newColor = QColorDialog::getColor(color, this);
    if (newColor.isValid()) {
        color = newColor;
        print_debug(DEBUG_GROUP, "color selected: %s",(const char*)color.name().toAscii());
        /*
        colorLabel->setText(color.name());
        colorLabel->setPalette(QPalette(color));
        colorLabel->setAutoFillBackground(true);
        */
    }
}
