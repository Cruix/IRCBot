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

#include "ircbot.hpp"

const QString IrcBot::_configPath = "config.json";
const QString IrcBot::_banlistPath = "banlist.rx";

IrcBot::IrcBot(QObject* parent) : QObject(parent), _server(&_banlist, &_password)
{
	loadConfig();
	loadBanlist();
	connect(&_server, &IrcServer::clientConnected, this, &IrcBot::ircClientConnected);
	connect(&_server, &IrcServer::clientDisconnected, this, &IrcBot::ircClientDisconnected);
	connect(&_server, &IrcServer::clientMessageReceived, this, &IrcBot::ircClientMessageReceived);
	_port = 6667;
	_publicChannel = nullptr;
	discordLogin();
	qDebug()<<this<<"constructed.";
}

void IrcBot::loadConfig()
{
	qDebug()<<"Loading configuration.";
	QFile configFile(this);
	configFile.setFileName(_configPath);
	if(!configFile.exists())
	{
		qDebug()<<"No configuration file exists. Creating"<<_configPath<<"with default values.";
		createDefaultConfig();
		exit(1);
	}
	if(!configFile.open(QFile::ReadOnly))
	{
		qDebug()<<"Could not open configuration file"<<_configPath<<"for reading.";
		qDebug()<<"Reason:"<<configFile.errorString();
		exit(1);
	}
	if(configFile.bytesAvailable() > 5242880)
	{
		qDebug()<<"Configuration file"<<_configPath<<"larger than 5 MB, overwriting with default values to avoid overload.";
		configFile.close();
		createDefaultConfig();
		exit(1);
	}
	QJsonDocument document = QJsonDocument::fromJson(configFile.readAll());
	configFile.close();
	setCurrentConfigToObject(document.object());
}

void IrcBot::loadBanlist()
{
	qDebug()<<"Loading ban list.";
	QFile banFile(this);
	banFile.setFileName(_banlistPath);
	if(!banFile.exists())
	{
		qDebug()<<"Banfile"<<_banlistPath<<"does not exist, creating empty list.";
		if(!banFile.open(QFile::WriteOnly|QFile::Truncate))
		{
			qDebug()<<"Could not open banfile"<<_banlistPath<<"for writing.";
			qDebug()<<"Reason:"<<banFile.errorString();
		}
		else banFile.close();
	}
	_banlist.clear();
	if(!banFile.open(QFile::ReadOnly))
	{
		qDebug()<<"Could not open banfile"<<_banlistPath<<"for reading.";
		qDebug()<<"Reason:"<<banFile.errorString();
	}
	else
	{
		while(!banFile.atEnd())
		{
			QString line = banFile.readLine().trimmed();
			if(!_banlist.contains(line))
				_banlist.append(line);
		}
		banFile.close();
	}
}

void IrcBot::discordLogin()
{
	connect(&_discord, &QDiscord::loginSuccess, this, &IrcBot::discordLoginFinished);
	connect(&_discord, &QDiscord::loginFailed, this, &IrcBot::discordLoginFailed);
	connect(_discord.state(), &QDiscordStateComponent::guildAvailable, this, &IrcBot::discordGuildAvailable);
	connect(_discord.state(), &QDiscordStateComponent::messageCreated, this, &IrcBot::discordMessageCreated);
	_discord.login(_loginToken);
}

void IrcBot::discordLoginFinished()
{
	qDebug()<<"Starting server.";
	_server.setHostname(_hostname);
	_server.setMotd(_motd);
	_server.setServerName(_serverName);
	_server.listen(QHostAddress::AnyIPv4, _port);
}

void IrcBot::discordLoginFailed()
{
	qDebug()<<"Discord login failed.";
	exit(1);
}

void IrcBot::discordGuildAvailable(QDiscordGuild* guild)
{
	if(guild)
		if(guild->id() == _guildId)
			for(int i = 0; i < guild->channels().keys().length(); i++)
				if(guild->channels().values()[i]->id() == _publicChannelId)
				{
					_publicChannel = guild->channels().values()[i];
					_server.setPublicChannel("#" + makeValidNick(_publicChannel->name()));
					qDebug()<<"Found public channel with name:"<<_publicChannel->name();
					break;
				}
}

void IrcBot::discordMessageCreated(QDiscordMessage message)
{
	if(message.channel()->id() == _publicChannelId && message.author()->id() != _discord.state()->self()->id())
		_server.broadcastMessage(":"+makeValidNick(message.author()->username())+"_"+message.author()->discriminator()+
							 " PRIVMSG #"+makeValidNick(_publicChannel->name())+" :"+message.content()+"\r\n");
}

