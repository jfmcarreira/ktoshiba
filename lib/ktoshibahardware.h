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
#include <QFile>
#include <QMap>

extern "C" {
#include <linux/toshiba.h>
}

#include "ktoshibahardware_export.h"

class KTOSHIBAHARDWARE_EXPORT KToshibaHardware : public QObject
{
    Q_OBJECT

public:
    KToshibaHardware(QObject *parent = 0);

    enum ReturnCodes {
        SUCCESS            = 0x0000,
        SUCCESS2           = 0x0001,
        FAILURE            = 0x1000,
        NOT_SUPPORTED      = 0x8000,
        INPUT_DATA_ERROR   = 0x8300,
        WRITE_PROTECTED    = 0x8400,
        NOT_READY          = 0x8c00,
        DATA_NOT_AVAILABLE = 0x8d20,
        NOT_INITIALIZED    = 0x8d50,
        NOT_INSTALLED      = 0x8e00,
    };

    enum USBSleepChargeModes {
        DISABLED  = 0x00,
        //MODE1     = 0x,
        //MODE2     = 0x,
        ALTERNATE = 0x09,
        TYPICAL   = 0x11,
        AUTO      = 0x21,
        //TABLET    = 0x,
    };

    enum KeyboardBacklightModes {
        FNZ   = 0x01,
        TIMER = 0x02,
        ON    = 0x08,
        OFF   = 0x10,
    };

    enum CoolingMethods {
        MAXIMUM_PERFORMANCE = 0,
        BATTERY_OPTIMIZED   = 1,
        HIGH_PERFORMANCE    = 0,
        BALANCED            = 1,
        POWER_SAVER         = 2,
    };

    enum DeviceState {
        TCI_DISABLED,
        TCI_ENABLED,
    };

    /*
     * System Information calls
     */
    bool getSysInfo();
    QString getDriverVersion();
    QString getDeviceHID();
    /*
     * HDD protection functions
     */
    int getProtectionLevel();
    void setProtectionLevel(int);
    void unloadHeads(int);
    /*
     * Toshiba Configuration Interface (TCI) access function
     */
    int tci_raw(const SMMRegisters *);
    /*
     * Hardware access functions
     */
    quint32 getTouchPad();
    void setTouchPad(quint32);
    quint32 getIllumination();
    void setIllumination(quint32);
    quint32 getEcoLed();
    void setEcoLed(quint32);
    quint32 getKBDBacklight(int *, int *, int *);
    void setKBDBacklight(int, int);
    quint32 getUSBSleepCharge(int *, int *, int *);
    void setUSBSleepCharge(int, int);
    quint32 getUSBSleepFunctionsBatLvl(int *, int *);
    void setUSBSleepFunctionsBatLvl(int);
    quint32 getUSBRapidCharge();
    void setUSBRapidCharge(quint32);
    quint32 getUSBSleepMusic();
    void setUSBSleepMusic(quint32);
    quint32 getKBDFunctions();
    void setKBDFunctions(quint32);
    quint32 getPanelPowerON();
    void setPanelPowerON(quint32);
    quint32 getUSBThree();
    void setUSBThree(quint32);
    quint32 getBootOrder(int *, int *, int *);
    void setBootOrder(quint32);
    quint32 getWakeOnKeyboard(int *, int *);
    void setWakeOnKeyboard(quint32);
    quint32 getWakeOnLAN(int *, int *);
    void setWakeOnLAN(quint32);
    quint32 getCoolingMethod(int *, int *);
    void setCoolingMethod(int);

    QString modelFamily;
    QString modelNumber;
    QString biosVersion;
    QString biosDate;
    QString biosManufacturer;
    QString ecVersion;

Q_SIGNALS:
    void touchpadToggled(int);

private:
    void printSMMError(QString, quint32);

    QMap<int, QString> m_errors;

    QString findDriverPath();

    QString m_device;
    QString m_driverPath;
    QFile m_file;
};

#endif // KTOSHIBAHARDWARE_H
