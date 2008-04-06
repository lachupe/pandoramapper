#ifndef CGROUPCHAR_H_
#define CGROUPCHAR_H_

#include <QByteArray>

class CGroupChar
{
	unsigned int pos;
	QByteArray name;
	QByteArray textHP;
	QByteArray textMoves;
	QByteArray textMana;
public:
	CGroupChar();
	virtual ~CGroupChar();
	
	QByteArray getName() { return name; }
	void setName(QByteArray _name) { name = _name; }
	QByteArray toBlob();
	bool updateFromBlob(QByteArray blob);
	
	void setPosition(unsigned int id) { pos = id; }
	unsigned int getPosition() { return pos; }
};

#endif /*CGROUPCHAR_H_*/
