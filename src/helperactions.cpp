/*
   Copyright (C) 2014-2015  Azael Avalos <coproscefalo@gmail.com>

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

#include <QStringList>
#include <QTextStream>

#include <KAuth/Action>
#include <KDebug>

#include "helperactions.h"

#define HELPER_ID "net.sourceforge.ktoshiba.ktoshhelper"

HelperActions::HelperActions(QObject *parent)
    : QObject( parent )
{
    isTouchPadSupported = false;
    isIlluminationSupported = false;
    isECOSupported = false;
    isKBDBacklightSupported = false;
    isKBDTypeSupported = false;
    isUSBSleepChargeSupported = false;
    isUSBRapidChargeSupported = false;
    isUSBSleepMusicSupported = false;
    isKBDFunctionsSupported = false;
    isPanelPowerONSupported = false;
    isUSBThreeSupported = false;
    isHAPSSupported = false;
}

bool HelperActions::init()
{
    m_driverPath = findDriverPath();
    if (m_driverPath.isEmpty())
        return false;

    m_ledsPath = "/sys/class/leds/toshiba::";
    m_hapsPath = "/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS620A:00/";

    isTouchPadSupported = checkTouchPad();
    isIlluminationSupported = checkIllumination();
    isECOSupported = checkECO();
    isKBDBacklightSupported = checkKBDBacklight();
    isKBDTypeSupported = checkKBDType();
    isUSBSleepChargeSupported = checkUSBSleepCharge();
    isUSBRapidChargeSupported = checkUSBRapidCharge();
    isUSBSleepMusicSupported = checkUSBSleepMusic();
    isKBDFunctionsSupported = checkKBDFunctions();
    isPanelPowerONSupported = checkPanelPowerON();
    isUSBThreeSupported = checkUSBThree();
    isHAPSSupported = checkHAPS();

    return true;
}

QString HelperActions::findDriverPath()
{
    QStringList m_devices;
    m_devices << "TOS1900:00" << "TOS6200:00" << "TOS6207:00" << "TOS6208:00";
    QString path("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/%1/path");
    for (int current = m_devices.indexOf(m_devices.first()); current <= m_devices.indexOf(m_devices.last());) {
        m_file.setFileName(path.arg(m_devices.at(current)));
        if (m_file.exists())
            return QString("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/%1/").arg(m_devices.at(current));

        current++;
    }

    kWarning() << "No known kernel interface found" << endl;

    return QString("");
}

bool HelperActions::deviceExists(QString device)
{
    if (device == "touchpad" || device == "kbd_backlight_mode" || device == "kbd_type" ||
        device == "usb_sleep_charge" || device == "sleep_functions_on_battery" ||
        device == "usb_rapid_charge" || device == "usb_sleep_music" || device == "kbd_function_keys" ||
        device == "panel_power_on" || device == "usb_three") {
        m_file.setFileName(m_driverPath + device);
    } else if (device == "illumination" || device == "eco_mode") {
        m_file.setFileName(m_ledsPath + device + "/brightness");
    } else if (device == "haps") {
        m_file.setFileName(m_hapsPath + "protection_level");
    } else {
        kError() << "Invalid device name" << "(" << device << ")";

        return false;
    }

    return m_file.exists();
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

bool HelperActions::checkUSBSleepCharge()
{
    return deviceExists("usb_sleep_charge");
}

bool HelperActions::checkUSBRapidCharge()
{
    return deviceExists("usb_rapid_charge");
}

bool HelperActions::checkUSBSleepMusic()
{
    return deviceExists("usb_sleep_music");
}

bool HelperActions::checkKBDFunctions()
{
    return deviceExists("kbd_function_keys");
}

bool HelperActions::checkPanelPowerON()
{
    return deviceExists("panel_power_on");
}

bool HelperActions::checkUSBThree()
{
    return deviceExists("usb_three");
}

bool HelperActions::checkHAPS()
{
    return deviceExists("haps");
}

void HelperActions::getSysInfo()
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.dumpsysinfo");
    action.setHelperID(HELPER_ID);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.dumpsysinfo failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";

        return;
    }

    m_file.setFileName("/var/tmp/dmidecode");
    if (!m_file.open(QIODevice::ReadOnly)) {
        kError() << "getSysInfo failed" << endl
                 << m_file.errorString() << "(" << m_file.error() << ")";

        return;
    }

    QTextStream in(&m_file);
    int count = 0;
    do {
        QString line = in.readLine();
        QStringList splited = line.split(":");
        // BIOS Information
        if (splited[0].contains("Vendor")) {
            sysinfo << splited[1];
            continue;
        }
        if (splited[0].contains("Version")) {
            sysinfo << splited[1];
            count++;
            if (count == 2)
               break;
            else
               continue;
        }
        if (splited[0].contains("Release Date")) {
            sysinfo << splited[1];
            continue;
        }
        if (splited[0].contains("Firmware Revision")) {
            sysinfo << splited[1];
            continue;
        }
        // System Information
        if (splited[0].contains("Product Name")) {
            sysinfo << splited[1];
            continue;
        }
    } while (!in.atEnd());
    m_file.close();
}

int HelperActions::getTouchPad()
{
    m_file.setFileName(m_driverPath + "touchpad");
    if (!m_file.open(QIODevice::ReadOnly)) {
        kError() << "getTouchpad failed" << endl
                 << m_file.errorString() << "(" << m_file.error() << ")";

        return -1;
    }

    QTextStream stream(&m_file);
    int state = stream.readAll().toInt();
    m_file.close();

    return state;
}

void HelperActions::setTouchPad(int state)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.settouchpad");
    action.setHelperID(HELPER_ID);
    action.addArgument("state", state);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.settouchpad failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }

    emit touchpadToggled(state);
}

int HelperActions::getIllumination()
{
    m_file.setFileName(m_ledsPath + "illumination/brightness");
    if (!m_file.open(QIODevice::ReadOnly)) {
        kError() << "getIllumination failed" << endl
                 << m_file.errorString() << "(" << m_file.error() << ")";

        return -1;
    }

    QTextStream stream(&m_file);
    int state = stream.readAll().toInt();
    m_file.close();

    return state;
}

void HelperActions::setIllumination(int state)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.setillumination");
    action.setHelperID(HELPER_ID);
    action.addArgument("state", state);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.setillumination failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }
}

int HelperActions::getEcoLed()
{
    m_file.setFileName(m_ledsPath + "eco_mode/brightness");
    if (!m_file.open(QIODevice::ReadOnly)) {
        kError() << "getEcoLed failed" << endl
                 << m_file.errorString() << "(" << m_file.error() << ")";

        return -1;
    }

    QTextStream stream(&m_file);
    int state = stream.readAll().toInt();
    m_file.close();

    return state;
}

void HelperActions::setEcoLed(int state)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.seteco");
    action.setHelperID(HELPER_ID);
    action.addArgument("state", state);
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

    emit kbdModeChanged();
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

int HelperActions::getUSBSleepCharge()
{
    m_file.setFileName(m_driverPath + "usb_sleep_charge");
    if (!m_file.open(QIODevice::ReadOnly)) {
        kError() << "getUSBSleepCharge failed" << endl
                 << m_file.errorString() << "(" << m_file.error() << ")";

        return -1;
    }

    QTextStream stream(&m_file);
    int mode = stream.readAll().toInt();
    m_file.close();

    return mode;
}

void HelperActions::setUSBSleepCharge(int mode)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.setusbsleepcharge");
    action.setHelperID(HELPER_ID);
    action.addArgument("mode", mode);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.setusbsleepcharge failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }
}

QStringList HelperActions::getUSBSleepFunctionsBatLvl()
{
    m_file.setFileName(m_driverPath + "sleep_functions_on_battery");
    if (!m_file.open(QIODevice::ReadOnly)) {
        kError() << "getUSBSleepFunctionsBatLvl failed" << endl
                 << m_file.errorString() << "(" << m_file.error() << ")";

        return QStringList();
    }

    QTextStream stream(&m_file);
    QString line = stream.readAll();
    m_file.close();

    return line.split(" ");
}

void HelperActions::setUSBSleepFunctionsBatLvl(int level)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.setusbsleepfunctionsbatlvl");
    action.setHelperID(HELPER_ID);
    action.addArgument("level", level);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.setusbsleepfunctionsbatlvl failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }
}

int HelperActions::getUSBRapidCharge()
{
    m_file.setFileName(m_driverPath + "usb_rapid_charge");
    if (!m_file.open(QIODevice::ReadOnly)) {
        kError() << "getUSBRapidCharge failed" << endl
                 << m_file.errorString() << "(" << m_file.error() << ")";

        return -1;
    }

    QTextStream stream(&m_file);
    int state = stream.readAll().toInt();
    m_file.close();

    return state;
}

void HelperActions::setUSBRapidCharge(int state)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.setusbrapidcharge");
    action.setHelperID(HELPER_ID);
    action.addArgument("state", state);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.setusbrapidcharge failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }
}

int HelperActions::getUSBSleepMusic()
{
    m_file.setFileName(m_driverPath + "usb_sleep_music");
    if (!m_file.open(QIODevice::ReadOnly)) {
        kError() << "getUSBSleepMusic failed" << endl
                 << m_file.errorString() << "(" << m_file.error() << ")";

        return -1;
    }

    QTextStream stream(&m_file);
    int state = stream.readAll().toInt();
    m_file.close();

    return state;
}

void HelperActions::setUSBSleepMusic(int state)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.setusbsleepmusic");
    action.setHelperID(HELPER_ID);
    action.addArgument("state", state);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.setusbsleepmusic failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }
}

int HelperActions::getKBDFunctions()
{
    m_file.setFileName(m_driverPath + "kbd_function_keys");
    if (!m_file.open(QIODevice::ReadOnly)) {
        kError() << "getKBDFunctions failed" << endl
                 << m_file.errorString() << "(" << m_file.error() << ")";

        return -1;
    }

    QTextStream stream(&m_file);
    int mode = stream.readAll().toInt();
    m_file.close();

    return mode;
}

void HelperActions::setKBDFunctions(int mode)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.setkbdfunctions");
    action.setHelperID(HELPER_ID);
    action.addArgument("mode", mode);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.setkbdfunctions failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }
}

int HelperActions::getPanelPowerON()
{
    m_file.setFileName(m_driverPath + "panel_power_on");
    if (!m_file.open(QIODevice::ReadOnly)) {
        kError() << "getPanelPowerON failed" << endl
                 << m_file.errorString() << "(" << m_file.error() << ")";

        return -1;
    }

    QTextStream stream(&m_file);
    int state = stream.readAll().toInt();
    m_file.close();

    return state;
}

void HelperActions::setPanelPowerON(int state)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.setpanelpoweron");
    action.setHelperID(HELPER_ID);
    action.addArgument("state", state);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.setpanelpoweron failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }
}

int HelperActions::getUSBThree()
{
    m_file.setFileName(m_driverPath + "usb_three");
    if (!m_file.open(QIODevice::ReadOnly)) {
        kError() << "getUSBThree failed" << endl
                 << m_file.errorString() << "(" << m_file.error() << ")";

        return -1;
    }

    QTextStream stream(&m_file);
    int mode = stream.readAll().toInt();
    m_file.close();

    return mode;
}

void HelperActions::setUSBThree(int mode)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.setusbthree");
    action.setHelperID(HELPER_ID);
    action.addArgument("mode", mode);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.setusbthree failed" << endl
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
