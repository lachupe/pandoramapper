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

#define MAX_ROOMS       35000		/* maximal amount of rooms */


#define MAX_STR_LEN     400
#define MAX_LINES_DESC  20

#define NORTH           0
#define EAST            1
#define SOUTH           2
#define WEST            3
#define UP              4
#define DOWN            5

/* coordinate's cap */
#define MAX_X           32000
#define MIN_X           -32000


/* global flags */
extern QString *logFileName;

void toggle_renderer_reaction();
void notify_analyzer();


#endif

