/*
 * IRCBot - An IRC relay for the Yogstation Discord.
 * Copyright (C) 2016  Yogstation13
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.	 If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IRCSERVER_HPP
#define IRCSERVER_HPP

#include <QObject>
#include <QTcpServer>
#include "ircclient.hpp"

class IrcServer : public QTcpServer
{
	Q_OBJECT
public:
	explicit IrcServer(QStringList* banlist, QString* password, QObject* parent = 0);
	~IrcServer();
	bool listen(const QHostAddress &address, quint16 port);
	void close();
	void broadcastMessage(QString message);
	void removeClient(IrcClient* client);
	void setPublicChannel(QString publicChannel) {_publicChannel = publicChannel;}
	void setHostname(QString hostname) {_hostname = hostname;}
	void setMotd(QString motd) {_motd = motd;}
	void setServerName(QString serverName) {_serverName = serverName;}
signals:
	void clientConnected(IrcClient* client);
	void clientDisconnected(IrcClient* client);
	void clientMessageReceived(IrcClient* client, QString message);
private:
	void handleClientConnect();
	void handleClientDisconnect();
	void handleClientMessageReceived(QString message);
	void newConnectionReceived();
	void connectClientSignals(IrcClient* client);
	QStringList* _banlist;
	QString* _password;
	QString _hostname;
	QString _publicChannel;
	QString _motd;
	QString _serverName;
	QList<IrcClient*> _clients;
};

#endif // IRCSERVER_HPP
