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

#include "ktoshibasmminterface.h"

#include <kdebug.h>

KToshibaSMMInterface::KToshibaSMMInterface(QObject *parent)
	: QObject( parent ),
	  mFd( 0 ),
	  hotkeys( false )
{
}

bool KToshibaSMMInterface::openInterface()
{
	int version;

	if (SciSupportCheck(&version) == SCI_FAILURE) {
		kdError() << "KToshibaSMMInterface::openInterface(): "
				  << "This computer is not supported "
				  << "or the kernel module is not installed." << endl;
		return false;
	}

	if (!(mFd = open(TOSH_DEVICE, O_RDWR))) {
		kdError() << "KToshibaSMMInterface::openInterface(): "
				  << "Failed to open " << TOSH_DEVICE
				  << ": " << strerror(errno) << "." << endl;
		return false;
	}

	SciOpenInterface();

	return true;
}

void KToshibaSMMInterface::closeInterface()
{
	if (mFd)
		close(mFd);
	SciCloseInterface();
}

int KToshibaSMMInterface::getBrightness()
{
	int bright;

	reg.eax = HCI_GET;
	reg.ebx = HCI_BRIGHTNESS_LEVEL;
	if (HciFunction(&reg) == HCI_SUCCESS) {
		bright = (reg.ecx & 0xffff);
		bright = bright >> HCI_LCD_BRIGHTNESS_SHIFT;
		return bright;
	} else
	if (HciFunction(&reg) == HCI_FAILURE)
		kdError() << "KToshibaSMMInterface::getBrightness(): "
				  << "Failed obtaining brightness" << endl;

	return -1;
}

void KToshibaSMMInterface::setBrightness(int value)
{
	// Check value and set the desired brightness level
	if (value < 0) {
		value = 0;
	} else 
	if (value > HCI_LCD_BRIGHTNESS_LEVELS)
		value = 7;

	if (getBrightness() != -1 && value >= 0 
			&& value < HCI_LCD_BRIGHTNESS_LEVELS) {
		reg.eax = HCI_SET;
		reg.ebx = HCI_BRIGHTNESS_LEVEL;
		value = value << HCI_LCD_BRIGHTNESS_SHIFT;
		reg.ecx = (int) value;

		if (HciFunction(&reg) == HCI_FAILURE)
			kdError() << "KToshibaSMMInterface::setBrightness(): "
					  << "Failed setting brightness" << endl;
	}
}

void KToshibaSMMInterface::batteryStatus(int *time, int *percent)
{
	reg.ebx = SCI_BATTERY_PERCENT;
	if (SciGet(&reg) == SCI_SUCCESS)
		*percent = ((100*(reg.ecx & 0xffff))/(reg.edx & 0xffff));
	else
		*percent = -1;

	reg.ebx = SCI_BATTERY_TIME;
	if (SciGet(&reg) == SCI_SUCCESS)
		*time = (reg.ecx & 0xffff);
	else {
		int hours = *time/60;
		int minutes = *time-(60*hours);
		*time = SCI_TIME(hours, minutes);
	}
}

int KToshibaSMMInterface::acPowerStatus()
{
	int status = 0;

	reg.eax = HCI_GET;
	reg.ebx = HCI_AC_ADAPTOR;
	if (HciFunction(&reg) == HCI_SUCCESS) {
		status = (reg.ecx & 0xffff);
		if (status == 4)
			return 1;
		else if (status == 3)
			return 0;
	}

	kdError() << "KToshibaSMMInterface::acPowerStatus(): "
			  << "Could not get AC Power status" << endl;

	return -1;
}

int KToshibaSMMInterface::procStatus()
{
	int key;
	char buffer[64];

	if (!(str = fopen(TOSH_PROC, "r")))
		return -1;

	fgets(buffer, sizeof(buffer)-1, str);
	buffer[sizeof(buffer)-1] = '\0';
	sscanf(buffer, "%*s %*x %*d.%*d %*d.%*d %*x %x\n", &key);
	fclose(str);

	return key;
}

int KToshibaSMMInterface::systemEvent()
{
	reg.eax = HCI_GET;
	reg.ebx = HCI_SYSTEM_EVENT;
	if (HciFunction(&reg) == HCI_SUCCESS) {
		return (int) (reg.ecx & 0xffff);
	} else
	if (HciFunction(&reg) == HCI_FIFO_EMPTY) {
		kdDebug() << "KToshibaSMMInterface::systemEvent(): "
				  << "FIFO Empty" << endl;
		return 0;
	} else
	if (HciFunction(&reg) == HCI_NOT_SUPPORTED) {
		reg.eax = HCI_SET;
		reg.ebx = HCI_SYSTEM_EVENT;
		reg.ecx = HCI_ENABLE;
		HciFunction(&reg);
		kdDebug() << "KToshibaSMMInterface::systemEvent(): "
				  << "Re-enabled Hotkeys" << endl;
		hotkeys = true;
		return 1;
	} else
	if (HciFunction(&reg) == HCI_FAILURE) {
		/**
		 *	ISSUE: After enabling the hotkeys again, we receice
		 *	HCI_FAILURE when 'no' events are present in the
		 *	system. However, when events are entered into
		 *	the system we receive HCI_SUCCESS. In the meanwhile
		 *	this is a workaround for this problem?
		 */
		if (!hotkeys) {
			reg.eax = HCI_SET;
			reg.ebx = HCI_SYSTEM_EVENT;
			reg.ecx = HCI_ENABLE;
			HciFunction(&reg);
			kdDebug() << "KToshibaSMMInterface::systemEvent(): "
					  << "Re-enabled Hotkeys" << endl;
			hotkeys = true;
		}
		return 1;
	}

	return -1;
}

