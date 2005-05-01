/***************************************************************************
 *   Copyright (C) 2004 by Azael Avalos                                    *
 *   neftali@utep.edu                                                      *
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
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

static const char description[] =
    I18N_NOOP("Hotkeys & Battery monitoring for Toshiba laptops.");

static const char version[] = "0.6Alpha";

static KCmdLineOptions options[] =
{
//    { "+[URL]", I18N_NOOP( "Document to open." ), 0 },
    KCmdLineLastOption
};

int main(int argc, char **argv)
{
    KAboutData about("ktoshiba", I18N_NOOP("KToshiba"), version, description,
                     KAboutData::License_GPL, "(C) 2004 Azael Avalos", 0, 0, "coproscefalo@gmail.com" );
    about.addAuthor( "Azael Avalos", 0, "neftali@utep.edu" );
	about.addCredit( "Jonathan A. Buzzard", I18N_NOOP("toshutils and HCI-SCI stuff"),
					"jonathan@buzzard.org.uk", "http://www.buzzard.org.uk/toshiba/" );
	about.addCredit( "John Belmonte", I18N_NOOP("Toshiba Laptop ACPI Extras driver"),
					0, "http://memebeam.org/toys/ToshibaAcpiDriver/" );
	about.addCredit( "Thomas Renninger", I18N_NOOP("Powersave Daemon & KPowersave"), "trenn@suse.de", 0 );
	about.addCredit( "KDE Team", I18N_NOOP("Some ideas and pieces of code"), 0, "http://www.kde.org/" );
	about.addCredit( "ksynaptics Team", I18N_NOOP("Code for enabling/disablig TouchPad"),
					0, "http://qsynaptics.sourceforge.net/" );
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions( options );
    KApplication app;
    KToshiba *mainWin = 0;

        // no session.. just start up normally
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        /// @todo do something with the command line args here

        mainWin = new KToshiba();
        app.setMainWidget( mainWin );
        mainWin->show();

        args->clear();

    // mainWin has WDestructiveClose flag by default, so it will delete itself.
    return app.exec();
}
