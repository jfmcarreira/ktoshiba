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

extern "C" {
#include <linux/toshiba.h>
}

#include "sleeputilities.h"
#include "systemsettings.h"
#include "ktoshibahardware.h"

SleepUtilities::SleepUtilities(QWidget *parent)
    : QWidget(parent),
      m_sys(qobject_cast<KToshibaSystemSettings *>(QObject::parent()))
{
    setupUi(this);

    m_sleepChargeSupported = isSleepChargeSupported();
    m_sleepMusicSupported = isSleepMusicSupported();
}

bool SleepUtilities::isSleepChargeSupported()
{
    quint32 result = m_sys->hw()->getUSBSleepCharge(&m_sleepcharge, &m_defaultsc);

    if (result != KToshibaHardware::SUCCESS && result != KToshibaHardware::SUCCESS2)
        return false;

    return true;
}

bool SleepUtilities::isSleepMusicSupported()
{
    m_sleepmusic = m_sys->hw()->getUSBSleepMusic();

    if (m_sleepmusic != 0 && m_sleepmusic != 1)
        return false;

    return true;
}

void SleepUtilities::load()
{
    // Sleep and Charge
    if (m_sleepChargeSupported) {
        int tmp;
        switch(m_sleepcharge) {
        case KToshibaHardware::DISABLED:
            tmp = 0;
        case KToshibaHardware::ALTERNATE:
            tmp = 1;
        case KToshibaHardware::TYPICAL:
            tmp = 2;
        case KToshibaHardware::AUTO:
            tmp = 3;
        };
        sleep_charge_combobox->setCurrentIndex(tmp);

        quint32 sleep_on_bat = m_sys->hw()->getUSBSleepFunctionsBatLvl(&m_batenabled, &m_batlevel);
        if (sleep_on_bat == KToshibaHardware::SUCCESS
            || sleep_on_bat == KToshibaHardware::SUCCESS2) {
            groupBox->setChecked(m_batenabled ? true : false);
            battery_level->setText(QString::number(m_batlevel) + "%");
            battery_level_slider->setValue(m_batlevel);
        } else {
            groupBox->setEnabled(false);
        }
    } else {
        sleep_charge_label->setEnabled(false);
        sleep_charge_combobox->setEnabled(false);
        groupBox->setEnabled(false);
    }
    // Sleep and Music
    if (m_sleepMusicSupported)
        sleep_music_checkbox->setChecked(m_sleepmusic ? true : false);
    else
        sleep_music_checkbox->setEnabled(false);
}

void SleepUtilities::save()
{
    int tmp;

    // Sleep and Charge
    if (m_sleepChargeSupported) {
        switch(sleep_charge_combobox->currentIndex()) {
        case 0:
            tmp = KToshibaHardware::DISABLED;
        case 1:
            tmp = KToshibaHardware::ALTERNATE;
        case 2:
            tmp = KToshibaHardware::TYPICAL;
        case 3:
            tmp = KToshibaHardware::AUTO;
        };
        
        if (m_sleepcharge != tmp) {
            m_sys->hw()->setUSBSleepCharge(tmp, m_defaultsc);
            m_sleepcharge = tmp;
        }
        tmp = groupBox->isChecked() ? 1 : 0;
        if (m_batenabled != tmp) {
            m_sys->hw()->setUSBSleepFunctionsBatLvl(tmp == 0 ? tmp : m_batlevel);
            m_batenabled = tmp;
        }
        tmp = battery_level_slider->value();
        if (m_batlevel != tmp) {
            m_sys->hw()->setUSBSleepFunctionsBatLvl(tmp);
            m_batlevel = tmp;
        }
    }
    // Sleep and Music
    if (m_sleepMusicSupported) {
        tmp = sleep_music_checkbox->checkState() == Qt::Checked ? 1 : 0;
        if (m_sleepmusic != tmp) {
            m_sys->hw()->setUSBSleepMusic(tmp);
            m_sleepmusic = tmp;
        }
    }
}

void SleepUtilities::defaults()
{
    // Sleep and Charge
    if (m_sleepChargeSupported) {
        if (m_sleepcharge != KToshibaHardware::AUTO)
            sleep_charge_combobox->setCurrentIndex(3);
        if (m_batenabled != 1)
            groupBox->setChecked(true);
        if (m_batlevel != 10) {
            battery_level->setText(QString::number(10) + "%");
            battery_level_slider->setValue(10);
        }
    }
    // Sleep and Music
    if (m_sleepMusicSupported)
        if (m_sleepmusic != 0)
            sleep_music_checkbox->setChecked(false);
}
