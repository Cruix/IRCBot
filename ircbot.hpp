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

#ifndef IRCBOT_HPP
#define IRCBOT_HPP

#include <QObject>
#include <QDiscord/QDiscord/QDiscord>
#include <QDir>
#include <QFile>
#include "ircserver.hpp"

class IrcBot : public QObject
{
	Q_OBJECT
public:
	explicit IrcBot(QObject* parent = 0);
	void addBan(const QString& ban);
	void removeBan(const QString& ban);
private:
	void loadConfig();
	void loadBanlist();
	void discordLogin();
	void discordLoginFinished();
	void discordLoginFailed();
	void discordGuildAvailable(QDiscordGuild* guild);
	void discordMessageCreated(QDiscordMessage message);
	void createDefaultConfig();
	QJsonObject currentConfigToObject();
	void setCurrentConfigToObject(const QJsonObject& object);
	void ircClientConnected(IrcClient* client);
	void ircClientDisconnected(IrcClient* client);
	void ircClientMessageReceived(IrcClient* client, QString message);
	static QString makeValidNick(QString nick);
	static QChar makeValidChar(QChar c);
	QDiscord _discord;
	QStringList _banlist;
	IrcServer _server;
	static const QString _configPath;
	static const QString _banlistPath;
	QDiscordChannel* _publicChannel;
	QString _loginToken;
	QString _publicChannelId;
	QString _guildId;
	QString _password;
	QString _hostname;
	QString _motd;
	QString _serverName;
	int _port;
	bool _announceConnections;
};

#endif // IRCBOT_HPP
