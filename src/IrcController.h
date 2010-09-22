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
class DbController;
class QAbstractSocket;

class IrcController : public QObject
{
    Q_OBJECT
    public:
        IrcController( DbController *db );
        ~IrcController();

        QAbstractSocket *connectionSocket();  /* returns socket in 'connection' */
        void ircCommandParser( const QByteArray &user, const QByteArray &msg, const QByteArray &ip );   /* parse irc commands given to the bot */
        void logIn();   /* log into server ( bot ) */
        void pong( const QByteArray &pingData );    /* respond to ping request from irc server */

    private:
        /****************
        * irc functions *
        ****************/
        void auth( const QByteArray &user, const QByteArray &msg, const QByteArray &ip );  /* auth user */
        void flood( const QByteArray &nick, const QByteArray &message );    /* flood client with message */
        void help();    /* print help message ( or send help file? )*/
        void kick( const QByteArray &nick, const QByteArray &reason );  /* kick client */

        /****************
        * bot functions *
        ****************/
        bool isAuthed( const QByteArray &user, const QByteArray &msg, const QByteArray &ip );
        QByteArray genChannelMessage( const QByteArray &messageToSend );
        QByteArray genPrivateMessage( const QByteArray &nick, const QByteArray &messageToSend );



        Connection *m_connection;
        DbController *m_dbController;
        //keep copy of connection's settings???
};

#endif // IRCCONTROLLER_H
