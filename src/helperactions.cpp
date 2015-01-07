/*
   Copyright (C) 2014-2015  Azael Avalos <coproscefalo@gmail.com>

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

#include <QtCore/QStringList>
#include <QtCore/QTextStream>

#include <KAuth/Action>
#include <KDebug>

#include "helperactions.h"

#define HELPER_ID "net.sourceforge.ktoshiba.ktoshhelper"

HelperActions::HelperActions(QObject *parent)
    : QObject( parent )
{
    m_driverPath = findDriverPath();
    if (m_driverPath.isEmpty())
        exit(-1);

    m_ledsPath = "/sys/class/leds/toshiba::";
    m_hapsPath = "/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS620A:00/";

    isTouchPadSupported = checkTouchPad();
    isIlluminationSupported = checkIllumination();
    isECOSupported = checkECO();
    isKBDBacklightSupported = checkKBDBacklight();
    isKBDTypeSupported = checkKBDType();
    isHAPSSupported = checkHAPS();
}

QString HelperActions::findDriverPath()
{
    QStringList m_devices;
    m_devices << "TOS1900:00" << "TOS6200:00" << "TOS6208:00";
    QString path("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/%1/path");
    for (int current = m_devices.indexOf(m_devices.first()); current <= m_devices.indexOf(m_devices.last());) {
        m_file.setFileName(path.arg(m_devices.at(current)));
        if (m_file.open(QIODevice::ReadOnly)) {
            m_file.close();
            return QString("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/%1/").arg(m_devices.at(current));
        }

        current++;
    }

    kWarning() << "No known interface found" << endl;

    return QString("");
}

bool HelperActions::deviceExists(QString device)
{
    if (device == "touchpad" || device == "kbd_backlight_mode" || device == "kbd_type") {
        m_file.setFileName(m_driverPath + device);
    } else if (device == "illumination" || device == "eco_mode") {
        m_file.setFileName(m_ledsPath + device + "/brightness");
    } else if (device == "haps") {
        m_file.setFileName(m_hapsPath + "protection_level");
    } else {
        kError() << "Invalid device name" << "(" << device << ")";

        return false;
    }

    if (!m_file.exists()) {
        kDebug() << "The specified device does not exist" << "(" << device << ")";

        return false;
    }

    return true;
}

bool HelperActions::checkTouchPad()
{
    return deviceExists("touchpad");
}

bool HelperActions::checkIllumination()
{
    return deviceExists("illumination");
}

bool HelperActions::checkECO()
{
    return deviceExists("eco_mode");
}

bool HelperActions::checkKBDBacklight()
{
    return deviceExists("kbd_backlight_mode");
}

bool HelperActions::checkKBDType()
{
    return deviceExists("kbd_type");
}

bool HelperActions::checkHAPS()
{
    return deviceExists("haps");
}

void HelperActions::toggleTouchPad()
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.toggletouchpad");
    action.setHelperID(HELPER_ID);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.toggletouchpad failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }
}

bool HelperActions::getIllumination()
{
    m_file.setFileName(m_ledsPath + "illumination/brightness");
    if (!m_file.open(QIODevice::ReadOnly)) {
        kError() << "getIllumination failed" << endl
                 << m_file.errorString() << "(" << m_file.error() << ")";

        return false;
    }

    QTextStream stream(&m_file);
    int state = stream.readAll().toInt();
    m_file.close();

    return state ? true : false;
}

void HelperActions::setIllumination(bool enabled)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.setillumination");
    action.setHelperID(HELPER_ID);
    action.addArgument("state", (enabled ? 1 : 0));
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.setillumination failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }
}

bool HelperActions::getEcoLed()
{
    m_file.setFileName(m_ledsPath + "eco_mode/brightness");
    if (!m_file.open(QIODevice::ReadOnly)) {
        kError() << "getEcoLed failed" << endl
                 << m_file.errorString() << "(" << m_file.error() << ")";

        return false;
    }

    QTextStream stream(&m_file);
    int state = stream.readAll().toInt();
    m_file.close();

    return state ? true : false;
}

void HelperActions::setEcoLed(bool enabled)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.seteco");
    action.setHelperID(HELPER_ID);
    action.addArgument("state", (enabled ? 1 : 0));
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.seteco failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }
}

int HelperActions::getKBDType()
{
    m_file.setFileName(m_driverPath + "kbd_type");
    if (!m_file.open(QIODevice::ReadOnly)) {
        kError() << "getKBDType failed" << endl
                 << m_file.errorString() << "(" << m_file.error() << ")";

        return -1;
    }

    QTextStream stream(&m_file);
    int type = stream.readAll().toInt();
    m_file.close();

    return type;
}

int HelperActions::getKBDMode()
{
    m_file.setFileName(m_driverPath + "kbd_backlight_mode");
    if (!m_file.open(QIODevice::ReadOnly)) {
        kError() << "getKBDMode failed" << endl
                 << m_file.errorString() << "(" << m_file.error() << ")";

        return -1;
    }

    QTextStream stream(&m_file);
    int mode = stream.readAll().toInt();
    m_file.close();

    return mode;
}

void HelperActions::setKBDMode(int mode)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.setkbdmode");
    action.setHelperID(HELPER_ID);
    action.addArgument("mode", mode);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.setkbdmode failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";

        return;
    }

    emit kbdModeChanged(mode);
}

int HelperActions::getKBDTimeout()
{
    m_file.setFileName(m_driverPath + "kbd_backlight_timeout");
    if (!m_file.open(QIODevice::ReadOnly)) {
        kError() << "getKBDTimeout failed" << endl
                 << m_file.errorString() << "(" << m_file.error() << ")";

        return -1;
    }

    QTextStream stream(&m_file);
    int timeout = stream.readAll().toInt();
    m_file.close();

    return timeout;
}

void HelperActions::setKBDTimeout(int time)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.setkbdtimeout");
    action.setHelperID(HELPER_ID);
    action.addArgument("time", time);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.setkbdtimeout failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }
}

int HelperActions::getProtectionLevel()
{
    m_file.setFileName("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS620A:00/protection_level");
    if (!m_file.open(QIODevice::ReadOnly)) {
        kError() << "getProtectionLevel failed" << endl
                 << m_file.errorString() << "(" << m_file.error() << ")";

        return -1;
    }

    QTextStream stream(&m_file);
    int level = stream.readAll().toInt();
    m_file.close();

    return level;
}

void HelperActions::setProtectionLevel(int level)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.setprotectionlevel");
    action.setHelperID(HELPER_ID);
    action.addArgument("level", level);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.setprotectionlevel failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }
}

void HelperActions::unloadHeads(int timeout)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.unloadheads");
    action.setHelperID(HELPER_ID);
    action.addArgument("timeout", timeout);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.unloadheads failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }
}


#include "helperactions.moc"
