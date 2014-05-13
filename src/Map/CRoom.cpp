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
//    rebuildDisplayList();
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

int CRoom::descCmp(QByteArray d) const
{ 
    if (room.desc().empty() != true)
        return comparator.strcmp_desc(d, getDesc());
    else
        return 0;
}

int CRoom::roomnameCmp(QByteArray n) const
{ 
    if (room.name().empty() != true)
        return comparator.strcmp_roomname(n, getName());
    else
        return 0;
}



/* --------------- check if exit in room is connected --------------- */
bool CRoom::isConnected(ExitDirection dir) const
{
    if (isExitUndefined(dir)  || isExitDeath(dir)  )
        return false;

    if (isExitPresent(dir))
        return true;

    return false;
}


/* is there anything at all in this direction ? */
bool CRoom::isExitPresent(ExitDirection dir) const
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



CRoom* CRoom::getExitRoom(ExitDirection dir) const {
    if (isConnected(dir) == false)
        return NULL;

    return parent->getRoom(room.exits().Get(dir).leads_to_id());
}

RoomId CRoom::getExitLeadsTo(ExitDirection dir) const {
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

    // create exit if there is none yet
    if (exit->type() == ET_NONE) {
        setExitUndefined(dir);
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

QByteArray CRoom::getDoor(ExitDirection dir) const
{
    return room.exits(dir).door().c_str();
}

bool CRoom::isDoorSet(ExitDirection dir) const
{
    if (room.exits(dir).door().empty())
        return false;
    else 
        return true;
}

char CRoom::dirbynum(ExitDirection dir) const
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


int CRoom::getX() const
{
    return room.pos().x();
}

int CRoom::getY() const
{
    return room.pos().y();
}

int CRoom::getZ() const
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

QByteArray CRoom::getName() const
{
    return room.name().c_str();
}



void CRoom::setAlignType(RoomAlignType val)
{
    if (val == getAlignType())
        return;
    room.set_flag_align(static_cast<mapdata::Room::RoomAlignType>(val));
    rebuildDisplayList();
    setModified(true);
}

void CRoom::setPortableType(RoomPortableType val)
{
    if (val == getPortableType())
        return;
    room.set_flag_portable(static_cast<mapdata::Room::RoomPortableType>(val));
    rebuildDisplayList();
    setModified(true);
}

void CRoom::setLightType(RoomLightType val)
{
    if (val == getLightType())
        return;
    room.set_flag_light(static_cast<mapdata::Room::RoomLightType>(val));
    rebuildDisplayList();
    setModified(true);
}

void CRoom::setRidableType(RoomRidableType val)
{
    if (val == getRidableType())
        return;
    room.set_flag_rideable(static_cast<mapdata::Room::RoomRidableType>(val));
    rebuildDisplayList();
    setModified(true);
}


RoomLightType CRoom::getLightType() const
{
    return static_cast<RoomLightType>( room.flag_light() );
}


RoomRidableType CRoom::getRidableType() const
{
    return static_cast<RoomRidableType>( room.flag_rideable() );
}

RoomPortableType CRoom::getPortableType() const
{
    return static_cast<RoomPortableType>( room.flag_portable() );
}

RoomAlignType CRoom::getAlignType() const
{
    return static_cast<RoomAlignType>( room.flag_align() );
}


QByteArray CRoom::getDesc() const
{
    return room.desc().c_str();
}

QByteArray CRoom::getDynamicDesc() const
{
    return room.dynamicdesc().c_str();
}


RoomTerrainType CRoom::getTerrain() const
{
    return static_cast<RoomTerrainType>( room.terrain() );
}


QByteArray CRoom::getNote() const
{
    return room.note().c_str();
}


void CRoom::setNoteColor(QByteArray color) {
    room.set_note_color(color);
    rebuildDisplayList();
}

QByteArray CRoom::getNoteColor() const
{
    return room.note_color().c_str();
}

void CRoom::setDesc(QByteArray newdesc)
{
    room.set_desc(newdesc);
    setModified(true);    
}
      
QByteArray CRoom::getSecretsInfo() const
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

void CRoom::setExitLeadsTo(ExitDirection dir, CRoom *r)
{
    room.mutable_exits(dir)->set_leads_to_id(r->getId());
    room.mutable_exits(dir)->set_type( mapdata::Exit::ET_NORMAL );

    // old code used EXIT_NONE ... this is going to deliver some problems!
    //exitFlags[dir] = EXIT_NONE;
    rebuildDisplayList();
}

void CRoom::setExitLeadsTo(ExitDirection dir, RoomId value)
{
    room.mutable_exits(dir)->set_leads_to_id(value);
    room.mutable_exits(dir)->set_type( mapdata::Exit::ET_NORMAL );

    // old code used EXIT_NONE ... this is going to deliver some problems!
    //exitFlags[dir] = EXIT_NONE;
    rebuildDisplayList();
}


bool CRoom::isExitUndefined(ExitDirection dir) const
{
    return room.exits(dir).type() == mapdata::Exit::ET_UNDEFINED;
}

void CRoom::setExitUndefined(ExitDirection dir)
{
    room.mutable_exits(dir)->set_leads_to_id(0);
    room.mutable_exits(dir)->set_type( mapdata::Exit::ET_UNDEFINED );
    rebuildDisplayList();
}



bool CRoom::isExitDeath(ExitDirection dir) const
{
    return room.exits(dir).type() == mapdata::Exit::ET_DEATH;
}

bool CRoom::isExitNormal(ExitDirection dir) const
{
    return room.exits(dir).type() == mapdata::Exit::ET_NORMAL;
}


bool CRoom::isMobFlagSet(RoomMobFlag flag) const
{
    switch(flag) {
        case RMF_RENT:
            return room.mob_flags().rent();
        case RMF_SHOP:
            return room.mob_flags().shop();
        case RMF_WEAPONSHOP:
            return room.mob_flags().weaponshop();
        case RMF_ARMOURSHOP:
            return room.mob_flags().armourshop();
        case RMF_FOODSHOP:
            return room.mob_flags().foodshop();
        case RMF_PETSHOP:
            return room.mob_flags().petshop();
        case RMF_GUILD:
            return room.mob_flags().guild();
        case RMF_SCOUTGUILD:
            return room.mob_flags().scoutguild();
        case RMF_MAGEGUILD:
            return room.mob_flags().mageguild();
        case RMF_CLERICGUILD:
            return room.mob_flags().clericguild();
        case RMF_WARRIORGUILD:
            return room.mob_flags().warriorguild();
        case RMF_RANGERGUILD:
            return room.mob_flags().rangerguild();
        case RMF_SMOB:
            return room.mob_flags().smob();
        case RMF_QUEST:
            return room.mob_flags().quest();
        case RMF_ANY:
            return room.mob_flags().any();
    }
}



void CRoom::setMobFlag(RoomMobFlag flag, bool value)
{
    switch(flag) {
        case RMF_RENT:
            room.mutable_mob_flags()->set_rent(value);
            break;
        case RMF_SHOP:
            room.mutable_mob_flags()->set_shop(value);
            break;
        case RMF_WEAPONSHOP:
            room.mutable_mob_flags()->set_weaponshop(value);
            break;
        case RMF_ARMOURSHOP:
            room.mutable_mob_flags()->set_armourshop(value);
            break;
        case RMF_FOODSHOP:
            room.mutable_mob_flags()->set_foodshop(value);
            break;
        case RMF_PETSHOP:
            room.mutable_mob_flags()->set_petshop(value);
            break;
        case RMF_GUILD:
            room.mutable_mob_flags()->set_guild(value);
            break;
        case RMF_SCOUTGUILD:
            room.mutable_mob_flags()->set_scoutguild(value);
            break;
        case RMF_MAGEGUILD:
            room.mutable_mob_flags()->set_mageguild(value);
            break;
        case RMF_CLERICGUILD:
            room.mutable_mob_flags()->set_clericguild(value);
            break;
        case RMF_WARRIORGUILD:
            room.mutable_mob_flags()->set_warriorguild(value);
            break;
        case RMF_RANGERGUILD:
            room.mutable_mob_flags()->set_rangerguild(value);
            break;
        case RMF_SMOB:
            room.mutable_mob_flags()->set_smob(value);
            break;
        case RMF_QUEST:
            room.mutable_mob_flags()->set_quest(value);
            break;
        case RMF_ANY:
            room.mutable_mob_flags()->set_any(value);
            break;
    }
}


bool CRoom::isLoadFlagSet(RoomLoadFlag flag) const
{
    switch(flag) {
    case RLF_TREASURE:
        return room.load_flags().treasure();
        break;
    case RLF_ARMOUR:
        return room.load_flags().armour();
        break;
    case RLF_WEAPON:
        return room.load_flags().weapon();
        break;
    case RLF_WATER:
        return room.load_flags().water();
        break;
    case RLF_FOOD:
        return room.load_flags().food();
        break;
    case RLF_HERB:
        return room.load_flags().herb();
        break;
    case RLF_KEY:
        return room.load_flags().key();
        break;
    case RLF_MULE:
        return room.load_flags().mule();
        break;
    case RLF_HORSE:
        return room.load_flags().horse();
        break;
    case RLF_PACKHORSE:
        return room.load_flags().packhorse();
        break;
    case RLF_TRAINEDHORSE:
        return room.load_flags().trainedhorse();
        break;
    case RLF_ROHIRRIM:
        return room.load_flags().rohirrim();
        break;
    case RLF_WARG:
        return room.load_flags().warg();
        break;
    case RLF_BOAT:
        return room.load_flags().boat();
        break;
    case RLF_ATTENTION:
        return room.load_flags().attention();
        break;
    case RLF_TOWER:
        return room.load_flags().tower();
        break;
    }
}

void CRoom::setLoadFlag(RoomLoadFlag flag, bool value)
{
    switch(flag) {
    case RLF_TREASURE:
        room.mutable_load_flags()->set_treasure(value);
        break;
    case RLF_ARMOUR:
        room.mutable_load_flags()->set_armour(value);
        break;
    case RLF_WEAPON:
        room.mutable_load_flags()->set_weapon(value);
        break;
    case RLF_WATER:
        room.mutable_load_flags()->set_water(value);
        break;
    case RLF_FOOD:
        room.mutable_load_flags()->set_food(value);
        break;
    case RLF_HERB:
        room.mutable_load_flags()->set_herb(value);
        break;
    case RLF_KEY:
        room.mutable_load_flags()->set_key(value);
        break;
    case RLF_MULE:
        room.mutable_load_flags()->set_mule(value);
        break;
    case RLF_HORSE:
        room.mutable_load_flags()->set_horse(value);
        break;
    case RLF_PACKHORSE:
        room.mutable_load_flags()->set_packhorse(value);
        break;
    case RLF_TRAINEDHORSE:
        room.mutable_load_flags()->set_trainedhorse(value);
        break;
    case RLF_ROHIRRIM:
        room.mutable_load_flags()->set_rohirrim(value);
        break;
    case RLF_WARG:
        room.mutable_load_flags()->set_warg(value);
        break;
    case RLF_BOAT:
        room.mutable_load_flags()->set_boat(value);
        break;
    case RLF_ATTENTION:
        room.mutable_load_flags()->set_attention(value);
        break;
    case RLF_TOWER:
        room.mutable_load_flags()->set_tower(value);
        break;
    }
}



bool CRoom::isExitFlagSet(ExitDirection dir, ExitFlag flag) const
{
    switch(flag) {
        case EF_EXIT: // special case
            return isExitPresent(dir);
        case EF_DOOR:
            return isDoorSet(dir);
        case EF_ROAD:
            return room.exits(dir).exit_flags().road();
        case EF_CLIMB:
            return room.exits(dir).exit_flags().climb();
        case EF_SPECIAL:
            return room.exits(dir).exit_flags().special();
        case EF_NO_MATCH:
            return room.exits(dir).exit_flags().no_match();
    };

    return false;
}

bool CRoom::isDoorFlagSet(ExitDirection dir, DoorFlag flag) const
{
    switch(flag) {
    case DF_HIDDEN:
        return isDoorSecret(dir);
    case DF_NEEDKEY:
        return room.exits(dir).door_flags().needkey();
    case DF_NOBLOCK:
        return room.exits(dir).door_flags().noblock();
    case DF_NOBREAK:
        return room.exits(dir).door_flags().nobreak();
    case DF_NOPICK:
        return room.exits(dir).door_flags().nopick();
    case DF_DELAYED:
        return room.exits(dir).door_flags().delayed();
    };

    return false;
}

void CRoom::setExitFlag(ExitDirection dir, ExitFlag flag, bool value)
{
    switch(flag) {
        case EF_EXIT: // special case
            if (value && isExitPresent(dir) == false) {
                // add exit
                setExitUndefined(dir);
            } else if (!value && isExitPresent(dir)) {
                // remove exit
                removeExit(dir);
            }
            break;
        case EF_DOOR:
            if (value) {
                // add default door
                setDoor(dir, "exit");
            } else {
                // remove door, if any
                removeDoor(dir);
            }
            break;
        case EF_ROAD:
            room.mutable_exits(dir)->mutable_exit_flags()->set_road(value);
            break;
        case EF_CLIMB:
            room.mutable_exits(dir)->mutable_exit_flags()->set_climb(value);
            break;
        case EF_SPECIAL:
            room.mutable_exits(dir)->mutable_exit_flags()->set_special(value);
            break;
        case EF_NO_MATCH:
            room.mutable_exits(dir)->mutable_exit_flags()->set_no_match(value);
            break;
    };
}

void CRoom::setDoorFlag(ExitDirection dir, DoorFlag flag, bool value)
{
    switch(flag) {
    case DF_HIDDEN:
        // read-only flag
        break;
    case DF_NEEDKEY:
        room.mutable_exits(dir)->mutable_door_flags()->set_needkey(value);
        break;
    case DF_NOBLOCK:
        room.mutable_exits(dir)->mutable_door_flags()->set_noblock(value);
        break;
    case DF_NOBREAK:
        room.mutable_exits(dir)->mutable_door_flags()->set_nobreak(value);
        break;
    case DF_NOPICK:
        room.mutable_exits(dir)->mutable_door_flags()->set_nopick(value);
        break;
    case DF_DELAYED:
        room.mutable_exits(dir)->mutable_door_flags()->set_delayed(value);
        break;
    };
}



void CRoom::setExitDeath(ExitDirection dir)
{
    room.mutable_exits(dir)->set_type(mapdata::Exit::ET_DEATH);
    room.mutable_exits(dir)->set_leads_to_id(0);
    rebuildDisplayList();
    setModified(true);
}



bool CRoom::anyUndefinedExits() const
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


bool CRoom::isDoorSecret(ExitDirection dir) const
{
    if (!isDoorSet(dir))
        return false;

    if (room.exits(dir).door() == std::string("exit"))
        return false;

    return true;
}

QByteArray CRoom::getRegionName() const
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
    
CRegion *CRoom::getRegion() const
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

QString CRoom::toolTip() const
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
void CRoom::sendRoom() const
{
    unsigned int i, pos;
    char line[MAX_STR_LEN];
    
    send_to_user(" Id: %i, Flags: %s, Region: %s, Coord: %i,%i,%i\r\n", getId(),
                     (const char *) conf->sectors[ (int) getTerrain() ].desc,
	    (const char *) region->getName(),
        getX(), getY(), getZ());
    send_to_user(" [32m%s[0m\r\n", (const char *) getName() );

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
    send_to_user(" note: %s\r\n", (const char *) getNote());

    
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


