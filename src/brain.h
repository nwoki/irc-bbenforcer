/***************************************************************************************
* ioQIC - Qt irc bot that also sends rcon commands to UrbanTerror game server          *
* Copyright (C) 2010, woki                                                             *
*                                                                                      *
* Brain.h is part of ioQIC                                                             *
*                                                                                      *
* ioQIC is free software: you can redistribute it and/or modify it under the           *
* terms of the GNU General Public License as published by the Free Software Foundation,*
* either version 3 of the License, or (at your option) any later version.              *
*                                                                                      *
* ioQIC is distributed in the hope that it will be useful, but WITHOUT ANY             *
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
* PARTICULAR PURPOSE.  See the GNU General Public License for more details.            *
*                                                                                      *
* You should have received a copy of the GNU General Public License along with this    *
* program.  If not, see <http://www.gnu.org/licenses/>.                                *
****************************************************************************************/

#ifndef BRAIN_H
#define BRAIN_H

#include <QObject>

class DbController;
class GameController;
class IrcController;
class QByteArray;

class Brain : public QObject
{
    Q_OBJECT
public:
    Brain();
    ~Brain();

    QByteArray extractIp( const QByteArray &text );     /*!< get's ip of client */
    QByteArray extractText( const QByteArray &text );   /*!< extracts text sent by client */
    QByteArray extractUser( const QByteArray &text );   /*!< extracts user from message */

public slots:
    void parseData();   /*!< parses data sent from server to bot */

private:
    //DbController *m_dbControl;    don't need this. Game and irc got pointer to it
    GameController *m_gameControl;
    IrcController *m_ircControl;
};

#endif // BRAIN_H