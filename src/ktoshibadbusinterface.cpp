/*
   Copyright (C) 2004-2016  Azael Avalos <coproscefalo@gmail.com>

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

#include "ktoshibadbusadaptor.h"
#include "ktoshibadbusinterface.h"
#include "ktoshiba_debug.h"

KToshibaDBusInterface::KToshibaDBusInterface(QObject *parent)
    : QObject(parent),
      m_dbus(QDBusConnection::sessionBus()),
      m_service(false),
      m_object(false)
{
    new KToshibaDBusAdaptor(this);

    m_object = m_dbus.registerObject(QStringLiteral("/Config"), this);
    if (!m_object) {
        qCCritical(KTOSHIBA) << "Could not register DBus object";
    }

    m_service = m_dbus.registerService(QStringLiteral("net.sourceforge.KToshiba"));
    if (!m_service) {
        qCCritical(KTOSHIBA) << "Could not register DBus service";
    }
}

KToshibaDBusInterface::~KToshibaDBusInterface()
{
    if (m_object) {
        m_dbus.unregisterObject(QStringLiteral("/Config"));
    }

    if (m_service) {
        if (!m_dbus.unregisterService(QStringLiteral("net.sourceforge.KToshiba"))) {
            qCCritical(KTOSHIBA) << "Could not unregister DBus service";
        }
    }
}

void KToshibaDBusInterface::reloadConfigFile()
{
    emit configFileChanged();
}

void KToshibaDBusInterface::lockScreen()
{
    QDBusInterface iface(QStringLiteral("org.freedesktop.ScreenSaver"),
                         QStringLiteral("/ScreenSaver"),
                         QStringLiteral("org.freedesktop.ScreenSaver"),
                         m_dbus, this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        qCCritical(KTOSHIBA) << err.name() << "Message:" << err.message();

        return;
    }

    QDBusReply<void> reply = iface.call(QStringLiteral("Lock"));
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        qCCritical(KTOSHIBA) << err.name() << "Message:" << err.message();
    }
}

void KToshibaDBusInterface::setBrightness(int level)
{
    QDBusInterface iface(QStringLiteral("org.kde.Solid.PowerManagement"),
                         QStringLiteral("/org/kde/Solid/PowerManagement/Actions/BrightnessControl"),
                         QStringLiteral("org.kde.Solid.PowerManagement.Actions.BrightnessControl"),
                         m_dbus, this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        qCCritical(KTOSHIBA) << err.name() << "Message:" << err.message();

        return;
    }

    QDBusReply<void> reply = iface.call(QStringLiteral("setBrightness"), level);
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        qCCritical(KTOSHIBA) << err.name() << "Message:" << err.message();
    }
}

void KToshibaDBusInterface::setKBDBacklight(int state)
{
    QDBusInterface iface(QStringLiteral("org.kde.Solid.PowerManagement"),
                         QStringLiteral("/org/kde/Solid/PowerManagement/Actions/KeyboardBrightnessControl"),
                         QStringLiteral("org.kde.Solid.PowerManagement.Actions.KeyboardBrightnessControl"),
                         m_dbus, this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        qCCritical(KTOSHIBA) << err.name() << "Message:" << err.message();

        return;
    }

    QDBusReply<void> reply = iface.call(QStringLiteral("setKeyboardBrightnessSilent"), state);
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        qCCritical(KTOSHIBA) << err.name() << "Message:" << err.message();
    }
}

void KToshibaDBusInterface::setZoom(ZoomActions zoom)
{
    if (!getCompositingState() || !isZoomEffectActive()) {
        emit zoomEffectDisabled();

        return;
    }

    QDBusInterface iface(QStringLiteral("org.kde.kglobalaccel"),
                         QStringLiteral("/component/kwin"),
                         QStringLiteral("org.kde.kglobalaccel.Component"),
                         m_dbus, this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        qCCritical(KTOSHIBA) << err.name() << "Message:" << err.message();

        return;
    }

    QDBusReply<void> reply;
    switch (zoom) {
    case ZoomReset:
        reply = iface.call(QStringLiteral("invokeShortcut"), QStringLiteral("view_actual_size"));
        break;
    case ZoomIn:
        reply = iface.call(QStringLiteral("invokeShortcut"), QStringLiteral("view_zoom_in"));
        break;
    case ZoomOut:
        reply = iface.call(QStringLiteral("invokeShortcut"), QStringLiteral("view_zoom_out"));
        break;
    }

    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        qCCritical(KTOSHIBA) << err.name() << "Message:" << err.message();
    }
}

void KToshibaDBusInterface::setPowerManagementInhibition(bool inhibit, QString reason, uint *cookie)
{
    QDBusInterface iface(QStringLiteral("org.freedesktop.PowerManagement.Inhibit"),
                         QStringLiteral("/org/freedesktop/PowerManagement/Inhibit"),
                         QStringLiteral("org.freedesktop.PowerManagement.Inhibit"),
                         m_dbus, this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        qCCritical(KTOSHIBA) << err.name() << "Message:" << err.message();

        return;
    }

    if (inhibit) {
        QDBusReply<uint> inhib = iface.call(QStringLiteral("Inhibit"), QStringLiteral("KToshiba"), reason);
        if (!inhib.isValid()) {
            QDBusError err(iface.lastError());
            qCCritical(KTOSHIBA) << err.name() << "Message:" << err.message();

            return;
        }

        *cookie = inhib;
    } else {
        QDBusReply<bool> has_inhib = iface.call(QStringLiteral("HasInhibit"));
        if (!has_inhib.isValid()) {
            QDBusError err(iface.lastError());
            qCCritical(KTOSHIBA) << err.name() << "Message:" << err.message();

            return;
        }

        if (!has_inhib) {
            return;
        }

        QDBusReply<void> uninhib = iface.call(QStringLiteral("UnInhibit"), *cookie);
        if (!uninhib.isValid()) {
            QDBusError err(iface.lastError());
            qCCritical(KTOSHIBA) << err.name() << "Message:" << err.message();
        }

        *cookie = 0;
    }
}

bool KToshibaDBusInterface::getCompositingState()
{
    QDBusInterface iface(QStringLiteral("org.kde.KWin"),
                         QStringLiteral("/Compositor"),
                         QStringLiteral("org.kde.kwin.Compositing"),
                         m_dbus, this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        qCCritical(KTOSHIBA) << err.name() << "Message:" << err.message();

        return false;
    }

    return iface.property("active").toBool();
}

QString KToshibaDBusInterface::getBatteryProfile()
{
    QDBusInterface iface(QStringLiteral("org.kde.Solid.PowerManagement"),
                         QStringLiteral("/org/kde/Solid/PowerManagement"),
                         QStringLiteral("org.kde.Solid.PowerManagement"),
                         m_dbus, this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        qCCritical(KTOSHIBA) << err.name() << "Message:" << err.message();

        return QString();
    }

    QDBusReply<QString> reply = iface.call(QStringLiteral("currentProfile"));
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        qCCritical(KTOSHIBA) << err.name() << "Message:" << err.message();

        return QString();
    }

    return reply;
}

bool KToshibaDBusInterface::isZoomEffectActive()
{
    QDBusInterface iface(QStringLiteral("org.kde.KWin"),
                         QStringLiteral("/Effects"),
                         QStringLiteral("org.kde.kwin.Effects"),
                         m_dbus, this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        qCCritical(KTOSHIBA) << err.name() << "Message:" << err.message();

        return false;
    }

    QDBusReply<bool> reply = iface.call(QStringLiteral("isEffectSupported"), QStringLiteral("zoom"));
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        qCCritical(KTOSHIBA) << err.name() << "Message:" << err.message();

        return false;
    }

    reply = iface.call(QStringLiteral("isEffectLoaded"), QStringLiteral("zoom"));
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        qCCritical(KTOSHIBA) << err.name() << "Message:" << err.message();

        return false;
    }

    return reply;
}
