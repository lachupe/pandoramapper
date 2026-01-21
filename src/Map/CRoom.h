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
// C++ Interface: croom
//
// Description:
//
//
// Author: Azazello <aza@alpha>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CROOM_H
#define CROOM_H

#include <QByteArray>

#include "defines.h"

#include "Map/CRegion.h"
#include "Renderer/CSquare.h"

struct room_flag_data
{
    QByteArray name;
    QByteArray xml_name;
    unsigned int flag;
};

extern const struct room_flag_data room_flags[];

// MMapper room property enums
enum MMLightType : uint8_t
{
    MM_LIGHT_UNDEFINED = 0,
    MM_LIGHT_DARK = 1,
    MM_LIGHT_LIT = 2
};

enum MMAlignType : uint8_t
{
    MM_ALIGN_UNDEFINED = 0,
    MM_ALIGN_GOOD = 1,
    MM_ALIGN_NEUTRAL = 2,
    MM_ALIGN_EVIL = 3
};

enum MMPortableType : uint8_t
{
    MM_PORTABLE_UNDEFINED = 0,
    MM_PORTABLE_PORTABLE = 1,
    MM_PORTABLE_NOT_PORTABLE = 2
};

enum MMRidableType : uint8_t
{
    MM_RIDABLE_UNDEFINED = 0,
    MM_RIDABLE_RIDABLE = 1,
    MM_RIDABLE_NOT_RIDABLE = 2
};

enum MMSundeathType : uint8_t
{
    MM_SUNDEATH_UNDEFINED = 0,
    MM_SUNDEATH_SUNDEATH = 1,
    MM_SUNDEATH_NO_SUNDEATH = 2
};

// MMapper MobFlags (19 flags)
enum MMMobFlag : uint32_t
{
    MM_MOB_RENT = 1 << 0,
    MM_MOB_SHOP = 1 << 1,
    MM_MOB_WEAPON_SHOP = 1 << 2,
    MM_MOB_ARMOUR_SHOP = 1 << 3,
    MM_MOB_FOOD_SHOP = 1 << 4,
    MM_MOB_PET_SHOP = 1 << 5,
    MM_MOB_GUILD = 1 << 6,
    MM_MOB_SCOUT_GUILD = 1 << 7,
    MM_MOB_MAGE_GUILD = 1 << 8,
    MM_MOB_CLERIC_GUILD = 1 << 9,
    MM_MOB_WARRIOR_GUILD = 1 << 10,
    MM_MOB_RANGER_GUILD = 1 << 11,
    MM_MOB_AGGRESSIVE_MOB = 1 << 12,
    MM_MOB_QUEST_MOB = 1 << 13,
    MM_MOB_PASSIVE_MOB = 1 << 14,
    MM_MOB_ELITE_MOB = 1 << 15,
    MM_MOB_SUPER_MOB = 1 << 16,
    MM_MOB_MILKABLE = 1 << 17,
    MM_MOB_RATTLESNAKE = 1 << 18
};

// MMapper LoadFlags (25 flags)
enum MMLoadFlag : uint32_t
{
    MM_LOAD_TREASURE = 1 << 0,
    MM_LOAD_ARMOUR = 1 << 1,
    MM_LOAD_WEAPON = 1 << 2,
    MM_LOAD_WATER = 1 << 3,
    MM_LOAD_FOOD = 1 << 4,
    MM_LOAD_HERB = 1 << 5,
    MM_LOAD_KEY = 1 << 6,
    MM_LOAD_MULE = 1 << 7,
    MM_LOAD_HORSE = 1 << 8,
    MM_LOAD_PACK_HORSE = 1 << 9,
    MM_LOAD_TRAINED_HORSE = 1 << 10,
    MM_LOAD_ROHIRRIM = 1 << 11,
    MM_LOAD_WARG = 1 << 12,
    MM_LOAD_BOAT = 1 << 13,
    MM_LOAD_ATTENTION = 1 << 14,
    MM_LOAD_TOWER = 1 << 15,
    MM_LOAD_CLOCK = 1 << 16,
    MM_LOAD_MAIL = 1 << 17,
    MM_LOAD_STABLE = 1 << 18,
    MM_LOAD_WHITE_WORD = 1 << 19,
    MM_LOAD_DARK_WORD = 1 << 20,
    MM_LOAD_EQUIPMENT = 1 << 21,
    MM_LOAD_COACH = 1 << 22,
    MM_LOAD_FERRY = 1 << 23,
    MM_LOAD_DEATHTRAP = 1 << 24
};

