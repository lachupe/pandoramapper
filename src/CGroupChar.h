#ifndef CGROUPCHAR_H_
#define CGROUPCHAR_H_

#include <QByteArray>

class CGroupChar
{
	QByteArray name;
	QByteArray textHP;
	QByteArray textMoves;
	QByteArray textMana;
public:
	CGroupChar();
	virtual ~CGroupChar();
};

#endif /*CGROUPCHAR_H_*/
