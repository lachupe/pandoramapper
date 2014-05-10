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
// C++ Implementation: CRoom
//
// Description: 
//
//
// Author: Azazello <aza@alpha>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "map.pb.h"

#include <QByteArray>
#include <QString>

#include "CConfigurator.h"
#include "utils.h"

#include "Map/CRoom.h"
#include "Map/CRoomManager.h"
#include "Map/CTree.h"


#include "Proxy/proxy.h"
#include "Engine/CEngine.h"


Strings_Comparator comparator;


const struct room_flag_data room_flags[] = {
  {"undefined", "UNDEFINED",  ET_UNDEFINED},
  {"death", "DEATH", ET_DEATH},
    
  {"", "", ET_NONE}
};


CRoom::CRoom(CRoomManager *parent) :
    parent(parent)
{
    square = NULL;
    setRegion("default");

    // populate room.exits with "none" exits

    // the infamoust cycle
    for (int i = 0; i <= 5; i++) {
        mapdata::Exit::ExitDirection dir = static_cast<mapdata::Exit::ExitDirection>(i);
        mapdata::Exit* exit = room.add_exits();
        exit->set_dir(dir);
    }
}


CRoom::~CRoom()
{
    parent->getRoomNamesTree()->deleteItem( getName(), getId());
}


void CRoom::setSector(RoomTerrainType val)
{
    room.set_terrain( static_cast<mapdata::Room::RoomTerrainType>(val) );
    rebuildDisplayList();
}


void CRoom::setTerrain(char terrain)
{
    RoomTerrainType sector = conf->getSectorByPattern(terrain);
    setModified(true);
    rebuildDisplayList();
}


void CRoom::setModified(bool b)
{
    if (b) {
        conf->setDatabaseModified(true);
    }
}

int CRoom::descCmp(QByteArray d)
{ 
    if (room.desc().empty() != true)
        return comparator.strcmp_desc(d, getDesc());
    else
        return 0;
}

int CRoom::roomnameCmp(QByteArray n)
{ 
    if (room.name().empty() != true)
        return comparator.strcmp_roomname(n, getName());
    else
        return 0;
}



/* --------------- check if exit in room is connected --------------- */
bool CRoom::isConnected(ExitDirection dir)
{
    if (isExitUndefined(dir)  || isExitDeath(dir) )
        return false;
    if (exits[dir] != NULL)
        return true;

    return false;
}


/* is there anything at all in this direction ? */
bool CRoom::isExitPresent(ExitDirection dir)
{
    const mapdata::Exit &exit = room.exits(dir);

    if (exit.type() == ET_NORMAL)
        return true;
    if (exit.type() == ET_UNDEFINED || exit.type() == ET_DEATH)
        return true;

    return false;
}

bool CRoom::isExitLeadingTo(ExitDirection dir, CRoom *room)
{
    if (isConnected(dir) == false)
        return false;
    if (getExitLeadsTo(dir) == room->getId())
        return true;
    return false;
}



CRoom* CRoom::getExitRoom(ExitDirection dir) {
    if (isConnected(dir) == false)
        return NULL;

    return parent->getRoom(room.exits().Get(dir).leads_to_id());
}

RoomId CRoom::getExitLeadsTo(ExitDirection dir) {
    if (isConnected(dir) == false)
        return NULL;

    return room.exits().Get(dir).leads_to_id();
}



/* ------------------------ add_door() ------------------------*/
int CRoom::setDoor(ExitDirection dir, QByteArray d)
{
    if (d == "")
        return 0;

    mapdata::Exit *exit = room.mutable_exits(dir);


    if (exit->type() == ET_NONE) {
        exit->set_type(mapdata::Exit::ET_UNDEFINED);
    }

    exit->set_door(d);
    
    rebuildDisplayList();
    setModified(true);
    return 1;
}

/* ------------------------ remove_door() ------------------------*/
void CRoom::removeDoor(ExitDirection dir)
{
    mapdata::Exit *exit = room.mutable_exits(dir);

    exit->clear_door();
    rebuildDisplayList();
    setModified(true);
}

QByteArray CRoom::getDoor(ExitDirection dir)
{
    const mapdata::Exit& exit = room.exits(dir);

    if (exit.door().empty() && exit.exit_flags().door() == true)
        return "exit";
    else
        return exit.door().c_str();
}

