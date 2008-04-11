#include "CGroupChar.h"
#include "utils.h"

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
	textHP = "";
	textMoves = "";
	textMana = "";
}

CGroupChar::~CGroupChar()
{
}

QDomNode CGroupChar::toXML()
{
	
	QDomDocument doc("charinfo");

	QDomElement root = doc.createElement("playerData");
	root.setAttribute("room", pos );
	root.setAttribute("name", QString(name) );
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

