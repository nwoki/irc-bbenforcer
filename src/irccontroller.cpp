/*****************************************************************************************
 * ioQIC-BBEnforcer - Qt irc bot that also sends rcon commands to UrbanTerror game server*
 * Copyright (C) 2010 - 2011, (n)woki                                                    *
 *                                                                                       *
 * irccontroller.cpp is part of ioQIC-BBEnforcer                                         *
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

#include "irccontroller.h"
#include "dbcontroller.h"

#include <QDir>
#include <QSettings>
#include <QTimer>

#define end "\r\n"

IrcController::IrcController( DbController *db )
    : m_connection( new QTcpSocket() )
    , m_dbController( db )
    , m_port( 0 )
    , m_chan( QString() )
    , m_ip( QString() )
    , m_nick( QString() )
{
    qDebug( "IrcController::IrcController" );

    connect( m_connection, SIGNAL( connected() ), this, SLOT( connectNotify() ) );
    //connection error
    connect( m_connection, SIGNAL( error( QAbstractSocket::SocketError ) ), this, SLOT( handleSocketErrors( QAbstractSocket::SocketError ) ) );
    //disconnect notification
    connect( m_connection, SIGNAL( disconnected() ), this, SLOT( disconnectNotify() ) );

    loadSettings();

    // start connection
    m_connection->connectToHost( m_ip, m_port, QIODevice::ReadWrite );
}

IrcController::~IrcController()
{
    delete m_connection;
    delete m_dbController;
}

QTcpSocket *IrcController::connectionSocket()
{
    return m_connection;
}


void IrcController::ircCommandParser( const QByteArray &user, const QByteArray &msg, const QByteArray &ip )
{
    qDebug() << "MESSAGE I GET IS -> " << msg;
    QList< QByteArray > msgList = msg.split( ' ' ); // split message

    QString command = msgList.at( 0 );              // command given by user

    if( command == "!help" )                        // print help
        help( user );
    else if( command == "!auth" )                   // auth user ( !auth <password> )
        auth( user, msgList, ip );
    else if( command == "!kick" )                   // !kick <nick> <reason>
        kick( user, msgList, ip );
    else if( command == "!ban" )                    // !ban <nick> <reason>
        ban( user, msgList, ip );
}


QMap< QString, QString > IrcController::ircSettings()
{
    QMap< QString, QString >  aux;
    aux.insert( "ip", m_ip );
    aux.insert( "port", QString::number( m_port ) );
    aux.insert( "nick", m_nick );
    aux.insert( "chan", m_chan );
    return aux;
}


void IrcController::logIn()
{
    qDebug( "IrcController::logIn" );
    //get settings
    QMap< QString, QString > data = ircSettings();
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

    m_connection->write( byteNick );
    m_connection->write( byteUser );
    m_connection->write( byteJoin );
}


void IrcController::pong( const QByteArray &pingData )
{
    QList<QByteArray>pingSplit = pingData.split( ':' );
    m_connection->write( "PONG :" + pingSplit.at( 1 ) + end );
}

/*******************************
*      PRIVATE FUNCTIONS       *
********************************/
/*********
 * SLOTS *
 ********/
void IrcController::connectNotify()
{
    qDebug() << "Connected to host " << m_ip << ":" << m_port;
}


void IrcController::disconnectNotify()
{
    qDebug( "Connection::disconnectNotify" );
}


void IrcController::handleSocketErrors( QAbstractSocket::SocketError error )
{
    m_connection->disconnectFromHost(); // or "abort()"?

    switch ( error ) {
        case QAbstractSocket::ConnectionRefusedError: {
            qWarning() << "\e[0;33m" << m_connection->errorString() << ", trying to reconnect..\e[0m";
            QTimer::singleShot( 3000, this, SLOT( reconnect() ) );
            //what to do?
            break;
        }
        case QAbstractSocket::RemoteHostClosedError: {
            qWarning() << "\e[0;33m" << m_connection->errorString() << ", trying to reconnect..\e[0m";
            reconnect();
            break;
        }
        case QAbstractSocket::HostNotFoundError: {
            qWarning() << "\e[0;33m" << m_connection->errorString() << " Please control your config file and check that all values have been inserted correctly\e[0m";
            exit( -1 ); //terminate program
            break;
        }
        case QAbstractSocket::SocketAccessError: {
            qWarning() << "\e[0;33" << m_connection->errorString() << " The application lacks the required privileges\e[0m";
            exit( -1 );
            break;
        }
        case QAbstractSocket::SocketTimeoutError: {
            qWarning() << "\e[0;33m" << m_connection->errorString() << ", trying to reconnect....\e[0m" ;
            reconnect();
            #warning FIX ME crashes when i get this error and try to reconnect.
            break;
        }
        default: {
            qWarning() << "\e[0;33m Following error is not handled -> " << m_connection->errorString();
            break;
        }
    }
}


