/***************************************************************************************
* ioQIC - Qt irc bot that also sends rcon commands to UrbanTerror game server          *
* Copyright (C) 2010, woki                                                             *
*                                                                                      *
* main.cpp is part of ioQIC                                                            *
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

#include <QtCore/QCoreApplication>
#include <iostream>

#include "Brain.h"
#include "Connection.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    std::cout << "\e[1;32m";
    std::cout << "*****************************************************************" << std::endl;
    std::cout << "* ioQIC Copyright (C) 2010 'woki'                               *" << std::endl;
    std::cout << "* This program comes with ABSOLUTELY NO WARRANTY                *" << std::endl;
    std::cout << "* This is free software, and you are welcome to redistribute it *" << std::endl;
    std::cout << "* under certain conditions.                                     *" << std::endl;
    std::cout << "* See LICENSE for details                                       *" << std::endl;
    std::cout << "*****************************************************************\e[0m" << std::endl;

    Brain *brain = new Brain();
    return a.exec();
}
