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

#include "CGroupChar.h"
#include "utils.h"
#include "CRoomManager.h"
#include "CConfigurator.h"

CGroupChar::CGroupChar(QTreeWidget* t) :
	charTable(t)
{
	print_debug(DEBUG_GROUP, "GroupChar Constructor");

	name = "Fistandantilus";
	pos = 0;
	hp = 999;
	maxhp = 999;
	moves = 999;
	maxmoves = 999;
	mana = 999;
	maxmana = 999;
	state = NORMAL;
	textHP = "Healthy";
	textMana = "Burning";
	textMoves = "Exhausted";
	lastMovement = "";

	arm = false;
	shld = false;
	str = false;
	bob = false;
	bls = false;
	sanc = false;

	blind = true;
	tblind.start();

	status = NORMAL;

	charItem = new QTreeWidgetItem(charTable);
	statusItem = new QTreeWidgetItem(charTable);

	print_debug(DEBUG_GROUP, "GroupChar Constructor done.");
}

CGroupChar::~CGroupChar()
{
	delete charItem;
	delete statusItem;
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

void CGroupChar::setSpellsFields()
{

	statusItem->setTextAlignment(0, Qt::AlignCenter);
	if (blind) {
		statusItem->setBackgroundColor(0, Qt::red);
	} else {
		statusItem->setBackgroundColor(0, Qt::darkGray);
	}
	statusItem->setText(0, "BLIND " + conf->calculateTimeElapsed(tblind) );

	charItem->setTextAlignment(5, Qt::AlignCenter);
	charItem->setBackgroundColor(5, arm ? Qt::green : Qt::darkGray);
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
	} else {
		charItem->setBackgroundColor(7, Qt::darkGray);
	}
	charItem->setText(7, "BLS " + conf->calculateTimeElapsed(tbless) );

	statusItem->setTextAlignment(7, Qt::AlignCenter);
	if (sanc) {
		statusItem->setBackgroundColor(7, Qt::green);
	} else {
		statusItem->setBackgroundColor(7, Qt::darkGray);
	}
	statusItem->setText(7, "SANC " + conf->calculateTimeElapsed(tsanc));

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
	} else 	if (textMoves == "Fainting") {
		col = QColor(qRgb(207, 47, 10));
	} else 	if (textMoves == "Exhausted") {
		col = QColor(qRgb(255, 0, 0));
	}
	charItem->setBackgroundColor(4, col);
	charItem->setText(4, textMoves);

	statusItem->setText(2, QString("%1/%2").arg(hp).arg(maxhp));
	statusItem->setText(3, QString("%1/%2").arg(mana).arg(maxmana));
	statusItem->setText(4, QString("%1/%2").arg(moves).arg(maxmoves));
}

void CGroupChar::setStatusFields()
{
	  statusItem->setTextAlignment(1, Qt::AlignCenter);
	  statusItem->setText(1, "STANDING");

/*
	switch (state) {
		case BASHED:
			labelState->setText("BASHED");
		break;
		case INCAPACITATED:
			labelState->setText("INCAP");
		break;
		case DEAD:
			labelState->setText("DEAD");
		break;
		default:
			labelState->setText("Normal");
			break;
	}
*/
}

void CGroupChar::updateLabels()
{
	setNameField(name);


	  if (pos == 0) {
		  setField(1, "The position of this users is unknown");
	  } else {
		  if (Map.tryLockForRead() == true) {
			    CRoom *r = Map.getRoom(pos);
				if (r == NULL)
					setField(1, "The position of this users is unknown");
				else
					setField(1,  r->getName());

				// QString("%1:%2").arg(r->id).arg( QString(r->getName()) )
			    Map.unlock();
			}
	  }

	  setScoreFields();
	  setSpellsFields();
	  setStatusFields();


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

	doc.appendChild(root);

	return root;
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

   	QDomElement e = node.toElement();

   	unsigned int newpos  = e.attribute("room").toInt();
   	if (newpos != pos) {
   		updated = true;
   		pos = newpos;
   	}


   	str = e.attribute("name").toAscii();
   	if (str != name) {
   		updated = true;
   		name = str;
   	}

   	str = e.attribute("lastMovement").toAscii();
   	if (str != lastMovement) {
   		updated = true;
   		lastMovement = str;
   	}


   	str = e.attribute("color").toAscii();
   	if (str != color.name().toAscii()) {
   		updated = true;
   		color = QColor(QString(str) );
   	}

//	printf("Tut 6.\r\n");

   	str = e.attribute("textHP").toAscii();
   	if (s != textHP) {
   		updated = true;
   		textHP = str;
   	}

   	str = e.attribute("textMana").toAscii();
   	if (s != textMana) {
   		updated = true;
   		textMana = str;
   	}

   	str = e.attribute("textMoves").toAscii();
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

//	printf("Tut 6.\r\n");

   	if (updated == true)
   		updateLabels();

//	printf("Tut 7.\r\n");

	return updated; // hrmpf!
}

QByteArray CGroupChar::getNameFromXML(QDomNode node)
{
    if (node.nodeName() != "playerData") {
    	print_debug(DEBUG_GROUP, "Called updateFromXML with wrong node. The name does not fit.");
    	return false;
    }

   	QDomElement e = node.toElement();

   	return e.attribute("name").toAscii();
}

