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
#include "ktoshibadbusinterface.h"

#include <qtimer.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kpassivepopup.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kprocess.h>

#ifdef ENABLE_POWERSAVE
#include <powerlib.h>
#endif // ENABLE_POWERSAVE

Suspend::Suspend(QWidget *parent)
    : QObject( parent )
{
    m_Parent = parent;
    m_Info = i18n("Before continuing, be aware that ACPI Sleep States are a "
                  "work in progress and may or may not work on your computer.\n"
                  "Also make sure to unload problematic modules");

    m_DBUSIFace = new KToshibaDBUSInterface();

#ifdef ENABLE_POWERSAVE
    powersaved_terminated = false;
#endif // ENABLE_POWERSAVE
    dbus_terminated =  false;
    suspended = false;
    resumed = false;

    connect( m_DBUSIFace, SIGNAL( msgReceived(msg_type, QString) ), this,
              SLOT( processMessage(msg_type, QString) ) );
}

Suspend::~Suspend()
{
    delete m_Parent; m_Parent = NULL;
    delete m_DBUSIFace; m_DBUSIFace = NULL;
}

void Suspend::toRAM()
{
    int res = KMessageBox::warningContinueCancel(m_Parent, m_Info, i18n("WARNING"));

    if (res == KMessageBox::Cancel)
        return;
#ifdef ENABLE_POWERSAVE
    if (res == KMessageBox::Continue)
        res = dbusSendMessage(ACTION, "SuspendToRAM");

    switch (res) {
        case REPLY_SUCCESS:
            kdDebug() << "Suspend::toRAM(): Suspending To RAM (using powersave)" << endl;
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
            kdDebug() << res << endl;
            return;
    }
#else // ENABLE_POWERSAVE
#ifdef ENABLE_HELPER
    QString helper = KStandardDirs::findExe("ktosh_helper");
    if (helper.isEmpty())
        helper = KStandardDirs::findExe("klaptop_acpi_helper");
    if (helper.isEmpty()) {
        KMessageBox::sorry(m_Parent, i18n("Could not Suspend To RAM because ktosh_helper cannot be found.\n"
                           "Please make sure that it is installed correctly."),
                           i18n("Suspend To RAM"));
        return;
    }

    if (res == KMessageBox::Continue) {
        kdDebug() << "Suspend::toRAM(): Suspending To RAM (using ACPI)" << endl;
        KProcess proc;
        proc << helper << "--suspend";
        proc.start(KProcess::DontCare);
        proc.detach();
    }

    return;
#endif // ENABLE_HELPER
    KMessageBox::sorry(m_Parent, i18n("No Suspend To RAM support was enabled."),
                       i18n("Suspend To RAM"));
#endif // ENABLE_POWERSAVE
}