void IrcBot::addBan(const QString& ban)
{
	if(_banlist.contains(ban.trimmed()))
	{
		qDebug()<<"Ban already in list, ignoring.";
		return;
	}
	qDebug()<<"Adding ban.";
	QFile banFile(this);
	banFile.setFileName(_banlistPath);
	if(!banFile.open(QFile::WriteOnly|QFile::Append))
	{
		qDebug()<<"Could not open banfile"<<_banlistPath<<"for writing. Adding ban without updating file.";
		qDebug()<<"Reason:"<<banFile.errorString();
	}
	else
	{
		banFile.write(ban.trimmed().append('\n').toUtf8());
		banFile.flush();
		banFile.close();
	}
	_banlist.append(ban.trimmed());
}

void IrcBot::removeBan(const QString& ban)
{
	if(!_banlist.contains(ban.trimmed()))
	{
		qDebug()<<"Ban already not in list, ignoring.";
		return;
	}
	qDebug()<<"Removing ban.";
	_banlist.removeAll(ban.trimmed());
	QFile banFile(this);
	banFile.setFileName(_banlistPath);
	if(!banFile.open(QFile::WriteOnly|QFile::Truncate))
	{
		qDebug()<<"Could not open banfile"<<_banlistPath<<"for writing. Removing ban without updating file.";
		qDebug()<<"Reason:"<<banFile.errorString();
	}
	else
	{
		for(int i = 0; i < _banlist.length(); i++)
			banFile.write(QString(_banlist[i] + '\n').toUtf8());
		banFile.flush();
		banFile.close();
	}
}

void IrcBot::createDefaultConfig()
{
	QJsonDocument document;
	document.setObject(currentConfigToObject());
	QFile configFile(this);
	configFile.setFileName(_configPath);
	if(!configFile.open(QFile::WriteOnly|QFile::Truncate))
	{
		qDebug()<<"Could not open configuration file"<<_configPath<<"for writing.";
		qDebug()<<"Reason:"<<configFile.errorString();
		exit(1);
	}
	configFile.write(document.toJson(QJsonDocument::Indented));
	configFile.flush();
	configFile.close();
}

QJsonObject IrcBot::currentConfigToObject()
{
	QJsonObject object;
	object["Port"] = _port;
	object["Login token"] = _loginToken;
	object["Public channel ID"] = _publicChannelId;
	object["Guild ID"] = _guildId;
	object["Password"] = _password;
	object["Announce connections"] = _announceConnections;
	object["Hostname"] = _hostname;
	object["Server name"] = _serverName;
	object["MOTD"] = _motd;
	return object;
}

void IrcBot::setCurrentConfigToObject(const QJsonObject& object)
{
	_port = object["Port"].toInt(6667);
	_loginToken = object["Login token"].toString("");
	_publicChannelId = object["Public channel ID"].toString("");
	_guildId = object["Guild ID"].toString("");
	_password = object["Password"].toString("");
	_announceConnections = object["Announce connections"].toBool(false);
	_hostname = object["Hostname"].toString("");
	_serverName = object["Server name"].toString("");
	_motd = object["MOTD"].toString("");
}

void IrcBot::ircClientConnected(IrcClient* client)
{
	if(!client)
		return;
	if(client->username() == "" || client->nickname() == "" || client->state() != IrcClient::ClientState::Connected)
		return;
	if(_announceConnections)
		_discord.rest()->sendMessage
		(
			QString("**" + client->nickname() + " (" + client->username() + ")** " + "has connected."),
			_publicChannel
		);
}

void IrcBot::ircClientDisconnected(IrcClient* client)
{
	_server.removeClient(client);
	if(!client)
	{
		client->deleteLater();
		return;
	}
	if(client->username() == "" || client->nickname() == "" || client->state() != IrcClient::ClientState::Connected)
	{
		client->deleteLater();
		return;
	}
	if(_announceConnections)
		_discord.rest()->sendMessage
		(
			QString("**" + client->nickname() + " (" + client->username() + ")** " + "has disconnected."),
			_publicChannel
		);
	client->deleteLater();
}

void IrcBot::ircClientMessageReceived(IrcClient* client, QString message)
{
	_discord.rest()->sendMessage(QString("**"+client->nickname()+" ("+client->username()+")**: "+message), _publicChannel);
}

QString IrcBot::makeValidNick(QString nick)
{
	if(nick.length() < 3)
		return "___";
	if(nick[0] == '#')
		nick[0] = '_';
	for(int i = 0; i < nick.length(); i++)
		nick[i] = makeValidChar(nick[i]);
	return nick;
}

QChar IrcBot::makeValidChar(QChar c)
{
	if ('a' <= c && c <= 'z')
		return c;
	if ('A' <= c && c <= 'Z')
		return c;
	if ('0' <= c && c <= '9')
		return c;
	if (c == '-' || c == '_' || c == '*' || c == '&' || c == '#')
		return c;
	return '_';
}
