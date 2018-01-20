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

#include "utils.h"
#include "CConfigurator.h"

#include "Gui/RoomEditDialog.h"

#include "Map/CRoomManager.h"


#define ROOMFLAG_UNDEFINED 0
#define ROOMFLAG_DEATH          1
#define ROOMFLAG_NONE           2

/* ROOM EDIT DIALOG */
RoomEditDialog::RoomEditDialog(QWidget *parent) : 
                                    QDialog(parent)
{
    setupUi(this);

    comboBox_N->insertItem(ROOMFLAG_UNDEFINED, "Undefined Exit");    
    comboBox_E->insertItem(ROOMFLAG_UNDEFINED, "Undefined Exit");    
    comboBox_S->insertItem(ROOMFLAG_UNDEFINED, "Undefined Exit");    
    comboBox_W->insertItem(ROOMFLAG_UNDEFINED, "Undefined Exit");    
    comboBox_U->insertItem(ROOMFLAG_UNDEFINED, "Undefined Exit");    
    comboBox_D->insertItem(ROOMFLAG_UNDEFINED, "Undefined Exit");    

    comboBox_N->insertItem(ROOMFLAG_DEATH, "DEATH");    
    comboBox_E->insertItem(ROOMFLAG_DEATH, "DEATH");    
    comboBox_S->insertItem(ROOMFLAG_DEATH, "DEATH");    
    comboBox_W->insertItem(ROOMFLAG_DEATH, "DEATH");    
    comboBox_U->insertItem(ROOMFLAG_DEATH, "DEATH");    
    comboBox_D->insertItem(ROOMFLAG_DEATH, "DEATH");    

    comboBox_N->insertItem(ROOMFLAG_NONE, "NONE");
    comboBox_E->insertItem(ROOMFLAG_NONE, "NONE");
    comboBox_S->insertItem(ROOMFLAG_NONE, "NONE");
    comboBox_W->insertItem(ROOMFLAG_NONE, "NONE");
    comboBox_U->insertItem(ROOMFLAG_NONE, "NONE");
    comboBox_D->insertItem(ROOMFLAG_NONE, "NONE");

    connect(comboBox_N, SIGNAL(activated(int)), this, SLOT(changedExitsFlagN(int)) );
    connect(comboBox_S, SIGNAL(activated(int)), this, SLOT(changedExitsFlagS(int)) );
    connect(comboBox_E, SIGNAL(activated(int)), this, SLOT(changedExitsFlagE(int)) );
    connect(comboBox_W, SIGNAL(activated(int)), this, SLOT(changedExitsFlagW(int)) );
    connect(comboBox_U, SIGNAL(activated(int)), this, SLOT(changedExitsFlagU(int)) );
    connect(comboBox_D, SIGNAL(activated(int)), this, SLOT(changedExitsFlagD(int)) );
    
    connect(checkBox_N, SIGNAL(toggled(bool)), this, SLOT(changedExitsStateN(bool)) );
    connect(checkBox_S, SIGNAL(toggled(bool)), this, SLOT(changedExitsStateS(bool)) );
    connect(checkBox_E, SIGNAL(toggled(bool)), this, SLOT(changedExitsStateE(bool)) );
    connect(checkBox_W, SIGNAL(toggled(bool)), this, SLOT(changedExitsStateW(bool)) );
    connect(checkBox_U, SIGNAL(toggled(bool)), this, SLOT(changedExitsStateU(bool)) );
    connect(checkBox_D, SIGNAL(toggled(bool)), this, SLOT(changedExitsStateD(bool)) );

/*    
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
*/
    connect(&Map.selections, SIGNAL( roomSelected(unsigned int)), SLOT( roomSelected(unsigned int)) );
}

void RoomEditDialog::roomSelected(unsigned int id) 
{
    if (this->isVisible() == true) {
        for (int i = 0; i < 5; i++) {
            set_door_context( i );
            if (leads->text() == "select Room!")
                leads->setText( QString("%1").arg(id) );
        }
    
    }
}


