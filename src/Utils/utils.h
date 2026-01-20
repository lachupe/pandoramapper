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

/* Utilities, String functions and stuff */
#ifndef UTILS_H
#define UTILS_H

#include <cctype>

class QByteArray;

// Modern C++ inline functions replacing legacy macros
// These provide type safety and better debugging

inline constexpr char toLower(char c) noexcept
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + ('a' - 'A')) : c;
}

inline constexpr char toUpper(char c) noexcept
{
    return (c >= 'a' && c <= 'z') ? static_cast<char>(c + ('A' - 'a')) : c;
}

inline constexpr const char *onOff(bool flag) noexcept
{
    return flag ? "ON" : "OFF";
}

inline constexpr const char *yesNo(bool flag) noexcept
{
    return flag ? "YES" : "NO";
}

// Bit manipulation functions with type safety
template <typename T, typename U>
inline constexpr bool isSet(T flag, U bit) noexcept
{
    return (flag & bit) != 0;
}

template <typename T, typename U>
inline constexpr T &setBit(T &var, U bit) noexcept
{
    var |= bit;
    return var;
}

template <typename T, typename U>
inline constexpr T &removeBit(T &var, U bit) noexcept
{
    var &= ~bit;
    return var;
}

template <typename T, typename U>
inline constexpr T &toggleBit(T &var, U bit) noexcept
{
    var ^= bit;
    return var;
}

// Legacy macro compatibility (for gradual migration)
#define LOWER(c) toLower(c)
#define UPPER(c) toUpper(c)
#define ON_OFF(flag) onOff(flag)
#define YES_NO(flag) yesNo(flag)
#define IS_SET(flag, bit) isSet(flag, bit)
#define SET_BIT(var, bit) setBit(var, bit)
#define REMOVE_BIT(var, bit) removeBit(var, bit)
#define TOGGLE_BIT(var, bit) toggleBit(var, bit)

#define MAX_INPUT_LENGTH 1024 /* Max length per *line* of input */

struct boolean_struct
{
    const char *name;
    int state;
};

#define DEBUG_GENERAL (1 << 0)
#define DEBUG_ANALYZER (1 << 1)
#define DEBUG_SYSTEM (1 << 2)
#define DEBUG_CONFIG (1 << 3)
#define DEBUG_DISPATCHER (1 << 4)
#define DEBUG_PROXY (1 << 5)
#define DEBUG_RENDERER (1 << 6)
#define DEBUG_ROOMS (1 << 7)
#define DEBUG_STACKS (1 << 8)
#define DEBUG_TREE (1 << 9)
#define DEBUG_USERFUNC (1 << 10)
#define DEBUG_XML (1 << 11)
#define DEBUG_INTERFACE (1 << 12)
#define DEBUG_SPELLS (1 << 13)
#define DEBUG_GROUP (1 << 14)

/* special flag */
#define DEBUG_TOUSER (1 << 15) /* send the message to user also */

extern const char *exitnames[];

struct debug_data_struct
{
    const char *name;
    const char *title;
    const char *desc;
    unsigned int flag;
    int state;
};

void print_debug(unsigned int flag, const char *messg, ...);

extern struct debug_data_struct debug_data[];
extern char timer_ken[MAX_INPUT_LENGTH];
extern double timer_now;

extern const boolean_struct input_booleans[];
extern const char *exits[];

int is_integer(char *p);
char *skip_spaces(const char *str);
char *next_space(char *str);
char *one_argument(char *argument, char *first_arg, int mode);
int is_abbrev(const char *arg1, const char *arg2);
int reversenum(int num);
char dirbynum(int dir);
int numbydir(char dir);
void send_prompt();
void send_to_user(const char *messg, ...);
void send_to_mud(const char *messg, ...);
int get_input_boolean(char *input);
int parse_dir(char *dir);
void basic_mud_log(const char *format, ...);
int MIN(int a, int b);

void latinToAscii(QByteArray &text);

#endif
