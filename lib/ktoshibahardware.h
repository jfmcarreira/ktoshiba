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
#include <QMap>

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

class KTOSHIBAHARDWARE_EXPORT KToshibaHardware : public QObject
{
    Q_OBJECT

public:
    KToshibaHardware(QObject *parent = 0);
    bool init();

    enum TCIReturnCodes {
        SUCCESS            = 0x0000,
        SUCCESS2           = 0x0001,
        FAILURE            = 0x1000,
        NOT_SUPPORTED      = 0x8000,
        INPUT_DATA_ERROR   = 0x8300,
        WRITE_PROTECTED    = 0x8400,
        NOT_READY          = 0x8c00,
        DATA_NOT_AVAILABLE = 0x8d20,
        NOT_INITIALIZED    = 0x8d50,
        NOT_INSTALLED      = 0x8e00
    };

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
    quint32 getBootOrder(quint32 *, quint32 *, quint32 *);
    void setBootOrder(quint32);
    quint32 getWakeOnKeyboard(quint32 *, quint32 *);
    void setWakeOnKeyboard(quint32);
    quint32 getWakeOnLAN(quint32 *, quint32 *);
    void setWakeOnLAN(quint32);
    quint32 getCoolingMethod(quint32 *, quint32 *);
    void setCoolingMethod(quint32);

Q_SIGNALS:
    void touchpadToggled(int);

private:
    QMap<int, QString> m_errors;

    QString findDriverPath();
    bool deviceExists(QString);

    QString m_device;
    QString m_driverPath;
    QString m_ledsPath;
    QString m_hapsPath;
    QFile m_file;
};

#endif // KTOSHIBAHARDWARE_H
