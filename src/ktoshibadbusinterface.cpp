/*
   Copyright (C) 2004-2014  Azael Avalos <coproscefalo@gmail.com>

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
#include <KWindowSystem>

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

void KToshibaDBusInterface::suspendTo(QString type)
{
    if (type == QString("RAM"))
        str();
    else if (type == QString("Disk"))
        std();
}

void KToshibaDBusInterface::str()
{
    QDBusInterface iface("org.freedesktop.PowerManagement",
			 "/org/freedesktop/PowerManagement",
			 "org.freedesktop.PowerManagement",
			 QDBusConnection::sessionBus(), this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
        return;
    }

    QDBusReply<bool> reply = iface.call("CanSuspend");
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
    }

    if (!reply.value()) {
        kWarning() << "System does not support STR (Suspend)" << endl;
        return;
    }

    QDBusReply<void> str = iface.call("Suspend");
    if (!str.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
    }
}

void KToshibaDBusInterface::std()
{
    QDBusInterface iface("org.freedesktop.PowerManagement",
			 "/org/freedesktop/PowerManagement",
			 "org.freedesktop.PowerManagement",
			 QDBusConnection::sessionBus(), this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
        return;
    }

    QDBusReply<bool> reply = iface.call("CanHibernate");
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
    }

    if (!reply.value()) {
        kWarning() << "System does not support STD (Hibernate)" << endl;
        return;
    }

    QDBusReply<void> std = iface.call("Hibernate");
    if (!std.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
    }
}

void KToshibaDBusInterface::setBrightness(int level)
{
    QDBusInterface iface("org.kde.Solid.PowerManagement",
			 "/org/kde/Solid/PowerManagement/Actions/BrightnessControl",
			 "org.kde.Solid.PowerManagement.BrightnessControl",
			 QDBusConnection::sessionBus(), this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
        return;
    }

    QDBusReply<int> bright = iface.call("brightness");
    if (!bright.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
    }

    if ((bright == 0 && level == -1) || (bright == 100 && level == 1))
        return;

    int brightness = bright.value();
    if (level == 1)
        brightness += 15;
    else
        brightness -= 15;

    QDBusReply<void> reply = iface.call("setBrightness", brightness);
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
    }
}

void KToshibaDBusInterface::kbdBacklight(bool state)
{
    QDBusInterface iface("org.kde.Solid.PowerManagement",
			 "/org/kde/Solid/PowerManagement/Actions/KeyboardBrightnessControl",
			 "org.kde.Solid.PowerManagement.Actions.KeyboardBrightnessControl",
			 QDBusConnection::sessionBus(), this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
        return;
    }

    QDBusReply<void> reply;
    reply = iface.call("keyboardBrightness", state ? 1 : 0);
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
    }
}

void KToshibaDBusInterface::setZoom(int zoom)
{
    if (!KWindowSystem::compositingActive()) {
        kWarning() << "Compositing have been disabled, Zoom actions cannot be activated" << endl;
        return;
    }

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
