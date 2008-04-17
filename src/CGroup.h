#ifndef CGROUP_H_
#define CGROUP_H_

#include <QWidget>
#include <QString>
#include <QDomNode>
#include <QVector>
#include <QGridLayout>

#include "CGroupCommunicator.h"
#include "CGroupChar.h"

class CGroup : public QWidget
{
	Q_OBJECT
	
	CGroupCommunicator *network;

	QVector<CGroupChar *> chars;
	CGroupChar	*self;

	QGridLayout *layout;
public:

	CGroup(QByteArray name, QWidget *parent);
	virtual ~CGroup();
	
	QByteArray getName() { return self->getName(); }
	CGroupChar* getCharByName(QByteArray name);

	void changeType(int newState);
	int getType() {return network->getType(); }
	bool isConnected() { return network->isConnected(); }
	void reconnect() { resetChars();  network->reconnect(); }

	bool addChar(QDomNode blob);
	void removeChar(QByteArray name);
	void removeChar(QDomNode node);
	bool isNamePresent(QByteArray name);
	bool addCharIfUnique(QDomNode blob);
	void updateChar(QDomNode blob);
	
	void resetChars();
	
	// changing settings
	void resetName();
	
	QDomNode getLocalCharData() { return self->toXML(); }
	void sendAllCharsData(CGroupClient *conn);
	
	void gTellArrived(QDomNode node);
	void sendGTell(QByteArray tell); // sends gtell from local user
public slots:
	// slots  {}()
	void connectionRefused(QString message);
	void connectionFailed(QString message);
	void connectionClosed(QString message);
	void connectionError(QString message);
	void serverStartupFailed(QString message);
	void gotKicked(QDomNode message);
	
	void update();

};

#endif /*CGROUP_H_*/
