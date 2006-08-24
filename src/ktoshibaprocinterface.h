/***************************************************************************
 *   Copyright (C) 2004-2006 by Azael Avalos                               *
 *   coproscefalo@gmail.com                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
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

#ifndef KTOSHIBA_PROCINTERFACE_H
#define KTOSHIBA_PROCINTERFACE_H

#include <qobject.h>
#include <qstring.h>

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
}

#define TOSH_PROC		"/proc/toshiba"
#define OMNI_ROOT		"/proc/omnibook"
#define OMNI_DMI		"/proc/omnibook/dmi"
#define OMNI_LCD		"/proc/omnibook/lcd"
#define OMNI_ONETOUCH	"/proc/omnibook/onetouch"
#define OMNI_FAN 		"/proc/omnibook/fan"
#define OMNI_BLANK 		"/proc/omnibook/blank"
#define OMNI_TOUCHPAD 	"/proc/omnibook/touchpad"
#define ACPI_ROOT 		"/proc/acpi"

// ECTYPE
#define NONE			0
#define XE3GF			1
#define TSP10			11
#define TSM30X			12
#define TSM40			13
#define TSA105			14

/**
 * @short Provides access to /proc files
 * @author Azael Avalos <coproscefalo@gmail.com>
 * @version 0.3
 */
class KToshibaProcInterface : public QObject
{
    Q_OBJECT
public:
    KToshibaProcInterface(QObject *parent = 0);
    ~KToshibaProcInterface();

    /**
    * Checks /proc entry for Toshiba model
    * @return @p true if found, false otherwise
    */
    bool checkOmnibook();
    /**
     * Gets the machine model name
     * @return @p the string holding the model name
     */
    QString omnibookModelName();
    /**
     * Gets the machine ectype
     * @return @p the int holding the ectype
     */
    int omnibookECType();
    /**
    * Checks /proc entry for battery status.
    * @param time the int to hold the current time
    * @param perc the int to hold the current percent
    */
    void omnibookBatteryStatus(int *time, int *perc);
    /**
    * Checks /proc entry for AC adapter status.
    * @return @p value holding the AC adaptor status
    */
    int omnibookAC();
    /**
    * Checks /proc entry for brightness status.
    * @return @p value holding the brightness status
    */
    int omnibookGetBrightness();
    /**
    * Set the display brightness.
    * @param bright the int to set the brightness
    */
    void omnibookSetBrightness(int bright);
    /**
    * Checks /proc entry for OneTouch buttons status.
    * @return @p the int holding the current status
    */
    int omnibookGetOneTouch();
    /**
    * Enables/Disables OneTouch buttons.
    * @param state the int to turn on/off OneTouch buttons
    */
    void omnibookSetOneTouch(int state);
    /**
    * Checks /proc entry for fan status.
    * @return @p the int holding the current fan status
    */
    int omnibookGetFan();
    /**
    * Enables/Disables the fan.
    * @param status the int to turn on/off the fan
    */
    void omnibookSetFan(int status);
    /**
    * Checks /proc entry for LCD Backlight status
    * @return @p the int holding the current LCD state
    */
    int omnibookGetLCDBackLight();
    /**
    * Enables/Disables the LCD Backlight
    * @param status the int holding the desired status
    */
    void omnibookSetLCDBackLight(int status);
    /**
    * Checks /proc entry for TouchPad status
    * @return @p the int holding the current TouchPad status
    */
    int omnibookGetTouchPad();
    /**
    * Enables/Disables the TouchPad
    * @param status the int holding the desired status
    */
    void omnibookSetTouchPad(int status);
    /**
    *
    */
    int omnibookGetVideo();
    /**
    * Checks /proc entry for battery status.
    * @param time the int to hold the current time
    * @param perc the int to hold the current percent
    */
    void acpiBatteryStatus(int *time, int *perc);
    /**
    * Checks /proc entry for the AC adaptor status.
    * @return @p value holding the AC adaptor status
    */
    int acpiAC();
    /**
    * Checks /proc entry for hotkeys.
    * @return @p value holding the Fn-Key combo id
    */
    int toshibaProcStatus();
private:
    FILE *str;
    QString model;
    int mFd;
    int BatteryCap;
    int RemainingCap;
    int ectype;
};

#endif // KTOSHIBA_PROCINTERFACE_H
