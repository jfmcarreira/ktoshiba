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

KToshibaDBusInterface::KToshibaDBusInterface(QObject *parent)
    : QObject(parent),
      m_dbus(QDBusConnection::sessionBus()),
      m_service(false),
      m_object(false)
{
    m_dbus.connect("org.kde.KWin", "/Compositor", "org.kde.kwin.Compositing",
                   "compositingToggled", QObject::parent(), SLOT(compositingChanged(bool)));
    m_dbus.connect("org.kde.Solid.PowerManagement", "/org/kde/Solid/PowerManagement",
                   "org.kde.Solid.PowerManagement", "profileChanged",
                   QObject::parent(), SLOT(updateBatteryProfile(QString)));
}

KToshibaDBusInterface::~KToshibaDBusInterface()
{
    if (m_object)
        m_dbus.unregisterObject("/Config");

    if (m_service)
        if (!m_dbus.unregisterService("net.sourceforge.KToshiba"))
            qCritical() << "Could not unregister DBus service";
}

void KToshibaDBusInterface::init()
{
    new KToshibaDBusAdaptor(this);

    m_object = m_dbus.registerObject("/Config", this);
    if (!m_object)
        qCritical() << "Could not register DBus object";

    m_service = m_dbus.registerService("net.sourceforge.KToshiba");
    if (!m_service)
        qCritical() << "Could not register DBus service";
}

void KToshibaDBusInterface::reloadConfigFile()
{
    emit configFileChanged();
}

void KToshibaDBusInterface::lockScreen()
{
    QDBusInterface iface("org.freedesktop.ScreenSaver",
                         "/ScreenSaver",
                         "org.freedesktop.ScreenSaver",
                         m_dbus, this);
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
                         m_dbus, this);
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
                         m_dbus, this);
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

void KToshibaDBusInterface::setZoom(ZoomActions zoom)
{
    if (!getCompositingState()) {
        qWarning() << "Compositing have been disabled, Zoom actions cannot be activated";

        return;
    }

    QDBusInterface iface("org.kde.kglobalaccel",
                         "/component/kwin",
                         "org.kde.kglobalaccel.Component",
                         m_dbus, this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        qCritical() << err.name() << "Message:" << err.message();

        return;
    }

    QDBusReply<void> reply;
    switch (zoom) {
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
        qCritical() << err.name() << "Message:" << err.message();
    }
}

uint KToshibaDBusInterface::inhibitPowerManagement(QString reason)
{
    QDBusInterface iface("org.freedesktop.PowerManagement.Inhibit",
                         "/org/freedesktop/PowerManagement/Inhibit",
                         "org.freedesktop.PowerManagement.Inhibit",
                         m_dbus, this);
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
                         m_dbus, this);
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
                         m_dbus, this);
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
                         m_dbus, this);
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
