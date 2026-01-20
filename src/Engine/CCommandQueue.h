/*
 * CCommandQueue.h
 *
 *  Created on: Mar 3, 2011
 *      Author: aza
 */

#ifndef CCOMMANDQUEUE_H_
#define CCOMMANDQUEUE_H_

#include <cstdio>
#include <memory>

#include <QQueue>
#include <QMutex>
#include <QMutexLocker>
#include <QElapsedTimer>
#include <QVector>

#include "utils.h"

#include "Map/CRoom.h"

class CCommand
{
public:
	QElapsedTimer timer;
	int type;
	int dir;

	enum TYPES { NONE = 0, MOVEMENT = 1, SCOUTING };

    CCommand() {}
	CCommand(int _type, int _dir) : type(_type), dir(_dir) { timer.start(); }

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
    	dir = -1;
    }
};



class CCommandQueue {
    mutable QMutex pipeMutex;
    QQueue<CCommand> pipe;

public:

    void addCommand(int type, int dir)
    {
        CCommand command;
        command.type = type;
        command.dir = dir;
        command.timer.start();

        QMutexLocker locker(&pipeMutex);
        pipe.enqueue(command);
    }

    void addCommand(CCommand e)
    {
        QMutexLocker locker(&pipeMutex);
        pipe.enqueue(e);
    }

    void clear()
    {
        QMutexLocker locker(&pipeMutex);
        pipe.clear();
    }

    bool isEmpty() const
    {
        QMutexLocker locker(&pipeMutex);
        return pipe.empty();
    }

    CCommand peek()
    {
        QMutexLocker locker(&pipeMutex);
        if (!pipe.empty())
            return pipe.head();
        return CCommand();
    }

    CCommand dequeue()
    {
        QMutexLocker locker(&pipeMutex);
        if (!pipe.empty())
            return pipe.dequeue();
        return CCommand();
    }

    void print()
    {
        QMutexLocker locker(&pipeMutex);
        printf("Commands: ");
        for (int i = 0; i < pipe.size(); i++) {
            CCommand cmd = pipe.at(i);
            if (cmd.type == CCommand::MOVEMENT)
                printf("M %s ", exits[cmd.dir]);
        }
        printf("\r\n");
    }

    std::unique_ptr<QVector<unsigned int>> getPrespam(CRoom *r)
    {
        auto list = std::make_unique<QVector<unsigned int>>();
        QMutexLocker locker(&pipeMutex);

        list->append(r->id);
        for (int i = 0; i < pipe.size(); i++) {
            CCommand cmd = pipe.at(i);
            if (cmd.type == CCommand::MOVEMENT) {
                if (r->isConnected(cmd.dir)) {
                    list->append(r->exits[cmd.dir]->id);
                    r = r->exits[cmd.dir];
                }
                else if (r->isExitUndefined(cmd.dir))
                    break;
            }
        }
        return list;
    }
};


#endif /* CCOMMANDQUEUE_H_ */
