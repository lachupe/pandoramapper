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
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QTimer>

#include "utils.h"
#include "CConfigurator.h"
#include "CGroup.h"
#include "CGroupCommunicator.h"
#include "CEngine.h"

CGroup::CGroup(QByteArray name, QWidget *parent) : QDialog(parent)
{
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
        height = conf->getWindowRect().height() / 4;

        conf->setGroupManagerRect( QRect(x, y, width, height) );
    }
    setGeometry( conf->getGroupManagerRect() );

	tree = new QTreeWidget(this);
	tree->setColumnCount(8);
	tree->setHeaderHidden(true);
//	tree->setHeaderLabels(QStringList() << tr("Name") << tr("Room Name") << tr("HP") << tr("Mana") << tr("Moves") <<
//			tr("Spells") << tr("Spells") << tr("Spells"));
	tree->setRootIsDecorated(false);
	tree->setAlternatingRowColors(true);
	tree->setSelectionMode(QAbstractItemView::NoSelection);
	tree->clear();
//	tree->hide();


	QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(tree);

    this->setLayout(layout);


    show();
    raise();


	CGroupChar *ch = new CGroupChar(tree);

	chars.clear();
	chars.append(ch);
	self = ch;
	ch->updateLabels();

	tree->resizeColumnToContents(0);
	tree->resizeColumnToContents(1);
	tree->resizeColumnToContents(2);
	tree->resizeColumnToContents(3);
	tree->resizeColumnToContents(4);
	tree->resizeColumnToContents(5);
	tree->resizeColumnToContents(6);
	tree->resizeColumnToContents(7);


	ch->setName(name);



	print_debug(DEBUG_GROUP, "Starting up the GroupManager.\r\n");
	network = new CGroupCommunicator(CGroupCommunicator::Off, this);
	network->changeType(conf->getGroupManagerState());

    if (conf->getShowGroupManager() == false)
    	hide();

	if (conf->getGroupManagerShowSelf() == false)
		hideSelf();

    // initialize timed updates of the group manager frame

    //startTimer( 0 );                            // run continuous timer
    QTimer * counter = new QTimer( this );
    connect( counter, SIGNAL(timeout()), this, SLOT(updateGroupManagerWindow()) );
    counter->start( 500 );


	print_debug(DEBUG_GROUP, "Leaving the GroupManager constructor");
}

void CGroup::hideSelf()
{
	self->setHidden( true );
	if (chars.contains(self) == true) {
		removeChar(conf->getGroupManagerCharName());
	}
}


void CGroup::addSelf()
{
	self->setHidden( false );
	if (chars.contains(self) != true) {
		chars.append(self);
	}
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
	delete tree;
	delete network;
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
		issueLocalCharPositionUpdate();
	}
}

QByteArray CGroup::getNameFromBlob(QDomNode blob)
{
	CGroupChar *newChar;

	newChar = new CGroupChar(tree);
	newChar->updateFromXML(blob);

	QByteArray name = newChar->getName();
	delete newChar;

	return name;
}


