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


#include "CSquare.h"
#include "CRoom.h"


#define MAX_SQUARE_SIZE         40
#define MAX_SQUARE_ROOMS        40


/* -------------------------------------------------------------------------*/
/*  Planes (CPlane) and Square-tree (CSquare) implementation is below       */
/* -------------------------------------------------------------------------*/
CSquare::CSquare()
{
    subsquares[Left_Upper] = NULL;
    subsquares[Right_Upper] = NULL;
    subsquares[Left_Lower] = NULL;
    subsquares[Right_Lower] = NULL;
    
    leftx =  -MAX_SQUARE_SIZE/2;      
    lefty =   MAX_SQUARE_SIZE/2;      
    rightx =  MAX_SQUARE_SIZE/2;
    righty = -MAX_SQUARE_SIZE/2;
    centerx = 0;
    centery = 0;
}

CSquare::~CSquare()
{
    if (subsquares[0]) {
        delete subsquares[0];
    }
    if (subsquares[1]) {
        delete subsquares[1];
    }
    if (subsquares[2]) {
        delete subsquares[2];
    }
    if (subsquares[3]) {
        delete subsquares[3];
    }
}


CSquare::CSquare(int lx, int ly, int rx, int ry)
{
    subsquares[Left_Upper] = NULL;
    subsquares[Right_Upper] = NULL;
    subsquares[Left_Lower] = NULL;
    subsquares[Right_Lower] = NULL;

    leftx = lx;
    lefty = ly;
    rightx = rx;
    righty = ry;
    
    centerx = leftx + (rightx - leftx) / 2;
    centery = righty + (lefty - righty) / 2;
}


void CSquare::addSubsquareByMode(int mode)
{
    switch (mode)
    {
            case Left_Upper : 
                    subsquares[Left_Upper] =  new CSquare(leftx, lefty, centerx, centery);
                    break;
            case Right_Upper :
                    subsquares[Right_Upper] = new CSquare(centerx, lefty, rightx, centery);
                    break;
            case Left_Lower:
                    subsquares[Left_Lower] =  new CSquare(leftx, centery, centerx, righty);
                    break;
            case Right_Lower:
                    subsquares[Right_Lower] = new CSquare(centerx, centery, rightx, righty);
                    break;
    }
}


void CSquare::addRoomByMode(CRoom *room, int mode)
{
    mode = getMode(room);
    if (subsquares[mode] == NULL) 
        this->addSubsquareByMode(mode);
        
    subsquares[ mode ]->add(room);
}


bool CSquare::toBePassed()
{
    if (!rooms.empty())
        return false;
    
    /* if we have ANY children, the node has to be passed */
    if (subsquares[0] || subsquares[1] || subsquares[2] || subsquares[3] )
        return true;
    
    return false;
}

void CSquare::add(CRoom *room)
{
    CRoom *r;
    int i;
    
    if (toBePassed() ) 
    {
        addRoomByMode(room, getMode(room) );
        return;
    }
    
    if (( rooms.size() < MAX_SQUARE_ROOMS) && ( (rightx - leftx) < MAX_SQUARE_SIZE) ) 
    {
        if (rooms.contains(room) == false) {
            rooms.push_back(room);
        }
        return;
    } else {
        
        for (i=0; i < rooms.size(); i++) {
            r = rooms[i] ;
            addRoomByMode(r, getMode(r) );
        }
        rooms.clear();
        rooms.resize(0);
        addRoomByMode(room, getMode(room) );
    }
}

void CSquare::remove(CRoom *room)
{
    CSquare *p;
    int i;
    
    p = this;
    while (p) {
        if (!p->toBePassed()) {       
            /* just for check */
            for ( i=0; i < p->rooms.size(); ++i) {
                if ( room->id == p->rooms[i]->id ) {
                    p->rooms.remove(i);
                    return;
                }
            }
        }
        p = p->subsquares[ p->getMode(room) ];
    }
}

int CSquare::getMode(CRoom *room)
{
    return getMode(room->getX(), room->getY());
}

int CSquare::getMode(int x, int y)
{
    if (this->centerx > x) {
        if (this->centery > y) {
            return Left_Lower;
        } else {
            return Left_Upper;
        }
    } else {
        if (this->centery > y) {
            return Right_Lower;
        } else {
            return Right_Upper;
        }
    }
}

bool CSquare::isInside(CRoom *room)
{
    /* note : right and lower borders are inclusive */
    
    if ((leftx <  room->getX()) && (rightx >= room->getX()) &&  
        (lefty >  room->getY()) && (righty <= room->getY())    )
        return true;    /* yes the room is inside this square then */
    
    return false; /* else its not */
}

/* CPlane classes implementation */

CPlane::CPlane()
{
    z = 0;
    next = NULL;
    squares = NULL;
}

CPlane::~CPlane()
{
    delete squares;
}

CPlane::CPlane(CRoom *room)
{
    next = NULL;

    z = room->getZ();

    
    squares = new CSquare(  room->getX() - ( MAX_SQUARE_SIZE - 1) / 2,  
                            room->getY() + ( MAX_SQUARE_SIZE - 1 ) / 2,
                            room->getX() + ( MAX_SQUARE_SIZE - 1 ) / 2,
                            room->getY() - ( MAX_SQUARE_SIZE - 1 ) / 2);

    if (squares->rooms.contains(room) == false) {
        squares->rooms.push_back(room);
    }
}


