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
    //: m_dbControl( 0 )
    : m_gameControl( 0 )
    , m_ircControl( 0 )
{
    qDebug("Brain::Brain");
    //m_dbControl = new DbController(); i don't need it on brain
    //m_gameControl TODO
    //create pointer to pass to irc Controller and GameController
    DbController *db = new DbController();

    m_ircControl = new IrcController( db );


    //read from server when data is available
    connect( m_ircControl->connectionSocket(), SIGNAL( readyRead() ), this, SLOT( parseData() ) );
    //connect various object attributes

    //delete local pointer to db, don't need it anymore
    db = 0;
    delete db;
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

QByteArray Brain::extractUser( const QByteArray &text )
{
    QList< QByteArray >aux = text.split( '!' );
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

        if( serverText.contains( "NOTICE AUTH :*** Found your hostname" ) ) {   /* start sending info to login and join */
            m_ircControl->logIn();
            return;
        }

        else if( serverText.contains( "NOTICE AUTH :*** No ident response" ) ) {    /* extra login send to make sure i get in channel */
            m_ircControl->logIn();
            return;
        }

        else if( serverText.contains( "PING" ) ) {  /* send back ping data */
            m_ircControl->pong( serverText );
            return;
        }

        // control this after i get the "end of" line from server
        else if( serverText.contains( "PRIVMSG" ) ) {   /* someone's talking */
            QByteArray user = extractUser( serverText );
            QByteArray msg = extractText( serverText );
            QByteArray ip = extractIp( serverText );

            if( msg.startsWith( '!' ) ) {   /* irc command is "!" */
                qDebug() << user << " ASKED FOR IRC BOT COMMAND with :" << msg;
                m_ircControl->ircCommandParser( user, msg, ip );
            }

            else if( msg.startsWith( '@' ) ) {  /* game server command is "@" */
                qDebug() << user << " ASKED FOR GAME BOT COMMAND with :" << msg;
                qDebug( "\e[1;31mBrain::parseData game server command -> need to implement! \e[0m" );
            }

            else    /* nothing, normal msg */
                qDebug() << user << " SENT NORMAL MESSAGE.LOG IT!";
        }
    }
}
