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

#include <QVector>
#include <QObject>
#include <QThread>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>

#include "defines.h"

#include "Map/CRoom.h"
#include "Map/CRegion.h"
#include "Gui/CSelectionManager.h"

class CPlane;
class CSquare;

struct LocalSpace
{
    int id;
    QByteArray name;
    float portalX;
    float portalY;
    float portalZ;
    float portalW;
    float portalH;
    bool hasPortal;
    bool hasBounds;
    float minX;
    float maxX;
    float minY;
    float maxY;
    float minZ;
    float maxZ;
};

class CRoomManager : public QObject
{
    Q_OBJECT

    QList<CRegion *> regions;
    QVector<CRoom *> rooms; /* rooms */
    CRoom *ids[MAX_ROOMS];  /* array of pointers */
    QVector<LocalSpace> localSpaces;
    int nextLocalSpaceId;

    CPlane *planes; /* planes/levels of the map, sorted by the Z coordinate, lower at first */

    bool blocked;

  public:
    CRoomManager();
    virtual ~CRoomManager();
    void init();
    void reinit(); /* reinitializer/utilizer */

    QVector<CRoom *> getRooms() { return rooms; }
    CPlane *getPlanes() { return planes; }

    unsigned int next_free; /* next free id */
    unsigned int oneway_room_id;

    unsigned int size() { return rooms.size(); }

    CSelectionManager selections;

    /* plane support */
    void addToPlane(CRoom *room);
    void removeFromPlane(CRoom *room);
    void expandPlane(CPlane *plane, CRoom *room);

    void addRoom(CRoom *room);

    inline CRoom *getRoom(unsigned int id)
    {
        if (id < MAX_ROOMS)
            return ids[id];
        else
            return nullptr;
    }

    inline QByteArray getName(unsigned int id)
    {
        if (ids[id])
            return (*(ids[id])).getName();
        return "";
    }

    int tryMergeRooms(CRoom *room, CRoom *copy, int j);
    bool isDuplicate(CRoom *addedroom);

    void fixFreeRooms();
    CRegion *getRegionByName(QByteArray name);
    bool addRegion(QByteArray name);
    void addRegion(CRegion *reg);
    void rebuildRegion(CRegion *reg);

    void sendRegionsList();
    int addLocalSpace(QByteArray name);
    int addLocalSpaceWithId(QByteArray name, int id);
    LocalSpace *getLocalSpace(int id);
    QVector<LocalSpace *> getLocalSpaces();
    bool setRegionLocalSpace(CRegion *region, int localSpaceId);
    bool setLocalSpacePortal(int id, float x, float y, float z, float w, float h);
    void updateLocalSpaceBounds();

    QList<CRegion *> getAllRegions();

    void deleteRoom(CRoom *r, int mode);
    void smallDeleteRoom(CRoom *r);

    QList<int> searchNames(QString s, Qt::CaseSensitivity cs);
    QList<int> searchDescs(QString s, Qt::CaseSensitivity cs);
    QList<int> searchNotes(QString s, Qt::CaseSensitivity cs);
    QList<int> searchExits(QString s, Qt::CaseSensitivity cs);

    CRoom *findDuplicateRoom(CRoom *orig);

    void loadMap(QString filename);
    void saveMap(QString filename);
    void clearAllSecrets();

    void setBlocked(bool b) { blocked = b; }
    bool isBlocked() { return blocked; }
};

extern class CRoomManager Map; /* room manager */

#endif
