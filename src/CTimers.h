/*
 * CTimers.h
 *
 *  Created on: Mar 13, 2011
 *      Author: aza
 */

#ifndef CTIMERS_H_
#define CTIMERS_H_

#include <QByteArray>
#include <QTime>
#include <QList>
#include <QMutex>

struct TTimer {
	int 		id;
	QByteArray name;
	QByteArray desc;
	int 	   duration;
	QTime	   timer;

};

class CTimers : public QObject {
	Q_OBJECT

	QMutex	m_lock;

	int 	m_nextId;
	QList<TTimer *> m_timers;
	QList<TTimer *> m_countdowns;


	QByteArray getTimers();
	QByteArray getCountdowns();


public:
	CTimers();
	virtual ~CTimers();

	static QString msToMinSec(int ms)
	{
	    QString s;
	    int min;
	    int sec;

	    sec = ms / 1000;
	    min = sec / 60;
	    sec = sec % 60;

	    s = QString("%1%2:%3%4")
	            .arg( min / 10 )
	            .arg( min % 10 )
	            .arg( sec / 10 )
	            .arg( sec % 10 );

	    return s;
	}


	void addTimer(QByteArray name, QByteArray desc);
	void addCountdown(QByteArray name, QByteArray desc, int time);

	bool removeTimer(QByteArray name);
	bool removeCountdown(QByteArray name);

	// for stat command representation
	QByteArray getStatCommandEntry();


	void clear();

public slots:
	void finishCountdownTimer();

};

#endif /* CTIMERS_H_ */
