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


#include "renderer.h"
#include "defines.h"
#include "CDispatcher.h"
#include "proxy.h"
#include "utils.h"
#include "CStacksManager.h"
#include "mainwindow.h"
#include "CRoomManager.h"

class CStacksManager stacker;


CRoom * CStacksManager::first()  
{ 
    if (sa->size() == 0)
        return NULL;
    return (*sa)[0]; 
}

CRoom * CStacksManager::nextFirst()  
{ 
    if (sb->size() == 0)
        return NULL;
    return (*sb)[0]; 
}

void CStacksManager::getCurrent(char *str)
{
  if (sa->size() == 0) {
    sprintf(str, "NO_SYNC");
    return;
  }
  
  if (sa->size() > 1) {
    sprintf(str, " MULT ");
    return;
  }
  
  sprintf(str, "%i", ((*sa)[0])->id );
}


void CStacksManager::printStacks()
{
    unsigned int i;

    send_to_user(" Possible positions : \n");
    if (sa->size() == 0)
    	send_to_user(" Current position is unknown!\n");
    for (i = 0; i < sa->size(); i++) {
    	send_to_user(" %i\n", (*sa)[i]->id);
    }
}

void CStacksManager::removeRoom(unsigned int id)
{
  unsigned int i;

  for (i = 0; i < sa->size(); i++) 
    if (stacker.get(i)->id != id) 
      stacker.put( stacker.get(i) );
  stacker.swap();
}

void CStacksManager::put(CRoom *r)
{
    if (mark[r->id] == turn)
    	return;
    sb->push_back(r);
    mark[r->id] = turn;
}


void CStacksManager::put(unsigned int id)
{
    put(Map.getRoom(id));
}


CRoom *CStacksManager::get(unsigned int i)
{
    return (*sa)[i];
}

CRoom *CStacksManager::getNext(unsigned int i)
{
    return (*sb)[i];
}


CStacksManager::CStacksManager()
{
    reset();
}

void CStacksManager::reset()
{
    sa = &stacka;
    sb = &stackb;
    sa->clear();
    sb->clear();
    memset(mark, 0, MAX_ROOMS * sizeof(mark[0]) );
    turn = 1;
    swap();
}

void CStacksManager::swap()
{
    vector<CRoom *> *t;

    turn++;
    if (turn == 0) {
        memset(mark, 0, MAX_ROOMS * sizeof(mark[0]) );
        turn = 1;
    }
    
    t = sa;
    sa = sb;
    sb = t;
    sb->clear();
  
    if (renderer_window)
      renderer_window->update_status_bar();
}
