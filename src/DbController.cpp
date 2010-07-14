/***************************************************************************************
* ioQIC - Qt irc bot that also sends rcon commands to UrbanTerror game server          *
* Copyright (C) 2010, woki                                                             *
*                                                                                      *
* DbController.cpp is part of ioQIC                                                    *
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

#include "DbController.h"

#include <QFile>
#include <QSettings>
#include <QtSql>

#define settingsFile "cfg/config"
#define adminFile "cfg/admins"

DbController::DbController()
    : QSqlDatabase( QSqlDatabase::addDatabase( "QSQLITE" ) )
{
    setup();
}

DbController::~DbController()
{
    qDebug( "DbController::~DbController" );
}

bool DbController::auth( const QByteArray &nick, const QByteArray &password, const QByteArray &ip )
{
    qDebug( "DbController::auth" );

    if( !isOpen() ) { //open connection to database
        if( !open() ) {
            qWarning( "\e[1;31mDbController::auth can't open database\e[0m" );
            return false;
        }
    }

    //check to see if user is already authed
    if( isAuthed( nick, ip ) ) {
        qDebug() << nick << " IS ALREADY AUTHED!!";
        return false;
    }

    qDebug() << nick << " IS NOT AUTHED!!";

    QSqlQuery query;
    if( !query.exec( "select * "
                      "from oplist "
                      "where nick='"
                      + nick +
                      "' and password='"
                      + password +
                      "';" ) ) {
        qWarning( "\e[1;31mDbController::auth FAILED to execute query \e[0m" );
        close();
        return false;
    }

    if( !query.next() ) {   //not on auth database
        qWarning( "\e[0;33mDbController::auth %s is not on oplist\e[0m", qPrintable ( QString( nick ) ) );
        return false;
    }

    //add to authed table
    addToAuthed( nick, ip );
    close();
    return true;
}

/*******************************
*      PRIVATE FUNCTIONS       *
********************************/

void DbController::addToAuthed( const QByteArray &nick, const QByteArray &ip )
{
    qDebug( "DbController::addToAuthed" );
    if( !isOpen() ) {   //check is db is open
        if( !open() ) {
            qWarning( "\e[1;31mDbController::addToAuthed can't open database\e[0m" );
            return;
        }
    }

    QSqlQuery query;
    //Insert Into TableName (FieldName1, FieldName2) Values (Value1, Value2);
    if( !query.exec( "insert into authed( nick, ip ) values ('" + nick + "', '" + ip + "');" ) ) {
        qWarning( "\e[1;31mDbController::addToAuthed can't add user to authed table\e[0m" );
        close();
        return;
    }
    else qDebug("added");
}

void DbController::createDatabaseFirstRun()
{
    qDebug( "DbController::createDatabaseFirstRun" );
    if( !open() ) { //open connection to database
        qWarning( "\e[1;31mDbController::createDatabaseFirstRun can't open database\e[0m" );
        return;
    }

    QSqlQuery query;
    //oplist table
    if ( !query.exec( "create table oplist("
                         //"id INTEGER PRIMARY KEY,"  //auotoincrement need this??
                         "nick TEXT,"
                         "password TEXT);" ) ) {
        qWarning( "\e[1;31mDbController::createDatabaseFirstRun FAILED to execute query ( oplist table )\e[0m" );
        return;
    }

    if( !query.exec( "create table authed("
                     "nick TEXT,"
                     "ip TEXT);" ) ) {
        qWarning( "\e[1;31mDbController::createDatabaseFirstRun FAILED to execute query ( authed table )\e[0m" );
        return;
    }
    //close database
    close();
}

bool DbController::isAuthed( const QByteArray &nick, const QByteArray &ip )
{
    qDebug( "DbController::isAuthed" );
    if( !isOpen() ) {   //check is db is open
        if( !open() ) {
            qWarning( "\e[1;31mDbController::isAuthed can't open database\e[0m" );
            return false;
        }
    }

    QSqlQuery query( "select nick from authed "
                     "where nick ='" + nick + "' "
                     "and ip='" + ip + "';" );

    if( query.next() )  //found match
        //qDebug() << "HAS NEXT -> " << query.value( 0 ).toString();
        return true;
    else    //no match found
        //qDebug() << "NO NEXT -> " << query.value( 0 ).toString();
        return false;
}

