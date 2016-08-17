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

#include <QStringBuilder>

#include <KLocalizedString>

#include <ktoshibahardware.h>

#include "sleeputilities.h"
#include "systemsettings.h"

SleepUtilities::SleepUtilities(QWidget *parent)
    : QWidget(parent),
      m_sys(qobject_cast<KToshibaSystemSettings * >(QObject::parent()))
{
    setupUi(this);

    m_sleepChargeSupported = isSleepChargeSupported();
    m_sleepMusicSupported = isSleepMusicSupported();
}

bool SleepUtilities::isSleepChargeSupported()
{
    quint32 result = m_sys->hw()->getUSBSleepCharge(&m_sleepCharge, &m_maxSleepCharge, &m_defaultSleepCharge);

    if (result != KToshibaHardware::SUCCESS && result != KToshibaHardware::SUCCESS2) {
        return false;
    }

    /*
     * NOTE: Not all laptop models support all the Sleep & Charge modes,
     * this helps determine which (of our supported) modes this specific
     * laptop supports, however, "Disabled" is always supported.
     */
    int index = 0;
    m_sleepModesMap[index] = KToshibaHardware::DISABLED;
    m_sleepModes << i18n("Disabled");
    index++;
    if ((m_maxSleepCharge & KToshibaHardware::ALTERNATE) != 1) {
        m_sleepModesMap[index] = KToshibaHardware::ALTERNATE;
        m_sleepModes << i18n("Alternate");
        index++;
    }
    if ((m_maxSleepCharge & KToshibaHardware::TYPICAL) != 1) {
        m_sleepModesMap[index] = KToshibaHardware::TYPICAL;
        m_sleepModes << i18n("Typical");
        index++;
    }
    if ((m_maxSleepCharge & KToshibaHardware::AUTO) != 1) {
        m_sleepModesMap[index] = KToshibaHardware::AUTO;
        m_sleepModes << i18n("Auto");
        index++;
    }

    return true;
}

bool SleepUtilities::isSleepMusicSupported()
{
    m_sleepMusic = m_sys->hw()->getSleepMusic();

    if (m_sleepMusic != KToshibaHardware::DEACTIVATED
        && m_sleepMusic != KToshibaHardware::ACTIVATED) {
        return false;
    }

    return true;
}

void SleepUtilities::load()
{
    // Sleep and Charge
    if (m_sleepChargeSupported) {
        sleep_charge_combobox->addItems(m_sleepModes);
        sleep_charge_combobox->setCurrentIndex(m_sleepModesMap.key(m_sleepCharge));

        quint32 sleep_on_bat =
                    m_sys->hw()->getSleepFunctionsOnBatteryStatus(&m_batteryEnabled, &m_batteryLevel);
        if (sleep_on_bat == KToshibaHardware::SUCCESS
            || sleep_on_bat == KToshibaHardware::SUCCESS2) {
            groupBox->setChecked(m_batteryEnabled ? true : false);
            battery_level->setText(QString::number(m_batteryLevel) % QStringLiteral("%"));
            battery_level_slider->setValue(m_batteryLevel);
        } else {
            groupBox->setEnabled(false);
        }
    } else {
        groupBox->setEnabled(false);
    }
    // Sleep and Music
    m_sleepMusicSupported ? sleep_music_checkbox->setChecked(m_sleepMusic ? true : false) :
        sleep_music_checkbox->setEnabled(false);
}

void SleepUtilities::save()
{
    int tmp;

    // Sleep and Charge
    if (m_sleepChargeSupported) {
        tmp = m_sleepModesMap.value(sleep_charge_combobox->currentIndex());
        if (m_sleepCharge != tmp) {
            m_sys->hw()->setUSBSleepCharge(tmp, m_defaultSleepCharge);
            m_sleepCharge = tmp;
        }
        tmp = groupBox->isChecked() ? KToshibaHardware::ACTIVATED : KToshibaHardware::DEACTIVATED;
        if (m_batteryEnabled != tmp) {
            m_sys->hw()->setSleepFunctionsOnBatteryStatus(tmp, m_batteryLevel);
            m_batteryEnabled = tmp;
        }
        tmp = battery_level_slider->value();
        if (m_batteryLevel != tmp) {
            m_sys->hw()->setSleepFunctionsOnBatteryStatus(m_batteryEnabled, tmp);
            m_batteryLevel = tmp;
        }
    }
    // Sleep and Music
    if (m_sleepMusicSupported) {
        tmp = (sleep_music_checkbox->checkState() == Qt::Checked) ?
              KToshibaHardware::ACTIVATED : KToshibaHardware::DEACTIVATED;
        if (m_sleepMusic != tmp) {
            m_sys->hw()->setSleepMusic(tmp);
            m_sleepMusic = tmp;
        }
    }
}

void SleepUtilities::defaults()
{
    // Sleep and Charge
    if (m_sleepChargeSupported) {
        if (m_sleepCharge != KToshibaHardware::AUTO) {
            sleep_charge_combobox->setCurrentIndex(m_sleepModesMap.key(KToshibaHardware::AUTO));
        }
        if (m_batteryEnabled != KToshibaHardware::ACTIVATED) {
            groupBox->setChecked(true);
        }
        if (m_batteryLevel != 10) {
            battery_level->setText(QString::number(10) % QStringLiteral("%"));
            battery_level_slider->setValue(10);
        }
    }
    // Sleep and Music
    if (m_sleepMusicSupported) {
        if (m_sleepMusic != KToshibaHardware::DEACTIVATED) {
            sleep_music_checkbox->setChecked(false);
        }
    }
}
