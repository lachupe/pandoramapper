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

#include "GroupManager/CGroup.h"
#include "GroupManager/CGroupChar.h"

#include "utils.h"
#include "CConfigurator.h"

#include "Map/CRoomManager.h"


CGroupChar::CGroupChar(CGroup *_parent, QTreeWidget* t) :
	parent(_parent), charTable(t)
{
	print_debug(DEBUG_GROUP, "GroupChar Constructor");

	name = "Fistandantilus";
	pos = 1;
	hp = 999;
	maxhp = 999;
	moves = 999;
	maxmoves = 999;
	mana = 999;
	maxmana = 999;
	state = STANDING;
	textHP = "Wounded";
	textMana = "Burning";
	textMoves = "Exhausted";
	lastMovement = "";

	arm = false;
	shld = false;
	str = false;
	bob = false;
	bls = false;
	sanc = false;

	blind = false;
	//tblind.start();
	blind_elapsed = 0;
	sanc_elapsed = 0;
	bless_elapsed = 0;

	color = conf->getGroupManagerColor();


	charItem = new QTreeWidgetItem(charTable);
	statusItem = new QTreeWidgetItem(charTable);

	hidden = false;

	print_debug(DEBUG_GROUP, "GroupChar Constructor done.");
}

CGroupChar::~CGroupChar()
{
	delete charItem;
	delete statusItem;
}

void CGroupChar::setHidden(bool b)
{
	hidden = b;
	charItem->setHidden(b);
	statusItem->setHidden(b);
	//charTable->collapseItem(charItem);
}

void CGroupChar::setNameField(QString name)
{
	charItem->setTextAlignment(0, Qt::AlignCenter);
	charItem->setText(0, name);
	charItem->setBackgroundColor(0, color);
	if (color.value() < 150)
		charItem->setTextColor(0, Qt::white);
	else
		charItem->setTextColor(0, Qt::black);
}

void CGroupChar::setField(int i, QString text)
{
	charItem->setText(i, text);
}

QString CGroupChar::calculateTimeElapsed(QTime& timer, int delay)
{
    QString s;
    int min;
    int sec;

    if (delay == -1) {
    	return " UNDEF ";
    }

    sec = (timer.elapsed() + delay) / 1000;
    min = sec / 60;
    sec = sec % 60;

    s = QString("- %1%2:%3%4")
            .arg( min / 10 )
            .arg( min % 10 )
            .arg( sec / 10 )
            .arg( sec % 10 );

    return s;
}

// TODO: Evil hack!
// FIXME: Evil hack!
void CGroupChar::updateSpells()
{
    for (unsigned int p = 0; p < conf->spells.size(); p++) {
    	if (conf->spells[p].name == "armour") {
			bool arm_new = conf->spells[p].up || conf->spells[p].silently_up;
			if (arm & !arm_new && conf->getGroupManagerNotifyArmour() ) {
				parent->sendGTell("My ARMOUR is DOWN!");
			}
			if (!arm && arm_new && conf->getGroupManagerNotifyArmour()) {
				parent->sendGTell("Armour spell restored!");
			}
			arm = arm_new;
			continue;
    	}
    	if (conf->spells[p].name == "breath of briskness") {
			bob = conf->spells[p].up || conf->spells[p].silently_up;
			continue;
    	}
    	if (conf->spells[p].name == "shield") {
			shld = conf->spells[p].up || conf->spells[p].silently_up;
			continue;
    	}
    	if (conf->spells[p].name == "strength") {
			str = conf->spells[p].up || conf->spells[p].silently_up;
			continue;
    	}
    	if (conf->spells[p].name == "bless") {
			bls = conf->spells[p].up || conf->spells[p].silently_up;
			tbless = conf->spells[p].timer;
			bless_elapsed = 0;
			continue;
    	}
    	if (conf->spells[p].name == "sanctuary") {
			bool sanc_new = conf->spells[p].up || conf->spells[p].silently_up;
			if (sanc & !sanc_new && conf->getGroupManagerNotifySanc())
				parent->sendGTell("My SANC is DOWN!");
			if (!sanc && sanc_new && conf->getGroupManagerNotifySanc())
				parent->sendGTell("Sanc spell is up!");
			sanc = sanc_new;
			tsanc = conf->spells[p].timer;

			// time is not know, the effect got up while sleeping for example
			if ( !conf->spells[p].up && conf->spells[p].silently_up )
				sanc_elapsed = -1;
			else
				sanc_elapsed = 0;
			continue;
    	}
    	if (conf->spells[p].name == "blindness") {
			blind = conf->spells[p].up || conf->spells[p].silently_up;
			tblind = conf->spells[p].timer;

			if ( !conf->spells[p].up && conf->spells[p].silently_up )
				blind_elapsed = -1;
			else
				blind_elapsed = 0;

			blind_elapsed = 0;
			continue;
    	}
    }
}

