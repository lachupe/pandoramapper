
#include "CConfigurator.h"
#include "CGroup.h"
#include "CGroupCommunicator.h"

CGroup::CGroup(QByteArray name, QObject *parent)
: QObject(parent), name(name)
{
	printf("Starting up the GroupManager.\r\n");
	network = new CGroupCommunicator(CGroupCommunicator::Off, this);
	network->changeType(conf->getGroupManagerState());
}

void CGroup::changeType(int newState)
{
	network->changeType(newState);
}


CGroup::~CGroup()
{
	delete network;
}

void CGroup::connectionRefused(QString message)
{
	printf("Connection refused: %s\r\n", (const char *) message.toAscii());
}

void CGroup::connectionFailed(QString message)
{
	printf("Failed to connect: %s\r\n", (const char *) message.toAscii());
}

void CGroup::connectionClosed(QString message)
{
	printf("Connection closed: %s\r\n", (const char *) message.toAscii());
}

void CGroup::connectionError(QString message)
{
	printf("Connection closed: %s\r\n", (const char *) message.toAscii());
}
