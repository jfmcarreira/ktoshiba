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

#include <qtimer.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kpassivepopup.h>
#include <kiconloader.h>

Suspend::Suspend(QWidget *parent)
    : QObject( parent ),
      m_Parent( parent )
{
    m_DBUSIFace = new KToshibaDBUSInterface();

    if (!initHAL())
        kdError() << "Suspend::Suspend(): Could not connect to HAL" << endl;

    dbus_terminated =  false;
    suspended = false;
    resumed = false;
    str_allowed = getAllowedSuspend("Suspend");
    std_allowed = getAllowedSuspend("Hibernate");

    connect( m_DBUSIFace, SIGNAL( msgReceived(msg_type, QString) ), this,
              SLOT( processMessage(msg_type, QString) ) );
}

Suspend::~Suspend()
{
    closeHAL();

    m_Parent = NULL;
    //delete m_DBUSIFace; m_DBUSIFace = NULL;
}

bool Suspend::initHAL()
{
    if (!m_DBUSIFace->isConnected())
        return false;

    DBusError error;
    dbus_error_init(&error);

    bool hal_is_ready = dbus_bus_name_has_owner(m_DBUSIFace->getDBUSConnection(), "org.freedesktop.Hal", &error);

    if (!hal_is_ready) {
        LIBHAL_FREE_DBUS_ERROR(&error);
        closeHAL();
        return false;
    }

    m_HALContext = libhal_ctx_new();

    /* setup dbus connection for hal */
    if (!libhal_ctx_set_dbus_connection(m_HALContext, m_DBUSIFace->getDBUSConnection())) {
        kdError() << "Suspend::initHAL(): "
                  << "Could not connect to D-Bus for HAL" << endl;
        closeHAL();
        return false;
    }

    /* init the hal library */
    if (!libhal_ctx_init(m_HALContext, &error)) {
        kdError() << "Suspend::initHAL(): "
                  << "Could not init HAL library: " << error.message << endl;
        closeHAL();
        return false;
    }

    hal_is_connected = true;

    return hal_is_connected;
}

void Suspend::closeHAL()
{
    if (m_HALContext != NULL) {
        libhal_ctx_free(m_HALContext);
        m_HALContext = NULL;
    }

    hal_is_connected = false;
}

bool Suspend::reconnectHAL()
{
    closeHAL();

    return initHAL();
}

bool Suspend::getAllowedSuspend(QString type)
{
    int res = -1;

    if (type == "Suspend" || type == "Hibernate")
        res = dbusSendMessage(REQUEST, type);

    return (res == 0)? true : false;
}

void Suspend::toRAM()
{
    switch (dbusSendMessage(ACTION, "Suspend")) {
        case NO_CONNECTION:
            KPassivePopup::message(i18n("WARNING"),
			i18n("No connection to the D-Bus daemon available."),
			SmallIcon("messagebox_warning", 20), m_Parent, i18n("WARNING"), 5000);
            return;
        case SUCCESS:
            kdDebug() << "Suspend::toRAM(): Suspending To RAM..." << endl;
            return;
        case FAILURE:
            KPassivePopup::message(i18n("WARNING"),
			i18n("Could not Suspend To RAM."),
			SmallIcon("messagebox_warning", 20), m_Parent, i18n("WARNING"), 5000);
            return;
        case NO_RIGTHS:
            KPassivePopup::message(i18n("WARNING"),
			i18n("Suspend To RAM disabled by administrator."),
			SmallIcon("messagebox_warning", 20), m_Parent, i18n("WARNING"), 5000);
            return;
        default:
            KPassivePopup::message(i18n("WARNING"),
			i18n("An unknown event was requested, ignoring..."),
			SmallIcon("messagebox_warning", 20), m_Parent, i18n("WARNING"), 5000);
            return;
    }
}

