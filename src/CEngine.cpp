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

/* rewritten analyzer engine. */
//#include <cstdio>
//#include <cstdlib>
//#include <cstring>
//#include <ctype.h>
#include <QMutex>
#include <QTime>
#include <QTimer>

#include "CConfigurator.h"
#include "defines.h"
#include "CStacksManager.h"
#include "CDispatcher.h"
#include "exits.h"
#include "proxy.h"
#include "utils.h"
#include "CEngine.h"
#include "CRoomManager.h"
#include "CTree.h"
#include "userfunc.h"
#include "CEvent.h"

class CEngine *engine;


/*---------------- * MAPPING OFF ---------------------------- */
void CEngine::mappingOff()
{
    if (mapping) {
        send_to_user("--[ Mapping is now OFF!\r\n");
        mapping = 0;
    }
}
/*---------------- * MAPPING OFF ---------------------------- */

/*---------------- * SWAP  ------------------------- */
void CEngine::swap()
{
    print_debug(DEBUG_ANALYZER, "in swap");
    stacker.swap();

    if (mapping && stacker.amount() != 1)
        mappingOff();
}
/*---------------- * SWAP  ------------------------- */


/*---------------- * RESYNC * ------------------------- */
void CEngine::resync()
{
  int j;
  TTree *n;

  mappingOff();

  print_debug(DEBUG_ANALYZER, "FULL RESYNC");
  n = NameMap.findByName(last_name);
  if (n != NULL)
    for (j = 0; j < n->ids.size(); j++) {
      if (last_name == Map.getName( n->ids[j] )) {
//        print_debug(DEBUG_ANALYZER, "Adding matches");
        stacker.put( n->ids[j] );
      }
    }

  stacker.swap();
}

bool CEngine::testRoom(CRoom *room)
{
    if (event.blind)
        return true;
    if  ((nameMatch = room->roomnameCmp(event.name)) >= 0) {
        if (event.desc == "")
            return true;
        else if ( (descMatch = room->descCmp(event.desc)) >= 0 )
            return true;
    }
    return false;
}


void CEngine::tryDir()
{
    int dir;
    unsigned int i;
    CRoom *room;
    CRoom *candidate;

    nameMatch = 0;
    descMatch = 0;

    print_debug(DEBUG_ANALYZER, "in try_dir");
    dir = numbydir(event.dir[0]);
    if (dir == -1) {
        print_debug(DEBUG_ANALYZER, "Error in try_dir - faulty DIR given as input!\r\n");
        return;
    }

    CCommand cmd = commandQueue.peek();
    if (cmd.timer.elapsed() > 5000) {
    	print_debug(DEBUG_ANALYZER, "The command queue has head entry with lifetime over 5 seconds. Resetting");
    	commandQueue.clear();
    } else if (cmd.dir == dir && !event.fleeing ) {
    	// we moved in awaited direction
    	commandQueue.dequeue();
    }


    if (stacker.amount() == 0) {
        print_debug(DEBUG_ANALYZER, "leaving. No candidates in stack to check. This results in FULL RESYNC.");
        return;
    }

    for (i = 0; i < stacker.amount(); i++) {
        room = stacker.get(i);
        if (room->isConnected(dir)) {
            candidate = room->exits[dir];
            if  (testRoom(candidate) ) {
                stacker.put(candidate);
            }

        } else {
            if (stacker.amount() == 1 && mapping)  {
                print_debug(DEBUG_ANALYZER, "Going to add new room...");
                mapCurrentRoom(room, dir);
                return;
            }
        }

    }

    /* roomname update */
    if (stacker.next() == 1) {
        /* this means we have exactly one match */
//        printf("nameMatch %i, descMatch %i\r\n", nameMatch, descMatch);
        if (nameMatch > 0) {
            /* Autorefresh only if case has been changed. */
            if (conf->getAutorefresh() && event.name.toLower() == stacker.nextFirst()->getName().toLower()) {
                send_to_user("--[ (AutoRefreshed) not exact room name match: %i errors.\r\n", nameMatch);
                stacker.nextFirst()->setName(event.name);
            } else {
                send_to_user("--[ not exact room name match: %i errors. Use 'mrefresh' to fix it!\r\n", nameMatch);
            }
        }
        if (conf->getAutorefresh() && descMatch > 0) {
            send_to_user("--[ (AutoRefreshed) not exact room desc match: %i errors.\r\n", descMatch);
            stacker.nextFirst()->setDesc(event.desc);
        } else if (!conf->getAutorefresh() && descMatch > 0) {
            send_to_user("--[ not exact room desc match: %i errors.\r\n", descMatch);
        }
    }


    print_debug(DEBUG_ANALYZER, "leaving tryDir");
}

