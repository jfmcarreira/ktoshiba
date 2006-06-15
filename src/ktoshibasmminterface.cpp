/***************************************************************************
 *   Copyright (C) 2004-2006 by Azael Avalos                               *
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

#include "ktoshibasmminterface.h"

#include <kdebug.h>

KToshibaSMMInterface::KToshibaSMMInterface(QObject *parent)
	: QObject( parent ),
	  hotkeys( false ),
	  mFd( 0 )
{
}

bool KToshibaSMMInterface::openSCIInterface()
{
	if (SciSupportCheck(&sciversion) == SCI_FAILURE) {
		kdError() << "KToshibaSMMInterface::openSCIInterface(): "
			  << "This computer is not supported or "
			  << "the kernel module is not installed." << endl;
		return false;
	}

	if (!(mFd = open(TOSH_DEVICE, O_RDWR))) {
		kdError() << "KToshibaSMMInterface::openSCIInterface(): "
			  << "Failed to open " << TOSH_DEVICE << endl;
		return false;
	}

	SciOpenInterface();
	SciCloseInterface();
	if (SciOpenInterface() == SCI_FAILURE) {
		kdError() << "KToshibaSMMInterface::openSCIInterface(): "
			  << "Failed to open SCI interface" << endl;
		return false;
	}

	return true;
}

void KToshibaSMMInterface::closeSCIInterface()
{
	if (mFd)
		close(mFd);
	if (SciCloseInterface() == SCI_FAILURE)
		kdError() << "KToshibaSMMInterface::closeSCIInterface(): "
			  << "Failed to close SCI interface" << endl;
}

int KToshibaSMMInterface::getBrightness()
{
	int bright;

	reg.eax = HCI_GET;
	reg.ebx = HCI_BRIGHTNESS_LEVEL;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (HciFunction(&reg) != HCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getBrightness(): "
			  << "Failed obtaining brightness" << endl;
		return -1;
	}
	else {
		bright = (reg.ecx & 0xffff);
		bright = bright >> HCI_LCD_BRIGHTNESS_SHIFT;
	}

	return bright;
}

void KToshibaSMMInterface::setBrightness(int value)
{
	if (value < 0 || value > HCI_LCD_BRIGHTNESS_LEVELS)
		value = ((value < 0)? 0 : 7);

	if (getBrightness() != -1 && value >= 0 &&
		    value < HCI_LCD_BRIGHTNESS_LEVELS) {
		reg.eax = HCI_SET;
		reg.ebx = HCI_BRIGHTNESS_LEVEL;
		value = value << HCI_LCD_BRIGHTNESS_SHIFT;
		reg.ecx = (int) value;
		reg.edx = 0x0000;

		if (HciFunction(&reg) != HCI_SUCCESS)
			kdError() << "KToshibaSMMInterface::setBrightness(): "
				  << "Failed setting brightness" << endl;
	}
}

void KToshibaSMMInterface::batteryStatus(int *time, int *percent)
{
	reg.ebx = SCI_BATTERY_PERCENT;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS)
		*percent = -1;
	else
		*percent = ((100 * (reg.ecx & 0xffff)) / (reg.edx & 0xffff));

	reg.ebx = SCI_BATTERY_TIME;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		int hours = *time/60;
		int minutes = *time-(60*hours);
		*time = SCI_TIME(hours, minutes);
	}
	else
		*time = (reg.ecx & 0xffff);
}

int KToshibaSMMInterface::acPowerStatus()
{
	reg.eax = HCI_GET;
	reg.ebx = HCI_AC_ADAPTOR;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (HciFunction(&reg) != HCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::acPowerStatus(): "
			  << "Could not get AC Power status" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xffff);
}

int KToshibaSMMInterface::getSystemEvent()
{
	reg.eax = HCI_GET;
	reg.ebx = HCI_SYSTEM_EVENT;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	int ev = HciFunction(&reg);
	if ((ev == HCI_FAILURE) || (ev == HCI_NOT_SUPPORTED)) {
		/**
		 *	ISSUE: After enabling the hotkeys again, we receive
		 *	HCI_FAILURE when 'no' events are present in the
		 *	system. However, when events are entered into
		 *	the system we receive HCI_SUCCESS.
		 */
		if (hotkeys == false) {
			kdError() << "KToshibaSMMInterface::getSystemEvent(): "
				  << "Failed accessing System Events" << endl;
		}
		return 1;
	} else
	if (ev == HCI_FIFO_EMPTY)
		return 0;

	return (int) (reg.ecx & 0xffff);
}

