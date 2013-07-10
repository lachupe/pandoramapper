/*
 *  Pandora MUME mapper
 *
 *  Copyright (C) 2000-2009  Azazello
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QMessageBox>
#include <QColorDialog>

#include "CConfigurator.h"

#include "Gui/ConfigWidget.h"

/* ----------------------- AnalyserConfigWidget -----------------------------*/
 
ConfigWidget::ConfigWidget (QWidget *parent) : QDialog(parent)
{
    setupUi(this);                        
    setWindowTitle(tr("General Settings"));
    
    connect(checkBox_autorefresh, SIGNAL(toggled(bool)), this, SLOT(autorefreshUpdated(bool)) );
}


void ConfigWidget::run()
{

    if (conf->getBriefMode()) 
        checkBox_brief->setChecked(true);
    else 
        checkBox_brief->setChecked(false);
              
       
    lineEdit_remoteport->setText(QString("%1").arg(conf->getRemotePort()) );
    lineEdit_remotehost->setText(conf->getRemoteHost());
    
    lineEdit_localport->setText(QString("%1").arg(conf->getLocalPort()) );

    spinBox_namequote->setValue(conf->getNameQuote());
    spinBox_descquote->setValue(conf->getDescQuote());
    if (conf->getAutorefresh()) {
        checkBox_autorefresh->setChecked(true);
        spinBox_namequote->setEnabled(true);
        spinBox_descquote->setEnabled(true);
    } else {
        checkBox_autorefresh->setChecked(false);
        spinBox_namequote->setEnabled(false);
        spinBox_descquote->setEnabled(false);
    }
    
    if (conf->getAutomerge()) 
        checkBox_automerge->setChecked(true);
    else
        checkBox_automerge->setChecked(false);
    
    if (conf->getAngrylinker()) 
        checkBox_angrylinker->setChecked(true);
    else
        checkBox_angrylinker->setChecked(false);

    if (conf->getDuallinker()) 
        checkBox_duallinker->setChecked(true);
    else
        checkBox_duallinker->setChecked(false);


    if (conf->getShowNotesRenderer()) 
        checkShowNotes->setChecked(true);    
    else 
        checkShowNotes->setChecked(false);    

    if (conf->getDisplayRegionsRenderer()) 
        checkShowRegions->setChecked(true);    
    else 
        checkShowRegions->setChecked(false);    

    if (conf->getShowRegionsInfo()) 
        checkShowSecrets->setChecked(true);    
    else 
        checkShowSecrets->setChecked(false);    

    lineEdit_visrange->setText(QString("%1").arg(conf->getTextureVisibility()) );
    lineEdit_detrange->setText(QString("%1").arg(conf->getDetailsVisibility()) );
    lineEdit_layers->setText(QString("%1").arg(conf->getVisibleLayers()) );
    
    misc_startupMode->setCurrentIndex(conf->getStartupMode());
}

void ConfigWidget::autorefreshUpdated(bool state)
{
    if (state) {
        spinBox_namequote->setEnabled(true);
        spinBox_descquote->setEnabled(true);
    } else {
        spinBox_namequote->setEnabled(false);
        spinBox_descquote->setEnabled(false);
    }
}

void ConfigWidget::accept()
{
    int i;
    
    if (conf->getBriefMode() !=  checkBox_brief->isChecked() ) 
        conf->setBriefMode( checkBox_brief->isChecked() );
       
    if (conf->getRemoteHost() != lineEdit_remotehost->text().toLocal8Bit() ) 
        conf->setRemoteHost(lineEdit_remotehost->text().toLocal8Bit());
    
    i = lineEdit_remoteport->text().toInt();
    if (i == 0) {
            QMessageBox::critical(this, "Cofiguration",
                              QString("Bad remote port!"));
            return;    
    }
    if (conf->getRemotePort() != i)
        conf->setRemotePort(i);
        
    
    i = lineEdit_localport->text().toInt();
    if (i == 0) {
            QMessageBox::critical(this, "Cofiguration",
                              QString("Bad local port!"));
            return;    
    }
    if (conf->getLocalPort() != i)
        conf->setLocalPort(i);


    if (conf->getNameQuote() != spinBox_namequote->value())
        conf->setNameQuote( spinBox_namequote->value() ); 
    if (conf->getDescQuote() != spinBox_descquote->value() )        
        conf->setDescQuote( spinBox_descquote->value() ); 
    
    if (conf->getAutorefresh() != checkBox_autorefresh->isChecked())
        conf->setAutorefresh( checkBox_autorefresh->isChecked() );
    
    
    if (conf->getAutomerge() != checkBox_automerge->isChecked()) 
        conf->setAutomerge(checkBox_automerge->isChecked());
    
    if (conf->getAngrylinker() != checkBox_angrylinker->isChecked() )
        conf->setAngrylinker(checkBox_angrylinker->isChecked() );

    if (conf->getDuallinker() != checkBox_duallinker->isChecked() )
        conf->setDuallinker(checkBox_duallinker->isChecked() );

 
    if (conf->getShowRegionsInfo() != checkShowSecrets->isChecked() )
        conf->setShowRegionsInfo( checkShowSecrets->isChecked() );
        
        
    if (conf->getShowNotesRenderer() != checkShowNotes->isChecked() )
        conf->setShowNotesRenderer( checkShowNotes->isChecked() );
        
    if (conf->getDisplayRegionsRenderer() != checkShowRegions->isChecked() )
        conf->setDisplayRegionsRenderer( checkShowRegions->isChecked() );
        
    i = lineEdit_visrange->text().toInt();
    if (i == 0) {
            QMessageBox::critical(this, "Cofiguration",
                              QString("Bad textures visibility range port!"));
            return;    
    }
    if (conf->getTextureVisibility() != i)
        conf->setTextureVisibility(i);


    i = lineEdit_detrange->text().toInt();
    if (i == 0) {
            QMessageBox::critical(this, "Cofiguration",
                              QString("Bad details visibility range!"));
            return;    
    }
    if (conf->getDetailsVisibility() != i)
        conf->setDetailsVisibility(i);

    i = lineEdit_layers->text().toInt();
    if (i == 0) {
            QMessageBox::critical(this, "Cofiguration",
                              QString("Bad visible Layers settings!"));
            return;    
    }
    if (conf->getVisibleLayers() != i)
        conf->setVisibleLayers(i);
    
    if (conf->getStartupMode() != misc_startupMode->currentIndex() )
        conf->setStartupMode( misc_startupMode->currentIndex() );

    done(Accepted);
}

void ConfigWidget::selectNoteColor() {
    QColor color = QColorDialog::getColor(conf->getNoteColor()==""?
        QColor("#F28003"):QColor((const char*)conf->getNoteColor()), this);
    if (color.isValid()) {
        conf->setNoteColor((const char*)color.name().toLocal8Bit());
        /*
        colorLabel->setText(color.name());
        colorLabel->setPalette(QPalette(color));
        colorLabel->setAutoFillBackground(true);
        */
    }
}

