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

#include "dbcontroller.h"
#include "irccontroller.h"

#include <QDir>
#include <QSettings>
#include <QTime>
#include <QTimer>

#define end "\r\n"

IrcController::IrcController( DbController *db, IrcUsersContainer *ircUsers )
    : m_connection( new QTcpSocket() )
    , m_dbController( db )
    , m_ircUsers( ircUsers )
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


void IrcController::addToTransition( const QByteArray &nick, IrcUsersContainer::WhoisStruct *ircClient )
{
    // in case the value already exists, it's overwritten
    qDebug( "IrcController::addToTransition" );
    qDebug() << "adding user: " << nick;
    m_ircUsers->addUser( nick, ircClient );

    // DEBUG outputs all users in transition hash
//     QHash<QString,WhoisStruct*>::const_iterator it = m_transitionUsers.constBegin();
//     while( it != m_transitionUsers.constEnd() ) {
//         qDebug() << "user: " << it.key();
//         it++;
//     }
}


void IrcController::channelUsersWhois() const
{
    qDebug( "IrcController::channelUsersWhois" );
    sendIrcCommand( "WHO " + m_chan.toUtf8() );
}


QTcpSocket *IrcController::connectionSocket()
{
    return m_connection;
}


void IrcController::extractUserWhois( const QByteArray& serverText )
{
    qDebug( "IrcController::extractUserWhois" );
    qDebug() << "server text is: " << serverText;

    // extract user data
    QList<QByteArray>auxList = serverText.split( '#' );
    if( auxList.count() < 2 ) {
        qDebug() <<  "\e[1;31m[ERROR] IrcController::extractUserWhois got a messed up line( auxList ): '" << serverText << "' \e[0m;";
        return;
    }

    QList<QByteArray>auxList2 = auxList.at( 1 ).split( ' ' );
    if( auxList2.count() < 5 ) {
        qDebug() <<  "\e[1;31m[ERROR] IrcController::extractUserWhois got a messed up line( auxList2 ): '" << serverText << "' \e[0m;";
        return;
    }

    IrcUsersContainer::WhoisStruct *whoStr = new IrcUsersContainer::WhoisStruct( auxList2.at( 4 ), auxList2.at( 1 ), auxList2.at( 2 ) );

    qDebug() << "\e[1;32mUSER IS: " << whoStr->nick() << " " << whoStr->ip() << " " << whoStr->userLogin() << "\e[0m";

    // then add to transition users
    addToTransition( whoStr->nick(), whoStr );
}


void IrcController::ircCommandParser( const QByteArray &nick, const QByteArray &msg, const QByteArray &ip )
{
    // check if user is banned. Only non-banned users can use the bot.
    // This is to prevent actions from outside the channel
    IrcUsersContainer::WhoisStruct *ircUser = m_ircUsers->user( nick );

    if( ircUser == 0 ) {            // don't have the user in memory
        sendPrivateMessage( nick, "I don't have your information. Retry the command. If this doesn't work, lease reconnect to channel" );
        singleUserWhois( nick );    // try to update user info
        return;
    }

    if( m_dbController->isBanned( ircUser->userLogin(), ircUser->ip() ) ) {
        sendPrivateMessage( ircUser->nick(), "you can't use me, you're currently banned. Sucks to be you" );
        return;
    }

    qDebug() << "MESSAGE I GET IS -> " << msg;
    QList< QByteArray > msgList = msg.split( ' ' ); // split message

    QString command = msgList.at( 0 );              // command given by user

    if( command == "!help" )                        // print help
        help( nick );
    else if( command == "!auth" )                   // auth user ( !auth <password> )
        auth( nick, msgList, ip );
    else if( command == "!kick" )                   // !kick <nick> <reason>
        kick( nick, msgList, ip );
    else if( command == "!ban" )                    // !ban <nick> <reason>
        ban( nick, msgList, ip );
    else if( command == "!op" )                     // !op <nick> <new password>
        addOp( nick, msgList, ip );
    else if( command == "!deop" )                   // !deop <nick>
        deop( nick, msgList, ip );
    else if( command == "!kickban" )                // !kickban <nick> <kick message>
        kickBan( nick, msgList, ip );
    else if( command == "!unban" )                  // unban <nick>
        unban( nick, msgList, ip );
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
    // get settings
    QMap< QString, QString > data = ircSettings();
    QString nick = data.value( "nick" );

    // nick
    QByteArray byteNick( "NICK " );
    byteNick.append( nick + end );

    // user
    QByteArray byteUser( "USER " );
    byteUser.append( nick + " " + nick + " " + nick + " " + nick + " : " + nick + end );

    // join request
    QByteArray byteJoin( "JOIN " );
    byteJoin.append( data.value( "chan" ) + end );


    qDebug() << byteNick;
    qDebug() << byteUser;
    qDebug() << byteJoin;

    m_connection->write( byteNick );
    m_connection->write( byteUser );
    m_connection->write( byteJoin );
}