void KToshibaSMMInterface::enableSystemEvent()
{
	reg.eax = HCI_SET;
	reg.ebx = HCI_SYSTEM_EVENT;
	reg.ecx = HCI_ENABLE;
	reg.edx = 0x0000;
	if (HciFunction(&reg) != HCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::enableSystemEvent(): "
			  << "Could not enable Hotkeys" << endl;
		return;
	}

	kdDebug() << "KToshibaSMMInterface::enableSystemEvent(): "
		  << "Enabled Hotkeys" << endl;
}

int KToshibaSMMInterface::machineID()
{
	int id;

	if (HciGetMachineID(&id) != HCI_SUCCESS)
		return -1;

	return id;
}

int KToshibaSMMInterface::getPointingDevice()
{
	reg.ebx = SCI_POINTING_DEVICE;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getPointingDevice(): "
			  << "Could not get pointing device status" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xffff);
}

void KToshibaSMMInterface::setPointingDevice(int status)
{
	reg.ebx = SCI_POINTING_DEVICE;
	if (status)
		reg.ecx = SCI_ENABLED;
	else if (!status)
		reg.ecx = SCI_DISABLED;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS)
		kdError() << "KToshibaSMMInterface::pointingDevice(): "
			  << "Could not " << ((status == 1)? "enable"
				 : "disable") << " MousePad" << endl;
}

int KToshibaSMMInterface::getBatterySaveMode()
{
	reg.ebx = SCI_BATTERY_SAVE;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getBatterySaveMode(): "
			  << "Could not get battery save mode state" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xffff);
}

void KToshibaSMMInterface::setBatterySaveMode(int mode)
{
	reg.ebx = SCI_BATTERY_SAVE;
	if (mode == 0)
		reg.ecx = SCI_USER_SETTINGS;
	else if (mode == 1)
		reg.ecx = SCI_LOW_POWER;	// SCI_LONG_LIFE
	else if (mode == 2)
		reg.ecx = SCI_FULL_POWER;	// SCI_NORMAL_LIFE
	else if (mode == 3)
		reg.ecx = SCI_FULL_LIFE;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS)
		kdError() << "KToshibaSMMInterface::setBatterySaveMode(): "
			  << "Could not change Battery Save Mode" << endl;
}

int KToshibaSMMInterface::getVideo()
{
	reg.eax = HCI_GET;
	reg.ebx = HCI_VIDEO_OUT;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (HciFunction(&reg) != HCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getVideo(): "
			  << "Could not get display state" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xff);
}

void KToshibaSMMInterface::setVideo(int vid)
{
	reg.eax = HCI_SET;
	reg.ebx = HCI_VIDEO_OUT;
	if (vid == 0)
		reg.ecx = HCI_INTERNAL;
	else if (vid == 1)
		reg.ecx = HCI_LCD;	// HCI_EXTERNAL
	else if (vid == 2)
		reg.ecx = HCI_CRT;	// HCI_SIMULTANEOUS
	else if (vid == 3)
		reg.ecx = HCI_LCD_CRT;
	else if (vid == 4)
		reg.ecx = HCI_S_VIDEO;
	reg.edx = 0x0000;
	if (HciFunction(&reg) != HCI_SUCCESS)
		kdError() << "KToshibaSMMInterface::setVideo(): "
			  << "Could not change display state" << endl;
}

