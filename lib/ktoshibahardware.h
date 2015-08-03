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

#ifndef KTOSHIBAHARDWARE_H
#define KTOSHIBAHARDWARE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QFile>

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <linux/toshiba.h>
}

#include "ktoshibahardware_export.h"

/* TODO: Remove these entries once upstream toshiba_acpi module
 *       gets updated.
 */
#define TOSHIBA_ACPI_DEV	"/dev/toshiba_acpi"
#define TOSHIBA_ACPI_SCI	_IOWR('t', 0x91, SMMRegisters)

class KTOSHIBAHARDWARE_EXPORT KToshibaHardware : public QObject
{
    Q_OBJECT

public:
    KToshibaHardware(QObject *parent = 0);
    bool init();

    QString modelFamily;
    QString modelNumber;
    QString biosVersion;
    QString biosDate;
    QString biosManufacturer;
    QString ecVersion;

    bool isTouchPadSupported;
    bool isIlluminationSupported;
    bool isECOSupported;
    bool isKBDBacklightSupported;
    bool isKBDTypeSupported;
    bool isUSBSleepChargeSupported;
    bool isUSBRapidChargeSupported;
    bool isUSBSleepMusicSupported;
    bool isKBDFunctionsSupported;
    bool isPanelPowerONSupported;
    bool isUSBThreeSupported;
    bool isHAPSSupported;
    bool isSMMSupported;

public Q_SLOTS:
    /*
     * System Information calls
     */
    bool getSysInfo();
    QString getDriverVersion();
    QString getDeviceHID();
    /*
     * Hardware calls
     */
    int getTouchPad();
    void setTouchPad(int);
    int getIllumination();
    void setIllumination(int);
    int getEcoLed();
    void setEcoLed(int);
    int getKBDType();
    int getKBDMode();
    void setKBDMode(int);
    int getKBDTimeout();
    void setKBDTimeout(int);
    int getUSBSleepCharge();
    void setUSBSleepCharge(int);
    QStringList getUSBSleepFunctionsBatLvl();
    void setUSBSleepFunctionsBatLvl(int);
    int getUSBRapidCharge();
    void setUSBRapidCharge(int);
    int getUSBSleepMusic();
    void setUSBSleepMusic(int);
    int getKBDFunctions();
    void setKBDFunctions(int);
    int getPanelPowerON();
    void setPanelPowerON(int);
    int getUSBThree();
    void setUSBThree(int);
    /*
     * HDD protection calls
     */
    int getProtectionLevel();
    void setProtectionLevel(int);
    void unloadHeads(int);
    /*
     * TCI access calls
     */
    int tci_raw(const SMMRegisters *);

Q_SIGNALS:
    void touchpadToggled(int);

private:
    QString findDriverPath();
    bool deviceExists(QString);

    QString m_device;
    QString m_driverPath;
    QString m_ledsPath;
    QString m_hapsPath;
    QFile m_file;
};

#endif // KTOSHIBAHARDWARE_H
