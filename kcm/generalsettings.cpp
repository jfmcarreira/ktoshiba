/*
   Copyright (C) 2015 Azael Avalos <coproscefalo@gmail.com>

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

#include "generalsettings.h"
#include "systemsettings.h"
#include "ktoshibahardware.h"

GeneralSettings::GeneralSettings(QWidget *parent)
    : QWidget(parent),
      m_sys(qobject_cast<KToshibaSystemSettings * >(QObject::parent()))
{
    setupUi(this);

    m_pointingDeviceSupported = isPointingDeviceSupported();
    m_rapidChargeSupported = isRapidChargeSupported();
    m_usbThreeSupported = isUSBThreeSupported();
    m_usbLegacySupported = isUSBLegacySupported();
    m_builtInLANSupported = isBuiltInLANSupported();
}

bool GeneralSettings::isPointingDeviceSupported()
{
    m_touchpad = m_sys->hw()->getPointingDevice();

    if (m_touchpad != KToshibaHardware::DEACTIVATED && m_touchpad != KToshibaHardware::ACTIVATED)
        return false;

    return true;
}

bool GeneralSettings::isRapidChargeSupported()
{
    m_rapidCharge = m_sys->hw()->getUSBRapidCharge();

    if (m_rapidCharge != KToshibaHardware::DEACTIVATED && m_rapidCharge != KToshibaHardware::ACTIVATED)
        return false;

    return true;
}

bool GeneralSettings::isUSBThreeSupported()
{
    m_usbThree = m_sys->hw()->getUSBThree();

    if (m_usbThree != KToshibaHardware::DEACTIVATED && m_usbThree != KToshibaHardware::ACTIVATED)
        return false;

    return true;
}

bool GeneralSettings::isUSBLegacySupported()
{
    m_usbLegacy = m_sys->hw()->getUSBLegacyEmulation();

    if (m_usbLegacy != KToshibaHardware::DEACTIVATED && m_usbLegacy != KToshibaHardware::ACTIVATED)
        return false;

    return true;
}

bool GeneralSettings::isBuiltInLANSupported()
{
    m_builtInLAN = m_sys->hw()->getBuiltInLAN();

    if (m_builtInLAN != KToshibaHardware::DEACTIVATED && m_builtInLAN != KToshibaHardware::ACTIVATED)
        return false;

    return true;
}

void GeneralSettings::load()
{
    // Pointing Device
    if (m_pointingDeviceSupported)
        pointing_device_checkbox->setChecked(m_touchpad ? true : false);
    else
        pointing_device_checkbox->setEnabled(false);
    // USB Rapid Charge
    if (m_rapidChargeSupported)
        rapid_charge_checkbox->setChecked(m_rapidCharge ? true : false);
    else
        rapid_charge_checkbox->setEnabled(false);
    // USB Three
    if (m_usbThreeSupported)
        usb_three_checkbox->setChecked(m_usbThree ? true : false);
    else
        usb_three_checkbox->setEnabled(false);
    // USB Legacy Emulation
    if (m_usbLegacySupported)
        usb_legacy_checkbox->setChecked(m_usbLegacy ? true : false);
    else
        usb_legacy_checkbox->setEnabled(false);
    // Built In LAN
    if (m_builtInLANSupported)
        built_in_lan_checkbox->setChecked(m_builtInLAN ? true : false);
    else
        built_in_lan_checkbox->setEnabled(false);
}

void GeneralSettings::save()
{
    unsigned int tmp;

    // Pointing Device
    if (m_pointingDeviceSupported) {
        tmp = (pointing_device_checkbox->checkState() == Qt::Checked) ?
              KToshibaHardware::ACTIVATED : KToshibaHardware::DEACTIVATED;
        if (m_touchpad != tmp) {
            m_sys->hw()->setPointingDevice(tmp);
            m_touchpad = tmp;
        }
    }
    // USB Rapid Charge
    if (m_rapidChargeSupported) {
        tmp = (rapid_charge_checkbox->checkState() == Qt::Checked) ?
              KToshibaHardware::ACTIVATED : KToshibaHardware::DEACTIVATED;
        if (m_rapidCharge != tmp) {
            m_sys->hw()->setUSBRapidCharge(tmp);
            m_rapidCharge = tmp;
        }
    }
    // USB Three
    if (m_usbThreeSupported) {
        tmp = (usb_three_checkbox->checkState() == Qt::Checked) ?
              KToshibaHardware::ACTIVATED : KToshibaHardware::DEACTIVATED;
        if (m_usbThree != tmp) {
            m_sys->hw()->setUSBThree(tmp);
            m_usbThree = tmp;
        }
    }
    // USB Legacy Emulation
    if (m_usbLegacySupported) {
        tmp = (usb_legacy_checkbox->checkState() == Qt::Checked) ?
              KToshibaHardware::ACTIVATED : KToshibaHardware::DEACTIVATED;
        if (m_usbLegacy != tmp) {
            m_sys->hw()->setUSBLegacyEmulation(tmp);
            m_usbLegacy = tmp;
        }
    }
    // Built In LAN
    if (m_builtInLANSupported) {
        tmp = (built_in_lan_checkbox->checkState() == Qt::Checked) ?
              KToshibaHardware::ACTIVATED : KToshibaHardware::DEACTIVATED;
        if (m_builtInLAN != tmp) {
            m_sys->hw()->setBuiltInLAN(tmp);
            m_builtInLAN = tmp;
        }
    }
}

void GeneralSettings::defaults()
{
    // Pointing Device
    if (m_pointingDeviceSupported && !m_touchpad)
        pointing_device_checkbox->setChecked(true);
    // USB Rapid Charge
    if (m_rapidChargeSupported && m_rapidCharge)
        rapid_charge_checkbox->setChecked(false);
    // USB Three
    if (m_usbThreeSupported && !m_usbThree)
        usb_three_checkbox->setChecked(true);
    // USB Legacy Emulation
    if (m_usbLegacySupported && !m_usbLegacy)
        usb_legacy_checkbox->setChecked(true);
    // Built In LAN
    if (m_builtInLANSupported && !m_builtInLAN)
        built_in_lan_checkbox->setChecked(true);
}
