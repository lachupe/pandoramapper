#ifndef CGROUPSERVER_H_
#define CGROUPSERVER_H_

#include <QList>
#include <QTcpServer>

#include "CGroupClient.h"
class CGroupCommunicator;

class CGroupServer: public QTcpServer
{
	Q_OBJECT
	
	QList<CGroupClient *> connections;
public:
	CGroupServer(int localPort, QObject *parent);
	virtual ~CGroupServer();
	void addClient(CGroupClient *client);
	
protected:
    void incomingConnection(int socketDescriptor);
    
public slots:
	void connectionClosed(CGroupClient *connection);

signals:
	void failedToStart();


};

#endif /*CGROUPSERVER_H_*/
