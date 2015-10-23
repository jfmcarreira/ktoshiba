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

#include <KLocalizedString>

#include "powersave.h"
#include "systemsettings.h"
#include "ktoshibahardware.h"

PowerSave::PowerSave(QWidget *parent)
    : QWidget(parent),
      m_sys(qobject_cast<KToshibaSystemSettings *>(QObject::parent())),
      m_config(KSharedConfig::openConfig("ktoshibarc"))
{
    setupUi(this);

    m_coolingMethodSupported = isCoolingMethodSupported();

    powersave = KConfigGroup(m_config, "Powersave");
    if (!powersave.exists()) {
        powersave.writeEntry("BatteryProfiles", true);
        powersave.writeEntry("CurrentProfile", 0);
        powersave.writeEntry("ManageCoolingMethod", true);
        powersave.writeEntry("CoolingMethodOnBattery", 1);
        powersave.writeEntry("CoolingMethodPluggedIn", 0);
        powersave.sync();
    }
}

bool PowerSave::isCoolingMethodSupported()
{
    quint32 result = m_sys->hw()->getCoolingMethod(&m_coolingMethod, &m_maxCoolingMethod);

    if (result != KToshibaHardware::SUCCESS && result != KToshibaHardware::SUCCESS2)
        return false;

    return true;
}

void PowerSave::load()
{
    // Battery Profiles
    m_manageBatteryProfiles = powersave.readEntry("BatteryProfiles", true);
    batteryGroupBox->setChecked(m_manageBatteryProfiles);
    m_batteryProfile = powersave.readEntry("CurrentProfile", 0);
    battery_profiles_combobox->setCurrentIndex(m_batteryProfile);
    // Cooling Method
    if (m_coolingMethodSupported) {
        m_coolingMethodType1 << i18n("Maximum Performance") << i18n("Battery Optimized");
        m_coolingMethodType2 << i18n("High Performance") << i18n("Balanced") << i18n("Power Saver");
        if (m_maxCoolingMethod == KToshibaHardware::BATTERY_OPTIMIZED) {
            cooling_method_battery_combobox->addItems(m_coolingMethodType1);
            cooling_method_plugged_combobox->addItems(m_coolingMethodType1);
        } else if (m_maxCoolingMethod == KToshibaHardware::POWER_SAVER) {
            cooling_method_battery_combobox->addItems(m_coolingMethodType2);
            cooling_method_plugged_combobox->addItems(m_coolingMethodType2);
        }

        if (!powersave.exists()) {
            powersave.sync();
        }
        m_manageCoolingMethod = powersave.readEntry("ManageCoolingMethod", true);
        coolingGroupBox->setChecked(m_manageCoolingMethod);
        m_coolingMethodBattery = powersave.readEntry("CoolingMethodOnBattery", 0);
        cooling_method_battery_combobox->setCurrentIndex(m_coolingMethodBattery);
        m_coolingMethodPlugged = powersave.readEntry("CoolingMethodPluggedIn", 1);
        cooling_method_plugged_combobox->setCurrentIndex(m_coolingMethodPlugged);
    } else {
        coolingGroupBox->setEnabled(false);
    }
}

void PowerSave::save()
{
    // Battery Profiles
    bool tmp = batteryGroupBox->isChecked();
    if (m_manageBatteryProfiles != tmp) {
        powersave.writeEntry("BatteryProfiles", tmp);
        m_manageBatteryProfiles = tmp;
        emit configFileChanged();
    }
    int tmp2 = battery_profiles_combobox->currentIndex();
    if (m_batteryProfile != tmp2) {
        powersave.writeEntry("CurrentProfile", tmp2);
        m_batteryProfile = tmp2;
        emit configFileChanged();
    }
    // Cooling Method
    if (m_coolingMethodSupported) {
        tmp = coolingGroupBox->isChecked();
        if (m_manageCoolingMethod != tmp) {
            powersave.writeEntry("ManageCoolingMethod", tmp);
            m_manageCoolingMethod = tmp;
            emit configFileChanged();
        }
        tmp2 = cooling_method_battery_combobox->currentIndex();
        if (m_coolingMethodBattery != tmp2) {
            powersave.writeEntry("CoolingMethodOnBattery", tmp2);
            m_coolingMethodBattery = tmp2;
            emit configFileChanged();
        }
        tmp2 = cooling_method_plugged_combobox->currentIndex();
        if (m_coolingMethodPlugged != tmp2) {
            powersave.writeEntry("CoolingMethodPluggedIn", tmp2);
            m_coolingMethodPlugged = tmp2;
            emit configFileChanged();
        }
    }
    powersave.sync();
}

void PowerSave::defaults()
{
    // Battery Profiles
    if (!m_manageBatteryProfiles)
        batteryGroupBox->setChecked(true);
    if (m_batteryProfile != Performance)
        battery_profiles_combobox->setCurrentIndex(Performance);
    // Cooling Method
    if (m_coolingMethodSupported) {
        if (!m_manageCoolingMethod)
            coolingGroupBox->setChecked(true);
        if (m_coolingMethodBattery != KToshibaHardware::MAXIMUM_PERFORMANCE)
            cooling_method_battery_combobox->setCurrentIndex(KToshibaHardware::MAXIMUM_PERFORMANCE);
        if (m_coolingMethodPlugged != KToshibaHardware::BATTERY_OPTIMIZED)
            cooling_method_plugged_combobox->setCurrentIndex(KToshibaHardware::BATTERY_OPTIMIZED);
    }
}
