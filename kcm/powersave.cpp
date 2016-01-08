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

#include "powersave.h"
#include "systemsettings.h"

PowerSave::PowerSave(QWidget *parent)
    : QWidget(parent),
      m_sys(qobject_cast<KToshibaSystemSettings * >(QObject::parent())),
      m_config(KSharedConfig::openConfig("ktoshibarc"))
{
    setupUi(this);

    m_coolingMethodSupported = isCoolingMethodSupported();
    m_sataInterfaceSupported = isSATAInterfaceSupported();
    m_oddPowerSupported = isODDPowerSupported();
    m_illuminationLEDSupported = isIlluminationLEDSupported();

    powersave = KConfigGroup(m_config, "Powersave");
    if (!powersave.exists()) {
        powersave.writeEntry("BatteryProfile", 0);
        powersave.writeEntry("PerformanceCoolingMethod", 0);
        powersave.writeEntry("PerformanceODDPower", 1);
        powersave.writeEntry("PerformanceIlluminationLED", 1);
        powersave.writeEntry("PowersaveCoolingMethod", 1);
        powersave.writeEntry("PowersaveODDPower", 1);
        powersave.writeEntry("PowersaveIlluminationLED", 0);
        powersave.writeEntry("PresentationCoolingMethod", 1);
        powersave.writeEntry("PresentationODDPower", 1);
        powersave.writeEntry("PresentationIlluminationLED", 0);
        powersave.writeEntry("EcoCoolingMethod", 1);
        powersave.writeEntry("EcoODDPower", 0);
        powersave.writeEntry("EcoIlluminationLED", 0);
        powersave.writeEntry("SATAInterface", 0);
        powersave.sync();
    }
}

bool PowerSave::isCoolingMethodSupported()
{
    quint32 result = m_sys->hw()->getCoolingMethod(&m_coolingMethod, &m_maxCoolingMethod);

    if (result != KToshibaHardware::SUCCESS && result != KToshibaHardware::SUCCESS2) {
        return false;
    }

    m_coolingMethods << i18n("Maximum Performance");
    if (m_maxCoolingMethod == KToshibaHardware::BATTERY_OPTIMIZED) {
        m_coolingMethods << i18n("Battery Optimized");
    } else if (m_maxCoolingMethod == KToshibaHardware::BATTERY_OPTIMIZED2) {
        m_coolingMethods << i18n("Performance") << i18n("Battery Optimized");
    }

    cooling_method_combobox->addItems(m_coolingMethods);

    return true;
}

bool PowerSave::isSATAInterfaceSupported()
{
    m_sataInterface = m_sys->hw()->getSATAInterfaceSetting();

    if (m_sataInterface != KToshibaHardware::SATA_PERFORMANCE
        && m_sataInterface != KToshibaHardware::SATA_BATTERY_LIFE) {
        return false;
    }

    return true;
}

bool PowerSave::isODDPowerSupported()
{
    m_oddPower = m_sys->hw()->getODDPower();

    if (m_oddPower != KToshibaHardware::ODD_DISABLED && m_oddPower != KToshibaHardware::ODD_ENABLED) {
        return false;
    }

    return true;
}

bool PowerSave::isIlluminationLEDSupported()
{
    m_illuminationLED = m_sys->hw()->getIlluminationLED();

    if (m_illuminationLED != KToshibaHardware::DEACTIVATED
        && m_illuminationLED != KToshibaHardware::ACTIVATED) {
        return false;
    }

    return true;
}

void PowerSave::loadProfile(int profile)
{
    switch (profile) {
    case Performance:
        m_cooling = powersave.readEntry("PerformanceCoolingMethod", 0);
        m_odd = powersave.readEntry("PerformanceODDPower", 1);
        m_illumination = powersave.readEntry("PerformanceIlluminationLED", 1);
        break;
    case Powersave:
        m_cooling = powersave.readEntry("PowersaveCoolingMethod", 1);
        m_odd = powersave.readEntry("PowersaveODDPower", 1);
        m_illumination = powersave.readEntry("PowersaveIlluminationLED", 0);
        break;
    case Presentation:
        m_cooling = powersave.readEntry("PresentationCoolingMethod", 0);
        m_odd = powersave.readEntry("PresentationODDPower", 1);
        m_illumination = powersave.readEntry("PresentationIlluminationLED", 0);
        break;
    case ECO:
        m_cooling = powersave.readEntry("EcoCoolingMethod", 1);
        m_odd = powersave.readEntry("EcoODDPower", 0);
        m_illumination = powersave.readEntry("EcoIlluminationLED", 0);
        break;
    }

    if (m_coolingMethodSupported) {
        cooling_method_combobox->setCurrentIndex(m_cooling);
    }
    if (m_oddPowerSupported) {
        odd_power_combobox->setCurrentIndex(m_odd);
    }
    if (m_illuminationLEDSupported) {
        illumination_combobox->setCurrentIndex(m_illumination);
    }
}

