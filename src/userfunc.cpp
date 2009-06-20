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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <QMutex>

#include "CRoomManager.h"
#include "defines.h"
#include "CConfigurator.h"
#include "CDispatcher.h"
#include "CStacksManager.h"
#include "proxy.h"
#include "utils.h"
#include "userfunc.h"
#include "xml2.h"
#include "CEngine.h"
#include "CTree.h"
#include "mainwindow.h"
#include "exits.h"

class Userland *userland_parser;

/* ================= ENCHANCED USER FUNCTIONS VERSIONS =============== */
#define USERCMD_FLAG_SYNC       (1 << 0)       /* sync is required */
#define USERCMD_FLAG_REDRAW     (1 << 1)       /* redraw after executing */
#define USERCMD_FLAG_INSTANT    (1 << 2)       /* commands flagged with instant flag */
                                               /* get executed in dispatcher thread */
                                               /* all others are executed in interface thread */

#define USER_CONF_MAP         1
#define USER_CONF_BRIEF       2
#define USER_CONF_EXITS       3
#define USER_CONF_DESCS       4
#define USER_CONF_TERRAIN     5

#define USER_MOVE_LOOK        6
#define USER_MOVE_EXAMINE     7

#define USER_CONF_AUTOMERGE   8
#define USER_CONF_ANGRYLINKER 9
#define USER_CONF_DUALLINKER  10


#define userfunc_print_debug print_debug(DEBUG_USERFUNC, "called function cmd %i, subcmd %i", cmd, subcmd);

#define GET_INT_ARGUMENT(arg, value) \
  if (!is_integer(arg)) {               \
    send_to_user("--[ argument %s is not an integer as its supposed to be.\r\n", arg); \
    send_prompt(); \
    return USER_PARSE_SKIP;             \
  }                                             \
  value = atoi(arg);


#define MISSING_ARGUMENTS \
  { \
    send_to_user("--[Pandora: Missing arguments.\n");      \
    send_prompt(); \
    return USER_PARSE_SKIP;                             \
  }


#define PARSE_DIR_ARGUMENT(dir, arg) \
    dir = parse_dir(arg);               \
    if (dir == -1) {                            \
      send_to_user("--[ %s is not a dirrection.\r\n", arg);     \
      send_prompt(); \
      return USER_PARSE_SKIP;             \
    }

#define CHECK_SYNC \
        if (stacker.amount() != 1) {      \
          send_to_user("--[Pandora: Current position is undefined(out of sync).\n");       \
          send_prompt(); \
          return USER_PARSE_SKIP;       \
        }


void display_debug_settings();

