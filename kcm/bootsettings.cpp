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

#include <KLocalizedString>

extern "C" {
#include <linux/toshiba.h>
}

#include "bootsettings.h"
#include "devicemodel.h"
#include "ktoshibahardware.h"

BootSettings::BootSettings(QWidget *parent)
    : QWidget(parent),
      m_hw(qobject_cast<KToshibaHardware * >(QObject::parent()))
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
}

BootSettings::~BootSettings()
{
    delete m_model; m_model = NULL;
}

bool BootSettings::isBootOrderSupported()
{
    quint32 result = m_hw->getBootOrder(&m_order, &m_maxdev, &m_default);

    if (result != KToshibaHardware::SUCCESS && result != KToshibaHardware::SUCCESS2)
        return false;

    m_model->setSupportedDevices(m_maxdev);

    return true;
}

bool BootSettings::isWOKSupported()
{
    quint32 result = m_hw->getWakeOnKeyboard(&m_wok, &m_defaultWOK);

    if (result != KToshibaHardware::SUCCESS && result != KToshibaHardware::SUCCESS2)
        return false;

    return true;
}

bool BootSettings::isWOLSupported()
{
    quint32 result = m_hw->getWakeOnLAN(&m_wol, &m_defaultWOL);

    if (result != KToshibaHardware::SUCCESS && result != KToshibaHardware::SUCCESS2)
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
    m_bootOrderSupported = isBootOrderSupported();
    if (m_bootOrderSupported) {
        m_model->setDeviceData(m_order);
    } else {
        deviceList->setEnabled(false);
        deferButton->setEnabled(false);
        preferButton->setEnabled(false);
    }
    // Wake on Keyboard
    m_wokSupported = isWOKSupported();
    if (m_wokSupported) {
        wol_checkbox->setChecked(m_wok ? true : false);
    } else {
        wok_label->setEnabled(false);
        wok_checkbox->setEnabled(false);
    }
    // Wake on LAN
    m_wolSupported = isWOLSupported();
    if (m_wolSupported) {
        wol_checkbox->setChecked(m_wol == 0x0801 ? true : false);
    } else {
        wol_label->setEnabled(false);
        wol_checkbox->setEnabled(false);
    }
}

void BootSettings::save()
{
    quint32 tmp;

    // Boot Order
    if (m_bootOrderSupported) {
        tmp = m_model->getDeviceData();
        if (m_order != tmp) {
            m_hw->setBootOrder(tmp);
            m_order = tmp;
        }
    }
    // Wake on Keyboard
    if (m_wokSupported) {
        tmp = wok_checkbox->checkState() == Qt::Checked ? 1 : 0;
        if (m_wok != tmp) {
            m_hw->setWakeOnKeyboard(tmp);
            m_wok = tmp;
        }
    }
    // Wake on LAN
    if (m_wolSupported) {
        tmp = wol_checkbox->checkState() == Qt::Checked ? 0x0801 : 0x0800;
        if (m_wol != tmp) {
            m_hw->setWakeOnLAN(tmp);
            m_wol = tmp;
        }
    }
}

void BootSettings::defaults()
{
    // Boot Order
    if (m_bootOrderSupported)
        m_model->setDeviceData(m_default);
    // Wake on Keyboard
    if (m_wokSupported)
        wok_checkbox->setChecked(false);
    // Wake on LAN
    if (m_wolSupported)
        wol_checkbox->setChecked(false);
}


#include "bootsettings.moc"