void KToshibaSMMInterface::getSystemLocks(int *lock, int bay)
{
	reg.eax = HCI_GET;
	reg.ebx = HCI_LOCK_STATUS;
	if (bay == 0)
		reg.ecx = HCI_BUILT_IN;
	else if (bay == 1)
		reg.ecx = HCI_SELECT_INT;
	else if (bay == 2)
		reg.ecx = HCI_SELECT_DOCK;
	else if (bay == 3)
		reg.ecx = HCI_5INCH_DOCK;
	reg.edx = 0x0000;
	if (HciFunction(&reg) != HCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::systemLocks(): "
			  << "Could not get Bay lock status" << endl;
		return;
	}
	if ((reg.ecx == HCI_UNLOCKED) && (*lock == HCI_LOCKED))
		*lock = HCI_UNLOCKED;
	else if ((reg.ecx == HCI_LOCKED) && (*lock == HCI_UNLOCKED))
		*lock = HCI_LOCKED;
	kdDebug() << "KToshibaSMMInterface::systemLocks(): "
		  << "Bay is " 
		  << ((*lock == HCI_LOCKED)? "locked": "unlocked") << endl;
}

int KToshibaSMMInterface::getBayDevice(int bay)
{
	reg.eax = HCI_GET;
	reg.ebx = HCI_SELECT_STATUS;
	if (bay == 0)
		reg.ecx = HCI_BUILT_IN;
	else if (bay == 1)
		reg.ecx = HCI_SELECT_INT;
	else if (bay == 2)
		reg.ecx = HCI_SELECT_DOCK;
	else if (bay == 3)
		reg.ecx = HCI_5INCH_DOCK;
	reg.edx = 0x0000;
	if (HciFunction(&reg) != HCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getBayDevice(): "
			  << "Could not get Bay device or "
			  << "laptop doesn't have one" << endl;
		return -1;
	}

	return (int) reg.ecx;
}

int KToshibaSMMInterface::getWirelessSwitch()
{
	reg.eax = HCI_GET;
	reg.ebx = HCI_RF_CONTROL;
	reg.ecx = 0x0000;
	reg.edx = HCI_WIRELESS_CHECK;
	if (HciFunction(&reg) != HCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getWirelessSwitch(): "
			  << "Could not check wireless switch "
			  << "or system doesn't have one" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xff);
}

int KToshibaSMMInterface::getBluetooth()
{
	reg.eax = HCI_GET;
	reg.ebx = HCI_RF_CONTROL;
	reg.ecx = 0x0000;
	reg.edx = HCI_BLUETOOTH_CHECK;
	if (HciFunction(&reg) != HCI_SUCCESS) {
		kdDebug() << "KToshibaSMMInterface::getBluetooth(): "
			  << "Could not check Bluetooth device "
			  << "or system doesn't have one" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xff);
}

void KToshibaSMMInterface::setBluetoothPower(int state)
{
	if (state == 0) {
		setBluetoothControl(state);
		reg.eax = HCI_SET;
		reg.ebx = HCI_RF_CONTROL;
		reg.ecx = HCI_DISABLE;
		reg.edx = HCI_BLUETOOTH_POWER;
		if (HciFunction(&reg) != HCI_SUCCESS) {
			kdError() << "KToshibaSMMInterface::setBluetooth(): "
				  << "Could not disable Bluetooth device" << endl;
			return;
		}
	} else
	if (state == 1) {
		reg.eax = HCI_SET;
		reg.ebx = HCI_RF_CONTROL;
		reg.ecx = HCI_ENABLE;
		reg.edx = HCI_BLUETOOTH_POWER;
		if (HciFunction(&reg) != HCI_SUCCESS) {
			kdError() << "KToshibaSMMInterface::setBluetooth(): "
				  << "Could not enable Bluetooth device" << endl;
			return;
		}
		setBluetoothControl(state);
	}
	kdDebug() << "KToshibaSMMInterface::setBluetooth(): "
		  << "Bluetooth device "
		  << ((state == 1)? "enabled" : "disabled")
		  << " successfully" << endl;
}

void KToshibaSMMInterface::setProcessingSpeed(int speed)
{
	reg.ebx = SCI_PROCESSING;
	if (speed == 0)
		reg.ecx = SCI_LOW;
	else if (speed == 1)
		reg.ecx = SCI_HIGH;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::setProcessingSpeed(): "
			  << "Could not change processor speed" << endl;
		return;
	}

	kdDebug() << "KToshibaSMMInterface::setProcessingSpeed(): "
		  << "Successfully changed processor to "
		  << ((speed == 0)? "LOW" : "HIGH") << " speed" << endl;
}