const struct user_command_type user_commands[] = {
  {"maddroom",     usercmd_maddroom,          0,                        USERCMD_FLAG_REDRAW,
    "Add current room to database [READ HELP]",
   "    Usage: maddroom \r\n"
   "    Examples: maddroom \r\n\r\n"
   "    Use this command for special cases (like rivendell entrances) when its impossible\r\n"
   "add/map the connection and following room in normal way(with mmap on and movement).\r\n"
   "This command takes the last seen roomname, description (optional), exits and terrain data\r\n"
   "of the last visited room and adds a room with coordinates 100, 100, 100.\r\n"},
  {"mhelp",     usercmd_mhelp,          0,                              0,
    "Brief help files or commands overview.",
   "    Usage: mhelp [command name]\r\n"
   "    Examples: mhelp / mhelp mmap\r\n\r\n"
   "    This command gives a brief overview of all available commands when no arguments\r\n"
   "are given. Or you can see more detailed data for any given command.\r\n"},
  {"mmap",     usercmd_config,         USER_CONF_MAP,   USERCMD_FLAG_SYNC,
    "Turn mapping mode on/off, if possible.",
   "    Usage: mmap [boolean]\r\n"
   "    Examples: mmap / mmap on / mmap false\r\n\r\n"
   "    This command turns mapping mode on/off. To turn the mapping mode on the analyzer needs\r\n"
   "to be 100% synced.\r\n"
   "    This will will automatically send commands brief off, spam on and prompt all to the game.\r\n"},
  {"mbrief",   usercmd_config,         USER_CONF_BRIEF,  0,
    "Turn mappers built in brief mode on/off.",
   "    Usage: mbrief [boolean]\r\n"
   "    Examples: mbrief / mbrief on / mbrief true / mbrief 1 / mbrief yes\r\n\r\n"
   "    This command turns internal mappers brief mode on/off. When its on\r\n"
   "mapper will not show room descriptions to the user if any are recognised.\r\n"},
  {"mcheckexits",   usercmd_config,    USER_CONF_EXITS,   0,
    "Turn exits analyzer on/off.",
   "    Usage: mcheckexits [boolean]\r\n"
   "    Examples: mcheckexits / mcheckexits on / mcheckexits true / mcheckexits 1 /mcheckexits yes\r\n\r\n"
   "    This command turns on the exits analyzing. Exits analyzer requires a very full\r\n"
   "map.\r\n" },
  {"mautomerge",   usercmd_config,   USER_CONF_AUTOMERGE,     0,
    "Turn descriptions analyzer in mapping mode on/off.",
   "    Usage: mautomerge [boolean]\r\n"
   "    Examples: mautomerge / mautomerge on / mautomerge true / mautomerge 1 / mautomerge yes\r\n\r\n"
   "    This command turns the descriptions analyzer and automatic merging\r\n"
   "on/off. Turn it off in badly created zones and link rooms manually.\r\n"},
  {"mangrylinker",   usercmd_config,   USER_CONF_ANGRYLINKER,     0,
    "Turn the AngryLinker on/off.",
   "    Usage: mangrylinker [boolean]\r\n"
   "    Examples: mangrylinker / mangrylinker on / mangrylinker true\r\n\r\n"
   "    AngryLinker attempts to link the surrounding rooms based on its coordinates\r\n"
   "Be VERY careful using this - you can easily get mistakenly linked rooms.\r\n"
   "Though this is a very helpful option for 10x10 zones.\r\n"},
  {"mduallinker",   usercmd_config,   USER_CONF_DUALLINKER,     0,
    "Turn the DualLinker on/off.",
   "    Usage: mduallinker [boolean]\r\n"
   "    Examples: mduallinker / mduallinker on / mduallinker true\r\n\r\n"
   "    DualLinker simply makes every mapped connection dual. IF you turn it off every\r\n"
   "new mapped connection will be a one-way connection\r\n"
   "It's very helpful to have it ON for all normal exits. Turn it off only when mapping\r\n"
   "randoms and generally strange areas\r\n"},
  {"mcheckterrain",   usercmd_config,   USER_CONF_TERRAIN,     0,
    "Turn terrain analyzer on/off.",
   "    Usage: mcheckterrain [boolean]\r\n"
   "    Examples: mcheckterrain / mcheckterrain on / mcheckterrain no / mcheckterrain 1\r\n\r\n"
   "    This command triggers the terrain analyzer (apply_prompt). \r\n"},
  {"msave",         usercmd_msave,         0,           0,
    "Save/Save as current database.",
   "    Usage: msave [filename]\r\n"
   "    Examples: msave / msave warpmap.xml\r\n\r\n"
   "    This command saves the database stored in memory in an xml file. Without arguments\r\n"
   "it saves to the default file.\r\n"},
  {"mload",         usercmd_mload,         0,           USERCMD_FLAG_REDRAW,
    "Load file/Reload the database from disk.",
   "    Usage: mload [filename]\r\n"
   "    Examples: mload / mload mume.xml\r\n\r\n"
   "    Without arguments this command reloads the currently opened database.\r\n"
   "As argument you can specify some other database-file to load.\r\n"},
  {"mreset",         usercmd_mreset,         0,           USERCMD_FLAG_REDRAW,
    "Reset mappers state stacks.",
   "    Usage: mreset\r\n"
   "    Examples: mreset\r\n\r\n"
   "    This command resets all stacks - current possible positions, cause and result stacks.\r\n"
   "This is very useful when something went wrong and some value stayed in movements stack.\r\n"
   "For example - you forgot to add some movement failure pattern to the config file. When this \r\n"
   "movement failure case will show up you will have to reset the stacks and resync manualy.\r\n"},
  {"mstat",         usercmd_mstat,         0,           0,
    "Display settings and mappers state stacks.",
   "    Usage: mstat\r\n"
   "    Examples: mstat\r\n\r\n"
   "    This command displays settings, stacks and possible current position room id's\r\n"},
  {"minfo",         usercmd_minfo,         0,           0,
    "Display current rooms data (or by given id).",
   "    Usage: minfo [id]\r\n"
   "    Examples: minfo / minfo 120\r\n\r\n"
   "    This command displays everything know about current room. Roomname, id, flags,\r\n"
   "room description, exits, connections and last update date.\r\n"},
  {"north",         usercmd_move,         NORTH,          USERCMD_FLAG_INSTANT,   NULL, NULL},
  {"east",          usercmd_move,         EAST,           USERCMD_FLAG_INSTANT,   NULL, NULL},
  {"south",         usercmd_move,         SOUTH,          USERCMD_FLAG_INSTANT,   NULL, NULL},
  {"west",          usercmd_move,         WEST,           USERCMD_FLAG_INSTANT,   NULL, NULL},
  {"up",            usercmd_move,         UP,             USERCMD_FLAG_INSTANT,   NULL, NULL},
  {"down",          usercmd_move,         DOWN,           USERCMD_FLAG_INSTANT,   NULL, NULL},
  {"look",          usercmd_move,         USER_MOVE_LOOK, USERCMD_FLAG_INSTANT,   NULL, NULL},
  {"examine",       usercmd_move,         USER_MOVE_EXAMINE, USERCMD_FLAG_INSTANT,   NULL, NULL},
  {"mmerge",        usercmd_mmerge,       0,    USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,
    "Merge twin rooms - manual launch.",
   "    Usage: mmerge [id] [force]\r\n"
   "    Examples: mmerge / mmerge 120 / mmerge 120 force\r\n\r\n"
   "    Without arguments this command will try to merge the last added room with either\r\n"
   "found twin room or given (by id) room. Force argument disables the roomname and desc checks.\r\n"},
  {"mdecx",             usercmd_mdec,        USER_DEC_X,   USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,
    "Decrease the X coordinate.",
   "    Usage: mdecx [integer]\r\n"
   "    Examples: mdecx / mdecx 2\r\n\r\n"
   "mdecx substracts 1 from the X coordinate. If you give an integer argument X, X will be substracted.\r\n"},
  {"mincx",             usercmd_mdec,        USER_INC_X,    USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,
    "Increase the X coordinate.",
   "    Usage: mincx [integer]\r\n"
   "    Examples: mincx / mincx 2\r\n\r\n"
   "mdecx adds 1 to the X coordinate. If you give an integer argument X, X will be added.\r\n"},
  {"mdecy",             usercmd_mdec,        USER_DEC_Y,     USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,
    "Decrease the Y coordinate.",
   "    See mdecx.\r\n"},
  {"mincy",             usercmd_mdec,        USER_INC_Y,     USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,
    "Increase the Y coordinate.",
   "    See mdecx.\r\n"},
  {"mdecz",             usercmd_mdec,        USER_DEC_Z,     USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,
    "Decrease the Z coordinate.",
   "    See mdecx.\r\n"},
  {"mincz",             usercmd_mdec,        USER_INC_Z,     USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,
    "Increase the Z coordinate.",
   "    See mdecx.\r\n"},
  {"mcoord",            usercmd_mcoord,       0,           USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,
    "Set the coordinates for current room.",
   "    Usage: mcoord [X] [Y] [Z]\r\n"
   "    Examples: mcoord -2 4 5 / mcoord -2 / mcoord -2 4 / mcoord -2 4 5 \r\n\r\n"
   "Set the coordinates of the current room to the given values.\r\n"},

  {"mdoor",             usercmd_mdoor,   USER_DOOR_NORMAL,  USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,
    "Add a door in direction",
    "   Usage: mdoor <doorname|remove> <dirrection>\r\n"
    "   Examples: mdoor stonedoor east\r\n"
    "             mdoor stonedoor w\r\n"
    "             mdoor remove w\r\n\r\n"
    "If there is no connection in given direction an undefined connection will\r\n"
    "be created. Any existing doorname will be removed - so take care.\r\n"
    "   Note that remove has to be written fully and not in capitals.\r\n"},

  {"mnote",             usercmd_mnote,   0,  USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,
    "Add a note to the room you are standing in",
    "   Usage: mnote <note>\r\n"
    "   Examples: mnote There is a huge nasty mob living in there! \r\n"
    "Adds a note to the room.\r\n"},

  {"mnotecolor",             usercmd_mnotecolor,   0,  USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,
    "Change the note color of the room you are standing in.",
    "   Usage: mnotecolor <color>\r\n"
    "   Examples: mnotecolor red\r\n"
    "             mnotecolor #ff0000\r\n"
    "             mnotecolor slategrey\r\n"
    "Changes the note color for the room.\r\n"},

  {"mexit",             usercmd_mdoor,   USER_DOOR_EXIT,     USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,
    "Add a nonsecret door",
    "   Usage: mdoor <doorname> <dirrection>\r\n"
    "    Examples: mdoor stonedoor east / mdoor stonedoor w\r\n\r\n"
    "If there is no connection in given direction an undefined connection will\r\n"
    "Any existing doorname will be removed - so take care. Door name \"exit\" is\r\n"
    "reserved word and basicly means - nonsecret exit. Use \"mexit\" to mark secret doors.\r\n"},
  {"mmark",             usercmd_mmark,          0,      USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,
    "Mark/Flag some direction.",
    "    Usage: mmark <dir> <flag>\r\n"
    "    Examples: mmark west undefined / mmark w death\r\n\r\n"
    "    Possible flags: undefined, death\r\n"},
  {"mlink",             usercmd_mlink,          0,      USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,
    "Link current room with some other.",
    "    Usage: mlink <dirrection> <id> [backdir] [force|oneway]\r\n"
    "    Examples: \r\n"
    "   mlink west 120          link exit west in current room with exit east in room 120.\r\n"
    "                    Wont work if exit is already taken(busy).\r\n"
    "   mlink e 120 o           link exit west as oneway to room 120.\r\n"
    "   mlink w 120 e force     link west to 120 and force binding of all connections. If exits are \r\n"
    "                           already takes(busy)they will be overwritten. Doornames will stay.\r\n"
    "   mlink w 120 o force     link west to 120 and force connections.\r\n\r\n"
    "    Link two rooms.\r\n"},
  {"mdetach",             usercmd_mdetach,          0,      USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,
    "Detach connections in 2 rooms.",
    "    Usage: mdetach <dirrection> [oneway|delete]\r\n"
    "    Examples: \r\n"
    "   mdetach west            detach the link in both rooms and mark exits as undefined.\r\n"
    "   mdetach w one           detach link only in this room.\r\n"
    "   mdetach west delete     detach in both rooms and remove the exits.\r\n"
    "   mdetach w one d         detach link west and delete the exit only in this room.\r\n"
    "                           Leave oneway in second room.\r\n\r\n"
    "    Disconnect two rooms.\r\n"},
  {"mgoto",               usercmd_mgoto,        0,       USERCMD_FLAG_REDRAW,
    "Set some room as current (using id or direction).",
    "    Usage: mgoto <id|direction>\r\n"
    "    Examples: mgoto 120 / mgoto west /mgoto w\r\n\r\n"
    "   Set some room as current mappers position. To use direction as argument\r\n"
    "you need to be synced.\r\n"},
  {"maction",               usercmd_maction,        0,       USERCMD_FLAG_INSTANT,
    "Perform some action with doors in current room.",
    "    Usage: maction [local] <dir|all> <action>\r\n"
    "    Examples: \r\n"
    "     maction east open             open exit(secret) east. will send command\r\n"
    "                                   open <doorname_east> e\r\n"
    "     maction e cast 'block'        will send cast 'block' <door> e to mud\r\n"
    "     maction local all --[ Door:   will show all secret doors with diven prompt\r\n"
    "     maction local e --[EAST:      will send --[EAST: door e to your client only\r\n\r\n"
    "   Performs some actions with doors.\r\n"},

  {"mdebug",               usercmd_mdebug,        0,       0,
    "Configure debug information/messages.",
    "    Usage: mdebug [debug-option] [boolean]\r\n"
    "    Examples: \r\n"
    "     mdebug                        display current config and available options\r\n"
    "     mdebug analyzer on            allow analyzers debug messages\r\n\r\n"
    "   Configure your debug messages.\r\n"},
  {"mdelete",               usercmd_mdelete,        0,       USERCMD_FLAG_REDRAW,
    "Delete current room.",
    "    Usage: mdelete [remove]\r\n"
    "   Deletes current room. Option remove forces exits and doors removal in other rooms.\r\n"},
  {"mrefresh",              usercmd_mrefresh,        0,      USERCMD_FLAG_SYNC,
    "Refresh roomdesc.",
    "    Usage: mrefresh \r\n\r\n"
    "    Uses last seen roomdesc and all other information (e.g terrain flags).\r\n"
    "Use together with mgoto.\r\n"},

  {"mregion",              usercmd_mregion,        0,      USERCMD_FLAG_REDRAW,
    "Region's system subcommands entry point",
    "    Usage: mregion \r\n\r\n"
    "    blabla.\r\n"
    "blabla2.\r\n"},



  {NULL, NULL, 0, 0, NULL, NULL}
};