bool CRoom::isDoorSet(ExitDirection dir)
{
    if (room.exits(dir).door().empty())
        return false;
    else 
        return true;
        
}

char CRoom::dirbynum(ExitDirection dir)
{
  switch (dir) {
    case  ED_NORTH :
                return 'n';
		break;
    case  ED_SOUTH :
                return 's';
		break;
    case  ED_EAST :
                return 'e';
		break;
    case  ED_WEST :
                return 'w';
		break;
    case  ED_UP :
                return 'u';
		break;
    case  ED_DOWN :
                return 'd';
		break;
  }


  return -1;
}


int CRoom::getX()
{
    return room.pos().x();
}

int CRoom::getY()
{
    return room.pos().y();
}

int CRoom::getZ()
{
    return room.pos().z();
}


void CRoom::setX(int nx)
{
    room.mutable_pos()->set_x(nx);
    setModified(true);
    rebuildDisplayList();
}

void CRoom::setY(int ny)
{
    room.mutable_pos()->set_y(ny);
    setModified(true);
    rebuildDisplayList();
}


void CRoom::setZ(int nz)
{
    parent->removeFromPlane(this);
    room.mutable_pos()->set_z(nz);


    rebuildDisplayList();
    // addToPlane will reset the square and call setSqaure of this room.

    parent->addToPlane(this);
    setModified(true);
}


void CRoom::simpleSetZ(int nz)
{
    room.mutable_pos()->set_z(nz);
    setModified(true);
}

QByteArray CRoom::getName()
{
    return room.name().c_str();
}


QByteArray CRoom::getDesc()
{
    return room.desc().c_str();
}


RoomTerrainType CRoom::getTerrain()
{
    return static_cast<RoomTerrainType>( room.terrain() );
}


QByteArray CRoom::getNote()
{
    return room.note().c_str();
}


void CRoom::setNoteColor(QByteArray color) {
    room.set_note_color(color);
    rebuildDisplayList();
}

QByteArray CRoom::getNoteColor() {
    return room.note_color().c_str();
}

void CRoom::setDesc(QByteArray newdesc)
{
    room.set_desc(newdesc);
    setModified(true);    
}
      
QByteArray CRoom::getSecretsInfo()
{
    int i;
    QByteArray res;
    QByteArray alias;
    
    res.clear();
    
    for (i = 0; i <= 5; i++) {
        ExitDirection iDir = static_cast<ExitDirection>(i);
        if (isDoorSecret( iDir ) == true) {
            res.append(dirbynum( iDir ));
            res = res + ": " + room.exits(i).door().c_str();
            alias = engine->get_users_region()->getAliasByDoor(room.exits(iDir).door().c_str(), iDir);
            if (alias.isEmpty() == false) 
                res += "[" + alias + "]";
        }
    }

    return res;
}

void CRoom::setName(QByteArray newname)
{
    parent->getRoomNamesTree()->deleteItem(room.name().c_str(), room.id());
    room.set_name(newname);
    parent->getRoomNamesTree()->addName(newname, room.id());
    setModified(true);
}


void CRoom::setNote(QByteArray newnote)
{
    room.set_note(newnote);
    rebuildDisplayList();
}


void CRoom::setSquare(CSquare *_square)
{
	if (square)
		rebuildDisplayList();

	square = _square;

	if (square)
		rebuildDisplayList();
}

void CRoom::setExit(ExitDirection dir, CRoom *r)
{
    room.mutable_exits(dir)->set_leads_to_id(r->getId());
    room.mutable_exits(dir)->set_type( mapdata::Exit::ET_NORMAL );

    // old code used EXIT_NONE ... this is going to deliver some problems!
    //exitFlags[dir] = EXIT_NONE;
    rebuildDisplayList();
}

void CRoom::setExit(ExitDirection dir, RoomId value)
{
    room.mutable_exits(dir)->set_leads_to_id(value);
    room.mutable_exits(dir)->set_type( mapdata::Exit::ET_NORMAL );

    // old code used EXIT_NONE ... this is going to deliver some problems!
    //exitFlags[dir] = EXIT_NONE;
    rebuildDisplayList();
}


bool CRoom::isExitUndefined(ExitDirection dir)
{
    return room.exits(dir).type() == mapdata::Exit::ET_UNDEFINED;
}