void KToshibaSMMInterface::setCPUSleepMode(int mode)
{
	reg.ebx = SCI_SLEEP_MODE;
	if (mode == 0)
		reg.ecx = SCI_DISABLED;
	else if (mode == 1)
		reg.ecx = SCI_ENABLED;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::setCPUSleepMode(): "
			  << "Could not change CPU Sleep Mode" << endl;
		return;
	}
	
	kdDebug() << "KToshibaSMMInterface::setCPUSleepMode(): "
		  << "Successfully " << ((mode == 0)?
			 "DISABLED" : "ENABLED") << " CPU Sleep Mode" << endl;
}

void KToshibaSMMInterface::setCoolingMethod(int method)
{
	reg.ebx = SCI_COOLING_METHOD;
	if (method == 0)
		reg.ecx = SCI_MAX_PERFORMANCE;
	else if (method == 1)
		reg.ecx = SCI_BAT_OPTIMIZED;
	else if (method == 2)
		reg.ecx = SCI_PERFORMANCE_2;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS)
		kdError() << "KToshibaSMMInterface::setCoolingMethod(): "
			  << "Could not change Cooling Method" << endl;
}

void KToshibaSMMInterface::setHDDAutoOff(int time)
{
	reg.ebx = SCI_HDD_AUTO_OFF;
	if (time == 0)
		reg.ecx = SCI_TIME_30;
	else if (time == 1)
		reg.ecx = SCI_TIME_20;
	else if (time == 2)
		reg.ecx = SCI_TIME_15;
	else if (time == 3)
		reg.ecx = SCI_TIME_10;
	else if (time == 4)
		reg.ecx = SCI_TIME_05;
	else if (time == 5)
		reg.ecx = SCI_TIME_03;
	else if (time == 6)
		reg.ecx = SCI_TIME_01;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS)
		kdError() << "KToshibaSMMInterface::setHDDAutoOff(): "
			  << "Could not change HDD Auto Off time" << endl;
}

void KToshibaSMMInterface::setDisplayAutoOff(int time)
{
	reg.ebx = SCI_DISPLAY_AUTO;
	if (time == 0)
		reg.ecx = SCI_TIME_30;
	else if (time == 1)
		reg.ecx = SCI_TIME_20;
	else if (time == 2)
		reg.ecx = SCI_TIME_15;
	else if (time == 3)
		reg.ecx = SCI_TIME_10;
	else if (time == 4)
		reg.ecx = SCI_TIME_05;
	else if (time == 5)
		reg.ecx = SCI_TIME_03;
	else if (time == 6)
		reg.ecx = SCI_TIME_01;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS)
		kdError() << "KToshibaSMMInterface::setDisplayAutoOff(): "
			  << "Could not change Display Auto Off time" << endl;
}

int KToshibaSMMInterface::getSpeedStep()
{
	reg.ebx = SCI_INTEL_SPEEDSTEP;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getSpeedStep(): "
			  << "Could not get SpeedStep mode "
			  << "or system doesn't support it" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xffff);
}

void KToshibaSMMInterface::setSpeedStep(int mode)
{
	reg.ebx = SCI_INTEL_SPEEDSTEP;
	if (mode == 0)
		reg.ecx = SCI_DYNAMICALLY;
	else if (mode == 1)
		reg.ecx = SCI_ALWAYS_HIGH;
	else if (mode == 2)
		reg.ecx = SCI_ALWAYS_LOW;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS)
		kdError() << "KToshibaSMMInterface::setSpeedStep(): "
			  << "Could not change SpeedStep mode" << endl;
}

int KToshibaSMMInterface::getHyperThreading()
{
	reg.ebx = SCI_HYPER_THREADING;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getHyperThreading(): "
			  << "Could not get Hyper-Threading mode "
			  << "or system doesn't support it" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xffff);
}

void KToshibaSMMInterface::setHyperThreading(int status)
{
	reg.ebx = SCI_HYPER_THREADING;
	if (status == 0)
		reg.ecx = SCI_DISABLED;
	else if (status == 1)
		reg.ecx = SCI_ENABLED_PM;
	else if (status == 2)
		reg.ecx = SCI_ENABLED_NO_PM;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS)
		kdError() << "KToshibaSMMInterface::setHyperThreading(): "
			  << "Could not change Hyper-Threading mode" << endl;
}

