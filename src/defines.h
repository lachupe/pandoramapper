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


#define SVN_REVISION	208

class QString;

#define MAX_ROOMS       45000		/* maximal amount of rooms */


#define MAX_STR_LEN     400
#define MAX_LINES_DESC  20

/* coordinate's cap */
#define MAX_X           32000
#define MIN_X           -32000

typedef unsigned int RoomId;

enum ExitType { ET_NORMAL = 0, ET_UNDEFINED = 3, ET_DEATH = 4, ET_NONE = 5 };

enum ExitDirection { ED_NORTH=0, ED_SOUTH, ED_EAST, ED_WEST, ED_UP,
               ED_DOWN, ED_UNKNOWN, ED_NONE };


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

