#include "CGroupChar.h"

CGroupChar::CGroupChar()
{
}

CGroupChar::~CGroupChar()
{
}

QByteArray CGroupChar::toBlob()
{
	QByteArray data;
	
	data = name;	// name
	data += " ";
	data += "14";	// ID
}

bool CGroupChar::updateFromBlob(QByteArray blob)
{
	int index;
	index = blob.indexOf(' ');
	QByteArray tmp;

	// first - simplify the array
	blob.simplified();

	// then take the ID
	index = blob.lastIndexOf(' ');
	tmp = blob.right( blob.size() - index - 1);
	pos = tmp.toInt();
	blob.chop(blob.size() - index);

	// and the rest is the charname !
	name = blob;
}