void IrcController::loginBan( const QByteArray& nick, const QByteArray& userLogin, const QByteArray& ip )
{
    qDebug( "IrcController::loginBan" );

    botBan( userLogin, ip );                // apply ban to user
    botKick( nick, "you're busted dude" );  // then kick
}


void IrcController::pong( const QByteArray &pingData )
{
    QList<QByteArray>pingSplit = pingData.split( ':' );
    sendIrcCommand( "PONG :" + pingSplit.at( 1 ) );
}


void IrcController::sendLineToUser( const QByteArray& nick, const QByteArray& line )
{
    sendPrivateMessage( nick, line );
}


void IrcController::singleUserWhois( const QByteArray& nick )
{
    qDebug( "IrcController::channelUsersWhois" );
    sendIrcCommand( "WHO " + nick );
}


void IrcController::updateUserStruct( const QByteArray& oldNick, const QByteArray& line )
{
    qDebug( "IrcController::updateUserStruct" );

    QList<QByteArray> msg = line.split( ':' );
    qDebug() << "Old nick -> " << oldNick << " new nick " << msg.last();

    if( !m_ircUsers->updateUserNick( oldNick, msg.last() ) )
        sendPrivateMessage( msg.last(), "error looking up your info. Please contact an admin" );
}


/*********
 * SLOTS *
 ********/
void IrcController::messageToUserSlot( const QByteArray& nick, const QByteArray& message )
{
    sendPrivateMessage( nick, message );
}


void IrcController::singleUserWhoisSlot( const QByteArray& nick )
{
    singleUserWhois( nick );
}


