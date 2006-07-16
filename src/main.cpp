/***************************************************************************
 *   Copyright (C) 2004-2006 by Azael Avalos                               *
 *   coproscefalo@gmail.com                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "ktoshiba.h"

#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kwin.h>

#if defined Q_WS_X11
#include <qxembed.h>
#endif

static const char description[] =
    I18N_NOOP("Hotkeys monitoring for Toshiba laptops.");

static const char version[] = "0.9Alpha";

int main(int argc, char *argv[])
{
    KAboutData about("ktoshiba", I18N_NOOP("KToshiba"), version, description,
                     KAboutData::License_GPL, "(C) 2004-2006 Azael Avalos", 0,
                     "http://ktoshiba.sourceforge.net/", "coproscefalo@gmail.com" );
    about.addAuthor( "Azael Avalos", 0, "coproscefalo@gmail.com", 0 );
    about.addCredit( "Jonathan A. Buzzard", I18N_NOOP("toshutils and HCI-SCI stuff"),
                    "jonathan@buzzard.org.uk", "http://www.buzzard.org.uk/toshiba/" );
    about.addCredit( "John Belmonte", I18N_NOOP("Toshiba Laptop ACPI Extras driver"),
                    "john@neggie.net", "http://memebeam.org/toys/ToshibaAcpiDriver/" );
    about.addCredit( "Thomas Renninger", I18N_NOOP("Powersave Daemon & KPowersave"),
                    "trenn@suse.de", 0 );
    about.addCredit( "KDE Team", I18N_NOOP("Some ideas and pieces of code"), 0,
                    "http://www.kde.org/" );
    about.addCredit( "ksynaptics Team", I18N_NOOP("library for enabling/disabling TouchPad"),
                    0, "http://qsynaptics.sourceforge.net/" );
    about.addCredit( "Nicolas Ternisien", I18N_NOOP("French translation"),
                    "nicolas.ternisien@gmail.com", 0 );
    about.addCredit( "Charles Barcza", I18N_NOOP("Hungarian translation"),
                    "kbarcza@blackpanther.hu", "http://www.blackpanther.hu" );
    about.addCredit( "Gonzalo Raúl Nemmi", I18N_NOOP("omnibook stuff tester"),
                    "gnemmi@gmail.com", 0 );
    KCmdLineArgs::init(argc, argv, &about);

    if (!KUniqueApplication::start()) {
        fprintf(stderr, "KToshiba is already running!\n");
        exit(0);
    }

    KUniqueApplication app;
    KToshiba *mainWin = new KToshiba();

#if defined Q_WS_X11 && ! defined K_WS_QTONLY
    QXEmbed::initialize();
#endif

    app.disableSessionManagement();
    app.setMainWidget( mainWin );
    mainWin->show();

    int res = app.exec();
    delete mainWin; mainWin = NULL;
    return res;
}