void IrcController::reconnect()
{
    qDebug( "RECONNECT" );
    m_connection->connectToHost( m_ip, m_port, QIODevice::ReadWrite );
}


/***********************************
**         IRC FUNCTIONS          **
***********************************/
void IrcController::auth( const QByteArray &user, const QList< QByteArray > &msg, const QByteArray &ip )
{
    if( msg.size() > 2 || msg.size() == 1 ) {       // wrong parameters, abort
        sendPrivateMessage( user, "wrong parameters. send me-> '!auth <password>'" );
        return;
    }
    else {
        DbController::authMsg response;

        response = m_dbController->auth( user, msg.at( 1 )/*<- password*/, ip );

        if( response == DbController::ALREADY_AUTHED )
            sendPrivateMessage( user, "you're already authed!" );
        else if( response == DbController::AUTH_FAIL )
            sendPrivateMessage( user, "NOT AUTHED! wrong username/password" );
        else if( response == DbController::AUTH_OK )
            sendPrivateMessage( user, "you have been authenticated to ioQIC" );
        else
            sendPrivateMessage( user, "problem with database, please contact an administrator" );
    }
}


void IrcController::ban( const QByteArray& user, const QList< QByteArray >& msg, const QByteArray& ip )
{
    // MODE #asder +b *!~<nick>@<ip>
    if( !isAuthed( user, ip ) ) {   // not authed
        sendPrivateMessage( user, "you're not authed to ioQIC-BBEnforcer" );
        return;
    }

    if( msg.size() == 1 ) {         // too few arguments
        sendPrivateMessage( user, "too few arguments. '!ban <nick> <reason>'" );
        return;
    }


}


void IrcController::help( const QByteArray &user )
{
    qDebug( "IrcController::ircCommandParser::help NEED TO IMPLEMENT" );
    Q_UNUSED( user );
    /*
     * single shot here
     */
//    QTimer::singleShot( 500, m_connection, sendPrivateMessage( user, "sei un gay" ) );
}


void IrcController::kick( const QByteArray &user, const QList< QByteArray > &msg, const QByteArray &ip )
{
    if( !isAuthed( user, ip ) ) {   // not authed
        sendPrivateMessage( user, "you're not authed to ioQIC-BBEnforcer" );
        return;
    }

    if( msg.size() == 1 ) {         // too few arguments
        sendPrivateMessage( user, "too few arguments. '!kick <nick> <reason>'" );
        return;
    }

    QByteArray reason( "kick" );    // this is for default in case no reason is specified
    if( msg.size() >= 3 ) {         // got reason!
        reason.clear();
        for( int i = 2; i < msg.size(); i++ ) {
            reason.append( msg.at( i ) );
            reason.append( " " );
        }
    }

    // send kick
    QByteArray cmd( "KICK " );
    cmd.append( m_chan + " " + /*nick*/msg.at( 1 ) + " :" + reason.trimmed() + end );
    m_connection->write( cmd );
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
    QMap< QString, QString > auxSettings = ircSettings();
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
    m_connection->write( genPrivateMessage( nick, message ) );
}


/***********
 * PRIVATE *
 **********/
void IrcController::loadSettings()
{
    //set config file
    QSettings settings( QDir::toNativeSeparators( "cfg/config" ), QSettings::IniFormat );

    if( settings.status() == QSettings::FormatError ) {
        qWarning( "\e[1;31m Connection::loadSettings FAILED to load settings. Format Error, check your config file\e[0m" );
        return;
    }

    qDebug( "Connection::loadSettings IRC SETTINGS" );

    settings.beginReadArray( "IRC" );
    m_ip = settings.value( "ip" ).toString();

    if( m_ip.isEmpty() ) {
        qWarning( "\e[1;31mConnection::loadSettings can't load 'ip'. Check your config file\e[0m" );
        return;
    }

    bool ok;
    m_port = settings.value( "port" ).toInt( &ok );
    if( !ok ) {
        qWarning( "\e[1;31mConnection::loadSettings can't load 'port'. Check your config file.\e[0m" );
        return;
    }

    m_chan = settings.value( "chan" ).toString();
    if( m_chan.isEmpty() ) {
        qWarning( "\e[1;31mConnection::loadSettings can't load 'chan'. Check your config file\e[0m" );
        return;
    }

    m_nick = settings.value( "nick" ).toString();
    if( m_nick.isEmpty() ) {
        qWarning( "\e[1;31mConnection::loadSettings can't load 'nick'. Check your config file\e[0m" );
        return;
    }
    qDebug() << ircSettings();
}

