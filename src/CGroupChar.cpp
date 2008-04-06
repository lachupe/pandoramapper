#include "CGroupChar.h"
#include "utils.h"

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
	print_debug(DEBUG_GROUP, "New blob: %s", (const char *) data );
	return data;
}

bool CGroupChar::updateFromBlob(QByteArray blob)
{
	int index;
	index = blob.indexOf(' ');
	QByteArray tmp;

	print_debug(DEBUG_GROUP, "Incoming blob: %s", (const char *) blob );

	// first - simplify the array
	blob.simplified();

	// then take the ID
	index = blob.lastIndexOf(' ');
	tmp = blob.right( blob.size() - index - 1);
	pos = tmp.toInt();
	blob.chop(blob.size() - index);

	// and the rest is the charname !
	name = blob;
	return true; // hrmpf!
}

