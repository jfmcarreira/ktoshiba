 /*
   Copyright (C) 2004-2011  Azael Avalos <coproscefalo@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QX11Info>

#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include "ktoshiba.h"

extern "C" {
#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
}

int main(int argc, char *argv[])
{
    KToshiba::createAboutData();
    KCmdLineArgs::init(argc, argv, KToshiba::aboutData());
    KUniqueApplication::addCmdLineOptions();

    if (!KUniqueApplication::start()) {
        fprintf(stderr, "KToshiba is already running!\n");
        exit(0);
    }

    KUniqueApplication *app = new KToshiba();
    app->disableSessionManagement();
    app->setQuitOnLastWindowClosed( false );

    int ret = app->exec();
    XSelectInput(QX11Info::display(), DefaultRootWindow(QX11Info::display()), KeyPressMask);
    KToshiba::destroyAboutData();

    return ret;
}
