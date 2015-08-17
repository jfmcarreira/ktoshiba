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
    : QWidget( parent ),
      m_hw( qobject_cast<KToshibaHardware *>(QObject::parent()) )
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

    connect( deferButton, SIGNAL( clicked() ), this, SLOT( deferClicked() ) );
    connect( preferButton, SIGNAL( clicked() ), this, SLOT( preferClicked() ) );
}

BootSettings::~BootSettings()
{
    delete m_model; m_model = NULL;
}

bool BootSettings::isBootOrderSupported()
{
    SMMRegisters regs = { 0xf300, 0x0157, 0, 0, 0, 0 };

    if (m_hw->tci_raw(&regs) < 0)
        return false;

    if (regs.eax != 0)
        return false;

    m_order = regs.ecx;
    m_default = regs.esi;
    m_model->setSupportedDevices(regs.edx);

    return true;
}

bool BootSettings::isWOKSupported()
{
    SMMRegisters regs = { 0xf300, 0x0137, 0, 0, 0, 0 };

    if (m_hw->tci_raw(&regs) < 0)
        return false;

    if (regs.eax != 0 && regs.eax != 1)
        return false;

    m_wok = regs.ecx;
    m_defaultWOK = regs.esi;

    return true;
}

bool BootSettings::isWOLSupported()
{
    SMMRegisters regs = { 0xf300, 0x0700, 0x0800, 0, 0, 0 };

    if (m_hw->tci_raw(&regs) < 0)
        return false;

    if (regs.eax != 0 && regs.eax != 1)
        return false;

    m_wol = regs.ecx;
    m_defaultWOL = regs.esi;

    return true;
}

void BootSettings::setDeviceOrder(quint32 order)
{
    SMMRegisters regs = { 0xf400, 0x0157, order, 0, 0, 0 };

    if (m_hw->tci_raw(&regs) < 0)
        qCritical() << "Could not set Boot Order";
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

void BootSettings::setWOK(quint32 state)
{
    SMMRegisters regs = { 0xf400, 0x0137, state, 0, 0, 0 };

    if (m_hw->tci_raw(&regs) < 0)
        qCritical() << "Could not set WOK state";
}

void BootSettings::setWOL(quint32 state)
{
    SMMRegisters regs = { 0xf300, 0x0700, state, 0, 0, 0 };

    if (m_hw->tci_raw(&regs) < 0)
        qCritical() << "Could not set WOL state";
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
        wol_checkbox->setChecked( m_wok ? true : false );
    } else {
        wok_label->setEnabled(false);
        wok_checkbox->setEnabled(false);
    }
    // Wake on LAN
    m_wolSupported = isWOLSupported();
    if (m_wolSupported) {
        wol_checkbox->setChecked( m_wol == 0x0801 ? true : false );
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
            setDeviceOrder(tmp);
            m_order = tmp;
        }
    }
    // Wake on Keyboard
    if (m_wokSupported) {
        tmp = wok_checkbox->checkState() == Qt::Checked ? 1 : 0;
        if (m_wok != tmp) {
            setWOK(tmp);
            m_wok = tmp;
        }
    }
    // Wake on LAN
    if (m_wolSupported) {
        tmp = wol_checkbox->checkState() == Qt::Checked ? 0x0801 : 0x0800;
        if (m_wol != tmp) {
            setWOL(tmp);
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
