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

#ifndef USERLAND_H 
#define USERLAND_H 


#define USERCMD(name)  int name(int cmd, int subcmd, char *line, char *original)
#include <deque>
#include <QMutex>
using namespace std;

#define USER_DEC_X      1
#define USER_DEC_Y      2
#define USER_DEC_Z      3
#define USER_INC_X      4
#define USER_INC_Y      5
#define USER_INC_Z      6
#define USER_DOOR_NORMAL        0
#define USER_DOOR_EXIT          1


USERCMD(usercmd_mhelp);
USERCMD(usercmd_config);
USERCMD(usercmd_msave);
USERCMD(usercmd_mload);
USERCMD(usercmd_mreset);
USERCMD(usercmd_mstat);
USERCMD(usercmd_minfo);
USERCMD(usercmd_move);
USERCMD(usercmd_mmerge);
USERCMD(usercmd_mdec);
USERCMD(usercmd_mcoord);
USERCMD(usercmd_mdoor);
USERCMD(usercmd_mmark);
USERCMD(usercmd_mlink);
USERCMD(usercmd_mdetach);
USERCMD(usercmd_mgoto);
USERCMD(usercmd_maction);
USERCMD(usercmd_mdebug);
USERCMD(usercmd_mdelete);
USERCMD(usercmd_mrefresh);
USERCMD(usercmd_maddroom);
USERCMD(usercmd_mnote);
USERCMD(usercmd_mnotecolor);
USERCMD(usercmd_mregion);






struct user_command_type {
  const char *name;
  int (*command_pointer) (int cmd, int subcmd, char *line, char *orig);
  int subcmd;           /* subcommand*/
  unsigned int flags;
  const char *desc;     /* short command description */
  const char *help;     /* long description/helpfile */
};

extern const struct user_command_type user_commands[];

struct queued_command_type {
  int  id;
  char arg[MAX_STR_LEN];  
};

class Userland {
  QMutex queue_mutex;
  deque<struct queued_command_type> commands_queue;
public:
    
  void parse_command();
  void add_command(int id, char *arg);
  int is_empty() { return commands_queue.empty(); }
  int parse_user_input_line(const char *line); 
 

};

extern class Userland *userland_parser;

#define USER_PARSE_NONE 0 /* 0 - not my area - send the line futher */
#define USER_PARSE_SKIP 1 /* skip this line */
#define USER_PARSE_DONE 2 /* parsed the line, pass corrected line futher */

#endif