int KToshibaSMMInterface::machineID()
{
	int id;

	if (HciGetMachineID(&id) == HCI_SUCCESS)
		return id;

	return -1;
}

void KToshibaSMMInterface::pointingDevice(bool status)
{
	if (status == false) {
		reg.ebx = SCI_POINTING_DEVICE;
		reg.ecx = SCI_DISABLED;
		if (SciSet(&reg) == SCI_FAILURE) {
			kdError() << "KToshibaSMMInterface::pointingDevice(): "
					  << "Could not disable MousePad" << endl;
			return;
		}
	} else if (status == true) {
		reg.ebx = SCI_POINTING_DEVICE;
		reg.ecx = SCI_ENABLED;
		if (SciSet(&reg) == SCI_FAILURE) {
			kdError() << "KToshibaSMMInterface::pointingDevice(): "
					  << "Could not enable MousePad" << endl;
			return;
		}
	}
}

int KToshibaSMMInterface::getBatterySaveMode()
{
	reg.ebx = SCI_BATTERY_SAVE;
	if (SciGet(&reg) == SCI_SUCCESS)
		return (int) (reg.ecx & 0xffff);
	else
		kdError() << "KToshibaSMMInterface::getBatterySaveMode(): "
				  << "Could not get battery save mode state" << endl;

	return -1;
}

void KToshibaSMMInterface::setBatterySaveMode(int mode)
{
	reg.ebx = SCI_BATTERY_SAVE;
	if (mode == 0)
		reg.ecx = SCI_USER_SETTINGS;
	else if (mode == 1)
		reg.ecx = SCI_LOW_POWER;
	else if (mode == 2)
		reg.ecx = SCI_FULL_POWER;
	else if (mode == 3)
		reg.ecx = SCI_FULL_LIFE;
	if (SciSet(&reg) == SCI_SUCCESS) {
		kdDebug() << "KToshibaSMMInterface::setBatterySaveMode(): "
				  << "Successfully changed to mode " << mode << endl;
	}
	else {
		kdError() << "KToshibaSMMInterface::setBatterySaveMode(): "
				  << "Could not change battery save mode" << endl;
	}
}

int KToshibaSMMInterface::getVideo()
{
	reg.eax = HCI_GET;
	reg.ebx = HCI_VIDEO_OUT;
	if (HciFunction(&reg) == HCI_SUCCESS) 
		return (reg.ecx & 0xffff);
	else
		kdError() << "KToshibaSMMInterface::getVideo(): "
				  << "Could not get display state" << endl;

	return -1;
}

void KToshibaSMMInterface::setVideo(int vid)
{
	reg.eax = HCI_SET;
	reg.ebx = HCI_VIDEO_OUT;
	if (vid == 1)
		reg.ecx = 0x0001;	// LCD
	else if (vid == 2)
		reg.ecx = 0x0002;	// CRT
	else if (vid == 3)
		reg.ecx = 0x0003;	// LCD/CRT
	else if (vid == 4)
		reg.ecx = 0x0004;	// TV
	if (HciFunction(&reg) == HCI_SUCCESS)
		kdDebug() << "KToshibaSMMInterface::setVideo(): "
				  << "Display state changed successfuly" << endl;
	else
		kdError() << "KToshibaSMMInterface::setVideo(): "
				  << "Could not change display state" << endl;
}

void KToshibaSMMInterface::selectBayLock(int *lock)
{
	reg.eax = HCI_GET;
	reg.ebx = HCI_LOCK_STATUS;
	reg.ecx = HCI_SELECT_INT;
	if (HciFunction(&reg) == HCI_SUCCESS) {
		if ((reg.ecx == HCI_UNLOCKED) && (*lock == HCI_LOCKED)) {
			kdDebug() << "KToshibaSMMInterface::selectBayLock(): "
					  << "SelectBay unlocked" << endl;
			*lock = HCI_UNLOCKED;
		} else if ((reg.ecx == HCI_LOCKED) && (*lock == HCI_UNLOCKED)) {
			kdDebug() << "KToshibaSMMInterface::selectBayLock(): "
					  << "SelectBay locked" << endl;
			*lock = HCI_LOCKED;
		}
	}
	else
		kdError() << "KToshibaSMMInterface::selectBayLock(): "
				  << "Could not get SelectBay lock status" << endl;
}

int KToshibaSMMInterface::selectBayStatus()
{
	reg.eax = HCI_GET;
	reg.ebx = HCI_SELECT_STATUS;
	if (HciFunction(&reg) == HCI_SUCCESS) {
		return reg.ecx;
	}
	else
		kdError() << "KToshibaSMMInterface::selectBayStatus(): "
				  << "Could not get SelectBay device" << endl;

	return -1;
}


#include "ktoshibasmminterface.moc"
