/*****************************************************************************************
 * ioQIC-BBEnforcer - Qt irc bot that also sends rcon commands to UrbanTerror game server*
 * Copyright (C) 2010 - 2011, (n)woki                                                    *
 *                                                                                       *
 * dbcontroller.cpp is part of ioQIC-BBEnforcer                                          *
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

#include "dbcontroller.h"

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


void DbController::addToBanned( const QByteArray& nick, const QByteArray& login, const QByteArray& ip, const QByteArray& author, const QString& date )
{
    if( !openDb() )
        return;

    QSqlQuery query;

    if( !query.exec( "insert into banned( nick, login, ip, author, date ) values('" + nick + "','" + login + "','" + ip + "','" + author + "','" + date + "');" ) ) {
        qWarning() << "\e[1;31m[FAIL]DbController::addToBanned failed to execute query" << query.lastError() << "\e[0m" ;
        return;
    }
}


DbController::opMsg DbController::addToOplist( const QByteArray& userLogin, const QByteArray& password )
{
    // check user doesn't already exist
    QByteArray userToAdd = cleanUserName( userLogin );
    QSqlQuery query;

    if( !query.exec( "select id from oplist where user = '" + userToAdd + "';" ) ) {
        qWarning() << "\e[1;31m[FAIL]DbController::addToOplist failed to execute query" << query.lastError() << "\e[0m" ;
        return OP_DATABASE_ERROR;
    }

    if( query.next() )          // already exists
        return ALREADY_OPPED;
    else {                      // add user
        qDebug() << "adding new user to oplist: " << userToAdd;
        if( !query.exec( "insert into oplist( user, password ) values('" + userToAdd + "','" + password + "');" ) ) {
            qWarning() << "\e[1;31m[FAIL]DbController::addToOplist failed to execute query" << query.lastError() << "\e[0m" ;
            return OP_DATABASE_ERROR;
        }
        return OP_OK;
    }
}


DbController::authMsg DbController::auth( const QByteArray &user, const QByteArray &password, const QByteArray &ip )
{
    // need to handle enum DATABASE_ERROR
    qDebug( "DbController::auth" );

    if( !openDb() )
        return AUTH_DATABASE_ERROR;

    QByteArray userCp = cleanUserName( user );

    // check to see if user is already authed
    if( isAuthed( userCp, ip ) ) {
        qDebug() << userCp << " IS ALREADY AUTHED!!";
        return ALREADY_AUTHED;          // don't need to auth again
    }

    qDebug() << userCp << " IS NOT AUTHED!!";

    QSqlQuery query;

    if( !query.exec( "select id from oplist where user='" + userCp + "' and password='" + password + "';" ) ) {     // query failed
        qWarning( "\e[1;31mDbController::auth FAILED to execute query \e[0m" );
        close();
        return AUTH_DATABASE_ERROR;
    }

    if( !query.next() ) {               // not on auth database
        qWarning( "\e[0;33mDbController::auth %s is not on oplist or wrong password \e[0m", qPrintable ( QString( userCp ) ) );
        return AUTH_FAIL;
    }

    if( addToAuthed( userCp, ip ) ) {     // add to authed table
        close();
        return AUTH_OK;
    }
    else{
        close();
        return AUTH_DATABASE_ERROR;
    }
}


/// TODO eliminate
DbController::IrcUser DbController::getIrcUser( const QByteArray& userNick )
{
    qDebug() << "DbController::getIrcUser looking for: " << userNick;
    IrcUser user;
    QSqlQuery query;

    if( !query.exec( "select login, ip from transition where nick='" + userNick + "';" ) ) {
        qWarning() << "\e[1;31m[FAIL]DbController::getIrcUser FAILED to execute query" << query.lastError() << "\e[0m";
        return user;
    }

    if( !query.next() ) {
        qWarning() << "\e[1;31m[FAIL]DbController::getIrcUser user: " << userNick << " is not on database \e[0m";
        return user;
    }

    user.nick = userNick;
    user.userLogin = query.value( 0 ).toByteArray();
    user.ip = query.value( 1 ).toByteArray();

    return user;
}


bool DbController::isAuthed( const QByteArray &user, const QByteArray &ip )
{
    qDebug( "DbController::isAuthed" );
    if( !openDb() )
        return false;

    QByteArray userCp = cleanUserName( user );
    QSqlQuery query;

    if( !query.exec( "select id from authed where user ='" + userCp + "' and ip='" + ip + "';" ) ) {
        qWarning() << "\e[1;31mDb[FAIL]DbController::isAuthed FAILED to execute query" << query.lastError() << "\e[0m" ;
        return false;
    }

    if( query.next() )  // found value
        return true;
    else                // no value found
        return false;
}


bool DbController::isBanned(const QByteArray& userLogin, const QByteArray& ip)
{
    qDebug("DbController::isBanned");
    if (!openDb()) {
        qWarning("\e[1;31m[FAIL]DbController::isBanned FAILED to open database. Can't verify if user is banned or not \e[0m");
        return false;
    }

    QSqlQuery query;

    // look for user login ban
    if (!query.exec("select id from banned where login='" + userLogin + "';")) {
        qWarning() << "\e[1;31mDb[FAIL]DbController::isBanned FAILED to execute query" << query.lastError() << "\e[0m" ;
        return false;
    }

    if (query.next()) {
        qDebug("user LOGIN is banned");
        return true;
    } else {
        qDebug("user is NOT banned");

        // look for ip ban
        if (!query.exec("select id from banned where ip='" + ip + "';")) {
            qWarning() << "\e[1;31mDb[FAIL]DbController::isBanned FAILED to execute query" << query.lastError() << "\e[0m" ;
            return false;
        }

        if (query.next()) {
            qDebug("user IP is banned");
            return true;
        } else {
            return false;
        }
    }
}


bool DbController::removeFromBanned( const QByteArray& user )
{
    if( !openDb() ) {
        qWarning( "\e[1;31m[FAIL]DbController::removeFromBanned FAILED to open database. Can't verify if user is banned or not \e[0m" );
        return false;
    }

    QSqlQuery query;
    if( !query.exec( "select id from banned where login='" + user + "';" ) ) {
        qWarning() << "\e[1;31mDb[FAIL]DbController::removeFromBanned FAILED to execute query" << query.lastError() << "\e[0m" ;
        return false;
    }

    if( query.next() ) {    // user is on Db
        if( !query.exec( "delete from banned where login='" + user + "';" ) ) {
            qWarning() << "\e[1;31mDb[FAIL]DbController::removeFromBanned FAILED to execute query" << query.lastError() << "\e[0m" ;
            return false;
        }
        return true;
    }
    return false;
}


bool DbController::removeFromOplist( const QByteArray& user )
{
    qDebug( "DbController::removeFromOplist" );

    // clean username
    QByteArray userCp = cleanUserName( user );

    if( !openDb() ) {
        qWarning( "\e[1;31m[FAIL]DbController::removeFromOplist FAILED to open database. Can't verify if user is banned or not \e[0m" );
        return false;
    }

    QSqlQuery query;
    if( !query.exec( "select id from oplist where user='" + userCp + "';" ) ) {
        qWarning() << "\e[1;31mDb[FAIL]DbController::removeFromOplist FAILED to execute query" << query.lastError() << "\e[0m" ;
        return false;
    }

    if( query.next() ) {    // found user
        if( !query.exec( "delete from oplist where user='" + userCp + "';" ) ) {
            qWarning() << "\e[1;31mDb[FAIL]DbController::removeFromOplist FAILED to execute query" << query.lastError() << "\e[0m" ;
            return false;
        }

        // delete from authed
        if( !query.exec( "delete from authed where user='" + userCp + "';" ) ) {
            qWarning() << "\e[1;31mDb[FAIL]DbController::removeFromOplist FAILED to execute query" << query.lastError() << "\e[0m" ;
            return false;
        }

        return true;
    }
    return false;
}


/*******************************
*      PRIVATE FUNCTIONS       *
********************************/

