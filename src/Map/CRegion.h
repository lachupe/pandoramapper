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

#ifndef REGIONS_H
#define REGIONS_H

#include <QMap>
#include <QByteArray>
#include "Map/CRoom.h"


class CRoomManager;
class CRegion {
    QByteArray name;
    QMap<QByteArray, QByteArray> doors;
    CRoomManager *  m_map;
public:
    CRegion(CRoomManager* parent);
    ~CRegion();
    
    void setName(QByteArray newname);
    QByteArray getName();
    
    void addDoor(QByteArray alias, QByteArray name);
    QByteArray getDoor(QByteArray alias);
    bool removeDoor(QByteArray alias);
    
    QMap<QByteArray, QByteArray> getAllDoors();
    
    QByteArray getAliasByDoor(QByteArray door, ExitDirection dir);
    
    void showRegion();
};


#endif
