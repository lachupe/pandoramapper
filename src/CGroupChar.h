#ifndef CGROUPCHAR_H_
#define CGROUPCHAR_H_

#include <QByteArray>
#include <QDomNode>

class CGroupChar
{
	unsigned int pos;
	QByteArray name;
	QByteArray textHP;
	QByteArray textMoves;
	QByteArray textMana;
	int hp, maxhp;
	int mana, maxmana;
	int moves, maxmoves;
	int state;
public:
	enum Char_States { NORMAL, BASHED };
	CGroupChar();
	virtual ~CGroupChar();
	
	QByteArray getName() { return name; }
	void setName(QByteArray _name) { name = _name; }
	QDomNode toXML();
	bool updateFromXML(QDomNode blob);
	
	void setPosition(unsigned int id) { pos = id; }
	unsigned int getPosition() { return pos; }
	static QByteArray getNameFromXML(QDomNode node);
};

#endif /*CGROUPCHAR_H_*/