void CGroupChar::setSpellsFields()
{

	statusItem->setTextAlignment(0, Qt::AlignCenter);
	if (blind) {
		statusItem->setBackgroundColor(0, Qt::red);
		statusItem->setText(0, "BLIND " + calculateTimeElapsed(tblind, blind_elapsed) );
	} else {
		statusItem->setBackgroundColor(0, Qt::darkGray);
		statusItem->setText(0, "BLIND - 00:00");
	}

	charItem->setTextAlignment(5, Qt::AlignCenter);
	charItem->setBackgroundColor(5, arm ? Qt::green : Qt::red);
	charItem->setText(5, "ARM");

	statusItem->setTextAlignment(5, Qt::AlignCenter);
	statusItem->setBackgroundColor(5, shld ? Qt::green : Qt::darkGray);
	statusItem->setText(5, "SHLD");

	charItem->setTextAlignment(6, Qt::AlignCenter);
	charItem->setBackgroundColor(6, bob ? Qt::green : Qt::darkGray);
	charItem->setText(6, "BOB");

	statusItem->setTextAlignment(6, Qt::AlignCenter);
	statusItem->setBackgroundColor(6, str ? Qt::green : Qt::darkGray);
	statusItem->setText(6, "STR");

	charItem->setTextAlignment(7, Qt::AlignCenter);
	if (bls) {
		charItem->setBackgroundColor(7, Qt::green);
		charItem->setText(7, "BLESS " + calculateTimeElapsed(tbless, bless_elapsed) );
	} else {
		charItem->setBackgroundColor(7, Qt::darkGray);
		charItem->setText(7, "BLESS - 00:00");
	}

	statusItem->setTextAlignment(7, Qt::AlignCenter);
	if (sanc) {
		statusItem->setBackgroundColor(7, Qt::green);
		statusItem->setText(7, "SANC " + calculateTimeElapsed(tsanc, sanc_elapsed));
	} else {
		statusItem->setBackgroundColor(7, Qt::darkGray);
		statusItem->setText(7, "SANC - 00:00");
	}
}


void CGroupChar::setScoreFields()
{
	statusItem->setTextAlignment(2, Qt::AlignCenter);
	statusItem->setTextAlignment(3, Qt::AlignCenter);
	statusItem->setTextAlignment(4, Qt::AlignCenter);
	charItem->setTextAlignment(2, Qt::AlignCenter);
	charItem->setTextAlignment(3, Qt::AlignCenter);
	charItem->setTextAlignment(4, Qt::AlignCenter);

	QColor col;
	col = QColor(qRgb(120, 249, 16));
	if (textHP == "Hurt") {
		col = QColor(qRgb(184, 243, 113));
	} else 	if (textHP == "Wounded") {
		col = QColor(qRgb(249, 251, 92));
	} else 	if (textHP == "Bad") {
		col = QColor(qRgb(224, 17, 31));
	} else 	if (textHP == "Awful") {
		col = QColor(qRgb(255, 0, 0));
	}
	charItem->setBackgroundColor(2, col);
	charItem->setText(2, textHP);


	col = QColor(qRgb(2, 0, 255));
	if (textMana == "Hot") {
		col = QColor(qRgb(184, 243, 113));
	} else 	if (textMana == "Warm") {
		col = QColor(qRgb(0, 79, 255));
	} else 	if (textMana == "Cold") {
		col = QColor(qRgb(0, 183, 255));
	} else 	if (textMana == "Icy") {
		col = QColor(qRgb(0, 255, 247));
	}
	charItem->setBackgroundColor(3, col);
	charItem->setText(3, textMana);

	col = QColor(qRgb(94, 36, 14));
	if (textMoves == "Tired") {
		col = QColor(qRgb(162, 47, 4));
	} else 	if (textMoves == "Slow") {
		col = QColor(qRgb(207, 64, 10));
	} else 	if (textMoves == "Weak") {
		col = QColor(qRgb(207, 47, 5));
	} else 	if (textMoves == "Fainting") {
		col = QColor(qRgb(237, 47, 0));
	} else 	if (textMoves == "Exhausted") {
		col = QColor(qRgb(255, 0, 0));
	}
	charItem->setBackgroundColor(4, col);
	charItem->setText(4, textMoves);

	statusItem->setText(2, QString("%1/%2").arg(hp).arg(maxhp));
	statusItem->setText(3, QString("%1/%2").arg(mana).arg(maxmana));
	statusItem->setText(4, QString("%1/%2").arg(moves).arg(maxmoves));
}

