/***************************************************************************************
* ioQIC - Qt irc bot that also sends rcon commands to UrbanTerror game server          *
* Copyright (C) 2010, woki                                                             *
*                                                                                      *
* IrcController.h is part of ioQIC                                                     *
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

#ifndef IRCCONTROLLER_H
#define IRCCONTROLLER_H

#include <QObject>

class Connection;
class QAbstractSocket;

class IrcController : public QObject
{
    Q_OBJECT
    public:
        IrcController();
        ~IrcController();

        QAbstractSocket *connectionSocket();  //returns socket in 'connection'
        void logIn();
        void pong( QByteArray pingData );

    private:
        Connection *m_connection;
};

#endif // IRCCONTROLLER_H