void IrcController::userNotAuthedSlot( const QByteArray& nick )
{
    sendNotAuthedMessage( nick );
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
    m_connection->abort();  // reset socket

    switch ( error ) {
        case QAbstractSocket::ConnectionRefusedError: {
            qWarning() << "\e[0;33m" << m_connection->errorString() << ", trying to reconnect..\e[0m";
            QTimer::singleShot( 3000, this, SLOT( reconnect() ) );
            //what to do?
            break;
        }
        case QAbstractSocket::RemoteHostClosedError: {
            qWarning() << "\e[0;33m" << m_connection->errorString() << ", trying to reconnect..\e[0m";
            QTimer::singleShot( 3000, this, SLOT( reconnect() ) );
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
            QTimer::singleShot( 3000, this, SLOT( reconnect() ) );
            #warning FIX ME crashes when i get this error and try to reconnect. Does it?
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
void IrcController::addOp( const QByteArray& nick, const QList< QByteArray >& msg, const QByteArray& ip )
{
    if( msg.size() != 3 ) {         // wrong arguments
        sendPrivateMessage( nick, "wrong arguments arguments. '!addOp <nick> <password>'" );
        return;
    }

    IrcUsersContainer::WhoisStruct *ircUser = m_ircUsers->user( nick );

    if( ircUser == 0 ) {
        qDebug() << "\e[1;31m[ERROR] IrcController::addOp can't find WhoisStruct for nick : " << nick << "\e[0m";
        sendPrivateMessage( nick, "error looking up your info. Please contact an admin" );
        return;
    }

    if( !m_dbController->isAuthed( ircUser->userLogin(), ip ) ) {   // not authed
        sendNotAuthedMessage( nick );
        return;
    }

    IrcUsersContainer::WhoisStruct *newAdmin = m_ircUsers->user( msg.at( 1 ) );

    if( newAdmin == 0 ) {       // don't have info
        qDebug() << "\e[1;31m[ERROR] IrcController::addOp can't find WhoisStruct for nick : " << msg.at( 1 ) << "\e[0m";
        sendPrivateMessage( nick, "error looking up info. Please contact an admin" );
        return;
    }

    DbController::opMsg result = m_dbController->addToOplist( newAdmin->userLogin(), msg.at( 2 ) );

    if( result == DbController::OP_OK )
        sendPrivateMessage( nick, "user '" + newAdmin->userLogin() + "' added to oplist" );
    else if( result == DbController::ALREADY_OPPED )
        sendPrivateMessage( nick, "user " + newAdmin->userLogin() + " is already in the oplist table" );
    else if( result == DbController::OP_DATABASE_ERROR )
        sendPrivateMessage( nick, "ERROR WITH DATABASE when adding: " + newAdmin->userLogin() + " to oplist" );
}


void IrcController::auth( const QByteArray &nick, const QList< QByteArray > &msg, const QByteArray &ip )
{
    IrcUsersContainer::WhoisStruct *ircUser = m_ircUsers->user( nick );

    if( msg.size() > 2 || msg.size() == 1 || ircUser == 0 ) {       // wrong parameters, abort
        sendPrivateMessage( nick, "wrong parameters. send me-> '!auth <password>'" );
        return;
    }
    else {
        DbController::authMsg response;

        response = m_dbController->auth( ircUser->userLogin(), msg.at( 1 )/*<- password*/, ip );

        if( response == DbController::ALREADY_AUTHED )
            sendPrivateMessage( nick, "you're already authed!" );
        else if( response == DbController::AUTH_FAIL )
            sendPrivateMessage( nick, "NOT AUTHED! wrong username/password" );
        else if( response == DbController::AUTH_OK )
            sendPrivateMessage( nick, "you have been authenticated to ioQIC" );
        else
            sendPrivateMessage( nick, "problem with database, please contact an administrator" );
    }
}


void IrcController::ban( const QByteArray& nick, const QList< QByteArray >& msg, const QByteArray& ip )
{
    IrcUsersContainer::WhoisStruct *ircUser = m_ircUsers->user( nick ); // user requesting

    if( !m_dbController->isAuthed( ircUser->userLogin(), ip ) ) { // not authed
        sendNotAuthedMessage( nick );
        return;
    }

    if( msg.size() == 1 ) {         // too few arguments
        sendPrivateMessage( nick, "too few arguments. '!ban <nick>'" );
        return;
    }

    // check to see someone isn't trying to ban me
    if( msg.at( 1 ) == m_nick ) {
        sendPrivateMessage( nick, "I am your god and you can't ban me with my own commands. You shall be punished" );
        botBan( ircUser->userLogin(), ircUser->ip() );
        botKick( ircUser->userLogin(), "TASTE MY WRATH!" );
        return;
    }

    // user to ban
    IrcUsersContainer::WhoisStruct *userStruct = m_ircUsers->user( msg.at( 1 ) /*nick*/ );

    // proceed with ban
    QByteArray cmd( "MODE " );
    cmd.append( m_chan );
    cmd.append( " +b *!" );
    cmd.append( userStruct->userLogin() );
    cmd.append( "@" );
    cmd.append( userStruct->ip() );

    // send to irc chan
    sendIrcCommand( cmd );

    // add to database
    m_dbController->addToBanned( userStruct->nick(), userStruct->userLogin(), userStruct->ip(), nick, QDateTime::currentDateTime().toString( "dd-mm-yyyy hh:mm:ss" ) );
}


void IrcController::botBan( const QByteArray& userLogin, const QByteArray& ip )
{
    QByteArray cmd( "MODE " );
    cmd.append( m_chan );
    cmd.append( " +b *!" );
    cmd.append( userLogin );
    cmd.append( "@" );
    cmd.append( ip );

    sendIrcCommand( cmd );
}


void IrcController::botKick( const QByteArray& nick, const QString& reason )
{
    QByteArray cmd( "KICK " );
    cmd.append( m_chan + " " + nick + " :" + reason + end );
    sendIrcCommand( cmd );
}


void IrcController::deop(const QByteArray& nick, const QList< QByteArray >& msg, const QByteArray& ip)
{
    // user requesting command
    IrcUsersContainer::WhoisStruct *ircUser = m_ircUsers->user( nick );

    if( !m_dbController->isAuthed( ircUser->userLogin(), ip ) ) { // not authed
        sendNotAuthedMessage( nick );
        return;
    }

    if( msg.size() != 2 ) { // wrong arguments
        sendPrivateMessage( nick, "wrong arguments. [usage]'!deop <nick>'" );
        return;
    }

    // get user to deop
    ircUser = m_ircUsers->user( msg.at( 1 ) );

    if( ircUser != 0 ) {
        if( !m_dbController->removeFromOplist( ircUser->userLogin() ) )
            sendPrivateMessage( nick, "ERROR: couldn't remove user from oplist" );
        else
            sendPrivateMessage( nick, "user removed successfully from database" );
    }
    // can't find user info
    else
        sendPrivateMessage( nick, "can't find user info for " + msg.at( 1 ) );
}


void IrcController::help( const QByteArray &nick )
{
    qDebug( "IrcController::ircCommandParser::help" );
    sendPrivateMessage( nick, "-> http://2s2h.com/ioQIC-BBEnforcer/ioQIC-BBEnforcer-README" );
}


void IrcController::kick( const QByteArray &nick, const QList< QByteArray > &msg, const QByteArray &ip )
{
    IrcUsersContainer::WhoisStruct *ircUser = m_ircUsers->user( nick );

    if( !m_dbController->isAuthed( ircUser->userLogin(), ip ) ) {   // not authed
        sendNotAuthedMessage( nick );
        return;
    }

    if( msg.size() == 1 ) {         // too few arguments
        sendPrivateMessage( nick, "too few arguments. '!kick <nick> <reason>'" );
        return;
    }

    // check to see someone isn't trying to ban me
    if( msg.at( 1 ) == m_nick ) {
        sendPrivateMessage( nick, "I am your god and you can't kick me with my own commands. You shall be punished" );
        botBan( ircUser->userLogin(), ircUser->ip() );
        botKick( ircUser->userLogin(), "TASTE MY WRATH!" );
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
    sendIrcCommand( cmd );
}


void IrcController::kickBan( const QByteArray& nick, const QList< QByteArray >& msg, const QByteArray& ip )
{
    // :)
    ban( nick, msg, ip );
    kick( nick, msg, ip );
}


void IrcController::unban( const QByteArray& nick, const QList< QByteArray >& msg, const QByteArray& ip )
{
    if( msg.count() != 2 ) {
        sendPrivateMessage( nick, "wrong parameters! [usage]!unban <nick>" );
        return;
    }

    IrcUsersContainer::WhoisStruct *ircUser = m_ircUsers->user( nick );
    IrcUsersContainer::WhoisStruct *userToUnban = m_ircUsers->user( msg.at( 1 ) );

    if( ircUser == 0 ) {
        sendPrivateMessage( nick, "error can't find your info. re-enter channel to reload data" );
        return;
    }

    if( userToUnban == 0 ) {
        sendPrivateMessage( nick, "error can't find user '" + msg.at( 1 ) + "' info." );
        return;
    }

    if( m_dbController->isAuthed( ircUser->userLogin(), ip ) ) {
        if( !m_dbController->removeFromBanned( userToUnban->userLogin() ) )
            sendPrivateMessage( nick, "ERROR removing banned user from database!" );
        else
            sendPrivateMessage( nick, "'" + userToUnban->nick() + "' removed from banned users" );

        // unban from irc channel
        QByteArray unbanCmd( "MODE " );
        unbanCmd.append( m_chan );
        unbanCmd.append( " -b *!" );
        unbanCmd.append( userToUnban->userLogin() );
        unbanCmd.append( "@" );
        unbanCmd.append( userToUnban->ip() );

        sendIrcCommand( unbanCmd );
    }
    else
        sendNotAuthedMessage( nick );
}



/***********************************
**         BOT FUNCTIONS          **
***********************************/
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


void IrcController::sendNotAuthedMessage( const QByteArray& nick )
{
    sendPrivateMessage( nick, "you're not authed to ioQIC-BBEnforcer" );
}


void IrcController::sendPrivateMessage( const QByteArray &nick, const QByteArray &message )
{
    sendIrcCommand( genPrivateMessage( nick, message ) );
}


void IrcController::whois( const QByteArray &nick )
{
    qDebug( "IrcController::whois" );
    qDebug() << "requesting info for: " << nick;
    QByteArray copy = nick;

    if( copy.isEmpty() )    // want channel WHO
        sendIrcCommand( "WHO " + m_chan.toUtf8() );
    else {
        QChar sym = copy.at( 0 );

        if( sym == '+' || sym == '@' )
            copy.remove( 0, 1 );

        sendIrcCommand( "WHO " + copy );
    }
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
    settings.endArray();
}


void IrcController::sendIrcCommand( const QByteArray &command ) const
{
    m_connection->write( command + end );
}


