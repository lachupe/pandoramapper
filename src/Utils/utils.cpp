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

/* utilities, string functions and stuff */
#include "defines.h"
#include "CConfigurator.h"

#if defined Q_OS_WIN32
  #define vsnprintf _vsnprintf
#endif

#include "utils.h"

#include "Proxy/proxy.h"
#include "Engine/CEngine.h"

FILE *logfile = NULL;
const char *exitnames[] = { "north", "east", "south", "west", "up", "down" };

struct debug_data_struct debug_data[] = {
  {"general",  "Debug", "general switch for debug",  DEBUG_GENERAL, 1},
  {"analyzer", "Analyzer",  "analyzer messages", DEBUG_ANALYZER,1},
  {"system", "System", "general messages", DEBUG_SYSTEM, 1},
  {"config", "Config", "configuration file reader/parser messages", DEBUG_CONFIG, 1},
  {"dispatcher", "Dispatcher", "network streams Dispatcher messages",  DEBUG_DISPATCHER,  1},
  {"proxy", "Proxy", "proxy messages", DEBUG_PROXY,   1},
  {"renderer", "Renderer", "OpenGL Renderer messages",DEBUG_RENDERER, 1},
  {"roommanager", "Roommanager", "roommanagers messages", DEBUG_ROOMS, 0},
  {"stacks", "Stacker", "stacks messages", DEBUG_STACKS, 0},
  {"tree", "Tree", "tree search engine messages", DEBUG_TREE, 0},
  {"userfunc", "Userfunc", "user interface messages", DEBUG_USERFUNC, 0},
  {"xml", "xml", "XML module messages",  DEBUG_XML, 0},
  {"interface", "Interface", "Qt windowing interface",  DEBUG_INTERFACE, 0},
  {"spells", "Spells", "Spells timers messages",  DEBUG_SPELLS, 0},
  {"group", "GroupManager", "Group Manager messages",  DEBUG_GROUP, 1},


  {NULL, NULL, NULL, 0, 0}
};

const char *exits[] = {
      "north",
      "east",
      "south",
      "west",
      "up",
      "down"
};


const boolean_struct input_booleans[] = {
  {"on", true},
  {"off", false},
  {"1", true},
  {"0", false},
  {"yes", true},
  {"no", false},
  {"+", true},
  {"-", false},
  {"true", true},
  {"false", false},


  {NULL, false}
};

QTime debug_timer;

int write_to_channel(int mode, const char *format, va_list args);

int write_debug(unsigned int flag, const char *format, va_list args);


int MIN(int a, int b)
{
  return (a < b ? a : b);
}


int write_debug(unsigned int flag, const char *format, va_list args)
{
    char txt[MAX_STR_LEN*2];
    int size;
    int i;

    if (conf != NULL && !conf->getLogFileEnabled())
        return -1; // log file is disabled

    if (logfile == NULL) {
    	debug_timer.start();

        QString fileName = QString("logs/" + QDateTime::currentDateTime().toString("dd.MM.yyyy-hh.mm.ss") + ".txt");
        printf("Using the LOGFILE : %s\r\n", (const char *) fileName.toLocal8Bit() );

        logFileName = new QString();
        *logFileName = fileName;

        logfile = fopen( (const char *) fileName.toLocal8Bit(), "w+");
        if (!logfile) {
            perror ("Error opening logfile for writing");
            return -1;
        }
    }

    size = vsnprintf(txt, sizeof(txt), format, args);

    for (i = 0; debug_data[i].name; i++)
        if (IS_SET(flag, debug_data[i].flag) && debug_data[i].state) {
        fprintf(logfile, "[%i] %s: %s\r\n", debug_timer.elapsed(), debug_data[i].title, txt);
        fflush(logfile);
        if (IS_SET(flag, DEBUG_TOUSER) )
            send_to_user("--[ %s: %s\r\n", debug_data[i].title, txt);
        }

    return size;
}


void print_debug(unsigned int flag, const char *messg, ...)
{
  va_list args;

  if (!debug_data[0].state)
    return;

  if (messg == NULL)
    return;

  va_start(args, messg);
  write_debug(flag, messg, args);
  va_end(args);
}


int parse_dir(char *dir)
{
  int i;

  for (i = 0; i<= 5; i++)
    if (is_abbrev(dir, exits[i]) )
      return i;

  return -1;
}


int is_integer(char *str)
{
  while ((isdigit(*str) || isspace(*str) || (*str == '-') || (*str == '+')) &&
              (*str != 0))
    str++;

  if (*str != 0)
    return 0;

  return 1;
}


int get_input_boolean(char *input)
{
  int i;

  for (i = 0; input_booleans[i].name != NULL; i++)
    if (strcmp(input, input_booleans[i].name) == 0)
      return input_booleans[i].state;

  return -1;
}