int RoomEditDialog::updateExitsInfo(int dir, CRoom *r)
{
    QString dname;
    unsigned int lead;
    int flag;
    
    set_door_context(dir);
    if (box->isChecked()) {
        flag = flags->currentIndex();
        if (flag == ROOMFLAG_UNDEFINED) {
            r->setExitUndefined(dir);
        } else if (flag == ROOMFLAG_DEATH) {
            r->setExitDeath(dir);
        } else  {        
            lead = leads->text().toInt();
            if (Map.getRoom(lead) == NULL) {
                QMessageBox::critical(this, "Room Info Edit",
                              QString("Bad ID given for connection to the %1").arg(exits[dir]) );
               return -1;
            }
            r->setExit(dir, lead);
        }
    
        dname = door->text();
        if (dname.length() > 40) {
            QMessageBox::critical(this, "Room Info Edit",
                              QString("Too long doorname for connection to the %1").arg(exits[dir]));
            return -1;    
        }
        r->setDoor(dir, dname.toLocal8Bit());    
    } else {
        r->removeExit(dir);    
        return 0;        
    }
    
    return 0;
}

void RoomEditDialog::clear_data()
{



    textEdit_note->clear();
    textEdit_desc->clear();
}


void    RoomEditDialog::set_door_context(int dir)
{
    switch (dir) {
        case NORTH: door = lineEdit_doornorth;
                    leads = lineEdit_leadN;
                    flags = comboBox_N;
                    box = checkBox_N;
                    break;
    
        case SOUTH: door = lineEdit_doorsouth;
                    leads = lineEdit_leadS;
                    flags = comboBox_S;
                    box = checkBox_S;
                    break;
    
        case EAST : door = lineEdit_dooreast; 
                    leads = lineEdit_leadE;
                    flags = comboBox_E;
                    box = checkBox_E;
                    break;
    
        case WEST : door = lineEdit_doorwest;
                    leads = lineEdit_leadW;
                    flags = comboBox_W;
                    box = checkBox_W;
                    break;
    
        case UP   : door = lineEdit_doorup;   
                    leads = lineEdit_leadU;
                    flags = comboBox_U;
                    box = checkBox_U;
                    break;
    
        case DOWN : door = lineEdit_doordown; 
                    leads = lineEdit_leadD;
                    flags = comboBox_D;
                    box = checkBox_D;
                    break;
    
        default:
            return;    
    }
}

void  RoomEditDialog::changedExitsFlag(int dir, int index)
{
    set_door_context(dir);  
      
//    if (flags->currentIndex() == index)
//        return;

    if (index == ROOMFLAG_NONE) {
        leads->setEnabled(true);
        leads->setText("select Room!");
        flags->setCurrentIndex(ROOMFLAG_NONE);
    } else if (index == ROOMFLAG_UNDEFINED) {
        leads->setText("UNDEF");
        leads->setEnabled(false);
        flags->setCurrentIndex(ROOMFLAG_UNDEFINED);
    } else if (index == ROOMFLAG_DEATH) {
        leads->setText("DEATH");
        leads->setEnabled(false);
        flags->setCurrentIndex(ROOMFLAG_DEATH);
    } 
    
}


void RoomEditDialog::setup_exit_widgets(int dir, CRoom *r)
{
    set_door_context(dir);
    
    door->clear();
    leads->clear();
    
    /* Exit North */
    door->setText(r->getDoor(dir) );        
    flags->setCurrentIndex(ROOMFLAG_NONE);
    
    if (r->isExitPresent(dir) == false) {
        box->setChecked(false);
        changedExitsState(dir, false);
    } else {
        box->setChecked(true);
        changedExitsState(dir, true);
                
        if (r->isExitUndefined(dir)) {
            changedExitsFlag(dir, ROOMFLAG_UNDEFINED);
        } else if (r->isExitDeath(dir) ) {
            changedExitsFlag(dir, ROOMFLAG_DEATH);
        } else {
            changedExitsFlag(dir, ROOMFLAG_NONE);
            leads->setText(QString("%1").arg(r->exits[dir]->id) );
            leads->setEnabled(true);
        }
        
                
    }
}


