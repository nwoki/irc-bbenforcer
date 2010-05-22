/***************************************************************************************
* ioQIC - Qt irc bot that also sends rcon commands to UrbanTerror game server          *
* Copyright (C) 2010, woki                                                             *
*                                                                                      *
* Connection.h is part of ioQIC                                                        *
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

#ifndef CONNECTION_H
#define CONNECTION_H

#include <QAbstractSocket>
#include <QObject>
#include <QString>

class QSettings;

class Connection : public QObject
{
    Q_OBJECT
    public:
    Connection( QAbstractSocket::SocketType type/*, const QString &ip, int port */);
        ~Connection();

        void loadSettings();
        void startConnect();
        QAbstractSocket *socket();  //returns the socket in use

    public slots:
        void connectNotify();
        void disconnectNotify();
        void handleSocketErrors( QAbstractSocket::SocketError );
        void reconnect();

    private:
        int m_port;
        QAbstractSocket *m_socket;
        QSettings *m_settings;
        QString m_ip;//, m_socketType;

};

#endif // CONNECTION_H