void Userland::add_command(int id, char *arg)
{
    struct queued_command_type t;

    print_debug(DEBUG_USERFUNC, "in addCommand");


    t.id = id;
    strcpy(t.arg, arg);

    queue_mutex.lock();
    commands_queue.push_back(t);
    queue_mutex.unlock();
    notify_analyzer();

    print_debug(DEBUG_USERFUNC, "leaving addCommand");
}


void Userland::parse_command()
{
  struct queued_command_type t;

  print_debug(DEBUG_USERFUNC, "in parseCommand");


  queue_mutex.lock();
  t = commands_queue.front();
  commands_queue.pop_front();
  queue_mutex.unlock();

  ((*user_commands[t.id].command_pointer) (t.id, user_commands[t.id].subcmd, t.arg, t.arg));

  print_debug(DEBUG_USERFUNC, "leaving parseCommand");
}

int Userland::parse_user_input_line(const char *line)
{
  char *p;
  char arg[MAX_STR_LEN];
  int i;
  int result;
//  int parse_result;

  print_debug(DEBUG_USERFUNC, "in parse_user_input_line");


  p = skip_spaces(line);


  if (!*p) {
    return USER_PARSE_NONE;
  }


  p = one_argument(p, arg, 0);
//  printf("One argument : line ..%s..,  arg ...%s...\r\n", p, arg);

  for (i=0; user_commands[i].name != NULL; i++)
    if (strcmp(user_commands[i].name, arg) == 0) {
      /* call the appropriate command handler */

      if (IS_SET(user_commands[i].flags, USERCMD_FLAG_SYNC))
        CHECK_SYNC;

      result = USER_PARSE_SKIP;
      if (IS_SET(user_commands[i].flags, USERCMD_FLAG_INSTANT)) {
        result = ((*user_commands[i].command_pointer) (i, user_commands[i].subcmd, p, (char *) line));
      }
      else {
        userland_parser->add_command(i, p);
      }


      if (IS_SET(user_commands[i].flags, USERCMD_FLAG_REDRAW))
        toggle_renderer_reaction();

//      if (renderer_window)
//        renderer_window->update_status_bar();

      return result;
    }

  if (proxy->isMudEmulation()) {
    send_to_user("Arglebargle...No such command\r\n");
    send_prompt();

    print_debug(DEBUG_USERFUNC, "leaving [emulation] parse_user_input_line");
    return USER_PARSE_SKIP;
  }


  print_debug(DEBUG_USERFUNC, "leaving parse_user_input_line");

  return USER_PARSE_NONE;
}


void display_debug_settings()
{
  int i;

  send_to_user("--[ Debug settings:\r\n");
  for (i = 0; debug_data[i].name; i++)
    send_to_user("   %-14s %-50s [%-3s]\r\n", debug_data[i].name,
        debug_data[i].desc, ON_OFF(debug_data[i].state));

  send_to_user("\r\n");
}



USERCMD(usercmd_mdebug)
{
  char *p;
  char arg[MAX_STR_LEN];
  int i, desired;

  userfunc_print_debug;


  p = skip_spaces(line);
  if (!*p) {
    display_debug_settings();
    send_prompt();
    return USER_PARSE_SKIP;
  }

  p = one_argument(p, arg, 0);
  for (i = 0; debug_data[i].name; i++)
    if (is_abbrev(arg, debug_data[i].name)) {
      /* found passing debug setting */

      p = skip_spaces(p);
      if (!*p) {
        debug_data[i].state = 1 - debug_data[i].state;
        send_to_user("--[ Setting %s to %s.\r\n", debug_data[i].desc,
                                                  ON_OFF(debug_data[i].state) );

        send_prompt();
        return USER_PARSE_SKIP;
      }
      p = one_argument(p, arg, 0);

      desired = get_input_boolean(arg);
      debug_data[i].state = desired;
      send_to_user("--[  Setting %s to %s.\r\n", debug_data[i].desc,
                                                ON_OFF(debug_data[i].state) );
      send_prompt();
      return USER_PARSE_SKIP;
    }

  display_debug_settings();

  send_prompt();
  return USER_PARSE_SKIP;
}


USERCMD(usercmd_maddroom)
{
  CRoom *r;

  if (proxy->isMudEmulation()) {
    send_to_user("Disabled in MUD emulation.\r\n");
    send_prompt();
    return USER_PARSE_SKIP;
  }

  if (line) {} /* just to avoid warning about unused variable - its annoying */

  userfunc_print_debug;

  if (engine->getRoomName().isEmpty()) {
    send_to_user("--[ Missing room name.\r\n");

    send_prompt();
    return USER_PARSE_SKIP;
  }

  if (engine->getDesc().isEmpty() ) {
    send_to_user("--[ Missing description!\r\n");
  }

  if (engine->getExits().isEmpty() ) {
    send_to_user("--[ Missing exits.\r\n");

    send_prompt();
    return USER_PARSE_SKIP;
  }

  send_to_user("--[ (Forced) adding new room!\n");
  print_debug(DEBUG_ANALYZER, "adding new room!");



  Map.fixFreeRooms();	/* making this call just for more safety - might remove */

  r = new CRoom;


  r->id = Map.next_free;
  r->setName( engine->getRoomName() );
  r->setDesc( engine->getDesc() );
  r->setTerrain( engine->getTerrain() );
  r->setRegion( engine->get_users_region() );


  engine->addedroom = r;
  do_exits(engine->getExits());

  r->setX(100);
  r->setY(100);
  r->setZ(100);

  Map.addRoom(r);

  stacker.put(r);
  stacker.swap();

  send_prompt();
  return USER_PARSE_SKIP;
}


