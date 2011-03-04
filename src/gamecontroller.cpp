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

#include "connection.h"
#include "dbcontroller.h"
#include "gamecontroller.h"

#include <QAbstractSocket>

#define RCON_START "\xff\xff\xff\xffrcon"


GameController::GameController( DbController *db )
    : m_db( db )
    , m_socket( new Connection( QAbstractSocket::UdpSocket ) )
{

}

QAbstractSocket *GameController::connectionSocket() const
{
    return m_socket;
}