void CRoom::setExitUndefined(ExitDirection dir)
{
    room.mutable_exits(dir)->set_leads_to_id(0);
    room.mutable_exits(dir)->set_type( mapdata::Exit::ET_UNDEFINED );
    rebuildDisplayList();
}



bool CRoom::isExitDeath(ExitDirection dir)
{
    return room.exits(dir).type() == mapdata::Exit::ET_DEATH;
}

bool CRoom::isExitNormal(ExitDirection dir)
{
    return room.exits(dir).type() == mapdata::Exit::ET_NORMAL;
}



void CRoom::setExitDeath(ExitDirection dir)
{
    room.mutable_exits(dir)->set_type(mapdata::Exit::ET_DEATH);
    room.mutable_exits(dir)->set_leads_to_id(0);
    rebuildDisplayList();
    setModified(true);
}



bool CRoom::anyUndefinedExits()
{
    for (unsigned int i = 0; i <= 5; i++) {
        ExitDirection iDir = static_cast<ExitDirection>(i);
        if (room.exits(iDir).type() == mapdata::Exit::ET_UNDEFINED)
            return true;
    }
    return false;
}

bool CRoom::isEqualNameAndDesc(CRoom *r)
{
    if ((r->getDesc() == room.desc().c_str()) && (r->getName() == room.name().c_str()))
        return true;
    return false;
}


bool CRoom::isDescSet()
{
    if (room.desc().empty())
        return false;
    return true;
}

bool CRoom::isNameSet()
{
    if (room.name().empty())
        return false;
    return true;
}


bool CRoom::isDoorSecret(ExitDirection dir)
{
    if (room.exits(dir).door_flags().hidden())
        return true;
    else 
        return false;
}

QByteArray CRoom::getRegionName()
{
    return room.area_name().c_str();
}

void CRoom::setRegion(QByteArray name)
{
    if (name == "")
        setRegion(Map.getRegionByName("default"));
    else 
        setRegion(Map.getRegionByName(name)); 
}

void CRoom::setRegion(CRegion *reg)
{
    if (reg != NULL)
        region = reg;

    room.set_area_name(reg->getName());

    rebuildDisplayList();
}
    
CRegion *CRoom::getRegion()
{
    return region;
}

void CRoom::disconnectExit(ExitDirection dir)
{
    room.mutable_exits(dir)->set_type(mapdata::Exit::ET_NONE);
    room.mutable_exits(dir)->set_leads_to_id(0);

    rebuildDisplayList();
}

void CRoom::removeExit(ExitDirection dir)
{
    room.mutable_exits(dir)->set_type(mapdata::Exit::ET_NONE);
    room.mutable_exits(dir)->set_leads_to_id(0);
    room.mutable_exits(dir)->clear_door();
    room.mutable_exits(dir)->clear_door_flags();
    room.mutable_exits(dir)->clear_exit_flags();

    rebuildDisplayList();
}

QString CRoom::toolTip()
{
    QString s;
    s += "#";
    s += QString::number(getId());
    s += "\n";
    s += getName();

    for (int i = 0; i <= 5; i++) {
        ExitDirection iDir = static_cast<ExitDirection>(i);
        if (isDoorSecret(iDir)) {
            s += "\n";
            s += (dirbynum(iDir));
            s += ": ";
            s += room.exits(iDir).door().c_str();
        }
    }

    return s;
}

