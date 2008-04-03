#include <QHostAddress>

#include "CGroupClient.h"
#include "CGroupCommunicator.h"
#include "utils.h"

CGroupClient::CGroupClient(QByteArray host, int remotePort, QObject *parent) :
	QTcpSocket(parent)
{
	
	setConnectionState(Connecting);
	connectToHost(host, remotePort);
	protocolState = Idle;
}

CGroupClient::CGroupClient(int socketDescriptor, QObject *parent) :
	QTcpSocket(parent)
{
	connectionState = Closed;
	if (setSocketDescriptor(socketDescriptor) == false) {
		// failure ... what to do?
		print_debug(DEBUG_GROUP, "Connection failed.");
	} 
	
	print_debug(DEBUG_GROUP, "Connected. Remote IP: %s", (const char *) peerAddress().toString().toAscii());
	connect(this, SIGNAL(disconnected()), this, SLOT(lostConnection() ) );
	connect(this, SIGNAL(error(QAbstractSocket::SocketError )), this, SLOT(errorHandler(QAbstractSocket::SocketError) ) );
	connect(this, SIGNAL(stateChanged(QAbstractSocket::SocketError )), this, SLOT(stateChangedHandler(QAbstractSocket::SocketError) ) );
	protocolState = Idle;
}


void CGroupClient::setProtocolState(int val)
{
	protocolState = val;
}

void CGroupClient::setConnectionState(int val) 
{
	connectionState = val;
	getParent()->connectionStateChanged(this);
}
	

CGroupClient::~CGroupClient()
{
	
}

void CGroupClient::lostConnection()
{
	connectionState = Closed;
	CGroupCommunicator *comm = (CGroupCommunicator *)parent();
	comm->connectionStateChanged(this);
}

void CGroupClient::errorHandler ( QAbstractSocket::SocketError socketError )
{
	CGroupCommunicator *comm = (CGroupCommunicator *)parent();
	comm->errorInConnection(this);
}

void CGroupClient::stateChangedHandler ( QAbstractSocket::SocketState socketState )
{

	switch (state()) {
		case QAbstractSocket::ConnectingState:
		case QAbstractSocket::HostLookupState:
			setConnectionState(Connecting);
			break;
		case QAbstractSocket::ConnectedState:
			setConnectionState(Connected);
			break;
		case QAbstractSocket::UnconnectedState:
			setConnectionState(Closed);
			break;
		case QAbstractSocket::ClosingState:
			setConnectionState(Quiting);
			break;
		case QAbstractSocket::BoundState:
		case QAbstractSocket::ListeningState:
		default:
			// ignore ...
			break;
	}
}

void CGroupClient::dataIncoming()
{
	print_debug(DEBUG_GROUP, "Connection: %s. Remote IP: %s", (const char *) peerAddress().toString().toAscii());
	getParent()->incomingData(this);
}


