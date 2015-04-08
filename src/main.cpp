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

#include <QApplication>

#include <KAboutData>
#include <KDBusAddons/KDBusService>

#include "ktoshiba.h"
#include "version.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName("KToshiba");
    QCoreApplication::setApplicationVersion(ktoshiba_version);
    QCoreApplication::setOrganizationDomain("sourceforge.net");

    QApplication app(argc, argv);

    KToshiba::createAboutData();
    //KAboutData::setApplicationData(KToshiba::aboutData());

    KToshiba *ktoshiba = new KToshiba();
    if (!ktoshiba->initialize()) {
        delete ktoshiba;
        exit(-1);
    }

    KDBusService service(KDBusService::Unique);

    return app.exec();
}
