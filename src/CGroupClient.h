#ifndef CGROUPCLIENT_H_
#define CGROUPCLIENT_H_

#include <QTcpSocket>

class CGroupClient : public QTcpSocket
{
	Q_OBJECT
	
public:
	CGroupClient(int socketDescriptor, QObject *parent);
	CGroupClient(QByteArray host, int remotePort, QObject *parent);
	virtual ~CGroupClient();


	
protected slots:
	void lostConnection();

signals:

};

#endif /*CGROUPCLIENT_H_*/
