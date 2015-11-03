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
#include <KLocalizedString>
#include <KDBusAddons/KDBusService>

#include "ktoshiba.h"
#include "version.h"

int main(int argc, char *argv[])
{
    KAboutData aboutData(QLatin1String("KToshiba"),
                         i18n("KToshiba"),
                         ktoshiba_version,
                         i18n("Fn key monitoring for Toshiba laptops"),
                         KAboutLicense::GPL_V2,
                         QString(),
                         i18n("Copyright (C) 2004-2015 Azael Avalos"),
                         QLatin1String("http://ktoshiba.sourceforge.net/"),
                         QLatin1String("coproscefalo@gmail.com"));

    aboutData.addAuthor(i18n("Azael Avalos"),
                        i18n("Original Author"),
                        QLatin1String("coproscefalo@gmail.com"));
    aboutData.addCredit(i18n("KDE Team"),
                        i18n("Some ideas and pieces of code"),
                        QString(),
                        QLatin1String("http://www.kde.org/"));

    QApplication app(argc, argv);
    app.setApplicationName("KToshiba");
    app.setApplicationVersion(ktoshiba_version);
    app.setOrganizationDomain("sourceforge.net");

    KAboutData::setApplicationData(aboutData);

    KDBusService service(KDBusService::Unique);

    KToshiba *ktoshiba = new KToshiba();

    QObject::connect(&service, SIGNAL(activateRequested(QStringList, QString)),
                     ktoshiba, SLOT(showApplication()));

    if (!ktoshiba->initialize()) {
        delete ktoshiba;

        exit(EXIT_FAILURE);
    }

    return app.exec();
}
