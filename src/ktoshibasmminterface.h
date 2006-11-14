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

#ifndef KTOSHIBA_SMMINTERFACE_H
#define KTOSHIBA_SMMINTERFACE_H

#include <qobject.h>

extern "C"
{
#include <fcntl.h>
#include <linux/types.h>

  // Taken from toshutils
#include "sci.h"
#include "hci.h"
}

// Taken from toshiba_acpi
#define HCI_LCD_BRIGHTNESS_BITS		3
#define HCI_LCD_BRIGHTNESS_SHIFT	(16 - HCI_LCD_BRIGHTNESS_BITS)
#define HCI_LCD_BRIGHTNESS_LEVELS	(1 << HCI_LCD_BRIGHTNESS_BITS)

/**
 * @short Provides access to the SMM functions
 * @author Azael Avalos <coproscefalo@gmail.com>
 * @version 0.10
 */
class KToshibaSMMInterface : public QObject
{
  Q_OBJECT
public:
  KToshibaSMMInterface(QObject *parent = 0);
  ~KToshibaSMMInterface();

  /**************************************
   *           SCI Functions            *
   * (Software Configuration Interface) *
   **************************************/
  /**
   * Opens an SCI interface.
   * @return @p true when opened interface
   */
  bool openSCIInterface(int *err);
  /**
   * Closes any opened SCI interface.
   */
  void closeSCIInterface();
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
   * Gets the Battery Save Mode type.
   * @return @p the int holding the BSM type
   */
  int getBatterySaveModeType();
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
   * Sets the Display Auto Off feature to the desired time.
   * @param time the int holding the desired time
   */
  void setDisplayAutoOff(int time);
  /**
   * Sets the HardDrive Auto Off feature to the desired time.
   * @param time the int holding the desired time
   */
  void setHDDAutoOff(int time);
  /**
   * Gets the current speaker volume.
   * @return @p the int holding the current volume
   */
  int getSpeakerVolume();
  /**
   * Sets the speaker to the desired value.
   * @param vol the int holding the desired volume
   */
  void setSpeakerVolume(int vol);
  /**
   * Get the Battery status.
   * @param time the int holding the battery time left
   * @param percent the int holding the percent charged
   */
  void batteryStatus(int *time, int *percent);
  /**
   * Gets the current system boot method.
   * @return @p the int holdint the current boot method
   */
  int getBootMethod();
  /**
   * Sets the system to the desired boot method.
   * @param method the int holding the current boot method
   */
  void setBootMethod(int method);
  /**
   * Sets the fan to the desired Cooling Method.
   * @param method the int holding the desired cooling method
   */
  void setCoolingMethod(int method);
  /**
   * Gets the current ATA Priority state.
   * @return @p the int holding the current state
   */
  int getATAPriority(int index);
  /**
   * Sets the ATA Priority to the desired index.
   * @param index the int holding the desired index
   */
  void setATAPriority(int index);
  /**
   * Gets the Device Configuration.
   * @return @p the int holding the device configuration
   */
  int getDeviceConfig();
  /**
   * Gets the Device Configuration.
   * @param mode the int holding the desired device config.
   */
  void setDeviceConfig(int mode);
  /**
   * Gets the system boot method supported.
   * @return @p the int holding the method supported
   */
  int getBootType();
  /**
   * Gets the current LAN controller state
   * @return @p the int holding the LAN controller state
   */
  int getLANController();
  /**
   * Sets the LAN controller On/Off
   * @param state the int holding the desired state
   */
  void setLANController(int state);
  /**
   * Checks the availability of SpeedStep Technology.
   * @return @p the int holding the current mode
   */
  int getSpeedStep();
  /**
   * Sets the SpeedStep mode of operation.
   * @param mode the int holding the desired mode
   */
  void setSpeedStep(int mode);
  /**
   * Gets the Sound Logo mode.
   * @return @p the int holding the current mode
   */
  int getSoundLogo();
  /**
   * Sets the Sound Logo mode.
   * @param mode the int holding the desired mode
   */
  void setSoundLogo(int state);
  /**
   * Gets the Power Button Lamp state.
   * @return @p the int holding the current state
   */
  int getPowerButtonLamp();
  /**
   * Sets the Power Button Lamp state.
   * @param mode the int holding the desired state
   */
  void setPowerButtonLamp(int mode);
  /**
   * ets the Start Up Logo state.
   * @return @p the int holding the current state
   */
  int getStartUpLogo();
  /**
   * Sets the Start Up Logo state.
   * @param mode the int holding the desired state
   */
  void setStartUpLogo(int mode);
  /**
   * Checks the availability of Hyper-Threading Thechnology.
   * @return @p the int holding the current status
   */
  int getHyperThreading();
  /**
   * Enables/Disables the Hyper-Threading operation.
   * @param status the int holding the current status
   */
  void setHyperThreading(int status);
  /**
   * Gets the Power Switch state.
   * @return @p the int holding the current status
   */
  int getPowerSW();
  /**
   * Sets the Power Switch state.
   * @param status the int holding the current state
   */
  void setPowerSW(int mode);
  /**
   * Gets the Display Device mode.
   * @return @p the int holding the current mode
   */
  int getDisplayDevice();
  /**
   * Sets the Display Device mode.
   * @param status the int holding the current status
   */
  void setDisplayDevice(int mode);
  /**
   * Gets the Parallel Port mode.
   * @return @p the int holding the current mode
   */
  int getParallelPort();
  /**
   * Sets the Parallel Port mode.
   * @param status the int holding the current mode
   */
  void setParallelPort(int mode);
  /**
   * Gets the Keyboard Type state.
   * @return @p the int holding the current status
   */
  int getKeyboardType();
  /**
   * Sets the Keyboard Type state.
   * @param status the int holding the current status
   */
  void setKeyboardType(int mode);
  /**
   * Gets the status of the mouse pad.
   * @return @p the int holding the status of the MousePad
   */
  int getPointingDevice();
  /**
   * Enables/Disables the MousePad.
   * @param status the int to activate/deactivate the MousePad
   */
  void setPointingDevice(int status);
  /**
   * Gets the USB Legacy Support state.
   * @return @p the int holding the current state
   */
  int getUSBLegacySupport();
  /**
   * Sets the USB Legacy Support state.
   * @param status the int holding the current state
   */
  void setUSBLegacySupport(int state);
  /**
   * Gets the USB Floppy Emulation state.
   * @return @p the int holding the current state
   */
  int getUSBFDDEmulation();
  /**
   * Sets the USB Floppy Emulation state.
   * @param status the int holding the current state
   */
  void setUSBFDDEmulation(int state);
  /**
   * Gets the Wake On LAN status.
   * @return @p the int holding the current status
   */
  int getWOL();
  /**
   * Sets the Wake On LAN status.
   * @param status the int holding the current status
   */
  void setWOL(int state);
  /**
   * Gets the Remote Boot Protocol mode.
   * @return @p the int holding the current mode
   */
  int getRemoteBootProtocol();
  /**
   * Sets the Remote Boot Protocol mode.
   * @param status the int holding the current mode
   */
  void setRemoteBootProtocol(int mode);

