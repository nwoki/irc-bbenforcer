/*****************************************************************************************
 * ioQIC-BBEnforcer - Qt irc bot that also sends rcon commands to UrbanTerror game server*
 * Copyright (C) 2010 - 2011, (n)woki                                                    *
 *                                                                                       *
 * gamecontroller.cpp is part of ioQIC-BBEnforcer                                        *
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
#include "ircuserscontainer.h"
#include "gamecontroller.h"

#include <QDir>
#include <QSettings>

#define RCON_START "\xff\xff\xff\xffrcon "


GameController::GameController( DbController *db, IrcUsersContainer *ircUsers )
    : QObject( 0 )
    , m_db( db )
    , m_ircUsers( ircUsers )
    , m_socket( new QUdpSocket() )
    , m_ip( QString() )
    , m_rconPass( QString() )
    , m_port( 0 )
{
    loadSettings();

    connect( m_socket, SIGNAL( connected() ), this, SLOT( connectNotify() ) );

    // connect
    m_socket->connectToHost( m_ip, m_port, QIODevice::ReadWrite );
}


QUdpSocket *GameController::connectionSocket() const
{
    return m_socket;
}


void GameController::gameCommandParser( const QByteArray& nick, const QByteArray& msg, const QByteArray& ip )
{
    // check if user is banned. Only non-banned users can use the bot.
    // This is to prevent actions from outside the channel
    IrcUsersContainer::WhoisStruct *ircUser = m_ircUsers->user( nick );

    if( m_db->isBanned( ircUser->userLogin(), ircUser->ip() ) ) {
        emit( messageToUserSignal( ircUser->nick(), "you can't use me, you're currently banned. Sucks to be you" ) );
        return;
    }

    qDebug() << "MESSAGE I GET IS -> " << msg;
    QList< QByteArray > msgList = msg.split( ' ' ); // split message

    QString cmd = msgList.at( 0 );              // command given by user

    if( cmd == "@bigtext" )                     // send bigtext to server
        bigText( nick, ip, msgList );
    else if( cmd == "@gravity" )                // set server gravity
        gravity( nick, ip, msgList );
    else if( cmd == "@nextmap" )                // set server nextmap
        nextMap( nick, ip, msgList );
    else if( cmd == "@map" )                    // set server current map
        map( nick, ip, msgList );
    else if( cmd == "@status" )                 // send "status" command to server
        status( nick, ip );
    else if( cmd == "@config" )                 // exec config file
        execConfig( nick, ip, msgList );
    else if( cmd == "@restart" )                // restart game
        restart( nick, ip );
    else if( cmd == "@reload" )                 // reload game
        reload( nick, ip );
}


QByteArray GameController::nextUserInLine()
{
    QByteArray nick;
    if( !m_userList.isEmpty() ) {   // no users in line
        nick = m_userList.first();
        m_userList.pop_front();     // remove user from list
    }
    return nick;
}


/*******************************
 *      PRIVATE FUNCTIONS      *
 *******************************/

/*********
 * SLOTS *
 ********/
void GameController::connectNotify()
{
    qDebug() << "Connected to host " << m_ip << ":" << m_port;
}


/*****************
* game functions *
*****************/
void GameController::bigText( const QByteArray& nick, const QByteArray& ip, const QList< QByteArray >& msgList )
{
    qDebug( "GameController::bigText" );

    IrcUsersContainer::WhoisStruct *ircUser = m_ircUsers->user( nick );

    if( msgList.count() > 1 ) {
    QByteArray bigtext;
        if( m_db->isAuthed( ircUser->userLogin(), ip ) ) {  // gerate command
            QByteArray cmd = cmdBegin();
            cmd.append( " bigtext " );

            bigtext.append( '"' );

            for( int i = 1; i < msgList.count(); i++ )
                bigtext.append( msgList.at( i ) + " " );

            bigtext.append( '"' );

            m_socket->write( cmd + bigtext );
            emit( messageToUserSignal( nick, "sent bigtext: " + bigtext ) );
        }
        else
            emit( notAuthedSignal( nick ) );
    }
    else
        emit( messageToUserSignal( nick, "wrong number of parameters. [usage]@bigtext <text>" ) );
}


void GameController::execConfig( const QByteArray& nick, const QByteArray& ip, const QList< QByteArray >& msgList )
{
    qDebug( "GameController::execConfig" );

    IrcUsersContainer::WhoisStruct *ircUser = m_ircUsers->user( nick );

    if( msgList.count() == 2 ) {
        if( m_db->isAuthed( ircUser->userLogin(), ip ) ) {
            QByteArray cmd = cmdBegin();
            cmd.append( " exec " );
            cmd.append( msgList.last() );   // config file
            m_socket->write( cmd );
            m_userList.append( nick );      // add to userlist for config status response
        }
        else
            emit( notAuthedSignal( nick ) );
    }
    else
        emit( messageToUserSignal( nick, "wrong number of parameters. [usage]@config <configFile> " ) );
}