/* now try all dirs, only removes other rooms, if there is a full 100% fit for new data */
/* resyncs only if the stacks are empty */
void CEngine::tryAllDirs()
{
    int dir;
    unsigned int i;
    CRoom *room;
    CRoom *candidate;

    mappingOff();

    print_debug(DEBUG_ANALYZER, "in try_dir_all_dirs");
    if (stacker.amount() == 0) {
        print_debug(DEBUG_ANALYZER, "leaving. No candidates in stack to check. This results in FULL RESYNC.");
        return;
    }

    for (i = 0; i < stacker.amount(); i++) {
        room = stacker.get(i);
        for (dir = 0; dir <= 5; dir++) {
            if (room->isConnected(dir)) {
                candidate = room->exits[dir];
                if  (testRoom(candidate) )
                    stacker.put(candidate);
            }
        }

    }

    print_debug(DEBUG_ANALYZER, "leaving try_dir_all_dirs");
}

void CEngine::tryLook()
{
    unsigned int i;
    CRoom *room;

    print_debug(DEBUG_ANALYZER, "in tryLook");

    if (stacker.amount() == 0) {
        print_debug(DEBUG_ANALYZER, "leaving. No candidates in stack to check. This results in FULL RESYNC.");
        return;
    }

    for (i = 0; i < stacker.amount(); i++) {
        room = stacker.get(i);
        if  (testRoom(room))
            stacker.put(room);

    }
}


void CEngine::slotRunEngine()
{
    print_debug(DEBUG_ANALYZER, "In slotRunEngine");

    // just ignore the even it the system is blocking.
    // supposedly the event will be repeated sometime later =)
    // for engine and userland redraw this does not matter much
    if (Map.tryLockForRead() == false) {
    	print_debug(DEBUG_GENERAL, "slotRunEngine tried to block the eventQueue. Delayed.");
    	QTimer::singleShot( 50, this, SLOT(slotRunEngine()) );
    	return;
    } else
    	Map.unlock();


    if (userland_parser->is_empty()) {
        print_debug(DEBUG_ANALYZER, "Calling the analyzer");
        exec();
    } else {
        print_debug(DEBUG_ANALYZER, "Calling userland Parser.");
        userland_parser->parse_command();
    }

    print_debug(DEBUG_ANALYZER, "leaving slotRunEngine");
}