bool DbController::addToAuthed( const QByteArray &user, const QByteArray &ip )
{
    qDebug( "DbController::addToAuthed" );
    if( !openDb() )
        return false;       // error with DB

    QSqlQuery query;

    if( !query.exec( "insert into authed( user, ip ) values ('" + user + "', '" + ip + "');" ) ) {
        qWarning( "\e[1;31mDbController::addToAuthed can't add user to authed table\e[0m" );
        close();
        return false;
    }
    else {
        qDebug( "added" );
        return true;
    }
}


QByteArray DbController::cleanUserName( const QByteArray& dirtyUserName )
{
    QByteArray aux = dirtyUserName;
    if( aux.at( 0 ) == '~' )    // strip useless chars from user ( ex: the "~" sign that sometimes is add by irc server )
        aux = dirtyUserName.right( dirtyUserName.length() - 1 );
    return aux;
}


void DbController::createDatabaseFirstRun()
{
    qDebug( "DbController::createDatabaseFirstRun" );
    if( !open() ) { //open connection to database
        qWarning( "\e[1;31mDbController::createDatabaseFirstRun can't open database\e[0m" );
        return;
    }

    QSqlQuery query;
    // oplist table
    /// TODO use "userLogin" instead of nick
    if ( !query.exec( "create table oplist("
                      "id INTEGER PRIMARY KEY,"  // auotoincrement PK
                      "user TEXT,"
                      "password TEXT);" ) ) {
        qWarning( "\e[1;31mDbController::createDatabaseFirstRun FAILED to execute query ( oplist table )\e[0m" );
        return;
    }

    // authed table
    /// TODO use "userLogin" instead of nick
    if( !query.exec( "create table authed("
                     "id INTEGER PRIMARY KEY,"  // autoincrement PK
                     "user TEXT,"
                     "ip TEXT);" ) ) {
        qWarning( "\e[1;31mDbController::createDatabaseFirstRun FAILED to execute query ( authed table )\e[0m" );
        return;
    }

    // banned table
    if( !query.exec( "create table banned("
                     "id INTEGER PRIMARY KEY,"  // autoincrement PK
                     "nick TEXT,"               // banned nick
                     "login TEXT,"              // banned user login
                     "ip TEXT,"                 // banned ip
                     "author TEXT,"             // admin who banned the user
                     "date TEXT);" ) ) {        // date the user was banned
        qWarning( "\e[1;31mDbController::createDatabaseFirstRun FAILED to execute query ( banned table )\e[0m" );
        return;
    }

    // close database
    close();
}


