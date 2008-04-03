#include <QDataStream>

#include "CGroupCommunicator.h"



#include "CConfigurator.h"
#include "utils.h"
#include "CGroup.h"
#include "CGroupServer.h"
#include "CGroupClient.h"


CGroupCommunicator::CGroupCommunicator(int _type, QObject *parent):
	QObject(parent)
{
	type = Off;
	peer = new CGroupDraftConnection;
	changeType(_type);
}

CGroupCommunicator::~CGroupCommunicator()
{
	
}

void CGroupCommunicator::changeType(int newState) {
	if (type == newState)
		return;

	delete peer;
	
	print_debug(DEBUG_GROUP, "Changing the Type of the GroupManager: new Type %i.", newState);
	switch (newState) {
		case Server:
			peer = new CGroupServer(conf->getGroupManagerLocalPort(), this);
			break;
		case Client:
			peer = new CGroupClient(conf->getGroupManagerHost(), conf->getGroupManagerRemotePort(), this);
			break;
		default:
			break;
	}
}

void CGroupCommunicator::connectionStateChanged(CGroupClient *connection)
{
//	Closed, Connecting, Connected, Logged, Quiting
	switch (connection->getConnectionState()) {
		case CGroupClient::Closed :
			print_debug(DEBUG_GROUP, "Closing the socket. Connection closed by the other side.");
			connectionClosed(connection);
			break;
		case CGroupClient::Connecting :
			print_debug(DEBUG_GROUP, "Closing the socket. Connection closed by the other side.");
			connecting(connection);
			break;
		case CGroupClient::Connected :
			print_debug(DEBUG_GROUP, "Connection established.");
			connectionEstablished(connection);
			break;
		case CGroupClient::Logged :
			print_debug(DEBUG_GROUP, "Logged on");
			//connectionClosed(connection);
			break;
		case CGroupClient::Quiting :
			print_debug(DEBUG_GROUP, "Closing the socket. Connection closed by the other side.");
			//connectionClosed(connection);
			break;
		default:
			printf("Some state change...\r\n");
			break;
	}
}


void CGroupCommunicator::connecting(CGroupClient *connection)
{
	if (type == Client) {
		
	} else if (type == Server) {
		
	}
}

QByteArray CGroupCommunicator::formMessageBlock(int message, QByteArray data)
{
	QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    out.setVersion(QDataStream::Qt_4_0);	
    out << (quint16)0;
    out << message;
    out << data;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));	
    
    return block;
}

void CGroupCommunicator::sendMessage(CGroupClient *connection, int message, QByteArray data)
{
	connection->write( formMessageBlock(message, data) );
}



void CGroupCommunicator::connectionEstablished(CGroupClient *connection)
{
	if (type == Client) {
		connection->setProtocolState(CGroupClient::Idle);
	} else if (type == Server) {
		sendMessage(connection, REQ_LOGIN, "");
		connection->setProtocolState(CGroupClient::AwaitingData);
	}
}

void CGroupCommunicator::connectionClosed(CGroupClient *connection)
{
	if (type == Client) {
		changeType(Off);
	} else if (type == Server) {
		getGroup()->connectionClosed("");
		
		CGroupServer *server = (CGroupServer *) peer;
		server->connectionClosed(connection);
	}
	
}

void CGroupCommunicator::errorInConnection(CGroupClient *connection)
{
	switch(connection->error()) {
		case QAbstractSocket::ConnectionRefusedError:	
			getGroup()->connectionRefused(connection->peerName() + " port "+ 
					connection->peerPort() );
			break;
		case QAbstractSocket::RemoteHostClosedError:
		case QAbstractSocket::HostNotFoundError:
			getGroup()->connectionRefused("Host not found" + connection->peerName());
			break;
		case QAbstractSocket::SocketAccessError:
			getGroup()->connectionError("Socket Access Error");
			break;
		case QAbstractSocket::SocketResourceError:
			getGroup()->connectionError("Socket Resource Error");
			break;
		case QAbstractSocket::SocketTimeoutError:
			getGroup()->connectionError("Socket Timeout Error");
			break;
		case QAbstractSocket::DatagramTooLargeError:
			getGroup()->connectionError("Diagram Too Large Error");
			break;
		case QAbstractSocket::NetworkError:
			getGroup()->connectionError("Network Error");
			break;
		case QAbstractSocket::AddressInUseError:
		case QAbstractSocket::SocketAddressNotAvailableError:
			getGroup()->connectionRefused("Network Error");
			break;
		case QAbstractSocket::UnsupportedSocketOperationError:
		case QAbstractSocket::ProxyAuthenticationRequiredError:
		case QAbstractSocket::UnknownSocketError:
		case QAbstractSocket::UnfinishedSocketOperationError:
			getGroup()->connectionError("Network Error " + (int) connection->error());
			break;
	}
	
}

void CGroupCommunicator::serverStartupFailed()
{
	CGroupServer *server = (CGroupServer *) peer;
	getGroup()->serverStartupFailed(server->errorString());
	changeType(Off);
}

// the core of the protocol
void CGroupCommunicator::incomingData(CGroupClient *connection)
{
	
	if (type == Client)
		retrieveDataClient(connection);
	else if (type == Server)
		retrieveDataServer(connection);
}

void CGroupCommunicator::retrieveDataClient(CGroupClient *conn)
{
	QByteArray data;
	data = conn->readAll();

	switch (conn->getConnectionState()) {
	//Closed, Connecting, Connected, Logged, Quiting
		case CGroupClient::Connected:
			if (conn->getProtocolState() == CGroupClient::Idle) {
				
			} else if (conn->getProtocolState() == CGroupClient::AwaitingAck) {
				
			} else if (conn->getProtocolState() == CGroupClient::AwaitingData) {
				
			}
		case CGroupClient::Logged:

		case CGroupClient::Closed:
		case CGroupClient::Connecting:
		case CGroupClient::Quiting:
			print_debug(DEBUG_GROUP, "Data arrival during wrong connection state.");
			break;
	}
}

void CGroupCommunicator::retrieveDataServer(CGroupClient *conn)
{
	QByteArray data;
	data = conn->readAll();
	
}