void CEngine::parseEvent()
{
    print_debug(DEBUG_ANALYZER, "in parseEvent()");

    if (event.movementBlocker) {
    	commandQueue.dequeue();
    	return;
    }

    if (event.name != "") {
        print_debug(DEBUG_ANALYZER, "Converting Room Name to ascii format");
        latinToAscii( event.name);
        last_name = event.name;
    }

    if (event.desc != "") {
        print_debug(DEBUG_ANALYZER, "Converting Description to ascii format");
        latinToAscii( event.desc );
        last_desc = event.desc;
    }
    if (event.exits != "") {
        last_exits = event.exits;
    }
    if (event.terrain != -1) {
        last_terrain = event.terrain;
    }

    setMgoto( false );    /* if we get a new room data incoming, mgoto has to go away */

    print_debug(DEBUG_ANALYZER, "Entering the main part of the function");

    print_debug(DEBUG_ANALYZER, "ANALYZER Event. \r\nNAME %s\r\nDESC %s\r\nEXITS %s\r\nBLIND %i, MOVEMENT %i SCOUT %i",
        (const char *) event.name, (const char *) event.desc, (const char *) event.exits
        /*(const char *) event.prompt*/, event.blind, event.movement, event.scout);

    if (event.scout) {
        print_debug(DEBUG_ANALYZER, "SCOUT flag is set. Dropping event");
        return;
    }


    if (event.name.indexOf("It is pitch black...") == 0) {
        print_debug(DEBUG_ANALYZER, "NO light BLIND set");
        event.blind = true;
    }
    if (event.name == "" && event.movement == true) {
        print_debug(DEBUG_ANALYZER, "NAME is empty and Movement is true. Assuming BLIND");
        event.blind = true;
    }

    if (event.name == "" && event.blind == false) {
        print_debug(DEBUG_ANALYZER, "EMPTY name and no blind set. Assuming addedroom-data update incoming.");
        if (addedroom) {
            addedroom->setTerrain(last_terrain);
            resetAddedRoomVar();
            toggle_renderer_reaction();
        }
        return;
    }


    if (event.movement == true) {
        last_movement = event.dir;
    	if (event.dir =="") {
            tryAllDirs();
            // command's queue is useless then, no?
            commandQueue.clear();
    	} else {
            tryDir();
    	}
    } else {
        if (event.name != "")
            tryLook();
    }


    swap();

    if (stacker.amount() == 0)
        resync();

    print_debug(DEBUG_ANALYZER, "Done. Sending an event to the Renderer");
    toggle_renderer_reaction();
}



CEngine::CEngine() : QObject()
{
  /* setting defaults */

  clear();
}



CEngine::~CEngine()
{
 /* DESTRUCTOR */
}


void CEngine::exec()
{

    print_debug(DEBUG_ANALYZER, "in main cycle");
    QTime t;
    t.start();


    if (eventPipe.isEmpty())
        return;


    print_debug(DEBUG_ANALYZER, "trying to dequeue the pipe ...");
    event = eventPipe.getEvent();
    print_debug(DEBUG_ANALYZER, "preparing to call the event parser...");
    parseEvent();

    print_debug(DEBUG_ANALYZER, "updating regions");
    updateRegions();


    print_debug(DEBUG_ANALYZER, "done. Time elapsed %d ms", t.elapsed());
    return;
}

void CEngine::updateRegions()
{
    CRoom *r;

    print_debug(DEBUG_ANALYZER, "in updateRegions");


    // update Regions info only if we are in full sync
    if (stacker.amount() == 1) {
        r = stacker.first();

        last_region = r->getRegion();
        // If this room was JUST added, it has no region set.
        if (last_region == NULL) {
        	return; // probably it needs some heavier work ...region settings might not work correctly
        }

        if (last_region != users_region && conf->getRegionsAutoReplace() == false) {

        	if (last_warning_region != last_region && conf->getRegionsAutoSet() == false) {
        		send_to_user("--[ Moved to another region: new region %s\r\n", (const char *)  last_region->getName() );
        		last_warning_region = last_region;
        	}

        	if (conf->getRegionsAutoSet())
                users_region = last_region;
        }

        if (conf->getRegionsAutoReplace() && last_region != users_region) {
                // update is required ...
                send_to_user( "--[ Regions update: Room region changed from %s to %s\r\n",
                                        (const char *) last_region->getName(),
                                        (const char *) users_region->getName());
                r->setRegion(users_region);
                last_region = users_region;
                toggle_renderer_reaction();
        }

    }


    print_debug(DEBUG_ANALYZER, "leaving updateRegions");
}


