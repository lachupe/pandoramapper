#include <QColorDialog>
#include <QDialog>

#include "CGroupSettingsDialog.h"
#include "utils.h"
#include "CConfigurator.h"

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
	
	lineEdit_charName->setText( conf->getGroupManagerCharName() );
	lineEdit_remoteHost->setText( conf->getGroupManagerHost() );
	lineEdit_localPort->setText( QString("%1").arg( conf->getGroupManagerLocalPort() ) );
	lineEdit_remotePort->setText( QString("%1").arg( conf->getGroupManagerRemotePort() ) );
	
}

void CGroupSettingsDialog::accept()
{

    done(Accepted);
}


void CGroupSettingsDialog::selectColor() 
{
    QColor color = QColorDialog::getColor(QColor("#F28003"), this);
    if (color.isValid()) {
        //conf->set_note_color((const char*)color.name().toAscii());
        print_debug(DEBUG_GROUP, "color selected: %s",(const char*)color.name().toAscii());
        /*
        colorLabel->setText(color.name());
        colorLabel->setPalette(QPalette(color));
        colorLabel->setAutoFillBackground(true);
        */
    }
}