USERCMD(usercmd_maction)
{
  char *p;
  char arg[MAX_STR_LEN];
  CRoom *r;
  int dir;
  int local;
  unsigned int i;
  char exit;

  const char *short_exits[] = {
      "n",
      "e",
      "s",
      "w",
      "u",
      "d"
  };


  userfunc_print_debug;

  p = skip_spaces(line);
  if (!*p) MISSING_ARGUMENTS
  p = one_argument(p, arg, 0);

  local = 0;
  if (is_abbrev(arg, "local")) {
    local = 1;
    p = skip_spaces(p);
    if (!*p) MISSING_ARGUMENTS
    p = one_argument(p, arg, 0);
  }

//  PARSE_DIR_ARGUMENT(dir, arg);
  dir = parse_dir(arg);
  if (dir == -1) {
    if (is_abbrev(arg, "all")) {
      dir = -1;
    } else {
      send_to_user("--[ %s is not a direction.\r\n", arg);
      send_prompt();
      return USER_PARSE_SKIP;
    }
  }

  p = skip_spaces(p);
  /* some safety checks */
  strncpy(arg, p, MAX_STR_LEN/2);

  original[0] = 0;      /* nullify the incoming line and get ready to put generated commands */

  if (stacker.amount() == 0) {
    if (dir == -1) {
      send_to_user("--[ undefined position. can not operate with secrets.\r\n");
      send_prompt();
      return USER_PARSE_SKIP;
    }

    send_to_user("%s exit %s\r\n", arg, short_exits[dir]);

    if (local) {
      send_prompt();
      return USER_PARSE_SKIP;
    } else  {
//      send_to_mud("%s exit %s\n", arg, short_exits[dir]);
      sprintf(original+strlen(original), "%s exit %s", arg, short_exits[dir]);
      return USER_PARSE_DONE;
    }
  }


  exit = 0;             /* this flag is for 'action exit' when there are several rooms */

  /* get the door names */
  for (i = 0; i < stacker.amount(); i++) {
    r = stacker.get(i);

    if (dir == -1) {
      int z;

      for (z = 0; z <= 5; z++)
        if (r->isDoorSecret(z) == true) {
          if (!local) {
            //send_to_mud("%s %s %s\n", arg, r->doors[z] , short_exits[z]);
            if (strlen(original) != 0) strcat(original, "\n");
            sprintf(original+strlen(original), "%s %s %s", arg, (const char *) r->getDoor(z) , short_exits[z]);
          }
          send_to_user("%s %s %s\r\n", arg, (const char *) r->getDoor(z) , short_exits[z]);
        }

    } else {

      if (r->isDoorSecret(dir) == false) {
        exit = 1;         /* set the flag that 'exit' exit should be opened */
      } else {
        if (!local) {
          //send_to_mud("%s %s %s\n", arg, r->doors[dir] , short_exits[dir]);
          if (strlen(original) != 0) strcat(original, "\n");
          sprintf(original+strlen(original), "%s %s %s", arg, (const char *) r->getDoor(dir) , short_exits[dir]);
        }
        send_to_user("%s %s %s\r\n", arg, (const char *) r->getDoor(dir) , short_exits[dir]);
      }

    }

  }


  /* open the exit only once */
  if (exit && dir != -1) {
    if (!local) {
      //send_to_mud("%s %s %s\n", arg, "exit", short_exits[dir]);
      if (strlen(original) != 0) strcat(original, "\n");
      sprintf(original+strlen(original), "%s %s %s", arg, "exit", short_exits[dir]);
    }
    send_to_user("%s %s %s\r\n", arg, "exit", short_exits[dir]);
  }

  if (local) {
    send_prompt();
    return USER_PARSE_SKIP;
  } else {
    return USER_PARSE_DONE;
  }
}


USERCMD(usercmd_mdelete)
{
    char *p;
    char arg[MAX_STR_LEN];
    CRoom *r;
    int remove;
    QList<int> ids;


    userfunc_print_debug;

    remove = 0;
    p = skip_spaces(line);
    if (*p) {
        p = one_argument(p, arg, 0);
        if (is_abbrev(arg, "remove"))
        remove = 1;

    }

    if (Map.selections.isEmpty() == false) {
        //r = Map.getRoom( Map.selections.getFirst() );
        ids = Map.selections.getList();

    } else {
        CHECK_SYNC;
        r = stacker.first();
        ids.append(r->id);
    }


    for (int i = 0; i < ids.size(); ++i) {
        r = Map.getRoom( ids.at(i) );
        if ( r == NULL )
            continue;
        if (r->id == 1) {
            send_to_user("--[ Sorry, you can not delete the base (id == 1) room!\r\n");
            send_prompt();
            return USER_PARSE_SKIP;
        }

        if (remove)
            Map.deleteRoom(r, 0);
        else
            Map.deleteRoom(r, 1);
    }


    stacker.swap();

    toggle_renderer_reaction();


    send_to_user("--[ Removed.\r\n");
    send_prompt();
    return USER_PARSE_SKIP;
}


USERCMD(usercmd_mnote)
{
  char *p;
  CRoom *r;

  userfunc_print_debug;

  r = stacker.first();

  p = skip_spaces(line);
  r->setNote(p);


  send_to_user("--[ Added.\r\n");
  send_prompt();
  return USER_PARSE_SKIP;
}

USERCMD(usercmd_mnotecolor)
{
  char *p;
  CRoom *r;

  userfunc_print_debug;

  r = stacker.first();

  p = skip_spaces(line);

  QColor color = QColor(p);
  if(color.isValid()) {
    r->setNoteColor(p);
    send_to_user("--[ Set note color.\r\n");
  } else {
    send_to_user("--[ Invalid color: %s!\r\n",line);
  }

  send_prompt();
  return USER_PARSE_SKIP;
}


USERCMD(usercmd_mtreestats)
{

  userfunc_print_debug;

  skip_spaces(line);


  NameMap.printTreeStats();

  send_prompt();
  return USER_PARSE_SKIP;
}


USERCMD(usercmd_mrefresh)
{
  userfunc_print_debug;
  skip_spaces(line);

  stacker.first()->setName(engine->getRoomName());
  stacker.first()->setDesc(engine->getDesc());
  stacker.first()->setTerrain(engine->getTerrain());

  send_to_user("--[ Refreshed.\r\n");
  send_prompt();
  return USER_PARSE_SKIP;
}


USERCMD(usercmd_mgoto)
{
    char *p;
    char arg[MAX_STR_LEN];
    unsigned int id;
    CRoom *r;
    int dir;

    userfunc_print_debug;

    // check for mgoto ID syntax
    p = skip_spaces(line);
    // if not go on with selection
    if (!*p) {
        if (Map.selections.isEmpty() == false) {
            stacker.put(Map.selections.getFirst());
        } else
            MISSING_ARGUMENTS

    } else {
        p = one_argument(p, arg, 0);
        if (is_integer(arg)) {
            id = atoi(arg);
            if (Map.getRoom(id) == NULL) {
                send_to_user("--[ There is no room with id %s.\r\n", arg);
                send_prompt();
                return USER_PARSE_SKIP;
            }

            stacker.put(Map.getRoom(id));

        } else {
            CHECK_SYNC;
            r = stacker.first();

            PARSE_DIR_ARGUMENT(dir, arg);

            if ( r->isConnected(dir) == false ) {
                send_to_user("--[ Bad direction - there is no connection.\r\n", arg);
                send_prompt();
                return USER_PARSE_SKIP;
            }

            stacker.put(r->exits[dir]->id);
        }
    }

    renderer_window->renderer->userX = 0;
    renderer_window->renderer->userY = 0;
//    renderer_window->renderer->userz = 0;


    engine->setMgoto(true);  /* ignore prompt while we are in mgoto mode */
    stacker.swap();
    send_prompt();
    return USER_PARSE_SKIP;
}


