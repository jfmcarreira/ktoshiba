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

#include <linux/toshiba.h>

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
}

#include "bootorder.h"
#include "devicemodel.h"
#include "ktoshibahardware.h"

BootOrder::BootOrder(QWidget *parent)
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

BootOrder::~BootOrder()
{
    delete m_model; m_model = NULL;
}

bool BootOrder::isBootOrderSupported()
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

quint32 BootOrder::getDeviceOrder()
{
    return m_model->getDeviceData();
}

void BootOrder::setDeviceOrder(quint32 order)
{
    SMMRegisters regs = { 0xf400, 0x0157, order, 0, 0, 0 };

    if (m_hw->tci_raw(&regs) < 0)
        qCritical() << "Could not set Boot Order";
}

void BootOrder::deferClicked()
{
    m_model->moveDown(deviceList->currentIndex());
    emit changed();
}

void BootOrder::preferClicked()
{
    m_model->moveUp(deviceList->currentIndex());
    emit changed();
}

void BootOrder::load()
{
    m_model->setDeviceData(m_order);
}

void BootOrder::save()
{
    quint32 tmp = getDeviceOrder();
    if (m_order != tmp)
        setDeviceOrder(tmp);
}

void BootOrder::defaults()
{
    m_model->setDeviceData(m_default);
}


#include "bootorder.moc"
