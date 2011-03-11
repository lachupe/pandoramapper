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

#ifndef CEVENT_H
#define CEVENT_H

#include <QQueue>
#include <QMutex>

class Event   {
    public:
        Event() {}
        Event(const Event &other) 
        {
            dir = other.dir;
            name = other.name;
            desc = other.desc;
            exits = other.exits;
            blind = other.blind;
            scout = other.scout;
            terrain = other.terrain;
            prompt = other.prompt;
            movement = other.movement;
            fleeing = other.fleeing;
            movementBlocker = other.movementBlocker;
        }

        Event &operator=(const Event &other) 
        {
            if (this != &other) {  // make sure not same object
                dir = other.dir;
                name = other.name;
                desc = other.desc;
                exits = other.exits;
                blind = other.blind;
                scout = other.scout;
                terrain = other.terrain;
                prompt = other.prompt;
                movement = other.movement;
                fleeing = other.fleeing;
                movementBlocker = other.movementBlocker;
            }
            return *this;    // Return ref for multiple assignment            
        }

        void clear()    {
            dir = "";
            name = "";
            desc = "";
            exits = "";
            prompt = "";
            scout = false;
            blind = false;
            movement = false;
            fleeing = false;
            terrain = -1;
            movementBlocker = false;
        }

        QByteArray dir;
        QByteArray name;
        QByteArray desc;
        QByteArray exits;
        bool       blind;         /* fog, no light, blind flag */
        bool       scout;
        bool       movement;
        bool	   fleeing;	/* this movement was caused by fleeing */
        bool	   movementBlocker;
        char       terrain;
        QByteArray prompt;
};

class PipeManager {
    QMutex pipeMutex;
    QQueue<Event> Pipe;

public:
    void addEvent(Event e)
    {
    	pipeMutex.lock();
    	Pipe.enqueue(e);
    	pipeMutex.unlock();
    }

    void clear()
    {
    	pipeMutex.lock();
    	Pipe.clear();
    	pipeMutex.unlock();
    }

    bool isEmpty() { return Pipe.empty(); }    

    Event getEvent() 
    {
        Event event;
        pipeMutex.lock(); 
        event = Pipe.dequeue(); 
        pipeMutex.unlock(); 
        return event;
    }; 
};


#endif
