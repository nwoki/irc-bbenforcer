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
#include <QTimer>

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


void IrcController::ircCommandParser( const QByteArray &user, const QByteArray &msg, const QByteArray &ip )
{
    qDebug() << "MESSAGE I GET IS -> " << msg;

    if( msg.contains( "!help" ) )   /* print help */
        help( user );

    else if( msg.contains( "!auth" ) )   /* auth user ( !auth <password> ) */
        auth( user, msg, ip );

    //kick
    //TODO fix kick reason. if given " asdasasd asd asd" it kick with "asd" as only reason
    else if( msg.contains( "!kick" ) )    /* !kick <nick> <reason> */
        kick( user, msg, ip );
}


void IrcController::logIn()
{
    qDebug( "IrcController::logIn" );
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


void IrcController::pong( const QByteArray &pingData )
{
    QList<QByteArray>pingSplit = pingData.split( ':' );
    connectionSocket()->write( "PONG :" + pingSplit.at( 1 ) + end );
}

/*******************************
*      PRIVATE FUNCTIONS       *
********************************/

/***********************************
**         IRC FUNCTIONS          **
***********************************/
void IrcController::auth( const QByteArray &user, const QByteArray &msg, const QByteArray &ip )
{
    QList< QByteArray >aux = msg.split( ' ' );

    if( aux.size() > 2 ) {  /* too many parameters, abort */
        sendPrivateMessage( user, "too many parameters. send me-> '!auth <password>'" );
        return;
    }
    else {
        DbController::authMsg response;

        response = m_dbController->auth( user, aux.at( 1 )/*<- password*/, ip );

        if( response == DbController::ALREADY_AUTHED )
            sendPrivateMessage( user, "you're already authed!" );
        else if( response == DbController::AUTH_FAIL )
            sendPrivateMessage( user, "NOT AUTHED! wrong username/password" );
        else if( response == DbController::AUTH_OK )
            sendPrivateMessage( user, "you have been authenticated to ioQIC");
        else
            sendPrivateMessage( user, "problem with database, please contact an administrator" );
    }
}

void IrcController::flood( const QByteArray &user, const QByteArray &msg, const QByteArray &ip )
{
    if( !isAuthed( user, ip ) ) { /* not authed */
        sendPrivateMessage( user, "you're not authed to ioQIC" );
        return;
    }

}


void IrcController::help( const QByteArray &user )
{
    qDebug( "IrcController::ircCommandParser::help NEED TO IMPLEMENT" );
    /*
     * single shot here
     */
//    QTimer::singleShot( 500, m_connection, sendPrivateMessage( user, "sei un gay" ) );
}


void IrcController::kick( const QByteArray &user, const QByteArray &msg, const QByteArray &ip )
{
    if( !isAuthed( user, ip ) ) {   /* not authed */
        sendPrivateMessage( user, "you're not authed to ioQIC" );
        return;
    }

    QList< QByteArray >aux = msg.split( ' ' );

    if( aux.size() == 1 ) { // too few arguments
        sendPrivateMessage( user, "too few arguments. '!kick <nick> <reason>'" );
        return;
    }

    QByteArray reason( "kick" );    // this is for default in case no reason is specified
    if( aux.size() >= 3 ) {         // got reason!
        reason.clear();
        for( int i = 2; i < aux.size(); i++ ) {
            reason.append( aux.at( i ) );   //+ " " ); put " " in front??
            reason.append( " " );
            qDebug() << "REASON = " << reason;
        }
    }
    // send kick
    qDebug() << "sending kick command-> " << reason;

    #warning TODO fix reason on kick, sends only last word from a phrase

    QByteArray cmd( "KICK " );
    QMap< QString, QString > auxSettings = m_connection->ircSettings();

    cmd.append( auxSettings.value( "chan" ) + " " + /*nick*/aux.at( 1 ) + " " + reason.trimmed() + end );
    qDebug() << "command to send IS: " << cmd;
    connectionSocket()->write( cmd );
}



/***********************************
**         BOT FUNCTIONS          **
***********************************/


bool IrcController::isAuthed( const QByteArray &user, const QByteArray &ip )
{
    return m_dbController->isAuthed( user, ip );
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

void IrcController::sendPrivateMessage( const QByteArray &nick, const QByteArray &message )
{
    connectionSocket()->write( genPrivateMessage( nick, message ) );
}
