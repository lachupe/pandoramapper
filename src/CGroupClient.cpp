#include "CGroupClient.h"
#include <QHostAddress>

CGroupClient::CGroupClient(QByteArray host, int remotePort, QObject *parent) :
	QTcpSocket(parent)
{
	
}

CGroupClient::CGroupClient(int socketDescriptor, QObject *parent) :
	QTcpSocket(parent)
{
	if (setSocketDescriptor(socketDescriptor) == false) {
		// failure ... what to do?
		printf("Connection failed.\r\n");
	} 
	
	printf("Connected. Remote IP: %s\r\n", (const char *) peerAddress().toString().toAscii());
	connect(this, SIGNAL(disconnected()), this, SLOT(lostConnection() ) );
	
}


void CGroupClient::lostConnection()
{
	printf("Connection lost!\r\n");
}


CGroupClient::~CGroupClient()
{
}