int KToshibaSMMInterface::getBatterySaveModeType()
{
	reg.ebx = SCI_BATTERY_SAVE;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getBatterySaveModeType(): "
			  << "Could not get Battery Save Mode type" << endl;
		return -1;
	}

	return (int) (reg.edx & 0xffff);
}

int KToshibaSMMInterface::getSpeakerVolume()
{
	reg.ebx = SCI_SPEAKER_VOLUME;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getSpeakerVolume(): "
			  << "Could not get speaker volume" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xffff);
}

void KToshibaSMMInterface::setSpeakerVolume(int vol)
{
	reg.ebx = SCI_SPEAKER_VOLUME;
	if (vol == 0)
		reg.ecx = SCI_VOLUME_OFF;
	else if (vol == 1)
		reg.ecx = SCI_VOLUME_LOW;
	else if (vol == 2)
		reg.ecx = SCI_VOLUME_MEDIUM;
	else if (vol == 3)
		reg.ecx = SCI_VOLUME_HIGH;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS)
		kdError() << "KToshibaSMMInterface::setSpeakerVolume(): "
			  << "Could not change volume" << endl;
}

int KToshibaSMMInterface::getFan()
{
	reg.eax = HCI_GET;
	reg.ebx = HCI_FAN;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (HciFunction(&reg) != HCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getFan(): "
			  << "Could not get fan status or laptop"
			  << " doesn't have one" << endl;
		return -1;
	}

	return (int) reg.ecx;
}

void KToshibaSMMInterface::setFan(int state)
{
	reg.eax = HCI_SET;
	reg.ebx = HCI_FAN;
	if (state == 0)
		reg.ecx = HCI_DISABLE;
	else if (state == 1)
		reg.ecx = HCI_ENABLE;
	reg.edx = 0x0000;
	if (HciFunction(&reg) != HCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::setFan(): "
			  << "Fan could not be turned "
			  << ((state == 1)? "On" : "Off") << endl;
		return;
	}
}

int KToshibaSMMInterface::getBootType()
{
	reg.ebx = SCI_BOOT_PRIORITY;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getBootType(): "
			  << "Could not get supported boot method" << endl;
		return -1;	
	}

	return (int) (reg.edx & 0xffff);
}

int KToshibaSMMInterface::getBootMethod()
{
	reg.ebx = SCI_BOOT_PRIORITY;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getBootMethod(): "
			  << "Could not get current boot method" << endl;
		return -1;	
	}

	return (int) (reg.ecx & 0xffff);
}

void KToshibaSMMInterface::setBootMethod(int method)
{
	reg.ebx = SCI_BOOT_PRIORITY;
	if (method == 0)
		reg.ecx = SCI_FD_HD_CDROM_LAN;
	else if (method == 1)
		reg.ecx = SCI_HD_CDROM_LAN_HD;
	else if (method == 2)
		reg.ecx = SCI_FD_CDROM_LAN_HD;
	else if (method == 3)
		reg.ecx = SCI_CDROM_LAN_HD_FD;
	else if (method == 4)
		reg.ecx = SCI_CDROM_LAN_FD_HD;
	else if (method == 5)
		reg.ecx = SCI_HD_FD_CDROM_LAN;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS)
		kdError() << "KToshibaSMMInterface::setBootMethod(): "
			  << "Could not set boot method" << endl;
}

int KToshibaSMMInterface::getWirelessPower()
{
	reg.eax = HCI_GET;
	reg.ebx = HCI_RF_CONTROL;
	reg.ecx = 0x0000;
	reg.edx = HCI_WIRELESS_CHECK;
	if (HciFunction(&reg) != HCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getWirelessPower(): "
			  << "Could not get wireless power state" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xff);
}

void KToshibaSMMInterface::setWirelessPower(int state)
{
	reg.eax = HCI_SET;
	reg.ebx = HCI_RF_CONTROL;
	if (state == 0)
		reg.ecx = HCI_DISABLE;
	else if (state == 1)
		reg.ecx = HCI_ENABLE;
	reg.edx = HCI_WIRELESS_POWER;
	if (HciFunction(&reg) != HCI_SUCCESS)
		kdError() << "KToshibaSMMInterface::setWirelessPower(): "
			  << "Could not set wireless power "
			  << ((state == 1)? "On" : "Off") << endl;
}

