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
    : QObject(parent)
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

    m_errors[FAILURE] = "HCI/SCI call could not be completed";
    m_errors[NOT_SUPPORTED] = "Feature is not supported";
    m_errors[INPUT_DATA_ERROR] = "Invalid parameters";
    m_errors[WRITE_PROTECTED] = "Device is write protected";
    m_errors[NOT_READY] = "Device is not ready";
    m_errors[DATA_NOT_AVAILABLE] = "Data is not available";
    m_errors[NOT_INITIALIZED] = "Device is not initialized";
    m_errors[NOT_INSTALLED] = "Device is not installed";
}

/*
 * Internal functions
 */

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

bool KToshibaHardware::deviceExists(QString device)
{
    if (device == "illumination" || device == "eco_mode")
        m_file.setFileName(m_ledsPath + device + "/brightness");
    else if (device == "haps")
        m_file.setFileName(m_hapsPath + "protection_level");
    else if (device == "toshiba_acpi")
        m_file.setFileName("/dev/toshiba_acpi");
    else
        m_file.setFileName(m_driverPath + device);

    return m_file.exists();
}

/*
 * INIT function
 */

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
    isSMMSupported = deviceExists("toshiba_acpi");

    return true;
}

/*
 * System Information functions
 */

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
    if (m_file.exists() && m_file.open(QIODevice::ReadOnly)) {
        QTextStream in(&m_file);
        QString version = in.readAll();
        m_file.close();

        return version;
    }

    m_file.setFileName("/proc/acpi/toshiba/version");
    if (m_file.exists() && m_file.open(QIODevice::ReadOnly)) {
        qWarning() << "An older driver found, some functionality won't be available."
                   << "Please see the file README.toshiba_acpi for upgrading instructions";

        QTextStream in(&m_file);
        QString line = in.readLine();
        QStringList split = line.split(":");
        m_file.close();

        return split[1].trimmed();
    }

    qCritical() << "No version file detected or toshiba_acpi module not loaded";

    return QString("0.0");
}

QString KToshibaHardware::getDeviceHID()
{
    return m_device;
}

/*
 * HDD protection functions
 */

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

/*
 * Toshiba Configuration Interface (TCI) access function
 */

int KToshibaHardware::tci_raw(const SMMRegisters *regs)
{
    int m_fd = ::open(TOSHIBA_ACPI_DEVICE, O_RDWR);
    if (m_fd < 0) {
        qCritical() << "Error while openning toshiba_acpi device:" << strerror(errno);

        return m_fd;
    }

    int ret;
    if (regs->eax == 0xf300 || regs->eax == 0xf400)
        ret = ioctl(m_fd, TOSHIBA_ACPI_SCI, regs);
    else if (regs->eax == 0xfe00 || regs->eax == 0xff00)
        ret = ioctl(m_fd, TOSH_SMM, regs);

    ::close(m_fd);
    if (ret < 0) {
        qCritical() << "Error while accessing toshiba_acpi device:" << strerror(errno);

        return ret;
    }

    return 0;
}

/*
 * Hardware access funtions
 */

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

quint32 KToshibaHardware::getBootOrder(quint32 *val, quint32 *maxval, quint32 *defval)
{
    SMMRegisters regs = { 0xf300, 0x0157, 0, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        qCritical() << "getBootOrder failed with error code"
                    << QString().setNum(FAILURE, 16) << m_errors.value(FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        qCritical() << "getBootOrder failed with error code"
                    << QString().setNum(regs.eax, 16) << m_errors.value(regs.eax);
    } else {
        *val = regs.ecx;
        *maxval = regs.edx;
        *defval = regs.esi;
    }

    return regs.eax;
}

void KToshibaHardware::setBootOrder(quint32 order)
{
    SMMRegisters regs = { 0xf300, 0x0157, order, 0, 0, 0 };

    if (tci_raw(&regs) < 0)
        qCritical() << "setBootOrder failed with error code"
                    << QString().setNum(FAILURE, 16) << m_errors.value(FAILURE);

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        qCritical() << "setBootOrder failed with error code"
                    << QString().setNum(regs.eax, 16) << m_errors.value(regs.eax);
}

quint32 KToshibaHardware::getWakeOnKeyboard(quint32 *val, quint32 *defval)
{
    SMMRegisters regs = { 0xf300, 0x0137, 0, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        qCritical() << "getWakeOnKeyboard failed with error code"
                    << QString().setNum(FAILURE, 16) << m_errors.value(FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        qCritical() << "getWakeOnKeyboard failed with error code"
                    << QString().setNum(regs.eax, 16) << m_errors.value(regs.eax);
    } else {
        *val = regs.ecx;
        *defval = regs.esi;
    }

    return regs.eax;
}

void KToshibaHardware::setWakeOnKeyboard(quint32 state)
{
    SMMRegisters regs = { 0xf300, 0x0137, state, 0, 0, 0 };

    if (tci_raw(&regs) < 0)
        qCritical() << "setWakeOnKeyboard failed with error code"
                    << QString().setNum(FAILURE, 16) << m_errors.value(FAILURE);

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        qCritical() << "setWakeOnKeyboard failed with error code"
                    << QString().setNum(regs.eax, 16) << m_errors.value(regs.eax);
}

quint32 KToshibaHardware::getWakeOnLAN(quint32 *val, quint32 *defval)
{
    SMMRegisters regs = { 0xf300, 0x0700, 0x0800, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        qCritical() << "getWakeOnLAN failed with error code"
                    << QString().setNum(FAILURE, 16) << m_errors.value(FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        qCritical() << "getWakeOnLAN failed with error code"
                    << QString().setNum(regs.eax, 16) << m_errors.value(regs.eax);
    } else {
        *val = regs.ecx;
        *defval = regs.esi;
    }

    return regs.eax;
}

void KToshibaHardware::setWakeOnLAN(quint32 state)
{
    SMMRegisters regs = { 0xf300, 0x0700, state, 0, 0, 0 };

    if (tci_raw(&regs) < 0)
        qCritical() << "setWakeOnLAN failed with error code"
                    << QString().setNum(FAILURE, 16) << m_errors.value(FAILURE);

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        qCritical() << "setWakeOnLAN failed with error code"
                    << QString().setNum(regs.eax, 16) << m_errors.value(regs.eax);
}

quint32 KToshibaHardware::getCoolingMethod(quint32 *val, quint32 *maxval)
{
    SMMRegisters regs = { 0xfe00, 0x007f, 0, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        qCritical() << "getCoolingMethod failed with error code"
                    << QString().setNum(FAILURE, 16) << m_errors.value(FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        qCritical() << "getCoolingMethod failed with error code"
                    << QString().setNum(regs.eax, 16) << m_errors.value(regs.eax);
    } else {
        *val = regs.ecx;
        *maxval = regs.edx;
    }

    return regs.eax;
}

void KToshibaHardware::setCoolingMethod(quint32 mode)
{
    SMMRegisters regs = { 0xfe00, 0x007f, mode, 0, 0, 0 };

    if (tci_raw(&regs) < 0)
        qCritical() << "setCoolingMethod failed with error code"
                    << QString().setNum(FAILURE, 16) << m_errors.value(FAILURE);

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        qCritical() << "setCoolingMethod failed with error code"
                    << QString().setNum(regs.eax, 16) << m_errors.value(regs.eax);
}


#include "ktoshibahardware.moc"
