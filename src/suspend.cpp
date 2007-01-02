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

Suspend::Suspend(QWidget *parent)
    : QObject( parent )
{
    m_Parent = parent;
    m_Info = i18n("Before continuing, be aware that ACPI Sleep States are a "
                  "work in progress and may or may not work on your computer.\n"
                  "Also make sure to unload problematic modules");

    m_DBUSIFace = new KToshibaDBUSInterface();

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
    KMessageBox::sorry(m_Parent, i18n("No Suspend To RAM support was enabled."),
                       i18n("Suspend To RAM"));
}

void Suspend::toDisk()
{
    int res = KMessageBox::warningContinueCancel(m_Parent, m_Info, i18n("WARNING"));

    if (res == KMessageBox::Cancel)
        return;
    KMessageBox::sorry(m_Parent, i18n("No Suspend To Disk support was enabled."),
                       i18n("Suspend To Disk"));
}

int Suspend::dbusSendMessage(msgtype msg, QString type)
{
    // Dummy entry to avoid compiler warnings
    if (msg || type) ;

    if (m_DBUSIFace->isConnected())
        if (!dbus_terminated) {
            // TODO: Implement me...
        }

    return -1;
}

void Suspend::checkDaemon()
{
    if (!m_DBUSIFace->isConnected() && !m_DBUSIFace->reconnect())
        KPassivePopup::message(i18n("WARNING"),
            i18n("D-BUS daemon not running."),
            SmallIcon("messagebox_warning", 20), m_Parent, i18n("WARNING"), 5000);
}

void Suspend::processMessage(msg_type type, QString signal)
{
    switch (type) {
        case DBUS_EVENT:
            //kdDebug() << "Suspend::processMessage(): D-BUS Event: " << signal.ascii() << endl;
            if (signal.startsWith("dbus.terminate")) {
                QTimer::singleShot( 4000, this, SLOT( checkDaemon() ) );
                dbus_terminated = true;
            }
            break;
        case HAL_EVENT:
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
