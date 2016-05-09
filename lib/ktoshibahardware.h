/*
   Copyright (C) 2014-2016  Azael Avalos <coproscefalo@gmail.com>

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

#include <QFile>
#include <QMap>
#include <QObject>

#include "ktoshibahardware_export.h"

extern "C" {
#include "toshiba.h"
}

class KTOSHIBAHARDWARE_EXPORT KToshibaHardware : public QObject
{
    Q_OBJECT

public:
    explicit KToshibaHardware(QObject *parent = 0);

    enum TCIOperations {
        SCI_READ  = 0xf300,
        SCI_WRITE = 0xf400,
        HCI_READ  = 0xfe00,
        HCI_WRITE = 0xff00,
    };

    enum TCIRegisters {
        /* HCI Registers */
        ODD_POWER_SUPPORT    = 0x0076,
        COOLING_METHOD       = 0x007f,
        KBD_ILLUM_LED        = 0x0095,
        ECO_LED              = 0x0097,
        /* SCI Registers */
        PANEL_POWER_ON       = 0x010d,
        BUILT_IN_LAN         = 0x0130,
        WAKE_ON_KEYBOARD     = 0x0137,
        ILLUMINATION_LED     = 0x014e,
        USB_SLEEP_CHARGE     = 0x0150,
        BOOT_ORDER           = 0x0157,
        KBD_ILLUM_STATUS     = 0x015c,
        BOOT_SPEED           = 0x015d,
        SLEEP_MUSIC          = 0x015e,
        USB_THREE            = 0x0169,
        POWER_ON_DISPLAY     = 0x0300,
        SATA_IFACE_SETTING   = 0x0406,
        USB_LEGACY_EMULATION = 0x050c,
        POINTING_DEVICE      = 0x050e,
        KBD_FUNCTION_KEYS    = 0x0522,
        WAKE_ON_LAN          = 0x0700,
    };

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
        NOT_INSTALLED      = 0x8e00,
    };

    enum ODDPowerStates {
        ODD_DISABLED = 0x0100,
        ODD_ENABLED  = 0x0101,
    };

    enum CoolingMethods {
        MAXIMUM_PERFORMANCE = 0,
        BATTERY_OPTIMIZED   = 1,
        PERFORMANCE         = 1,
        BATTERY_OPTIMIZED2  = 2,
    };

    enum USBSleepChargeModes {
        DISABLED  = 0x00,
        ALTERNATE = 0x09,
        TYPICAL   = 0x11,
        AUTO      = 0x21,
    };

    enum USBSleepChargeSubfunctions {
        SUBFUNCTIONS_AVAILABLE     = 0x0100,
        MAX_SLEEP_MODE             = 0x0102,
        SLEEP_FUNCTIONS_ON_BATTERY = 0x0200,
        USB_RAPID_CHARGE           = 0x0300,
    };

    enum KeyboardBacklightModes {
        FNZ   = 0x01,
        TIMER = 0x02,
        ON    = 0x08,
        OFF   = 0x10,
    };

    enum SATAInterfaceSettingMode {
        SATA_PERFORMANCE,
        SATA_BATTERY_LIFE,
    };

    enum BootSpeedModes {
        NORMAL,
        FAST,
    };

    enum PowerOnDisplay {
        LCD_DISPLAY      = 0x1290,
        AUTO_DISPLAY     = 0x3290,
        RGB_DISPLAY      = 0x3291,
        UNKNOWN_DISPLAY1 = 0x3292,
        HDMI_DISPLAY     = 0x3294,
        UNKNOWN_DISPLAY2 = 0x3298,
    };

    enum DeviceState {
        DEACTIVATED,
        ACTIVATED,
    };

    /*
     * HDD protection functions
     */
    int getHDDProtectionLevel();
    void setHDDProtectionLevel(int);
    void unloadHDDHeads(int);
    /*
     * Toshiba Configuration Interface (TCI) access function
     */
    int tci_raw(const SMMRegisters *);
    /*
     * Hardware access functions
     */
    quint32 getPointingDevice();
    void setPointingDevice(quint32);
    quint32 getIlluminationLED();
    void setIlluminationLED(quint32);
    quint32 getEcoLED();
    void setEcoLED(quint32);
    quint32 getKBDBacklight(int *, int *, int *);
    void setKBDBacklight(int, int);
    quint32 getUSBSleepCharge(int *, int *, int *);
    void setUSBSleepCharge(int, int);
    quint32 getSleepFunctionsOnBatteryStatus(int *, int *);
    void setSleepFunctionsOnBatteryStatus(int, int);
    quint32 getUSBRapidCharge();
    void setUSBRapidCharge(quint32);
    quint32 getSleepMusic();
    void setSleepMusic(quint32);
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
    void setCoolingMethod(quint32);
    quint32 getUSBLegacyEmulation();
    void setUSBLegacyEmulation(quint32);
    quint32 getBuiltInLAN();
    void setBuiltInLAN(quint32);
    quint32 getSATAInterfaceSetting();
    void setSATAInterfaceSetting(quint32);
    quint32 getBootSpeed();
    void setBootSpeed(quint32);
    quint32 getODDPower();
    void setODDPower(quint32);
    quint32 getPowerOnDisplay(int *, int *, int *);
    void setPowerOnDisplay(quint32);

Q_SIGNALS:
    void touchpadToggled(int);

private:
    void printSMMError(const char *, quint32);

    SMMRegisters regs;

    QMap<int, QString> m_errors;
    QFile m_file;
    bool m_devDeviceExist;
};

#endif // KTOSHIBAHARDWARE_H