void CGroupChar::setStateFields()
{
	statusItem->setTextAlignment(1, Qt::AlignCenter);

	switch (state) {
		case STANDING:
			statusItem->setBackgroundColor(1, Qt::white);
			statusItem->setText(1, "STANDING");
			break;
		case ENGAGED:
			statusItem->setBackgroundColor(1, Qt::yellow);
			statusItem->setText(1, "ENGAGED");
			break;
		case BASHED:
			statusItem->setBackgroundColor(1, Qt::red);
			statusItem->setText(1, "BASHED");
		break;
		case SLEEPING:
			statusItem->setBackgroundColor(1, Qt::blue);
			statusItem->setText(1, "SLEEPING");
			break;
		case RESTING:
			statusItem->setBackgroundColor(1, Qt::gray);
			statusItem->setText(1, "RESTING");
			break;
		case DEAD:
			statusItem->setBackgroundColor(1, Qt::black);
			statusItem->setText(1, "DEAD");
			break;
		case INCAP:
			statusItem->setBackgroundColor(1, Qt::magenta);
			statusItem->setText(1, "INCAP");
		break;
	}


}

void CGroupChar::updateLabels()
{
	setNameField(name);


	  if (pos == 0) {
		  setField(1, "The position of this users is unknown");
	  } else {
			CRoom *r = Map.getRoom(pos);
			if (r == NULL)
				setField(1, "The position of this users is unknown");
			else
				setField(1,  r->getName());

			// QString("%1:%2").arg(r->id).arg( QString(r->getName()) )
	  }

	  setScoreFields();
	  setSpellsFields();
	  setStateFields();


}


bool CGroupChar::updateFromXML(QDomNode node)
{
	bool updated;


	updated = false;
    if (node.nodeName() != "playerData") {
    	print_debug(DEBUG_GROUP, "Called updateFromXML with wrong node. The name does not fit.");
    	return false;
    }

   	QString s;
   	QByteArray str;
   	int newval;
   	bool spell;

   	QDomElement e = node.toElement();

   	unsigned int newpos  = e.attribute("room").toInt();
   	if (newpos != pos) {
   		updated = true;
   		pos = newpos;
   	}


    str = e.attribute("name").toLocal8Bit();
   	if (str != name) {
   		updated = true;
   		name = str;
   	}

    str = e.attribute("lastMovement").toLocal8Bit();
   	if (str != lastMovement) {
   		updated = true;
   		lastMovement = str;
   	}


    str = e.attribute("color").toLocal8Bit();
    if (str != color.name().toLocal8Bit()) {
   		updated = true;
   		color = QColor(QString(str) );
   	}

    str = e.attribute("textHP").toLocal8Bit();
   	if (s != textHP) {
   		updated = true;
   		textHP = str;
   	}

    str = e.attribute("textMana").toLocal8Bit();
   	if (s != textMana) {
   		updated = true;
   		textMana = str;
   	}

    str = e.attribute("textMoves").toLocal8Bit();
   	if (s != textMoves) {
   		updated = true;
   		textMoves = str;
   	}

   	newval  = e.attribute("hp").toInt();
   	if (newval != hp) {
   		updated = true;
   		hp = newval;
   	}

   	newval  = e.attribute("maxhp").toInt();
   	if (newval != maxhp) {
   		updated = true;
   		maxhp = newval;
   	}

   	newval  = e.attribute("mana").toInt();
   	if (newval != mana) {
   		updated = true;
   		mana = newval;
   	}

   	newval  = e.attribute("maxmana").toInt();
   	if (newval != maxmana) {
   		updated = true;
   		maxmana = newval;
   	}

   	newval  = e.attribute("moves").toInt();
   	if (newval != moves) {
   		updated = true;
   		moves = newval;
   	}

   	newval  = e.attribute("maxmoves").toInt();
   	if (newval != maxmoves) {
   		updated = true;
   		maxmoves = newval;
   	}

   	newval  = e.attribute("state").toInt();
   	if (newval != state) {
   		updated = true;
   		state = newval;
   	}

/*
	root.setAttribute("arm", arm ? "up" : "down" );
	root.setAttribute("shld", shld ? "up" : "down" );
	root.setAttribute("str", str ? "up" : "down" );
	root.setAttribute("bob", bob ? "up" : "down" );
	root.setAttribute("bless", bless ? "up" : "down" );
	root.setAttribute("sanc", sanc ? "up" : "down" );
	root.setAttribute("blind", blind ? "up" : "down" );
*/


    spell = (e.attribute("arm").toLocal8Bit() == "true");
   	if (spell != arm) {
   		updated = true;
   		arm = spell;
   	}

    spell = (e.attribute("shld").toLocal8Bit() == "true");
   	if (spell != shld) {
   		updated = true;
   		shld = spell;
   	}

    spell = (e.attribute("str").toLocal8Bit() == "true");
   	if (spell != CGroupChar::str) {
   		updated = true;
   		CGroupChar::str = spell;
   	}

    spell = (e.attribute("bob").toLocal8Bit() == "true");
   	if (spell != bob) {
   		updated = true;
   		bob = spell;
   	}

    spell = (e.attribute("bless").toLocal8Bit() == "true");
   	if (spell != bls) {
   		updated = true;
   		bls = spell;
   	}
   	tbless.restart();
   	bless_elapsed = e.attribute("blessTimer").toInt();

    spell = (e.attribute("sanc").toLocal8Bit() == "true");
   	if (spell != sanc) {
   		updated = true;
   		sanc = spell;
   	}
   	tsanc.restart();
   	sanc_elapsed = e.attribute("sancTimer").toInt();

    spell = (e.attribute("blind").toLocal8Bit() == "true");
   	if (spell != blind) {
   		updated = true;
   		blind = spell;
   	}
   	tblind.restart();
   	blind_elapsed = e.attribute("blindTimer").toInt();


   	if (updated == true)
   		updateLabels();


	return updated; // hrmpf!
}

