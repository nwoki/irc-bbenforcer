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

#include <QAbstractSocket>

IrcController::IrcController()
    : m_connection( new Connection( QAbstractSocket::TcpSocket ) )  //get these on constructor
{
    qDebug( "IrcController::IrcController" );
    m_connection->startConnect();
}

IrcController::~IrcController()
{

}

QAbstractSocket *IrcController::connectionSocket()
{
    return m_connection->socket();
}

void IrcController::logIn()
{
    //get settings
    QMap< QString, QString > data = m_connection->ircSettings();
    QString end( "\r\n" ), nick = data.value( "nick" );

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

void IrcController::ircCommendParser( const QByteArray &user, const QByteArray &msg )
{

}

void IrcController::pong( const QByteArray &pingData )
{
    QList<QByteArray>pingSplit = pingData.split( ':' );
    connectionSocket()->write( "PONG :" + pingSplit.at( 1 ) + "\r\n");
}
