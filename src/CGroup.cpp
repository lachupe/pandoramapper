
#include <QApplication>
#include <QDesktopWidget>

#include "utils.h"
#include "CConfigurator.h"
#include "CGroup.h"
#include "CGroupCommunicator.h"
#include "CEngine.h"

CGroup::CGroup(QByteArray name, QWidget *parent)
: QWidget(parent, Qt::Tool)
{
	CGroupChar *ch;
	
	ch = new CGroupChar();
	
	ch->setName(name);
	ch->setPosition(1); // FIXME ... or does not really matter.
	ch->setColor(conf->getGroupManagerColor());
	chars.append(ch);
	self = ch;
	
	print_debug(DEBUG_GROUP, "Starting up the GroupManager.\r\n");
	network = new CGroupCommunicator(CGroupCommunicator::Off, this);
	network->changeType(conf->getGroupManagerState());
	
	
/*	
    QRect rect = app.desktop()->availableGeometry(-1);
    if (conf->get_window_rect().x() == 0 || conf->get_window_rect().x() >= rect.width() || 
        conf->get_window_rect().y() >= rect.height() ) {
        print_debug(DEBUG_SYSTEM && DEBUG_INTERFACE, "Autosettings for window size and position");
        int x, y, height, width;

        x = rect.width() / 3 * 2;
        y = 0;
        height = rect.height() / 3;
        width = rect.width() - x;

        conf->set_window_rect( x, y, width, height);        
    }
*/
	
	setWindowTitle("GroupManager");
	QApplication *app = qApp;
    QRect rect = app->desktop()->availableGeometry(-1);
    if (conf->getGroupManagerRect().x() == 0 || conf->getGroupManagerRect().x() >= rect.width() || 
        conf->getGroupManagerRect().y() >= rect.height() ) {
        print_debug(DEBUG_GROUP, "Autosettings for window size and position");
        int x, y, height, width;

        x = conf->getWindowRect().x() - (rect.width() / 3) ;
        y = conf->getWindowRect().y();
        width = rect.width() / 3;
        height = conf->getWindowRect().height() / 2;

        conf->setGroupManagerRect( QRect(x, y, width, height) );        
    }
    show();
    raise();
    setGeometry( conf->getGroupManagerRect() );

    if (conf->getShowGroupManager() == false)
    	hide();
    
	layout = new QGridLayout(this);
	layout->setVerticalSpacing(100);
	this->setLayout(layout);
	//status = new QFrame(this);
	//status->resize(conf->getGroupManagerRect().width(), conf->getGroupManagerRect().height());
	//status->setFrameStyle(QFrame::StyledPanel);
	//layout->addWidget(status);
	print_debug(DEBUG_GROUP, "Leaving the GroupManager constructor");
}


void CGroup::setType(int newState)
{
	network->changeType(newState);
}

void CGroup::resetChars()
{
	for (int i = 0; i < chars.size(); ++i) 
		delete chars[i];
	
	chars.clear();
}

CGroup::~CGroup()
{
	resetChars();
	delete layout;
	delete network;
}

void CGroup::resetName()
{
	if (self->getName() == conf->getGroupManagerCharName())
		return;
	
	self->setName(conf->getGroupManagerCharName());
	issueLocalCharUpdate();
}


void CGroup::resetColor()
{
	if (self->getColor() == conf->getGroupManagerColor() )
		return;
	
	self->setColor(conf->getGroupManagerColor());
	issueLocalCharUpdate();
}

void CGroup::setCharPosition(unsigned int pos)
{
	if (self->getPosition() != pos) {
		self->setPosition(pos);
		self->setLastMovement( engine->getLastMovement() );
		issueLocalCharUpdate();
	}
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
		layout->addWidget( newChar->getCharFrame(), layout->rowCount(), 0 );
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

			layout->removeWidget( ch->getCharFrame() );

			
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
	
	if (ch->updateFromXML(blob) == true)
		toggle_renderer_reaction(); // issue a redraw
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
	
//	QDomElement root = node.toElement();
	QString from = e.toElement().attribute("from");
		
		
	if (e.nodeName() != "gtell") {
    	print_debug(DEBUG_GROUP, "Called gotKicked with wrong node. No text node.");
		return;
	}

	QDomElement text = e.toElement();
	print_debug(DEBUG_GROUP, "GTell from %s, Arrived : %s", 
			(const char *) from.toAscii(), 
			(const char *) text.text().toAscii() );
	send_to_user("\r\n%s tells you [GT] '%s'.\r\n", 
			(const char *) from.toAscii(), 
			(const char *) text.text().toAscii() ); 
	send_to_user("\r\n");
	send_to_user( engine->getPrompt() );
}

void CGroup::sendGTell(QByteArray tell)
{
	network->sendGTell(tell);
}



