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

#ifndef ENGINE_H
#define ENGINE_H

#include <QObject>

#include "CRoom.h"
#include "CEvent.h"
#include "CCommandQueue.h"



class CEngine : public QObject {
Q_OBJECT


	enum ExitFlags { E_NOEXIT = 0, E_NORMAL, E_OPENDOOR, E_CLOSEDDOOR, E_PORTAL, E_CLIMBUP, E_CLIMBDOWN };

  /* flags */
    bool mapping;                 /* mapping is On/OFF */
    bool mgoto;
    int   nameMatch;
    int   descMatch;

    CRegion  *users_region;
    CRegion  *last_region;
    CRegion	 *last_warning_region;

    QByteArray last_name;
    QByteArray last_desc;
    QByteArray last_exits;
    QByteArray last_prompt;
    char 	   last_terrain;
    QByteArray last_movement;

    PipeManager  eventPipe;
    Event        event;

    CCommandQueue commandQueue;


    void parseEvent();
    void tryAllDirs();
    void tryDir();
    void tryLook();
    void swap();
    void resync();
    void mappingOff();

    bool testRoom(CRoom *room);

    void mapCurrentRoom(CRoom *room, int dir);

public:
    CEngine();
    ~CEngine();

    CRoom *addedroom;	/* fresh added room */

    void addEvent(Event e) { eventPipe.addEvent(e); }

    void addMovementCommand(int dir) { commandQueue.addCommand(CCommand::MOVEMENT, dir); }
    QVector<unsigned int> *getPrespammedDirs();




    void exec();

    void angryLinker(CRoom *r);
    void printStacks();

    QByteArray getRoomName() { return last_name; }
    QByteArray getDesc() { return last_desc; }
    QByteArray getExits() { return last_exits; }
    QByteArray getPrompt() { return last_prompt; }
    char getTerrain() { return last_terrain; }
    QByteArray getLastMovement() { return last_movement; }

    void setRoomname(QByteArray s) { last_name = s; }
    void setDesc(QByteArray s) { last_desc = s; }
    void setExits(QByteArray s) { last_exits = s; }
    void setTerrain(char c) { last_terrain = c; }

    int compare_exits(CRoom *p, int exits[]);
    void parse_exits(const char *exits_line, int exits[]);
    void do_exits(const char *exits_line);


    bool isMapping() { return mapping; }
    void setMapping(bool b) { mapping = b; }

    void setMgoto(bool b) { mgoto = b; }
    bool isMgoto() { return mgoto; }
    bool empty() { return eventPipe.isEmpty(); };                      /* are pipes empty? */
    void clear();                      /* clears events pipes */

    void set_users_region(CRegion *reg);
    void set_last_region(CRegion *reg);
    CRegion *get_users_region();
    CRegion *get_last_region();

    void updateRegions();

    void resetAddedRoomVar() { addedroom = NULL; }
public slots:
    void slotRunEngine();
    void setPrompt(QByteArray s) { last_prompt = s; }
};

extern class CEngine *engine;

#endif

