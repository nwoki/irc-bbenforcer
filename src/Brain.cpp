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
#include <QSettings>
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


    //QObject::connect( m_ircControl->connectionSocket(), SIGNAL(), this, SLOT( )
    //load settings
    //connect various object attributes

}

Brain::~Brain()
{
    delete m_dbControl;
    delete m_gameControl;
    delete m_ircControl;
}

void Brain::parseData()
{
    qDebug("Brain::parseData");

    while( m_ircControl->connectionSocket()->bytesAvailable() )
        qDebug() << m_ircControl->connectionSocket()->readLine(200);
}