QByteArray CGroupChar::getNameFromXML(QDomNode node)
{

    if (node.nodeName() != "playerData") {
    	print_debug(DEBUG_GROUP, "Called updateFromXML with wrong node. The name does not fit.");
    	return "Error"; // no idea what to return
    }

   	QDomElement e = node.toElement();

    return e.attribute("name").toLocal8Bit();
}



QDomNode CGroupChar::toXML()
{

	QDomDocument doc("charinfo");

	QDomElement root = doc.createElement("playerData");
	root.setAttribute("room", pos );
	root.setAttribute("name", QString(name) );
	root.setAttribute("color", color.name() );
	root.setAttribute("textHP", QString(textHP) );
	root.setAttribute("textMana", QString(textMana) );
	root.setAttribute("textMoves", QString(textMoves) );
	root.setAttribute("hp", hp );
	root.setAttribute("maxhp", maxhp );
	root.setAttribute("mana", mana);
	root.setAttribute("maxmana", maxmana);
	root.setAttribute("moves", moves );
	root.setAttribute("maxmoves", maxmoves );
	root.setAttribute("state", state );
	root.setAttribute("lastMovement", QString(lastMovement));

	// spells
	root.setAttribute("arm", arm ? "true" : "false" );
	root.setAttribute("shld", shld ? "true" : "false" );
	root.setAttribute("str", str ? "true" : "false" );
	root.setAttribute("bob", bob ? "true" : "false" );
	root.setAttribute("bless", bls ? "true" : "false" );
	root.setAttribute("blessTimer", tbless.elapsed() + bless_elapsed );

	root.setAttribute("sanc", sanc ? "true" : "false" );
	if (sanc_elapsed == -1) {
		root.setAttribute("sancTimer", -1 );
	} else {
		root.setAttribute("sancTimer", tsanc.elapsed() + sanc_elapsed );
	}

	root.setAttribute("blind", blind ? "true" : "false" );
	if ( blind_elapsed == -1 ) {
		root.setAttribute("blindTimer", -1 );
	} else {
		root.setAttribute("blindTimer", tblind.elapsed() + blind_elapsed);
	}


	doc.appendChild(root);
	return root;
}


QDomNode CGroupChar::promptToXML()
{
	QDomDocument doc("charinfo");

	QDomElement root = doc.createElement("playerData");
	root.setAttribute("name", QString(name) );
	root.setAttribute("textHP", QString(textHP) );
	root.setAttribute("textMana", QString(textMana) );
	root.setAttribute("textMoves", QString(textMoves) );

	doc.appendChild(root);

	return root;
}

