#include "CGroupChar.h"
#include "utils.h"
#include "CRoomManager.h"

CGroupChar::CGroupChar()
{
	name = "";
	pos = 0;
	hp = 0;
	maxhp = 0;
	moves = 0;
	maxmoves = 0;
	mana = 0;
	maxmana = 0;
	state = NORMAL;
	textHP = "Healthy";
	textMoves = "Full";
	textMana = "Full";

	// create the info-labels
	labelName = new QLabel;
	labelRoom = new QLabel;
	labelHpText = new QLabel;
	labelHpInt = new QLabel;
	labelManaText = new QLabel;
	labelManaInt = new QLabel;
	labelMovesText = new QLabel;
	labelMovesInt = new QLabel;
	labelState = new QLabel;

	// setup the layout of labels
	layout = new QGridLayout;
	layout->addWidget(labelName, 0, 0, 2, 0);
	layout->addWidget(labelState, 0, 1, 2, 1);
	layout->addWidget(labelRoom, 3, 0, 3, 1);
	layout->addWidget(labelHpText, 0, 2, 1, 2);
	layout->addWidget(labelHpInt, 2, 2, 3, 2);
	layout->addWidget(labelManaText, 0, 3, 1, 3);
	layout->addWidget(labelManaInt, 2, 3, 3, 3);
	layout->addWidget(labelMovesText, 0, 4, 1, 4);
	layout->addWidget(labelMovesInt, 2, 4, 3, 4);

	// setup the stretching factors
	layout->setColumnStretch(0, 25);
	layout->setColumnStretch(1, 15);
	layout->setColumnStretch(2, 8);
	layout->setColumnStretch(3, 8);
	layout->setColumnStretch(4, 8);
	
	
	charFrame = new QFrame;
	charFrame->setFrameStyle(QFrame::StyledPanel);
	charFrame->setLayout(layout);

}

CGroupChar::~CGroupChar()
{
	delete layout;
	delete labelName;
	delete labelRoom;
	delete labelHpText;
	delete labelHpInt;
	delete labelManaText;
	delete labelManaInt;
	delete labelMovesText;
	delete labelMovesInt;
	delete labelState;
}

void CGroupChar::updateLabels()
{
	labelName->setText(name);
	
	if (pos == 0) {
		labelRoom->setText("Unknown");
	} else {
		CRoom *r = Map.getRoom(pos);
		if (r == NULL)
			labelRoom->setText("Unknown");
		else 
			labelRoom->setText(QString("%1:%2").arg(r->id).arg( QString(r->getName()) ) );
	}
		
	labelHpText->setText(textHP);
	
	labelHpInt->setText( QString("%1/%2").arg(hp).arg(maxhp) );
	labelManaText->setText(textMana);
	labelManaInt->setText( QString("%1/%2").arg(mana).arg(maxmana) );
	labelMovesText->setText(textMoves);
	labelMovesInt->setText( QString("%1/%2").arg(moves).arg(maxmoves) );

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

   	str = e.attribute("color").toAscii();
   	if (str != color.name().toAscii()) {
   		updated = true;
   		color = QColor(QString(str) );
   	}

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
   	
   	
   	if (updated == true)
   		updateLabels();
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

