/***************************************************************************************
* ioQIC - Qt irc bot that also sends rcon commands to UrbanTerror game server          *
* Copyright (C) 2010, woki                                                             *
*                                                                                      *
* Brain.cpp is part of ioQIC                                                           *
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

#include "Brain.h"
#include "DbController.h"
#include "GameController.h"
#include "IrcController.h"

#include <QAbstractSocket>
#include <QDebug>

Brain::Brain()
    : m_dbControl( 0 )
    , m_gameControl( 0 )
    , m_ircControl( 0 )
{
    qDebug("Brain::Brain");
    m_ircControl = new IrcController();

    //read from server when data is available
    connect( m_ircControl->connectionSocket(), SIGNAL( readyRead() ), this, SLOT( parseData() ) );
    //connect various object attributes
}

Brain::~Brain()
{
    delete m_dbControl;
    delete m_gameControl;
    delete m_ircControl;
}

QByteArray Brain::extractText( const QByteArray &text )
{
    QList< QByteArray >aux = text.split( ':' );
    //qDebug() << "TEXT2 -> " << aux.value( 2 ).trimmed();
    return aux.value( 2 ).trimmed();
}

QByteArray Brain::extractUser( const QByteArray &text )
{
    QList< QByteArray >aux = text.split( '!' );
    //qDebug() << "USER -> " << aux.value( 0 ).right( aux.value( 0 ).size() - 1 );
    return aux.value( 0 ).right( aux.value( 0 ).size() - 1 ).trimmed();
}

/*******************
*      SLOTS       *
********************/

//from here i dispatch irc lines with commands to corrispective classes
void Brain::parseData()
{
    while( m_ircControl->connectionSocket()->bytesAvailable() ) {
        QByteArray serverText = m_ircControl->connectionSocket()->readLine( 2000 );
        qDebug() << serverText;

        //start sending info to login and join
        if( serverText.contains( "NOTICE AUTH :*** Found your hostname" ) ) {
            m_ircControl->logIn();
            return;
        }

        //extra login send to make sure i get in channel
        else if( serverText.contains( "NOTICE AUTH :*** No ident response" ) ) {
            m_ircControl->logIn();
            return;
        }

        //send back ping data
        else if( serverText.contains( "PING" ) ) {
            m_ircControl->pong( serverText );
            return;
        }

        //control this after i get the "end of" line from server
        else if( serverText.contains( "PRIVMSG" ) ) {
            //someone's talking
            QByteArray user = extractUser( serverText );
            QByteArray msg = extractText( serverText );

            //irc command
            if( msg.startsWith( '!' ) ) {
                qDebug() << user << " ASKED FOR IRC BOT COMMAND with :" << msg;
                m_ircControl->ircCommendParser( user, msg );
            }

            //game server command
            else if( msg.startsWith( '@' ) ) {
                qDebug() << user << " ASKED FOR GAME BOT COMMAND with :" << msg;
            }

            //nothing, normal msg
            else {
                qDebug() << user << " SENT NORMAL MESSAGE.LOG IT!";
            }

            //check on database. User needs to login whenever he has new ip to use bot
            //bot checks database and adds the admin
        }
    }
}
