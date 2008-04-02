#ifndef CGROUPSERVER_H_
#define CGROUPSERVER_H_

#include <QVector>
#include <QTcpServer>

#include "CGroupClient.h"
class CGroupCommunicator;

class CGroupServer: public QTcpServer
{
	Q_OBJECT
	
	QVector<CGroupClient *> connections;
public:
	CGroupServer(int localPort, QObject *parent);
	virtual ~CGroupServer();
	void addClient(CGroupClient *client);
	
protected:
    void incomingConnection(int socketDescriptor);
    
private slots:

signals:
	void failedToStart();


};

#endif /*CGROUPSERVER_H_*/