void Suspend::toDisk()
{
    switch (dbusSendMessage(ACTION, "Hibernate")) {
        case NO_CONNECTION:
            KPassivePopup::message(i18n("WARNING"),
			i18n("No connection to the D-Bus daemon available."),
			SmallIcon("messagebox_warning", 20), m_Parent, i18n("WARNING"), 5000);
            return;
        case SUCCESS:
            suspended = true;
            resumed = false;
            kdDebug() << "Suspend::toDisk(): Suspending To Disk..." << endl;
            emit setSuspendToDisk();
            return;
        case FAILURE:
            KPassivePopup::message(i18n("WARNING"),
			i18n("Could not Suspend To Disk."),
			SmallIcon("messagebox_warning", 20), m_Parent, i18n("WARNING"), 5000);
            return;
        case NO_RIGTHS:
            KPassivePopup::message(i18n("WARNING"),
			i18n("Suspend To Disk disabled by administrator."),
			SmallIcon("messagebox_warning", 20), m_Parent, i18n("WARNING"), 5000);
            return;
        default:
            KPassivePopup::message(i18n("WARNING"),
			i18n("An unknown event was requested, ignoring..."),
			SmallIcon("messagebox_warning", 20), m_Parent, i18n("WARNING"), 5000);
            return;
    }
}

bool Suspend::getHALProperty(QString path, QString iface, bool *retval)
{
    if (!hal_is_connected) return false;

    DBusError error;
    dbus_error_init(&error);

    if (!libhal_device_property_exists(m_HALContext, path, iface, &error)) {
        kdError() << "Suspend::getHALProperty(): Interface does not exist." << endl;
        return false;
    }

    *retval = libhal_device_get_property_bool(m_HALContext, path, iface, &error);

    if (!(*retval)) {
        LIBHAL_FREE_DBUS_ERROR(&error);
        return false;
    }

    return true;
}

int Suspend::dbusSendMessage(msgtype msg, QString method)
{
    switch (msg) {
        case REQUEST:
            if (hal_is_connected) {
                bool ret = false;
                if (method == "Suspend") {
                    if (getHALProperty(HAL_PATH, "power_management.can_suspend", &ret))
                        return (ret)? SUCCESS : NO_RIGTHS;
                    else
                        return FAILURE;
                } else
                if(method == "Hibernate") {
                    if (getHALProperty(HAL_PATH, "power_management.can_hibernate", &ret))
                        return (ret)? SUCCESS : NO_RIGTHS;
                    else
                        return FAILURE;
                } else
                    return UNKNOWN;
            } else
                return NO_CONNECTION;
            break;
        case ACTION:
            if (m_DBUSIFace->isConnected() && hal_is_connected && !dbus_terminated) {
                int *retval = NULL;
                if (method == "Suspend") {
                    if (getAllowedSuspend("Suspend")) {
                        return (m_DBUSIFace->methodCall(HAL_SERVICE, HAL_PATH, HAL_INTERFACE, method,
                                &retval, DBUS_TYPE_UINT32, DBUS_TYPE_UINT32, 0))? SUCCESS : FAILURE;
                    } else
                        return NO_RIGTHS;
                } else
                if (method == "Hibernate") {
                    if (getAllowedSuspend("Hibernate")) {
                        return (m_DBUSIFace->methodCall(HAL_SERVICE, HAL_PATH, HAL_INTERFACE, method,
                                &retval, DBUS_TYPE_UINT32, DBUS_TYPE_INVALID))? SUCCESS : FAILURE;
                    } else
                        return NO_RIGTHS;
                } else
                    return UNKNOWN;
            } else
                return NO_CONNECTION;
            break;
    }

    return FAILURE;
}

void Suspend::checkDaemon()
{
    if (!m_DBUSIFace->isConnected() && !m_DBUSIFace->reconnect())
        KPassivePopup::message(i18n("WARNING"),
            i18n("D-Bus daemon not running."),
            SmallIcon("messagebox_warning", 20), m_Parent, i18n("WARNING"), 5000);
}

void Suspend::processMessage(msg_type type, QString signal)
{
    switch (type) {
        case DBUS_EVENT:
            if (signal.startsWith("dbus.terminate")) {
                QTimer::singleShot( 4000, this, SLOT( checkDaemon() ) );
                dbus_terminated = true;
            }
            break;
        case HAL_EVENT:
            kdDebug() << "Suspend::processMessage(): HAL Event: "
                      << signal.ascii() << endl;
            // TODO: Implement me...
            break;
    }
}


#include "suspend.moc"
