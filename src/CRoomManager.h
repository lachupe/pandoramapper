#ifndef ROOMS_H 
#define ROOMS_H 

#include "defines.h"
#include "CRoom.h"
#include "CRegion.h"
#include "CSelectionManager.h"

#include <QVector>
#include <QObject>

class CPlane;
class CSquare;


#define MAX_ROOMS       30000		/* maximal amount of rooms */

class CRoomManager : public QObject {
	Q_OBJECT
	
    QList<CRegion *>    regions;
public:
    QVector<CRoom *> rooms;   		/* rooms */
    CRoom *ids[MAX_ROOMS];	/* array of pointers */
    CSelectionManager   selections;
    
    
    unsigned int size()  { return rooms.size(); }
    unsigned int next_free; 	/* next free id */
    
    unsigned int oneway_room_id;
    
    /* plane support */  
    CPlane        *planes;        /* planes/levels of the map, sorted by the Z coordinate, lower at first */
    void          addToPlane(CRoom *room);
    void          removeFromPlane(CRoom *room);
    void          expandPlane(CPlane *plane, CRoom *room);
    
    CRoomManager();
    virtual ~CRoomManager();
    void init();
    void reinit();			/* reinitializer/utilizer */
    
    void addRoomNonsorted(CRoom *room);   /* use only at loading */
    void addRoom(CRoom *room);
    
    
    inline CRoom * getRoom(unsigned int id)        { 
            if (id < MAX_ROOMS) 
                return ids[id]; 
            else 
                return NULL; 
    }

    inline QByteArray getName(unsigned int id)  { if (ids[id]) return (*(ids[id])).getName(); return "";}
        
    int tryMergeRooms(CRoom *room, CRoom *copy, int j);

    void fixFreeRooms();
    CRegion *getRegionByName(QByteArray name);
    bool addRegion(QByteArray name);
    void addRegion(CRegion *reg);
    
    void sendRegionsList();  
    
    QList<CRegion *> getAllRegions();
    
    
    void deleteRoom(CRoom *r, int mode);  /* user interface function */
    void smallDeleteRoom(CRoom *r);  /* user interface function */

    QList<int> searchNames(QString s, Qt::CaseSensitivity cs);
    QList<int> searchDescs(QString s, Qt::CaseSensitivity cs);
    QList<int> searchNotes(QString s, Qt::CaseSensitivity cs);
    QList<int> searchExits(QString s, Qt::CaseSensitivity cs);

    
    void loadMap(QString filename);
    void saveMap(QString filename);
public slots:

signals:

};

extern class CRoomManager Map;/* room manager */

#endif

