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



#ifndef STACKSMANAGER_H
#define STACKSMANAGER_H

#include <vector>
#include "Map/CRoom.h"
#include "Map/CRoomManager.h"

class CStacksManager {
private:

  std::vector<CRoom *> stacka;
  std::vector<CRoom *> stackb;

  std::vector<CRoom *> *sa;
  std::vector<CRoom *> *sb;

  unsigned int  mark[MAX_ROOMS];
  unsigned int  turn;
public:
  unsigned int amount() { return sa->size(); }
  unsigned int next()    { return sb->size(); }

  CRoom * first();
  CRoom * nextFirst();

  void swap();
  CStacksManager();
  void reset();

  CRoom * get(unsigned int i);

  CRoom * getNext(unsigned int i);

  void put(unsigned int id);
  void put(CRoom *r);
  void removeRoom(unsigned int id);    /* swaps */

  /* DEBUG */
  void printStacks();

  void getCurrent(char *);

};

extern class CStacksManager stacker;


#endif
