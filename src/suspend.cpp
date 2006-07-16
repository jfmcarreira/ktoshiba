/***************************************************************************
 *   Copyright (C) 2006 by Azael Avalos                                    *
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

#include "suspend.h"

#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kpassivepopup.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kprocess.h>

#ifdef ENABLE_POWERSAVE
#include <powersave_dbus.h>
#endif // ENABLE_POWERSAVE

suspend::suspend(QWidget *parent)
    : QObject( parent )
{
    m_Parent = parent;
}

suspend::~ suspend()
{
    delete m_Parent; m_Parent = NULL;
}

void suspend::toRAM()
{
    int res = KMessageBox::warningContinueCancel(0,
			i18n("Before continuing, be aware that ACPI Sleep States\n"
			 "are a work in progress and may or may not work on your computer.\n"
			 "Also make sure to unload problematic modules"), i18n("WARNING"));

#ifdef ENABLE_POWERSAVE
    if (res == KMessageBox::Continue)
        res = dbusSendSimpleMessage(ACTION_MESSAGE, "SuspendToRam");
    else if (res == KMessageBox::Cancel)
        return;

    switch (res) {
        case REPLY_SUCCESS:
            kdDebug() << "KToshiba: Suspending To RAM (using powersave)" << endl;
            return;
        case REPLY_DISABLED:
            KPassivePopup::message(i18n("WARNING"),
			i18n("Suspend To RAM disabled by administrator."),
			SmallIcon("messagebox_warning", 20), m_Parent, i18n("WARNING"), 5000);
            return;
        default:
            KPassivePopup::message(i18n("WARNING"),
			i18n("The powersave daemon must be running in the background for a Suspend To RAM."),
			SmallIcon("messagebox_warning", 20), m_Parent, i18n("WARNING"), 5000);
            return;
    }
#else // ENABLE_POWERSAVE
    QString helper = KStandardDirs::findExe("ktosh_helper");
    if (helper.isEmpty())
        helper = KStandardDirs::findExe("klaptop_acpi_helper");
    if (helper.isEmpty()) {
        KMessageBox::sorry(0, i18n("Could not Suspend To RAM because ktosh_helper cannot be found.\n"
			   "Please make sure that it is installed correctly."),
			   i18n("STR"));
        return;
    }

    if (res == KMessageBox::Continue) {
        kdDebug() << "KToshiba: Suspending To RAM (using ACPI)" << endl;
        KProcess proc;
        proc << helper << "--suspend";
        proc.start(KProcess::DontCare);
        proc.detach();
    }
#endif // ENABLE_POWERSAVE
}

void suspend::toDisk()
{
    int res = KMessageBox::warningContinueCancel(0,
			i18n("Before continuing, be aware that ACPI Sleep States\n"
			 "are a work in progress and may or may not work on your computer.\n"
			 "Also make sure to unload problematic modules"), i18n("WARNING"));

#ifdef ENABLE_POWERSAVE // ENABLE_POWERSAVE
    if (res == KMessageBox::Continue)
        res = dbusSendSimpleMessage(ACTION_MESSAGE, "SuspendToDisk");
    else if (res == KMessageBox::Cancel)
        return;

    switch (res) {
        case REPLY_SUCCESS:
            kdDebug() << "KToshiba: Suspending To Disk (using powersave)" << endl;
            return;
        case REPLY_DISABLED:
            KPassivePopup::message(i18n("WARNING"),
			i18n("Suspend To Disk disabled by administrator."),
			SmallIcon("messagebox_warning", 20), m_Parent, i18n("WARNING"), 5000);
            return;
        default:
            KPassivePopup::message(i18n("WARNING"),
			i18n("The powersave daemon must be running in the background for a Suspend To Disk."),
			SmallIcon("messagebox_warning", 20), m_Parent, i18n("WARNING"), 5000);
            return;
    }
#else // ENABLE_POWERSAVE
    QString helper = KStandardDirs::findExe("ktosh_helper");
    if (helper.isEmpty())
        helper = KStandardDirs::findExe("klaptop_acpi_helper");
    if (helper.isEmpty()) {
        KMessageBox::sorry(0, i18n("Could not Suspend To Disk because ktosh_helper cannot be found.\n"
			   "Please make sure that it is installed correctly."),
			   i18n("STD"));
        return;
    }

    if (res == KMessageBox::Continue) {
        kdDebug() << "KToshiba: Suspending To Disk (using ACPI)" << endl;
        KProcess proc;
        proc << helper << "--hibernate";
        proc.start(KProcess::DontCare);
        proc.detach();
    }
#endif // ENABLE_POWERSAVE
}


#include "suspend.moc"
