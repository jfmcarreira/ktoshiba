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

#include "bootsettings.h"
#include "devicemodel.h"
#include "systemsettings.h"
#include "ktoshibahardware.h"

BootSettings::BootSettings(QWidget *parent)
    : QWidget(parent),
      m_sys(qobject_cast<KToshibaSystemSettings * >(QObject::parent()))
{
    setupUi(this);

    deferButton->setIcon(QIcon::fromTheme("go-down"));
    preferButton->setIcon(QIcon::fromTheme("go-up"));

    m_model = new DeviceModel(deviceList);
    deviceList->setModel(m_model);
    deviceList->setSelectionMode(QAbstractItemView::SingleSelection);
    deviceList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    deviceList->setAlternatingRowColors(false);
    deviceList->setRootIsDecorated(false);

    connect(deferButton, SIGNAL(clicked()), this, SLOT(deferClicked()));
    connect(preferButton, SIGNAL(clicked()), this, SLOT(preferClicked()));

    m_bootOrderSupported = isBootOrderSupported();
    m_panelPowerOnSupported = isPanelPowerOnSupported();
    m_wokSupported = isWOKSupported();
    m_wolSupported = isWOLSupported();
    m_bootSpeedSupported = isBootSpeedSupported();
}

BootSettings::~BootSettings()
{
    delete m_model; m_model = NULL;
}

bool BootSettings::isBootOrderSupported()
{
    quint32 result = m_sys->hw()->getBootOrder(&m_bootOrder, &m_maxDevices, &m_defaultBootOrder);

    if (result != KToshibaHardware::SUCCESS && result != KToshibaHardware::SUCCESS2)
        return false;

    m_model->setSupportedDevices(m_maxDevices);

    return true;
}

bool BootSettings::isPanelPowerOnSupported()
{
    m_panelPowerON = m_sys->hw()->getPanelPowerON();

    if (m_panelPowerON != KToshibaHardware::DEACTIVATED && m_panelPowerON != KToshibaHardware::ACTIVATED)
        return false;

    return true;
}

bool BootSettings::isWOKSupported()
{
    quint32 result = m_sys->hw()->getWakeOnKeyboard(&m_wok, &m_defaultWOK);

    if (result != KToshibaHardware::SUCCESS && result != KToshibaHardware::SUCCESS2)
        return false;

    return true;
}

bool BootSettings::isWOLSupported()
{
    quint32 result = m_sys->hw()->getWakeOnLAN(&m_wol, &m_defaultWOL);

    if (result != KToshibaHardware::SUCCESS && result != KToshibaHardware::SUCCESS2)
        return false;

    return true;
}

bool BootSettings::isBootSpeedSupported()
{
    m_bootSpeed = m_sys->hw()->getBootSpeed();

    if (m_bootSpeed != KToshibaHardware::NORMAL && m_bootSpeed != KToshibaHardware::FAST)
        return false;

    return true;
}

void BootSettings::deferClicked()
{
    m_model->moveDown(deviceList->currentIndex());
    emit changed();
}

void BootSettings::preferClicked()
{
    m_model->moveUp(deviceList->currentIndex());
    emit changed();
}

void BootSettings::load()
{
    // Boot Order
    if (m_bootOrderSupported)
        m_model->setDeviceData(m_bootOrder);
    else
        groupBox->setEnabled(false);
    // Panel Power ON
    if (m_panelPowerOnSupported)
        panel_power_checkbox->setChecked(m_panelPowerON ? true : false);
    else
        panel_power_checkbox->setEnabled(false);
    // Wake on Keyboard
    if (m_wokSupported)
        wol_checkbox->setChecked(m_wok ? true : false);
    else
        wok_checkbox->setEnabled(false);
    // Wake on LAN
    if (m_wolSupported)
        wol_checkbox->setChecked(m_wol == 0x0801 ? true : false);
    else
        wol_checkbox->setEnabled(false);
    // Boot Speed
    if (m_bootSpeedSupported) {
        boot_speed_combobox->setCurrentIndex(m_bootSpeed);
    } else {
        boot_speed_label->setEnabled(false);
        boot_speed_combobox->setEnabled(false);
    }
}

void BootSettings::save()
{
    int tmp;

    // Boot Order
    if (m_bootOrderSupported) {
        tmp = m_model->getDeviceData();
        if (m_bootOrder != tmp) {
            m_sys->hw()->setBootOrder(tmp);
            m_bootOrder = tmp;
        }
    }
    // Panel Power ON
    if (m_panelPowerOnSupported) {
        tmp = (panel_power_checkbox->checkState() == Qt::Checked) ?
              KToshibaHardware::ACTIVATED : KToshibaHardware::DEACTIVATED;
        if (m_panelPowerON != tmp) {
            m_sys->hw()->setPanelPowerON(tmp);
            m_panelPowerON = tmp;
        }
    }
    // Wake on Keyboard
    if (m_wokSupported) {
        tmp = (wok_checkbox->checkState() == Qt::Checked) ?
              KToshibaHardware::ACTIVATED : KToshibaHardware::DEACTIVATED;
        if (m_wok != tmp) {
            m_sys->hw()->setWakeOnKeyboard(tmp);
            m_wok = tmp;
        }
    }
    // Wake on LAN
    if (m_wolSupported) {
        tmp = (wol_checkbox->checkState() == Qt::Checked) ? 0x0801 : 0x0800;
        if (m_wol != tmp) {
            m_sys->hw()->setWakeOnLAN(tmp);
            m_wol = tmp;
        }
    }
    // Boot Speed
    if (m_bootSpeedSupported) {
        tmp = boot_speed_combobox->currentIndex();
        if (m_bootSpeed != tmp) {
            m_sys->hw()->setBootSpeed(tmp);
            m_bootSpeed = tmp;
        }
    }
}

void BootSettings::defaults()
{
    // Boot Order
    if (m_bootOrderSupported && m_bootOrder != m_defaultBootOrder)
        m_model->setDeviceData(m_defaultBootOrder);
    // Panel Power ON
    if (m_panelPowerOnSupported && m_panelPowerON)
        panel_power_checkbox->setChecked(false);
    // Wake on Keyboard
    if (m_wokSupported && m_wok)
        wok_checkbox->setChecked(false);
    // Wake on LAN
    if (m_wolSupported && m_wol)
        wol_checkbox->setChecked(false);
    // Boot Speed
    if (m_bootSpeedSupported && m_bootSpeed)
        boot_speed_combobox->setCurrentIndex(0);
}