int KToshibaSMMInterface::getBackLight()
{
	reg.eax = HCI_GET;
	reg.ebx = HCI_BACKLIGHT;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (HciFunction(&reg) != HCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getBackLight(): "
			  << "Could not get LCD backlight status" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xff);
}

void KToshibaSMMInterface::setBackLight(int state)
{
	reg.eax = HCI_SET;
	reg.ebx = HCI_BACKLIGHT;
	if (state == 0)
		reg.ecx = HCI_DISABLE;
	else if (state == 1)
		reg.ecx = HCI_ENABLE;
	reg.edx = 0x0000;
	if (HciFunction(&reg) != HCI_SUCCESS)
		kdError() << "KToshibaSMMInterface::setBackLight(): "
			  << "Could not turn LCD backlight " 
			  << ((state == 1)? "On" : "Off") << endl;
}

int KToshibaSMMInterface::getBluetoothPower()
{
	reg.eax = HCI_GET;
	reg.ebx = HCI_RF_CONTROL;
	reg.ecx = 0x0000;
	reg.edx = 0x0001;
	if (HciFunction(&reg) != HCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getBluetoothPower(): "
			  << "Could not get Bluetooth power state" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xff);
}

void KToshibaSMMInterface::setBluetoothControl(int state)
{
	reg.eax = HCI_SET;
	reg.ebx = HCI_RF_CONTROL;
	if (state == 0)
		reg.ecx = HCI_DISABLE;
	else if (state == 1)
		reg.ecx = HCI_ENABLE;
	reg.edx = HCI_BLUETOOTH_CTRL;
	if (HciFunction(&reg) != HCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::setBluetoothControl(): "
			  << "Could not attach/detach Bluetooth device" << endl;
		return;
	}
	
	kdDebug() << "KToshibaSMMInterface::setBluetoothControl(): "
		  << "Bluetooth device "
		  << ((state == 1)? "attached" : "detached")
		  << " successfully" << endl;
}

int KToshibaSMMInterface::getATAPriorityIndex()
{
	reg.ebx = SCI_ATA_PRIORITY;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getATAPriorityIndex(): "
			  << "Could not get current ATA Boot Priority index" << endl;
		return -1;
	}

	return (int) reg.edx;
}

int KToshibaSMMInterface::getATAPriority(int index)
{
	int maxindex = getATAPriorityIndex();
	reg.ebx = SCI_ATA_PRIORITY;
	if (index > maxindex)
		reg.ecx = maxindex;
	else
		reg.ecx = index;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getATAPriority(): "
			  << "Could not get current ATA Boot Priority" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xffff);
}

void KToshibaSMMInterface::setATAPriority(int index)
{
	int maxindex = getATAPriorityIndex();
	reg.ebx = SCI_ATA_PRIORITY;
	if (index > maxindex)
		reg.ecx = maxindex;
	else
		reg.ecx = index;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::setATAPriority(): "
			  << "Could not set desired ATA Boot Priority index" << endl;
		return;
	}
}

int KToshibaSMMInterface::getDeviceConfig()
{
	reg.ebx = SCI_DEVICE_CONFIG;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getDeviceConfig(): "
			  << "Could not get current device configuration mode" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xffff);
}

void KToshibaSMMInterface::setDeviceConfig(int mode)
{
	reg.ebx = SCI_DEVICE_CONFIG;
	if (mode == 0)
		reg.ecx = SCI_DEV_CONFIG_ALL;
	else if (mode == 1)
		reg.ecx = SCI_DEV_CONFIG_OS;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::setDeviceConfig(): "
			  << "Could not set device configuration mode" << endl;
		return;
	}
}

int KToshibaSMMInterface::getPowerButtonLamp()
{
	reg.ebx = SCI_PWR_BUTTON_LAMP;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getPowerButtonLamp(): "
			  << "Could not get current Power Button Lamp mode" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xffff);
}