// where from where to ... map it!
void CEngine::mapCurrentRoom(CRoom *room, int dir)
{
    print_debug(DEBUG_ANALYZER, "in mapCurrentRoom");


    /* casual checks for data */
    if (event.blind) {
        send_to_user( "--[Pandora: Failed to add new room. Blind !\r\n");
        mappingOff();
        return;
    } else if (event.name == "") {
        send_to_user( "--[Pandora: Failed to add new room. Missing roomname!\r\n");
        mappingOff();
        return;
    } else if (event.desc == "") {
        send_to_user( "--[Pandora: Failed to add new room. Missing description!\r\n");
        mappingOff();
        return;
    } else if (event.exits == "") {
        send_to_user( "--[Pandora: Failed to add new room. Missing exits data!\r\n");
        mappingOff();
        return;
    }
    send_to_user("--[ Adding new room!\n");

    Map.fixFreeRooms();	// making this call just for more safety - might remove

    addedroom = new CRoom;

    addedroom->id = Map.next_free;
    addedroom->setName(event.name);
    addedroom->setDesc(event.desc);
    addedroom->setRegion( users_region );

    room->setExit(dir, addedroom);
    if (conf->getDuallinker() == true)
        addedroom->setExit(reversenum(dir), room);
    else
        Map.oneway_room_id = room->id;

    setExits(event.exits);
    do_exits((const char *) event.exits);


    int x = room->getX();
    int y = room->getY();
    int z = room->getZ();

    if (dir == NORTH)	    y += 2;
    if (dir == SOUTH)       y -= 2;
    if (dir == EAST)          x+= 2;
    if (dir == WEST)         x -= 2;
    if (dir == UP)	            z += 2;
    if (dir == DOWN)        z -= 2;

    addedroom->setX(x);
    addedroom->setY(y);
    addedroom->simpleSetZ(z);


    Map.addRoom(addedroom);
    stacker.put(addedroom);

    if (Map.isDuplicate(addedroom) == true) {
    	resetAddedRoomVar();
    	send_to_user("--[Pandora: Twin rooms merged!\n");
    	proxy->send_line_to_user( (const char *) last_prompt );
    	print_debug(DEBUG_ANALYZER, "Twins merged");
    } else {
        angryLinker(addedroom);
    }

    print_debug(DEBUG_ANALYZER, "leaving mapCurrentRoom");

    return;
}


void CEngine::angryLinker(CRoom *r)
{
  CRoom *p;
  unsigned int i;
  CRoom *candidates[6];
  int distances[6];
  int z;


  if (!conf->getAngrylinker())
    return;

  print_debug(DEBUG_ROOMS && DEBUG_ANALYZER, "in AngryLinker");

  if (r == NULL) {
    print_debug(DEBUG_ROOMS, "given room is NULL");
    return;
  }

  if (r->anyUndefinedExits() != true) {
    print_debug(DEBUG_ROOMS, "returning, no undefined exits in room found");
    return;     /* no need to try and link this room - there are no undefined exits */
  }

  /* reset the data */
  for (i=0; i <= 5; i++) {
    distances[i] = 15000;
    candidates[i] = 0;
  }
  z = 0;

  // if you are performing the full run over all rooms, it's better
  // to lock the Map completely.
  // else the other thread might delete the room you are examining at the moment!
  Map.lockForWrite();


  QVector<CRoom *> rooms = Map.getRooms();
  /* find the closest neighbours by coordinate */
  for (i = 0; i < Map.size(); i++) {
      p = rooms[i];

    /* z-axis: up and down exits */
    if (p->getZ() != r->getZ()) {

      if ((p->getX() != r->getX()) || (p->getY() != r->getY()))
        continue;

      /* up exit */
      if (p->getZ() > r->getZ()) {
        z = p->getZ() - r->getZ();
        if (z < distances[UP]) {
          /* update */
          distances[UP] = z;
          candidates[UP] = p;
        }
      }

      /* DOWN exit */
      if (r->getZ() > p->getZ()) {
        z = r->getZ() - p->getZ();
        if (z < distances[DOWN]) {
          /* update */
          distances[DOWN] = z;
          candidates[DOWN] = p;
        }
      }

    }
    /* done with z-axis */

    /* x-axis. */
    if ((p->getY() == r->getY()) && (p->getZ() == r->getZ())) {

      if (p->getX() == r->getX())
        continue;                       /* all coordinates are the same - skip */

      /* EAST exit */
      if (p->getX() > r->getX()) {
        z = p->getX() - r->getX();
        if (z < distances[EAST]) {
          /* update */
          distances[EAST] = z;
          candidates[EAST] = p;
        }
      }

      /* WEST exit */
      if (r->getX() > p->getX()) {
        z = r->getX() - p->getX();
        if (z < distances[WEST]) {
          /* update */
          distances[WEST] = z;
          candidates[WEST] = p;
        }
      }

    }
    /* y-axis.  */
    if ((p->getX() == r->getX()) && (p->getZ() == r->getZ())) {

      if (p->getY() == r->getY())
        continue;                       /* all coordinates are the same - skip */

      /* NORTH exit */
      if (p->getY() > r->getY()) {
        z = p->getY() - r->getY();
        if (z < distances[NORTH]) {
          /* update */
          distances[NORTH] = z;
          candidates[NORTH] = p;
        }
      }

      /* SOUTH exit */
      if (r->getY() > p->getY()) {
        z = r->getY() - p->getY();
        if (z < distances[SOUTH]) {
          /* update */
          distances[SOUTH] = z;
          candidates[SOUTH] = p;
        }
      }

    }


  }

  print_debug(DEBUG_ROOMS, "candidates gathered");

  /* ok, now we have candidates for linking - lets check directions and connections*/
  for (i=0; i <= 5; i++) {
    if (r->isExitUndefined(i) && candidates[i] != NULL)
      if (candidates[i]->isExitUndefined( reversenum(i) )  == true) {

        if (distances[ i ] <= 2) {
          print_debug(DEBUG_ROOMS, "we have a match for AngryLinker!");
          print_debug(DEBUG_ROOMS, "ID: %i to %i exit %s.", r->id, candidates[i]->id, exits[i] );

          /* ok, do the linking */
          candidates[ i ]->setExit( reversenum(i), r);
          r->setExit(i, candidates[ i ]);
          print_debug(DEBUG_ROOMS, "Linked.", r->id, candidates[i]->id, exits[i] );

          send_to_user("--[ (AngryLinker) Linked exit %s with %s [%i].\r\n",
                      exits[ i ], (const char*) candidates[i]->getName(), candidates[i]->id);

        }



      }
  }

  Map.unlock();
}


