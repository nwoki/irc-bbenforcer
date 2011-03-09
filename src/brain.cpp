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
#include "ircuserscontainer.h"

#include <QDebug>
#include <QTimer>

Brain::Brain()
    : QObject( 0 )
    , m_dbControl( new DbController() )
    , m_ircUsers( new IrcUsersContainer() )
    , m_gameControl( new GameController( m_dbControl, m_ircUsers ) )
    , m_ircControl( new IrcController( m_dbControl, m_ircUsers ) )
{
    qDebug( "Brain::Brain" );

    // read from server when data is available
    connect( m_ircControl->connectionSocket(), SIGNAL( readyRead() ), this, SLOT( parseIrcData() ) );
    connect( m_gameControl->connectionSocket(), SIGNAL( readyRead() ), this, SLOT( parseGameData() ) );

    // connect gameControl signals
    connect( m_gameControl, SIGNAL( notAuthedSignal( QByteArray ) ), m_ircControl, SLOT( userNotAuthedSlot( QByteArray ) ) );
    connect( m_gameControl, SIGNAL( messageToUserSignal( QByteArray, QByteArray ) ), m_ircControl, SLOT( messageToUserSlot( QByteArray, QByteArray ) ) );

    // connect ircUsersContainer
    connect( m_ircUsers, SIGNAL( sendUserSignal( IrcUsersContainer::WhoisStruct* ) ), this, SLOT( checkUserOnJoin( IrcUsersContainer::WhoisStruct* ) ) );
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
    return aux.value( 0 ).right( aux.value( 0 ).size() - 1 ).trimmed(); // eliminate the ":"
}

QByteArray Brain::extractUserLogin( const QByteArray& text )
{
    QList< QByteArray >aux = text.split( '@' );
    QList< QByteArray >aux2 = aux.at( 0 ).split( '!' );

    return aux2.value( 1 );
}

/*******************
*      SLOTS       *
********************/
void Brain::checkUserOnJoin( IrcUsersContainer::WhoisStruct* ircUser )
{
    if( ircUser != 0 ) {
        if( m_dbControl->isBanned( ircUser->userLogin(), ircUser->ip() ) )
            m_ircControl->loginBan( ircUser->nick(), ircUser->userLogin(), ircUser->ip() );
    }
}


void Brain::parseGameData()
{
    QByteArray nick = m_gameControl->nextUserInLine();

    while( m_gameControl->connectionSocket()->hasPendingDatagrams() ) {
        qint64 bytesToRead = m_gameControl->connectionSocket()->pendingDatagramSize();
        QByteArray serverText = m_gameControl->connectionSocket()->read( bytesToRead );

//         // use for debugging
//         qDebug() << "GAME RECIEVED DATA: " << serverText;

        QList<QByteArray> msgLines = serverText.split( '\n' );

        if( !nick.isEmpty() ) {
            for( int i = 0; i < msgLines.count(); i++ ) {
                QByteArray msg = msgLines.at( i );

                if( !msg.isEmpty() && !msg.contains( "print" ) )    // don't need annoying "print" on output
                    m_ircControl->sendLineToUser( nick, msg );
            }
        }
    }
}


//from here i dispatch irc lines with commands to corrispective classes
void Brain::parseIrcData()
{
    QMap< QString, QString > ircSettings = m_ircControl->ircSettings();

    // save all data from
    while( !m_ircControl->connectionSocket()->atEnd()/*bytesAvailable()*/ ) {
        m_ircData.append( m_ircControl->connectionSocket()->readLine( 5000 ) );

        qDebug() << "\e[1;31m " << m_ircData << "\e[0m";

        // start sending info to login and join
        if( m_ircData.contains( "NOTICE AUTH :*** Found your hostname" ) )
            m_ircControl->logIn();

        // extra login send to make sure i get in channel
        else if( m_ircData.contains( "NOTICE AUTH :*** No ident response" ) )
            m_ircControl->logIn();

        // send back ping data
        else if( m_ircData.contains( "PING" ) )
            m_ircControl->pong( m_ircData );

        // send whois for users in channel on startup
        else if( m_ircData.contains( "End of /MOTD command" ) )
            m_ircControl->channelUsersWhois();

        // get user info from a "whois"
        /// TODO find out what these mean, the + and @ are statuses, but what are the letters???
        // i noticed these letters appear when asking WHO info. Still have to find out what they are
        else if( m_ircData.contains( " H+ :3 " ) ||
                 m_ircData.contains( " H@ :3 " ) ||
                 m_ircData.contains( " Hx :3 " ) ||
                 m_ircData.contains( " H :3 " )  ||
                 m_ircData.contains( " G+ :3 " ) ||
                 m_ircData.contains( " G@ :3 " ) ||
                 m_ircData.contains( " Gx :3 " ) ||
                 m_ircData.contains( " G :3 " ) )
            m_ircControl->extractUserWhois( m_ircData );

        // on join, add user to transition and check if banned
        else if( m_ircData.contains( "JOIN "  + ircSettings.value( "chan" ).toUtf8() ) ) {
            IrcUsersContainer::WhoisStruct *ircUser = new IrcUsersContainer::WhoisStruct( extractNick( m_ircData )
                                                                                        , extractUserLogin( m_ircData )
                                                                                        , extractIp( m_ircData ) );
            m_ircControl->addToTransition( ircUser->nick(), ircUser );

            if( m_dbControl->isBanned( ircUser->userLogin(), ircUser->ip() ) )
                m_ircControl->loginBan( ircUser->nick(), ircUser->userLogin(), ircUser->ip() );  // kick - ban the user!
        }

        // after the bot has channel users, check them for ban
        else if( m_ircData.contains( "End of /NAMES list" ) )
            QTimer::singleShot( 5000, m_ircUsers, SLOT( emitUsers() ) );   // checkusers after 5 sec. Be shure to have them all

        // update user struct
        else if( m_ircData.contains( "NICK :" ) )
            m_ircControl->updateUserStruct( extractNick( m_ircData ), extractText( m_ircData ) );

        // control this after i get the "end of" line from server
        else if( m_ircData.contains( "PRIVMSG" ) ) {           // someone's talking
            QByteArray nick = extractNick( m_ircData );
            QByteArray msg = extractText( m_ircData );
            QByteArray ip = extractIp( m_ircData );

            if( msg.startsWith( '!' ) ) {                       // irc command is "!"
                qDebug() << nick << " ASKED FOR IRC BOT COMMAND with :" << msg;
                m_ircControl->ircCommandParser( nick, msg, ip );
            }

            else if( msg.startsWith( '@' ) ) {                  // game server command is "@"
                qDebug() << nick << " ASKED FOR GAME BOT COMMAND with :" << msg;
                m_gameControl->gameCommandParser( nick, msg, ip );
            }

            else                                                // nothing, normal msg
                qDebug() << nick << " SENT NORMAL MESSAGE.LOG IT SOMEWHERE!";

        }

        // clear data
        m_ircData.clear();
    }
}
