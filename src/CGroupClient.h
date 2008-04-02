#ifndef CGROUPCLIENT_H_
#define CGROUPCLIENT_H_

#include <QTcpSocket>
#include "CGroupCOmmunicator.h"


class CGroupClient : public QTcpSocket
{
	Q_OBJECT

	int	 connectionState;
	CGroupCommunicator* getParent() { return (CGroupCommunicator *) parent(); }
public:
	enum States { Closed, Connecting, Connected, Neogtiating, Logged, Quiting};

	CGroupClient(int socketDescriptor, QObject *parent);
	CGroupClient(QByteArray host, int remotePort, QObject *parent);
	virtual ~CGroupClient();

	int getConnectionState() {return connectionState; }
	void setConnectionState(int val) {connectionState = val; }
	
protected slots:
	void lostConnection();
	void errorHandler ( QAbstractSocket::SocketError socketError );
	void stateChangedHandler ( QAbstractSocket::SocketState socketState );
	void dataIncoming();

signals:

};

#endif /*CGROUPCLIENT_H_*/
