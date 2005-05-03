/***************************************************************************
 *   Copyright (C) 2004 by Azael Avalos                                    *
 *   neftali@utep.edu                                                      *
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

#ifndef KTOSHIBA_SMMINTERFACE_H
#define KTOSHIBA_SMMINTERFACE_H

#include <qobject.h>

extern "C" {
#include <fcntl.h>
#include <linux/types.h>

// Taken from toshutils
#include "sci.h"
#include "hci.h"
}

// Taken from toshiba_acpi
#define HCI_LCD_BRIGHTNESS_BITS		3
#define HCI_LCD_BRIGHTNESS_SHIFT	(16-HCI_LCD_BRIGHTNESS_BITS)
#define HCI_LCD_BRIGHTNESS_LEVELS	(1 << HCI_LCD_BRIGHTNESS_BITS)

/**
 * @author Azael Avalos <neftali@utep.edu>
 * @version 0.6
 */
class KToshibaSMMInterface : public QObject
{
	Q_OBJECT
public:
    /**
     * Default Constructor
     */
	KToshibaSMMInterface(QObject *parent = 0);
	/**
	 * Opens an interface to the driver.
	 * @return @p true when opened interface
	 */
	bool openInterface();
	/** Closes any opened interfaces. */
	void closeInterface();
	/**
	 * Return the current display brightness.
	 * @return @p value the current brightness is
	 */
	int getBrightness();
	/**
	 * Set the display brightness.
	 * @param value the int to set brightness to
	 */
	void setBrightness(int value);
	/**
	 * Get the Battery status.
	 * @param time the int holding the battery time left
	 * @param percent the int holding the percent charged
	 */
	void batteryStatus(int *time, int *percent);
	/**
	 * Get the AC status.
	 * @return @p one when connected, zero disconnected
	 */
	int acPowerStatus();
	/**
	 * Checks proc entry for hotkeys.
	 * @return @p value holding the Fn-Key combo id
	 */
	int procStatus();
	/**
	 * Checks System Event FIFO for hotkeys.
	 * @return @p value holding the hotkey id
	 */
	int systemEvent();
	/**
	 * Gets the machine ID number.
	 * @return @p value holding the machine id
	 */
	int machineID();
	/**
	 * Enables/Disables MousePad.
	 * @param status the int to activate/deactivate the mousepad
	 */
	void pointingDevice(int status);
	/**
	 * Gets the current battery save mode.
	 * @return @p value holding the current save mode
	 */
	int getBatterySaveMode();
	/**
	 * Sets the desired battery save mode.
	 * @param mode the int holding the desired battery save mode
	 */
	void setBatterySaveMode(int mode);
	/**
	 * Gets the current video-out setting.
	 * @return @p the int holdint the current display setting
	 */
	int getVideo();
	/**
	 * Sets the desired video-out setting.
	 * @param vid the int holding the desired display setting
	 */
	void setVideo(int vid);
	/**
	 * Gets the status of the selectbay lock.
	 * @param lock the int holding the current lock status
	 * @param bay the int holding the desired select bay
	 */
	void selectBayLock(int *lock, int bay);
	/**
	 * Gets the device attached to the selectbay.
	 * @param bay the int holding the desired select bay
	 * @return @p the int holding the current device
	 */
	int selectBayStatus(int bay);
	/**
	 * Verifies the wireless antenna switch.
	 * @return @p the int holding the status
	 */
	int getWirelessSwitch();
	/**
	 * Verifies the Bluetooth device availability.
	 * @return @p the int holding the device status
	 */
	int getBluetooth();
	/**
	 * Enables the Bluetooth device.
	 */
	void setBluetooth();
	/**
	 * Sets the Processor to the desired speed (high/low).
	 * @param speed the int holding the desired speed
	 */
	void setProcessingSpeed(int speed);
	/**
	 * Sets the CPU Sleep Mode to the desired status (on/off).
	 * @param mode the int holding the desired status
	 */
	void setCPUSleepMode(int mode);
	/**
	 * Sets the fan to the desired Cooling Method.
	 * @param method the int holding the desired cooling method
	 */
	void setCoolingMethod(int method);
	/**
	 * Sets the HardDrive Auto Off feature to the desired time.
	 * @param time the int holding the desired time
	 */
	void setHDDAutoOff(int time);
	/**
	 * Sets the Display Auto Off feature to the desired time.
	 * @param time the int holding the desired time
	 */
	void setDisplayAutoOff(int time);
	/**
	 * Checks the availability of SpeedStep Technology
	 * @return @p the int holding the current mode
	 */
	int getSpeedStep();
	/**
	 * Sets the SpeedStep mode of operation
	 * @param mode the int holding the desired mode
	 */
	void setSpeedStep(int mode);
	/**
	 * Checks the availability of Hyper-Threading Thechnology
	 * @return @p the int holding the current status
	 */
	int getHyperThreading();
	/**
	 * Enables/Disables the Hyper-Threading operation
	 * @param status the int holding the current status
	 */
	void setHyperThreading(int status);
private:
	SMMRegisters reg;
	FILE *str;
	int mFd;
	bool hotkeys;
};

#endif // KTOSHIBA_SMMINTERFACE_H
