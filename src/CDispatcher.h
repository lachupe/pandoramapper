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

#ifndef CDISPATCHER_H
#define CDISPATCHER_H


#include "proxy.h"
#include "CEngine.h"

using namespace std;

struct Tincoming_lines {
    QByteArray              line;
    unsigned short int   type;
    unsigned short int   xmlType;
};


class Cdispatcher
{
    struct Tincoming_lines buffer[4096];
    int 	amount;
    char    commandBuffer[4096];
    
    int       xmlState;          /* desc shall be incoming - just got roomname */
    int       mbrief_state;
    bool      awaitingData;
    bool      scouting;
    Event     event;

    enum dispatcherStates { STATE_NORMAL = 0, 
                                               STATE_ROOM, 
                                               STATE_DESC, 
                                               STATE_NAME, 
                                               STATE_PROMPT, 
                                               STATE_EXITS };
    
    bool spells_print_mode; /* After "Affected by:" until next prompt */
	
    enum LineTypes {IS_NORMAL    = 0,
                                   IS_XML,
                                   IS_SKIP,
                                   IS_DATA,
                                   IS_PROMPT,
                                   };

    enum XmlTypes {
                                   XML_START_ROOM = 1, 
                                   XML_START_NAME = 2, 
                                   XML_START_DESC = 3,  
                                   XML_START_PROMPT = 4, 
                                   XML_START_EXITS = 5, 
                                   XML_START_MOVEMENT = 6, 
                                   XML_START_TERRAIN = 7,
                                   XML_END_ROOM = 11, 
                                   XML_END_NAME = 12, 
                                   XML_END_DESC = 13,  
                                   XML_END_PROMPT = 14, 
                                   XML_END_EXITS = 15, 
                                   XML_END_MOVEMENT = 16, 
                                   XML_END_TERRAIN = 17
                                 };

    enum MainStates {
        NORMAL = 0,
        XML, 
        TELNET,
        STUFFING
    };
    
    
    
    enum SubStatesNormal {
        CR,
        LF,
    };
    
    enum StuffingStates {
        AMP,
        G,
        L,
        A,
        A_M,
        Q,
        Q_U,
        Q_U_O,
        LASTCHAR
     };
     
     
     enum SubStatesTelnet {
        T_NORMAL, 
        T_GOTIAC, 
        T_GOTWILL, 
        T_GOTWONT, 
        T_GOTDO, 
        T_GOTDONT, 
        T_GOTSB, 
        T_GOTSBIAC,
        T_SKIP,
     };

    QRegExp scoreExp;
    QRegExp scoreTrollExp;


    char parseTerrain(QByteArray prompt);


    bool parseXml(QByteArray tag);
    void dispatchBuffer(ProxySocket &c);
    QByteArray cutColours(QByteArray line);
public:
    
    int  analyzeMudStream(ProxySocket &c);
    int  analyzeUserStream(ProxySocket &c);
    
    Cdispatcher();
};

//extern class Cdispatcher *dispatcher;

#endif
