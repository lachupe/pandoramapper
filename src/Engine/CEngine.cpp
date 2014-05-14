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

#include <QMutex>
#include <QTime>
#include <QTimer>

#include "defines.h"
#include "CConfigurator.h"
#include "utils.h"

#include "Engine/CStacksManager.h"

#include "Proxy/CDispatcher.h"
#include "Proxy/proxy.h"


#include "Engine/CEngine.h"
#include "Engine/CEvent.h"

#include "Map/CRoomManager.h"
#include "Map/CTree.h"

#include "Proxy/userfunc.h"

#include "Engine/CStacksManager.h"

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
  n = map->findByName(last_name);
  if (n != NULL)
    for (j = 0; j < n->ids.size(); j++) {
      if (last_name == map->getName( n->ids[j] )) {
//        print_debug(DEBUG_ANALYZER, "Adding matches");
        stacker.put( n->ids[j] );
      }
    }

  stacker.swap();
}


CRoom* CEngine::getCurrentRoom()
{
    return stacker.first();
}

void CEngine::setCurrentRoom(CRoom *r)
{
    stacker.put(r);
    stacker.swap();
}

void CEngine::setCurrentRoom(RoomId id)
{
    stacker.put(id);
    stacker.swap();
}



void CEngine::initEmulationMode()
{
    setPrompt("-->");
    stacker.put(1);
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
    unsigned int i;
    CRoom *room;
    CRoom *candidate;

    nameMatch = 0;
    descMatch = 0;

    print_debug(DEBUG_ANALYZER, "in try_dir");
    ExitDirection dir = numbydir(event.dir[0]);
    if (dir == ED_UNKNOWN) {
        print_debug(DEBUG_ANALYZER, "Error in try_dir - faulty DIR given as input!\r\n");
        return;
    }

    CCommand cmd = commandQueue.peek();
    if (cmd.timer.elapsed() > conf->getPrespamTTL() ) {
        print_debug(DEBUG_ANALYZER, "The command queue has head entry with lifetime over limit. Resetting");
    	commandQueue.clear();
    	toggle_renderer_reaction();
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
            candidate = room->getExitRoom(dir);
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
        // dodgy part - iterate over enum! so iterate over 6 exits
        for (int d = 0; d < 6; d++) {
            ExitDirection dir = static_cast<ExitDirection>(d);
            if (room->isConnected(dir)) {
                candidate = room->getExitRoom(dir);
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

    if (map->isBlocked()) {
    	// well, not much we can do - ignore the message
    	printf("The Map is blocked. Delaying the execution of the slotRunEngine.\r\n");
		print_debug(DEBUG_GENERAL, "The Map is blocked. Delaying the execution of the slotRunEngine.");
		QTimer::singleShot( 100, this, SLOT(slotRunEngine()) );
		return;
    }

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
    	// notify renderer to remove all the unnecessary line drawn
    	commandQueue.dequeue();
    	toggle_renderer_reaction();
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



CEngine::CEngine(CRoomManager *_map) : QObject(), map(_map), stacker(this)
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
                set_users_region( last_region );
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
void CEngine::mapCurrentRoom(CRoom *room, ExitDirection dir)
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

    int x = room->getX();
    int y = room->getY();
    int z = room->getZ();

    if (dir == ED_NORTH)
        y += 2;
    if (dir == ED_SOUTH)
        y -= 2;
    if (dir == ED_EAST)
        x+= 2;
    if (dir == ED_WEST)
        x -= 2;
    if (dir == ED_UP)
        z += 2;
    if (dir == ED_DOWN)
        z -= 2;

    addedroom = map->createRoom(event.name, event.desc, x, y, z);

    addedroom->setRegion( users_region );

    room->setExitLeadsTo(dir, addedroom);
    if (conf->getDuallinker() == true)
        addedroom->setExitLeadsTo(reversenum(dir), room);
    else
        map->oneway_room_id = room->getId();

    setExits(event.exits);
    do_exits((const char *) event.exits);

    //stacker.put(addedroom);

    CRoom *checkedDups = map->isDuplicate(addedroom);
    if (checkedDups == NULL) {
        resetAddedRoomVar();
    } else {
        stacker.put(checkedDups);
        if (checkedDups == addedroom) {
            // was not a duplicate, so see what angryLinker might be able to achieve
            angryLinker(addedroom);
        } else {
            resetAddedRoomVar();
            send_to_user("--[Pandora: Twin rooms merged!\n");
            proxy->send_line_to_user( (const char *) last_prompt );
            print_debug(DEBUG_ANALYZER, "Twins merged");
        }
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
  //map->lockForWrite();


  QVector<CRoom *> rooms = map->getRooms();
  /* find the closest neighbours by coordinate */
  for (i = 0; i < map->size(); i++) {
      p = rooms[i];

    /* z-axis: up and down exits */
    if (p->getZ() != r->getZ()) {

      if ((p->getX() != r->getX()) || (p->getY() != r->getY()))
        continue;

      /* up exit */
      if (p->getZ() > r->getZ()) {
        z = p->getZ() - r->getZ();
        if (z < distances[ED_UP]) {
          /* update */
          distances[ED_UP] = z;
          candidates[ED_UP] = p;
        }
      }

      /* DOWN exit */
      if (r->getZ() > p->getZ()) {
        z = r->getZ() - p->getZ();
        if (z < distances[ED_DOWN]) {
          /* update */
          distances[ED_DOWN] = z;
          candidates[ED_DOWN] = p;
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
        if (z < distances[ED_EAST]) {
          /* update */
          distances[ED_EAST] = z;
          candidates[ED_EAST] = p;
        }
      }

      /* WEST exit */
      if (r->getX() > p->getX()) {
        z = r->getX() - p->getX();
        if (z < distances[ED_WEST]) {
          /* update */
          distances[ED_WEST] = z;
          candidates[ED_WEST] = p;
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
        if (z < distances[ED_NORTH]) {
          /* update */
          distances[ED_NORTH] = z;
          candidates[ED_NORTH] = p;
        }
      }

      /* SOUTH exit */
      if (r->getY() > p->getY()) {
        z = r->getY() - p->getY();
        if (z < distances[ED_SOUTH]) {
          /* update */
          distances[ED_SOUTH] = z;
          candidates[ED_SOUTH] = p;
        }
      }

    }


  }

  print_debug(DEBUG_ROOMS, "candidates gathered");

  /* ok, now we have candidates for linking - lets check directions and connections*/
  for (i=0; i <= 5; i++) {
    ExitDirection iDir = static_cast<ExitDirection> (i);

    if (r->isExitUndefined(iDir) && candidates[i] != NULL)
      if (candidates[i]->isExitUndefined( reversenum(iDir) )  == true) {

        if (distances[ i ] <= 2) {
          print_debug(DEBUG_ROOMS, "we have a match for AngryLinker!");
          print_debug(DEBUG_ROOMS, "ID: %i to %i exit %s.", r->getId(), candidates[i]->getId(), exits[i] );

          /* ok, do the linking */
          candidates[ i ]->setExitLeadsTo( reversenum(iDir), r);
          r->setExitLeadsTo(iDir, candidates[ i ]);
          print_debug(DEBUG_ROOMS, "Linked.", r->getId(), candidates[i]->getId(), exits[i] );

          send_to_user("--[ (AngryLinker) Linked exit %s with %s [%i].\r\n",
                      exits[ i ], (const char*) candidates[i]->getName(), candidates[i]->getId());

        }



      }
  }

  //map->unlock();
}


void CEngine::printStacks()
{
    char line[2048];
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
    stacker.clear();

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

    set_users_region(map->getRegionByName("default"));
    set_last_region(map->getRegionByName("default"));

    resetAddedRoomVar();
}

void CEngine::set_users_region(CRegion *reg)
{
	CRegion *prev_reg = users_region;

	if (users_region == reg)
		return;

	users_region = reg;

    map->rebuildRegion( prev_reg );
    map->rebuildRegion( reg );
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

// this method ensures that we are in sync!
QVector<RoomId> *CEngine::getPrespammedDirs()
{
    if ( commandQueue.isEmpty() || stacker.amount() != 1 )
		return NULL; // return an empty list

    return commandQueue.getPrespam( stacker.get(0) );
}

void CEngine::do_exits(const char *exits_line)
{
    int exits[6];
    unsigned int i;
    CRoom *r;

    parse_exits(exits_line, exits);

    r = engine->addedroom;
    if (r == NULL)
        return;

    print_debug(DEBUG_ANALYZER /*& DEBUG_TOUSER*/,
                    "Exits: Adding exits information to the new room.");

    for (i = 0; i <= 5; i++) {
        ExitDirection iDir = static_cast<ExitDirection>(i);
        if (r->isExitPresent(iDir) == true) {
            if (exits[i] == 0) {	/* oneway case */
                map->oneway_room_id = r->getExitLeadsTo(iDir);
                r->removeExit(iDir);
            }

            if (exits[i] == E_CLOSEDDOOR)
                r->setDoor(iDir, "exit");


            continue;
        }
        if (exits[i] == E_CLOSEDDOOR) {
            r->setDoor(iDir, "exit");
        }

        if (exits[i] == E_PORTAL) {
            send_to_user("--[ Attention PORTAL was in room as you entered it. Fix existing exits if needed\r\n");
        }

        if ((exits[i] > 0) && (exits[i] != E_PORTAL))
            r->setExitUndefined(iDir);
        }

        stacker.put(engine->addedroom);

        return;
}

int CEngine::compare_exits(CRoom *p, int exits[])
{
    int counter;
    int i;
    CRoom *localExit;
    QByteArray localDoor;

    counter = 0;
    /* for each door there are following situations : */
    /* 0) closed door and secret exit in base -> skip this one */
    /* 1) open exit and normal exit in base -> skip! */
    /* a) normal exit there and exit in base (secret or not doesnt matter) */
    /* b) no exit and secret exit in base -> fine */

    print_debug(DEBUG_ANALYZER, "compare_exits called.");
    for (i = 0; i <= 5; i++) {
        ExitDirection iDir = static_cast<ExitDirection>(i);
        localExit = p->getExitRoom(iDir);
        localDoor = p->getDoor(iDir);
        if ((exits[i] == 3) && (localExit != NULL) && (localDoor != ""))
            if (localDoor != "exit")
                break;		/* situation 0) */

            if (exits[i] == 4) {
                counter++;		/* portal - we ignore this exit and count as possible */
	    	continue;
            }

            if ((exits[i] == 2) && (localExit != NULL) && (localDoor != ""))
	    	break;		/* situation 1) */


            if ((exits[i] > 0) && (localExit != NULL)) {
	    	counter++;
	    	continue;
            }

            if ((exits[i] == 0) && (localExit != NULL) && (localDoor != ""))
	    	if (localDoor != "exit") {
                    counter++;
                    continue;
	    	}

            if ((exits[i] == 0) && (localExit != NULL)) {
                counter++;
                continue;
            }
    }

    if (counter == 6) {		/* only if all 6 exits are the same we can say it probably the same room */
        print_debug(DEBUG_ANALYZER, "Compare exits result - positive.");
        return 1;
    }
    print_debug(DEBUG_ANALYZER, "Compare exits result - negative.");
    return 0;
}


void CEngine::parse_exits(const char *p, int exits[])
{
    const char *exit_patterns[] = {
      "no",
      "ea",
      "so",
      "we",
      "up",
      "do"
    };


    int len;
    int i;
    int dir;

    /* NOrth, EAst, SOuth, WEst, UP, DOwn */
    /* output format : */
    /* 0 - no exit there, 1 - normal exit, 2 - open door, 3 - closed door, 4 - portal */

    print_debug(DEBUG_ANALYZER, "Parsing exits line");

    exits[0] = E_NOEXIT;
    exits[1] = E_NOEXIT;
    exits[2] = E_NOEXIT;
    exits[3] = E_NOEXIT;
    exits[4] = E_NOEXIT;
    exits[5] = E_NOEXIT;

    print_debug(DEBUG_ANALYZER, "PARSING EXITS: ...%s...\r\n", p);
    len = strlen(p);
    for (i = 0; i <= len; i++)
      for (dir = 0; dir <= 5; dir++)
        if ((p[i] == exit_patterns[dir][0]) &&
            (p[i + 1] == exit_patterns[dir][1]))
        {
            if (i == 0) {
                exits[dir] = E_NORMAL;
                continue;
            }
            if (p[i - 1] == '(') {
                exits[dir] = E_OPENDOOR;
                continue;
            }
            if (p[i - 1] == '[') {
                exits[dir] = E_CLOSEDDOOR;
                continue;
            }
            if (p[i - 1] == '{') {
                exits[dir] = E_PORTAL;
                continue;
            }
            exits[dir] = E_NORMAL;
            continue;
          }


}