char *skip_spaces(const char *str)
{
  while (isspace(*str) && (*str != 0))
    str++;
  return (char *) str;
}

char *next_space(char *str)
{
  while (isspace(*str) && (*str != 0))
    str++;
  return str;
}

/* mode 0 - lower all chars in argument */
/* mode 1 - do not lower all chars in argument */
/* mode 2 - upper all chars in argument */
char *one_argument(char *argument, char *first_arg, int mode)
{

  if (!argument) {
    *first_arg = '\0';
    return (NULL);
  }

  while (*argument && !isspace(*argument)) {
    if (mode == 0) {
      *(first_arg++) = LOWER(*argument);
    } else if (mode == 1) {
      *(first_arg++) = *argument;
    } else if (mode == 2) {
      *(first_arg++) = UPPER(*argument);
    }
    argument++;
  }

  *(first_arg) = '\0';

  return (argument);
}


/*
 * determine if a given string is an abbreviation of another
 */
int is_abbrev(const char *arg1, const char *arg2)
{
  if (!*arg1)
    return (0);

  for (; *arg1 && *arg2; arg1++, arg2++)
    if (*arg1 == '*')
      arg2--;
    else
      if (LOWER(*arg1) != LOWER(*arg2))
        return (0);

  if (!*arg1 || *arg1 == '*')
    return (1);
  else
    return (0);

}

int numbydir(char dir)
{
    if (LOWER(dir) == 'n')
	return NORTH;
    if (LOWER(dir) == 's')
	return SOUTH;
    if (LOWER(dir) == 'e')
	return EAST;
    if (LOWER(dir) == 'w')
	return WEST;
    if (LOWER(dir) == 'u')
	return UP;
    if (LOWER(dir) == 'd')
	return DOWN;

    return -1;
}

char dirbynum(int dir)
{
  switch (dir) {
	case  NORTH :
                return 'n';
		break;
	case  SOUTH :
                return 's';
		break;
	case  EAST :
                return 'e';
		break;
	case  WEST :
                return 'w';
		break;
	case  UP :
                return 'u';
		break;
	case  DOWN :
                return 'd';
		break;
  }


  return -1;
}

int reversenum(int num)
{
    if (num == NORTH)
	return SOUTH;
    if (num == SOUTH)
	return NORTH;
    if (num == EAST)
	return WEST;
    if (num == WEST)
	return EAST;
    if (num == UP)
	return DOWN;
    if (num == DOWN)
	return UP;
    return -1;
}

void send_to_user(const char *messg, ...)
{
  va_list args;

  if (messg == NULL)
    return;

  va_start(args, messg);
  write_to_channel(0, messg, args);
  va_end(args);
}


void send_prompt() {
	proxy->send_line_to_user( (const char *) engine->getPrompt() );
}


void send_to_mud(const char *messg, ...)
{
  va_list args;

  if (messg == NULL)
    return;

  va_start(args, messg);
  write_to_channel(1, messg, args);
  va_end(args);
}

/* mode 0 - to user, mode 1 to mud */
int write_to_channel(int mode, const char *format, va_list args)
{
  char txt[MAX_STR_LEN*2];
  int size;

  size = vsnprintf(txt, sizeof(txt), format, args);
  if (mode == 0)
    proxy->send_line_to_user(txt);
  else if (mode == 1)
    proxy->send_line_to_mud(txt);

  return size;
}


// latin1 to 7-bit Ascii
void latinToAscii(QByteArray &text)
{
    const unsigned char table[]= {
/*192*/   'A',
          'A',
          'A',
          'A',
          'A',
          'A',
          'A',
          'C',
          'E',
          'E',
          'E',
          'E',
          'I',
          'I',
          'I',
          'I',
          'D',
          'N',
          'O',
          'O',
          'O',
          'O',
          'O',
          'x',
          'O',
          'U',
          'U',
          'U',
          'U',
          'Y',
          'b',
          'B',
          'a',
          'a',
          'a',
          'a',
          'a',
          'a',
          'a',
          'c',
          'e',
          'e',
          'e',
          'e',
          'i',
          'i',
          'i',
          'i',
          'o',
          'n',
          'o',
          'o',
          'o',
          'o',
          'o',
          ':',
          'o',
          'u',
          'u',
          'u',
          'u',
          'y',
          'b',
          'y'
    };
    unsigned char ch;
    int pos;

    for (pos = 0; pos <= text.length(); pos++) {
        ch = text[pos];
        if (ch > 128) {
            if (ch < 192)
                ch = 'z';
            else
                ch = table[ ch - 192 ];

            text[pos] = ch;
        }
    }
}
