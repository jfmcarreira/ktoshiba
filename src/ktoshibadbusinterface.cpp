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

#include <KDebug>
#include <KWindowSystem>

#include "ktoshibadbusinterface.h"
#include "ktoshibadbusadaptor.h"
#include "helperactions.h"

KToshibaDBusInterface::KToshibaDBusInterface(QObject *parent)
    : QObject( parent ),
      m_helper( new HelperActions( this ) )
{
    new KToshibaDBusAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService("net.sourceforge.KToshiba");
    dbus.registerObject("/net/sourceforge/KToshiba", this);
}

void KToshibaDBusInterface::toggleTouchPad()
{
    if (m_helper->isTouchPadSupported)
        m_helper->toggleTouchPad();
}

bool KToshibaDBusInterface::getECOLed()
{
    if (m_helper->isECOSupported)
        return m_helper->getEcoLed();

    return false;
}

void KToshibaDBusInterface::setECOLed(bool enabled)
{
    if (m_helper->isECOSupported)
        m_helper->setEcoLed(enabled);
}

bool KToshibaDBusInterface::getIllumination()
{
    if (m_helper->isIlluminationSupported)
        return m_helper->getIllumination();

    return false;
}

void KToshibaDBusInterface::setIllumination(bool enabled)
{
    if (m_helper->isIlluminationSupported)
        m_helper->setIllumination(enabled);
}

int KToshibaDBusInterface::getKBDMode()
{
    if (!m_helper->isKBDBacklightSupported)
        return 0;

    return m_helper->getKBDMode();
}

void KToshibaDBusInterface::changeKBDMode()
{
    if (!m_helper->isKBDBacklightSupported)
        return;

    int mode = m_helper->getKBDMode();

    m_helper->setKBDMode((mode == 1) ? 2 : 1);
}

int KToshibaDBusInterface::getKBDTimeout()
{
    if (!m_helper->isKBDBacklightSupported)
        return -1;

    return (m_helper->getKBDMode() == 1) ? -1 : m_helper->getKBDTimeout();
}

void KToshibaDBusInterface::setKBDTimeout(int time)
{
    if (!m_helper->isKBDBacklightSupported)
        return;

    m_helper->setKBDTimeout(time);
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
                 << "Message: " << err.message();
        return;
    }

    QDBusReply<void> reply = iface.call("Lock");
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message();
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
        kError() << err.name() << endl
                 << "Message: " << err.message();
        return;
    }

    QDBusReply<int> bright = iface.call("brightness");
    if (!bright.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message();
        return;
    }

    QDBusReply<void> reply = iface.call("setBrightness", level);
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message();
    }
}

void KToshibaDBusInterface::setKBDBacklight(bool state)
{
    QDBusInterface iface("org.kde.Solid.PowerManagement",
			 "/org/kde/Solid/PowerManagement/Actions/KeyboardBrightnessControl",
			 "org.kde.Solid.PowerManagement.Actions.KeyboardBrightnessControl",
			 QDBusConnection::sessionBus(), this);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message();
        return;
    }

    QDBusReply<void> reply;
    reply = iface.call("keyboardBrightness", state ? 1 : 0);
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message();
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
                 << "Message: " << err.message();
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
                 << "Message: " << err.message();
    }
}


#include "ktoshibadbusinterface.moc"
