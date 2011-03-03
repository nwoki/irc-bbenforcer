/***************************************************************************************
* ioQIC - Qt irc bot that also sends rcon commands to UrbanTerror game server          *
* Copyright (C) 2010, woki                                                             *
*                                                                                      *
* DbController.h is part of ioQIC                                                      *
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

#ifndef DBCONTROLLER_H
#define DBCONTROLLER_H

#include <QSqlDatabase>

class QSettings;

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
        AUTHED
    };

    DbController();
    ~DbController();

    authMsg auth( const QByteArray &nick, const QByteArray &password, const QByteArray &ip );  /*!< auth's client to the bot giving admin priviledges */
    bool isAuthed( const QByteArray &nick, const QByteArray &ip );

private:
    bool addToAuthed( const QByteArray &nick, const QByteArray &ip );   // add client to auth
    void createDatabaseFirstRun();                                      // creates authed and oplist tables
    void loadAdmins();                                                  // loads admins to oplist table
    int genNewId( table t );                                            // creates new id for insertion in oplist table ( DB MUST BE ALREADY OPEN! )
    void setup();                                                       // setup the database
//    void populateDatabase();
};

#endif // DBCONTROLLER_H
