/*
 * CCommandQueue.h
 *
 *  Created on: Mar 3, 2011
 *      Author: aza
 */

#ifndef CCOMMANDQUEUE_H_
#define CCOMMANDQUEUE_H_

#include <cstdio>

#include <QQueue>
#include <QMutex>
#include <QTime>
#include <QVector>

#include "utils.h"

#include "Map/CRoom.h"
#include "Map/CRoomManager.h"

class CCommand
{
public:
	QTime timer;
	int type;
    ExitDirection dir;

	enum TYPES { NONE = 0, MOVEMENT = 1, SCOUTING };

    CCommand() {}
    CCommand(int _type, ExitDirection _dir) : type(_type), dir(_dir) { timer.start(); }

	CCommand(const CCommand &other)
    {
        dir = other.dir;
        type = other.type;
        timer = other.timer;
    }

	CCommand &operator=(const CCommand &other)
    {
        if (this != &other) {  // make sure not same object
            dir = other.dir;
            type = other.type;
            timer = other.timer;
        }
        return *this;    // Return ref for multiple assignment
    }

    void clear()    {
    	timer.restart();
    	type = NONE;
        dir = ED_UNKNOWN;
    }
};



class CCommandQueue {
    QMutex pipeMutex;
    QQueue<CCommand> pipe;

public:

    void addCommand(int type, ExitDirection dir)
    {
    	CCommand command;
    	pipeMutex.lock();
    	command.type = type;
    	command.dir = dir;
    	command.timer.start();
    	pipe.enqueue(command);
    	pipeMutex.unlock();
    }

    void addCommand(CCommand e)
    {
    	pipeMutex.lock();
    	pipe.enqueue(e);
    	pipeMutex.unlock();
    }

    void clear() { pipeMutex.lock(); pipe.clear(); pipeMutex.unlock();  }
    bool isEmpty() { return pipe.empty(); }

    CCommand peek() {
    	CCommand command;
        pipeMutex.lock();
        if (!pipe.empty())
        		command = pipe.head();
        pipeMutex.unlock();
        return command;
    }

    CCommand dequeue()
    {
    	CCommand command;
        pipeMutex.lock();
        if (!pipe.empty())
        	command = pipe.dequeue();
        pipeMutex.unlock();
        return command;
    };

    void print() {
    	printf("Commands: ");
    	for (int i = 0; i < pipe.size(); i++) {
    		CCommand cmd = pipe.at(i);
    		if (cmd.type == CCommand::MOVEMENT)
    			printf("M %s ", exits[cmd.dir] );

    	}
   		printf("\r\n");
    }

    QVector<unsigned int> *getPrespam(CRoom *r)
	{
    	QVector<unsigned int> *list = new QVector<unsigned int>();

        pipeMutex.lock();

        list->append(r->getId());
    	for (int i = 0; i < pipe.size(); i++) {
    		CCommand cmd = pipe.at(i);
    		if (cmd.type == CCommand::MOVEMENT) {
				if (r->isConnected(cmd.dir) ) {
                    int id = r->getExitLeadsTo(cmd.dir);
                    list->append(id);
                    r = Map.getRoom(id);
				}
				else if (r->isExitUndefined( cmd.dir ) )
					break;
    		}

    	}

        pipeMutex.unlock();
    	return list;
	}
};


#endif /* CCOMMANDQUEUE_H_ */