// MMapper ExitFlags (13 flags)
enum MMExitFlag : uint16_t
{
    MM_EXIT_EXIT = 1 << 0,
    MM_EXIT_DOOR = 1 << 1,
    MM_EXIT_ROAD = 1 << 2,
    MM_EXIT_CLIMB = 1 << 3,
    MM_EXIT_RANDOM = 1 << 4,
    MM_EXIT_SPECIAL = 1 << 5,
    MM_EXIT_NO_MATCH = 1 << 6,
    MM_EXIT_FLOW = 1 << 7,
    MM_EXIT_NO_FLEE = 1 << 8,
    MM_EXIT_DAMAGE = 1 << 9,
    MM_EXIT_FALL = 1 << 10,
    MM_EXIT_GUARDED = 1 << 11,
    MM_EXIT_STUB = 1 << 12
};

// MMapper DoorFlags (11 flags)
enum MMDoorFlag : uint16_t
{
    MM_DOOR_HIDDEN = 1 << 0,
    MM_DOOR_NEED_KEY = 1 << 1,
    MM_DOOR_NO_BLOCK = 1 << 2,
    MM_DOOR_NO_BREAK = 1 << 3,
    MM_DOOR_NO_PICK = 1 << 4,
    MM_DOOR_DELAYED = 1 << 5,
    MM_DOOR_CALLABLE = 1 << 6,
    MM_DOOR_KNOCKABLE = 1 << 7,
    MM_DOOR_MAGIC = 1 << 8,
    MM_DOOR_ACTION = 1 << 9,
    MM_DOOR_NO_BASH = 1 << 10
};

class Strings_Comparator
{
  private:
    static const int MAX_N = MAX_LINES_DESC * 80;
    static const int MAX_M = MAX_LINES_DESC * 80;
    int D[MAX_N][MAX_M];

  public:
    int compare(QByteArray pattern, QByteArray text);
    int compare_with_quote(QByteArray str, QByteArray text, int quote);
    int strcmp_roomname(QByteArray name, QByteArray text);
    int strcmp_desc(QByteArray name, QByteArray text);
};

extern Strings_Comparator comparator;

class CRoom
{
    unsigned int flags;
    QByteArray name;      /* POINTER to the room name */
    QByteArray note;      /* note, if needed, additional info etc */
    QByteArray noteColor; /* note color in this room */
    QByteArray desc;      /* descrition */
    char sector;          /* terrain marker */
                          /* _no need to free this one_ */
    CRegion *region;      /* region of this room */

    QByteArray doors[6]; /* if the door is secret */
    unsigned char exitFlags[6];

    CSquare *square; /* which square this room belongs to */

    int x, y, z; /* coordinates on our map */

    // MMapper room properties
    uint8_t lightType;      // MMLightType
    uint8_t alignType;      // MMAlignType
    uint8_t portableType;   // MMPortableType
    uint8_t ridableType;    // MMRidableType
    uint8_t sundeathType;   // MMSundeathType
    uint32_t mobFlags;      // MMMobFlag bitmask
    uint32_t loadFlags;     // MMLoadFlag bitmask
    QByteArray contents;    // Room contents description

    // MMapper exit properties (per direction)
    uint16_t mmExitFlags[6];  // MMExitFlag bitmask per direction
    uint16_t mmDoorFlags[6];  // MMDoorFlag bitmask per direction

  public:
    enum ExitFlags
    {
        EXIT_NONE = 0,
        EXIT_UNDEFINED,
        EXIT_DEATH
    };

    unsigned int id; /* identifier, public for speed up - its very often used  */
    CRoom *exits[6]; /* very often used in places where performance matters */

    CRoom();
    ~CRoom();

    QByteArray getName();
    QByteArray getDesc();
    char getTerrain();
    QByteArray getNote();

    QByteArray getNoteColor();
    void setNoteColor(QByteArray color);

