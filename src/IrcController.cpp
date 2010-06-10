/***************************************************************************************
* ioQIC - Qt irc bot that also sends rcon commands to UrbanTerror game server          *
* Copyright (C) 2010, woki                                                             *
*                                                                                      *
* IrcController.cpp is part of ioQIC                                                   *
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

#include "IrcController.h"
#include "Connection.h"
#include "DbController.h"

#include <QAbstractSocket>

#define end "\r\n"

IrcController::IrcController( DbController *db )
    : m_connection( new Connection( QAbstractSocket::TcpSocket ) )  //get these on constructor
    , m_dbController( db )
{
    qDebug( "IrcController::IrcController" );
    m_connection->startConnect();
}

IrcController::~IrcController()
{
    delete m_connection;
    delete m_dbController;
}

QAbstractSocket *IrcController::connectionSocket()
{
    return m_connection->socket();
}

void IrcController::logIn()
{
    //get settings
    QMap< QString, QString > data = m_connection->ircSettings();
    QString nick = data.value( "nick" );

    //nick
    QByteArray byteNick( "NICK " );
    byteNick.append( nick + end );

    //user
    QByteArray byteUser( "USER " );
    byteUser.append( nick + " " + nick + " " + nick + " " + nick + " : " + nick + end );

    //join request
    QByteArray byteJoin( "JOIN " );
    byteJoin.append( data.value( "chan" ) + end );


    qDebug() << byteNick;
    qDebug() << byteUser;
    qDebug() << byteJoin;

    connectionSocket()->write( byteNick );
    connectionSocket()->write( byteUser );
    connectionSocket()->write( byteJoin );
}

void IrcController::ircCommandParser( const QByteArray &user, const QByteArray &msg, const QByteArray &ip )
{
    qDebug() << "MESSAGE I GET IS -> " << msg;
    //help
    if( msg.contains( "!help" ) ) {
        //print help
        //connectionSocket()->write(
        qDebug( "IrcController::ircCommandParser::help NEED TO IMPLEMENT" );
    }

    //auth user
    else if( msg.contains( "!auth" ) ) {    //!auth <password>
        QList< QByteArray >aux = msg.split( ' ' );

        if( aux.size() > 2 ) {
            connectionSocket()->write( genPrivateMessage( user, "too many parameters. send me-> '!auth <password>'" ) );
            return;
        }
        else {
            if( !m_dbController->auth( user, aux.at( 1 )/* <- password*/, ip ) ) {
                //not authed
                connectionSocket()->write( genPrivateMessage( user, "NOT AUTHED!Either you used the wrong password or you're not on my database" ) );
                return;
            }
            //been authed
            connectionSocket()->write( genPrivateMessage( user, "you have been authenticated to ioQIC " ) );
        }
    }

    //kick
    //TODO fix kick reason. if given " asdasasd asd asd" it kick with "asd" as only reason
    else if( msg.contains( "!kick" ) ) {    //!kick <nick> <reason>
        QList< QByteArray >aux = msg.split( ' ' );

        if( aux.size() == 1 ) { //too few arguments
            connectionSocket()->write( genPrivateMessage( user, "too few arguments. '!kick <nick> <reason>'" ) );
            return;
        }

        QByteArray reason( "kick" );
        if( aux.size() >= 3 ) { //got reason!
            reason.clear();
            for( int i = 2; i < aux.size(); i++ ) {
                reason.append( aux.at( i ) );//+ " " ); put " " in front??
                reason.append( " " );
                qDebug() << "REASON = " << reason;
            }
        }
        //send kick
        qDebug() << "sending kick command-> " << reason;
        kick( aux.at( 1 ), reason );
    }
}

void IrcController::pong( const QByteArray &pingData )
{
    QList<QByteArray>pingSplit = pingData.split( ':' );
    connectionSocket()->write( "PONG :" + pingSplit.at( 1 ) + end );
}

/*******************************
*      PRIVATE FUNCTIONS       *
********************************/

bool IrcController::auth( const QByteArray &user, const QByteArray &password )
{
    //call database and do query
    qDebug( "\e[1;31mIrcController::auth NEED TO IMPLEMENT\e[0m" );
    return false;
}

bool IrcController::checkIfAuthed( const QByteArray &nick )
{
    //check authed table
    qDebug( "\e[1;31mIrcController::checkIfAuthed NOT IMPLEMENTED YET\e[0m" );
    return false;
}

QByteArray IrcController::genChannelMessage( const QByteArray &messageToSend )
{
    QMap< QString, QString > auxSettings = m_connection->ircSettings();
    QByteArray aux( "PRIVMSG " );
    aux.append( auxSettings.value( "chan" ) + " " + messageToSend.trimmed() + " " + end );
    return aux;
}

QByteArray IrcController::genPrivateMessage( const QByteArray &nick, const QByteArray &messageToSend )
{
    QByteArray aux( "PRIVMSG " );
    aux.append( nick + " :" + messageToSend.trimmed() + end );
    return aux;
}

void IrcController::kick( const QByteArray &nick, const QByteArray &reason )
{
    QByteArray aux( "KICK " );
    QMap< QString, QString > auxSettings = m_connection->ircSettings();
    aux.append( auxSettings.value( "chan" ) + " " + nick + " " + reason.trimmed() + end );
    qDebug() << "AUX IS: " << aux;
    connectionSocket()->write( aux );
}
