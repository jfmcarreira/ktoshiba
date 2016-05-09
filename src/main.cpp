/*
   Copyright (C) 2004-2016  Azael Avalos <coproscefalo@gmail.com>

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
#include <KLocalizedString>

#include "ktoshiba.h"
#include "ktoshiba_debug.h"
#include "ktoshiba_version.h"

int main(int argc, char *argv[])
{
    KAboutData aboutData(QStringLiteral("KToshiba"),
                         i18n("KToshiba"),
                         QStringLiteral(KTOSHIBA_VERSION_STRING),
                         i18n("Fn key monitoring for Toshiba laptops"),
                         KAboutLicense::GPL_V2,
                         i18n("(C) 2004-2015 Azael Avalos"));
    aboutData.setHomepage(QStringLiteral("http://ktoshiba.sourceforge.net/"));
    aboutData.addAuthor(i18n("Azael Avalos"),
                        i18n("Original Author"),
                        QStringLiteral("coproscefalo@gmail.com"));
    aboutData.addCredit(i18n("KDE Team"),
                        i18n("Some ideas and pieces of code"),
                        QString(),
                        QStringLiteral("http://www.kde.org/"));

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("KToshiba"));
    app.setApplicationVersion(QStringLiteral(KTOSHIBA_VERSION_STRING));
    app.setOrganizationDomain(QStringLiteral("sourceforge.net"));

    KAboutData::setApplicationData(aboutData);

    KDBusService service(KDBusService::Unique);

    KToshiba *ktoshiba = new KToshiba();

    QObject::connect(&service, SIGNAL(activateRequested(QStringList, QString)),
                     ktoshiba, SLOT(showApplication()));

    if (!ktoshiba->initialize()) {
        qCWarning(KTOSHIBA) << "Could not initialize KToshiba, the program will now exit";
        delete ktoshiba;

        exit(EXIT_FAILURE);
    }

    return app.exec();
}
