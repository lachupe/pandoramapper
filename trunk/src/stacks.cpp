
#include "renderer.h"
#include "defines.h"
#include "dispatch.h"
#include "forwarder.h"
#include "utils.h"
#include "stacks.h"
#include "mainwindow.h"

class stackmanager stacker;


CRoom * stackmanager::first()  
{ 
    if (sa->size() == 0)
        return NULL;
    return (*sa)[0]; 
}

CRoom * stackmanager::nextFirst()  
{ 
    if (sb->size() == 0)
        return NULL;
    return (*sb)[0]; 
}

void stackmanager::getCurrent(char *str)
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


void stackmanager::printStacks()
{
    unsigned int i;

    send_to_user(" Possible positions : \n");
    if (sa->size() == 0)
	send_to_user(" Current position is unknown!\n");
    for (i = 0; i < sa->size(); i++) {
	send_to_user(" %i\n", (*sa)[i]->id);
    }
}

void stackmanager::removeRoom(unsigned int id)
{
  unsigned int i;

  for (i = 0; i < sa->size(); i++) 
    if (stacker.get(i)->id != id) 
      stacker.put( stacker.get(i) );
  stacker.swap();
}

void stackmanager::put(CRoom *r)
{
    if (mark[r->id] == turn)
	return;
    sb->push_back(r);
    mark[r->id] = turn;
}


void stackmanager::put(unsigned int id)
{
    put(Map.getRoom(id));
}


CRoom *stackmanager::get(unsigned int i)
{
    return (*sa)[i];
}

CRoom *stackmanager::getNext(unsigned int i)
{
    return (*sb)[i];
}


stackmanager::stackmanager()
{
    reset();
}

void stackmanager::reset()
{
    sa = &stacka;
    sb = &stackb;
    sa->clear();
    sb->clear();
    memset(mark, 0, MAX_ROOMS);
    turn = 1;
    swap();
}

void stackmanager::swap()
{
    vector<CRoom *> *t;

    turn++;
    if (turn == 0) {
        memset(mark, 0, MAX_ROOMS);
        turn = 1;
    }
    
    t = sa;
    sa = sb;
    sb = t;
    sb->clear();
  
    if (renderer_window)
      renderer_window->update_status_bar();
}
