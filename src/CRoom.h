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

//
// C++ Interface: croom
//
// Description: 
//
//
// Author: Azazello <aza@alpha>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CROOM_H
#define CROOM_H

#include "defines.h"
#include "CRegion.h"

#include <QByteArray>

//#define EXIT_UNDEFINED  (MAX_ROOMS+1)
//#define EXIT_DEATH      (MAX_ROOMS+2)


struct room_flag_data {
  QByteArray name;
  QByteArray xml_name;
  unsigned int flag;
};

extern const struct room_flag_data room_flags[];

//#define MAX_N   ( MAX_LINES_DESC * 80 )
//#define MAX_M   ( MAX_LINES_DESC * 80 )


class Strings_Comparator {
    private:
        static const int MAX_N = MAX_LINES_DESC * 80;
        static const int MAX_M = MAX_LINES_DESC * 80;
        int D[ MAX_N ] [MAX_M ];
    public:
        int compare(QByteArray pattern, QByteArray text);
        int compare_with_quote(QByteArray str, QByteArray text, int quote);
        int strcmp_roomname(QByteArray name, QByteArray text);
        int strcmp_desc(QByteArray name, QByteArray text);

};

extern Strings_Comparator comparator;


class CRoom {
    unsigned int    flags;          
    QByteArray      name; 			/* POINTER to the room name */ 
    QByteArray      note; 			/* note, if needed, additional info etc */
    QByteArray      noteColor;      /* note color in this room */
    QByteArray      desc;			/* descrition */
    char                  sector;                 /* terrain marker */ 		
                                        /* _no need to free this one_ */
    CRegion           *region;               /* region of this room */
    
    
    QByteArray    doors[6];		/* if the door is secret */
    unsigned  char exitFlags[6];
    
  
    int x, y, z;		/* coordinates on our map */

public:
    enum ExitFlags { EXIT_NONE = 0, EXIT_UNDEFINED, EXIT_DEATH};
  
    unsigned int    id; 		        /* identifier, public for speed up - its very often used  */
    CRoom           *exits[6];              /* very often used in places where performance matters */
  
  
    CRoom();
    ~CRoom();
    
    QByteArray getName();
    QByteArray getDesc();
    char getTerrain();
    QByteArray getNote();
    
    QByteArray getNoteColor();
    void setNoteColor(QByteArray color);
    
    void setDesc(QByteArray newdesc);
    void setName(QByteArray newname);
    void setTerrain(char terrain);
    void setSector(char val);
    void setNote(QByteArray note);
    
    bool isDescSet();
    bool isNameSet();
    bool isEqualNameAndDesc(CRoom *room);
    
    QString toolTip();
    
    void setModified(bool b);
    bool isConnected(int dir);
    void sendRoom();
    
    // door stuff
    int setDoor(int dir, QByteArray door);
    void removeDoor(int dir);
    QByteArray getDoor(int dir);
    bool isDoorSet(int dir);
    bool isDoorSecret(int dir);
    
    void disconnectExit(int dir);       /* just detaches the connection */
    void removeExit(int dir);           /* also removes the door */
    void setExit(int dir, CRoom *room);
    void setExit(int dir, unsigned int id);
//    CRoom *getExit(int dir);
    bool isExitLeadingTo(int dir, CRoom *room);
    
    bool isExitDeath(int dir);
    void setExitDeath(int dir);

    bool isExitNormal(int dir);
    bool isExitPresent(int dir);       /* if there is anything at all in this direction, deathtrap, undefined exit or normal one */
    bool isExitUndefined(int dir);
    void setExitUndefined(int dir);
    
    
    bool anyUndefinedExits();
    
    void setExitFlags(int dir, unsigned char flag);
    
    // coordinates     
    void setX(int x);
    void setY(int x);
    void setZ(int x);
    void simpleSetZ(int val); // does not perform any Plane operations (see rendering) on  CPlane in Map.h
    
    inline int getX() { return x; }
    inline int getY() { return y; }
    inline int getZ() { return z; }
    
    int descCmp(QByteArray desc);
    int roomnameCmp(QByteArray name);
    
    
    QByteArray getRegionName();
    CRegion *getRegion();
    void setRegion(QByteArray name);
    void setRegion(CRegion *reg);
    QByteArray getSecretsInfo();
    QByteArray getDoorAlias(int i);
    
    char dirbynum(int dir);
};

#endif