void KToshibaSMMInterface::setPowerButtonLamp(int mode)
{
	reg.ebx = SCI_PWR_BUTTON_LAMP;
	if (mode == 1)
		reg.ecx = SCI_PWR_BTTN_MODE1;
	else if (mode == 2)
		reg.ecx = SCI_PWR_BTTN_MODE2;
	else if (mode == 3)
		reg.ecx = SCI_PWR_BTTN_MODE3;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::setPowerButtonLamp(): "
			  << "Could not set Power Button Lamp mode" << endl;
		return;
	}
}

int KToshibaSMMInterface::getStartUpLogo()
{
	reg.ebx = SCI_START_UP_LOGO;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getStartUpLogo(): "
			  << "Could not get current Start Up Logo mode" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xffff);
}

void KToshibaSMMInterface::setStartUpLogo(int mode)
{
	reg.ebx = SCI_START_UP_LOGO;
	if (mode == 0)
		reg.ecx = SCI_PICTURE_LOGO;
	else if (mode == 1)
		reg.ecx = SCI_ANIMATION_LOGO;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::setStartUpLogo(): "
			  << "Could not set Start Up Logo mode" << endl;
		return;
	}
}

int KToshibaSMMInterface::getPowerSW()
{
	reg.ebx = SCI_POWER_SW;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getPowerSW(): "
			  << "Could not get current Power SW mode" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xffff);
}

void KToshibaSMMInterface::setPowerSW(int mode)
{
	reg.ebx = SCI_POWER_SW;
	if (mode == 0)
		reg.ecx = SCI_PWS_ENABLED;
	else if (mode == 1)
		reg.ecx = SCI_PWS_DIS_PCLOSE;
	else if (mode == 2)
		reg.ecx = SCI_PWS_DIS_NOAC;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::setPowerSW(): "
			  << "Could not set Power SW mode" << endl;
		return;
	}
}

int KToshibaSMMInterface::getRemoteBootProtocol()
{
	reg.ecx = SCI_REMOTE_BOOT;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getRemoteBootProtocol(): "
			  << "Could not get current Boot Protocol" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xffff);
}

void KToshibaSMMInterface::setRemoteBootProtocol(int mode)
{
	reg.ecx = SCI_REMOTE_BOOT;
	if (mode == 0)
		reg.ecx = SCI_NET_BOOT_PXE;
	else if (mode == 1)
		reg.ecx = SCI_NET_BOOT_RPL;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::setRemoteBootProtocol(): "
			  << "Could not set Boot Protocol" << endl;
		return;
	}
}

int KToshibaSMMInterface::getWOL()
{
	reg.ebx = SCI_WAKE_ON_LAN;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getWOL(): "
			  << "Could not get current WOL mode" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xff);
}

void KToshibaSMMInterface::setWOL(int state)
{
	reg.ebx = SCI_WAKE_ON_LAN;
	if (state == 0)
		reg.ecx = SCI_DISABLED;
	else if (state == 1)
		reg.ecx = SCI_ENABLED;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::setWOL(): "
			  << "WOL could not be "
			  << ((state == 0)? "disabled" : "enabled") << endl;
		return;
	}
}

int KToshibaSMMInterface::getLANController()
{
	reg.ebx = SCI_LAN_CONTROL;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getLANController(): "
			  << "Could not get LAN controller mode "
			  << "or system doesn't have one" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xffff);
}

void KToshibaSMMInterface::setLANController(int state)
{
	reg.ebx = SCI_LAN_CONTROL;
	if (state == 0)
		reg.ecx = SCI_DISABLED;
	else if (state == 1)
		reg.ecx = SCI_ENABLED;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::setLANController(): "
			  << "LAN controller could not be "
			  << ((state == 0)? "disabled" : "enabled") << endl;
		return;
	}
}

int KToshibaSMMInterface::getSoundLogo()
{
	reg.ebx = SCI_SOUND_LOGO;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getSoundLogo(): "
			  << "Could not get Sound Logo state "
			  << "or system doesn't support it" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xffff);
}

