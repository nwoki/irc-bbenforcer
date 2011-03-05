/*****************************************************************************************
 * ioQIC-BBEnforcer - Qt irc bot that also sends rcon commands to UrbanTerror game server*
 * Copyright (C) 2010 - 2011, (n)woki                                                    *
 *                                                                                       *
 * gamecontroller.h is part of ioQIC-BBEnforcer                                          *
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

#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QUdpSocket>

class DbController;

class GameController : public QObject
{
    Q_OBJECT

public:
    GameController( DbController *db );

    QUdpSocket *connectionSocket() const;   /** get socket used by class for connection to server */

    /**
     * parses game commands sent via irc by user
     * @param user user of the command to parse
     * @param msg message of the user to parse
     * @param ip ip of user
     */
    void gameCommandParser( const QByteArray &user, const QByteArray &msg, const QByteArray &ip );

private slots:
    void connectNotify();

signals:
    void notAuthedSignal( const QByteArray& user );

private:
    /*****************
    * game functions *
    *****************/
    /**
     * request game server status
     * @param user user requesting command
     * @param ip ip of user requesting command
     */
    void status( const QByteArray &user, const QByteArray &ip );


    /***********
     * PRIVATE *
     **********/
    void loadSettings();                    /** load game controller settings */

    DbController *m_db;
    QUdpSocket *m_socket;
    QString m_ip
    , m_rconPass;
    int m_port;
};

#endif // GAMECONTROLLER_H
