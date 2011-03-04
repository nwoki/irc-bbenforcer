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

#include <QObject>

class DbController;
class GameController;
class IrcController;

class Brain : public QObject
{
    Q_OBJECT
public:
    Brain();
    ~Brain();

    QByteArray extractIp( const QByteArray &text );         /** extracts ip of client */
    QByteArray extractText( const QByteArray &text );       /** extracts text sent by client */
    QByteArray extractNick( const QByteArray &text );       /** extracts user's nick from message */
    QByteArray extractUserLogin( const QByteArray &text );  /** extracts user's login name on irc server ( nick and login name can be different! )*/

public slots:
    void parseIrcData();                                /** parses data sent from server to bot */

private:
    DbController *m_dbControl;
    GameController *m_gameControl;
    IrcController *m_ircControl;
};

#endif // BRAIN_H
