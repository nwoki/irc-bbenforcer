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

#include "Connection.h"

#include <iostream>
#include <QAbstractSocket>
#include <QDir>
#include <QSettings>
#include <QTcpSocket>
#include <QTimer>
#include <QUdpSocket>

Connection::Connection( QAbstractSocket::SocketType type )
    : m_port( 0 )
    , m_socket( 0 )
    , m_chan( QString() )
    , m_ip( QString() )
    , m_nick( QString() )
{
    if( type == QAbstractSocket::TcpSocket )
        m_socket = new QTcpSocket();
    else if( type == QAbstractSocket::UdpSocket )
        m_socket = new QUdpSocket();

    loadSettings();
}

Connection::~Connection()
{
    delete m_socket;
}

void Connection::loadSettings()
{
    //set config file
    QSettings settings( QDir::toNativeSeparators( "cfg/config" ), QSettings::IniFormat );

    if( settings.status() == QSettings::FormatError ) {
        qWarning( "\e[1;31m Connection::loadSettings FAILED to load settings. Format Error, check your config file\e[0m" );
        return;
    }

    if( m_socket->socketType() == QAbstractSocket::TcpSocket ) {    //load irc settings
        qDebug( "Connection::loadSettings IRC SETTINGS" );

        /*int values = */settings.beginReadArray( "IRC" );
        /* TODO check validity of settings, shouldn't pass empty settings */
//        if(  ) {
//            qWarning( "\e[1;31m Connection::loadSettings FAIL, no values to read for IRC settings. Check your config file\e[0m" );
//            return;
//        }
//        qDebug() << "VALUES #" << values;

        m_ip = settings.value( "ip" ).toString();
        m_port = settings.value( "port" ).toInt();
        m_chan = settings.value( "chan" ).toString();
        m_nick = settings.value( "nick" ).toString();

//        qDebug() << "values saved are-> " << m_ip << " " << m_port << " " << m_chan << " " << m_nick;
    }
    else if( m_socket->socketType() == QAbstractSocket::UdpSocket ) {   //load game settings

    }
    else {
        qWarning( "Connection::loadSettings Unknown socket type, not loading settings" );
        return;
    }
    //close settings
    settings.endArray();
}

void Connection::startConnect()
{
    qDebug( "Connection::startConnect");
    if( !m_socket ) {
        qWarning( "Connection::connect no socket initialized for connection" );
        return;
    }

    //connection notification
    connect( m_socket, SIGNAL( connected() ), this, SLOT( connectNotify() ) );

    //connection error
    connect( m_socket, SIGNAL( error( QAbstractSocket::SocketError ) ), this, SLOT( handleSocketErrors( QAbstractSocket::SocketError ) ) );

    //disconnect notification
    connect( m_socket, SIGNAL( disconnected() ), this, SLOT( disconnectNotify()) );

    m_socket->connectToHost( m_ip, m_port, QIODevice::ReadWrite );
}

QAbstractSocket *Connection::socket()
{
    if( !m_socket ) {
        qWarning( "Connection::socket no socket to return!" );//(created empty one) " );
        return 0;
//        if( m_type == "tcp" )
//           return new QTcpSocket();
//        else if( m_type == "udp" )
//            return new QUdpSocket();
    }
    else
        return m_socket;
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
    m_socket->disconnectFromHost(); // or "abort()"?

    switch ( error ) {
        case QAbstractSocket::ConnectionRefusedError: {
                qWarning() << "\e[0;33m" << m_socket->errorString() << ", trying to reconnect..\e[0m";
            QTimer::singleShot( 3000, this, SLOT( reconnect() ) );
            //what to do?
            break;
        }
        case QAbstractSocket::RemoteHostClosedError: {
            qWarning() << "\e[0;33m" << m_socket->errorString() << ", trying to reconnect..\e[0m";
            reconnect();
            break;
        }
        case QAbstractSocket::HostNotFoundError: {
            qWarning() << "\e[0;33m" << m_socket->errorString() << " Please control your config file and check that all values have been inserted correctly\e[0m";
            exit( -1 ); //terminate program
            break;
        }
        case QAbstractSocket::SocketAccessError: {
            qWarning() << "\e[0;33" << m_socket->errorString() << " The application lacks the required privileges\e[0m";
            exit( -1 );
            break;
        }
        case QAbstractSocket::SocketTimeoutError: {
            qWarning() << "\e[0;33m" << m_socket->errorString() << ", trying to reconnect....\e[0m" ;
            reconnect();
#warning FIX ME crashes when i get this error and try to reconnect.
            break;
        }
        case QAbstractSocket::DatagramTooLargeError: {
            qWarning() << "\e[0;33m" << m_socket->errorString() << "\e[0m";
            break;
        }
        default: {
            qWarning() << "\e[0;33m Following error is not handled -> " << m_socket->errorString();
            break;
        }

    }
}

void Connection::reconnect()
{
    //delete and recreate socket
    QAbstractSocket *aux = m_socket;
    m_socket = 0;

    if( aux->socketType() == QAbstractSocket::TcpSocket )
        m_socket = new QTcpSocket();
    else if( aux->socketType() == QAbstractSocket::UdpSocket )
        m_socket = new QUdpSocket();

    delete aux;
    qDebug( "Connection::reconnect" );
    startConnect();
}
