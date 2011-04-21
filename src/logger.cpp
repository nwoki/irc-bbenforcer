/****************************************************************************************
 * ioQIC-BBEnforcer - Qt irc bot that also sends rcon commands to UrbanTerror game server*
 * Copyright (C) 2010 - 2011, (n)woki                                                    *
 *                                                                                       *
 * logger.cpp is part of ioQIC-BBEnforcer                                                *
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

#include "logger.h"

#include <QDebug>
#include <QDate>
#include <QDir>
#include <QSettings>

#define DEFAULT_LOG_PATH "logs/"

Logger::Logger()
    : QFile (QString())
    , m_valid(true)
{
    loadSettings();
}

QString Logger::generateFileName()
{
    return QDate::currentDate().toString("dd-MM-yyyy");
}

void Logger::loadSettings()
{
    //set config file
    QSettings settings(QDir::toNativeSeparators("cfg/config"), QSettings::IniFormat);

    if (settings.status() == QSettings::FormatError) {
        qWarning("\e[1;31m Logger::loadSettings FAILED to load settings. Format Error, check your config file\e[0m");
        return;
    }

    qDebug("Connection::loadSettings IRC SETTINGS");

    settings.beginReadArray("LOGGER");

    QString path = settings.value("path").toString();

    if (!QDir::isAbsolutePath(path)) {
        // create relative DEFAULT path
        qDebug("\e[0;33mLogger::loadSettings using default path for log files because no valid path was specified in config\e[0m");

        // set to default log folder
        path = DEFAULT_LOG_PATH;

        if (!QDir(path).exists()) {
            if (!QDir().mkdir(path)) {
                qDebug("\e[1;31m Logger::loadSettings can't create default directory. Please check folder permissions\e[0m");
                m_valid = false;
                return;
            }
        }
    } else {
        // check absolute path existance
        if (!QDir(path).exists()) {
            if (!QDir().mkpath(path)) {
                qDebug("\e[1;31m Logger::loadSettings can't create directory. Please check folder permissions\e[0m");
                m_valid = false;
                return;
            }
        }
    }

    // check for last separator
    if (path[path.length()-1] != '/')
        path.append('/');

    // check for file existance
    path.append(generateFileName());

    // set file path
    setFileName(path);
    open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append);
}
