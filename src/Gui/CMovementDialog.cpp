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
#include <QIntValidator>

#include "utils.h"
#include "CConfigurator.h"

#include "Gui/CMovementDialog.h"

#include "Map/CRoomManager.h"
#include "Engine/CStacksManager.h"

CMovementDialog::CMovementDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);                        
    setWindowTitle(tr("Room's Movement Dialog"));
    lineEdit_x->setValidator(new QIntValidator(this));
    lineEdit_y->setValidator(new QIntValidator(this));
    lineEdit_z->setValidator(new QIntValidator(this));
}

void CMovementDialog::run()
{
    lineEdit_x->setText(QString("0"));
    lineEdit_y->setText(QString("0"));
    lineEdit_z->setText(QString("0"));
}

void CMovementDialog::accept()
{
    QList<int> ids;
    CRoom *r;


    x = lineEdit_x->text().toInt();
    y = lineEdit_y->text().toInt();
    z = lineEdit_z->text().toInt();


    // now proceed if needed to the actuall operation
    print_debug(DEBUG_INTERFACE, "movement dialog accepted");
    if (x != 0 || y != 0 || z != 0) {
        print_debug(DEBUG_INTERFACE, "moving rooms by shift : x %i, y %i, z %i", x, y, z);

        // collect the room for the movement            
        if (Map.selections.isEmpty() == false) {
            ids = Map.selections.getList();
        } else {
            if (stacker.amount() != 1) {
                QMessageBox::critical(this, "Movement Dialog", QString("You are not in sync!"));
                done(Accepted);
                return;
            } 
            ids.append(stacker.first()->id);
        }

        for (int i = 0; i < ids.size(); ++i) {
            r = Map.getRoom( ids.at(i) );
            r->setX( r->getX() + x);
            r->setY( r->getY() + y);
            if (z != 0) 
                r->setZ( r->getZ() + z);
        }
        toggle_renderer_reaction();
    }

    done(Accepted);
}
