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
#include <QDebug>
#include <QDir>
#include <QStringBuilder>

#include <KAuth/KAuth>

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
}

#include "ktoshibahardware.h"

#define HELPER_ID "net.sourceforge.ktoshiba.ktoshhelper"

using namespace KAuth;

KToshibaHardware::KToshibaHardware(QObject *parent)
    : QObject(parent)
{
    m_file.setFileName(TOSHIBA_ACPI_DEVICE);
    m_devDeviceExist = m_file.exists();
    if (!m_devDeviceExist)
        qCritical() << "The toshiba_acpi device does not exist, perhaps an older driver is loaded?"
                    << "Please see the file README.toshiba_acpi for upgrading instructions";

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
 * Error printing function
 */

void KToshibaHardware::printSMMError(QString function, quint32 error)
{
    qCritical() << function << "failed with error code"
                << QString::number(error, 16) << m_errors.value(error);
}

/*
 * System Information function
 */

QString KToshibaHardware::getDeviceHID()
{
    m_devices << "TOS1900:00" << "TOS6200:00" << "TOS6207:00" << "TOS6208:00";

    QDir dir;
    QString path("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/%1/");
    foreach (const QString &device, m_devices)
        if (dir.exists(path.arg(device)))
            return device;

    qWarning() << "No known kernel interface found" << endl;

    return QString();
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
    if (!m_devDeviceExist)
        return -1;

    int m_fd = ::open(TOSHIBA_ACPI_DEVICE, O_RDWR);
    if (m_fd < 0) {
        qCritical() << "Error while openning toshiba_acpi device:" << strerror(errno);

        return m_fd;
    }

    int ret = -1;
    if (regs->eax == SCI_READ || regs->eax == SCI_WRITE)
        ret = ioctl(m_fd, TOSHIBA_ACPI_SCI, regs);
    else if (regs->eax == HCI_READ || regs->eax == HCI_WRITE)
        ret = ioctl(m_fd, TOSH_SMM, regs);

    ::close(m_fd);
    if (ret < 0)
        qCritical() << "Error while accessing toshiba_acpi device:" << strerror(errno);

    return ret;
}

/*
 * Hardware access funtions
 */

quint32 KToshibaHardware::getPointingDevice()
{
    regs = { SCI_READ, POINTING_DEVICE, 0, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        printSMMError("getPointingDevice", FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        printSMMError("getPointingDevice", regs.eax);

        return regs.eax;
    }

    return regs.ecx;
}

void KToshibaHardware::setPointingDevice(quint32 state)
{
    regs = { SCI_WRITE, POINTING_DEVICE, state, 0, 0, 0 };

    if (state != DEACTIVATED && state != ACTIVATED) {
        printSMMError("setPointingDevice", INPUT_DATA_ERROR);

        return;
    }


    if (tci_raw(&regs) < 0) {
        printSMMError("setPointingDevice", FAILURE);

        return;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        printSMMError("setPointingDevice", regs.eax);
}

quint32 KToshibaHardware::getIllumination()
{
    regs = { SCI_READ, ILLUMINATION_LED, 0, 0, 0, 0 };

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
    regs = { SCI_WRITE, ILLUMINATION_LED, state, 0, 0, 0 };

    if (state != DEACTIVATED && state != ACTIVATED) {
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
    regs = { HCI_READ, ECO_LED, 0, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        printSMMError("getEcoLed", FAILURE);

        return FAILURE;
    }

    if (regs.eax != INPUT_DATA_ERROR) {
        printSMMError("getEcoLed", regs.eax);

        return regs.eax;
    }

    regs = { HCI_READ, 0x97, 0, 1, 0, 0 };
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
    regs = { HCI_WRITE, ECO_LED, state, 0, 0, 0 };

    if (state != DEACTIVATED && state != ACTIVATED) {
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
    regs = { SCI_READ, KBD_ILLUM_STATUS, 0, 0, 0, 0 };

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
    regs = { SCI_WRITE, KBD_ILLUM_STATUS, 0, 0, 0, 0 };

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
    regs = { SCI_READ, USB_SLEEP_CHARGE, 0, 0, 0, 0 };

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
    regs = { SCI_WRITE, USB_SLEEP_CHARGE, 0, 0, 0, 0 };

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
    regs = { SCI_READ, USB_SLEEP_CHARGE, 0, 0, 0, SLEEP_FUNCTIONS_ON_BATTERY };

    if (tci_raw(&regs) < 0) {
        printSMMError("getUSBSleepFunctionsBatLvl", FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        printSMMError("getUSBSleepFunctionsBatLvl", regs.eax);
    } else {
        int tmp = regs.ecx & 0x7;
        *state = (tmp == 0x4) ? ACTIVATED : DEACTIVATED;
        *level = regs.ecx >> 0x10;
    }

    return regs.eax;
}

void KToshibaHardware::setUSBSleepFunctionsBatLvl(int level)
{
    regs = { SCI_WRITE, USB_SLEEP_CHARGE, 0, 0, 0, SLEEP_FUNCTIONS_ON_BATTERY };

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
    regs = { SCI_READ, USB_SLEEP_CHARGE, 0, 0, 0, USB_RAPID_CHARGE };

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
    regs = { SCI_WRITE, USB_SLEEP_CHARGE, state, 0, 0, USB_RAPID_CHARGE };

    if (state != DEACTIVATED && state != ACTIVATED) {
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
    regs = { SCI_READ, SLEEP_MUSIC, 0, 0, 0, 0 };

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
    regs = { SCI_WRITE, SLEEP_MUSIC, state, 0, 0, 0 };

    if (state != DEACTIVATED && state != ACTIVATED) {
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
    regs = { SCI_READ, KBD_FUNCTION_KEYS, 0, 0, 0, 0 };

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
    regs = { SCI_WRITE, KBD_FUNCTION_KEYS, state, 0, 0, 0 };

    if (state != DEACTIVATED && state != ACTIVATED) {
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
    regs = { SCI_READ, PANEL_POWER_ON, 0, 0, 0, 0 };

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
    regs = { SCI_WRITE, PANEL_POWER_ON, state, 0, 0, 0 };

    if (state != DEACTIVATED && state != ACTIVATED) {
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
    regs = { SCI_READ, USB_THREE, 0, 0, 0, 0 };

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
    regs = { SCI_WRITE, USB_THREE, state, 0, 0, 0 };

    if (state != DEACTIVATED && state != ACTIVATED) {
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
    regs = { SCI_READ, BOOT_ORDER, 0, 0, 0, 0 };

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
    regs = { SCI_WRITE, BOOT_ORDER, order, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        printSMMError("setBootOrder", FAILURE);

        return;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        printSMMError("setBootOrder", regs.eax);
}

quint32 KToshibaHardware::getWakeOnKeyboard(int *val, int *defval)
{
    regs = { SCI_READ, WAKE_ON_KEYBOARD, 0, 0, 0, 0 };

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
    regs = { SCI_WRITE, WAKE_ON_KEYBOARD, state, 0, 0, 0 };

    if (state != DEACTIVATED && state != ACTIVATED) {
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
    regs = { SCI_READ, WAKE_ON_LAN, 0x0800, 0, 0, 0 };

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
    regs = { SCI_WRITE, WAKE_ON_LAN, state, 0, 0, 0 };

    if (state != DEACTIVATED && state != ACTIVATED) {
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
    regs = { HCI_READ, COOLING_METHOD, 0, 0, 0, 0 };

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

void KToshibaHardware::setCoolingMethod(quint32 mode)
{
    regs = { HCI_WRITE, COOLING_METHOD, mode, 0, 0, 0 };

    if (mode != MAXIMUM_PERFORMANCE && mode != BATTERY_OPTIMIZED && mode != POWER_SAVER) {
        printSMMError("setCoolingMethod", INPUT_DATA_ERROR);

        return;
    }

    if (tci_raw(&regs) < 0) {
        printSMMError("setCoolingMethod", FAILURE);

        return;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        printSMMError("setCoolingMethod", regs.eax);
}

quint32 KToshibaHardware::getUSBLegacyEmulation()
{
    regs = { SCI_READ, USB_LEGACY_EMULATION, 0, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        printSMMError("getUSBLegacyEmulation", FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        printSMMError("getUSBLegacyEmulation", regs.eax);

        return regs.eax;
    }

    return regs.ecx;
}

void KToshibaHardware::setUSBLegacyEmulation(quint32 state)
{
    regs = { SCI_READ, USB_LEGACY_EMULATION, state, 0, 0, 0 };

    if (state != DEACTIVATED && state != ACTIVATED) {
        printSMMError("setUSBLegacyEmulation", INPUT_DATA_ERROR);

        return;
    }

    if (tci_raw(&regs) < 0) {
        printSMMError("setUSBLegacyEmulation", FAILURE);

        return;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        printSMMError("setUSBLegacyEmulation", regs.eax);
}

quint32 KToshibaHardware::getBuiltInLAN()
{
    regs = { SCI_READ, BUILT_IN_LAN, 0, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        printSMMError("getBuiltInLAN", FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        printSMMError("getBuiltInLAN", regs.eax);

        return regs.eax;
    }

    return regs.ecx;
}

void KToshibaHardware::setBuiltInLAN(quint32 state)
{
    regs = { SCI_READ, BUILT_IN_LAN, state, 0, 0, 0 };

    if (state != DEACTIVATED && state != ACTIVATED) {
        printSMMError("setBuiltInLAN", INPUT_DATA_ERROR);

        return;
    }

    if (tci_raw(&regs) < 0) {
        printSMMError("setBuiltInLAN", FAILURE);

        return;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        printSMMError("setBuiltInLAN", regs.eax);
}

quint32 KToshibaHardware::getSATAInterfaceSetting()
{
    regs = { SCI_READ, SATA_IFACE_SETTING, 0, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        printSMMError("getSATAInterfaceSetting", FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        printSMMError("getSATAInterfaceSetting", regs.eax);

        return regs.eax;
    }

    return regs.ecx;
}

void KToshibaHardware::setSATAInterfaceSetting(quint32 mode)
{
    regs = { SCI_WRITE, SATA_IFACE_SETTING, mode, 0, 0, 0 };

    if (mode != PERFORMANCE && mode != BATTERY_LIFE) {
        printSMMError("setSATAInterfaceSetting", INPUT_DATA_ERROR);

        return;
    }

    if (tci_raw(&regs) < 0) {
        printSMMError("setSATAInterfaceSetting", FAILURE);

        return;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        printSMMError("setSATAInterfaceSetting", regs.eax);
}

quint32 KToshibaHardware::getBootSpeed()
{
    regs = { SCI_READ, BOOT_SPEED, 0, 0, 0, 0 };

    if (tci_raw(&regs) < 0) {
        printSMMError("getBootSpeed", FAILURE);

        return FAILURE;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2) {
        printSMMError("getBootSpeed", regs.eax);

        return regs.eax;
    }

    return regs.ecx;
}

void KToshibaHardware::setBootSpeed(quint32 state)
{
    regs = { SCI_WRITE, BOOT_SPEED, state, 0, 0, 0 };

    if (state != NORMAL && state != FAST) {
        printSMMError("setBootSpeed", INPUT_DATA_ERROR);

        return;
    }

    if (tci_raw(&regs) < 0) {
        printSMMError("setBootSpeed", FAILURE);

        return;
    }

    if (regs.eax != SUCCESS && regs.eax != SUCCESS2)
        printSMMError("setBootSpeed", regs.eax);
}