void DbController::loadAdmins()
{
    qDebug( "DbController::loadAdmins" );

    QSettings settings( adminFile, QSettings::IniFormat );

    //open database for writing
    if( !open() ) {
        qWarning( "\e[1;31mDbController::loadAdmins FAILED to open database. No admins loaded\e[0m" );
        return;
    }

    int size = settings.beginReadArray( "OPLIST" );
    QString auxNick, auxPassword;

    for( int i = 0; i < size; ++i ) {
        settings.setArrayIndex( i );
        auxNick = settings.value( "nick" ).toString();
        auxPassword = settings.value( "password" ).toString();

        qDebug() << "nick -> " << auxNick << "pass -> " << auxPassword;

        QSqlQuery query;

        //check existance on database
        if( query.exec( "select nick "
                    "from oplist "
                    "where nick = '"
                    + auxNick + "';" ) ) {

#warning fix this, doesn't load admins DbController::loadAdmins();
#warning seems like this ( DbController::loadAdmins )is fixed. If there are problems, check here

            if( query.next() )
                qWarning( "\e[0;33m%s already in database\e[0m", qPrintable( auxNick ) );

            else {    //write to database
                if( !query.exec( "insert into oplist values('" + auxNick + "','" + auxPassword + "');" ) ) {
                    qWarning() << "\e[1;31mDbController::loadAdmins FAILED to execute query" << query.lastError() << "\e[0m" ;
                    return;
                }
            }
        }
        else
            qWarning() <<  "\e[1;31mDbController::loadAdmins didn't execute query " << query.lastError() << "\e[0m";
    }
    settings.endArray();
    close();
}

void DbController::setup()
{
    qDebug( "DbController::setup" );

    QString databasePath( "database" );
    databasePath.append( QDir::separator() );   //add separator at end

    //check if database folder exists
    if( !QDir().exists( databasePath ) ) {
        if( !QDir().mkdir( "database" ) ) { //create directory
            qWarning( "\e[1;31mDbController::setup can't create folder for database. Check permissions\e[0m" );
            return;
        }
    }

    //check if config file exists! ( extra control, who knows what stupid users can do ;)  )
    if( !QFile::exists( settingsFile ) ) {
        qWarning( "\e[1;31m DbController::setup FAIL , can't find settings file!\e[0m" );
        return;
    }

    //initialize settings to get dbName
    QSettings settings( QDir::toNativeSeparators( settingsFile ), QSettings::IniFormat );
    settings.beginReadArray( "DATABASE" );

    if( settings.status() == QSettings::FormatError ) {
        qWarning( "\e[1;31m DbController::setup FAILED to load settings. Format Error, check your config file\e[0m" );
        settings.endArray();
        return;
    }

    QString dbName = settings.value( "dbName" ).toString();
    setDatabaseName( databasePath + dbName );

    //check if database file exists
    if( QFile::exists( databasePath + dbName ) ) {
        qWarning( "\e[0;33mDbController::setup found database.. skipping setup\e[0m" );
        settings.endArray();
        loadAdmins();  //load and check if new admins have been added to file
        return;
    }
//    else    //set database
//        setDatabaseName( databasePath + dbName );

    //check database was set correctly
    if( databaseName().isEmpty() ) {
        qWarning( "\e[1;31mDbController::setup can't load database name setting. Check your config file!\e[0m" );
        settings.endArray();
        return;
    }

    //open database to check everything was set correctly
    if ( !open() ) {
        qWarning( "\e[1;31mDbController::setup can't create connection to SQLITE database: \"%s\"\e[0m", qPrintable( lastError().text() ) );
        return;
    }

    //i only opened database to test settings were correct. Now close
    close();

    createDatabaseFirstRun();
    loadAdmins();
}
