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
    enum authMsg {
        ALREADY_AUTHED, /* already authed */
        AUTH_OK,        /* auth procedure went well */
        AUTH_FAIL,      /* auth procedure failed( wrong username or password ) */
        DATABASE_ERROR  /* still need to handle this one */
    };

    enum table {
        OPLIST,
        AUTHED,
        BANNED
    };

    DbController();
    ~DbController();

    void addToTransition( const QByteArray &nick, const QByteArray &userLogin, const QByteArray &ip );  /** adds user to transition database */
    authMsg auth( const QByteArray &nick, const QByteArray &password, const QByteArray &ip );           /** auth's client to the bot giving admin priviledges */
    bool isAuthed( const QByteArray &nick, const QByteArray &ip );                                      /** checks if client is authed */
    bool isBanned( const QByteArray &userLogin, const QByteArray &ip );                                 /** checks if client is banned by irc bot */

private:
    bool addToAuthed( const QByteArray &nick, const QByteArray &ip );   // add client to auth
    void createDatabaseFirstRun();                                      // creates authed and oplist tables
    bool openDb();                                                      /** opens a connection to the database if there is none. Returns the status of the operation */
    void loadAdmins();                                                  // loads admins to oplist table
    int genNewId( table t );                                            // creates new id for insertion in oplist table ( DB MUST BE ALREADY OPEN! )
    void setup();                                                       // setup the database
};

#endif // DBCONTROLLER_H
