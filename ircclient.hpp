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

#ifndef IRCCLIENT_HPP
#define IRCCLIENT_HPP

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QHostInfo>
#include <QTimer>
#include <QDebug>

class IrcClient : public QObject
{
	Q_OBJECT
public:
	explicit IrcClient(QTcpSocket* socket, QStringList* banlist, QString* password,
					   QList<IrcClient*>* clients, QString hostname, const QString& motd,
					   QString serverName, QString publicChannel, QObject* parent = 0);
	~IrcClient();
	enum class ClientState
	{
		AwaitingAuthentication,
		AwaitingIdentification,
		Connected
	};
	QString username(){return _username;}
	QString nickname(){return _nickname;}
	ClientState state(){return _state;}
	void sendMessage(QString message);
signals:
	void connected();
	void disconnected();
	void messageReceived(QString messsage);
private:
	ClientState _state = ClientState::AwaitingAuthentication;
	void _connected();
	void socketDisconnected();
	void socketReadyRead();
	void ping();
	bool validNickname(const QString& nick);
	bool validChar(QChar c);
	QString _username;
	QString _nickname;
	QTimer _pingTimer;
	bool _pinged = false;
	bool _ponged = false;
	QTcpSocket* _socket;
	QStringList* _banlist;
	QString* _password;
	QList<IrcClient*>* _clients;
	QString _hostname;
	const QString& _motd;
	QString _serverName;
	QString _publicChannel;
};

#endif // IRCCLIENT_HPP
