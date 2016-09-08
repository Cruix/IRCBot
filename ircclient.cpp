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

#include "ircclient.hpp"

IrcClient::IrcClient(QTcpSocket* socket, QStringList* banlist, QString* password,
					 QList<IrcClient*>*clients, QString hostname, const QString& motd,
					 QString serverName, QString publicChannel, QObject* parent)
	: QObject(parent), _socket(socket), _banlist(banlist), _password(password),
	  _clients(clients), _hostname(hostname), _motd(motd), _serverName(serverName),
	  _publicChannel(publicChannel)
{
	_socket->setParent(this);
	connect(_socket, &QTcpSocket::disconnected, this, &IrcClient::socketDisconnected);
	connect(_socket, &QTcpSocket::readyRead, this, &IrcClient::socketReadyRead);
	connect(&_pingTimer, &QTimer::timeout, this, &IrcClient::ping);
	bool banned = false;
	QString address = _socket->peerAddress().toString();
	qDebug()<<"Incoming connection from"<<address<<":"<<_socket->peerPort();
	qDebug()<<"DNS resolves to:"<<QHostInfo::fromName(address).hostName();
	for(int i = 0; i < _banlist->length(); i++)
	{
		QRegExp banExp((*_banlist)[i]);
		banExp.setPatternSyntax(QRegExp::WildcardUnix);
		if(address.contains(banExp))
		{
			qDebug()<<"Ban string"<<(*_banlist)[i]<<"matched. Rejecting connection.";
			banned = true;
			_socket->close();
			deleteLater();
			break;
		}
	}
	if(!banned)
		_connected();
	qDebug()<<this<<"constructed";
}

IrcClient::~IrcClient()
{
	_socket->close();
	_socket->deleteLater();
	qDebug()<<this<<"destroyed";
}

void IrcClient::sendMessage(QString message)
{
	_socket->write(message.toUtf8());
	_socket->flush();
}

void IrcClient::_connected()
{
	if(*_password == "")
		_state = ClientState::AwaitingIdentification;
}

void IrcClient::socketDisconnected()
{
	emit disconnected();
	deleteLater();
}