  /**************************************
   *           HCI Functions            *
   * (Hardware Configuration Interface) *
   **************************************/
  /**
   * Gets the machine BIOS version.
   * @return @p value holding the BIOS version
   */
  int machineBIOS();
  /**
   * Gets the machine ID number.
   * @return @p value holding the machine id
   */
  int machineID();
  /**
   * Gets the current state of the LCD backlight.
   * @return @p the int holding the current state
   */
  int getBackLight();
  /**
   * Sets the LCD backlight On/Off.
   * @param state the int holding the desired state
   */
  void setBackLight(int state);
  /**
   * Get the AC status.
   * @return @p four when connected, three disconnected
   */
  int acPowerStatus();
  /**
   * Gets the current fan status.
   * @return @p the int holdint the current fan state
   */
  int getFan();
  /**
   * Sets the fan On/Off.
   * @param state the int holding the desired state
   */
  void setFan(int state);
  /**
   * Gets the device attached to the desired bay.
   * @param bay the int holding the desired bay
   * @return @p the int holding the current device
   */
  int getBayDevice(int bay);
  /**
   * Checks System Event FIFO for hotkeys.
   * @return @p the value holding the hotkey id
   */
  int getSystemEvent();
  /**
   * Enables the System Events
   * @return @p true if successful, false otherwise
   */
  bool enableSystemEvent();
  /**
   * Disables the System Events
   */
  void disableSystemEvent();
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
  void getSystemLocks(int *lock, int bay);
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
   * Verifies the wireless antenna switch.
   * @return @p the int holding the status
   */
  int getWirelessSwitch();
  /**
   * Gets the current wireless power state.
   * @return @p the int holding the current state
   */
  int getWirelessPower();
  /**
   * Sets the wireless device to on/off.
   * @param state the int holding the desired state
   */
  void setWirelessPower(int state);
  /**
   * Verifies the Bluetooth device existence.
   * @return @p the int holding the device status
   */
  int getBluetooth();
  /**
   * Gets the current Bluetooth power state.
   * @return @p the int holding the current state
   */
  int getBluetoothPower();
  /**
   * Sets the Bluetooth device on/off.
   * @param state the int holding the desired state
   */
  void setBluetoothPower(int state);
public:
  bool mHotkeys;
  int sciversion;
private:
  /**
   * Attach/detach the Bluetooth device control.
   * @param state the int holding the desired state
   */
  void setBluetoothControl(int state);
  /**
   * Gets the maximum ATA Priority index.
   * @return @p the int holding the maximum index
   */
  int getATAPriorityIndex();
  /**
   * Sets the SCI error string.
   * @return @p the QString with the error
   */
   QString sciError(int err);
  /**
   * Sets the HCI error string.
   * @return @p the QString with the error
   */
   QString hciError(int err);
private:
  SMMRegisters reg;
  QString mError;
  int mFd;
};

#endif // KTOSHIBA_SMMINTERFACE_H