/* ------------------------------ prints the given room --------------------*/
void CRoom::sendRoom()
{
    unsigned int i, pos;
    char line[MAX_STR_LEN];
    
    send_to_user(" Id: %i, Flags: %s, Region: %s, Coord: %i,%i,%i\r\n", getId(),
                     (const char *) conf->sectors[ (int) getTerrain() ].desc,
	    (const char *) region->getName(),
        getX(), getY(), getZ());
    send_to_user(" [32m%s[0m\n", (const char *) getName() );

    line[0] = 0;
    pos = 0;

    if (!(proxy->isMudEmulation() && conf->getBriefMode() ) ) {
        QByteArray desc = getDesc();

        for (i = 0; i <= strlen(desc); i++)
            if (desc[i] == '|') {
                line[pos] = 0;
                send_to_user("%s\r\n", line);
                line[0] = 0;
                pos = 0;
            } else {
                line[pos++] = desc[i];
            }
    }
    send_to_user(" note: %s\n", (const char *) getNote());

    
    sprintf(line, "Doors:");
    for (i = 0; i <= 5; i++) {
        ExitDirection iDir = static_cast<ExitDirection>(i);
      if (isDoorSecret(iDir)) {
        sprintf(line + strlen(line), " %c: %s", dirbynum(iDir), (const char *) getDoor(iDir));
      }
    
    }
    send_to_user("%s\r\n", line);

    
    if (conf->getBriefMode() && proxy->isMudEmulation()) {
      sprintf(line, "Exits: ");
      for (i = 0; i <= 5; i++) {
          ExitDirection iDir = static_cast<ExitDirection>(i);
          if (isExitPresent(iDir) == true) {
              if ( isExitUndefined(iDir) ) {
                  sprintf(line + strlen(line), " #%s#", exitnames[i]);
                  continue;
              }
              if ( isExitDeath(iDir) ) {
                  sprintf(line + strlen(line), " !%s!", exitnames[i]);
                  continue;
              }

              if (isDoorSet(iDir)) {
                  if (isDoorSecret(iDir) == false)  {
                      sprintf(line + strlen(line), " (%s)", exitnames[i]);
                  } else {
                      sprintf(line + strlen(line), " +%s+", exitnames[i]);
                  }
              } else {
                  sprintf(line + strlen(line), " %s", exitnames[i]);
              }
          }
      }
      
      
    } else {
      
      line[0] = 0;
      sprintf(line, " exits:");
  
      for (i = 0; i <= 5; i++) {
          ExitDirection iDir = static_cast<ExitDirection>(i);
          if (isExitPresent(iDir) == true) {
              if (isExitUndefined(iDir) ) {
                  sprintf(line + strlen(line), " #%s#", exitnames[i]);
                  continue;
              }
              if (isExitDeath(iDir)) {
                  sprintf(line + strlen(line), " !%s!", exitnames[i]);
                  continue;
              }

              if (isDoorSet(iDir)) {
                  if (isDoorSecret(iDir) == false)  {
                      sprintf(line + strlen(line), " (%s)", exitnames[i]);
                  } else {
                      sprintf(line + strlen(line), " +%s+", exitnames[i]);
                  }
              } else {
                  sprintf(line + strlen(line), " %s", exitnames[i]);
              }

              sprintf(line + strlen(line), " -[to %i]-", getExitLeadsTo(iDir) );
          }
       }

    }

    send_to_user("%s\r\n", line);
}


/* Returns Levenshtein distance between two strings. */
int Strings_Comparator::compare(QByteArray pattern, QByteArray text)
{
  int n, m, i, j;
  int cost;

  if (pattern == text)
    return 0;

  /* Use char arrays for faster access. */
  const char *s1 = pattern.constData();
  const char *s2 = text.constData();

  n = pattern.length();
  m = text.length();

  /* initialization */
  for (i = 0; i <= n; i++)
    D[i][0] = i;

  for (i = 0; i <= m; i++)
    D[0][i] = i;

  /* recurence */
  for (i = 1; i <= n; i++) 
    for (j = 1; j <= m; j++) 
    {
      cost = D[i - 1][j - 1];
      if (s1[i - 1] != s2[j - 1])
        cost += 1;
    
      D[ i ][ j ] = MIN( cost, MIN( D[i - 1][j ] + 1 , D[i][j - 1] + 1) );
    }
  
    
//  print_debug(DEBUG_ROOMS, "result of comparison : %i", D[n][m]);

  return D[n][m];
}


int Strings_Comparator::compare_with_quote(QByteArray str, QByteArray text, int quote)
{
    int n;
    int allowed_errors;
    int result;
    
    n = str.length();
    allowed_errors = (int) ( (double) quote / 100.0  * (double) n );
    
    result = compare(str, text);
    
    if (result == 0) 
        return 0;       /* they match ! */
    
    if (result <= allowed_errors)
        return result;  /* we are a little bit different from the strcmp function */
    
    return -1;  /* else the strings do not match */
}



int Strings_Comparator::strcmp_roomname(QByteArray name, QByteArray text)
{
    return compare_with_quote(name, text, conf->getNameQuote());
}


int Strings_Comparator::strcmp_desc(QByteArray name, QByteArray text)
{
    return compare_with_quote(name, text, conf->getDescQuote());
}