void CEngine::printStacks()
{
    char line[MAX_DATA_LEN];
    QByteArray s;

    send_to_user(" -----------------------------\n");

    sprintf(line,
	    "Conf: Mapping %s, AutoChecks [Desc %s, Exits %s, Terrain %s],\r\n"
            "      AutoRefresh settings %s (RName/Desc quotes %i/%i), \r\n"
            "      AngryLinker %s DualLinker %s\r\n",
            ON_OFF(mapping), ON_OFF(conf->getAutomerge()),
            ON_OFF(conf->getExitsCheck() ), ON_OFF(conf->getTerrainCheck() ),
            ON_OFF(conf->getAutorefresh() ), conf->getNameQuote(), conf->getDescQuote(),
            ON_OFF(conf->getAngrylinker() ),ON_OFF(conf->getDuallinker() )              );

    send_to_user(line);
    stacker.printStacks();
}

void CEngine::clear()
{
    eventPipe.clear();
    commandQueue.clear();

    print_debug(DEBUG_ANALYZER, "Engine INIT.\r\n");
    mapping =                0;
    mgoto             =      0;


    last_name.clear();
    last_desc.clear();
    last_exits.clear();
    last_terrain = 0;
    last_prompt.clear();
    last_prompt = "-->";
    last_movement = "";

    set_users_region(Map.getRegionByName("default"));
    set_last_region(Map.getRegionByName("default"));

    resetAddedRoomVar();
}

void CEngine::set_users_region(CRegion *reg)
{
    users_region = reg;
}

void CEngine::set_last_region(CRegion *reg)
{
    last_region = reg;
}

CRegion *CEngine::get_users_region()
{
    return users_region;
}

CRegion *CEngine::get_last_region()
{
    return last_region;
}


QVector<unsigned int> *CEngine::getPrespammedDirs()
{
    if ( commandQueue.isEmpty() || stacker.amount() != 1 )
		return NULL; // return an empty list

    return commandQueue.getPrespam( stacker.get(0) );
}
