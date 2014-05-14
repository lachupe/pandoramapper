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

#ifndef CMOVEMENTDIALOG_H 
#define CMOVEMENTDIALOG_H

#include <QDialog>
#include "ui_movementdialog.h"

class CRoomManager;
class CMovementDialog : public QDialog, public Ui::MovementDialog {
Q_OBJECT

    CRoomManager *map;
public:
    CMovementDialog(CRoomManager *_map, QWidget *parent = 0);
    int x;
    int y;
    int z;
    
    void run();
public slots:
    virtual void accept();
};


#endif
