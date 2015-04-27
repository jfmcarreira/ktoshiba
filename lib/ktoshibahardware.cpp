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

#include <QTextStream>
#include <QDebug>

#include <KAuth/KAuth>

#include "ktoshibahardware.h"

#define HELPER_ID "net.sourceforge.ktoshiba.ktoshhelper"

using namespace KAuth;

KToshibaHardware::KToshibaHardware(QObject *parent)
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

bool KToshibaHardware::init()
{
    m_driverPath = findDriverPath();
    if (m_driverPath.isEmpty())
        return false;

    m_ledsPath = "/sys/class/leds/toshiba::";
    m_hapsPath = "/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS620A:00/";

    isTouchPadSupported = deviceExists("touchpad");
    isIlluminationSupported = deviceExists("illumination");
    isECOSupported = deviceExists("eco_mode");
    isKBDBacklightSupported = deviceExists("kbd_backlight_mode");
    isKBDTypeSupported = deviceExists("kbd_type");
    isUSBSleepChargeSupported = deviceExists("usb_sleep_charge");
    isUSBRapidChargeSupported = deviceExists("usb_rapid_charge");
    isUSBSleepMusicSupported = deviceExists("usb_sleep_music");
    isKBDFunctionsSupported = deviceExists("kbd_function_keys");
    isPanelPowerONSupported = deviceExists("panel_power_on");
    isUSBThreeSupported = deviceExists("usb_three");
    isHAPSSupported = deviceExists("haps");

    return true;
}

QString KToshibaHardware::findDriverPath()
{
    QStringList m_devices;
    m_devices << "TOS1900:00" << "TOS6200:00" << "TOS6207:00" << "TOS6208:00";
    QString path("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/%1/path");
    for (int current = m_devices.indexOf(m_devices.first()); current <= m_devices.indexOf(m_devices.last());) {
        m_file.setFileName(path.arg(m_devices.at(current)));
        if (m_file.exists()) {
            m_device = m_devices.at(current);

            return QString("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/%1/").arg(m_device);
        }

        current++;
    }

    qWarning() << "No known kernel interface found" << endl;

    return QString();
}

QString KToshibaHardware::getDeviceHID()
{
    return m_device;
}

bool KToshibaHardware::deviceExists(QString device)
{
    if (device == "illumination" || device == "eco_mode")
        m_file.setFileName(m_ledsPath + device + "/brightness");
    else if (device == "haps")
        m_file.setFileName(m_hapsPath + "protection_level");
    else
        m_file.setFileName(m_driverPath + device);

    return m_file.exists();
}

bool KToshibaHardware::getSysInfo()
{
    Action action("net.sourceforge.ktoshiba.ktoshhelper.dumpsysinfo");
    action.setHelperId(HELPER_ID);
    ExecuteJob *job = action.execute();
    if (!job->exec()) {
        qCritical() << "net.sourceforge.ktoshiba.ktoshhelper.dumpsysinfo failed";

        return false;
    }

    m_file.setFileName("/var/tmp/dmidecode");
    if (!m_file.open(QIODevice::ReadOnly)) {
        qCritical() << "getSysInfo failed with error code" << m_file.error() << m_file.errorString();

        return false;
    }

    QTextStream in(&m_file);
    int count = 0;
    while (count < 2) {
        QString line = in.readLine();
        QStringList split = line.split(":");
        // BIOS Information
        if (split[0].contains("Vendor")) {
            biosManufacturer = split[1].trimmed();
            continue;
        }
        if (split[0].contains("Version")) {
            if (count == 0)
                biosVersion = split[1].trimmed();
            else
                modelNumber = split[1].trimmed();
            count++;
            continue;
        }
        if (split[0].contains("Release Date")) {
            biosDate = split[1].trimmed();
            continue;
        }
        if (split[0].contains("Firmware Revision")) {
            ecVersion = split[1].trimmed();
            continue;
        }
        // System Information
        if (split[0].contains("Product Name")) {
            modelFamily = split[1].trimmed();
            continue;
        }
    };
    m_file.close();

    return true;
}

