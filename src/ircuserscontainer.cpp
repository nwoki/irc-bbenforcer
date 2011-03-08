/****************************************************************************************
 * ioQIC-BBEnforcer - Qt irc bot that also sends rcon commands to UrbanTerror game server*
 * Copyright (C) 2010 - 2011, (n)woki                                                    *
 *                                                                                       *
 * ircuserscontainer.cpp is part of ioQIC-BBEnforcer                                     *
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

#include "ircuserscontainer.h"

#include <QDebug>

IrcUsersContainer::IrcUsersContainer()
{
}


void IrcUsersContainer::addUser( const QByteArray& nick, WhoisStruct* userStruct )
{
    m_container.insert( nick, userStruct );
}


void IrcUsersContainer::removeUser( const QByteArray& nick )
{
    m_container.remove( nick );
}


bool IrcUsersContainer::updateUserNick( const QByteArray& oldNick, const QByteArray& newNick )
{
    qDebug( "IrcUsersContainer::updateUserNick" );

    WhoisStruct *auxStruct = m_container.value( oldNick );

    if( auxStruct == 0 ) {
        qDebug() << "\e[1;31m[ERROR]IrcUsersController::updateUserNick can't find WhoisStruct for nick : " << oldNick << "\e[0m";
        return false;
    }

    WhoisStruct *newStruct = new WhoisStruct( newNick
                                            , auxStruct->userLogin()
                                            , auxStruct->ip() );
    m_container.remove( oldNick );                                  // delete old record
    addUser( newStruct->nick(), newStruct );                        // add new record
    return true;                                                    // success
}


IrcUsersContainer::WhoisStruct* IrcUsersContainer::user( const QByteArray& nick ) const
{
    return m_container.value( nick );
}


