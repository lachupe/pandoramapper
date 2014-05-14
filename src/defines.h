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
#ifndef DEFINES_H
#define DEFINES_H


#define SVN_REVISION	220

class QString;

#define MAX_ROOMS       200000		/* maximal amount of rooms */


#define MAX_STR_LEN     400
#define MAX_LINES_DESC  20


typedef unsigned int RoomId;

enum ExitDirection { ED_NORTH=0, ED_SOUTH, ED_EAST, ED_WEST, ED_UP,
               ED_DOWN, ED_UNKNOWN, ED_NONE };

enum ExitType { ET_NORMAL = 0, ET_UNDEFINED = 3, ET_DEATH = 4, ET_NONE = 5 };
enum ExitFlag { EF_EXIT = 0, EF_DOOR, EF_ROAD, EF_CLIMB, EF_RANDOM, EF_SPECIAL, EF_NO_MATCH };
enum DoorFlag { DF_HIDDEN = 0, DF_NEEDKEY, DF_NOBLOCK, DF_NOBREAK, DF_NOPICK, DF_DELAYED };

enum RoomMobFlag {
    RMF_RENT, RMF_SHOP, RMF_WEAPONSHOP, RMF_ARMOURSHOP, RMF_FOODSHOP,
    RMF_PETSHOP, RMF_GUILD, RMF_SCOUTGUILD, RMF_MAGEGUILD, RMF_CLERICGUILD,
    RMF_WARRIORGUILD, RMF_RANGERGUILD, RMF_SMOB, RMF_QUEST, RMF_ANY
};

enum RoomLoadFlag {
    RLF_TREASURE, RLF_ARMOUR, RLF_WEAPON, RLF_WATER, RLF_FOOD, RLF_HERB,
    RLF_KEY, RLF_MULE, RLF_HORSE, RLF_PACKHORSE, RLF_TRAINEDHORSE, RLF_ROHIRRIM,
    RLF_WARG, RLF_BOAT, RLF_ATTENTION, RLF_TOWER
};


enum RoomTerrainType    { RTT_UNDEFINED = 0, RTT_INDOORS, RTT_CITY, RTT_FIELD, RTT_FOREST, RTT_HILLS , RTT_MOUNTAINS,
                          RTT_SHALLOW, RTT_WATER, RTT_RAPIDS, RTT_UNDERWATER, RTT_ROAD, RTT_BRUSH,
                          RTT_TUNNEL, RTT_CAVERN, RTT_DEATHTRAP, RTT_RANDOM};

enum RoomPortableType   { RPT_UNDEFINED = 0, RPT_PORTABLE, RPT_NOTPORTABLE };
enum RoomLightType      { RLT_UNDEFINED = 0, RLT_DARK, RLT_LIT };
enum RoomAlignType      { RAT_UNDEFINED = 0, RAT_GOOD, RAT_NEUTRAL, RAT_EVIL };
enum RoomRidableType    { RRT_UNDEFINED = 0, RRT_RIDABLE, RRT_NOTRIDABLE };






/* global flags */
extern QString *logFileName;

void toggle_renderer_reaction();
void notify_analyzer();


#endif

