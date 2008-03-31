#ifndef ENGINE_H
#define ENGINE_H

#include <QObject>

#include "CRoom.h"
#include "CEvent.h"

class CEngine : public QObject {
Q_OBJECT

  /* flags */
    bool mapping;                 /* mapping is On/OFF */
    bool mgoto;
    int   nameMatch;
    int   descMatch;
    
    CRegion  *users_region;
    CRegion  *last_region;
    
    QByteArray last_name;
    QByteArray last_desc;
    QByteArray last_exits;
    QByteArray last_prompt;
    char 	   last_terrain;	
    QByteArray last_movement;
  
    PipeManager  eventPipe;
    Event        event;

    void parseEvent();
    void tryAllDirs();
    void tryDir();
    void tryLook();
    void swap();
    void resync();
    void mappingOff();
    
    bool testRoom(CRoom *room);

    void mapCurrentRoom(CRoom *room, int dir);

public:
    CEngine();
    ~CEngine();

    CRoom *addedroom;	/* fresh added room */
    
    void addEvent(Event e) { eventPipe.addEvent(e); }

    int parseCommandLine(char cause, char result, char *line);

    void exec();       
    
    int checkRoomDesc();
    void angryLinker(CRoom *r);
    void printStacks();
    
    QByteArray getRoomName() { return last_name; }
    QByteArray getDesc() { return last_desc; }
    QByteArray getExits() { return last_exits; }
    QByteArray getPrompt() { return last_prompt; }
    char getTerrain() { return last_terrain; }
    QByteArray getLastMovement() { return last_movement; }
    
    void setRoomname(QByteArray s) { last_name = s; }
    void setDesc(QByteArray s) { last_desc = s; }
    void setExits(QByteArray s) { last_exits = s; }
    void setPrompt(QByteArray s) { last_prompt = s; }
    void setTerrain(char c) { last_terrain = c; }

    bool isMapping() { return mapping; }
    void setMapping(bool b) { mapping = b; }
    
    void setMgoto(bool b) { mgoto = b; }
    bool isMgoto() { return mgoto; }
    bool empty() { return eventPipe.isEmpty(); };                      /* are pipes empty? */
    void clear();                      /* clears events pipes */
    
    void set_users_region(CRegion *reg);
    void set_last_region(CRegion *reg);
    CRegion *get_users_region();
    CRegion *get_last_region();

    void updateRegions();

    void resetAddedRoomVar() { addedroom = NULL; }    
public slots:
    void slotRunEngine();
};

extern class CEngine *engine;

#endif

