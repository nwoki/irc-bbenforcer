/*****************************************************************************************
 * ioQIC-BBEnforcer - Qt irc bot that also sends rcon commands to UrbanTerror game server*
 * Copyright (C) 2010 - 2011, (n)woki                                                    *
 *                                                                                       *
 * brain.cpp is part of ioQIC-BBEnforcer                                                 *
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

#include "brain.h"
#include "dbcontroller.h"
#include "gamecontroller.h"
#include "irccontroller.h"

#include <QAbstractSocket>
#include <QDebug>

Brain::Brain()
    : QObject( 0 )
    , m_dbControl( 0 )
    , m_gameControl( 0 )
    , m_ircControl( 0 )
{
    qDebug( "Brain::Brain" );

    m_dbControl = new DbController();
    m_ircControl = new IrcController( m_dbControl );
//     m_gameControl = new GameController( db );


    //read from server when data is available
    connect( m_ircControl->connectionSocket(), SIGNAL( readyRead() ), this, SLOT( parseIrcData() ) );
//     connect( m_gameControl->connectionSocket(), SIGNAL( readyRead() ), this, SLOT( parseData() ) );
}

Brain::~Brain()
{
    //delete m_dbControl;
    delete m_gameControl;
    delete m_ircControl;
}

QByteArray Brain::extractIp( const QByteArray &text )
{
    QList< QByteArray >aux = text.split( '@' );
    aux = aux.value( 1 ).split( ' ' );
    return aux.value( 0 ).trimmed();
}

QByteArray Brain::extractText( const QByteArray &text )
{
    QList< QByteArray >aux = text.split( ':' );
    return aux.value( 2 ).trimmed();
}

QByteArray Brain::extractNick( const QByteArray &text )
{
    QList< QByteArray >aux = text.split( '!' );
    return aux.value( 0 ).right( aux.value( 0 ).size() - 1 ).trimmed();
}

QByteArray Brain::extractUserLogin( const QByteArray& text )
{
    QList< QByteArray >aux = text.split( '@' );
    QList< QByteArray >aux2 = aux.at( 0 ).split( '~' );
    return aux2.value( 1 );
}

/*******************
*      SLOTS       *
********************/

//from here i dispatch irc lines with commands to corrispective classes
void Brain::parseIrcData()
{
    while( m_ircControl->connectionSocket()->bytesAvailable() ) {
        QByteArray serverText = m_ircControl->connectionSocket()->readLine( 2000 );
        qDebug() << serverText;

        // start sending info to login and join
        if( serverText.contains( "NOTICE AUTH :*** Found your hostname" ) ) {
            m_ircControl->logIn();
            return;
        }

        // extra login send to make sure i get in channel
        else if( serverText.contains( "NOTICE AUTH :*** No ident response" ) ) {
            m_ircControl->logIn();
            return;
        }

        // send back ping data
        else if( serverText.contains( "PING" ) ) {
            m_ircControl->pong( serverText );
            return;
        }

        // on join, add user to transition database and check if banned
        else if( serverText.contains( "JOIN" ) ) {
            QByteArray user = extractNick( serverText );
            QByteArray userLogin = extractUserLogin( serverText );
            QByteArray ip = extractIp( serverText );

            m_dbControl->addToTransition( user, userLogin, ip );

//             if( m_dbControl->isBanned( userLogin, ip ) )
//             { /* BAN + kick */ }///TODO!!!!!!!!!!!!!
        }

        // control this after i get the "end of" line from server
        else if( serverText.contains( "PRIVMSG" ) ) {           // someone's talking
            QByteArray user = extractNick( serverText );
            QByteArray msg = extractText( serverText );
            QByteArray ip = extractIp( serverText );

            if( msg.startsWith( '!' ) ) {                       // irc command is "!"
                qDebug() << user << " ASKED FOR IRC BOT COMMAND with :" << msg;
                m_ircControl->ircCommandParser( user, msg, ip );
            }

            else if( msg.startsWith( '@' ) ) {                  // game server command is "@"
                qDebug() << user << " ASKED FOR GAME BOT COMMAND with :" << msg;
                qDebug( "\e[1;31mBrain::parseData game server command -> need to implement! \e[0m" );
            }

            else                                                // nothing, normal msg
                qDebug() << user << " SENT NORMAL MESSAGE.LOG IT SOMEWHERE!";
        }
    }
}
