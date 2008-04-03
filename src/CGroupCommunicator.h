#ifndef CGROUPCOMMUNICATOR_H_
#define CGROUPCOMMUNICATOR_H_

#include <QObject>

class CGroup;
class CGroupClient;

// draft for no peer
class CGroupDraftConnection : public QObject 
{
	Q_OBJECT
public:
	CGroupDraftConnection() {}
	virtual ~CGroupDraftConnection() {}
};

class CGroupCommunicator : public QObject
{
	Q_OBJECT
	int type;
	
	enum Messages { ACK, REQ_LOGIN, DATA_LOGIN, REQ_INFO, CHAR_DATA };
	
	QObject *peer;	// server or client
	CGroup *getGroup() { return (CGroup *) parent(); }
	void connectionClosed(CGroupClient *connection);
	void connectionEstablished(CGroupClient *connection);
	void connecting(CGroupClient *connection);
	QByteArray formMessageBlock(int message, QByteArray data);
	void sendMessage(CGroupClient *connection, int message, QByteArray data);
	
	void retrieveDataClient(CGroupClient *connection);
	void retrieveDataServer(CGroupClient *connection);
public:
	const static int version = 100; // Protocol version. Those must match!
	enum States { Server, Client, Off };


	CGroupCommunicator(int type, QObject *parent);
	virtual ~CGroupCommunicator();
	
	void changeType(int newState);

public slots:
	void connectionStateChanged(CGroupClient *connection);
	void errorInConnection(CGroupClient *connection);
	void serverStartupFailed();
	void incomingData(CGroupClient *connection);

};

#endif /*CGROUPCOMMUNICATOR_H_*/