bool DbController::openDb()
{
    bool dbStatusOpen = true;

    if( !isOpen() ) {   //check if db is open
        if( !open() ) {
            qWarning( "\e[1;31m[FAIL]DbController::openDb can't open database\e[0m" );
            dbStatusOpen = false;
        }
    }

    return dbStatusOpen;
}


void DbController::loadAdmins()
{
    qDebug( "DbController::loadAdmins" );

    if( !openDb() ) { // open database for writing
        qWarning( "\e[1;31mDbController::loadAdmins FAILED to open database. No admins loaded\e[0m" );
        return;
    }

    QString auxUser, auxPassword;
    QSettings settings( adminFile, QSettings::IniFormat );
    int size = settings.beginReadArray( "OPLIST" ); // size of settings

    for( int i = 0; i < size; ++i ) {

        // get admins from config file
        settings.setArrayIndex( i );
        auxUser = settings.value( "user" ).toString();
        auxPassword = settings.value( "password" ).toString();

        QSqlQuery query;

        // check existance on database
        if( query.exec( "select user from oplist where user = '" + auxUser + "';" ) ) {

            if( query.next() )
                qWarning( "\e[0;33m%s already in database\e[0m", qPrintable( auxUser ) );
            else {    //write to database
                qDebug( "\e[0;33mnick( %s ) is not on database..Adding now..\e[0m", qPrintable( auxUser ) );

                if( !query.exec( "insert into oplist( user, password ) values('" + auxUser + "','" + auxPassword + "');" ) ) {
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

int DbController::genNewId( table t )
{
    QSqlQuery maxIdQuery;
    QString query;

    if( t == OPLIST )
        query = "select max( id ) from oplist;";
    else
        query = "select max( id ) from authed;";

    if( !maxIdQuery.exec( query ) ) {
        qWarning() << "\e[1;31m didn't execute query " << maxIdQuery.lastError() ;
        return 0;
    }

    bool ok;
    int id;

    maxIdQuery.next();   // got to record
    id = maxIdQuery.value( 0 ).toInt( &ok );

    if( !ok )
        return 1;
    else
        return id+1;
}

void DbController::setup()
{
    qDebug( "DbController::setup" );

    QString databasePath( "database" );
    databasePath.append( QDir::separator() );   //add separator at end

    // check if database folder exists
    if( !QDir().exists( databasePath ) ) {
        if( !QDir().mkdir( "database" ) ) { //create directory
            qWarning( "\e[1;31mDbController::setup can't create folder for database. Check permissions\e[0m" );
            return;
        }
    }

    // check if config file exists! ( extra control, who knows what stupid users can do ;)  )
    if( !QFile::exists( settingsFile ) ) {
        qWarning( "\e[1;31m DbController::setup FAIL , can't find settings file!\e[0m" );
        return;
    }

    // initialize settings to get dbName
    QSettings settings( QDir::toNativeSeparators( settingsFile ), QSettings::IniFormat );
    settings.beginReadArray( "DATABASE" );

    if( settings.status() == QSettings::FormatError ) {
        qWarning( "\e[1;31m DbController::setup FAILED to load settings. Format Error, check your config file\e[0m" );
        settings.endArray();
        return;
    }

    QString dbName = settings.value( "dbName" ).toString();
    setDatabaseName( databasePath + dbName );

    // check if database file exists
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
    if ( !openDb() ) {
        qWarning( "\e[1;31mDbController::setup can't create connection to SQLITE database: \"%s\"\e[0m", qPrintable( lastError().text() ) );
        return;
    }

    //i only opened database to test settings were correct. Now close
    close();

    createDatabaseFirstRun();
    loadAdmins();
}

