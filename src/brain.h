/****************************************************************************************
* ioQIC-BBEnforcer - Qt irc bot that also sends rcon commands to UrbanTerror game server*
* Copyright (C) 2010 - 2011, (n)woki                                                    *
*                                                                                       *
* brain.h is part of ioQIC-BBEnforcer                                                   *
*                                                                                       *
* ioQIC-BBEnforcer is free software: you can redistribute it and/or modify it under the *
* terms of the GNU General Public License as published by the Free Software Foundation, *
* either version 3 of the License, or (at your option) any later version.               *
*                                                                                       *
* ioQIC-BBEnforcer is distributed in the hope that it will be useful, but WITHOUT ANY   *
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A       *
* PARTICULAR PURPOSE.  See the GNU General Public License for more details.             *
*                                                                                       *
* You should have received a copy of the GNU General Public License along with this     *
* program.  If not, see <http://www.gnu.org/licenses/>.                                 *
****************************************************************************************/

#ifndef BRAIN_H
#define BRAIN_H

#include "ircuserscontainer.h"

#include <QObject>

class DbController;
class GameController;
class IrcController;
class Logger;

class Brain : public QObject
{
    Q_OBJECT
public:

    Brain();
    ~Brain();

    QByteArray extractIp(const QByteArray &text);         /** extracts ip of client */
    QByteArray extractText(const QByteArray &text);       /** extracts text sent by client */
    QByteArray extractNick(const QByteArray &text);       /** extracts user's nick from message */
    QByteArray extractUserLogin(const QByteArray &text);  /** extracts user's login name on irc server ( nick and login name can be different! )*/

public slots:
    /** checks users in container, for banned. This function is called after the bot has joined the channel */
    void checkUserOnJoin(IrcUsersContainer::WhoisStruct *ircUser);


    /** parses data recieved from gameserver and then
     * sends it to the user that requested the info in
     * case there is a response from the server */
    void parseGameData();


    /** parses data recieved from irc server */
    void parseIrcData();

private:
    DbController *m_dbControl;
    IrcUsersContainer *m_ircUsers;
    GameController *m_gameControl;
    IrcController *m_ircControl;
    Logger *m_logger;

    QByteArray m_ircData;
};

#endif // BRAIN_H

