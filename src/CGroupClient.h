#ifndef CGROUPCLIENT_H_
#define CGROUPCLIENT_H_

#include <QTcpSocket>
#include "CGroupCommunicator.h"


class CGroupClient : public QTcpSocket
{
	Q_OBJECT

	int	 connectionState;
	int  protocolState;
	
	CGroupCommunicator* getParent() { return (CGroupCommunicator *) parent(); }
public:
	enum ConnectionStates { Closed, Connecting, Connected, Logged, Quiting};
	enum ProtocolStates { Idle, AwaitingAck, AwaitingData }; 
	
	CGroupClient(int socketDescriptor, QObject *parent);
	CGroupClient(QByteArray host, int remotePort, QObject *parent);
	virtual ~CGroupClient();

	int getConnectionState() {return connectionState; }
	void setConnectionState(int val);
	void setProtocolState(int val);
	int getProtocolState() { return protocolState; }
		
protected slots:
	void lostConnection();
	void errorHandler ( QAbstractSocket::SocketError socketError );
	void stateChangedHandler ( QAbstractSocket::SocketState socketState );
	void dataIncoming();

signals:

};

#endif /*CGROUPCLIENT_H_*/
