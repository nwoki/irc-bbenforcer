/*****************************************************************************************
 * ioQIC-BBEnforcer - Qt irc bot that also sends rcon commands to UrbanTerror game server*
 * Copyright (C) 2010 - 2011, (n)woki                                                    *
 *                                                                                       *
 * irccontroller.h is part of ioQIC-BBEnforcer                                           *
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

#ifndef IRCCONTROLLER_H
#define IRCCONTROLLER_H

#include <QObject>
#include <QTcpSocket>

class DbController;

class IrcController : public QObject
{
    Q_OBJECT
public:
    IrcController( DbController *db );
    ~IrcController();

    /** returns socket in 'connection' */
    QTcpSocket *connectionSocket();

    /**
     * parse irc commands given to the bot
     * @param user user of the command to parse
     * @param msg message of the user to parse
     * @param ip ip of user
     */
    void ircCommandParser( const QByteArray &user, const QByteArray &msg, const QByteArray &ip );

    /**
     * return irc settings as a map
     */
    QMap< QString, QString > ircSettings();

    /**
     * log bot to server
     */
    void logIn();

    /**
     * ban user on login if found in banned table. This is called from "BRAIN" when it checks the user
     * JOIN on irc channel and then kick
     * @param nick users nick to ban
     * @param userLogin login to banned
     * @param ip ip to ban
     */
    void loginBan( const QByteArray &nick, const QByteArray &userLogin, const QByteArray &ip );

    /**
     * respond to ping request from irc server
     * @param pingData ping data sent by server to resend
     */
    void pong( const QByteArray &pingData );

public slots:
    void userNotAuthedSlot( const QByteArray& nick );

private slots:
    void connectNotify();
    void disconnectNotify();
    void handleSocketErrors( QAbstractSocket::SocketError );
    void reconnect();

private:
    /****************
    * irc functions *
    ****************/
    /**
     * auth user
     * @param user user to auth
     * @param msg msg given by user
     * @param ip user ip
     */
    void auth( const QByteArray &user, const QList< QByteArray > &msg, const QByteArray &ip );

    /**
     * ban user
     * @param user user requesting ban
     * @param msg msg given by user
     * @param ip user ip
     */
    void ban( const QByteArray &user, const QList< QByteArray > &msg, const QByteArray &ip );

    /**
     * ban function used by bot( when auto-banning )
     * @param userLogin login name to banned
     * @param ip ip to ban
     */
    void botBan( const QByteArray &userLogin, const QByteArray &ip );

    /**
     * kick function used by the bot( when kick-banning )
     * @param nick nick to kick from chan
     * @param reason reason for kick
     */
    void botKick( const QByteArray &nick, const QString &reason );

    /**
     * print help message for user ( or send help file? )
     * @param user user to send info to
     */
    void help( const QByteArray &user );

    /**
     * kick client
     * @param user user requesting kick
     * @param msg message sent by user
     * @param ip ip of user requesting kick
     */
    void kick( const QByteArray &user, const QList< QByteArray > &msg, const QByteArray &ip );

    /****************
    * bot functions *
    ****************/
    bool isAuthed( const QByteArray &user, const QByteArray &ip );
    QByteArray genChannelMessage( const QByteArray &messageToSend );
    QByteArray genPrivateMessage( const QByteArray &nick, const QByteArray &messageToSend );
    void sendNotAuthedMessage( const QByteArray &nick );
    void sendPrivateMessage( const QByteArray &nick, const QByteArray &message );               // send PVT message to nick


    /***********
     * PRIVATE *
     **********/
    void loadSettings();

    QTcpSocket *m_connection;
    DbController *m_dbController;

    int m_port;
    QString m_chan, m_ip, m_nick;
};

#endif // IRCCONTROLLER_H