/*     "    Usage: mdetach <direction> [oneway|delete]\r\n" */
USERCMD(usercmd_mdetach)
{
  char *p;
  char arg[MAX_STR_LEN];
  CRoom *r, *s;
  int del = 0, oneway = 0;
  int dir;
  unsigned int i;

  userfunc_print_debug;

  p = skip_spaces(line);
  if (!*p) MISSING_ARGUMENTS

  p = one_argument(p, arg, 0);

  PARSE_DIR_ARGUMENT(dir, arg);

  for (i = 0; i < 2; i++) {
    p = skip_spaces(p);
    if (*p) {
      p = one_argument(p, arg, 0);
      if (is_abbrev(arg, "oneway") )
        oneway = 1;
      if (is_abbrev(arg, "delete") )
        del = 1;
    }
  }

  r = stacker.first();
  if (r->isExitPresent(dir) == true) {
    s = r->exits[dir];
    if (del) {
        r->removeExit(dir);
    } else {
        r->setExitUndefined(dir);
    }
  } else {
    send_to_user("--[ %s is not marked nor linked.\r\n", exits[dir] );
    send_prompt();
    return USER_PARSE_SKIP;
  }

  if (!oneway && s != NULL) {
    for (i = 0; i<= 5; i++)
      if (s->isExitLeadingTo(i, r) == true) {
        if (del) {
            s->removeExit(i);
        } else {
            s->setExitUndefined(i);
        }
      }
  }

  send_prompt();
  return USER_PARSE_SKIP;
}

/* "    Usage: mlink <dirrection> <id> [backdir] [force|oneway]\r\n"  */
USERCMD(usercmd_mlink)
{
  int force = 0, oneway = 0, backdir = -1, id, dir;
  char *p;
  char arg[MAX_STR_LEN];
  CRoom *r, *second;
  int i;

  userfunc_print_debug;

  r = stacker.first();


  /* get essential arguments - id and direction */
  p = skip_spaces(line);
  if (!*p) MISSING_ARGUMENTS

  p = one_argument(p, arg, 0);


  PARSE_DIR_ARGUMENT(dir, arg);

  p = skip_spaces(p);
  if (!*p) MISSING_ARGUMENTS
  p = one_argument(p, arg, 0);

  GET_INT_ARGUMENT(arg, id);

  second = Map.getRoom(id);

  if (second == NULL) {
    send_to_user("--[ There is no room with id %i.\r\n", id);
    send_prompt();
    return USER_PARSE_SKIP;
  }

  /* get optional arguments */
  do {
    p = skip_spaces(p);
    p = one_argument(p, arg, 0);

    for (i = 0; i <= 5; i++)
      if (is_abbrev(arg, exits[i]) )
        backdir = i;

    if (is_abbrev(arg, "force"))
      force = 1;

    if (is_abbrev(arg, "oneway"))
      oneway = 1;


  } while (*p);

  /* first room */
  if (r->isConnected(dir) && !force) {
    send_to_user("--[ There is an existing connection to the %s.\r\n", exits[dir]);
    send_prompt();
    return USER_PARSE_SKIP;
  }

  r->setExit(dir, id);
  send_to_user("--[ Linked exit %s with %s [%i].\r\n", exits[dir], (const char *) second->getName(), id);

  /* if oneway option was given - we are done here */
  if (!oneway) {
    /* setup backdir to dummie-opposite if needed */
    if (backdir == -1)
      backdir = reversenum(dir);

    if (second->isConnected(backdir) && !force) {
      send_to_user("--[ There is an existing connection to the %s in second room.\r\n", exits[backdir]);
      send_prompt();
      return USER_PARSE_SKIP;
    }


    second->setExit(backdir, r);
    send_to_user("--[ Linked exit %s in second room with this room.\r\n", exits[backdir]);
  }

  send_prompt();
  return USER_PARSE_SKIP;
}


USERCMD(usercmd_mmark)
{
  char *p;
  char arg[MAX_STR_LEN];
  CRoom *r;
  int i, dir;

  userfunc_print_debug;

  p = skip_spaces(line);
  if (!*p) MISSING_ARGUMENTS

  p = one_argument(p, arg, 0);    /* direction */

  PARSE_DIR_ARGUMENT(dir, arg);

  p = skip_spaces(p);
  if (!*p) MISSING_ARGUMENTS

  p = one_argument(p, arg, 0);    /* flag */

  r = stacker.first();


  i = 0;
  while (room_flags[i].name != "") {
    if (is_abbrev(arg, room_flags[i].name) ) {
      r->setExitFlags(dir, room_flags[i].flag);
      send_to_user("--[Pandora: Marked %s as %s\r\n", exits[dir],  (const char *) room_flags[i].xml_name);

      send_prompt();
      return USER_PARSE_SKIP;
    }
    i++;
  }
  send_to_user("--[ no such flag.\r\n");
  send_prompt();
  return USER_PARSE_SKIP;
}

USERCMD(usercmd_mdoor)
{
  char *p;
  char arg[MAX_STR_LEN];
  char arg2[MAX_STR_LEN];
  CRoom *r;
  int i;

  userfunc_print_debug;

  p = skip_spaces(line);
  if (!*p) MISSING_ARGUMENTS

  switch (subcmd)
  {
	case  USER_DOOR_NORMAL:
                p = one_argument(p, arg, 1);    /* do not lower-case the door name */

                p = skip_spaces(p);
                if (!*p) MISSING_ARGUMENTS

		break;
	case  USER_DOOR_EXIT:
                strcpy(arg, "exit");             /* normal exit */

		break;
  }

  p = one_argument(p, arg2, 0);    /* direction */

  r = stacker.first();

  PARSE_DIR_ARGUMENT(i, arg2);

  if (strcmp(arg, "remove") == 0) {
    if (r->isDoorSet(i) == false) {
      send_to_user("--[ There is no door to the %s.\r\n", exits[i]);
    } else {
      r->removeDoor(i);
      send_to_user("--[Pandora: Removed the door to the %s\n", exits[i]);
    }
  } else {
    r->setDoor(i, arg);
    send_to_user("--[Pandora: Added the door %s to the %s\n", arg, exits[i]);
  }

  send_prompt();
  return USER_PARSE_SKIP;
}



USERCMD(usercmd_mcoord)
{
  char *p;
  char arg[MAX_STR_LEN];
  int value;
  CRoom *r;

  userfunc_print_debug;

  r = stacker.first();

  p = skip_spaces(line);
  if (!*p) {
    send_to_user("--[ Current coordinates : X %i, Y %i, Z %i.\r\n", r->getX(), r->getY(), r->getZ());
    send_prompt();
    return USER_PARSE_SKIP;
  }

  p = one_argument(p, arg, 0);

  GET_INT_ARGUMENT(arg, value);
  r->setX(value);
  send_to_user("--[ X set to %i.\r\n", r->getX());

  p = skip_spaces(p);
  if (!*p) {
    send_prompt();
    return USER_PARSE_SKIP;
  }

  p = one_argument(p, arg, 0);

  GET_INT_ARGUMENT(arg, value);
  r->setY(value);
  send_to_user("--[ Y set to %i.\r\n", r->getY());

  p = skip_spaces(p);
  if (!*p) {
    send_prompt();
    return USER_PARSE_SKIP;
  }

  p = one_argument(p, arg, 0);
  GET_INT_ARGUMENT(arg, value);
  r->setZ(value);
  send_to_user("--[ Z set to %i.\r\n", r->getZ());

  send_prompt();
  return USER_PARSE_SKIP;
}