    void setDesc(QByteArray newdesc);
    void setName(QByteArray newname);
    void setTerrain(char terrain);
    void setSector(char val);
    void setNote(QByteArray note);

    void setSquare(CSquare *square);
    CSquare *getSquare() { return square; }

    bool isDescSet();
    bool isNameSet();
    bool isEqualNameAndDesc(CRoom *room);

    QString toolTip();

    void setModified(bool b);
    bool isConnected(int dir);
    void sendRoom();

    // door stuff
    int setDoor(int dir, QByteArray door);
    void removeDoor(int dir);
    QByteArray getDoor(int dir);
    bool isDoorSet(int dir);
    bool isDoorSecret(int dir);

    void disconnectExit(int dir); /* just detaches the connection */
    void removeExit(int dir);     /* also removes the door */
    void setExit(int dir, CRoom *room);
    void setExit(int dir, unsigned int id);
    //    CRoom *getExit(int dir);
    bool isExitLeadingTo(int dir, CRoom *room);

    bool isExitDeath(int dir);
    void setExitDeath(int dir);

    bool isExitNormal(int dir);
    bool
    isExitPresent(int dir); /* if there is anything at all in this direction, deathtrap, undefined exit or normal one */
    bool isExitUndefined(int dir);
    void setExitUndefined(int dir);

    bool anyUndefinedExits();

    void setExitFlags(int dir, unsigned char flag);

    // coordinates
    void setX(int x);
    void setY(int x);
    void setZ(int x);
    void simpleSetZ(int val);  // does not perform any Plane operations (see rendering) on  CPlane in Map.h

    inline int getX() { return x; }
    inline int getY() { return y; }
    inline int getZ() { return z; }

    int descCmp(QByteArray desc);
    int roomnameCmp(QByteArray name);

    QByteArray getRegionName();
    CRegion *getRegion();
    void setRegion(QByteArray name);
    void setRegion(CRegion *reg);
    QByteArray getSecretsInfo();
    QByteArray getDoorAlias(int i);

    char dirbynum(int dir);

    void rebuildDisplayList()
    {
        if (square)
            square->rebuildDisplayList();
    }

    // MMapper property getters/setters
    uint8_t getLightType() const { return lightType; }
    void setLightType(uint8_t type) { lightType = type; }

    uint8_t getAlignType() const { return alignType; }
    void setAlignType(uint8_t type) { alignType = type; }

    uint8_t getPortableType() const { return portableType; }
    void setPortableType(uint8_t type) { portableType = type; }

    uint8_t getRidableType() const { return ridableType; }
    void setRidableType(uint8_t type) { ridableType = type; }

    uint8_t getSundeathType() const { return sundeathType; }
    void setSundeathType(uint8_t type) { sundeathType = type; }

    uint32_t getMobFlags() const { return mobFlags; }
    void setMobFlags(uint32_t flags) { mobFlags = flags; }

    uint32_t getLoadFlags() const { return loadFlags; }
    void setLoadFlags(uint32_t flags) { loadFlags = flags; }

    QByteArray getContents() const { return contents; }
    void setContents(const QByteArray &c) { contents = c; }

    uint16_t getMMExitFlags(int dir) const { return (dir >= 0 && dir < 6) ? mmExitFlags[dir] : 0; }
    void setMMExitFlags(int dir, uint16_t flags) { if (dir >= 0 && dir < 6) mmExitFlags[dir] = flags; }

    uint16_t getMMDoorFlags(int dir) const { return (dir >= 0 && dir < 6) ? mmDoorFlags[dir] : 0; }
    void setMMDoorFlags(int dir, uint16_t flags) { if (dir >= 0 && dir < 6) mmDoorFlags[dir] = flags; }

    // Helper methods for displaying MMapper properties
    static QByteArray mobFlagsToString(uint32_t flags);
    static QByteArray loadFlagsToString(uint32_t flags);
    static QByteArray exitFlagsToString(uint16_t flags);
    static QByteArray doorFlagsToString(uint16_t flags);
    static const char *lightTypeToString(uint8_t type);
    static const char *alignTypeToString(uint8_t type);
    static const char *portableTypeToString(uint8_t type);
    static const char *ridableTypeToString(uint8_t type);
    static const char *sundeathTypeToString(uint8_t type);
};

#endif