QString KToshibaHardware::getDriverVersion()
{
    m_file.setFileName(m_driverPath + "version");
    if (!m_file.exists()) {
        qWarning() << "An older driver found, some functionality won't be available."
                   << "Please see the file README.toshiba_acpi for upgrading instructions";
        m_file.setFileName("/proc/acpi/toshiba/version");
        if (!m_file.exists()) {
            qCritical() << "No version file detected";

            return QString("Unknown");
        }

        if (!m_file.open(QIODevice::ReadOnly)) {
            qCritical() << "getDriverVersion failed with error code" << m_file.error() << m_file.errorString();

            return QString("Unknown");
       }

        QTextStream in(&m_file);
        QString line = in.readLine();
        QStringList split = line.split(":");
        m_file.close();

        return split[1].trimmed();
    } else {
        if (!m_file.open(QIODevice::ReadOnly)) {
            qCritical() << "getDriverVersion failed with error code" << m_file.error() << m_file.errorString();

            return QString("Unknown");
       }

        QTextStream in(&m_file);
        QString version = in.readAll();
        m_file.close();

        return version;
    }

    return QString("Unknown");
}

int KToshibaHardware::getTouchPad()
{
    m_file.setFileName(m_driverPath + "touchpad");
    if (!m_file.open(QIODevice::ReadOnly)) {
        qCritical() << "getTouchpad failed with error code" << m_file.error() << m_file.errorString();

        return -1;
    }

    QTextStream stream(&m_file);
    int state = stream.readAll().toInt();
    m_file.close();

    return state;
}

void KToshibaHardware::setTouchPad(int state)
{
    Action action("net.sourceforge.ktoshiba.ktoshhelper.settouchpad");
    action.setHelperId(HELPER_ID);
    action.addArgument("state", state);
    ExecuteJob *job = action.execute();
    if (!job->exec())
        qCritical() << "net.sourceforge.ktoshiba.ktoshhelper.settouchpad failed";

    emit touchpadToggled(state);
}

int KToshibaHardware::getIllumination()
{
    m_file.setFileName(m_ledsPath + "illumination/brightness");
    if (!m_file.open(QIODevice::ReadOnly)) {
        qCritical() << "getIllumination failed with error code" << m_file.error() << m_file.errorString();

        return -1;
    }

    QTextStream stream(&m_file);
    int state = stream.readAll().toInt();
    m_file.close();

    return state;
}

void KToshibaHardware::setIllumination(int state)
{
    Action action("net.sourceforge.ktoshiba.ktoshhelper.setillumination");
    action.setHelperId(HELPER_ID);
    action.addArgument("state", state);
    ExecuteJob *job = action.execute();
    if (!job->exec())
        qCritical() << "net.sourceforge.ktoshiba.ktoshhelper.setillumination failed";
}

int KToshibaHardware::getEcoLed()
{
    m_file.setFileName(m_ledsPath + "eco_mode/brightness");
    if (!m_file.open(QIODevice::ReadOnly)) {
        qCritical() << "getEcoLed failed with error code" << m_file.error() << m_file.errorString();

        return -1;
    }

    QTextStream stream(&m_file);
    int state = stream.readAll().toInt();
    m_file.close();

    return state;
}

void KToshibaHardware::setEcoLed(int state)
{
    Action action("net.sourceforge.ktoshiba.ktoshhelper.seteco");
    action.setHelperId(HELPER_ID);
    action.addArgument("state", state);
    ExecuteJob *job = action.execute();
    if (!job->exec())
        qCritical() << "net.sourceforge.ktoshiba.ktoshhelper.seteco failed";
}

int KToshibaHardware::getKBDType()
{
    m_file.setFileName(m_driverPath + "kbd_type");
    if (!m_file.open(QIODevice::ReadOnly)) {
        qCritical() << "getKBDType failed with error code" << m_file.error() << m_file.errorString();

        return -1;
    }

    QTextStream stream(&m_file);
    int type = stream.readAll().toInt();
    m_file.close();

    return type;
}

int KToshibaHardware::getKBDMode()
{
    m_file.setFileName(m_driverPath + "kbd_backlight_mode");
    if (!m_file.open(QIODevice::ReadOnly)) {
        qCritical() << "getKBDMode failed with error code" << m_file.error() << m_file.errorString();

        return -1;
    }

    QTextStream stream(&m_file);
    int mode = stream.readAll().toInt();
    m_file.close();

    return mode;
}

