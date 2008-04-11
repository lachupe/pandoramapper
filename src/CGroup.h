#ifndef CGROUP_H_
#define CGROUP_H_

#include <QObject>
#include <QString>
#include <QDomNode>
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

	bool addChar(QDomNode blob);
	void removeChar(QByteArray name);
	void removeChar(QDomNode node);
	bool isNamePresent(QByteArray name);
	bool addCharIfUnique(QDomNode blob);
	void updateChar(QDomNode blob);
	
	QDomNode getLocalCharData() { return self->toXML(); }
	void sendAllCharsData(CGroupClient *conn);
	
	void gTellArrived(QDomNode node);
	void sendGTell(QByteArray tell); // sends gtell from local user
public slots:

	void connectionRefused(QString message);
	void connectionFailed(QString message);
	void connectionClosed(QString message);
	void connectionError(QString message);
	void serverStartupFailed(QString message);
	void gotKicked(QDomNode message);
};

#endif /*CGROUP_H_*/
