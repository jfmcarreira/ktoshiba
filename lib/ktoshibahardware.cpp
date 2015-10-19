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

void KToshibaHardware::printSMMError(QString function, quint32 error)
{
    qCritical() << function << "failed with error code"
                << QString::number(error, 16) << m_errors.value(error);
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

        return FAILURE;
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
    m_file.setFileName(TOSHIBA_ACPI_DEVICE);
    if (!m_file.exists()) {
        qCritical() << "The toshiba_acpi device does not exist, perhaps an older driver is loaded?";

        return -1;
    }
    
    int m_fd = ::open(TOSHIBA_ACPI_DEVICE, O_RDWR);
    if (m_fd < 0) {
        qCritical() << "Error while openning toshiba_acpi device:" << strerror(errno);

        return m_fd;
    }

    int ret = -1;
    if (regs->eax == 0xf300 || regs->eax == 0xf400)
        ret = ioctl(m_fd, TOSHIBA_ACPI_SCI, regs);
    else if (regs->eax == 0xfe00 || regs->eax == 0xff00)
        ret = ioctl(m_fd, TOSH_SMM, regs);

    ::close(m_fd);
    if (ret < 0)
        qCritical() << "Error while accessing toshiba_acpi device:" << strerror(errno);

    return ret;
}

/*
 * Hardware access funtions
 */

