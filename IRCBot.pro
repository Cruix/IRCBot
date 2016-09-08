QT += core network websockets
QT -= gui

CONFIG += c++11

TARGET = IRCBot
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ircclient.cpp \
    ircserver.cpp \
    ircbot.cpp \
    QDiscord/QDiscord/qdiscord/qdiscord.cpp \
    QDiscord/QDiscord/qdiscord/qdiscordchannel.cpp \
    QDiscord/QDiscord/qdiscord/qdiscordgame.cpp \
    QDiscord/QDiscord/qdiscord/qdiscordguild.cpp \
    QDiscord/QDiscord/qdiscord/qdiscordmember.cpp \
    QDiscord/QDiscord/qdiscord/qdiscordmessage.cpp \
    QDiscord/QDiscord/qdiscord/qdiscordrestcomponent.cpp \
    QDiscord/QDiscord/qdiscord/qdiscordstatecomponent.cpp \
    QDiscord/QDiscord/qdiscord/qdiscorduser.cpp \
    QDiscord/QDiscord/qdiscord/qdiscordutilities.cpp \
    QDiscord/QDiscord/qdiscord/qdiscordwscomponent.cpp

HEADERS += \
    ircclient.hpp \
    ircserver.hpp \
    ircbot.hpp \
    QDiscord/QDiscord/QDiscord \
    QDiscord/QDiscord/qdiscord/qdiscord.hpp \
    QDiscord/QDiscord/qdiscord/qdiscordchannel.hpp \
    QDiscord/QDiscord/qdiscord/qdiscordgame.hpp \
    QDiscord/QDiscord/qdiscord/qdiscordguild.hpp \
    QDiscord/QDiscord/qdiscord/qdiscordmember.hpp \
    QDiscord/QDiscord/qdiscord/qdiscordmessage.hpp \
    QDiscord/QDiscord/qdiscord/qdiscordrestcomponent.hpp \
    QDiscord/QDiscord/qdiscord/qdiscordstatecomponent.hpp \
    QDiscord/QDiscord/qdiscord/qdiscorduser.hpp \
    QDiscord/QDiscord/qdiscord/qdiscordutilities.hpp \
    QDiscord/QDiscord/qdiscord/qdiscordwscomponent.hpp