void Suspend::toDisk()
{
    int res = KMessageBox::warningContinueCancel(m_Parent, m_Info, i18n("WARNING"));

    if (res == KMessageBox::Cancel)
        return;
#ifdef ENABLE_POWERSAVE
    if (res == KMessageBox::Continue)
        res = dbusSendMessage(ACTION, "SuspendToDisk");

    switch (res) {
        case REPLY_SUCCESS:
            suspended = true;
            kdDebug() << "Suspend::toDisk(): Suspending To Disk (using powersave)" << endl;
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
#ifdef ENABLE_HELPER
    QString helper = KStandardDirs::findExe("ktosh_helper");
    if (helper.isEmpty())
        helper = KStandardDirs::findExe("klaptop_acpi_helper");
    if (helper.isEmpty()) {
        KMessageBox::sorry(m_Parent, i18n("Could not Suspend To Disk because ktosh_helper cannot be found.\n"
                           "Please make sure that it is installed correctly."),
                           i18n("Suspend To Disk"));
        return;
    }

    if (res == KMessageBox::Continue) {
        kdDebug() << "Suspend::toDisk(): Suspending To Disk (using ACPI)" << endl;
        KProcess proc;
        proc << helper << "--hibernate";
        proc.start(KProcess::DontCare);
        proc.detach();
        QTimer::singleShot( 1000, this, SLOT( emitSTD() ) );
    }

    return;
#endif // ENABLE_HELPER
    KMessageBox::sorry(m_Parent, i18n("No Suspend To Disk support was enabled."),
                       i18n("Suspend To Disk"));
#endif // ENABLE_POWERSAVE
}

int Suspend::dbusSendMessage(msgtype msg, QString type)
{
    if (m_DBUSIFace->isConnected()) {
        if (!dbus_terminated) {
            // TODO: Implement me...
            // Dummy entry to avoid compiler warnings
            if (msg) return 1; if (type) return 1;
        }
#ifdef ENABLE_POWERSAVE
        if (!powersaved_terminated) {
            char *ret = NULL;
            switch (msg) {
                case REQUEST:
                    if (type == "AllowedSuspendToRam" || type == "AllowedSuspendToDisk") {
                        if (m_DBUSIFace->psMethodCall(type, &ret, DBUS_TYPE_UINT16, DBUS_TYPE_INVALID))
                            return REPLY_SUCCESS;
                    } else
                    if (type == "Ping") {
                        if (m_DBUSIFace->psMethodCall(type, &ret, DBUS_TYPE_UINT16, DBUS_TYPE_INVALID))
                            return REPLY_SUCCESS;
                    } else
                        return REPLY_DISABLED;
                    break;
                case ACTION:
                    if (type == "SuspendToRAM" || type == "SuspendToDisk") {
                        if (m_DBUSIFace->psMethodCall(type, &ret, DBUS_TYPE_UINT16, DBUS_TYPE_INVALID))
                            return REPLY_SUCCESS;
                        else
                            return REPLY_DISABLED;
                    }
                    break;
            }
        }
#endif // ENABLE_POWERSAVE
    }

    return -1;
}

void Suspend::checkDaemon()
{
    if (!m_DBUSIFace->isConnected() && !m_DBUSIFace->reconnect()) {
        KPassivePopup::message(i18n("WARNING"),
            i18n("D-BUS daemon not running."),
            SmallIcon("messagebox_warning", 20), m_Parent, i18n("WARNING"), 5000);
#ifdef ENABLE_POWERSAVE
        if (powersaved_terminated) {
            KPassivePopup::message(i18n("WARNING"),
                i18n("The powersave daemon must be running in the background."),
                SmallIcon("messagebox_warning", 20), m_Parent, i18n("WARNING"), 5000);
        }
#endif // ENABLE_POWERSAVE
    }
}

void Suspend::processMessage(msg_type type, QString signal)
{
    switch (type) {
        case DBUS_EVENT:
            kdDebug() << "Suspend::processMessage(): D-BUS Event: " << signal.ascii() << endl;
            if (signal.startsWith("dbus.terminate")) {
                QTimer::singleShot( 4000, this, SLOT( checkDaemon() ) );
                dbus_terminated = true;
            }
            break;
        case POWERSAVE_EVENT:
#ifdef ENABLE_POWERSAVE
            if (signal.startsWith("daemon.terminate")) {
                kdDebug() << "Suspend::processMessage(): Powersave Event: "
                          << signal.ascii() << endl;
                powersaved_terminated = true;
            } else
            if (signal.startsWith("global.suspend2disk")) {
                QTimer::singleShot( 1000, this, SLOT( emitSTD() ) );
                suspended = true;
                resumed = false;
            } else
            if (signal.startsWith("global.resume.suspend2disk")) {
                QTimer::singleShot( 60000, this, SLOT( emitResumedSTD() ) );
                resumed = true;
                suspended = false;
            }
#endif // ENABLE_POWERSAVE
            break;
    }
}

void Suspend::emitSTD()
{
    emit setSuspendToDisk();
    suspended = true;
    resumed = false;
}

void Suspend::emitResumedSTD()
{
    emit resumedFromSTD();
    resumed = true;
    suspended = false;
}


#include "suspend.moc"
