#ifndef CGROUP_H_
#define CGROUP_H_

#include <QObject>

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
};

#endif /*CGROUP_H_*/
