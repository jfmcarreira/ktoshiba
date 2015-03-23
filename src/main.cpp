/*
   Copyright (C) 2004-2015  Azael Avalos <coproscefalo@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, see
   <http://www.gnu.org/licenses/>.
*/

#include <KUniqueApplication>
#include <KAboutData>
#include <KCmdLineArgs>

extern "C" {
#include <stdio.h>
#include <stdlib.h>
}

#include "ktoshiba.h"
#include "version.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName("KToshiba");
    QCoreApplication::setApplicationVersion(ktoshiba_version);
    QCoreApplication::setOrganizationDomain("sourceforge.net");

    KToshiba::createAboutData();
    KCmdLineArgs::init(argc, argv, KToshiba::aboutData());
    KUniqueApplication::addCmdLineOptions();

    if (!KUniqueApplication::start()) {
        fprintf(stderr, "KToshiba is already running!\n");
        return 0;
    }

    KUniqueApplication app;
    app.disableSessionManagement();
    app.setQuitOnLastWindowClosed( false );

    KToshiba *ktoshiba = new KToshiba();
    if (!ktoshiba->initialize()) {
        delete ktoshiba;
        return -1;
    }

    return app.exec();
}
