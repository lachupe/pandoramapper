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

#ifndef FORWARDER_H
#define FORWARDER_H

#include <QMutex>
#include <QThread>

#if defined Q_OS_LINUX || defined Q_OS_MACX || defined Q_OS_FREEBSD
#define SOCKET int
#elif defined Q_OS_WIN32
#include <winsock.h>
#endif

#define SOCKET int

#define PROXY_BUFFER_SIZE 8192

#define IAC 255
#define DONT 254
#define DO 253
#define WONT 252
#define WILL 251
#define SB 250
#define TN_GA 249
#define TN_EL 248
#define TN_EC 247
#define TN_AYT 246
#define TN_AO 245
#define TN_IP 244
#define BREAK 243
#define TN_DM 242
#define TN_NOP 241
#define SE 240
#define TN_EOR 239
#define TN_ABORT 238
#define TN_SUSP 237
#define TN_EOF 236
#define LAST_TN_CMD 236

class Cdispatcher;
class Proxy;

class ProxySocket {
	/* connection sockets */
	SOCKET sock;
	QMutex mutex;
	Proxy  *parent;

public:
	int mainState;
	int subState;
	char buffer[PROXY_BUFFER_SIZE];
	int length;

	ProxySocket(Proxy * parent, bool xml = false);

	/* xml flags */
	bool xmlMode;
	int n;

	bool xmlTogglable;

	QByteArray subchars;
	QByteArray fragment;

	// returns new position
	void append(QByteArray line) {
		memcpy(buffer + length, line, line.length());
		length += line.length();
	}

	void append(const char *line) {
		memcpy(buffer + length, line, strlen(line));
		length += strlen(line);
	}

	void clearBuffer() {
		length = 0;
	}

	void close();
	void clear();
	void send_line(const char *line);SOCKET getSocket();
	bool isXmlMode();
	void setXmlMode(bool b);
	bool isXmlTogglable();
	void setXmlTogglable(bool b);

	bool isConnected();
	void setConnection(SOCKET sock);
	bool openConnection(QByteArray name, int port);

	void nonblock();

	int read(); // read stuff in internal buffer
	int read(char *buf, int len);
	void write(char *buf, int len);

};

/* PROXY THREAD DEFINES */
class Proxy: public QThread {
Q_OBJECT

	ProxySocket *mud;
	ProxySocket *user;
	SOCKET proxy_hangsock;

	Cdispatcher *dispatcher;

	int loop();
	bool mudEmulation;

	bool connectToMud();
	void incomingConnection();
	void sendMudEmulationGreeting();

public:

	Proxy();
	~Proxy();

	void run();
	int  init();
	void send_line_to_user(const char *line);
	void send_line_to_mud(const char *line);
	bool isMudEmulation() {
		return mudEmulation;
	}
	void setMudEmulation(bool b);
	void shutdown();

	void startEngineCall() {
		emit startEngine();
	}
	void startRendererCall() {
		emit startRenderer();
	}

	void sendGroupTellEvent(QByteArray data);
	void sendScoreLineEvent(QByteArray data);
	void sendPromptLineEvent(QByteArray data);
	void sendSpellsUpdatedEvent() {
		emit sendSpellsUpdate();
	}
	void sendCharStateUpdatedEvent(int state) {
		emit sendCharStateUpdate(state);
	}

	void sendLog(const QString& message) {
		emit log("Network", message);
	}

signals:
	void connectionEstablished();
	void connectionLost();
	void startEngine();
	void startRenderer();
	void sendGTell(QByteArray);
	void sendScoreLine(QByteArray);
	void sendPromptLine(QByteArray);
	void sendSpellsUpdate();
	void sendCharStateUpdate(int);
	void log(const QString& module, const QString& message);
};

/* PROX THREAD ENDS */

extern class Proxy *proxy;

#endif
