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
#include <QVector>

class DbController;
class IrcUsersContainer;

class GameController : public QObject
{
    Q_OBJECT

public:
    GameController( DbController *db, IrcUsersContainer *ircUsers );

    QUdpSocket *connectionSocket() const;   /** get socket used by class for connection to server */

    /**
     * parses game commands sent via irc by user
     * @param nick nick of the command to parse
     * @param msg message of the user to parse
     * @param ip ip of user
     */
    void gameCommandParser( const QByteArray &nick, const QByteArray &msg, const QByteArray &ip );

    /**
     * returns the next user in line that has requested a command with info in response
     * from the gameserver
     */
    QByteArray nextUserInLine();

private slots:
    void connectNotify();

signals:
    void messageToUserSignal( const QByteArray &nick, const QByteArray &message );
    void notAuthedSignal( const QByteArray &nick );
    void singleUserWhoisSignal( const QByteArray &nick );

private:
    /*****************
    * game functions *
    *****************/
    /**
     * send bigtext message to server
     * @param nick nick requesting command
     * @param ip ip of nick requesting command
     * @param msgList message sent by nick to send to server
     */
    void bigText( const QByteArray &nick, const QByteArray &ip, const QList<QByteArray> &msgList );


    /**
     * execute the given config file on server
     * @param nick nick requesting command
     * @param ip ip of nick requesting command
     * @param msgList message sent by nick to send to server
     */
    void execConfig( const QByteArray &nick, const QByteArray &ip, const QList<QByteArray> &msgList );


    /**
     * set gravity of the server
     * @param nick nick requesting command
     * @param ip ip of nick requesting command
     * @param msgList message sent by nick containing the gravity value
     */
    void gravity( const QByteArray &nick, const QByteArray &ip, const QList<QByteArray> &msgList );


    /**
     * change current map
     * @param nick nick requesting command
     * @param ip ip of nick requesting command
     * @param msgList message sent by nick containing the map name
     */
    void map( const QByteArray &nick, const QByteArray &ip, const QList<QByteArray> &msgList );


    /**
     * set the next map for the server
     * @param nick nick requesting command
     * @param ip ip of nick requesting command
     * @param msgList message sent by nick containing the map name
     */
    void nextMap( const QByteArray &nick, const QByteArray &ip, const QList<QByteArray> &msgList );


    /**
     * reload game
     * @param nick nick requesting command
     * @param ip ip of nick requesting command
     * @param msgList message sent by nick containing the map name ( SHOULD BE EMPTY )
     */
    void reload( const QByteArray &nick, const QByteArray &ip );


    /**
     * restart game
     * @param nick nick requesting command
     * @param ip ip of nick requesting command
     */
    void restart( const QByteArray &nick, const QByteArray &ip );


    /**
     * request game server status
     * @param nick nick requesting command
     * @param ip ip of nick requesting command
     */
    void status( const QByteArray &nick, const QByteArray &ip );


    /***********
     * PRIVATE *
     **********/
    QByteArray cmdBegin();                  /** returns a QByteArray with the beginning of the Rcon message to send to server */
    void loadSettings();                    /** load game controller settings */

    DbController *m_db;
    IrcUsersContainer *m_ircUsers;
    QUdpSocket *m_socket;
    QString m_ip
    , m_rconPass;
    int m_port;

    /**
     * keeps track of the users that sent a command to the server.
     * Whenever a new command is sent, the user that requested the
     * command is sent in line.
     * This way from brain I know who to send the response message to
     * in case the game command needs to send back any info
     */
    QVector< QByteArray > m_userList;
};

#endif // GAMECONTROLLER_H

