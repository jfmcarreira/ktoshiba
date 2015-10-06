/*
   Copyright (C) 2004-2015  Azael Avalos <coproscefalo@gmail.com>

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

#include <QDebug>

#include "ktoshibadbusinterface.h"
#include "ktoshibadbusadaptor.h"
#include "fnactions.h"
#include "ktoshibahardware.h"

KToshibaDBusInterface::KToshibaDBusInterface(FnActions *parent)
    : QObject(parent),
      m_service(false),
      m_object(false)
{
    m_fn = qobject_cast<FnActions *>(QObject::parent());
}

KToshibaDBusInterface::~KToshibaDBusInterface()
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    if (m_object)
        dbus.unregisterObject("/net/sourceforge/KToshiba");

    if (m_service)
        if (!dbus.unregisterService("net.sourceforge.KToshiba"))
            qCritical() << "Could not unregister DBus service";
}

void KToshibaDBusInterface::init()
{
    new KToshibaDBusAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    m_service = dbus.registerService("net.sourceforge.KToshiba");
    if (!m_service)
        qCritical() << "Could not register DBus service";

    m_object = dbus.registerObject("/net/sourceforge/KToshiba", this);
    if (!m_object)
        qCritical() << "Could not register DBus object";

    dbus.connect("org.kde.KWin", "/Compositor", "org.kde.kwin.Compositing",
                 "compositingToggled", m_fn, SLOT(compositingChanged(bool)));
    dbus.connect("org.kde.Solid.PowerManagement", "/org/kde/Solid/PowerManagement",
                 "org.kde.Solid.PowerManagement", "profileChanged",
                 this, SLOT(profileChanged(QString)));
}

void KToshibaDBusInterface::configFileChanged()
{
    emit configChanged();
}

void KToshibaDBusInterface::profileChanged(QString profile)
{
    emit batteryProfileChanged(profile);
}

void KToshibaDBusInterface::lockScreen()
{
    QDBusInterface iface("org.freedesktop.ScreenSaver",
                         "/ScreenSaver",
                         "org.freedesktop.ScreenSaver",
                         QDBusConnection::sessionBus(), this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        qCritical() << err.name() << "Message:" << err.message();

        return;
    }

    QDBusReply<void> reply = iface.call("Lock");
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        qCritical() << err.name() << "Message:" << err.message();
    }
}

void KToshibaDBusInterface::setBrightness(int level)
{
    QDBusInterface iface("org.kde.Solid.PowerManagement",
                         "/org/kde/Solid/PowerManagement/Actions/BrightnessControl",
                         "org.kde.Solid.PowerManagement.Actions.BrightnessControl",
                         QDBusConnection::sessionBus(), this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        qCritical() << err.name() << "Message:" << err.message();

        return;
    }

    QDBusReply<void> reply = iface.call("setBrightness", level);
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        qCritical() << err.name() << "Message:" << err.message();
    }
}

void KToshibaDBusInterface::setKBDBacklight(int state)
{
    QDBusInterface iface("org.kde.Solid.PowerManagement",
                         "/org/kde/Solid/PowerManagement/Actions/KeyboardBrightnessControl",
                         "org.kde.Solid.PowerManagement.Actions.KeyboardBrightnessControl",
                         QDBusConnection::sessionBus(), this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        qCritical() << err.name() << "Message:" << err.message();

        return;
    }

    QDBusReply<void> reply = iface.call("keyboardBrightness", state);
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        qCritical() << err.name() << "Message:" << err.message();
    }
}

void KToshibaDBusInterface::setZoom(int zoom)
{
    if (!getCompositingState()) {
        qWarning() << "Compositing have been disabled, Zoom actions cannot be activated";

        return;
    }

    QDBusInterface iface("org.kde.kglobalaccel",
                         "/component/kwin",
                         "org.kde.kglobalaccel.Component",
                         QDBusConnection::sessionBus(), this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        qCritical() << err.name() << "Message:" << err.message();

        return;
    }

    QDBusReply<void> reply;
    switch (zoom) {
    case FnActions::Reset:
        reply = iface.call("invokeShortcut", "view_actual_size");
        break;
    case FnActions::In:
        reply = iface.call("invokeShortcut", "view_zoom_in");
        break;
    case FnActions::Out:
        reply = iface.call("invokeShortcut", "view_zoom_out");
        break;
    }

    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        qCritical() << err.name() << "Message:" << err.message();
    }
}

uint KToshibaDBusInterface::inhibitPowerManagement(QString reason)
{
    QDBusInterface iface("org.freedesktop.PowerManagement.Inhibit",
                         "/org/freedesktop/PowerManagement/Inhibit",
                         "org.freedesktop.PowerManagement.Inhibit",
                         QDBusConnection::sessionBus(), this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        qCritical() << err.name() << "Message:" << err.message();

        return 0;
    }

    QDBusReply<uint> reply = iface.call("Inhibit", QString("KToshiba"), reason);
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        qCritical() << err.name() << "Message:" << err.message();

        return 0;
    }

    return reply;
}

void KToshibaDBusInterface::unInhibitPowerManagement(uint cookie)
{
    QDBusInterface iface("org.freedesktop.PowerManagement.Inhibit",
                         "/org/freedesktop/PowerManagement/Inhibit",
                         "org.freedesktop.PowerManagement.Inhibit",
                         QDBusConnection::sessionBus(), this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        qCritical() << err.name() << "Message:" << err.message();

        return;
    }

    QDBusReply<void> reply = iface.call("UnInhibit", cookie);
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        qCritical() << err.name() << "Message:" << err.message();
    }
}

bool KToshibaDBusInterface::getCompositingState()
{
    QDBusInterface iface("org.kde.KWin",
                         "/Compositor",
                         "org.kde.kwin.Compositing",
                         QDBusConnection::sessionBus(), this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        qCritical() << err.name() << "Message:" << err.message();

        return false;
    }

    return iface.property("active").toBool();
}

QString KToshibaDBusInterface::getBatteryProfile()
{
    QDBusInterface iface("org.kde.Solid.PowerManagement",
                         "/org/kde/Solid/PowerManagement",
                         "org.kde.Solid.PowerManagement",
                         QDBusConnection::sessionBus(), this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        qCritical() << err.name() << "Message:" << err.message();

        return QString();
    }

    QDBusReply<QString> reply = iface.call("currentProfile");
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        qCritical() << err.name() << "Message:" << err.message();

        return QString();
    }

    return reply;
}