void KToshibaSMMInterface::setSoundLogo(int state)
{
	reg.ebx = SCI_SOUND_LOGO;
	if (state == 0)
		reg.ecx = SCI_DISABLED;
	else if (state == 1)
		reg.ecx = SCI_ENABLED;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::setSoundLogo(): "
			  << "Sound Logo could not be "
			  << ((state == 0)? "disabled" : "enabled") << endl;
		return;
	}
}

int KToshibaSMMInterface::getDisplayDevice()
{
	reg.ebx = SCI_DISPLAY_DEVICE;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getDisplayDevice(): "
			  << "Could not get current Display Device mode" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xffff);
}

void KToshibaSMMInterface::setDisplayDevice(int mode)
{
	reg.ebx = SCI_DISPLAY_DEVICE;
	if (mode == 0)
		reg.ecx = SCI_DISPLAY_DEV_DV;
	else if (mode == 1)
		reg.ecx = SCI_DISPLAY_DEV_DO;
	else if (mode == 2)
		reg.ecx = SCI_DISPLAY_DEV_DI;
	else if (mode == 3)
		reg.ecx = SCI_DISPLAY_DEV_DE;
	else if (mode == 4)
		reg.ecx = SCI_DISPLAY_DEV_DS;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::setDisplayDevice(): "
			  << "Could not set Display Device mode" << endl;
		return;
	}
}

int KToshibaSMMInterface::getUSBLegacySupport()
{
	reg.ebx = SCI_USB_LEGACY;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getUSBLegacySupport(): "
			  << "Could not get USB Legacy Support state" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xffff);
}

void KToshibaSMMInterface::setUSBLegacySupport(int state)
{
	reg.ebx = SCI_USB_LEGACY;
	if (state == 0)
		reg.ecx = SCI_DISABLED;
	else if (state == 1)
		reg.ecx = SCI_ENABLED;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::setUSBLegacySupport(): "
			  << "USB Legacy Support "
			  << ((state == 0)? "disabled" : "enabled") << endl;
		return;
	}
}

int KToshibaSMMInterface::getUSBFDDEmulation()
{
	reg.ebx = SCI_USB_FDD_EMUL;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getUSBFDDEmulation(): "
			  << "Could not get USB FDD Legacy state" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xffff);
}

void KToshibaSMMInterface::setUSBFDDEmulation(int state)
{
	reg.ebx = SCI_USB_FDD_EMUL;
	if (state == 0)
		reg.ecx = SCI_DISABLED;
	else if (state == 1)
		reg.ecx = SCI_ENABLED;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::setUSBFDDEmulation(): "
			  << "USB FDD Emulation is "
			  << ((state == 0)? "disabled" : "enabled") << endl;
		return;
	}
}

int KToshibaSMMInterface::getParallelPort()
{
	reg.ebx = SCI_PARALLEL_PORT;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getParallelPort(): "
			  << "Could not get current Parallel Port mode" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xffff);
}

void KToshibaSMMInterface::setParallelPort(int mode)
{
	reg.ebx = SCI_PARALLEL_PORT;
	if (mode == 0)
		reg.ecx = SCI_PARALLEL_ECP;
	else if (mode == 1)
		reg.ecx = SCI_PARALLEL_SPP;
	else if (mode == 2)
		reg.ecx = SCI_PARALLEL_PS2;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::setParallelPort(): "
			  << "Could not set Parallel Port mode" << endl;
		return;
	}
}

int KToshibaSMMInterface::getKeyboardType()
{
	reg.ebx = SCI_KEYBOARD_TYPE;
	reg.ecx = 0x0000;
	reg.edx = 0x0000;
	if (SciGet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::getKeyboardType(): "
			  << "Could not get current Keyboard Type mode" << endl;
		return -1;
	}

	return (int) (reg.ecx & 0xffff);
}

void KToshibaSMMInterface::setKeyboardType(int mode)
{
	reg.ebx = SCI_KEYBOARD_TYPE;
	if (mode == 0)
		reg.ecx = 0x0000;
	else if (mode == 1)
		reg.ecx = 0x0001;
	reg.edx = 0x0000;
	if (SciSet(&reg) != SCI_SUCCESS) {
		kdError() << "KToshibaSMMInterface::setKeyboardType(): "
			  << "Could not set Keyboard Type mode" << endl;
		return;
	}
}


#include "ktoshibasmminterface.moc"
