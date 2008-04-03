#include <QHostAddress>

#include "CGroupClient.h"
#include "CGroupCommunicator.h"
#include "utils.h"

void CGroupClient::linkSignals()
{
	connect(this, SIGNAL(disconnected()), this, SLOT(lostConnection() ) );
	connect(this, SIGNAL(connected()), this, SLOT(connectionEstablished() ) );
		
	connect(this, SIGNAL(error(QAbstractSocket::SocketError )), this, SLOT(errorHandler(QAbstractSocket::SocketError) ) );
	connect(this, SIGNAL(stateChanged(QAbstractSocket::SocketError )), this, SLOT(stateChangedHandler(QAbstractSocket::SocketError) ) );
	connect(this, SIGNAL(readyRead()), this, SLOT( dataIncoming() )   );
}

CGroupClient::CGroupClient(QByteArray host, int remotePort, QObject *parent) :
	QTcpSocket(parent)
{
	print_debug(DEBUG_GROUP, "Connecting to remote host...");
	setConnectionState(Connecting);
	print_debug(DEBUG_GROUP, "NOW issuing the connect command...");
	connectToHost(host, remotePort);
	protocolState = AwaitingLogin;

	linkSignals();
}

CGroupClient::CGroupClient(QObject *parent) :
	QTcpSocket(parent)
{
	connectionState = Closed;
	
	linkSignals();
	protocolState = Idle;
}


void CGroupClient::setSocket(int socketDescriptor)
{
	if (setSocketDescriptor(socketDescriptor) == false) {
		// failure ... what to do?
		print_debug(DEBUG_GROUP, "Connection failed. Native socket not recognized by CGroupClient.");
	} 

	setConnectionState(Connected);
}

void CGroupClient::setProtocolState(int val)
{
	print_debug(DEBUG_GROUP, "changing protocol state: %i", val);
	protocolState = val;
}

void CGroupClient::setConnectionState(int val) 
{
	print_debug(DEBUG_GROUP, "changing connection state: %i", val);
	connectionState = val;
	getParent()->connectionStateChanged(this);
}
	

CGroupClient::~CGroupClient()
{
	
}

void CGroupClient::lostConnection()
{
	print_debug(DEBUG_GROUP, "Lost connection");
	connectionState = Closed;
	CGroupCommunicator *comm = (CGroupCommunicator *)parent();
	comm->connectionStateChanged(this);
}

void CGroupClient::connectionEstablished()
{
	print_debug(DEBUG_GROUP, "CGroupClient: connectionEstablished");
	setConnectionState(Connected);
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
			print_debug(DEBUG_GROUP, "ConnectingState");
			setConnectionState(Connecting);
			break;
		case QAbstractSocket::HostLookupState:
			print_debug(DEBUG_GROUP, "HostLookupState");
			//setConnectionState(Connecting);
			break;
		case QAbstractSocket::ConnectedState:
			print_debug(DEBUG_GROUP, "ConnectedState");
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
	print_debug(DEBUG_GROUP, "Incoming Data [conn %i, IP: %s]", socketDescriptor(),
			(const char *) peerAddress().toString().toAscii() );
	getParent()->incomingData(this);
}