void PowerSave::saveProfile(int profile)
{
    switch (profile) {
    case Performance:
        powersave.writeEntry("PerformanceSATAInterface", 0);
        powersave.writeEntry("PerformanceCoolingMethod", 0);
        powersave.writeEntry("PerformanceODDPower", 1);
        powersave.writeEntry("PerformanceIlluminationLED", 1);
        break;
    case Powersave:
        powersave.writeEntry("PowersaveSATAInterface", 1);
        powersave.writeEntry("PowersaveCoolingMethod", 1);
        powersave.writeEntry("PowersaveODDPower", 1);
        powersave.writeEntry("PowersaveIlluminationLED", 0);
        break;
    case Presentation:
        powersave.writeEntry("PresentationSATAInterface", 0);
        powersave.writeEntry("PresentationCoolingMethod", 1);
        powersave.writeEntry("PresentationODDPower", 1);
        powersave.writeEntry("PresentationIlluminationLED", 0);
        break;
    case ECO:
        powersave.writeEntry("EcoSATAInterface", 1);
        powersave.writeEntry("EcoCoolingMethod", 1);
        powersave.writeEntry("EcoODDPower", 0);
        powersave.writeEntry("EcoIlluminationLED", 0);
        break;
    }
    powersave.sync();
}

void PowerSave::load()
{
    // Battery Profiles
    m_batteryProfile = powersave.readEntry("BatteryProfile", 0);
    battery_profiles_combobox->setCurrentIndex(m_batteryProfile);
    // Cooling Method
    if (!m_coolingMethodSupported) {
        cooling_method_label->setEnabled(false);
        cooling_method_combobox->setEnabled(false);
    }
    // Optical Disc Device (ODD) Power Support
    if (!m_oddPowerSupported) {
        odd_power_label->setEnabled(false);
        odd_power_combobox->setEnabled(false);
    }
    // Illumunation LED
    if (!m_illuminationLEDSupported) {
        illumination_label->setEnabled(false);
        illumination_combobox->setEnabled(false);
    }
    // SATA Interface Setting
    if (m_sataInterfaceSupported) {
        sata_iface_combobox->setCurrentIndex(m_sataInterface);
    } else {
        sata_iface_label->setEnabled(false);
        sata_iface_combobox->setEnabled(false);
    }

    loadProfile(m_batteryProfile);
}

void PowerSave::save()
{
    bool config_changed = false;

    // Battery Profiles
    int tmp = battery_profiles_combobox->currentIndex();
    if (m_batteryProfile != tmp) {
        config_changed = true;
        powersave.writeEntry("BatteryProfile", tmp);
        powersave.sync();
        m_batteryProfile = tmp;
    }
    // Cooling Method
    if (m_coolingMethodSupported) {
        m_cooling = cooling_method_combobox->currentIndex();
        if (m_coolingMethod != m_cooling) {
            config_changed = true;
            m_sys->hw()->setCoolingMethod(m_cooling);
            m_coolingMethod = m_cooling;
        }
    }
    // Optical Disc Device (ODD) Power Support
    if (m_oddPowerSupported) {
        m_odd = odd_power_combobox->currentIndex();
        if (m_oddPower != (0x100 | m_odd)) {
            config_changed = true;
            m_sys->hw()->setODDPower((0x100 | m_odd));
            m_oddPower = (0x100 | m_odd);
        }
    }
    // Illumunation LED
    if (m_illuminationLEDSupported) {
        m_illumination = illumination_combobox->currentIndex();
        if (m_illuminationLED != m_illumination) {
            config_changed = true;
            m_sys->hw()->setIlluminationLED(m_illumination);
            m_illuminationLED = m_illumination;
        }
    }
    // SATA Interface Setting
    if (m_sataInterfaceSupported) {
        tmp = sata_iface_combobox->currentIndex();
        if (m_sataInterface != tmp) {
            m_sys->hw()->setSATAInterfaceSetting(tmp);
            m_sataInterface = tmp;
        }
    }

    if (config_changed) {
        saveProfile(m_batteryProfile);
        emit configFileChanged();
    }
}

void PowerSave::defaults()
{
    // Battery Profiles
    if (m_batteryProfile != Performance) {
        m_batteryProfile = Performance;
        battery_profiles_combobox->setCurrentIndex(Performance);
    }
    // Cooling Method
    if (m_coolingMethodSupported) {
        if (m_coolingMethod != KToshibaHardware::MAXIMUM_PERFORMANCE) {
            cooling_method_combobox->setCurrentIndex(KToshibaHardware::MAXIMUM_PERFORMANCE);
        }
    }
    // Optical Disc Device (ODD) Power Support
    if (m_oddPowerSupported && m_oddPower != KToshibaHardware::ODD_ENABLED) {
        odd_power_combobox->setCurrentIndex(KToshibaHardware::ODD_ENABLED);
    }
    // Illumunation LED
    if (m_illuminationLEDSupported && m_illuminationLED != KToshibaHardware::ACTIVATED) {
        illumination_combobox->setCurrentIndex(KToshibaHardware::ACTIVATED);
    }
    // SATA Interface Setting
    if (m_sataInterfaceSupported && m_sataInterface != KToshibaHardware::SATA_PERFORMANCE) {
        sata_iface_combobox->setCurrentIndex(KToshibaHardware::SATA_PERFORMANCE);
    }
}