USERCMD(usercmd_mdec)
{
  char *p;
  char arg[MAX_STR_LEN];
  int value;
  CRoom *r;

  userfunc_print_debug;

  value = 1;
  p = skip_spaces(line);
  if (*p) {
    p = one_argument(p, arg, 0);

    GET_INT_ARGUMENT(arg, value);
  }

  r = stacker.first();

  switch (subcmd)
  {
	case  USER_DEC_X:
                r->setX(r->getX() - value);
		break;
	case  USER_INC_X:
                r->setX(r->getX() + value);
		break;
	case  USER_DEC_Y:
                r->setY(r->getY() - value);
		break;
	case  USER_INC_Y:
                r->setY(r->getY() + value);
		break;
	case  USER_DEC_Z:
                r->setZ(r->getZ() - value);
		break;
	case  USER_INC_Z:
                r->setZ(r->getZ() + value);
		break;
  }

  send_to_user("--[Pandora: Changed.\n");
  send_prompt();
  return USER_PARSE_SKIP;
}

/* print brief help if no arguments given, or detailed help for given argument*/
USERCMD(usercmd_mhelp)
{
  char *p;
  char arg[MAX_STR_LEN];

  userfunc_print_debug;

  p = skip_spaces(line);
  if (*p) {
    /* argument is given */
    p = one_argument(p, arg, 0);

    for (int i=0; user_commands[i].name != NULL; i++)
      if ((strcmp(user_commands[i].name, arg) == 0) && user_commands[i].help)
      {
        send_to_user("---[Help file : %s.\r\n", user_commands[i].name);

        proxy->send_line_to_user((char *)user_commands[i].help);

        send_to_user("\r\n");

        send_prompt();
        return USER_PARSE_SKIP;
      }

  }

  /* else we print all brief help files */

  send_to_user("----[ Brief help files/commands overview.\r\n\r\n");
  for (int i=0; user_commands[i].name != NULL; i++)
    if (user_commands[i].desc)
      send_to_user("  %-15s  %-65s\r\n", user_commands[i].name, user_commands[i].desc);

  send_to_user("\r\n");

  send_prompt();

  return USER_PARSE_SKIP;
}

USERCMD(usercmd_config)
{
  int desired;
  char *p;
  char arg[MAX_STR_LEN];

  userfunc_print_debug;

  p = skip_spaces(line);
  if (!*p) {
    /* no arguments */
    desired = -1;       /* so, toggle the state */
  } else {
    /* else get the desired state */

    p = one_argument(p, arg, 0);

    desired = get_input_boolean(arg);
  }

  switch (subcmd)
  {
	case  USER_CONF_MAP :
                if (desired == -1) {

                }

                if ((desired == 0) || (desired == -1 && engine->isMapping())) {
                    engine->setMapping(false);
                    send_to_user("--[Pandora: Mapping is now OFF!\r\n");

                    send_prompt();
                    return USER_PARSE_SKIP;
                } else if (desired == 1 || (desired == -1 && !engine->isMapping()) )

                {
                  if ( !engine->empty() ) {
                    send_to_user("--[Pandora: mreset is required to clear the Cause-Result stacks.\n");
                    send_prompt();
                    return USER_PARSE_SKIP;
                  }

                  engine->setMapping(true);
                  conf->setExitsCheck(false);
                  send_to_mud("brief OFF\n");
                  send_to_mud("spam ON\n");
                  send_to_mud("prompt all\n");

                  send_to_user("--[Pandora: Exits analyzer is now OFF\n");
                  send_to_user("--[Pandora: Mapping is now on!\n");

                  send_prompt();
                  return USER_PARSE_SKIP;
                }

		break;
	case  USER_CONF_BRIEF:
                if (desired == -1)
                  conf->setBriefMode( !conf->getBriefMode());
                else
                  conf->setBriefMode(desired);

                send_to_user("----[ Mapper Brief Mode is now %s.\r\n",
                              ON_OFF(conf->getBriefMode()) );
		break;
	case  USER_CONF_AUTOMERGE:
                if (desired == -1)
                  conf->setAutomerge( !conf->getAutomerge() );
                else
                  conf->setAutomerge(desired);

                send_to_user("----[ Description analyzer and automatic merging is now %s.\r\n",
                              ON_OFF(conf->getAutomerge()) );
		break;
	case  USER_CONF_ANGRYLINKER:
                if (desired == -1)
                  conf->setAngrylinker( !conf->getAngrylinker() );
                else
                  conf->setAngrylinker(desired);

                send_to_user("----[ AngryLinker is now %s.\r\n",
                              ON_OFF(conf->getAngrylinker()) );
		break;

	case  USER_CONF_DUALLINKER:
                if (desired == -1)
                  conf->setDuallinker( !conf->getDuallinker());
                else
                  conf->setDuallinker(desired);

                send_to_user("----[ DualLinker is now %s.\r\n",
                              ON_OFF(conf->getDuallinker()) );
		break;

        case  USER_CONF_EXITS:
                if (desired == -1)
                  conf->setExitsCheck( !conf->getExitsCheck() );
                else
                  conf->setExitsCheck(desired);

                send_to_user("----[ Exits analyzer is now %s.\r\n",
                              ON_OFF( conf->getExitsCheck() ) );
		break;
	case  USER_CONF_TERRAIN:
                if (desired == -1)
                  conf->setTerrainCheck( !conf->getTerrainCheck());
                else
                  conf->setTerrainCheck(desired);

                send_to_user("----[ Terrain analyzer is now %s.\r\n",
                              ON_OFF(conf->getTerrainCheck()) );
		break;
  }

  send_prompt();
  return USER_PARSE_SKIP;
}


USERCMD(usercmd_msave)
{
  char *p;
  char arg[MAX_STR_LEN];


  userfunc_print_debug;


  p = skip_spaces(line);
  if (!*p) {
    /* no arguments */
    //xml_writebase( conf->get_base_file() );
    Map.saveMap(conf->getBaseFile() );
    send_to_user("--[Pandora: Saved...\r\n");
    conf->setDatabaseModified(false);


    send_prompt();
    return USER_PARSE_SKIP;
  } else {
    //p = one_argument(p, arg, 1);        /* do not lower or upper case - filename */

    //xml_writebase(arg);
    Map.saveMap( p );

    send_to_user("--[Pandora: Saved to %s...\r\n", arg);

    conf->setDatabaseModified(false);

    send_prompt();
    return USER_PARSE_SKIP;
  }
}

USERCMD(usercmd_mload)
{
  char *p;
  char arg[MAX_STR_LEN];

  userfunc_print_debug;

  send_to_user("--[Pandora: Reloading the database ...\n");
  send_to_user(" * Clearing the database class...\r\n");
  Map.reinit();  /* this one reinits Ctree structure also */

  send_to_user(" * Resetting possibility stacks...\r\n");
  stacker.reset();  /* resetting stacks */

  send_to_user(" * Clearing events stacks...\r\n");
  engine->clear();

  engine->setMapping(false);


  p = skip_spaces(line);
  if (!*p) {
    /* no arguments */
    send_to_user(" * Loading the file %s from disk...\r\n",
                  (const char *) conf->getBaseFile()  );

    //xml_readbase( conf->get_base_file() );
    Map.loadMap( conf->getBaseFile() );

  } else {
    //p = one_argument(p, arg, 1);        /* do not lower or upper case - filename */

    send_to_user(" * Loading the base %s from disk...\r\n", p);
    //xml_readbase(arg);
    Map.loadMap( p );

  }


  send_to_user(" * Ready to go!\r\n");

  send_to_user(" * Unlocking analyzer-thread...\r\n");

  send_to_user("--[Pandora: Done.\r\n");

  conf->setDatabaseModified(false);


  send_prompt();
  return USER_PARSE_SKIP;
}


