#ifndef CGROUPCHAR_H_
#define CGROUPCHAR_H_

#include <QByteArray>
#include <QDomNode>
#include <QLabel>
#include <QGridLayout>
#include <QFrame>

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
	QColor color;
	
	QLabel 	*labelName;
	QLabel	*labelRoom;
	QLabel	*labelHpText;
	QLabel	*labelHpInt;
	QLabel	*labelManaText;
	QLabel	*labelManaInt;
	QLabel	*labelMovesText;
	QLabel	*labelMovesInt;
	QLabel	*labelState;
	QGridLayout *layout;
	
	QFrame *charFrame;
public:
	enum Char_States { NORMAL, BASHED, INCAPACITATED, DEAD };
	CGroupChar();
	virtual ~CGroupChar();
	
	QByteArray getName() { return name; }
	void setName(QByteArray _name) { name = _name; }
	void setColor(QColor col) { color = col; updateLabels(); }
	QColor getColor() { return color; }
	QDomNode toXML();
	bool updateFromXML(QDomNode blob);
	QFrame *getCharFrame() { return charFrame; }
	
	void setPosition(unsigned int id) { pos = id; }
	unsigned int getPosition() { return pos; }
	static QByteArray getNameFromXML(QDomNode node);

	void draw(int x, int y);
	void updateLabels();

};

#endif /*CGROUPCHAR_H_*/
