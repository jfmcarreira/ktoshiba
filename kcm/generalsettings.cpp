/*
   Copyright (C) 2015-2016 Azael Avalos <coproscefalo@gmail.com>

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

#include <KLocalizedString>

#include <ktoshibahardware.h>

#include "generalsettings.h"
#include "systemsettings.h"

GeneralSettings::GeneralSettings(QWidget *parent)
    : QWidget(parent),
      m_sys(qobject_cast<KToshibaSystemSettings * >(QObject::parent())),
      m_config(KSharedConfig::openConfig(QStringLiteral("ktoshibarc")))
{
    setupUi(this);

    m_pointingDeviceSupported = isPointingDeviceSupported();
    m_rapidChargeSupported = isRapidChargeSupported();
    m_usbThreeSupported = isUSBThreeSupported();
    m_usbLegacySupported = isUSBLegacySupported();
    m_builtInLANSupported = isBuiltInLANSupported();
    m_powerOnDisplaySupported = isPowerOnDisplaySupported();
    m_hdmiCECSupported = isHDMICECSupported();
    if (m_hdmiCECSupported) {
        m_hdmiRemotePowerSupported = isHDMIRemotePowerSupported();
    }

    general = KConfigGroup(m_config, "General");
    if (!general.exists()) {
        general.writeEntry("PointingDevice", 1);
        general.sync();
    }
}

bool GeneralSettings::isPointingDeviceSupported()
{
    m_pointingDevice = m_sys->hw()->getPointingDevice();

    if (m_pointingDevice != KToshibaHardware::DEACTIVATED && m_pointingDevice != KToshibaHardware::ACTIVATED) {
        return false;
    }

    return true;
}

bool GeneralSettings::isRapidChargeSupported()
{
    m_rapidCharge = m_sys->hw()->getUSBRapidCharge();

    if (m_rapidCharge != KToshibaHardware::DEACTIVATED && m_rapidCharge != KToshibaHardware::ACTIVATED) {
        return false;
    }

    return true;
}

bool GeneralSettings::isUSBThreeSupported()
{
    m_usbThree = m_sys->hw()->getUSBThree();

    if (m_usbThree != KToshibaHardware::DEACTIVATED && m_usbThree != KToshibaHardware::ACTIVATED) {
        return false;
    }

    return true;
}

bool GeneralSettings::isUSBLegacySupported()
{
    m_usbLegacy = m_sys->hw()->getUSBLegacyEmulation();

    if (m_usbLegacy != KToshibaHardware::DEACTIVATED && m_usbLegacy != KToshibaHardware::ACTIVATED) {
        return false;
    }

    return true;
}

bool GeneralSettings::isBuiltInLANSupported()
{
    m_builtInLAN = m_sys->hw()->getBuiltInLAN();

    if (m_builtInLAN != KToshibaHardware::DEACTIVATED && m_builtInLAN != KToshibaHardware::ACTIVATED) {
        return false;
    }

    return true;
}

bool GeneralSettings::isPowerOnDisplaySupported()
{
    quint32 result = m_sys->hw()->getPowerOnDisplay(&m_currentDisplayDevice,
                                                    &m_maximumDisplayDevice,
                                                    &m_defaultDisplayDevice);

    if (result != KToshibaHardware::SUCCESS && result != KToshibaHardware::SUCCESS2) {
        return false;
    }

    // NOTE: Some laptop models return success but with zero values
    if (m_currentDisplayDevice == 0 && m_maximumDisplayDevice == 0 && m_defaultDisplayDevice == 0) {
        return false;
    }

    int index = 0;
    if ((m_maximumDisplayDevice & KToshibaHardware::AUTO_DISPLAY) == KToshibaHardware::AUTO_DISPLAY) {
        m_displayDevicesMap[index] = KToshibaHardware::AUTO_DISPLAY;
        m_displayDevices << i18n("AUTO");
        index++;
    }
    if ((m_maximumDisplayDevice & KToshibaHardware::LCD_DISPLAY) == KToshibaHardware::LCD_DISPLAY) {
        m_displayDevicesMap[index] = KToshibaHardware::LCD_DISPLAY;
        m_displayDevices << i18n("Internal LCD");
        index++;
    }
    if ((m_maximumDisplayDevice & KToshibaHardware::RGB_DISPLAY) == KToshibaHardware::RGB_DISPLAY) {
        m_displayDevicesMap[index] = KToshibaHardware::RGB_DISPLAY;
        m_displayDevices << i18n("RGB");
        index++;
    }
    if ((m_maximumDisplayDevice & KToshibaHardware::UNKNOWN_DISPLAY1) == KToshibaHardware::UNKNOWN_DISPLAY1) {
        m_displayDevicesMap[index] = KToshibaHardware::UNKNOWN_DISPLAY1;
        m_displayDevices << i18n("External Unknown 1");
        index++;
    }
    if ((m_maximumDisplayDevice & KToshibaHardware::HDMI_DISPLAY) == KToshibaHardware::HDMI_DISPLAY) {
        m_displayDevicesMap[index] = KToshibaHardware::HDMI_DISPLAY;
        m_displayDevices << i18n("HDMI");
        index++;
    }
    if ((m_maximumDisplayDevice & KToshibaHardware::UNKNOWN_DISPLAY2) == KToshibaHardware::UNKNOWN_DISPLAY2) {
        m_displayDevicesMap[index] = KToshibaHardware::UNKNOWN_DISPLAY2;
        m_displayDevices << i18n("External Unknown 2");
        index++;
    }

    return true;
}

bool GeneralSettings::isHDMICECSupported()
{
    m_hdmiCEC = m_sys->hw()->getHDMICEC();

    if (m_hdmiCEC != KToshibaHardware::DEACTIVATED && m_hdmiCEC != KToshibaHardware::ACTIVATED) {
        return false;
    }

    return true;
}

bool GeneralSettings::isHDMIRemotePowerSupported()
{
    m_hdmiRemotePower = m_sys->hw()->getHDMIRemotePower();

    if (m_hdmiRemotePower != KToshibaHardware::DEACTIVATED && m_hdmiRemotePower != KToshibaHardware::ACTIVATED) {
        return false;
    }

    return true;
}

void GeneralSettings::load()
{
    // Pointing Device
    m_pointingDeviceSupported ? pointing_device_checkbox->setChecked(m_pointingDevice ? true : false) :
        pointing_device_checkbox->setEnabled(false);
    // USB Rapid Charge
    m_rapidChargeSupported ? rapid_charge_checkbox->setChecked(m_rapidCharge ? true : false) :
        rapid_charge_checkbox->setEnabled(false);
    // USB Three
    m_usbThreeSupported ? usb_three_checkbox->setChecked(m_usbThree ? true : false) :
        usb_three_checkbox->setEnabled(false);
    // USB Legacy Emulation
    m_usbLegacySupported ? usb_legacy_checkbox->setChecked(m_usbLegacy ? true : false) :
        usb_legacy_checkbox->setEnabled(false);
    // Built In LAN
    m_builtInLANSupported ? built_in_lan_checkbox->setChecked(m_builtInLAN ? true : false) :
        built_in_lan_checkbox->setEnabled(false);
    // Power On Display
    if (m_powerOnDisplaySupported) {
        power_on_display_combobox->addItems(m_displayDevices);
        power_on_display_combobox->setCurrentIndex(m_displayDevicesMap.key(m_currentDisplayDevice));
    } else {
        power_on_display_combobox->setEnabled(false);
        power_on_display_label->setEnabled(false);
    }
    // HDMI-CEC
    if (m_hdmiCECSupported) {
        hdmiCECGroupBox->setChecked(m_hdmiCEC ? true : false);
        m_hdmiRemotePowerSupported ? hdmi_remote_power_checkbox->setChecked(m_hdmiRemotePower ? true : false) :
            hdmi_remote_power_checkbox->setEnabled(false);
    } else {
        hdmiCECGroupBox->setEnabled(false);
    }
}

void GeneralSettings::save()
{
    unsigned int tmp;

    // Pointing Device
    if (m_pointingDeviceSupported) {
        tmp = (pointing_device_checkbox->checkState() == Qt::Checked) ?
              KToshibaHardware::ACTIVATED : KToshibaHardware::DEACTIVATED;
        if (m_pointingDevice != tmp) {
            m_sys->hw()->setPointingDevice(tmp);
            general.writeEntry("PointingDevice", tmp);
            general.sync();
            m_pointingDevice = tmp;
            emit configFileChanged();
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
    // Power On Display
    if (m_powerOnDisplaySupported) {
        int tmp2 = m_displayDevicesMap.value(power_on_display_combobox->currentIndex());
        if (m_currentDisplayDevice != tmp2) {
            m_sys->hw()->setPowerOnDisplay(tmp2);
            m_currentDisplayDevice = tmp2;
        }
    }
    // HDMI-CEC
    if (m_hdmiCECSupported) {
        tmp = hdmiCECGroupBox->isChecked() ?
              KToshibaHardware::ACTIVATED : KToshibaHardware::DEACTIVATED;
        if (tmp != m_hdmiCEC) {
            m_sys->hw()->setHDMICEC(tmp);
            m_hdmiCEC = tmp;
        }
        if (m_hdmiRemotePowerSupported) {
            tmp = (hdmi_remote_power_checkbox->checkState() == Qt::Checked) ?
                  KToshibaHardware::ACTIVATED : KToshibaHardware::DEACTIVATED;
            if (tmp != m_hdmiRemotePower) {
                m_sys->hw()->setHDMIRemotePower(tmp);
                m_hdmiRemotePower = tmp;
            }
        }
    }
}

void GeneralSettings::defaults()
{
    // Pointing Device
    if (m_pointingDeviceSupported && !m_pointingDevice) {
        pointing_device_checkbox->setChecked(true);
    }
    // USB Rapid Charge
    if (m_rapidChargeSupported && m_rapidCharge) {
        rapid_charge_checkbox->setChecked(false);
    }
    // USB Three
    if (m_usbThreeSupported && !m_usbThree) {
        usb_three_checkbox->setChecked(true);
    }
    // USB Legacy Emulation
    if (m_usbLegacySupported && !m_usbLegacy) {
        usb_legacy_checkbox->setChecked(true);
    }
    // Built In LAN
    if (m_builtInLANSupported && !m_builtInLAN) {
        built_in_lan_checkbox->setChecked(true);
    }
    // Power On Display
    if (m_powerOnDisplaySupported && m_currentDisplayDevice != KToshibaHardware::AUTO_DISPLAY) {
        power_on_display_combobox->setCurrentIndex(m_displayDevicesMap.key(KToshibaHardware::AUTO_DISPLAY));
    }
    // HDMI-CEC
    if (m_hdmiCECSupported) {
        hdmiCECGroupBox->setChecked(false);
        if (m_hdmiRemotePowerSupported) {
            hdmi_remote_power_checkbox->setChecked(false);
        }
    }
}
