/*
 *  Pandora MUME mapper
 *
 *  Copyright (C) 2000-2009  Azazello
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

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
	connection->disconnect();
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

