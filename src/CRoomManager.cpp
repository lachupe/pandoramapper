/* Implementation of room manager of Pandora Project (c) Azazello 2003 */
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <QDateTime>
#include <vector>
//using namespace std;

#include "defines.h"
#include "CConfigurator.h"

#include "CRoomManager.h"
#include "utils.h"
#include "CTree.h"
#include "CStacksManager.h"
#include "CDispatcher.h"
#include "mainwindow.h"


class CRoomManager Map;

CRoomManager::~CRoomManager()
{
}

//
//void CRoomManager::loadMap(QString filename)
//{
//	xmlReadMap(filename);
//}
//
//void CRoomManager::saveMap(QString filename)
//{
//	xmlWriteMap(filename);
//}
//


void CRoomManager::clearAllSecrets()
{
    bool mark[MAX_ROOMS];
    CRoom *r;
    unsigned int i;
    unsigned int z;

    // "wave" over all rooms reacheable over non-secret exits.         
    memset(mark, 0, MAX_ROOMS);
    stacker.reset();
    stacker.put(1);
    stacker.swap();

    
    lockForWrite();
    while (stacker.amount() != 0) {
        for (i = 0; i < stacker.amount(); i++) {
            r = stacker.get(i);
            mark[r->id] = true;
            for (z = 0; z <= 5; z++) 
                if (r->isConnected(z) && mark[ r->exits[z]->id  ] != true  && r->isDoorSecret(z) != true  ) 
                    stacker.put(r->exits[z]->id);
        }
        stacker.swap();
    }

    // delete all unreached rooms    
    for (i = 0; i < MAX_ROOMS; i++) {
        r = getRoomUnlocked( i );
        if (r == NULL)
            continue;
        if (r) {
            if (mark[r->id] == false) {
                deleteRoomUnlocked(r, 0);
                continue;        
            }
        }
        
    }

    QVector<CRoom *> rooms = getRooms();
    // roll over all still accessible rooms and delete the secret doors if they are still left in the database
    for (i = 0; i < size(); i++) {
        r = rooms[i];
        if (r) {
            for (z = 0; z <= 5; z++) {
                if ( r->isDoorSecret(z) == true ) {
                    print_debug(DEBUG_ROOMS,"Secret door was still in database...\r\n");
                    r->removeDoor(z);
                }
            }
        }
        
    }
    
    unlock();
}


/*------------ merge_rooms ------------------------- */
int CRoomManager::tryMergeRooms(CRoom *r, CRoom *copy, int j)
{
  unsigned int i;
  CRoom *p;
  
  QWriteLocker locker(&mapLock);
  
  if (j == -1) {
    /* oneway ?! */
    print_debug(DEBUG_ROOMS, "fixing one way in previous room, repointing at merged room");
    
     p = getRoom(oneway_room_id);
     for (i = 0; i <= 5; i++)
         if (p->isExitLeadingTo(i, copy) == true) 
             p->setExit(i, r);
     
    smallDeleteRoomUnlocked(copy);


    stacker.put(r);
    return 1;
  }
  if ( r->isExitUndefined(j) ) {
    r->setExit(j, copy->exits[j] );
                      
    p = copy->exits[j] ;
    if (p->isExitLeadingTo( reversenum(j), copy) == true)
        p->setExit( reversenum(j), r);
        
    smallDeleteRoomUnlocked(copy);

    stacker.put(r);
    return 1;
  }
  return 0;
}

/* ------------ fixfree ------------- */
void CRoomManager::fixFreeRooms()
{
    unsigned int i;

    for (i = 1; i < MAX_ROOMS; i++)
	if (ids[i] == NULL) {
	    next_free = i;
	    return;
	}

    print_debug(DEBUG_ROOMS, "roomer: error - no more space for rooms in ids[] array! reached limit\n");
    exit(1);
}

/* ------------ addroom --------------*/
void CRoomManager::addRoomNonsorted(CRoom *room)
{
	if (ids[room->id] != NULL) {
        print_debug(DEBUG_ROOMS, "Error while adding new element to database! This id already exists!\n");
        exit(1);
    }
    
    
    if (ids[room->id] != NULL)
        return;
    rooms.push_back(room);
    ids[room->id] = room;	/* add to the first array */
    NameMap.addName(room->getName(), room->id);	/* update name-searhing engine */
    
    
    fixFreeRooms();
    addToPlane(room);
}

void CRoomManager::addRoom(CRoom *room)
{
	QWriteLocker locker(&mapLock);
	addRoomNonsorted(room);
}
/* ------------ addroom ENDS ---------- */

