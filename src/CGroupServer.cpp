#include <QHostAddress>

#include "utils.h"
#include "CGroupServer.h"

CGroupServer::CGroupServer(int localPort, QObject *parent) : 
	QTcpServer(parent)
{
	printf("Creating the GroupManager Server.\r\n");
	if (listen(QHostAddress::Any, localPort) != true) {
		print_debug(DEBUG_GROUP, "Failed to start a group Manager server!");
		emit failedToStart();
	} else {
		print_debug(DEBUG_GROUP, "Listening on port %i!", localPort);
	}
}

CGroupServer::~CGroupServer()
{
	
}

void CGroupServer::incomingConnection(int socketDescriptor)
{
	print_debug(DEBUG_GROUP, "Incoming connection");
	// connect the client straight to the Communicator, as he handles all the state changes 
	// data transfers and similar.
	CGroupClient *client = new CGroupClient(parent());
	addClient(client);
	
	client->setSocket(socketDescriptor);
}

void CGroupServer::addClient(CGroupClient *client)
{
	print_debug(DEBUG_GROUP, "Adding a client to the connections list.");
	connections.append(client);
}

void CGroupServer::connectionClosed(CGroupClient *connection)
{
	print_debug(DEBUG_GROUP, "Removing and deleting the connection completely.");
	connections.removeAll(connection);
	print_debug(DEBUG_GROUP, "Deleting the connection");
	connection->deleteLater();
}

void CGroupServer::sendToAll(QByteArray message)
{
	sendToAllExceptOne(NULL, message);
}


void CGroupServer::sendToAllExceptOne(CGroupClient *conn, QByteArray message)
{
	for (int i = 0; i < connections.size(); i++) 
		if (connections[i] != conn) 
			connections[i]->sendData(message);
}

void CGroupServer::closeAll()
{
	
	printf("Closing connections\r\n");
	for (int i = 0; i < connections.size(); i++) {
		printf("Closing connections %i\r\n", connections[i]->socketDescriptor());
		connections[i]->deleteLater();
	}
}

