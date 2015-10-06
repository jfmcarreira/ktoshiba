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

#include <QDebug>
#include <QStringList>
#include <QtDBus/QtDBus>

#include <KLocalizedString>
#include <KConfigGroup>

extern "C" {
#include <linux/toshiba.h>
}

#include "powersave.h"
#include "systemsettings.h"
#include "ktoshibahardware.h"

#define CONFIG_FILE "ktoshibarc"

PowerSave::PowerSave(QWidget *parent)
    : QWidget(parent),
      m_sys(qobject_cast<KToshibaSystemSettings *>(QObject::parent())),
      m_config(KSharedConfig::openConfig(CONFIG_FILE))
{
    setupUi(this);

    m_coolingMethodSupported = isCoolingMethodSupported();
}

PowerSave::~PowerSave()
{
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
    qDebug() << "powersave load";
    // Boot Order
    if (!m_coolingMethodSupported) {
        m_type1 << "Maximum Performance" << "Battery Optimized";
        m_type2 << "High Performance" << "Balanced" << "Power Saver";
        if (m_maxCoolingMethod == KToshibaHardware::BATTERY_OPTIMIZED) {
            cooling_method_battery_combobox->addItems(m_type1);
            cooling_method_plugged_combobox->addItems(m_type1);
        } else if (m_maxCoolingMethod == KToshibaHardware::POWER_SAVER) {
            cooling_method_battery_combobox->addItems(m_type2);
            cooling_method_plugged_combobox->addItems(m_type2);
        }

        KConfigGroup powersave(m_config, "PowerSave");
        if (!powersave.exists()) {
            powersave.writeEntry("ManageCoolingMethod", true);
            powersave.writeEntry("CoolingMethodOnBattery", 1);
            powersave.writeEntry("CoolingMethodPluggedIn", 0);
            powersave.sync();
        }
        m_manageCoolingMethod = powersave.readEntry("ManageCoolingMethod", true);
        groupBox->setChecked(m_manageCoolingMethod);
        m_coolingMethodBattery = powersave.readEntry("CoolingMethodOnBattery", 0);
        cooling_method_battery_combobox->setCurrentIndex(m_coolingMethodBattery);
        m_coolingMethodPlugged = powersave.readEntry("CoolingMethodPluggedIn", 1);
        cooling_method_plugged_combobox->setCurrentIndex(m_coolingMethodPlugged);
    }
}

void PowerSave::save()
{
    if (m_coolingMethodSupported) {
        KConfigGroup powersave(m_config, "PowerSave");
        QDBusInterface iface("net.sourceforge.KToshiba",
                             "/net/sourceforge/KToshiba",
                             "net.sourceforge.KToshiba",
                             QDBusConnection::sessionBus(), this);
        bool tmp = groupBox->isChecked();
        if (m_manageCoolingMethod != tmp) {
            powersave.writeEntry("ManageCoolingMethod", tmp);
            powersave.sync();
            m_manageCoolingMethod = tmp;
            if (iface.isValid())
                iface.call("configFileChanged");
        }
        int tmp2 = cooling_method_battery_combobox->currentIndex();
        if (m_coolingMethodBattery != tmp2) {
            powersave.writeEntry("CoolingMethodOnBattery", tmp2);
            powersave.sync();
            m_coolingMethodBattery = tmp2;
            if (iface.isValid())
                iface.call("configFileChanged");
        }
        tmp = cooling_method_plugged_combobox->currentIndex();
        if (m_coolingMethodPlugged != tmp2) {
            powersave.writeEntry("CoolingMethodPluggedIn", tmp2);
            powersave.sync();
            m_coolingMethodPlugged = tmp2;
            if (iface.isValid())
                iface.call("configFileChanged");
        }
    } else {
        groupBox->setEnabled(false);
    }
}

void PowerSave::defaults()
{
    if (m_coolingMethodSupported) {
        groupBox->setChecked(true);
        cooling_method_battery_combobox->setCurrentIndex(KToshibaHardware::MAXIMUM_PERFORMANCE);
        cooling_method_plugged_combobox->setCurrentIndex(KToshibaHardware::BATTERY_OPTIMIZED);
    }
}
