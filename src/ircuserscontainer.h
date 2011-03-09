/****************************************************************************************
 * ioQIC-BBEnforcer - Qt irc bot that also sends rcon commands to UrbanTerror game server*
 * Copyright (C) 2010 - 2011, (n)woki                                                    *
 *                                                                                       *
 * ircuserscontainer.h is part of ioQIC-BBEnforcer                                       *
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

#ifndef IRCUSERSCONTAINER
#define IRCUSERSCONTAINER

#include <QByteArray>
#include <QHash>

class IrcUsersContainer : public QObject
{
    Q_OBJECT

public:
    class WhoisStruct
    {
    public:
        WhoisStruct( const QByteArray &nick, const QByteArray &userLogin, const QByteArray &ip )
        : m_nick( nick )
        , m_userLogin( userLogin )
        , m_ip( ip )
        {}

        const QByteArray nick() { return m_nick; }
        const QByteArray userLogin() { return m_userLogin; }
        const QByteArray ip() { return m_ip; }

    private:
        QByteArray m_nick
        , m_userLogin
        , m_ip;
    };

    IrcUsersContainer();

    void addUser( const QByteArray &nick, WhoisStruct *userStruct );
    void removeUser( const QByteArray &nick );
    bool updateUserNick( const QByteArray &oldNick, const QByteArray &newNick );
    WhoisStruct* user( const QByteArray &nick ) const;

public slots:
    void emitUsers();       /** class emits WhoIsStructs one by one */

signals:
    void sendUserSignal( IrcUsersContainer::WhoisStruct* ircUser );

private:
    QHash< QByteArray, WhoisStruct* >m_container;

};

#endif  // IRCUSERSCONTAINER
