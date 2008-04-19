
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>

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
	layout->setVerticalSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);
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

void CGroup::resetAllChars()
{
	for (int i = 0; i < chars.size(); ++i) {
		delete chars[i];
	}
	chars.clear();
}

void CGroup::resetChars()
{
	for (int i = 0; i < chars.size(); ++i) {
		if (chars[i] != self)
			delete chars[i];
	}
	chars.clear();
	chars.append(self);
}

CGroup::~CGroup()
{
	resetAllChars();
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

QByteArray CGroup::getNameFromBlob(QDomNode blob)
{
	CGroupChar *newChar;
	
	newChar = new CGroupChar;
	newChar->updateFromXML(blob);

	QByteArray name = newChar->getName();
	delete newChar;

	return name;
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
		printf("LAYOUT [before]: Columns %i, Rows %i\r\n", layout->columnCount(), layout->rowCount());
		layout->addWidget( newChar->getCharFrame());
		printf("LAYOUT [after]: Columns %i, Rows %i\r\n", layout->columnCount(), layout->rowCount());
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
    QMessageBox::information(this, "groupManager", QString("Connection refused: %1.").arg(message));
}

void CGroup::connectionFailed(QString message)
{
	print_debug(DEBUG_GROUP, "Failed to connect: %s", (const char *) message.toAscii());
    QMessageBox::information(this, "groupManager", QString("Failed to connect: %1.").arg(message));
}

void CGroup::connectionClosed(QString message)
{
	print_debug(DEBUG_GROUP, "Connection closed: %s", (const char *) message.toAscii());
    QMessageBox::information(this, "groupManager", QString("Connection closed: %1.").arg(message));
}

void CGroup::connectionError(QString message)
{
	print_debug(DEBUG_GROUP, "Connection closed: %s", (const char *) message.toAscii());
    QMessageBox::information(this, "groupManager", QString("Connection error: %1.").arg(message));
}

void CGroup::serverStartupFailed(QString message)
{
	print_debug(DEBUG_GROUP, "Failed to start the Group server: %s", (const char *) message.toAscii());
    QMessageBox::information(this, "groupManager", QString("Failed to start the groupManager server: %1.").arg(message));
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
    
	QMessageBox::information(this, "groupManager", QString("You got kicked! Reason: %1.").arg(text.text()));
}

void CGroup::gTellArrived(QDomNode node)
{
	
	if (node.nodeName() != "data") {
    	print_debug(DEBUG_GROUP, "Called gTellArrived with wrong node. No data node.");
		return;
	}
	
	QDomNode e = node.firstChildElement();
	
//	QDomElement root = node.toElement();
	QString from = e.toElement().attribute("from");
		
		
	if (e.nodeName() != "gtell") {
    	print_debug(DEBUG_GROUP, "Called gTellArrived with wrong node. No text node.");
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


void CGroup::parseScoreInformation(QByteArray score)
{
	print_debug(DEBUG_GROUP, "Score line: %s", (const char *) score);

	
	if (score.contains("mana, ") == true) {
		score.replace(" hits, ", "/");
		score.replace(" mana, and ", "/");
		score.replace(" moves.", "");
		
		
		
		QString temp = score;
		QStringList list = temp.split('/');
		
		
		print_debug(DEBUG_GROUP, "Hp: %s", (const char *) list[0].toAscii());
		print_debug(DEBUG_GROUP, "Hp max: %s", (const char *) list[1].toAscii());
		print_debug(DEBUG_GROUP, "Mana: %s", (const char *) list[2].toAscii());
		print_debug(DEBUG_GROUP, "Max Mana: %s", (const char *) list[3].toAscii());
		print_debug(DEBUG_GROUP, "Moves: %s", (const char *) list[4].toAscii());
		print_debug(DEBUG_GROUP, "Max Moves: %s", (const char *) list[5].toAscii());

		self->setScore(list[0].toInt(), list[1].toInt(), list[2].toInt(), list[3].toInt(), 
						list[4].toInt(), list[5].toInt()			);

		issueLocalCharUpdate();

	} else {
		// 399/529 hits and 121/133 moves.
		score.replace(" hits and ", "/");
		score.replace(" moves.", "");
		
		
		
		QString temp = score;
		QStringList list = temp.split('/');
		
		
		print_debug(DEBUG_GROUP, "Hp: %s", (const char *) list[0].toAscii());
		print_debug(DEBUG_GROUP, "Hp max: %s", (const char *) list[1].toAscii());
		print_debug(DEBUG_GROUP, "Moves: %s", (const char *) list[2].toAscii());
		print_debug(DEBUG_GROUP, "Max Moves: %s", (const char *) list[3].toAscii());

		self->setScore(list[0].toInt(), list[1].toInt(), "--", "--", 
						list[2].toInt(), list[3].toInt()			);

		issueLocalCharUpdate();
	}
}

void CGroup::parsePromptInformation(QByteArray prompt)
{
	QByteArray hp, mana, moves;
	
	if (prompt.indexOf('>') == -1)
		return; // false prompt

	hp = "Healthy";
	mana = "Full";
	moves = "A lot";
	
	int index = prompt.indexOf("HP:");
	if (index != -1) {
		hp = "";
		int k = index + 3;
		while (prompt[k] != ' ' && prompt[k] != '>' )
			hp += prompt[k++];
	}

	index = prompt.indexOf("Mana:");
	if (index != -1) {
		mana = "";
		int k = index + 5;
		while (prompt[k] != ' ' && prompt[k] != '>' )
			mana += prompt[k++];
	}

	index = prompt.indexOf("Move:");
	if (index != -1) {
		moves = "";
		int k = index + 5;
		while (prompt[k] != ' ' && prompt[k] != '>' )
			moves += prompt[k++];
	}

	self->setTextScore(hp, mana, moves);
}

void CGroup::parseStateChangeLine(int message, QByteArray line)
{
	
}