void KToshibaHardware::setKBDMode(int mode)
{
    Action action("net.sourceforge.ktoshiba.ktoshhelper.setkbdmode");
    action.setHelperId(HELPER_ID);
    action.addArgument("mode", mode);
    ExecuteJob *job = action.execute();
    if (!job->exec())
        qCritical() << "net.sourceforge.ktoshiba.ktoshhelper.setkbdmode failed";
}

int KToshibaHardware::getKBDTimeout()
{
    m_file.setFileName(m_driverPath + "kbd_backlight_timeout");
    if (!m_file.open(QIODevice::ReadOnly)) {
        qCritical() << "getKBDTimeout failed with error code" << m_file.error() << m_file.errorString();

        return -1;
    }

    QTextStream stream(&m_file);
    int timeout = stream.readAll().toInt();
    m_file.close();

    return timeout;
}

void KToshibaHardware::setKBDTimeout(int time)
{
    Action action("net.sourceforge.ktoshiba.ktoshhelper.setkbdtimeout");
    action.setHelperId(HELPER_ID);
    action.addArgument("time", time);
    ExecuteJob *job = action.execute();
    if (!job->exec())
        qCritical() << "net.sourceforge.ktoshiba.ktoshhelper.setkbdtimeout failed";
}

int KToshibaHardware::getUSBSleepCharge()
{
    m_file.setFileName(m_driverPath + "usb_sleep_charge");
    if (!m_file.open(QIODevice::ReadOnly)) {
        qCritical() << "getUSBSleepCharge failed with error code" << m_file.error() << m_file.errorString();

        return -1;
    }

    QTextStream stream(&m_file);
    int mode = stream.readAll().toInt();
    m_file.close();

    return mode;
}

void KToshibaHardware::setUSBSleepCharge(int mode)
{
    Action action("net.sourceforge.ktoshiba.ktoshhelper.setusbsleepcharge");
    action.setHelperId(HELPER_ID);
    action.addArgument("mode", mode);
    ExecuteJob *job = action.execute();
    if (!job->exec())
        qCritical() << "net.sourceforge.ktoshiba.ktoshhelper.setusbsleepcharge failed";
}

QStringList KToshibaHardware::getUSBSleepFunctionsBatLvl()
{
    m_file.setFileName(m_driverPath + "sleep_functions_on_battery");
    if (!m_file.open(QIODevice::ReadOnly)) {
        qCritical() << "getUSBSleepFunctionsBatLvl failed with error code" << m_file.error() << m_file.errorString();

        return QStringList();
    }

    QTextStream stream(&m_file);
    QString line = stream.readAll();
    m_file.close();

    return line.split(" ");
}

void KToshibaHardware::setUSBSleepFunctionsBatLvl(int level)
{
    Action action("net.sourceforge.ktoshiba.ktoshhelper.setusbsleepfunctionsbatlvl");
    action.setHelperId(HELPER_ID);
    action.addArgument("level", level);
    ExecuteJob *job = action.execute();
    if (!job->exec())
        qCritical() << "net.sourceforge.ktoshiba.ktoshhelper.setusbsleepfunctionsbatlvl failed";
}

int KToshibaHardware::getUSBRapidCharge()
{
    m_file.setFileName(m_driverPath + "usb_rapid_charge");
    if (!m_file.open(QIODevice::ReadOnly)) {
        qCritical() << "getUSBRapidCharge failed with error code" << m_file.error() << m_file.errorString();

        return -1;
    }

    QTextStream stream(&m_file);
    int state = stream.readAll().toInt();
    m_file.close();

    return state;
}

void KToshibaHardware::setUSBRapidCharge(int state)
{
    Action action("net.sourceforge.ktoshiba.ktoshhelper.setusbrapidcharge");
    action.setHelperId(HELPER_ID);
    action.addArgument("state", state);
    ExecuteJob *job = action.execute();
    if (!job->exec())
        qCritical() << "net.sourceforge.ktoshiba.ktoshhelper.setusbrapidcharge failed";
}

int KToshibaHardware::getUSBSleepMusic()
{
    m_file.setFileName(m_driverPath + "usb_sleep_music");
    if (!m_file.open(QIODevice::ReadOnly)) {
        qCritical() << "getUSBSleepMusic failed with error code" << m_file.error() << m_file.errorString();

        return -1;
    }

    QTextStream stream(&m_file);
    int state = stream.readAll().toInt();
    m_file.close();

    return state;
}