/*------------- Constructor of the room manager ---------------*/
CRoomManager::CRoomManager()
{
	init();
}


void CRoomManager::init()
{
	QWriteLocker locker(&mapLock);

    print_debug(DEBUG_ROOMS,"Roommanager INIT.\r\n");

    next_free = 1;

    print_debug(DEBUG_ROOMS, "In roomer.init()");
  
    /* adding first (empty) root elements to the lists */
    rooms.clear();
    regions.clear();
    
    
    
    CRegion *region = new CRegion;
    region->setName("default");
    
    regions.push_back(region);
    
    
    ids[0] = NULL;
    planes = NULL;
}

/*------------- Constructor of the room manager ENDS  ---------------*/

CRegion *CRoomManager::getRegionByName(QByteArray name)
{
	// TODO: threadsafety the class regions QMutexLocker locker(mapLock);

	CRegion    *region;
    for (int i=0; i < regions.size(); i++) {
        region = regions[i];
        if (region->getName() == name) 
            return region;
    }
    return NULL;
}

bool CRoomManager::addRegion(QByteArray name)
{
    CRegion    *region;
	// TODO: threadsafety the class regions QMutexLocker locker(mapLock);

    if (getRegionByName(name) == false) {
        region = new CRegion();
        region->setName( name );
        regions.push_back(region);
        return true;
    } else {
        return false;
    }
    
}

void CRoomManager::addRegion(CRegion *reg)
{
	// TODO: threadsafety the class regions QMutexLocker locker(mapLock);

	if (reg != NULL) 
        regions.push_back(reg);
}


void CRoomManager::sendRegionsList()
{
    CRegion    *region;
	// TODO: threadsafety the class regions QMutexLocker locker(mapLock);

    
    send_to_user( "Present regions: \r\n");
    for (int i=0; i < regions.size(); i++) {
        region = regions[i];
        send_to_user("  %s\r\n", (const char *) region->getName() );
    }
    

}

QList<CRegion *> CRoomManager::getAllRegions()
{
	// TODO: threadsafety the class regions QMutexLocker locker(mapLock);
	return regions;
}


/* -------------- reinit ---------------*/
void CRoomManager::reinit()
{
	QWriteLocker locker(&mapLock);

	next_free = 1;
    {
        CPlane *p, *next;
        
        print_debug(DEBUG_ROOMS,"Resetting Cplane structures ... \r\n");
        p = planes;
        while (p) {
            next = p->next;
            delete p;
            p = next;
        }
        planes = NULL;
    }

    memset(ids, 0, MAX_ROOMS * sizeof (CRoom *) );        
/*
    for (i = 0; i < MAX_ROOMS; i++)
      ids[i] = NULL;
    ids[0] = rooms;
*/
    
    rooms.clear();     
    NameMap.reinit();
}

/* -------------- reinit ENDS --------- */

/* ------------ delete_room --------- */
/* mode 0 - remove all links in other rooms together with exits and doors */
/* mode 1 - keeps the doors and exits in other rooms, but mark them as undefined */
void CRoomManager::deleteRoomUnlocked(CRoom *r, int mode)
{
    int k;
    int i;
    
    if (r->id == 1) {
	print_debug(DEBUG_ROOMS,"Cant delete base room!\n");
	return;
    }

    /* have to do this because of possible oneways leading in */
    for (i = 0; i < rooms.size(); i++) 
	for (k = 0; k <= 5; k++)
	    if (rooms[i]->isExitLeadingTo(k, r) == true) {
                if (mode == 0) {
                    rooms[i]->removeExit(k);
                } else if (mode == 1) {
                    rooms[i]->setExitUndefined(k);
                }
	    }

}

/* --------- _delete_room ENDS --------- */

/* ------------ small_delete_room --------- */
void CRoomManager::smallDeleteRoomUnlocked(CRoom *r)
{
	if (r->id == 1) {
		print_debug(DEBUG_ROOMS,"ERROR (!!): Attempted to delete the base room!\n");
		return;
    }
    removeFromPlane(r);
    stacker.removeRoom(r->id);
    selections.unselect(r->id);
    if (engine->addedroom == r)
        engine->resetAddedRoomVar();

    renderer_window->renderer->deletedRoom = r->id;

    
    int i;
    ids[ r->id ] = NULL;
    
    for (i = 0; i < rooms.size(); i++)
        if (rooms[i]->id == r->id ) {
            print_debug(DEBUG_ROOMS,"Deleting the room from rooms vector.\r\n");
            rooms.remove(i);
            break;
        }
    
    delete r;    


    fixFreeRooms();
    toggle_renderer_reaction();
}
/* --------- small_delete_room ENDS --------- */

