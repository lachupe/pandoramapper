/************************************************************************
**
** Authors:   Ulf Hermann <ulfonk_mennhar@gmx.de> (Alve),
**            Marek Krejza <krejza@gmail.com> (Caligor),
**            Nils Schimmelmann <nschimme@gmail.com> (Jahara)
**
** This file is part of the MMapper project. 
** Maintained by Nils Schimmelmann <nschimme@gmail.com>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the:
** Free Software Foundation, Inc.
** 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
************************************************************************/

#include "roomeditattrdlg.h"
#include "defines.h"
#include "Map/CRoom.h"
#include "Map/CRoomManager.h"

#include <QShortcut>
#include <QSettings>

#include <math.h>

RoomEditAttrDlg::RoomEditAttrDlg(QWidget *parent)
        : QDialog(parent)
{
    setupUi(this);
	roomDescriptionTextEdit->setLineWrapMode(QTextEdit::NoWrap);
		
	mobListItems[0] = (QListWidgetItem*) new QListWidgetItem("Rent place");
	mobListItems[1] = (QListWidgetItem*) new QListWidgetItem("Generic shop");
	mobListItems[2] = (QListWidgetItem*) new QListWidgetItem("Weapon shop");
	mobListItems[3] = (QListWidgetItem*) new QListWidgetItem("Armour shop");
	mobListItems[4] = (QListWidgetItem*) new QListWidgetItem("Food shop");
	mobListItems[5] = (QListWidgetItem*) new QListWidgetItem("Pet shop");
	mobListItems[6] = (QListWidgetItem*) new QListWidgetItem("Generic guild");
	mobListItems[7] = (QListWidgetItem*) new QListWidgetItem("Scout guild");
	mobListItems[8] = (QListWidgetItem*) new QListWidgetItem("Mage guild");
	mobListItems[9] = (QListWidgetItem*) new QListWidgetItem("Cleric guild");
	mobListItems[10] = (QListWidgetItem*) new QListWidgetItem("Warrior guild");
	mobListItems[11] = (QListWidgetItem*) new QListWidgetItem("Ranger guild");
	mobListItems[12] = (QListWidgetItem*) new QListWidgetItem("SMOB");
	mobListItems[13] = (QListWidgetItem*) new QListWidgetItem("Quest mob");
	mobListItems[14] = (QListWidgetItem*) new QListWidgetItem("Any mob"); 
	mobListItems[15] = NULL;

	mobFlagsListWidget->clear();
	for (int i=0; i<15; i++)
	{	
		mobListItems[i]->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsTristate);
		mobFlagsListWidget->addItem(mobListItems[i]);
	}

	loadListItems[0] = (QListWidgetItem*) new QListWidgetItem("Treasure");
	loadListItems[1] = (QListWidgetItem*) new QListWidgetItem("Equipment");
	loadListItems[2] = (QListWidgetItem*) new QListWidgetItem("Weapon");
	loadListItems[3] = (QListWidgetItem*) new QListWidgetItem("Water");
	loadListItems[4] = (QListWidgetItem*) new QListWidgetItem("Food");
	loadListItems[5] = (QListWidgetItem*) new QListWidgetItem("Herb");
	loadListItems[6] = (QListWidgetItem*) new QListWidgetItem("Key");
	loadListItems[7] = (QListWidgetItem*) new QListWidgetItem("Mule");
	loadListItems[8] = (QListWidgetItem*) new QListWidgetItem("Horse");
	loadListItems[9] = (QListWidgetItem*) new QListWidgetItem("Pack horse");
	loadListItems[10] = (QListWidgetItem*) new QListWidgetItem("Trained horse");
	loadListItems[11] = (QListWidgetItem*) new QListWidgetItem("Rohirrim");
	loadListItems[12] = (QListWidgetItem*) new QListWidgetItem("Warg"); 
	loadListItems[13] = (QListWidgetItem*) new QListWidgetItem("Boat"); 
	loadListItems[14] = (QListWidgetItem*) new QListWidgetItem("Attention");
	loadListItems[15] = (QListWidgetItem*) new QListWidgetItem("Tower"); 
	loadListItems[16] = NULL;
		
	loadFlagsListWidget->clear();
	for (int i=0; i<16; i++)
	{
		loadListItems[i]->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsTristate);
		loadFlagsListWidget->addItem(loadListItems[i]);
	}

	exitListItems[0] = (QListWidgetItem*) new QListWidgetItem("Exit");
	exitListItems[1] = (QListWidgetItem*) new QListWidgetItem("Door");
	exitListItems[2] = (QListWidgetItem*) new QListWidgetItem("Road");
	exitListItems[3] = (QListWidgetItem*) new QListWidgetItem("Climb");
	exitListItems[4] = (QListWidgetItem*) new QListWidgetItem("Random");
	exitListItems[5] = (QListWidgetItem*) new QListWidgetItem("Special");
	exitListItems[6] = (QListWidgetItem*) new QListWidgetItem("No match");
	exitListItems[7] = NULL;

	exitFlagsListWidget->clear();
	for (int i=0; i<7; i++)
	{	
		exitListItems[i]->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		exitFlagsListWidget->addItem(exitListItems[i]);
	}
	
	doorListItems[0] = (QListWidgetItem*) new QListWidgetItem("Hidden");
	doorListItems[1] = (QListWidgetItem*) new QListWidgetItem("Need key");
	doorListItems[2] = (QListWidgetItem*) new QListWidgetItem("No block");
	doorListItems[3] = (QListWidgetItem*) new QListWidgetItem("No break");
	doorListItems[4] = (QListWidgetItem*) new QListWidgetItem("No pick");
	doorListItems[5] = (QListWidgetItem*) new QListWidgetItem("Delayed");
	doorListItems[6] = NULL;

	doorFlagsListWidget->clear();
	for (int i=0; i<6; i++)
	{
		doorListItems[i]->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		doorFlagsListWidget->addItem(doorListItems[i]);
	}

  m_hiddenShortcut = new QShortcut( QKeySequence( tr( "Ctrl+H", "Room edit > hidden flag" ) ), this );

	updatedLabel->setText("Room has not been online updated yet!!!");
		
	readSettings();	
}

