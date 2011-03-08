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

#include "brain.h"
#include "ircuserscontainer.h"

#include <QObject>
#include <QTcpSocket>

class DbController;

class IrcController : public QObject
{
    Q_OBJECT
public:
    IrcController( DbController *db, IrcUsersContainer *ircUsers );
    ~IrcController();

    /**
     * adds irc client to transition hash
     * @param nick nick of the new ircClient
     * @param ircClient class for the ircClient
     */
    void addToTransition( const QByteArray &nick, IrcUsersContainer::WhoisStruct *ircClient );

    /**
     * Requests whois for all users in channel
     */
    void channelUsersWhois() const;

    /** returns socket in 'connection' */
    QTcpSocket *connectionSocket();

    /**
     * retrieves the users ip and login info from the channel and adds
     * them to the transition table
     * Usually this is called on bot startup
     * @param serverText server message containing all the channels users nicks
     */
    void extractUserWhois( const QByteArray &serverText );

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

    /**
     * used to send info tu user from outside irccontroller
     * @param nick nick to send message to
     * @param line line to send to nick
     */
    void sendLineToUser( const QByteArray &nick, const QByteArray &line );


    /**
     * updates the user info on nick change
     * @param oldNick nick before change
     * @param line irc line containing ip and new nick
     */
    void updateUserStruct( const QByteArray &oldNick, const QByteArray &line );

public slots:
    /**
     * slot used to send messages to user from other classes
     * @param nick nick to send message to
     * @param message message to send
     */
    void messageToUserSlot( const QByteArray &nick, const QByteArray &message );

    /**
     * slot used to send not authed messages to users
     * from other classes
     * @param nick nick to send message to
     */
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
    /** add user to oplist in database
     * @param nick user request addOp
     * @param msg msg given by user
     * @param ip user ip
     */
    void addOp( const QByteArray &nick, const QList< QByteArray > &msg, const QByteArray &ip );


    /**
     * auth user
     * @param nick nick to auth
     * @param msg msg given by user
     * @param ip user ip
     */
    void auth( const QByteArray &nick, const QList< QByteArray > &msg, const QByteArray &ip );


    /**
     * ban user
     * @param nick user requesting ban
     * @param msg msg given by user
     * @param ip user ip
     */
    void ban( const QByteArray &nick, const QList< QByteArray > &msg, const QByteArray &ip );


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
     * deop user from database and auth table
     * @param nick user requesting kick
     * @param msg message sent by user
     * @param ip ip of user requesting kick
     */
    void deop( const QByteArray &nick, const QList< QByteArray > &msg, const QByteArray &ip );


    /**
     * print help message for user ( or send help file? )
     * @param nick user to send info to
     */
    void help( const QByteArray &nick );


    /**
     * kick client
     * @param nick user requesting kick
     * @param msg message sent by user
     * @param ip ip of user requesting kick
     */
    void kick( const QByteArray &nick, const QList< QByteArray > &msg, const QByteArray &ip );


    /**
     * kickbans client
     * @param nick user requesting kick
     * @param msg message sent by user
     * @param ip ip of user requesting kick
     */
    void kickBan( const QByteArray &nick, const QList< QByteArray > &msg, const QByteArray &ip );

    /****************
    * bot functions *
    ****************/
    /**
     * generate a channel message
     * @param messageToSend message to send to channel
     */
    QByteArray genChannelMessage( const QByteArray &messageToSend );

    /**
     * generate a private message
     * @param nick nick to send message to
     * @param messageToSend message to send ti nick
     */
    QByteArray genPrivateMessage( const QByteArray &nick, const QByteArray &messageToSend );

    /**
     * send default not authed notification to user in chan
     * @param nick nick to send message to
     */
    void sendNotAuthedMessage( const QByteArray &nick );

    /**
     * sends private message to user in chan
     * @param nick nick to send message to
     * @param message message to send
     */
    void sendPrivateMessage( const QByteArray &nick, const QByteArray &message );

    /**
     * gather whois info about a user or the current chan. If an empty value is
     * passed a channel whois is launched asking for info on all users in the channel.
     * @param nick nick to do whois lookup on
     */
    void whois( const QByteArray &nick = QByteArray() );


    /***********
     * PRIVATE *
     **********/
    void loadSettings();                                /** load bot's irc settings from config file */

    QTcpSocket *m_connection;
    DbController *m_dbController;
    IrcUsersContainer *m_ircUsers;

    int m_port;
    QString m_chan, m_ip, m_nick;
};

#endif // IRCCONTROLLER_H