// this function is only called as result of CRoom.setZ and 
// addToPlace() functions. 
// addToPlane is protected by CRoomManager locks 
// CRoom.setZ() stays as open issue aswell as the whole
// set of writing CRoom functions ...
void CRoomManager::removeFromPlane(CRoom *room)
{
    CPlane *p;
   
    if (planes == NULL)
    	return;
    
    p = planes;
    while (p->z != room->getZ()) {
        if (!p) {
            print_debug(DEBUG_ROOMS," FATAL ERROR. remove_fromplane() the given has impossible Z coordinate!\r\n");
            return;     /* no idea what happens next ... */
        }
        p = p->next;
    }

    p->squares->remove(room);
}

void CRoomManager::expandPlane(CPlane *plane, CRoom *room)
{
    CSquare *p, *new_root = NULL;
    int size;
    
    p = plane->squares;
    
    while ( p->isInside(room) != true ) {
        /* plane fork/expanding cycle */
        
        size = p->rightx - p->leftx;
        
        switch ( p->getMode(room) )
        {
            case  CSquare::Left_Upper:
                new_root = new CSquare(p->leftx - size, p->lefty + size, p->rightx, p->righty);
                new_root->subsquares[ CSquare::Right_Lower ] = p;
                break;
            case  CSquare::Right_Upper:
                new_root = new CSquare(p->leftx,  p->lefty + size, p->rightx + size, p->righty);
                new_root->subsquares[ CSquare::Left_Lower ] = p;
                break;
            case  CSquare::Right_Lower:
                new_root = new CSquare(p->leftx,  p->lefty, p->rightx + size, p->righty - size);
                new_root->subsquares[ CSquare::Left_Upper ] = p;
                break;
            case  CSquare::Left_Lower:
                new_root = new CSquare(p->leftx - size,  p->lefty, p->rightx , p->righty - size);
                new_root->subsquares[ CSquare::Right_Upper ] = p;
                break;
        }
        
        p = new_root;
    }    

/*    printf("Ok, it fits. Adding!\r\n");
*/    
    p->add(room);
    plane->squares = p;
}


void  CRoomManager::addToPlane(CRoom *room)
{
    CPlane *p, *prev, *tmp;

    // is protected by CRoomManager locker
    //	QMutexLocker locker(mapLock);
    
    
    if (planes == NULL) {
        planes = new CPlane(room);
        return;
    }
    
    p = planes;
    prev = NULL;
    while (p) {
        if (room->getZ() < p->z) {
            tmp = new CPlane(room);
            tmp->next = p;
            if (prev)
                prev->next = tmp;
            else 
                planes = tmp;
            return;
        }
        /* existing plane with already set borders */
        if (room->getZ() == p->z) {
            expandPlane(p, room);
            return;
        }
        prev = p;
        p = p->next;
    }
    
    /* else .. this is a plane with highest yet found Z coordinate */
    /* we add it to the end of the list */
    prev->next = new CPlane(room);
}

QList<int> CRoomManager::searchNames(QString s, Qt::CaseSensitivity cs)
{
    QList<int> results;

	QReadLocker locker(&mapLock);

    for (int i = 0; i < rooms.size(); i++) {
        if (QString(rooms[i]->getName()).contains(s, cs)) {
            results << rooms[i]->id;
        }
    }

    return results;
}

QList<int> CRoomManager::searchDescs(QString s, Qt::CaseSensitivity cs)
{
    QList<int> results;

    QReadLocker locker(&mapLock);

    for (int i = 0; i < rooms.size(); i++) {
        if (QString(rooms[i]->getDesc()).contains(s, cs)) {
            results << rooms[i]->id;
        }
    }

    return results;
}

QList<int> CRoomManager::searchNotes(QString s, Qt::CaseSensitivity cs)
{
    QList<int> results;

    QReadLocker locker(&mapLock);

	for (int i = 0; i < rooms.size(); i++) {
        if (QString(rooms[i]->getNote()).contains(s, cs)) {
            results << rooms[i]->id;
        }
    }

    return results;
}

QList<int> CRoomManager::searchExits(QString s, Qt::CaseSensitivity cs)
{
    QList<int> results;

    QReadLocker locker(&mapLock);

    for (int i = 0; i < rooms.size(); i++) {
        for (int j = 0; j <= 5; j++) {
            if (rooms[i]->isDoorSecret( j ) == true) {
                if (QString(rooms[i]->getDoor(j)).contains(s, cs)) {
                    results << rooms[i]->id;
                }
            }
        }
    }

    return results;
}