QDomNode CGroupChar::positionToXML()
{
	QDomDocument doc("charinfo");

	QDomElement root = doc.createElement("playerData");
	root.setAttribute("room", pos );
	root.setAttribute("name", QString(name) );

	doc.appendChild(root);
	return root;
}

QDomNode CGroupChar::scoreToXML()
{
	QDomDocument doc("charinfo");

	QDomElement root = doc.createElement("playerData");
	root.setAttribute("name", QString(name) );
	root.setAttribute("hp", hp );
	root.setAttribute("maxhp", maxhp );
	root.setAttribute("mana", mana);
	root.setAttribute("maxmana", maxmana);
	root.setAttribute("moves", moves );
	root.setAttribute("maxmoves", maxmoves );

	doc.appendChild(root);
	return root;
}


QDomNode CGroupChar::stateToXML()
{
	QDomDocument doc("charinfo");

	QDomElement root = doc.createElement("playerData");
	root.setAttribute("name", QString(name) );
	root.setAttribute("state", state );

	doc.appendChild(root);
	return root;
}



bool CGroupChar::updatePositionFromXML(QDomNode node)
{
	bool updated;

	updated = false;
    if (node.nodeName() != "playerData") {
    	print_debug(DEBUG_GROUP, "Called updatePositionFromXML with wrong node. The name does not fit.");
    	return false;
    }

   	QString s;
   	QByteArray str;

   	QDomElement e = node.toElement();

   	unsigned int newpos  = e.attribute("room").toInt();
   	if (newpos != pos) {
   		updated = true;
   		pos = newpos;
   	}

   	if (updated == true)
   		updateLabels();

	return updated; // hrmpf!
}


bool CGroupChar::updateScoreFromXML(QDomNode node)
{
	bool updated;

	updated = false;
    if (node.nodeName() != "playerData") {
    	print_debug(DEBUG_GROUP, "Called updateScoreFromXML with wrong node. The name does not fit.");
    	return false;
    }

   	QString s;
   	QByteArray str;
   	int newval;

   	QDomElement e = node.toElement();

	newval  = e.attribute("hp").toInt();
	if (newval != hp) {
		updated = true;
		hp = newval;
	}

	newval  = e.attribute("maxhp").toInt();
	if (newval != maxhp) {
		updated = true;
		maxhp = newval;
	}

	newval  = e.attribute("mana").toInt();
	if (newval != mana) {
		updated = true;
		mana = newval;
	}

	newval  = e.attribute("maxmana").toInt();
	if (newval != maxmana) {
		updated = true;
		maxmana = newval;
	}

	newval  = e.attribute("moves").toInt();
	if (newval != moves) {
		updated = true;
		moves = newval;
	}

	newval  = e.attribute("maxmoves").toInt();
	if (newval != maxmoves) {
		updated = true;
		maxmoves = newval;
	}

   	if (updated == true)
   		updateLabels();

	return updated; // hrmpf!
}


bool CGroupChar::updateStateFromXML(QDomNode node)
{
	bool updated;

	updated = false;
    if (node.nodeName() != "playerData") {
    	print_debug(DEBUG_GROUP, "Called updateScoreFromXML with wrong node. The name does not fit.");
    	return false;
    }

   	int newval;

   	QDomElement e = node.toElement();

	newval  = e.attribute("state").toInt();
	if (newval != state) {
		updated = true;
		state = newval;
	}

   	if (updated == true)
   		updateLabels();

	return updated; // hrmpf!
}



bool CGroupChar::updatePromptFromXML(QDomNode node)
{
	bool updated;

	updated = false;
    if (node.nodeName() != "playerData") {
    	print_debug(DEBUG_GROUP, "Called updatePromptFromXML with wrong node. The name does not fit.");
    	return false;
    }

   	QString s;
   	QByteArray str;

   	QDomElement e = node.toElement();

    str = e.attribute("textHP").toLocal8Bit();
   	if (s != textHP) {
   		updated = true;
   		textHP = str;
   	}

    str = e.attribute("textMana").toLocal8Bit();
   	if (s != textMana) {
   		updated = true;
   		textMana = str;
   	}

    str = e.attribute("textMoves").toLocal8Bit();
   	if (s != textMoves) {
   		updated = true;
   		textMoves = str;
   	}

   	if (updated == true)
   		updateLabels();

	return updated; // hrmpf!
}