quint32 KToshibaHardware::getTouchPad()
{
    SMMRegisters regs = { 0xf300, 0x050e, 0, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        printSMMError("getTouchPad", FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        printSMMError("getTouchPad", regs.eax);

        return regs.eax;
    }

    return regs.ecx;
}

void KToshibaHardware::setTouchPad(quint32 state)
{
    SMMRegisters regs = { 0xf400, 0x050e, state, 0, 0, 0 };

    if (state != 0 && state != 1) {
        printSMMError("setTouchPad", INPUT_DATA_ERROR);

        return;
    }


    if (tci_raw(&regs) < 0) {
        printSMMError("setTouchPad", FAILURE);

        return;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        printSMMError("setTouchPad", regs.eax);
}

quint32 KToshibaHardware::getIllumination()
{
    SMMRegisters regs = { 0xf300, 0x014e, 0, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        printSMMError("getIllumination", FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        printSMMError("getIllumination", regs.eax);

        return regs.eax;
    }

    return regs.ecx;
}

void KToshibaHardware::setIllumination(quint32 state)
{
    SMMRegisters regs = { 0xf400, 0x014e, state, 0, 0, 0 };

    if (state != 0 && state != 1) {
        printSMMError("setIllumination", INPUT_DATA_ERROR);

        return;
    }

    if (tci_raw(&regs) < 0) {
        printSMMError("setIllumination", FAILURE);

        return;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        printSMMError("setIllumination", regs.eax);
}

quint32 KToshibaHardware::getEcoLed()
{
    SMMRegisters regs = { 0xfe00, 0x97, 0, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        printSMMError("getEcoLed", FAILURE);

        return FAILURE;
    }

    if (regs.eax != INPUT_DATA_ERROR) {
        printSMMError("getEcoLed", regs.eax);

        return regs.eax;
    }

    regs = { 0xfe00, 0x97, 0, 1, 0, 0 };
    if (tci_raw(&regs) < 0) {
        printSMMError("getEcoLed", FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        printSMMError("getEcoLed", regs.eax);

        return regs.eax;
    }

    return regs.ecx;
}

void KToshibaHardware::setEcoLed(quint32 state)
{
    SMMRegisters regs = { 0xff00, 0x97, state, 0, 0, 0 };

    if (state != 0 && state != 1) {
        printSMMError("setEcoLed", INPUT_DATA_ERROR);

        return;
    }

    if (tci_raw(&regs) < 0) {
        printSMMError("setEcoLed", FAILURE);

        return;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        printSMMError("setEcoLed", regs.eax);
}

quint32 KToshibaHardware::getKBDBacklight(int *mode, int *time, int *type)
{
    SMMRegisters regs = { 0xf300, 0x015c, 0, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        printSMMError("getKBDBacklight", FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        printSMMError("getKBDBacklight", regs.eax);
    } else {
        *mode = (regs.ecx & 0x1f);
        *time = regs.ecx >> 0x10;
        if (regs.edx == 0x3c0003)
            *type = 1;
        else if (regs.edx == 0x3c001a)
            *type = 2;
    }

    return regs.eax;
}

void KToshibaHardware::setKBDBacklight(int mode, int time)
{
    SMMRegisters regs = { 0xf400, 0x015c, 0, 0, 0, 0 };

    if (time < 0 || time > 100) {
        printSMMError("setKBDBacklight", INPUT_DATA_ERROR);

        return;
    }

    regs.ecx = time << 0x10;
    regs.ecx |= mode;
    if (tci_raw(&regs) < 0) {
        printSMMError("setKBDBacklight", FAILURE);

        return;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        printSMMError("setKBDBacklight", regs.eax);
}

quint32 KToshibaHardware::getUSBSleepCharge(int *val, int *maxval, int *defval)
{
    SMMRegisters regs = { 0xf300, 0x0150, 0, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        printSMMError("getUSBSleepCharge", FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        printSMMError("getUSBSleepCharge", regs.eax);
    } else {
        *val = regs.ecx & 0xff;
        *maxval = regs.edx & 0xff;
        *defval = regs.esi;
    }

    return regs.eax;
}

void KToshibaHardware::setUSBSleepCharge(int mode, int base)
{
    SMMRegisters regs = { 0xf400, 0x0150, 0, 0, 0, 0 };

    regs.ecx = base | mode;
    if (tci_raw(&regs) < 0) {
        printSMMError("setUSBSleepCharge", FAILURE);

        return;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        printSMMError("setUSBSleepCharge", regs.eax);
}

quint32 KToshibaHardware::getUSBSleepFunctionsBatLvl(int *state, int *level)
{
    SMMRegisters regs = { 0xf300, 0x0150, 0, 0, 0, 0x0200 };

    if (tci_raw(&regs) < 0) {
        printSMMError("getUSBSleepFunctionsBatLvl", FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        printSMMError("getUSBSleepFunctionsBatLvl", regs.eax);
    } else {
        int tmp = regs.ecx & 0x7;
        *state = (tmp == 0x4) ? 1 : 0;
        *level = regs.ecx >> 0x10;
    }

    return regs.eax;
}

void KToshibaHardware::setUSBSleepFunctionsBatLvl(int level)
{
    SMMRegisters regs = { 0xf400, 0x0150, 0, 0, 0, 0x0200 };

    if (level < 0 || level > 100) {
        printSMMError("setUSBSleepFunctionsBatLvl", INPUT_DATA_ERROR);

        return;
    }

    regs.ecx = level << 0x10;
    regs.ecx |= (level == 0 ? 0x1 : 0x4);
    if (tci_raw(&regs) < 0) {
        printSMMError("setUSBSleepFunctionsBatLvl", FAILURE);

        return;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        printSMMError("setUSBSleepFunctionsBatLvl", regs.eax);
}

quint32 KToshibaHardware::getUSBRapidCharge()
{
    SMMRegisters regs = { 0xf300, 0x0150, 0, 0, 0, 0x0300 };

    if (tci_raw(&regs) < 0) {
        printSMMError("getUSBRapidCharge", FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        printSMMError("getUSBRapidCharge", regs.eax);

        return regs.eax;
    }

    return regs.ecx;
}

void KToshibaHardware::setUSBRapidCharge(quint32 state)
{
    SMMRegisters regs = { 0xf400, 0x0150, state, 0, 0, 0x0300 };

    if (state != 0 && state != 1) {
        printSMMError("setUSBRapidCharge", INPUT_DATA_ERROR);

        return;
    }

    if (tci_raw(&regs) < 0) {
        printSMMError("setUSBRapidCharge", FAILURE);

        return;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        printSMMError("setUSBRapidCharge", regs.eax);
}

quint32 KToshibaHardware::getUSBSleepMusic()
{
    SMMRegisters regs = { 0xf300, 0x015e, 0, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        printSMMError("getUSBSleepMusic", FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        printSMMError("getUSBSleepMusic", regs.eax);

        return regs.eax;
    }

    return regs.ecx;
}

void KToshibaHardware::setUSBSleepMusic(quint32 state)
{
    SMMRegisters regs = { 0xf400, 0x015e, state, 0, 0, 0 };

    if (state != 0 && state != 1) {
        printSMMError("setUSBSleepMusic", INPUT_DATA_ERROR);

        return;
    }

    if (tci_raw(&regs) < 0) {
        printSMMError("setUSBSleepMusic", FAILURE);

        return;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        printSMMError("setUSBSleepMusic", regs.eax);
}

quint32 KToshibaHardware::getKBDFunctions()
{
    SMMRegisters regs = { 0xf300, 0x0522, 0, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        printSMMError("getKBDFunctions", FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        printSMMError("getKBDFunctions", regs.eax);

        return regs.eax;
    }

    return regs.ecx;
}

void KToshibaHardware::setKBDFunctions(quint32 state)
{
    SMMRegisters regs = { 0xf400, 0x0522, state, 0, 0, 0 };

    if (state != 0 && state != 1) {
        printSMMError("setKBDFunctions", INPUT_DATA_ERROR);

        return;
    }

    if (tci_raw(&regs) < 0) {
        printSMMError("setKBDFunctions", FAILURE);

        return;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        printSMMError("setKBDFunctions", regs.eax);
}

quint32 KToshibaHardware::getPanelPowerON()
{
    SMMRegisters regs = { 0xf300, 0x010d, 0, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        printSMMError("getPanelPowerON", FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        printSMMError("getPanelPowerON", regs.eax);

        return regs.eax;
    }

    return regs.ecx;
}

void KToshibaHardware::setPanelPowerON(quint32 state)
{
    SMMRegisters regs = { 0xf400, 0x010d, state, 0, 0, 0 };

    if (state != 0 && state != 1) {
        printSMMError("setPanelPowerON", INPUT_DATA_ERROR);

        return;
    }

    if (tci_raw(&regs) < 0) {
        printSMMError("setPanelPowerON", FAILURE);

        return;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        printSMMError("setPanelPowerON", regs.eax);
}

quint32 KToshibaHardware::getUSBThree()
{
    SMMRegisters regs = { 0xf300, 0x0169, 0, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        printSMMError("getUSBThree", FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        printSMMError("getUSBThree", regs.eax);

        return regs.eax;
    }

    return regs.ecx;
}

void KToshibaHardware::setUSBThree(quint32 state)
{
    SMMRegisters regs = { 0xf400, 0x0169, state, 0, 0, 0 };

    if (state != 0 && state != 1) {
        printSMMError("setUSBThree", INPUT_DATA_ERROR);

        return;
    }

    if (tci_raw(&regs) < 0) {
        printSMMError("setUSBThree", FAILURE);

        return;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        printSMMError("setUSBThree", regs.eax);
}

quint32 KToshibaHardware::getBootOrder(int *val, int *maxval, int *defval)
{
    SMMRegisters regs = { 0xf300, 0x0157, 0, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        printSMMError("getBootOrder", FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        printSMMError("getBootOrder", regs.eax);
    } else {
        *val = regs.ecx;
        *maxval = regs.edx;
        *defval = regs.esi;
    }

    return regs.eax;
}

void KToshibaHardware::setBootOrder(quint32 order)
{
    SMMRegisters regs = { 0xf400, 0x0157, order, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        printSMMError("setBootOrder", FAILURE);

        return;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        printSMMError("setBootOrder", regs.eax);
}

quint32 KToshibaHardware::getWakeOnKeyboard(int *val, int *defval)
{
    SMMRegisters regs = { 0xf300, 0x0137, 0, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        printSMMError("getWakeOnKeyboard", FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        printSMMError("getWakeOnKeyboard", regs.eax);
    } else {
        *val = regs.ecx;
        *defval = regs.esi;
    }

    return regs.eax;
}

void KToshibaHardware::setWakeOnKeyboard(quint32 state)
{
    SMMRegisters regs = { 0xf400, 0x0137, state, 0, 0, 0 };

    if (state != 0 && state != 1) {
        printSMMError("setWakeOnKeyboard", INPUT_DATA_ERROR);

        return;
    }

    if (tci_raw(&regs) < 0) {
        printSMMError("setWakeOnKeyboard", FAILURE);

        return;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        printSMMError("setWakeOnKeyboard", regs.eax);
}

quint32 KToshibaHardware::getWakeOnLAN(int *val, int *defval)
{
    SMMRegisters regs = { 0xf300, 0x0700, 0x0800, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        printSMMError("getWakeOnLAN", FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        printSMMError("getWakeOnLAN", regs.eax);
    } else {
        *val = regs.ecx;
        *defval = regs.esi;
    }

    return regs.eax;
}

void KToshibaHardware::setWakeOnLAN(quint32 state)
{
    SMMRegisters regs = { 0xf400, 0x0700, state, 0, 0, 0 };

    if (state != 0 && state != 1) {
        printSMMError("setWakeOnLAN", INPUT_DATA_ERROR);

        return;
    }

    if (tci_raw(&regs) < 0) {
        printSMMError("setWakeOnLAN", FAILURE);

        return;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        printSMMError("setWakeOnLAN", regs.eax);
}

quint32 KToshibaHardware::getCoolingMethod(int *val, int *maxval)
{
    SMMRegisters regs = { 0xfe00, 0x007f, 0, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        printSMMError("getCoolingMethod", FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        printSMMError("getCoolingMethod", regs.eax);
    } else {
        *val = regs.ecx;
        *maxval = regs.edx;
    }

    return regs.eax;
}

void KToshibaHardware::setCoolingMethod(int mode)
{
    SMMRegisters regs = { 0xff00, 0x007f, 0, 0, 0, 0 };

    if (mode != 0 && mode != 1 && mode != 2) {
        printSMMError("setCoolingMethod", INPUT_DATA_ERROR);

        return;
    }

    regs.ecx = mode;
    if (tci_raw(&regs) < 0) {
        printSMMError("setCoolingMethod", FAILURE);

        return;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        printSMMError("setCoolingMethod", regs.eax);
}
