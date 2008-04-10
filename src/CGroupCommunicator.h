#ifndef CGROUPCOMMUNICATOR_H_
#define CGROUPCOMMUNICATOR_H_

#include <QObject>
#include <QDomNode>

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
	
	enum Messages { NONE, ACK, 
					REQ_VERSION, REQ_ACK, REQ_LOGIN, REQ_INFO, 
					PROT_VERSION, GTELL, 
					STATE_LOGGED, STATE_KICKED,
					ADD_CHAR, REMOVE_CHAR, UPDATE_CHAR };
	
	QObject *peer;	// server or client
	CGroup *getGroup() { return (CGroup *) parent(); }
	
	void connectionClosed(CGroupClient *connection);
	void connectionEstablished(CGroupClient *connection);
	void connecting(CGroupClient *connection);
	QByteArray formMessageBlock(int message, QDomNode data);
	void sendMessage(CGroupClient *connection, int message, QByteArray data = "");
	void sendMessage(CGroupClient *connection, int message, QDomNode data);

	void sendLoginInformation(CGroupClient *connection);
	void parseLoginInformation(CGroupClient *connection, QDomNode data);
	void sendGroupInformation(CGroupClient *connection);
	void parseGroupInformation(CGroupClient *connection, QDomNode data);
	
	void userLoggedOn(CGroupClient *conn);
	void userLoggedOff(CGroupClient *conn);
		
	void retrieveDataClient(CGroupClient *connection, int message, QDomNode data);
	void retrieveDataServer(CGroupClient *connection, int message, QDomNode data);
	
public:
	const static int protocolVersion = 102;
	enum States { Server, Client, Off };


	CGroupCommunicator(int type, QObject *parent);
	virtual ~CGroupCommunicator();
	
	void changeType(int newState);
	void sendCharUpdate(CGroupClient *conn, QDomNode blob);


public slots:
	void connectionStateChanged(CGroupClient *connection);
	void errorInConnection(CGroupClient *connection);
	void serverStartupFailed();
	void incomingData(CGroupClient *connection);

};

#endif /*CGROUPCOMMUNICATOR_H_*/
