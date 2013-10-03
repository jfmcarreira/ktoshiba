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

#include <QtDBus/QtDBus>
#include <QString>

#include <KDebug>

#include "ktoshibadbusinterface.h"
#include "ktoshibadbusadaptor.h"

KToshibaDBusInterface::KToshibaDBusInterface(QObject *parent)
    : QObject( parent )
{
    new KToshibaDBusAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService("net.sourceforge.KToshiba");
    dbus.registerObject("/", this);
}

void KToshibaDBusInterface::gotKey(int key)
{
    kDebug() << "Key received: " << key << endl;
}

bool KToshibaDBusInterface::checkCompositeStatus()
{
    QDBusInterface iface("org.kde.kwin",
			 "/KWin",
			 "org.kde.KWin",
			 QDBusConnection::sessionBus(), this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
        return false;
    }

    QDBusReply<bool> reply = iface.call("compositingActive");

    if (!reply.isValid()) {
        QDBusError err(reply.error());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
        return false;
    }

    return reply.value();
}

void KToshibaDBusInterface::lockScreen()
{
    QDBusInterface iface("org.freedesktop.ScreenSaver",
			 "/ScreenSaver",
			 "org.freedesktop.ScreenSaver",
			 QDBusConnection::sessionBus(), this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
        return;
    }

    QDBusReply<void> reply = iface.call("Lock");

    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
    }
}

void KToshibaDBusInterface::setZoom(int zoom)
{
    QDBusInterface iface("org.kde.kglobalaccel",
			 "/component/kwin",
			 "org.kde.kglobalaccel.Component",
			 QDBusConnection::sessionBus(), this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
        return;
    }

    QDBusReply<void> reply;
    switch(zoom) {
    case ZoomReset:
        reply = iface.call("invokeShortcut", "view_actual_size");
        break;
    case ZoomIn:
        reply = iface.call("invokeShortcut", "view_zoom_in");
        break;
    case ZoomOut:
        reply = iface.call("invokeShortcut", "view_zoom_out");
        break;
    }

    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
    }
}


#include "ktoshibadbusinterface.moc"
