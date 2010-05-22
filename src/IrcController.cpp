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
    //, m_settings( 0 )
{
    qDebug("IrcController::IrcController");
    m_connection->startConnect();
}

IrcController::~IrcController()
{

}

QAbstractSocket *IrcController::connectionSocket()
{
    return m_connection->socket();
}
