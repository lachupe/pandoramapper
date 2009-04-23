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

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <QMutex>


#include "CRoomManager.h"
#include "CConfigurator.h"
#include "defines.h"
#include "exits.h"
#include "CDispatcher.h"
#include "CStacksManager.h"
#include "proxy.h"
#include "utils.h"
#include "CEngine.h"


void do_exits(const char *exits_line)
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
        if (r->isExitPresent(i) == true) {
            if (exits[i] == 0) {	/* oneway case */
                Map.oneway_room_id = r->exits[i]->id;
                r->removeExit(i);
            }
                
            if (exits[i] == E_CLOSEDDOOR) 
                r->setDoor(i, "exit");
                
                
            continue;
        }
        if (exits[i] == E_CLOSEDDOOR) {
            r->setDoor(i, "exit");
        }
            
        if (exits[i] == E_PORTAL) {
            send_to_user("--[AUDA: attention PORTAL was in room as you entered it. Fix existing exits if needed\r\n");
        }
            
        if ((exits[i] > 0) && (exits[i] != E_PORTAL))
            r->setExitUndefined(i);
        }

        stacker.put(engine->addedroom);
            
        return;
}

int compare_exits(CRoom *p, int exits[])
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
        localExit = p->exits[i];
        localDoor = p->getDoor(i);
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


void parse_exits(const char *p, int exits[])
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
    if (p == NULL) {
        printf("parse_exits: ERROR - NULL EVENT GIVEN!\r\n");
    }

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
