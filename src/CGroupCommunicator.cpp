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
	
	type = newState;
}

void CGroupCommunicator::connectionStateChanged(CGroupClient *connection)
{
//	Closed, Connecting, Connected, Logged, Quiting
	print_debug(DEBUG_GROUP, "CGroupMananger: connection state changed.");
	switch (connection->getConnectionState()) {
		case CGroupClient::Connecting :
			print_debug(DEBUG_GROUP, "Connecting to the remote host.");
			connecting(connection);
			connectionEstablished(connection);
			break;
		case CGroupClient::Connected :
			print_debug(DEBUG_GROUP, "Connection established.");
			connectionEstablished(connection);
			break;
		case CGroupClient::Closed :
			print_debug(DEBUG_GROUP, "Connection closed.");
			connectionClosed(connection);
			break;
		case CGroupClient::Quiting :
			print_debug(DEBUG_GROUP, "Closing the socket. Quiting");
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

void CGroupCommunicator::connectionEstablished(CGroupClient *connection)
{
	print_debug(DEBUG_GROUP, "Connection established. Type %i", type);
	if (type == Client) {
		connection->setProtocolState(CGroupClient::AwaitingLogin);
	} 
	if (type == Server) {
		sendMessage(connection, REQ_LOGIN);
		connection->setProtocolState(CGroupClient::AwaitingLogin);
	}
}

void CGroupCommunicator::connectionClosed(CGroupClient *connection)
{
	print_debug(DEBUG_GROUP, "Communicator::Connection closed");
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
	QString str;

	switch(connection->error()) {
		case QAbstractSocket::ConnectionRefusedError:	
			getGroup()->connectionRefused(connection->peerName() + " port "+ 
					connection->peerPort() );
			break;
		case QAbstractSocket::RemoteHostClosedError:
			
			connectionClosed(connection);

			//getGroup()->connectionRefused("Remote host closed the connection");
			break;
		case QAbstractSocket::HostNotFoundError:
			str = "Host not found";
			str += connection->peerName();
			getGroup()->connectionRefused( str );
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
			getGroup()->connectionError("Network Error ");
			break;
	}
	
}

void CGroupCommunicator::serverStartupFailed()
{
	CGroupServer *server = (CGroupServer *) peer;
	getGroup()->serverStartupFailed(server->errorString());
	changeType(Off);
}

//
// COmmunication protocol switches and logic
//

// the core of the protocol
void CGroupCommunicator::incomingData(CGroupClient *connection)
{
	
	if (type == Client)
		retrieveDataClient(connection);
	else if (type == Server)
		retrieveDataServer(connection);
}




// Client side of the communication protocol
void CGroupCommunicator::retrieveDataClient(CGroupClient *conn)
{
	QByteArray data;
	int message;
	
	data = conn->readAll();
	message = data[1];
	
	print_debug(DEBUG_GROUP, "Client side datagram arrived. Message : %i", message);
	
	
	switch (conn->getConnectionState()) {
		//Closed, Connecting, Connected, Quiting
		case CGroupClient::Connected:
			// AwaitingLogin, AwaitingInfo, Logged 

			if (conn->getProtocolState() == CGroupClient::AwaitingLogin) {
				// Login state. either REQ_LOGIN or ACK should come
				if (message == REQ_LOGIN) {
					sendLoginInformation(conn);
				} else if (message == ACK) {
					// aha! logged on!
					sendMessage(conn, REQ_INFO);
					conn->setProtocolState(CGroupClient::AwaitingInfo);
				} else {
					// ERROR: unexpected message marker!
					// try to ignore?
					print_debug(DEBUG_GROUP, "(AwaitingInfo) Unexpected message marker. Trying to ignore.");
				}
				
			} else if (conn->getProtocolState() == CGroupClient::AwaitingInfo) {
				// almost connected. awaiting full information about the connection
				if (message == DATA_INFO) {
					parseGroupInformation(conn, data);
					sendMessage(conn, ACK);
					conn->setProtocolState(CGroupClient::Logged);
				} else {
					// ERROR: unexpected message marker!
					// try to ignore?
					print_debug(DEBUG_GROUP, "(AwaitingData) Unexpected message marker. Trying to ignore.");
				}
				
			} else if (conn->getProtocolState() == CGroupClient::Logged) {
				// usual update situation. receive update, unpack, apply.
				
			} 
			
		case CGroupClient::Closed:
		case CGroupClient::Connecting:
		case CGroupClient::Quiting:
			print_debug(DEBUG_GROUP, "Data arrival during wrong connection state.");
			break;
	}
}



// Server side of the communication protocol
void CGroupCommunicator::retrieveDataServer(CGroupClient *conn)
{
	QByteArray data;
	int message;
	
	data = conn->readAll();
	message = data[1];
	
	print_debug(DEBUG_GROUP, "Server side. Datagram arrived. Message : %i", message);
	
	
	switch (conn->getConnectionState()) {
		//Closed, Connecting, Connected, Quiting
		case CGroupClient::Connected:
			// AwaitingLogin, AwaitingInfo, Logged 

			if (conn->getProtocolState() == CGroupClient::AwaitingLogin) {
				// Login state. either REQ_LOGIN or ACK should come
				if (message == DATA_LOGIN) {
					// aha! parse the data
					parseLoginInformation(conn, data);
					sendMessage(conn, ACK); 
					conn->setProtocolState(CGroupClient::AwaitingInfo);
				} else {
					// ERROR: unexpected message marker!
					// try to ignore?
					print_debug(DEBUG_GROUP, "Unexpected message marker. Trying to ignore.");
				}
			} else if (conn->getProtocolState() == CGroupClient::AwaitingInfo) {
				// almost connected. awaiting full information about the connection
				if (message == REQ_INFO) {
					sendGroupInformation(conn);
				} if (message == ACK) {
					conn->setProtocolState(CGroupClient::Logged);
				} else {
					// ERROR: unexpected message marker!
					// try to ignore?
					print_debug(DEBUG_GROUP, "Unexpected message marker. Trying to ignore.");
				}
				
			} else if (conn->getProtocolState() == CGroupClient::Logged) {
				// usual update situation. receive update, unpack, apply.
				
			} 
			
		case CGroupClient::Closed:
			
			
		case CGroupClient::Connecting:
		case CGroupClient::Quiting:
			print_debug(DEBUG_GROUP, "Data arrival during wrong connection state.");
			break;
	}
}

//
// user functions
//
void CGroupCommunicator::userLoggedOn(CGroupClient *conn)
{
	// TODO: call group-function. 
	// TODO: message all other peers on server side
}

void CGroupCommunicator::userLoggedOff(CGroupClient *conn)
{
	// TODO: call group-function. 
	// TODO: message all other peers on server side
}


//
// Parsers and Senders of information and signals to upper and lower objects
//
void CGroupCommunicator::sendLoginInformation(CGroupClient *conn)
{
	QByteArray info;
	
	// temporary
	info = "Login Information";
	info += "Name" + getGroup()->getName();
	sendMessage(conn, DATA_LOGIN, info);
}

void CGroupCommunicator::parseLoginInformation(CGroupClient *conn, QByteArray data)
{

	// temporary
	print_debug(DEBUG_GROUP, "Login Information arrived %s", (const char *) data);
}

void CGroupCommunicator::sendGroupInformation(CGroupClient *conn)
{
	QByteArray info;
	
	// temporary
	info = "Group Information";
	info += "Aza 15000 Fine Cold Tired 100 10 40";
	info += "Stolb 15000 Fine Cold Tired 100 10 40";
	sendMessage(conn, DATA_INFO, info);
}

void CGroupCommunicator::parseGroupInformation(CGroupClient *conn, QByteArray data)
{
	// temporary
	print_debug(DEBUG_GROUP, "Login Information arrived %s", (const char *) data);
}


//
// Low level. Message forming and messaging
//
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

	print_debug(DEBUG_GROUP, "Message: %s", (const char *) block);

    return block;
}

void CGroupCommunicator::sendMessage(CGroupClient *connection, int message, QByteArray data)
{
	print_debug(DEBUG_GROUP, "Sending message");
	if (connection->write( formMessageBlock(message, data) ) == -1)
		print_debug(DEBUG_GROUP, "Error at attempt of writing the data!");
}

