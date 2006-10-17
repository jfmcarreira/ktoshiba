/***************************************************************************
 *   Copyright (C) 2006 by Azael Avalos                                    *
 *   coproscefalo@gmail.com                                                *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef KTOSHIBA_OMNIBOOKINTERFACE_H
#define KTOSHIBA_OMNIBOOKINTERFACE_H

#include <qobject.h>
#include <qstring.h>

#define OMNI_ROOT		"/proc/omnibook"
#define OMNI_DMI		"/proc/omnibook/dmi"
#define OMNI_LCD		"/proc/omnibook/lcd"
#define OMNI_ONETOUCH	"/proc/omnibook/onetouch"
#define OMNI_FAN		"/proc/omnibook/fan"
#define OMNI_BLANK		"/proc/omnibook/blank"
#define OMNI_TOUCHPAD 	"/proc/omnibook/touchpad"
#define OMNI_WIFI		"/proc/omnibook/wifi"
#define OMNI_BLUETOOTH	"/proc/omnibook/bluetooth"

// ECTYPE
#define NONE			0
#define XE3GF			1
#define TSP10			11
#define AMILOD			10
#define TSM30X			12
#define TSM40			13
#define TSA105			14

/**
 * @short Provides access to /proc/omnibook files
 * @author Azael Avalos <coproscefalo@gmail.com>
 * @version 0.1
 */
class KToshibaOmnibookInterface : public QObject
{
    Q_OBJECT
public:
    KToshibaOmnibookInterface(QObject *parent = 0);
    ~KToshibaOmnibookInterface();
    /**
     * Checks /proc entry for Toshiba model
     * @return @p true if found, false otherwise
     */
    bool checkOmnibook();
    /**
     * Gets the machine BIOS version
     * @return @p the int holding the BIOS version
     */
    int machineBIOS();
    /**
     * Gets the machine model name
     * @return @p the string holding the model name
     */
    QString modelName();
    /**
     * Gets the machine ectype
     * @return @p the int holding the ectype
     */
    int ecType();
    /**
     * Checks /proc entry for battery status.
     * @param time the int to hold the current time
     * @param perc the int to hold the current percent
     */
    void batteryStatus(int *time, int *perc);
    /**
     * Checks /proc entry for AC adapter status.
     * @return @p value holding the AC adaptor status
     */
    int omnibookAC();
    /**
     * Checks /proc entry for brightness status.
     * @return @p value holding the brightness status
     */
    int getBrightness();
    /**
     * Set the display brightness.
     * @param bright the int to set the brightness
     */
    void setBrightness(int bright);
    /**
     * Checks /proc entry for OneTouch buttons status.
     * @return @p the int holding the current status
     */
    int getOneTouch();
    /**
     * Enables/Disables OneTouch buttons.
     * @param state the int to turn on/off OneTouch buttons
     */
    void setOneTouch(int state);
    /**
     * Checks /proc entry for fan status.
     * @return @p the int holding the current fan status
     */
    int getFan();
    /**
     * Enables/Disables the fan.
     * @param status the int to turn on/off the fan
     */
    void setFan(int status);
    /**
     * Checks /proc entry for LCD Backlight status
     * @return @p the int holding the current LCD state
     */
    int getLCDBackLight();
    /**
     * Enables/Disables the LCD Backlight
     * @param status the int holding the desired status
     */
    void setLCDBackLight(int status);
    /**
     * Checks /proc entry for TouchPad status
     * @return @p the int holding the current TouchPad status
     */
    int getTouchPad();
    /**
     * Enables/Disables the TouchPad
     * @param status the int holding the desired status
     */
    void setTouchPad(int status);
    /**
     * Checks /proc entry for WiFi switch status
     * @return @p the int holding the current WiFi switch status
     */
    int getWifiSwitch();
    /**
     * Checks /proc entry for WiFi status
     * @return @p the int holding the current WiFi adapter status
     */
    int getWifi();
    /**
     * Enables/Disables the WiFi adapter
     * @param status the int holding the desired status
     */
    void setWifi(int status);
    /**
     * Checks /proc entry for Bluetooth status
     * @return @p the int holding the current WiFi adapter status
     */
    int getBluetooth();
    /**
     * Enables/Disables the Bluetooth adapter
     * @param status the int holding the desired status
     */
    void setBluetooth(int status);
    /**
     *
     */
    int getVideo();
private:
    QString model;
    int mFd;
    int BatteryCap;
    int RemainingCap;
    int ectype;
};

#endif // KTOSHIBA_OMNIBOOKINTERFACE_H
