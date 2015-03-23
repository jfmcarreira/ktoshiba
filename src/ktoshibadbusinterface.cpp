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

#include <QtDBus>

#include <KDebug>
#include <KWindowSystem>

#include "ktoshibadbusinterface.h"
#include "ktoshibadbusadaptor.h"
#include "fnactions.h"
#include "helperactions.h"

KToshibaDBusInterface::KToshibaDBusInterface(FnActions *parent)
    : QObject( parent ),
      m_service( false ),
      m_object( false )
{
    m_fn = qobject_cast<FnActions *>(parent);
}

void KToshibaDBusInterface::init()
{
    new KToshibaDBusAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    m_service = dbus.registerService("net.sourceforge.KToshiba");
    if (!m_service)
        kError() << "Could not register DBus service";

    m_object = dbus.registerObject("/net/sourceforge/KToshiba", this);
    if (!m_object)
        kError() << "Could not register DBus object";
}

KToshibaDBusInterface::~KToshibaDBusInterface()
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    if (m_object)
        dbus.unregisterObject("/net/sourceforge/KToshiba");

    if (m_service)
        if (!dbus.unregisterService("net.sourceforge.KToshiba"))
            kError() << "Could not unregister DBus service";
}

int KToshibaDBusInterface::getTouchPad()
{
    if (m_fn->m_helper->isTouchPadSupported)
        return m_fn->m_helper->getTouchPad();

    return -1;
}

void KToshibaDBusInterface::setTouchPad(int state)
{
    if (m_fn->m_helper->isTouchPadSupported)
        m_fn->m_helper->setTouchPad(state);
}

int KToshibaDBusInterface::getECOLed()
{
    if (m_fn->m_helper->isECOSupported)
        return m_fn->m_helper->getEcoLed();

    return -1;
}

void KToshibaDBusInterface::setECOLed(int state)
{
    if (m_fn->m_helper->isECOSupported)
        m_fn->m_helper->setEcoLed(state);
}

int KToshibaDBusInterface::getIllumination()
{
    if (m_fn->m_helper->isIlluminationSupported)
        return m_fn->m_helper->getIllumination();

    return -1;
}

void KToshibaDBusInterface::setIllumination(int state)
{
    if (m_fn->m_helper->isIlluminationSupported)
        m_fn->m_helper->setIllumination(state);
}

int KToshibaDBusInterface::getKBDType()
{
    if (m_fn->m_helper->isKBDTypeSupported)
        return m_fn->m_helper->getKBDType();

    return -1;
}

int KToshibaDBusInterface::getKBDMode()
{
    if (m_fn->m_helper->isKBDBacklightSupported)
        return m_fn->m_helper->getKBDMode();

    return -1;
}

void KToshibaDBusInterface::setKBDMode(int mode)
{
    if (m_fn->m_helper->isKBDBacklightSupported)
        m_fn->m_helper->setKBDMode(mode);
}

int KToshibaDBusInterface::getKBDTimeout()
{
    if (m_fn->m_helper->isKBDBacklightSupported)
        return (getKBDMode() == FnActions::TIMER) ? m_fn->m_helper->getKBDTimeout() : 0;

    return -1;
}

void KToshibaDBusInterface::setKBDTimeout(int time)
{
    if (m_fn->m_helper->isKBDBacklightSupported)
        m_fn->m_helper->setKBDTimeout(time);
}

int KToshibaDBusInterface::getUSBSleepCharge()
{
    if (m_fn->m_helper->isUSBSleepChargeSupported)
        return m_fn->m_helper->getUSBSleepCharge();

    return -1;
}

void KToshibaDBusInterface::setUSBSleepCharge(int mode)
{
    if (m_fn->m_helper->isUSBSleepChargeSupported)
        m_fn->m_helper->setUSBSleepCharge(mode);
}

int KToshibaDBusInterface::getUSBSleepFunctionsBatLvl()
{
    if (m_fn->m_helper->isUSBSleepChargeSupported) {
        QStringList values = m_fn->m_helper->getUSBSleepFunctionsBatLvl();
        return values[1].toInt();
    }

    return -1;
}

void KToshibaDBusInterface::setUSBSleepFunctionsBatLvl(int level)
{
    if (m_fn->m_helper->isUSBSleepChargeSupported)
        m_fn->m_helper->setUSBSleepFunctionsBatLvl(level);
}

int KToshibaDBusInterface::getUSBRapidCharge()
{
    if (m_fn->m_helper->isUSBRapidChargeSupported)
        return m_fn->m_helper->getUSBRapidCharge();

    return -1;
}

void KToshibaDBusInterface::setUSBRapidCharge(int state)
{
    if (m_fn->m_helper->isUSBSleepChargeSupported)
        m_fn->m_helper->setUSBRapidCharge(state);
}

int KToshibaDBusInterface::getUSBSleepMusic()
{
    if (m_fn->m_helper->isUSBSleepMusicSupported)
        return m_fn->m_helper->getUSBSleepMusic();

    return -1;
}

void KToshibaDBusInterface::setUSBSleepMusic(int state)
{
    if (m_fn->m_helper->isUSBSleepChargeSupported)
        m_fn->m_helper->setUSBSleepMusic(state);
}

int KToshibaDBusInterface::getKBDFunctions()
{
    if (m_fn->m_helper->isKBDFunctionsSupported)
        return m_fn->m_helper->getKBDFunctions();

    return -1;
}

void KToshibaDBusInterface::setKBDFunctions(int mode)
{
    if (m_fn->m_helper->isKBDFunctionsSupported)
        m_fn->m_helper->setKBDFunctions(mode);
}

int KToshibaDBusInterface::getPanelPowerON()
{
    if (m_fn->m_helper->isPanelPowerONSupported)
        return m_fn->m_helper->getPanelPowerON();

    return -1;
}

void KToshibaDBusInterface::setPanelPowerON(int state)
{
    if (m_fn->m_helper->isPanelPowerONSupported)
        m_fn->m_helper->setPanelPowerON(state);
}

int KToshibaDBusInterface::getUSBThree()
{
    if (m_fn->m_helper->isUSBThreeSupported)
        return m_fn->m_helper->getUSBThree();

    return -1;
}

void KToshibaDBusInterface::setUSBThree(int mode)
{
    if (m_fn->m_helper->isUSBThreeSupported)
        m_fn->m_helper->setUSBThree(mode);
}

int KToshibaDBusInterface::getProtectionLevel()
{
    if (m_fn->m_helper->isHAPSSupported)
        return m_fn->m_helper->getProtectionLevel();

    return -1;
}

void KToshibaDBusInterface::setProtectionLevel(int level)
{
    if (m_fn->m_helper->isHAPSSupported)
        m_fn->m_helper->setProtectionLevel(level);
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
                 << "Message:" << err.message();

        return;
    }

    QDBusReply<void> reply = iface.call("Lock");
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message:" << err.message();
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
                 << "Message:" << err.message();

        return;
    }

    QDBusReply<void> reply = iface.call("setBrightness", level);
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message:" << err.message();
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
        kError() << err.name() << endl
                 << "Message:" << err.message();

        return;
    }

    QDBusReply<void> reply;
    reply = iface.call("keyboardBrightness", state);
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message:" << err.message();
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
                 << "Message:" << err.message();

        return;
    }

    QDBusReply<void> reply;
    switch(zoom) {
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
        kError() << err.name() << endl
                 << "Message:" << err.message();
    }
}


#include "ktoshibadbusinterface.moc"
