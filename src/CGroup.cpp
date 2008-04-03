
#include "utils.h"
#include "CConfigurator.h"
#include "CGroup.h"
#include "CGroupCommunicator.h"

CGroup::CGroup(QByteArray name, QObject *parent)
: QObject(parent), name(name)
{
	print_debug(DEBUG_GROUP, "Starting up the GroupManager.\r\n");
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
	print_debug(DEBUG_GROUP, "Connection refused: %s\r\n", (const char *) message.toAscii());
}

void CGroup::connectionFailed(QString message)
{
	print_debug(DEBUG_GROUP, "Failed to connect: %s\r\n", (const char *) message.toAscii());
}

void CGroup::connectionClosed(QString message)
{
	print_debug(DEBUG_GROUP, "Connection closed: %s\r\n", (const char *) message.toAscii());
}

void CGroup::connectionError(QString message)
{
	print_debug(DEBUG_GROUP, "Connection closed: %s\r\n", (const char *) message.toAscii());
}

void CGroup::serverStartupFailed(QString message)
{
	print_debug(DEBUG_GROUP, "Failed to start the Group server: %s\r\n", (const char *) message.toAscii());
}
