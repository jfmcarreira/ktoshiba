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

#include <KWindowSystem>

#include "ktoshibadbusinterface.h"
#include "ktoshibadbusadaptor.h"
#include "fnactions.h"
#include "ktoshibahardware.h"

KToshibaDBusInterface::KToshibaDBusInterface(FnActions *parent)
    : QObject( parent ),
      m_service( false ),
      m_object( false )
{
    m_fn = qobject_cast<FnActions *>(parent);
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
}

void KToshibaDBusInterface::configFileChanged()
{
    emit configChanged();
}

int KToshibaDBusInterface::getTouchPad()
{
    if (m_fn->hw()->isTouchPadSupported)
        return m_fn->hw()->getTouchPad();

    return -1;
}

void KToshibaDBusInterface::setTouchPad(int state)
{
    if (m_fn->hw()->isTouchPadSupported)
        m_fn->hw()->setTouchPad(state);
}

int KToshibaDBusInterface::getECOLed()
{
    if (m_fn->hw()->isECOSupported)
        return m_fn->hw()->getEcoLed();

    return -1;
}

void KToshibaDBusInterface::setECOLed(int state)
{
    if (m_fn->hw()->isECOSupported)
        m_fn->hw()->setEcoLed(state);
}

int KToshibaDBusInterface::getIllumination()
{
    if (m_fn->hw()->isIlluminationSupported)
        return m_fn->hw()->getIllumination();

    return -1;
}

void KToshibaDBusInterface::setIllumination(int state)
{
    if (m_fn->hw()->isIlluminationSupported)
        m_fn->hw()->setIllumination(state);
}

int KToshibaDBusInterface::getKBDType()
{
    if (m_fn->hw()->isKBDTypeSupported)
        return m_fn->hw()->getKBDType();

    return -1;
}

int KToshibaDBusInterface::getKBDMode()
{
    if (m_fn->hw()->isKBDBacklightSupported)
        return m_fn->hw()->getKBDMode();

    return -1;
}

void KToshibaDBusInterface::setKBDMode(int mode)
{
    if (m_fn->hw()->isKBDBacklightSupported)
        m_fn->hw()->setKBDMode(mode);
}

int KToshibaDBusInterface::getKBDTimeout()
{
    if (m_fn->hw()->isKBDBacklightSupported)
        return (getKBDMode() == FnActions::TIMER) ? m_fn->hw()->getKBDTimeout() : 0;

    return -1;
}

void KToshibaDBusInterface::setKBDTimeout(int time)
{
    if (m_fn->hw()->isKBDBacklightSupported)
        m_fn->hw()->setKBDTimeout(time);
}

int KToshibaDBusInterface::getUSBSleepCharge()
{
    if (m_fn->hw()->isUSBSleepChargeSupported)
        return m_fn->hw()->getUSBSleepCharge();

    return -1;
}

void KToshibaDBusInterface::setUSBSleepCharge(int mode)
{
    if (m_fn->hw()->isUSBSleepChargeSupported)
        m_fn->hw()->setUSBSleepCharge(mode);
}

int KToshibaDBusInterface::getUSBSleepFunctionsBatState()
{
    if (m_fn->hw()->isUSBSleepChargeSupported) {
        QStringList values = m_fn->hw()->getUSBSleepFunctionsBatLvl();

        return values[0].toInt();
    }

    return -1;
}

void KToshibaDBusInterface::setUSBSleepFunctionsBatState(int state)
{
    if (m_fn->hw()->isUSBSleepChargeSupported)
        m_fn->hw()->setUSBSleepFunctionsBatLvl(state);
}

int KToshibaDBusInterface::getUSBSleepFunctionsBatLvl()
{
    if (m_fn->hw()->isUSBSleepChargeSupported) {
        QStringList values = m_fn->hw()->getUSBSleepFunctionsBatLvl();

        return values[1].toInt();
    }

    return -1;
}

void KToshibaDBusInterface::setUSBSleepFunctionsBatLvl(int level)
{
    if (m_fn->hw()->isUSBSleepChargeSupported)
        m_fn->hw()->setUSBSleepFunctionsBatLvl(level);
}

int KToshibaDBusInterface::getUSBRapidCharge()
{
    if (m_fn->hw()->isUSBRapidChargeSupported)
        return m_fn->hw()->getUSBRapidCharge();

    return -1;
}

void KToshibaDBusInterface::setUSBRapidCharge(int state)
{
    if (m_fn->hw()->isUSBSleepChargeSupported)
        m_fn->hw()->setUSBRapidCharge(state);
}

int KToshibaDBusInterface::getUSBSleepMusic()
{
    if (m_fn->hw()->isUSBSleepMusicSupported)
        return m_fn->hw()->getUSBSleepMusic();

    return -1;
}

void KToshibaDBusInterface::setUSBSleepMusic(int state)
{
    if (m_fn->hw()->isUSBSleepChargeSupported)
        m_fn->hw()->setUSBSleepMusic(state);
}

int KToshibaDBusInterface::getKBDFunctions()
{
    if (m_fn->hw()->isKBDFunctionsSupported)
        return m_fn->hw()->getKBDFunctions();

    return -1;
}

void KToshibaDBusInterface::setKBDFunctions(int mode)
{
    if (m_fn->hw()->isKBDFunctionsSupported)
        m_fn->hw()->setKBDFunctions(mode);
}

int KToshibaDBusInterface::getPanelPowerON()
{
    if (m_fn->hw()->isPanelPowerONSupported)
        return m_fn->hw()->getPanelPowerON();

    return -1;
}

void KToshibaDBusInterface::setPanelPowerON(int state)
{
    if (m_fn->hw()->isPanelPowerONSupported)
        m_fn->hw()->setPanelPowerON(state);
}

int KToshibaDBusInterface::getUSBThree()
{
    if (m_fn->hw()->isUSBThreeSupported)
        return m_fn->hw()->getUSBThree();

    return -1;
}

void KToshibaDBusInterface::setUSBThree(int mode)
{
    if (m_fn->hw()->isUSBThreeSupported)
        m_fn->hw()->setUSBThree(mode);
}

int KToshibaDBusInterface::getProtectionLevel()
{
    if (m_fn->hw()->isHAPSSupported)
        return m_fn->hw()->getProtectionLevel();

    return -1;
}

void KToshibaDBusInterface::setProtectionLevel(int level)
{
    if (m_fn->hw()->isHAPSSupported)
        m_fn->hw()->setProtectionLevel(level);
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

    QDBusReply<void> reply;
    reply = iface.call("keyboardBrightness", state);
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        qCritical() << err.name() << "Message:" << err.message();
    }
}

void KToshibaDBusInterface::setZoom(int zoom)
{
    if (!KWindowSystem::compositingActive()) {
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

    QDBusReply<uint> reply;
    reply = iface.call("Inhibit", QString("KToshiba"), reason);
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

    QDBusReply<void> reply;
    reply = iface.call("UnInhibit", cookie);
    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        qCritical() << err.name() << "Message:" << err.message();
    }
}


#include "ktoshibadbusinterface.moc"
