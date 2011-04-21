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
        WhoisStruct(const QByteArray &nick, const QByteArray &userLogin, const QByteArray &ip)
        : m_nick(nick)
        , m_userLogin(userLogin)
        , m_ip(ip)
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

    /**
     * add user info to container
     * @param nick nick of user to add
     * @param userStruct respective struct with info on user to add
     */
    void addUser(const QByteArray &nick, WhoisStruct *userStruct);

    /**
     * remove user info from container
     * @param nick nick of user to search for
     */
    void removeUser(const QByteArray &nick);

    /**
     * update user nick in container
     * @param oldNick old nick to look for
     * @param newNick nick to set
     */
    bool updateUserNick(const QByteArray &oldNick, const QByteArray &newNick);

    /**
     * retrieve user information
     * @param nick user nick to look for
     */
    WhoisStruct* user(const QByteArray &nick) const;

public slots:
    /** class emits WhoIsStructs one by one */
    void emitUsers();

signals:
    void sendUserSignal(IrcUsersContainer::WhoisStruct* ircUser);

private:
    QHash<QByteArray, WhoisStruct*>m_container;

};

#endif  // IRCUSERSCONTAINER
