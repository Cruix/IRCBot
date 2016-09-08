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

#include "ircserver.hpp"

IrcServer::IrcServer(QStringList* banlist, QString* password, QObject* parent)
	: QTcpServer(parent), _banlist(banlist), _password(password)
{
	connect(this, &IrcServer::newConnection, this, &IrcServer::newConnectionReceived);
	qDebug()<<this<<"constructed";
}

IrcServer::~IrcServer()
{
	close();
}

bool IrcServer::listen(const QHostAddress& address, quint16 port)
{
	qDebug()<<"Starting server on"<<address<<":"<<port;
	bool result = QTcpServer::listen(address, port);
	if(!result)
		return result;

	return result;
}

void IrcServer::close()
{
	for(int i = 0; i < _clients.length(); i++)
		_clients[i]->deleteLater();
	_clients.clear();
	QTcpServer::close();
}

void IrcServer::broadcastMessage(QString message)
{
	for(int i = 0; i < _clients.length(); i++)
		_clients[i]->sendMessage(message);
}

void IrcServer::removeClient(IrcClient* client)
{
	_clients.removeAll(client);
}

void IrcServer::handleClientConnect()
{
	IrcClient* client = static_cast<IrcClient*>(sender());
	if(!client)
		return;
	emit clientConnected(client);
}

void IrcServer::handleClientDisconnect()
{
	IrcClient* client = static_cast<IrcClient*>(sender());
	if(!client)
		return;
	emit clientDisconnected(client);
}

void IrcServer::handleClientMessageReceived(QString message)
{
	IrcClient* client = static_cast<IrcClient*>(sender());
	if(!client)
		return;
	for(int i = 0; i < _clients.length(); i++)
		if(_clients[i] != client)
			_clients[i]->sendMessage(QString(":"+client->nickname()+"!"+client->username()+"@" + _hostname + " PRIVMSG " + _publicChannel + " :"+message+"\r\n"));
	emit clientMessageReceived(client, message);
}

void IrcServer::newConnectionReceived()
{
	IrcClient* client = new IrcClient(nextPendingConnection(), _banlist, _password, &_clients, _hostname, _motd, _serverName, _publicChannel, this);
	connectClientSignals(client);
	_clients.append(client);
}

void IrcServer::connectClientSignals(IrcClient* client)
{
	connect(client, &IrcClient::connected, this, &IrcServer::handleClientConnect);
	connect(client, &IrcClient::disconnected, this, &IrcServer::handleClientDisconnect);
	connect(client, &IrcClient::messageReceived, this, &IrcServer::handleClientMessageReceived);
}
