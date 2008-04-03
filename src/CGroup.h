#ifndef CGROUP_H_
#define CGROUP_H_

#include <QObject>
#include <QString>
#include "CGroupCommunicator.h"


class CGroup : QObject
{
	Q_OBJECT
	
	CGroupCommunicator *network;
	
	QByteArray name;
public:

	CGroup(QByteArray name, QObject *parent);
	virtual ~CGroup();
	
	QByteArray getName() { return name; }
	void changeType(int newState);
	
public slots:
	void connectionRefused(QString message);
	void connectionFailed(QString message);
	void connectionClosed(QString message);
	void connectionError(QString message);
	void serverStartupFailed(QString message);

};

#endif /*CGROUP_H_*/