void KToshibaHardware::setUSBSleepMusic(int state)
{
    Action action("net.sourceforge.ktoshiba.ktoshhelper.setusbsleepmusic");
    action.setHelperId(HELPER_ID);
    action.addArgument("state", state);
    ExecuteJob *job = action.execute();
    if (!job->exec())
        qCritical() << "net.sourceforge.ktoshiba.ktoshhelper.setusbsleepmusic failed";
}

int KToshibaHardware::getKBDFunctions()
{
    m_file.setFileName(m_driverPath + "kbd_function_keys");
    if (!m_file.open(QIODevice::ReadOnly)) {
        qCritical() << "getKBDFunctions failed with error code" << m_file.error() << m_file.errorString();

        return -1;
    }

    QTextStream stream(&m_file);
    int mode = stream.readAll().toInt();
    m_file.close();

    return mode;
}

void KToshibaHardware::setKBDFunctions(int mode)
{
    Action action("net.sourceforge.ktoshiba.ktoshhelper.setkbdfunctions");
    action.setHelperId(HELPER_ID);
    action.addArgument("mode", mode);
    ExecuteJob *job = action.execute();
    if (!job->exec())
        qCritical() << "net.sourceforge.ktoshiba.ktoshhelper.setkbdfunctions failed";
}

int KToshibaHardware::getPanelPowerON()
{
    m_file.setFileName(m_driverPath + "panel_power_on");
    if (!m_file.open(QIODevice::ReadOnly)) {
        qCritical() << "getPanelPowerON failed with error code" << m_file.error() << m_file.errorString();

        return -1;
    }

    QTextStream stream(&m_file);
    int state = stream.readAll().toInt();
    m_file.close();

    return state;
}

void KToshibaHardware::setPanelPowerON(int state)
{
    Action action("net.sourceforge.ktoshiba.ktoshhelper.setpanelpoweron");
    action.setHelperId(HELPER_ID);
    action.addArgument("state", state);
    ExecuteJob *job = action.execute();
    if (!job->exec())
        qCritical() << "net.sourceforge.ktoshiba.ktoshhelper.setpanelpoweron failed";
}

int KToshibaHardware::getUSBThree()
{
    m_file.setFileName(m_driverPath + "usb_three");
    if (!m_file.open(QIODevice::ReadOnly)) {
        qCritical() << "getUSBThree failed with error code" << m_file.error() << m_file.errorString();

        return -1;
    }

    QTextStream stream(&m_file);
    int mode = stream.readAll().toInt();
    m_file.close();

    return mode;
}

void KToshibaHardware::setUSBThree(int mode)
{
    Action action("net.sourceforge.ktoshiba.ktoshhelper.setusbthree");
    action.setHelperId(HELPER_ID);
    action.addArgument("mode", mode);
    ExecuteJob *job = action.execute();
    if (!job->exec())
        qCritical() << "net.sourceforge.ktoshiba.ktoshhelper.setusbthree failed";
}

int KToshibaHardware::getProtectionLevel()
{
    m_file.setFileName("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS620A:00/protection_level");
    if (!m_file.open(QIODevice::ReadOnly)) {
        qCritical() << "getProtectionLevel failed with error code" << m_file.error() << m_file.errorString();

        return -1;
    }

    QTextStream stream(&m_file);
    int level = stream.readAll().toInt();
    m_file.close();

    return level;
}

void KToshibaHardware::setProtectionLevel(int level)
{
    Action action("net.sourceforge.ktoshiba.ktoshhelper.setprotectionlevel");
    action.setHelperId(HELPER_ID);
    action.addArgument("level", level);
    ExecuteJob *job = action.execute();
    if (!job->exec())
        qCritical() << "net.sourceforge.ktoshiba.ktoshhelper.setprotectionlevel failed";
}

void KToshibaHardware::unloadHeads(int timeout)
{
    Action action("net.sourceforge.ktoshiba.ktoshhelper.unloadheads");
    action.setHelperId(HELPER_ID);
    action.addArgument("timeout", timeout);
    ExecuteJob *job = action.execute();
    if (!job->exec())
        qCritical() << "net.sourceforge.ktoshiba.ktoshhelper.unloadheads failed";
}


#include "ktoshibahardware.moc"
