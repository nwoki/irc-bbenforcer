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
        DbController();
        ~DbController();

        bool auth( const QByteArray &nick, const QByteArray &password, const QByteArray &ip );    //check database and return result for auth

    private:
        void addToAuthed( const QByteArray &nick, const QByteArray &ip );
        void createDatabaseFirstRun();  //creates authed and oplist tables
        bool isAuthed( const QByteArray &nick, const QByteArray &ip );
        void loadAdmins();  //loads admins to oplist table
        void setup();   //setup the database
//        void populateDatabase();
};

#endif // DBCONTROLLER_H
