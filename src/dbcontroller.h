/*****************************************************************************************
 * ioQIC-BBEnforcer - Qt irc bot that also sends rcon commands to UrbanTerror game server*
 * Copyright (C) 2010 - 2011, (n)woki                                                    *
 *                                                                                       *
 * dbcontroller.h is part of ioQIC-BBEnforcer                                            *
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

#ifndef DBCONTROLLER_H
#define DBCONTROLLER_H

#include <QSqlDatabase>

class DbController : public QSqlDatabase
{
public:
    enum opMsg {
        ALREADY_OPPED,      /* value is already present in database */
        OP_OK,              /* op procedure went well */
        OP_DATABASE_ERROR   /* database error ( still need to handle? ) */
    };

    enum authMsg {
        ALREADY_AUTHED, /* already authed */
        AUTH_OK,        /* auth procedure went well */
        AUTH_FAIL,      /* auth procedure failed( wrong username or password ) */
        AUTH_DATABASE_ERROR  /* still need to handle this one */
    };

    enum table {
        OPLIST,
        AUTHED,
        BANNED
    };

    /**
     * this struct is used to rappresent a user on an irc channel
     */
    /// TODO eliminate
    struct IrcUser {
        QByteArray nick;
        QByteArray userLogin;
        QByteArray ip;

        /**
         * tells me if info is valid. All fields should contain data!
         */
        bool isValid()
        {
            if( nick.isEmpty() || userLogin.isEmpty() || ip.isEmpty() )
                return false;
            else return true;
        }
    };

    DbController();
    ~DbController();

    /**
     * add banned user to database
     * @param nick nick used by client when banned
     * @param login of client to ban
     * @param ip ip of banned client
     * @param author who did the ban
     * @param date when the client was banned
     */
    void addToBanned( const QByteArray &nick, const QByteArray &login, const QByteArray &ip, const QByteArray &author, const QString &date );


    /**
     * add user to oplist table
     * @param userLogin new admin login
     * @param password new admin pass
     */
    opMsg addToOplist( const QByteArray &userLogin, const QByteArray &password );


    /** auth's client to the bot giving admin priviledges
     * @param user username to auth
     * @param password password of user to auth
     * @param ip of user to auth
     */
    authMsg auth( const QByteArray &user, const QByteArray &password, const QByteArray &ip );


    /**
     * retrieve info about a user from transition database
     * @param userNick user nick to search database for
     */
    IrcUser getIrcUser( const QByteArray &userNick );


    /**
     * checks if client is authed
     * @param nick nick of user that requests the authentication
     * @param ip ip of the user that requests the authentication
     */
    bool isAuthed( const QByteArray &user, const QByteArray &ip );


    /**
     * checks if client is banned by irc bot
     * @param userLogin user
     * @param user ip
     */
    bool isBanned( const QByteArray &userLogin, const QByteArray &ip );


    /**
     * removes users from banned table
     * @param user user to remove from banned table
     */
    bool removeFromBanned( const QByteArray &user );


    /**
     * removes user from oplist and authed table in case the user was authed
     * when this command was given
     * @param user user to deop
     */
    bool removeFromOplist( const QByteArray &user );


private:
    bool addToAuthed( const QByteArray &user, const QByteArray &ip );   /** add client to auth */
    QByteArray cleanUserName( const QByteArray &dirtyUserName );        /** returns clean username without the "~" */
    void createDatabaseFirstRun();                                      /** creates authed and oplist tables */
    bool openDb();                                                      /** opens a connection to the database if there is none. Returns the status of the operation */
    void loadAdmins();                                                  /** loads admins to oplist table */
    int genNewId( table t );                                            /** creates new id for insertion in oplist table ( DB MUST BE ALREADY OPEN! ) */
    void setup();                                                       /** setup the database */
};

#endif // DBCONTROLLER_H