void  RoomEditDialog::changedExitsState(int dir, bool state)
{
    set_door_context(dir);
    if (state) {
        door->setEnabled(true);
        leads->setEnabled(false);
        flags->setEnabled(true);
        flags->setCurrentIndex(ROOMFLAG_UNDEFINED);
        leads->setText("UNDEF");
    } else {
        door->setEnabled(false);
        leads->setEnabled(false);
        flags->setEnabled(false);
        flags->setCurrentIndex(ROOMFLAG_NONE);
    }
}

void RoomEditDialog::load_room_data(unsigned int id)
{
    CRoom *r;
    unsigned int i;
    
    /* stuff dialog with room data */    
    r = Map.getRoom(id);
    setup_exit_widgets(NORTH, r);
    setup_exit_widgets(EAST, r);
    setup_exit_widgets(SOUTH, r);
    setup_exit_widgets(WEST, r);
    setup_exit_widgets(UP, r);
    setup_exit_widgets(DOWN, r);
    
    lineEdit_name->setText(r->getName());
    lineEdit_coordx->setText(QString("%1").arg(r->getX()));
    lineEdit_coordy->setText(QString("%1").arg(r->getY()));
    lineEdit_coordz->setText(QString("%1").arg(r->getZ()));
    
    QString desc = r->getDesc();
    desc.replace("|", "\n");
    textEdit_desc->append( desc );
    label_roomid->setText(QString("%1").arg(r->id) );
    textEdit_note->append(r->getNote());
        
    
        
    for (i=0; i< conf->sectors.size(); i++)
        comboBox_terrain->insertItem(i, conf->sectors[i].desc);    
        
    comboBox_terrain->setCurrentIndex(r->getTerrain() );
}


void RoomEditDialog::accept()
{
    CRoom *r;
    QString name, desc, note;
    int x, y, z;
    int id;
    unsigned int i;

    print_debug(DEBUG_INTERFACE, "Accepted!\r\n");
    /* OK! Now check the data */
    
    id = label_roomid->text().toInt();
    print_debug(DEBUG_INTERFACE, "Room id : %i\r\n", id);
    
    r = Map.getRoom(id);
    if (r == NULL) {
        QMessageBox::critical(this, "Room Info Edit",
                              QString("The room with this ID does not exist anymore."));
        return;    
    }
    
    name = lineEdit_name->text();
    if (name.length() == 0 || name.length() > 80) {
        QMessageBox::critical(this, "Room Info Edit",
                              QString("Bad room name!"));
        return;    
    }
    
    
    x = lineEdit_coordx->text().toInt();
    y = lineEdit_coordy->text().toInt();
    z = lineEdit_coordz->text().toInt();
    
    if (x != r->getX())
        r->setX(x);
    if (y != r->getY())
        r->setY(y);
    if (z != r->getZ())
        r->setZ(z);
        
    desc = textEdit_desc->toPlainText();
    desc.replace("\n", "|");
    //char terrain = comboBox_terrain->currentIndex();
    for (i=0; i< conf->sectors.size(); i++)
        if (comboBox_terrain->currentText() == conf->sectors[i].desc) {
            r->setSector(i);
        }
    
    note = textEdit_note->toPlainText();
            
    if (r->getName() != name) 
        r->setName(name.toLocal8Bit());            
    if (r->getDesc() != desc) 
        r->setDesc(desc.toLocal8Bit());            
    if (r->getNote() != note) 
        r->setNote(note.toLocal8Bit());            

    if (updateExitsInfo(NORTH, r) == -1) return;
    if (updateExitsInfo(EAST, r) == -1) return;
    if (updateExitsInfo(SOUTH, r) == -1) return;
    if (updateExitsInfo(WEST, r) == -1) return;
    if (updateExitsInfo(UP, r) == -1) return;
    if (updateExitsInfo(DOWN, r) == -1) return;
            
    toggle_renderer_reaction();
    done(Accepted);
}

void RoomEditDialog::reject()
{
    print_debug(DEBUG_INTERFACE, "Rejected!\r\n");
    done(Rejected);
}