USERCMD(usercmd_mreset)
{
  userfunc_print_debug;
  skip_spaces(line);

  engine->clear();

  stacker.reset();

  send_to_user("--[Pandora: Resetted!\n");

  send_prompt();
  return USER_PARSE_SKIP;
}

USERCMD(usercmd_mmerge)
{
  char *p;
  char arg[MAX_STR_LEN];
  int force = 0;
  CRoom *t = NULL;
  int j = -1;
  unsigned int i;
  unsigned int id;

  userfunc_print_debug;

  if (engine->addedroom == NULL) {
    send_to_user("--[There is new added room to merge.\r\n");
    send_prompt();
    return USER_PARSE_SKIP;
  }

  /* find the only defined exit in new room - the one we came from */
  for (i = 0; i <= 5; i++)
    if (engine->addedroom->isExitNormal(i) == true) {
      j = i;
      break;
    }


  p = skip_spaces(line);
  if (!*p) {
    /* no arguments at all - so, find ID */

    // yet again, sensitive!
    t = Map.findDuplicateRoom(engine->addedroom);
    if (t == NULL) {
      send_to_user("--[ No matching room found.\r\n");
      send_prompt();
      return USER_PARSE_SKIP;
    }

  } else {

    p = one_argument(p, arg, 0);
    GET_INT_ARGUMENT(arg, id);

    if (id <= 0 || id > MAX_ROOMS) {
      send_to_user("--[ %s is not a room id.\r\n", arg);
      send_prompt();
      return USER_PARSE_SKIP;
    }

    t = Map.getRoom(id);
    if (t == NULL) {
      send_to_user("--[ There is no room with this id %i.\r\n", id);
      send_prompt();
      return USER_PARSE_SKIP;
    }

    p = skip_spaces(p);
    if (*p) {
      p = one_argument(p, arg, 0);
      if (is_abbrev(arg, "force"))
        force = 1;

    }

    if (!force)
      if (engine->addedroom->isEqualNameAndDesc(t) == false)
      {
        send_to_user("--[ Roomname or description do not match.\r\n");
        send_prompt();
        return USER_PARSE_SKIP;
      }

  }

  if (Map.tryMergeRooms(t, engine->addedroom, j)) {
	  send_to_user("--[ merged.\r\n");
  } else {
	  send_to_user("--[ failed.\r\n");
	  stacker.put(engine->addedroom);
  }

  stacker.swap();

  /* now make sure we have a room in stack */

  send_prompt();
  return USER_PARSE_SKIP;
}


USERCMD(usercmd_mstat)
{
  userfunc_print_debug;
  skip_spaces(line);

  engine->printStacks();

  send_prompt();
  return USER_PARSE_SKIP;
}

USERCMD(usercmd_minfo)
{
  char *p;
  char arg[MAX_STR_LEN];
  int id;
  CRoom *t;
  unsigned int i;

  userfunc_print_debug;

  p = skip_spaces(line);
  if (!*p) {
    /* no arguments */
  } else {
    p = one_argument(p, arg, 1);        /* do not lower or upper case - filename */

    GET_INT_ARGUMENT(arg, id);

    if (id <= 0 || id > MAX_ROOMS) {
      send_to_user("--[ %s is not a room id.\r\n", arg);
      send_prompt();
      return USER_PARSE_SKIP;
    }

    t = Map.getRoom(id);
    if (t == NULL) {
      send_to_user("--[ There is no room with this id %i.\r\n", id);
      send_prompt();
      return USER_PARSE_SKIP;
    }

    t->sendRoom();
    send_prompt();
    return USER_PARSE_SKIP;
  }



   for (i = 0; i < stacker.amount(); i++) {
     t = stacker.get(i);
     t->sendRoom();
   }

   send_prompt();
   return USER_PARSE_SKIP;
}


USERCMD(usercmd_move)
{
  CRoom *r;
  int dir;

  userfunc_print_debug;

  /*firstly - send the command futher to mud, as if it matches */
  /* used commands - it wont be automatically send futher */

  if (proxy->isMudEmulation()) {

    if (stacker.amount() == 0) {
        send_to_user( "You are in an undefined position.\r\n");
        send_to_user( "Use mgoto <room_id> to go to some place...\r\n");

	send_prompt();
        return USER_PARSE_SKIP;
    }
    r = stacker.first();


    dir = -1;

    switch (subcmd)
    {

          case  NORTH:
                  dir = NORTH;
                  break;
          case  EAST:
                  dir = EAST;
                  break;
          case  SOUTH:
                  dir = SOUTH;
                  break;
          case  WEST:
                  dir = WEST;
                  break;
          case  UP:
                  dir = UP;
                  break;
          case  DOWN:
                  dir = DOWN;
                  break;
          case USER_MOVE_LOOK:
          case USER_MOVE_EXAMINE:
                  r->sendRoom();
		  send_prompt();
                  return USER_PARSE_SKIP;

                  break;
    }

    if (dir == -1)
      return USER_PARSE_NONE;
    if ( r->isConnected(dir)  == false ) {
      send_to_user("Alas, you cannot go this way.\r\n\r\n");
    } else {
      stacker.put(r->exits[dir]->id);
      stacker.swap();


      r = stacker.first();
      engine->updateRegions();

      toggle_renderer_reaction();

      r->sendRoom();
    }


    send_prompt();
    return USER_PARSE_SKIP;

  }

  return USER_PARSE_DONE;   /* leave the line as it is */
}


