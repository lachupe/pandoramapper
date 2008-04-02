#include "CGroupCommunicator.h"

#include "CConfigurator.h"

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
	
	printf("Changing the Type of the GroupManager.\r\n");
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
	switch (connection->state()) {
		case QAbstractSocket::ClosingState :
			printf("Closing the socket. Connection closed by the other side.\r\n");
			break;
		case QAbstractSocket::UnconnectedState :
			printf("Closing the socket. Connection closed by the other side.\r\n");
			break;
		default:
			printf("Some state change...\r\n");
			break;
	}
}