void GameController::gravity( const QByteArray& nick, const QByteArray& ip, const QList< QByteArray >& msgList )
{
    qDebug( "GameController::gravity" );

    IrcUsersContainer::WhoisStruct *ircUser = m_ircUsers->user( nick );

    if( msgList.count() == 2 ) {
        bool ok;

        msgList.at( 1 ).toInt( &ok );

        if( !ok ) {
            emit( messageToUserSignal( nick, "the value for gravity has to be a number" ) );
            return;
        }

        if( m_db->isAuthed( ircUser->userLogin(), ip ) ) {  // gerate command
            QByteArray cmd = cmdBegin();
            cmd.append( " set g_gravity " );
            cmd.append( msgList.at( 1 ) );              // number

            m_socket->write( cmd );
            emit( messageToUserSignal( nick, "gravity set to " + msgList.at( 1 ) ) );
        }
        else
            emit( notAuthedSignal( nick ) );
    }
    else
        emit( messageToUserSignal( nick, "wrong number of parameters. [usage]@gravity <num>" ) );
}


void GameController::map( const QByteArray& nick, const QByteArray& ip, const QList< QByteArray >& msgList )
{
    qDebug( "GameController::map" );

    IrcUsersContainer::WhoisStruct *ircUser = m_ircUsers->user( nick );

    if( msgList.count() == 2 ) {
        if( m_db->isAuthed( ircUser->userLogin(), ip ) ) {  // gerate command
            QByteArray cmd = cmdBegin();
            cmd.append( " map " );
            cmd.append( msgList.at( 1 ) );                // map

            m_socket->write( cmd );
            emit( messageToUserSignal( nick, "map set to: " + msgList.at( 1 ) ) );
        }
        else
            emit( notAuthedSignal( nick ) );
    }
    else
        emit( messageToUserSignal( nick, "wrong number of parameters. [usage]@map <map>" ) );
}


void GameController::nextMap( const QByteArray& nick, const QByteArray& ip, const QList< QByteArray >& msgList )
{
    qDebug( "GameController::nextMap" );

    IrcUsersContainer::WhoisStruct *ircUser = m_ircUsers->user( nick );

    if( msgList.count() == 2 ) {
        if( m_db->isAuthed( ircUser->userLogin(), ip ) ) {  // gerate command
            QByteArray cmd = cmdBegin();
            cmd.append( " set g_nextmap " );
            cmd.append( msgList.at( 1 ) );                // map

            m_socket->write( cmd );
            emit( messageToUserSignal( nick, "next map set to: " + msgList.at( 1 ) ) );
        }
        else
            emit( notAuthedSignal( nick ) );
    }
    else
        emit( messageToUserSignal( nick, "wrong number of parameters. [usage]@nextmap <map>" ) );
}


void GameController::reload( const QByteArray& nick, const QByteArray& ip )
{
    qDebug( "GameController::reload" );

    IrcUsersContainer::WhoisStruct *ircUser = m_ircUsers->user( nick );

    if( m_db->isAuthed( ircUser->userLogin(), ip ) ) {
        QByteArray cmd = cmdBegin();
        cmd.append( " reload" );

        m_socket->write( cmd );
        emit( messageToUserSignal( nick, "reloading game" ) );
    }
    else
        emit( notAuthedSignal( nick ) );
}


void GameController::restart( const QByteArray& nick, const QByteArray& ip )
{
    qDebug( "GameController::restart" );

    IrcUsersContainer::WhoisStruct *ircUser = m_ircUsers->user( nick );

    if( m_db->isAuthed( ircUser->userLogin(), ip ) ) {
        QByteArray cmd = cmdBegin();
        cmd.append( " restart" );

        m_socket->write( cmd );
        emit( messageToUserSignal( nick, "restarting game" ) );
    }
    else
        emit( notAuthedSignal( nick ) );
}


void GameController::status( const QByteArray& nick, const QByteArray &ip )
{
    qDebug( "GameController::status" );

    IrcUsersContainer::WhoisStruct *ircUser = m_ircUsers->user( nick );

    if( m_db->isAuthed( ircUser->userLogin(), ip ) ) {  // generate status command
        QByteArray cmd = cmdBegin();
        cmd.append( " status" );

        m_socket->write( cmd );
        m_userList.append( nick );      // add user to list
    }
    else                                // tell user he/she's not authed
        emit( notAuthedSignal( nick ) );
}


/***********
 * PRIVATE *
 **********/
QByteArray GameController::cmdBegin()
{
    QByteArray cmd( RCON_START );
    cmd.append( m_rconPass );
    return cmd;
}


void GameController::loadSettings()
{
    qDebug( "GameController::loadSettings" );
    //set config file
    QSettings settings( QDir::toNativeSeparators( "cfg/config" ), QSettings::IniFormat );

    if( settings.status() == QSettings::FormatError ) {
        qWarning( "\e[1;31m[FAIL] GameController::loadSettings FAILED to load settings. Format Error, check your config file\e[0m" );
        return;
    }

    qDebug( "GameController::loadSettings GAME SERVER SETTINGS" );

    settings.beginReadArray( "GAMESERVER" );

    m_ip = settings.value( "ip" ).toString();
    if( m_ip.isEmpty() ) {
        qWarning( "\e[1;31m[FAIL]GameController::loadSettings can't load 'ip'. Check your config file\e[0m" );
        return;
    }

    bool ok;
    m_port = settings.value( "port" ).toInt( &ok );
    if( !ok ) {
        qWarning( "\e[1;31m[FAIL]GameController::loadSettings can't load 'port'. Check your config file.\e[0m" );
        return;
    }

    m_rconPass = settings.value( "rconPass" ).toString();

    if( m_rconPass.isEmpty() )
        qWarning( "\e[1;31mWARNING: empty rcon pass!" );

    // close settings file
    settings.endArray();
    qDebug() << "Settings: " << m_ip << "  " << m_port << " " << m_rconPass;
}
