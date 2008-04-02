#include "CGroupServer.h"
#include <QHostAddress>

CGroupServer::CGroupServer(int localPort, QObject *parent) : 
	QTcpServer(parent)
{
	printf("Creating the GroupManager Server.\r\n");
	if (listen(QHostAddress::Any, localPort) != true) {
		printf("Failed to start a group Manager server!\r\n");
		emit failedToStart();
	} else {
		printf("GroupManager: Listening on port %i!\r\n", localPort);
	}
}

CGroupServer::~CGroupServer()
{
	
}

void CGroupServer::incomingConnection(int socketDescriptor)
{
	printf("Incoming connection.\r\n");
	// connect the client straight to the Communicator, as he handles all the state changes 
	// data transfers and similar.
	CGroupClient *client = new CGroupClient(socketDescriptor, parent());
	addClient(client);
}

void CGroupServer::addClient(CGroupClient *client)
{
	connections.push_back(client);
}

