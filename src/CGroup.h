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

#ifndef CGROUP_H_
#define CGROUP_H_

#include <QDialog>
#include <QString>
#include <QDomNode>
#include <QVector>
#include <QGridLayout>
#include <QCloseEvent>
#include <QFrame>
#include <QHash>

#include "CGroupCommunicator.h"
#include "CGroupChar.h"

class CGroup : public QDialog
{
	Q_OBJECT

	void resetAllChars();
public:

	enum StateMessages { NORMAL, FIGHTING, RESTING, SLEEPING, CASTING, INCAP, DEAD, BLIND, UNBLIND };


	CGroup(QByteArray name, QWidget *parent);
	virtual ~CGroup();

	QByteArray getName() { return self->getName(); }
	CGroupChar* getCharByName(QByteArray name);

	void setType(int newState);
	int getType() {return network->getType(); }
	bool isConnected() { return network->isConnected(); }
	void reconnect() { resetChars();  network->reconnect(); }

	void addSelf();
	void hideSelf();
	bool addChar(QDomNode blob);
	void removeChar(QByteArray name);
	void removeChar(QDomNode node);
	bool isNamePresent(QByteArray name);
	QByteArray getNameFromBlob(QDomNode blob);
//	bool addCharIfUnique(QDomNode blob);
	void updateChar(QDomNode blob); // updates given char from the blob
	void updateCharPosition(QDomNode blob);
	void updateCharScore(QDomNode blob);
	void updateCharPrompt(QDomNode blob);

	CGroupCommunicator *getCommunicator() { return network; }


	void resetChars();
	QVector<CGroupChar *>  getChars() { return chars; }
	// changing settings
	void resetName();
	void resetColor();

	QDomNode getLocalCharData() { return self->toXML(); }
	void sendAllCharsData(CGroupClient *conn);
	void issueLocalCharUpdate() { 	network->sendCharUpdate(self->toXML()); }
	void issueLocalCharScoreUpdate() { 	network->sendCharScoreUpdate(self->scoreToXML()); }
	void issueLocalCharPromptUpdate() { 	network->sendCharPromptUpdate(self->promptToXML()); }
	void issueLocalCharPositionUpdate() { 	network->sendCharPositionUpdate(self->positionToXML()); }

	void gTellArrived(QDomNode node);

	// dispatcher/Engine hooks
	bool isGroupTell(QByteArray tell);

	void renameChar(QDomNode blob);


public slots:
	void connectionRefused(QString message);
	void connectionFailed(QString message);
	void connectionClosed(QString message);
	void connectionError(QString message);
	void serverStartupFailed(QString message);
	void gotKicked(QDomNode message);
	void setCharPosition(unsigned int pos);

	void closeEvent( QCloseEvent * event ) { hide(); event->accept(); emit hides();}
	void sendGTell(QByteArray tell); // sends gtell from local user
	void parseScoreInformation(QByteArray score);
	void parsePromptInformation(QByteArray prompt);

	void parseStateChangeLine(int message, QByteArray line);

	void updateSpellsInfo();

	void updateGroupManagerWindow();

signals:
	void hides();

private:
	CGroupCommunicator*		network;
	QVector<CGroupChar *> 	chars;
	CGroupChar*				self;
	QTreeWidget*			tree;
};

#endif /*CGROUP_H_*/