bool CGroup::addChar(QDomNode node)
{
	CGroupChar *newChar;

	newChar = new CGroupChar(tree);
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


	/*
	if (name == conf->getGroupManagerCharName())
		return;
	*/

	for (int i = 0; i < chars.size(); i++)
		if (chars[i]->getName() == name) {
			print_debug(DEBUG_GROUP, "CGroup: removing the char from the list.");
			ch = chars[i];
			chars.remove(i);

			if (name != conf->getGroupManagerCharName())
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

// UPDATERS
void CGroup::updateChar(QDomNode blob)
{
	CGroupChar *ch = getCharByName(CGroupChar::getNameFromXML(blob));
	if (ch == NULL)
		return;

	if (ch->updateFromXML(blob) == true)
		toggle_renderer_reaction(); // issue a redraw
}

void CGroup::updateCharPosition(QDomNode blob)
{
	CGroupChar *ch = getCharByName(CGroupChar::getNameFromXML(blob));
	if (ch == NULL)
		return;

	if (ch->updatePositionFromXML(blob) == true)
		toggle_renderer_reaction(); // issue a redraw
}

void CGroup::updateCharScore(QDomNode blob)
{
	CGroupChar *ch = getCharByName(CGroupChar::getNameFromXML(blob));
	if (ch == NULL)
		return;

	ch->updateScoreFromXML(blob);
}

void CGroup::updateCharPrompt(QDomNode blob)
{
	CGroupChar *ch = getCharByName(CGroupChar::getNameFromXML(blob));
	if (ch == NULL)
		return;

	ch->updatePromptFromXML(blob);
}



void CGroup::connectionRefused(QString message)
{
	print_debug(DEBUG_GROUP, "Connection refused: %s", (const char *) message.toAscii());
	if (network->getType() == CGroupCommunicator::Client)
		QMessageBox::information(this, "groupManager", QString("Connection refused: %1.").arg(message));
}

void CGroup::connectionFailed(QString message)
{
	print_debug(DEBUG_GROUP, "Failed to connect: %s", (const char *) message.toAscii());
	if (network->getType() == CGroupCommunicator::Client)
		QMessageBox::information(this, "groupManager", QString("Failed to connect: %1.").arg(message));
}

void CGroup::connectionClosed(QString message)
{
	print_debug(DEBUG_GROUP, "Connection closed: %s", (const char *) message.toAscii());

	if (network->getType() == CGroupCommunicator::Client)
		QMessageBox::information(this, "groupManager", QString("Connection closed: %1.").arg(message));
}

void CGroup::connectionError(QString message)
{
	print_debug(DEBUG_GROUP, "Connection error: %s", (const char *) message.toAscii());
	if (network->getType() == CGroupCommunicator::Client)
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

	// somehow this always leads to crash! :-(
	//QMessageBox::critical(this, "groupManager", QString("You got kicked! Reason: %1.").arg(text.text()));

	print_debug(DEBUG_GROUP, "You got kicked! Reason [nodename %s] : %s",
			(const char *) text.nodeName().toAscii(),
			(const char *) text.text().toAscii());

	send_to_user("--[ You got kicked! Reason [nodename %s] : %s",
			(const char *) text.nodeName().toAscii(),
			(const char *) text.text().toAscii());

	send_to_user("\r\n");
	send_prompt();
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
	send_prompt();
}

void CGroup::sendGTell(QByteArray tell)
{
	network->sendGTell(tell);
}

void CGroup::resetName()
{
	if (self->getName() == conf->getGroupManagerCharName())
		return;

	QByteArray oldname = self->getName();
	QByteArray newname = conf->getGroupManagerCharName();

	network->sendUpdateName(oldname, newname);
	network->renameConnection(oldname, newname);

	self->setName(conf->getGroupManagerCharName());
}

void CGroup::updateGroupManagerWindow()
{
	for (int i = 0; i < chars.size(); ++i) {
		chars[i]->updateLabels();
	}
}


void CGroup::renameChar(QDomNode blob)
{
	if (blob.nodeName() != "data") {
    	print_debug(DEBUG_GROUP, "Called renameChar with wrong node. No data node.");
		return;
	}

	QDomNode e = blob.firstChildElement();

//	QDomElement root = node.toElement();
	QString oldname = e.toElement().attribute("oldname");
	QString newname = e.toElement().attribute("newname");


	print_debug(DEBUG_GROUP, "Renaming a char from %s to %s",
			(const char *) oldname.toAscii(),
			(const char *) newname.toAscii() );

	CGroupChar *ch;
	ch = getCharByName(oldname.toAscii());
	if (ch == NULL)
		return;

	ch->setName(newname.toAscii());
	ch->updateLabels();
}

void CGroup::updateSpellsInfo()
{
	self->updateSpells();
	issueLocalCharUpdate();
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

		self->setScore(list[0].toInt(), list[1].toInt(), list[2].toInt(), list[3].toInt(),
						list[4].toInt(), list[5].toInt()			);

		issueLocalCharScoreUpdate();

	} else {
		// 399/529 hits and 121/133 moves.
		score.replace(" hits and ", "/");
		score.replace(" moves.", "");

		QString temp = score;
		QStringList list = temp.split('/');
		self->setScore(list[0].toInt(), list[1].toInt(), 0, 0,
						list[2].toInt(), list[3].toInt()			);

		issueLocalCharScoreUpdate();
	}
}

void CGroup::parsePromptInformation(QByteArray prompt)
{
	QByteArray hp, mana, moves;

	if (prompt.indexOf('>') == -1)
		return; // false prompt

	hp = "Healthy";
	mana = "Full";
	moves = "Lots";

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
	issueLocalCharPromptUpdate();
}

void CGroup::parseStateChangeLine(int message, QByteArray line)
{

}

