#ifndef CGROUP_H_
#define CGROUP_H_

#include <QObject>
#include <QString>
#include <QVector>
#include "CGroupCommunicator.h"
#include "CGroupChar.h"

class CGroup : QObject
{
	Q_OBJECT
	
	CGroupCommunicator *network;

	QVector<CGroupChar *> chars;
	CGroupChar	*self;
	
public:

	CGroup(QByteArray name, QObject *parent);
	virtual ~CGroup();
	
	QByteArray getName() { return self->getName(); }
	CGroupChar* getCharByName(QByteArray name);

	void changeType(int newState);

	bool addChar(QByteArray blob);
	void removeChar(QByteArray name);
	bool isNamePresent(QByteArray name);
	bool addCharIfUnique(QByteArray blob);
	void updateChar(QByteArray blob);
	
	QByteArray getLocalCharData() { return self->toBlob(); }
	void sendAllCharsData(CGroupClient *conn);
	
public slots:

	void connectionRefused(QString message);
	void connectionFailed(QString message);
	void connectionClosed(QString message);
	void connectionError(QString message);
	void serverStartupFailed(QString message);
	void gotKicked(QString message);
};

#endif /*CGROUP_H_*/
