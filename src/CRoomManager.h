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

#ifndef ROOMSMANAGER_H
#define ROOMSMANAGER_H

#include "defines.h"
#include "CRoom.h"
#include "CRegion.h"
#include "CSelectionManager.h"

#include <QVector>
#include <QObject>
#include <QThread>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>

class CPlane;
class CSquare;

class CRoomManager : public QObject {
	Q_OBJECT
	QReadWriteLock mapLock;

	QList<CRegion *>    regions;
    QVector<CRoom* > rooms;   		/* rooms */
    CRoom* ids[MAX_ROOMS];	/* array of pointers */

    CPlane        *planes;        /* planes/levels of the map, sorted by the Z coordinate, lower at first */

    inline QByteArray getNameUnlocked(unsigned int id)  {
    	if (ids[id]) return (*(ids[id])).getName(); return "";
    }

    inline CRoom*  getRoomUnlocked(unsigned int id)        {
   	    if (id < MAX_ROOMS)
            return ids[id];
        else
            return NULL;
    }

    void deleteRoomUnlocked(CRoom* r, int mode);  /* user interface function */
    void smallDeleteRoomUnlocked(CRoom* r);  /* user interface function */

public:
    CRoomManager();
    virtual ~CRoomManager();
    void init();
    void reinit();			/* reinitializer/utilizer */
    void lockForRead() {
    	mapLock.lockForRead();
    }
    void lockForWrite() {
    	mapLock.lockForWrite();
    }
    void unlock() {
    	mapLock.unlock();
    }
    bool tryLockForRead() { return mapLock.tryLockForRead(); }
    bool tryLockForWrite() { return mapLock.tryLockForWrite(); }

    // make sure you LOCK before you use those lists ... !
    QVector<CRoom* > getRooms() { return rooms; }
    CPlane* getPlanes() { return planes; }

    unsigned int next_free; 	/* next free id */
    unsigned int oneway_room_id;

    unsigned int size()  { return rooms.size(); }


    CSelectionManager   selections;

    /* plane support */
    void          addToPlane(CRoom* room);
    void          removeFromPlane(CRoom* room);
    void          expandPlane(CPlane *plane, CRoom* room);


    void addRoom(CRoom* room);
    void addRoomNonsorted(CRoom* room);   /* use only at loading, it's unlocked! */



    inline CRoom* getRoom(unsigned int id)        {
    	QReadLocker locker(&mapLock);
    	return getRoomUnlocked(id);
    }

    inline QByteArray getName(unsigned int id)  {
    	QReadLocker locker(&mapLock);
    	return getNameUnlocked(id); // this GOT to be inlined!
    }

    int tryMergeRooms(CRoom* room, CRoom* copy, int j);
    bool isDuplicate(CRoom* addedroom);

    void fixFreeRooms();
    CRegion* getRegionByName(QByteArray name);
    bool addRegion(QByteArray name);
    void addRegion(CRegion *reg);

    void sendRegionsList();

    QList<CRegion *> getAllRegions();

    void deleteRoom(CRoom* r, int mode) { lockForWrite(); deleteRoomUnlocked(r, mode); unlock(); }
    void smallDeleteRoom(CRoom* r) { lockForWrite(); smallDeleteRoomUnlocked(r); unlock(); }

    QList<int> searchNames(QString s, Qt::CaseSensitivity cs);
    QList<int> searchDescs(QString s, Qt::CaseSensitivity cs);
    QList<int> searchNotes(QString s, Qt::CaseSensitivity cs);
    QList<int> searchExits(QString s, Qt::CaseSensitivity cs);


    CRoom* findDuplicateRoom(CRoom *orig);

    void loadMap(QString filename);
    void saveMap(QString filename);
    void clearAllSecrets();
};

extern class CRoomManager Map;/* room manager */

#endif