RoomEditAttrDlg::~RoomEditAttrDlg()
{
	writeSettings();
}

void RoomEditAttrDlg::readSettings()
{
    QSettings settings("Aza Soft", "Pandoramapper");
    settings.beginGroup("RoomEditAttrDlg");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    settings.endGroup();
    move(pos);
}

void RoomEditAttrDlg::writeSettings()
{
    QSettings settings("Aza Soft", "Pandoramapper");
    settings.beginGroup("RoomEditAttrDlg");
    settings.setValue("pos", pos());
    settings.endGroup();
}

void RoomEditAttrDlg::connectAll()
{
    connect(neutralRadioButton, SIGNAL(toggled(bool)), this, SLOT(neutralRadioButtonToggled(bool)));	
    connect(goodRadioButton, SIGNAL(toggled(bool)), this, SLOT(goodRadioButtonToggled(bool)));	
    connect(evilRadioButton, SIGNAL(toggled(bool)), this, SLOT(evilRadioButtonToggled(bool)));	
    connect(alignUndefRadioButton, SIGNAL(toggled(bool)), this, SLOT(alignUndefRadioButtonToggled(bool)));	

    connect(noPortRadioButton, SIGNAL(toggled(bool)), this, SLOT(noPortRadioButtonToggled(bool)));	
    connect(portableRadioButton, SIGNAL(toggled(bool)), this, SLOT(portableRadioButtonToggled(bool)));	
    connect(portUndefRadioButton, SIGNAL(toggled(bool)), this, SLOT(portUndefRadioButtonToggled(bool)));	
	
    connect(noRideRadioButton, SIGNAL(toggled(bool)), this, SLOT(noRideRadioButtonToggled(bool)));	
    connect(ridableRadioButton, SIGNAL(toggled(bool)), this, SLOT(ridableRadioButtonToggled(bool)));	
    connect(rideUndefRadioButton, SIGNAL(toggled(bool)), this, SLOT(rideUndefRadioButtonToggled(bool)));	

    connect(litRadioButton, SIGNAL(toggled(bool)), this, SLOT(litRadioButtonToggled(bool)));	
    connect(darkRadioButton, SIGNAL(toggled(bool)), this, SLOT(darkRadioButtonToggled(bool)));	
    connect(lightUndefRadioButton, SIGNAL(toggled(bool)), this, SLOT(lightUndefRadioButtonToggled(bool)));	
	
	connect(mobFlagsListWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(mobFlagsListItemChanged(QListWidgetItem*)));
	connect(loadFlagsListWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(loadFlagsListItemChanged(QListWidgetItem*)));

    connect(exitNButton, SIGNAL(toggled(bool)), this, SLOT(exitButtonToggled(bool)));	
    connect(exitSButton, SIGNAL(toggled(bool)), this, SLOT(exitButtonToggled(bool)));	
    connect(exitEButton, SIGNAL(toggled(bool)), this, SLOT(exitButtonToggled(bool)));	
    connect(exitWButton, SIGNAL(toggled(bool)), this, SLOT(exitButtonToggled(bool)));	
    connect(exitUButton, SIGNAL(toggled(bool)), this, SLOT(exitButtonToggled(bool)));	
    connect(exitDButton, SIGNAL(toggled(bool)), this, SLOT(exitButtonToggled(bool)));	

	connect(exitFlagsListWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(exitFlagsListItemChanged(QListWidgetItem*)));
	connect(doorFlagsListWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(doorFlagsListItemChanged(QListWidgetItem*)));

	connect(doorNameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(doorNameLineEditTextChanged(QString)));

	connect(toolButton00, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	connect(toolButton01, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	connect(toolButton02, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	connect(toolButton03, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	connect(toolButton04, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	connect(toolButton05, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	connect(toolButton06, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	connect(toolButton07, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	connect(toolButton08, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	connect(toolButton09, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	connect(toolButton10, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	connect(toolButton11, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	connect(toolButton12, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	connect(toolButton13, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	connect(toolButton14, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	connect(toolButton15, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));

    connect(roomNoteTextEdit, SIGNAL(textChanged()), this, SLOT(roomNoteChanged()));

    connect(m_hiddenShortcut, SIGNAL(activated()), this, SLOT(toggleHiddenDoor()));
}

void RoomEditAttrDlg::disconnectAll()
{
    disconnect(neutralRadioButton, SIGNAL(toggled(bool)), this, SLOT(neutralRadioButtonToggled(bool)));	
    disconnect(goodRadioButton, SIGNAL(toggled(bool)), this, SLOT(goodRadioButtonToggled(bool)));	
    disconnect(evilRadioButton, SIGNAL(toggled(bool)), this, SLOT(evilRadioButtonToggled(bool)));	
    disconnect(alignUndefRadioButton, SIGNAL(toggled(bool)), this, SLOT(alignUndefRadioButtonToggled(bool)));	

    disconnect(noPortRadioButton, SIGNAL(toggled(bool)), this, SLOT(noPortRadioButtonToggled(bool)));	
    disconnect(portableRadioButton, SIGNAL(toggled(bool)), this, SLOT(portableRadioButtonToggled(bool)));	
    disconnect(portUndefRadioButton, SIGNAL(toggled(bool)), this, SLOT(portUndefRadioButtonToggled(bool)));	
	
    disconnect(noRideRadioButton, SIGNAL(toggled(bool)), this, SLOT(noRideRadioButtonToggled(bool)));	
    disconnect(ridableRadioButton, SIGNAL(toggled(bool)), this, SLOT(ridableRadioButtonToggled(bool)));	
    disconnect(rideUndefRadioButton, SIGNAL(toggled(bool)), this, SLOT(rideUndefRadioButtonToggled(bool)));	

    disconnect(litRadioButton, SIGNAL(toggled(bool)), this, SLOT(litRadioButtonToggled(bool)));	
    disconnect(darkRadioButton, SIGNAL(toggled(bool)), this, SLOT(darkRadioButtonToggled(bool)));	
    disconnect(lightUndefRadioButton, SIGNAL(toggled(bool)), this, SLOT(lightUndefRadioButtonToggled(bool)));	
	
	disconnect(mobFlagsListWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(mobFlagsListItemChanged(QListWidgetItem*)));
	disconnect(loadFlagsListWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(loadFlagsListItemChanged(QListWidgetItem*)));

    disconnect(exitNButton, SIGNAL(toggled(bool)), this, SLOT(exitButtonToggled(bool)));	
    disconnect(exitSButton, SIGNAL(toggled(bool)), this, SLOT(exitButtonToggled(bool)));	
    disconnect(exitEButton, SIGNAL(toggled(bool)), this, SLOT(exitButtonToggled(bool)));	
    disconnect(exitWButton, SIGNAL(toggled(bool)), this, SLOT(exitButtonToggled(bool)));	
    disconnect(exitUButton, SIGNAL(toggled(bool)), this, SLOT(exitButtonToggled(bool)));	
    disconnect(exitDButton, SIGNAL(toggled(bool)), this, SLOT(exitButtonToggled(bool)));	

	disconnect(exitFlagsListWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(exitFlagsListItemChanged(QListWidgetItem*)));
	disconnect(doorFlagsListWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(doorFlagsListItemChanged(QListWidgetItem*)));

	disconnect(doorNameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(doorNameLineEditTextChanged(QString)));

	disconnect(toolButton00, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	disconnect(toolButton01, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	disconnect(toolButton02, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	disconnect(toolButton03, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	disconnect(toolButton04, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	disconnect(toolButton05, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	disconnect(toolButton06, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	disconnect(toolButton07, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	disconnect(toolButton08, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	disconnect(toolButton09, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	disconnect(toolButton10, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	disconnect(toolButton11, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	disconnect(toolButton12, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	disconnect(toolButton13, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	disconnect(toolButton14, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));
	disconnect(toolButton15, SIGNAL(toggled(bool)), this, SLOT(terrainToolButtonToggled(bool)));

    disconnect(roomNoteTextEdit, SIGNAL(textChanged()), this, SLOT(roomNoteChanged()));	

    disconnect(m_hiddenShortcut, SIGNAL(activated()), this, SLOT(toggleHiddenDoor()));
}

CRoom* RoomEditAttrDlg::getSelectedRoom()
{
    if ( m_roomSelection.size() == 0 )
        return NULL;
    if ( m_roomSelection.size() == 1 ) {
        RoomId id = m_roomSelection.first();
        return m_manager->getRoom(id);
    } else {
        RoomId id = roomListComboBox->itemData(roomListComboBox->currentIndex()).toInt();
        return m_manager->getRoom(id);
    }
}

ExitDirection RoomEditAttrDlg::getSelectedExit()
{
    if (exitNButton->isChecked())
        return ED_NORTH;
    if (exitSButton->isChecked())
        return ED_SOUTH;
    if (exitEButton->isChecked())
        return ED_EAST;
    if (exitWButton->isChecked())
        return ED_WEST;
    if (exitUButton->isChecked())
        return ED_UP;
    if (exitDButton->isChecked())
        return ED_DOWN;
    return ED_UNKNOWN;
}

void RoomEditAttrDlg::roomListCurrentIndexChanged(int)
{
	disconnectAll();
    alignUndefRadioButton->setChecked(true);
    portUndefRadioButton->setChecked(true);
    lightUndefRadioButton->setChecked(true);
	connectAll();

	updateDialog( getSelectedRoom() );
}

void RoomEditAttrDlg::setRoomSelection(QList<RoomId> rs, CRoomManager *manager)
{
    m_manager = manager;
    m_roomSelection = rs;

	roomListComboBox->clear();


    if (rs.size() > 1)
    {
    	tabWidget->setCurrentWidget(selectionTab);
    	roomListComboBox->addItem("All", 0);
    	updateDialog(NULL);    	    	
    	
		disconnectAll();
        alignUndefRadioButton->setChecked(true);
        portUndefRadioButton->setChecked(true);
        lightUndefRadioButton->setChecked(true);
		connectAll();
    }
    else if (rs.size() == 1)
    {
    	tabWidget->setCurrentWidget(attributesTab);
        updateDialog( m_manager->getRoom( m_roomSelection.first() ) );
    }
    

    for ( auto it = rs.begin(); it != rs.end(); it++) {
        RoomId id = *it;
        roomListComboBox->addItem( manager->getName(id), id);
    }
        
    connect(roomListComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(roomListCurrentIndexChanged(int)) );	
    connect(closeButton, SIGNAL(clicked()), this, SLOT(closeClicked()));	

    // FIXME: signal to renderer to redraw
    //connect(this, SIGNAL(mapChanged()), m_mapCanvas, SLOT(update()));
}


void RoomEditAttrDlg::updateDialog(const CRoom *r)
{
	disconnectAll();
	
	if (r == NULL) 
	{
		roomDescriptionTextEdit->clear();
		roomDescriptionTextEdit->setEnabled(false);

		updatedLabel->setText("");

		roomNoteTextEdit->clear();
		roomNoteTextEdit->setEnabled(false);

        terrainLabel->setPixmap(QPixmap(QString(":/textures/terrain%1.png").arg(0)));
		
		exitsFrame->setEnabled(false);                                

		int index = 0;
		while(loadListItems[index]!=NULL)
		{
			loadListItems[index]->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsTristate);
			loadListItems[index]->setCheckState(Qt::PartiallyChecked);
			index++;		
		}
	
		index = 0;
		while(mobListItems[index]!=NULL)
		{
			mobListItems[index]->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsTristate);
			mobListItems[index]->setCheckState(Qt::PartiallyChecked);		
			index++;		
		}
	}
	else
	{
		roomDescriptionTextEdit->clear();
        roomDescriptionTextEdit->setEnabled(true);

        exitsFrame->setEnabled(true);

        ExitDirection dir = getSelectedExit();
				
        if (r->isExitNormal(dir))
			exitListItems[0]->setCheckState(Qt::Checked);
		else
			exitListItems[0]->setCheckState(Qt::Unchecked);
		
        if (r->isDoorSet(dir))
		{
            doorNameLineEdit->setEnabled(true);
            doorFlagsListWidget->setEnabled(true);
						
			exitListItems[1]->setCheckState(Qt::Checked);			
            doorNameLineEdit->setText( r->getDoor(dir) );
				
            if (r->isDoorSecret(dir))
				doorListItems[0]->setCheckState(Qt::Checked);
			else
				doorListItems[0]->setCheckState(Qt::Unchecked);
				
            if (r->isDoorFlagSet(dir, DF_NEEDKEY))
				doorListItems[1]->setCheckState(Qt::Checked);
			else
				doorListItems[1]->setCheckState(Qt::Unchecked);
	
            if (r->isDoorFlagSet(dir, DF_NOBLOCK))
				doorListItems[2]->setCheckState(Qt::Checked);
			else
				doorListItems[2]->setCheckState(Qt::Unchecked);
	
            if (r->isDoorFlagSet(dir, DF_NOBREAK))
				doorListItems[3]->setCheckState(Qt::Checked);
			else
				doorListItems[3]->setCheckState(Qt::Unchecked);
	
            if (r->isDoorFlagSet(dir, DF_NOPICK))
				doorListItems[4]->setCheckState(Qt::Checked);
			else
				doorListItems[4]->setCheckState(Qt::Unchecked);
	
            if (r->isDoorFlagSet(dir, DF_DELAYED))
				doorListItems[5]->setCheckState(Qt::Checked);
			else
				doorListItems[5]->setCheckState(Qt::Unchecked);
		}
		else
		{
			doorNameLineEdit->clear();
			doorNameLineEdit->setEnabled(false);
			doorFlagsListWidget->setEnabled(false);				

			exitListItems[1]->setCheckState(Qt::Unchecked);
		}

        if (r->isExitFlagSet(dir, EF_ROAD))
			exitListItems[2]->setCheckState(Qt::Checked);
		else
			exitListItems[2]->setCheckState(Qt::Unchecked);

        if (r->isExitFlagSet(dir, EF_CLIMB))
			exitListItems[3]->setCheckState(Qt::Checked);
		else
			exitListItems[3]->setCheckState(Qt::Unchecked);
	
        if (r->isExitFlagSet(dir, EF_RANDOM))
			exitListItems[4]->setCheckState(Qt::Checked);
		else
			exitListItems[4]->setCheckState(Qt::Unchecked);
	
        if (r->isExitFlagSet(dir, EF_SPECIAL))
			exitListItems[5]->setCheckState(Qt::Checked);
		else
			exitListItems[5]->setCheckState(Qt::Unchecked);

        if (r->isExitFlagSet(dir, EF_NO_MATCH))
			exitListItems[6]->setCheckState(Qt::Checked);
		else
			exitListItems[6]->setCheckState(Qt::Unchecked);

		roomNoteTextEdit->clear();
		roomNoteTextEdit->setEnabled(false);


		int index = 0;
		while(loadListItems[index]!=NULL)
		{
			loadListItems[index]->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			index++;		
		}
	
		index = 0;
		while(mobListItems[index]!=NULL)
		{
			mobListItems[index]->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			index++;		
		}

		
        if (r->isMobFlagSet(RMF_RENT))
			mobListItems[0]->setCheckState(Qt::Checked);
		else 
			mobListItems[0]->setCheckState(Qt::Unchecked);
	
        if (r->isMobFlagSet(RMF_SHOP))
			mobListItems[1]->setCheckState(Qt::Checked);
		else
			mobListItems[1]->setCheckState(Qt::Unchecked);
	
        if (r->isMobFlagSet(RMF_WEAPONSHOP))
			mobListItems[2]->setCheckState(Qt::Checked);
		else
			mobListItems[2]->setCheckState(Qt::Unchecked);
	
        if (r->isMobFlagSet(RMF_ARMOURSHOP))
			mobListItems[3]->setCheckState(Qt::Checked);
		else
			mobListItems[3]->setCheckState(Qt::Unchecked);
	
        if (r->isMobFlagSet(RMF_FOODSHOP))
			mobListItems[4]->setCheckState(Qt::Checked);
		else
			mobListItems[4]->setCheckState(Qt::Unchecked);
	
        if (r->isMobFlagSet(RMF_PETSHOP))
			mobListItems[5]->setCheckState(Qt::Checked);
		else
			mobListItems[5]->setCheckState(Qt::Unchecked);
	
        if (r->isMobFlagSet(RMF_GUILD))
			mobListItems[6]->setCheckState(Qt::Checked);
		else
			mobListItems[6]->setCheckState(Qt::Unchecked);
	
        if (r->isMobFlagSet(RMF_SCOUTGUILD))
			mobListItems[7]->setCheckState(Qt::Checked);
		else
			mobListItems[7]->setCheckState(Qt::Unchecked);
	
        if (r->isMobFlagSet(RMF_MAGEGUILD))
			mobListItems[8]->setCheckState(Qt::Checked);
		else
			mobListItems[8]->setCheckState(Qt::Unchecked);
	
        if (r->isMobFlagSet(RMF_CLERICGUILD))
			mobListItems[9]->setCheckState(Qt::Checked);
		else
			mobListItems[9]->setCheckState(Qt::Unchecked);
	
        if (r->isMobFlagSet(RMF_WARRIORGUILD))
			mobListItems[10]->setCheckState(Qt::Checked);
		else
			mobListItems[10]->setCheckState(Qt::Unchecked);
	
        if (r->isMobFlagSet(RMF_RANGERGUILD))
			mobListItems[11]->setCheckState(Qt::Checked);
		else
			mobListItems[11]->setCheckState(Qt::Unchecked);
	
        if (r->isMobFlagSet(RMF_SMOB))
			mobListItems[12]->setCheckState(Qt::Checked);
		else
			mobListItems[12]->setCheckState(Qt::Unchecked);
	
        if (r->isMobFlagSet(RMF_QUEST))
			mobListItems[13]->setCheckState(Qt::Checked);
		else
			mobListItems[13]->setCheckState(Qt::Unchecked);
	
        if (r->isMobFlagSet(RMF_ANY))
			mobListItems[14]->setCheckState(Qt::Checked);
		else
			mobListItems[14]->setCheckState(Qt::Unchecked);
	
	
        if (r->isLoadFlagSet(RLF_TREASURE))
			loadListItems[0]->setCheckState(Qt::Checked);
		else
			loadListItems[0]->setCheckState(Qt::Unchecked);
	
        if (r->isLoadFlagSet(RLF_ARMOUR))
			loadListItems[1]->setCheckState(Qt::Checked);
		else
			loadListItems[1]->setCheckState(Qt::Unchecked);
	
        if (r->isLoadFlagSet(RLF_WEAPON))
			loadListItems[2]->setCheckState(Qt::Checked);
		else
			loadListItems[2]->setCheckState(Qt::Unchecked);
	
        if (r->isLoadFlagSet(RLF_WATER))
			loadListItems[3]->setCheckState(Qt::Checked);
		else
			loadListItems[3]->setCheckState(Qt::Unchecked);
	
        if (r->isLoadFlagSet(RLF_FOOD))
			loadListItems[4]->setCheckState(Qt::Checked);
		else
			loadListItems[4]->setCheckState(Qt::Unchecked);
	
        if (r->isLoadFlagSet(RLF_HERB))
			loadListItems[5]->setCheckState(Qt::Checked);
		else
			loadListItems[5]->setCheckState(Qt::Unchecked);
	
        if (r->isLoadFlagSet(RLF_KEY))
			loadListItems[6]->setCheckState(Qt::Checked);
		else
			loadListItems[6]->setCheckState(Qt::Unchecked);
	
        if (r->isLoadFlagSet(RLF_MULE))
			loadListItems[7]->setCheckState(Qt::Checked);
		else
			loadListItems[7]->setCheckState(Qt::Unchecked);
	
        if (r->isLoadFlagSet(RLF_HORSE))
			loadListItems[8]->setCheckState(Qt::Checked);
		else
			loadListItems[8]->setCheckState(Qt::Unchecked);
	
        if (r->isLoadFlagSet(RLF_PACKHORSE))
			loadListItems[9]->setCheckState(Qt::Checked);
		else
			loadListItems[9]->setCheckState(Qt::Unchecked);
	
        if (r->isLoadFlagSet(RLF_TRAINEDHORSE))
			loadListItems[10]->setCheckState(Qt::Checked);
		else
			loadListItems[10]->setCheckState(Qt::Unchecked);
	
        if (r->isLoadFlagSet(RLF_ROHIRRIM))
			loadListItems[11]->setCheckState(Qt::Checked);
		else
			loadListItems[11]->setCheckState(Qt::Unchecked);
	
        if (r->isLoadFlagSet(RLF_WARG))
			loadListItems[12]->setCheckState(Qt::Checked);
		else
			loadListItems[12]->setCheckState(Qt::Unchecked);
	
        if (r->isLoadFlagSet(RLF_BOAT))
			loadListItems[13]->setCheckState(Qt::Checked);
		else
			loadListItems[13]->setCheckState(Qt::Unchecked);
	
        if (r->isLoadFlagSet(RLF_ATTENTION))
			loadListItems[14]->setCheckState(Qt::Checked);
		else
			loadListItems[14]->setCheckState(Qt::Unchecked);
	
        if (r->isLoadFlagSet(RLF_TOWER))
			loadListItems[15]->setCheckState(Qt::Checked);
		else
			loadListItems[15]->setCheckState(Qt::Unchecked);
	
	
        roomDescriptionTextEdit->setEnabled(true);
        roomNoteTextEdit->setEnabled(true);
				
		roomDescriptionTextEdit->clear();
		roomDescriptionTextEdit->setFontItalic(false);
        QString str = r->getDesc();
		str = str.left(str.length()-1);
		roomDescriptionTextEdit->append(str);	
        roomDescriptionTextEdit->setFontItalic(true);
        roomDescriptionTextEdit->append(r->getDynamicDesc());
		roomDescriptionTextEdit->scroll(-100,-100);
		
		roomNoteTextEdit->clear();
        roomNoteTextEdit->append(r->getNote());
		
        terrainLabel->setPixmap(QPixmap(QString(":/textures/terrain%1.png").arg((int) r->getTerrain())));
	
        switch (r->getAlignType())
		{
			case RAT_GOOD: 
                goodRadioButton->setChecked(true);
				break;
			case RAT_NEUTRAL: 
                neutralRadioButton->setChecked(true);
				break;
			case RAT_EVIL: 
                evilRadioButton->setChecked(true);
				break;
			case RAT_UNDEFINED:	
                alignUndefRadioButton->setChecked(true);
				break;
		}	
	
        switch (r->getPortableType())
		{
			case RPT_PORTABLE: 
                portableRadioButton->setChecked(true);
				break;
			case RPT_NOTPORTABLE: 
                noPortRadioButton->setChecked(true);
				break;
			case RPT_UNDEFINED:	
                portUndefRadioButton->setChecked(true);
				break;
		}	
	
        switch (r->getRidableType())
		{
			case RRT_RIDABLE: 
                ridableRadioButton->setChecked(true);
				break;
			case RRT_NOTRIDABLE: 
                noRideRadioButton->setChecked(true);
				break;
			case RRT_UNDEFINED:	
                rideUndefRadioButton->setChecked(true);
				break;
		}	

        switch (r->getLightType())
		{
			case RLT_DARK: 
                darkRadioButton->setChecked(true);
				break;
			case RLT_LIT: 
                litRadioButton->setChecked(true);
				break;
			case RLT_UNDEFINED:	
                lightUndefRadioButton->setChecked(true);
				break;
		}	
	}

	connectAll();
}




//attributes page
void RoomEditAttrDlg::exitButtonToggled(bool)
{
	updateDialog( getSelectedRoom() );	
}


void RoomEditAttrDlg::setAlign(RoomAlignType flag)
{
    for(auto it = m_roomSelection.begin(); it != m_roomSelection.end(); it++) {
        CRoom* r = m_manager->getRoom(*it);
        r->setAlignType(flag);
    }
    emit mapChanged();
    updateDialog( getSelectedRoom() );
}


void RoomEditAttrDlg::setPortable(RoomPortableType flag)
{
    for(auto it = m_roomSelection.begin(); it != m_roomSelection.end(); it++) {
        CRoom* r = m_manager->getRoom( *it);
        if (r)
            r->setPortableType(flag);
    }
    emit mapChanged();
    updateDialog( getSelectedRoom() );
}

void RoomEditAttrDlg::setRidable(RoomRidableType flag)
{
    for(auto it = m_roomSelection.begin(); it != m_roomSelection.end(); it++) {
        CRoom* r = m_manager->getRoom(*it);
        r->setRidableType(flag);
    }
    emit mapChanged();
    updateDialog( getSelectedRoom() );
}

void RoomEditAttrDlg::setLight(RoomLightType flag)
{
    for(auto it = m_roomSelection.begin(); it != m_roomSelection.end(); it++) {
        CRoom* r = m_manager->getRoom(*it);
        r->setLightType(flag);
    }
    emit mapChanged();
    updateDialog( getSelectedRoom() );
}


void RoomEditAttrDlg::neutralRadioButtonToggled(bool val)
{
	if (val)
        setAlign(RAT_NEUTRAL);
}

void RoomEditAttrDlg::goodRadioButtonToggled(bool val)
{
    if (val)
        setAlign(RAT_GOOD);
}

void RoomEditAttrDlg::evilRadioButtonToggled(bool val)
{
    if (val)
        setAlign(RAT_EVIL);
}

void RoomEditAttrDlg::alignUndefRadioButtonToggled(bool val)
{
    if (val)
        setAlign(RAT_UNDEFINED);
}

	
void RoomEditAttrDlg::noPortRadioButtonToggled(bool val)
{
    if (val)
        setPortable(RPT_NOTPORTABLE);
}

void RoomEditAttrDlg::portableRadioButtonToggled(bool val)
{
    if (val)
        setPortable(RPT_PORTABLE);
}

void RoomEditAttrDlg::portUndefRadioButtonToggled(bool val)
{
    if (val)
        setPortable(RPT_UNDEFINED);
}

void RoomEditAttrDlg::noRideRadioButtonToggled(bool val)
{
    if (val)
        setRidable(RRT_NOTRIDABLE);
}

void RoomEditAttrDlg::ridableRadioButtonToggled(bool val)
{
    if (val)
        setRidable(RRT_RIDABLE);
}

void RoomEditAttrDlg::rideUndefRadioButtonToggled(bool val)
{
    if (val)
        setRidable(RRT_UNDEFINED);
}
		
void RoomEditAttrDlg::litRadioButtonToggled(bool val)
{
    if (val)
        setLight(RLT_LIT);
}

void RoomEditAttrDlg::darkRadioButtonToggled(bool val)
{
    if (val)
        setLight(RLT_DARK);
}

void RoomEditAttrDlg::lightUndefRadioButtonToggled(bool val)
{
    if (val)
        setLight(RLT_UNDEFINED);
}


void RoomEditAttrDlg::setMobFlag(RoomMobFlag flag, bool val)
{
    for(auto it = m_roomSelection.begin(); it != m_roomSelection.end(); it++) {
        CRoom* r = m_manager->getRoom(*it);
        r->setMobFlag(flag, val);
    }
    emit mapChanged();
    updateDialog( getSelectedRoom() );
}


void RoomEditAttrDlg::setLoadFlag(RoomLoadFlag flag, bool val)
{
    for(auto it = m_roomSelection.begin(); it != m_roomSelection.end(); it++) {
        CRoom* r = m_manager->getRoom(*it);
        r->setLoadFlag(flag, val);
    }
    emit mapChanged();
    updateDialog( getSelectedRoom() );
}


void RoomEditAttrDlg::setExitFlag(ExitDirection dir, ExitFlag flag, bool val)
{
    for(auto it = m_roomSelection.begin(); it != m_roomSelection.end(); it++) {
        CRoom* r = m_manager->getRoom(*it);
        r->setExitFlag(dir, flag, val);
    }
    emit mapChanged();
    updateDialog( getSelectedRoom() );
}

void RoomEditAttrDlg::setDoorFlag(ExitDirection dir, DoorFlag flag, bool val)
{
    for(auto it = m_roomSelection.begin(); it != m_roomSelection.end(); it++) {
        CRoom* r = m_manager->getRoom(*it);
        r->setDoorFlag(dir, flag, val);
    }
    emit mapChanged();
    updateDialog( getSelectedRoom() );
}


void RoomEditAttrDlg::mobFlagsListItemChanged(QListWidgetItem* item)
{
	int index = 0;
	while(item != mobListItems[index])
		index++;

	switch (item->checkState())
	{
	case Qt::Unchecked:
        setMobFlag( static_cast<RoomMobFlag>(index), false);
        break;
	case Qt::PartiallyChecked:
		break;
	case Qt::Checked:
        setMobFlag( static_cast<RoomMobFlag>(index), true);
        break;
	}
}



void RoomEditAttrDlg::loadFlagsListItemChanged(QListWidgetItem* item)
{
	int index = 0;
	while(item != loadListItems[index])
		index++;

	switch (item->checkState())
	{
	case Qt::Unchecked:
        setLoadFlag( static_cast<RoomLoadFlag>(index), false);
		break;		
	case Qt::PartiallyChecked:
		break;
	case Qt::Checked:
        setLoadFlag( static_cast<RoomLoadFlag>(index), true);
        break;
	}
}

void RoomEditAttrDlg::exitFlagsListItemChanged(QListWidgetItem* item)
{
	int index = 0;
	while(item != exitListItems[index])
		index++;


	switch (item->checkState())
	{
	case Qt::Unchecked:
        setExitFlag(getSelectedExit(), static_cast<ExitFlag>(index), false);
        break;
	case Qt::PartiallyChecked:
		break;
	case Qt::Checked:
        setExitFlag(getSelectedExit(), static_cast<ExitFlag>(index), true);
		break;		
	}
}


void RoomEditAttrDlg::doorNameLineEditTextChanged(QString)
{
    CRoom* r = getSelectedRoom();
    r->setDoor(getSelectedExit(), doorNameLineEdit->text().toLocal8Bit());
}

void RoomEditAttrDlg::doorFlagsListItemChanged(QListWidgetItem* item)
{
	int index = 0;
	while(item != doorListItems[index])
		index++;

	switch (item->checkState())
	{
	case Qt::Unchecked:
        setDoorFlag(getSelectedExit(), static_cast<DoorFlag>(index), false);
        break;
	case Qt::PartiallyChecked:
		break;
	case Qt::Checked:
        setDoorFlag(getSelectedExit(), static_cast<DoorFlag>(index), true);
        break;
	}
			
	emit mapChanged();		  				
}

void RoomEditAttrDlg::toggleHiddenDoor()
{
  if ( doorFlagsListWidget->isEnabled() )  
    doorListItems[0]->setCheckState( doorListItems[0]->checkState() == Qt::Unchecked ? Qt::Checked : Qt::Unchecked );
}

//terrain tab
void RoomEditAttrDlg::terrainToolButtonToggled(bool val)
{
	if (val)
	{
		uint index = 0;
		if (toolButton00->isChecked()) index = 0;
		if (toolButton01->isChecked()) index = 1;
		if (toolButton02->isChecked()) index = 2;
		if (toolButton03->isChecked()) index = 3;
		if (toolButton04->isChecked()) index = 4;
		if (toolButton05->isChecked()) index = 5;
		if (toolButton06->isChecked()) index = 6;
		if (toolButton07->isChecked()) index = 7;
		if (toolButton08->isChecked()) index = 8;
		if (toolButton09->isChecked()) index = 9;
		if (toolButton10->isChecked()) index = 10;
		if (toolButton11->isChecked()) index = 11;
		if (toolButton12->isChecked()) index = 12;
		if (toolButton13->isChecked()) index = 13;
		if (toolButton14->isChecked()) index = 14;
		if (toolButton15->isChecked()) index = 15;
		
        terrainLabel->setPixmap(QPixmap(QString(":/textures/terrain%1.png").arg(index)));
	
        for(auto it = m_roomSelection.begin(); it != m_roomSelection.end(); it++) {
            CRoom* r = m_manager->getRoom(*it);
            r->setSector( static_cast< RoomTerrainType > (index) );
        }
        emit mapChanged();
        updateDialog( getSelectedRoom() );
	}
}
	
//note tab
void RoomEditAttrDlg::roomNoteChanged()
{
    for(auto it = m_roomSelection.begin(); it != m_roomSelection.end(); it++) {
        CRoom* r = m_manager->getRoom(*it);
        r->setNote(roomNoteTextEdit->document()->toPlainText().toLocal8Bit());
    }
    emit mapChanged();
}

	
//all tabs
void RoomEditAttrDlg::closeClicked()
{
	accept();	
}