USERCMD(usercmd_mregion)
{
    char *p;
    char arg[MAX_STR_LEN];

    userfunc_print_debug;

    p = skip_spaces(line);
    if (!*p) {
            /* print help file or current settings */
        send_to_user("Settings: autoReplace %s, info in client view %s, info in renderer %s\r\n",
                                    ON_OFF(conf->getRegionsAutoReplace()) ,
                                    ON_OFF(conf->getShowRegionsInfo() ),
                                    ON_OFF(conf->getDisplayRegionsRenderer() )
                                    );
        send_to_user("Current USER region: %s\r\n", (const char *) engine->get_users_region()->getName());
        send_to_user("Current ROOMS region: %s\r\n", (const char *) engine->get_last_region()->getName());

        send_prompt();
        return USER_PARSE_SKIP;
    }

    p = one_argument(p, arg, 0);


    // ---------------------------------------------- ADD  NEW REGION -------------------------------------------------------------------
    if (is_abbrev(arg, "add") ) {
        /* add new region */

        p = skip_spaces(p);
        if (!*p) {
            send_to_user("Error. Missing new regions name. \r\n");
            send_prompt();
            return USER_PARSE_SKIP;
        }
        p = one_argument(p, arg, 0);
        if (Map.addRegion(arg) == true) {
//            engine->set_users_region( Map.getRegionByName( arg ) );
            send_to_user("Ok. Done.\r\n");
        } else {
            send_to_user("Error. Failed to add new region named %s.\r\n", arg);
        }
	send_prompt();
        return USER_PARSE_SKIP;
    }


    // -------------------------------------------- DOORS IN REGION ----------------------------------------------------------------------
    if (is_abbrev( arg, "door") ) {

        p = skip_spaces(p);
        if (!*p) {
            // PRINT USAGE
	    send_prompt();
            return USER_PARSE_SKIP;
        }
        p = one_argument(p, arg, 0);

        if (is_abbrev(arg, "add")) {
            p = skip_spaces(p);


            if (!*p) {
                // PRINT USAGE
                send_to_user( "Missing arguments. Usage: mregion door add <alias> <door> <dirrection>\r\n");
                send_prompt();
                return USER_PARSE_SKIP;
            }
            p = one_argument(p, arg, 0);
            p = skip_spaces(p);
            if (!*p) {
                // PRINT USAGE
                send_to_user( "Missing arguments. Usage: mregion door add <alias> <door> <dirrection>\r\n");
                send_prompt();
                return USER_PARSE_SKIP;
            }


            engine->get_users_region()->addDoor(arg, p);
            send_to_user( "Ok. Added %s (alias %s) to the region %s\r\n", p, arg, (const char *) engine->get_users_region()->getName() );

	    send_prompt();
            return USER_PARSE_SKIP;
        } else  if (is_abbrev(arg, "action")) {
                bool local;
                QByteArray alias;

                p = skip_spaces(p);
                if (!*p) MISSING_ARGUMENTS
                p = one_argument(p, arg, 0);

                local = false;
                if (strcmp(arg, "local") == 0) {
                    local = true;
                    p = skip_spaces(p);
                    if (!*p) MISSING_ARGUMENTS
                    p = one_argument(p, arg, 0);
                }
                alias = arg;

                p = skip_spaces(p);
                strncpy(arg, p, MAX_STR_LEN/2);

                original[0] = 0;      /* nullify the incoming line and get ready to put generated commands */

                QByteArray door = engine->get_users_region()->getDoor( alias );
                if (door == "") {
                    send_to_user( "--[ No door with such an alias (%s) in this region!\r\n", (const char *) alias);
                } else {
                    if (local == false)
                        sprintf(original+strlen(original), "%s %s", arg, (const char *) door);
                    send_to_user("%s %s\r\n", arg, (const char *) door);
                }

                if (local) {
		    send_prompt();
                    return USER_PARSE_SKIP;
                } else {
                    return USER_PARSE_DONE;
                }
        } else  if (is_abbrev(arg, "remove")) {
            p = skip_spaces(p);

            if (!*p) {
                // PRINT USAGE
                send_to_user( "Missing arguments. Usage: mregion door remove <alias>\r\n");
                send_prompt();
                return USER_PARSE_SKIP;
            }
            p = one_argument(p, arg, 0);
            if (engine->get_users_region()->removeDoor(arg) == true)
                send_to_user( "Ok. Removed.\r\n");
            else
                send_to_user( "Sorry, failed.\r\n");

	    send_prompt();
            return USER_PARSE_SKIP;
        } else if (is_abbrev(arg, "list")) {
            engine->get_users_region()->showRegion();
	    send_prompt();
            return USER_PARSE_SKIP;
        }

    }


    //------------------------------------------------------------  SET USERS REGION ---------------------------------------------------
    if (is_abbrev(arg, "set") ) {
        /* set current USERS region */
        p = skip_spaces(p);
        if (!*p) {
            send_to_user("Error. Missing regions name. \r\n");
            send_prompt();
            return USER_PARSE_SKIP;
        }
        p = one_argument(p, arg, 0);
        CRegion *reg;

        reg = Map.getRegionByName( arg );
        if (reg != NULL) {
            engine->set_users_region( reg );
        } else {
            send_to_user( "Failed. No such region!\r\n");
        }

	send_prompt();
        return USER_PARSE_SKIP;
    }


    // ---------------------------------------------------------------- LIST ALL REGIONS -------------------------------------------------------
    if (is_abbrev(arg, "list") ) {
        /* show a list of all regions  */
        Map.sendRegionsList();
	send_prompt();
        return USER_PARSE_SKIP;
    }


    // ---------------------------------------------------------------- REPLACE REGION IN THIS ROOM --------------------------------------
    if (is_abbrev(arg, "replace") ) {
        /* set region in current ROOM (reset) */
        CRoom *r;

        if (stacker.amount() != 1) {
            send_to_user( "Error. Must be in sync to use this subcommand!\r\n");
            send_prompt();
            return USER_PARSE_SKIP;
        }

        p = skip_spaces(p);
        if (!*p) {
            send_to_user("Error. Missing regions name. \r\n");
	    send_prompt();
            return USER_PARSE_SKIP;
        }
        p = one_argument(p, arg, 0);
        CRegion *reg;

        reg = Map.getRegionByName( arg );
        if (reg != NULL) {


            send_to_user( "Ok. Setting current room's region to %s.\r\n", (const char *) reg->getName() );
            r = stacker.first();
            r->setRegion( reg );
            engine->set_last_region( reg );
            engine->set_users_region( reg );


        } else {
            send_to_user( "Failed. No such region!\r\n");
        }


	send_prompt();
        return USER_PARSE_SKIP;
    }





    if (is_abbrev(arg, "config")) {
        int desired;
        /* automatic regions REPLACEMENT on movement  */
        p = skip_spaces(p);

        if (!*p) {
            // TODO a little help - diplay possible options here
	    send_prompt();
            return USER_PARSE_SKIP;
        }

        p = one_argument(p, arg, 0);

        if (is_abbrev(arg, "autoreplace")) {
            int desired;
            /* automatic regions REPLACEMENT on movement  */

            p = one_argument(p, arg, 0);

            desired = get_input_boolean(arg);
            if (desired == -1)
                conf->setRegionsAutoReplace( !conf->getRegionsAutoReplace());
            else
                conf->setRegionsAutoReplace( desired);

            send_to_user("----[ Auto Regions Replace Mode is now %s.\r\n",
                                ON_OFF(conf->getRegionsAutoReplace()) );

	    send_prompt();
            return USER_PARSE_SKIP;
        } else if (is_abbrev(arg, "render")) {
            desired = get_input_boolean(arg);
            if (desired == -1)
                conf->setDisplayRegionsRenderer( !conf->getDisplayRegionsRenderer());
            else
                conf->setDisplayRegionsRenderer( desired);

            send_to_user("----[ 'Display regions in renderer' Mode is now %s.\r\n",
                                    ON_OFF(conf->getDisplayRegionsRenderer()) );


        } else if (is_abbrev(arg, "info")) {
            desired = get_input_boolean(arg);
            if (desired == -1)
                conf->setShowRegionsInfo( !conf->getShowRegionsInfo());
            else
                conf->setShowRegionsInfo( desired);

            send_to_user("----[ 'Show  regions info' Mode  is now %s.\r\n",
                                    ON_OFF(conf->getShowRegionsInfo()) );
        }

	send_prompt();
	return USER_PARSE_SKIP;
    }


    if (strcmp(arg, "autoset") == 0) {
        /* automatic regions change  setting */
        int desired;
        /* automatic regions REPLACEMENT on movement  */

        p = one_argument(p, arg, 0);

        desired = get_input_boolean(arg);
        if (desired == -1)
            conf->setRegionsAutoSet( !conf->getRegionsAutoSet());
        else
             conf->setRegionsAutoSet( desired );

        send_to_user("----[ Automatic regions switching is now %s.\r\n",
                              ON_OFF(conf->getRegionsAutoSet()) );

        send_prompt();
        return USER_PARSE_SKIP;
    }

    send_prompt();
    return USER_PARSE_SKIP;
}