void IrcClient::socketReadyRead()
{
	while(!_socket->atEnd())
	{
		QString line = _socket->readLine().trimmed();
		QStringList messageParameters = line.split(' ');
		if(_state == ClientState::AwaitingAuthentication)
		{
			if(line.startsWith("PASS ")&&line.mid(5)==*_password)
			{
				qDebug()<<"Successful pass from"<<_socket->peerAddress().toString();
				_state = ClientState::AwaitingIdentification;
			}
			else deleteLater();
		}
		else if(_state == ClientState::AwaitingIdentification)
		{
			if(messageParameters.length() == 2 && messageParameters[0] == "NICK")
			{
				QString nick = messageParameters[1];
				if(validNickname(nick))
				{
					bool passed = true;
					for(int i = 0; i < _clients->length(); i++)
						if((*_clients)[i] && (*_clients)[i]->nickname() == nick)
						{
							passed = false;
							break;
						}
					if(passed)
					{
						qDebug()<<_socket->peerAddress().toString()<<"is now known as"<<nick;
						_nickname = nick;
					}
					else
					{
						_socket->write(QString(":" + _hostname + " 433 "+nick+" :Nickname is already in use\r\n").toUtf8());
						_socket->flush();
					}
				}
				else
				{
					_socket->write(QString(":" + _hostname + " 432 "+nick+" :Erroneous nickname\r\n").toUtf8());
					_socket->flush();
				}
			}
			else if(messageParameters.length() > 4 && messageParameters[0] == "USER")
			{
				QString username = messageParameters[1];
				if(validNickname(username))
					_username = username;
				else
				{
					_socket->write(QString(":" + _hostname + " 432 "+username+" :Erroneous nickname\r\n").toUtf8());
					_socket->flush();
				}
			}
			if(_nickname != "" && _username != "")
			{
				_state = ClientState::Connected;
				_socket->write(QString(":" + _hostname + " 001 "+nickname()+" :Welcome to the " + _serverName + " IRC Network "
									   +nickname()+"!"+username()+"@"+_socket->peerAddress().toString()+"\r\n").toUtf8());
				_socket->write(QString(":" + _hostname + " 002 "+nickname()+" :Your host is " + _hostname + " running version IRCBot(https://github.com/yogstation13/IRCBot)\r\n").toUtf8());
				_socket->write(QString(":" + _hostname + " 005 "+nickname()+" NETWORK=" + _serverName + "\r\n").toUtf8());
				_socket->write(QString(":" + _hostname + " 375 "+nickname()+" :- " + _hostname + " Message of the Day -\r\n").toUtf8());
				QStringList motdList = _motd.split('\n');
				for(int i = 0; i < motdList.length(); i++)
					_socket->write(QString(":" + _hostname + " 372 "+nickname()+" :- " + motdList.at(i) + "\r\n").toUtf8());
				_socket->write(QString(":" + _hostname + " 376 "+nickname()+" :End of /MOTD command.\r\n").toUtf8());
				_socket->write(QString(":"+nickname()+"!"+username()+"@" + _hostname + " JOIN " + _publicChannel + "\r\n").toUtf8());
				_socket->flush();
				emit connected();
				ping();
				_pingTimer.start(30*1000);
			}
		}
		else if(_state == ClientState::Connected)
		{
			if(messageParameters.length() > 2 && messageParameters[0] == "PRIVMSG" && messageParameters[1] == _publicChannel)
			{
				int pos = 0;
				pos = line.indexOf(' ');
				pos = line.indexOf(' ', pos + 1);
				QString message = line.mid(pos + 2);
				qDebug()<<nickname()<<"!"<<username()<<":"<<message;
				emit messageReceived(message);
			}
		}
		if(_state != ClientState::AwaitingAuthentication)
		{
			if(line == "PONG :" + _hostname)
			{
				qDebug()<<"Pong received from"<<nickname()<<"!"<<username();
				_ponged = true;
			}
			else if(line == "QUIT")
			{
				_socket->write(QString("ERROR :Closing Link: "+nickname()+"["+_socket->peerAddress().toString()+"] (Quit: "+nickname()+")\r\n").toUtf8());
				_socket->flush();
				_socket->close();
			}
			else if(line.startsWith("PING ") && line.length() > 5)
			{
				qDebug()<<"Received ping from"<<nickname()<<"!"<<username();
				_socket->write(QString(":" + _hostname + " PONG "+line.mid(5)+"\r\n").toUtf8());
				_socket->flush();
			}
		}
	}
}

void IrcClient::ping()
{
	qDebug()<<"Pinging"<<nickname()<<"!"<<username();
	if(_pinged)
	{
		if(_ponged)
		{
			_ponged = false;
			_socket->write(QString("PING :" + _hostname + "\r\n").toUtf8());
			_socket->flush();
		}
		else
			_socket->close();
	}
	else
	{
		_pinged = true;
		_socket->write(QString("PING :" + _hostname + "\r\n").toUtf8());
		_socket->flush();
	}
}

bool IrcClient::validNickname(const QString& nick)
{
	qDebug()<<"Checking if"<<nick<<"is valid.";
	if(nick.length() < 3)
		return false;
	if(nick[0] == '#')
		return false;
	for(int i = 0; i < nick.length(); i++)
		if(!validChar(nick[i]))
			return false;
	return true;
}

bool IrcClient::validChar(QChar c)
{
	if ('a' <= c && c <= 'z')
		return true;
	if ('A' <= c && c <= 'Z')
		return true;
	if ('0' <= c && c <= '9')
		return true;
	if (c == '-' || c == '_' || c == '*' || c == '&' || c == '#')
		return true;
	return false;
}
