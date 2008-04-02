
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
