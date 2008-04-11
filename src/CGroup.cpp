
#include "utils.h"
#include "CConfigurator.h"
#include "CGroup.h"
#include "CGroupCommunicator.h"

CGroup::CGroup(QByteArray name, QObject *parent)
: QObject(parent)
{
	CGroupChar *ch;
	
	ch = new CGroupChar;
	
	ch->setName(name);
	ch->setPosition(1); // FIXME ... or does not really matter.
	chars.append(ch);
	self = ch;
	
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

bool CGroup::addChar(QDomNode node)
{
	CGroupChar *newChar;
	
	newChar = new CGroupChar;
	newChar->updateFromXML(node);
	if ( isNamePresent(newChar->getName()) == true || newChar->getName() == "") {
		print_debug(DEBUG_GROUP, "Adding new char FAILED. the name %s already existed.", 
				(const char *) newChar->getName());
		delete newChar;
		return false;
	} else {
		print_debug(DEBUG_GROUP, "Added new char. Name %s", 
				(const char *) newChar->getName());
		chars.append(newChar);
		return true;
	}
}

void CGroup::removeChar(QByteArray name)
{
	CGroupChar *ch;
	
	
	if (name == conf->getGroupManagerCharName()) 
		return; // just in case... should never happen
	
	for (int i = 0; i < chars.size(); i++)
		if (chars[i]->getName() == name) {
			print_debug(DEBUG_GROUP, "CGroup: removing the char from the list.");
			ch = chars[i];
			chars.remove(i);
			delete ch;
		}
}

void CGroup::removeChar(QDomNode node)
{
	QByteArray name = CGroupChar::getNameFromXML(node);
	if (name == "")
		return;
	
	removeChar(name);
}


bool CGroup::isNamePresent(QByteArray name)
{

	for (int i = 0; i < chars.size(); i++)
		if (chars[i]->getName() == name) {
			print_debug(DEBUG_GROUP, "The name %s is already present.", (const char *) name);
			return true;
		}
	
	return false;
}


CGroupChar* CGroup::getCharByName(QByteArray name)
{
	for (int i = 0; i < chars.size(); i++)
		if (chars[i]->getName() == name) 
			return chars[i];

	return NULL;
}

void CGroup::sendAllCharsData(CGroupClient *conn)
{
	for (int i = 0; i < chars.size(); i++)
		network->sendCharUpdate(conn, chars[i]->toXML());
}


void CGroup::updateChar(QDomNode blob)
{
	CGroupChar *ch;
	
	
	ch = getCharByName(CGroupChar::getNameFromXML(blob));
	if (ch == NULL)
		return;
	
	ch->updateFromXML(blob);
  // TODO: all teh shit here ...	
}


void CGroup::connectionRefused(QString message)
{
	print_debug(DEBUG_GROUP, "Connection refused: %s", (const char *) message.toAscii());
}

void CGroup::connectionFailed(QString message)
{
	print_debug(DEBUG_GROUP, "Failed to connect: %s", (const char *) message.toAscii());
}

void CGroup::connectionClosed(QString message)
{
	print_debug(DEBUG_GROUP, "Connection closed: %s", (const char *) message.toAscii());
}

void CGroup::connectionError(QString message)
{
	print_debug(DEBUG_GROUP, "Connection closed: %s", (const char *) message.toAscii());
}

void CGroup::serverStartupFailed(QString message)
{
	print_debug(DEBUG_GROUP, "Failed to start the Group server: %s", (const char *) message.toAscii());
}

void CGroup::gotKicked(QDomNode message)
{
	
	if (message.nodeName() != "data") {
    	print_debug(DEBUG_GROUP, "Called gotKicked with wrong node. No data node.");
		return;
	}
	
	QDomNode e = message.firstChildElement();
	
	if (e.nodeName() != "text") {
    	print_debug(DEBUG_GROUP, "Called gotKicked with wrong node. No text node.");
		return;
	}

	QDomElement text = e.toElement();
	print_debug(DEBUG_GROUP, "You got kicked! Reason [nodename %s] : %s", 
			(const char *) text.nodeName().toAscii(), 
			(const char *) text.text().toAscii());
}

void CGroup::gTellArrived(QDomNode node)
{
	
	if (node.nodeName() != "data") {
    	print_debug(DEBUG_GROUP, "Called gotKicked with wrong node. No data node.");
		return;
	}
	
	QDomNode e = node.firstChildElement();
	
	if (e.nodeName() != "gtell") {
    	print_debug(DEBUG_GROUP, "Called gotKicked with wrong node. No text node.");
		return;
	}

	QDomElement text = e.toElement();
	print_debug(DEBUG_GROUP, "GTell Arrived : %s", (const char *) text.text().toAscii());
}

void CGroup::sendGTell(QByteArray tell)
{
	network->sendGTell(tell);
}


