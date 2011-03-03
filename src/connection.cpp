/***************************************************************************************
* ioQIC - Qt irc bot that also sends rcon commands to UrbanTerror game server          *
* Copyright (C) 2010, woki                                                             *
*                                                                                      *
* Connection.cpp is part of ioQIC                                                      *
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

#include "connection.h"

#include <iostream>
#include <QAbstractSocket>
#include <QDir>
#include <QSettings>
#include <QTcpSocket>
#include <QTimer>
#include <QUdpSocket>

Connection::Connection( QAbstractSocket::SocketType type )
    : QAbstractSocket( type, 0 )
    , m_port( 0 )
    , m_chan( QString() )
    , m_ip( QString() )
    , m_nick( QString() )
    , m_rconPass( QString() )
{
    loadSettings();
}

Connection::~Connection()
{
}

QMap< QString, QString > Connection::ircSettings()
{
    QMap< QString, QString >  aux;
    aux.insert( "ip", m_ip );
    aux.insert( "port", QString::number( m_port ) );
    aux.insert( "nick", m_nick );
    aux.insert( "chan", m_chan );
    return aux;
}

void Connection::loadSettings()
{
    //set config file
    QSettings settings( QDir::toNativeSeparators( "cfg/config" ), QSettings::IniFormat );

    if( settings.status() == QSettings::FormatError ) {
        qWarning( "\e[1;31m Connection::loadSettings FAILED to load settings. Format Error, check your config file\e[0m" );
        return;
    }

    /*
     * IRC LOADER
     */
    if( socketType() == QAbstractSocket::TcpSocket ) {    //load irc settings
        qDebug( "Connection::loadSettings IRC SETTINGS" );

        settings.beginReadArray( "IRC" );

        m_ip = settings.value( "ip" ).toString();
        if( m_ip.isEmpty() ) {
            qWarning( "\e[1;31mConnection::loadSettings can't load 'ip'. Check your config file\e[0m" );
            return;
        }

        bool ok;
        m_port = settings.value( "port" ).toInt( &ok );
        if( !ok ) {
            qWarning( "\e[1;31mConnection::loadSettings can't load 'port'. Check your config file.\e[0m" );
            return;
        }

        m_chan = settings.value( "chan" ).toString();
        if( m_chan.isEmpty() ) {
            qWarning( "\e[1;31mConnection::loadSettings can't load 'chan'. Check your config file\e[0m" );
            return;
        }

        m_nick = settings.value( "nick" ).toString();
        if( m_nick.isEmpty() ) {
            qWarning( "\e[1;31mConnection::loadSettings can't load 'nick'. Check your config file\e[0m" );
            return;
        }
        qDebug() << ircSettings();
    }

    /*
     * GAMESERVER LOADER
     */
    else if( socketType() == QAbstractSocket::UdpSocket ) {   //load game settings
        qDebug( "Connection::loadSettings GAME SERVER SETTINGS" );

        settings.beginReadArray( "GAMESERVER" );

        m_ip = settings.value( "ip" ).toString();
        if( m_ip.isEmpty() ) {
            qWarning( "\e[1;31mConnection::loadSettings can't load 'ip'. Check your config file\e[0m" );
            return;
        }

        bool ok;
        m_port = settings.value( "port" ).toInt( &ok );
        if( !ok ) {
            qWarning( "\e[1;31mConnection::loadSettings can't load 'port'. Check your config file.\e[0m" );
            return;
        }

        m_rconPass = settings.value( "rconPass" ).toString();

        if( m_rconPass.isEmpty() )
            qWarning( "\e[1;31mWARNING: empty rcon pass!" );

    }
    else {
        qWarning( "\e[1;31mConnection::loadSettings Unknown socket type, not loading settings\e[0m" );
        return;
    }
    //close settings
    settings.endArray();
}

void Connection::startIrcConnect()
{
    qDebug( "Connection::startIrcConnect");
//     if( !m_socket ) {
//         qWarning( "Connection::connect no socket initialized for connection" );
//         return;
//     }


    //connection notification
    connect( this, SIGNAL( connected() ), this, SLOT( connectNotify() ) );

    //connection error
    connect( this, SIGNAL( error( QAbstractSocket::SocketError ) ), this, SLOT( handleSocketErrors( QAbstractSocket::SocketError ) ) );

    //disconnect notification
    connect( this, SIGNAL( disconnected() ), this, SLOT( disconnectNotify()) );

    connectToHost( m_ip, m_port, QIODevice::ReadWrite );
}

void Connection::startGameConnect()
{
    qDebug( "Connection::startGameConnect" );
//     if( !m_socket ) {
//         qWarning( "Connection::connect no socket initialized for game connection" );
//         return;
//     }

}


/*******************
*      SLOTS       *
********************/

void Connection::connectNotify()
{
    //std::cout << "Connected to host " << m_ip << ":" << m_port << std::endl;
    qDebug() << "Connected to host " << m_ip << ":" << m_port;
}

void Connection::disconnectNotify()
{
    qDebug( "Connection::disconnectNotify" );
}

void Connection::handleSocketErrors( QAbstractSocket::SocketError error )
{
    qDebug("AAAAAAAAAA");
    disconnectFromHost(); // or "abort()"?

    switch ( error ) {
        case QAbstractSocket::ConnectionRefusedError: {
            qWarning() << "\e[0;33m" << errorString() << ", trying to reconnect..\e[0m";
            QTimer::singleShot( 3000, this, SLOT( reconnect() ) );
            //what to do?
            break;
        }
        case QAbstractSocket::RemoteHostClosedError: {
            qWarning() << "\e[0;33m" << errorString() << ", trying to reconnect..\e[0m";
            reconnect();
            break;
        }
        case QAbstractSocket::HostNotFoundError: {
            qWarning() << "\e[0;33m" << errorString() << " Please control your config file and check that all values have been inserted correctly\e[0m";
            exit( -1 ); //terminate program
            break;
        }
        case QAbstractSocket::SocketAccessError: {
            qWarning() << "\e[0;33" << errorString() << " The application lacks the required privileges\e[0m";
            exit( -1 );
            break;
        }
        case QAbstractSocket::SocketTimeoutError: {
            qWarning() << "\e[0;33m" << errorString() << ", trying to reconnect....\e[0m" ;
            reconnect();
#warning FIX ME crashes when i get this error and try to reconnect.
            break;
        }
        case QAbstractSocket::DatagramTooLargeError: {
            qWarning() << "\e[0;33m" << errorString() << "\e[0m";
            break;
        }
        default: {
            qWarning() << "\e[0;33m Following error is not handled -> " << errorString();
            break;
        }

    }
}

void Connection::reconnect()
{
    //delete and recreate same socket
//     QAbstractSocket *aux = m_socket;
//     m_socket = 0;
//
//     if( aux->socketType() == QAbstractSocket::TcpSocket )
//         m_socket = new QTcpSocket();
//     else if( aux->socketType() == QAbstractSocket::UdpSocket )
//         m_socket = new QUdpSocket();
//
//     delete aux;
    qDebug( "Connection::reconnect" );
    qDebug() << "Socket state is : " << state();
    startIrcConnect();
}
