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

//
// C++ Implementation: Regions
//
// Description: 
//
//
// Author:  <aza@alpha>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "utils.h"

#include "Map/CRegion.h"
#include "Map/CRoomManager.h"

CRegion::CRegion()
{
    name.clear();
    doors.clear();
    

}

CRegion::~CRegion()
{
    name.clear();
    doors.clear();
}

    
void CRegion::setName(QByteArray newname)
{
    name = newname;
}

QByteArray CRegion::getName()
{
    return name;
}

void CRegion::addDoor(QByteArray alias, QByteArray name)
{
    doors.insert(alias.trimmed(), name.simplified());
    Map.rebuildRegion( this );
}

QByteArray CRegion::getDoor(QByteArray alias)
{
    if (doors.contains(alias) == true) 
        return doors[alias];        
    return "";
}

bool CRegion::removeDoor(QByteArray alias)
{
	bool b = doors.remove(alias);
    Map.rebuildRegion( this );
	return b;
}

void CRegion::showRegion()
{
    send_to_user("-- Region: %s\r\n", (const char *) name);
    
    QMapIterator<QByteArray, QByteArray> i(doors);
    while (i.hasNext()) {
        i.next();
        send_to_user("Alias: %-15s Door: %-40s\r\n", (const char *)  i.key(), (const char *) i.value() );
    }
    send_to_user("\r\n");
}

QMap<QByteArray, QByteArray> CRegion::getAllDoors()
{
    return doors;
}

QByteArray CRegion::getAliasByDoor(QByteArray door, ExitDirection dir)
{
    QByteArray fulldoor;
    
    fulldoor = door + " ";
    fulldoor.append( dirbynum(dir) );

    QMapIterator<QByteArray, QByteArray> i(doors);
    while (i.hasNext()) {
        i.next();
        print_debug(DEBUG_USERFUNC, "Checking ...%s...\r\n", (const char *) i.value() );
        if (i.value() == door || i.value() == fulldoor)
            return i.key();
    }
    return "";    
}



