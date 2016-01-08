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

#include "devicemodel.h"

DeviceModel::DeviceModel(QObject *parent)
    : QAbstractListModel(parent),
      m_supportedDevices(MAX_BOOT_DEVICES)
{
    m_devicesMap[FDD] = "FDD";
    m_devicesMap[HDD1] = "HDD/SSD 1";
    m_devicesMap[ODD] = "ODD";
    m_devicesMap[LAN] = "LAN";
    m_devicesMap[USB_MEMORY] = "USB MEMORY";
    m_devicesMap[HDD2] = "HDD/SSD 2";
    m_devicesMap[eSATA] = "eSATA";
    m_devicesMap[USB_ODD] = "USB ODD";
    m_devicesMap[SD] = "SD";
    m_devicesMap[USB] = "USB";
    m_devicesMap[HDD3] = "HDD/SSD 3";
    m_devicesMap[NO_DEVICE] = "NO DEVICE";
}

QString DeviceModel::tooltip(int value) const
{
    QString tooltip;

    switch (value) {
    case FDD:
        tooltip = i18n("Floppy Disk Drive");
        break;
    case HDD1:
        tooltip = i18n("Hard Disk Drive/Solid State Drive 1");
        break;
    case ODD:
        tooltip = i18n("Optical Disk Drive");
        break;
    case LAN:
        tooltip = i18n("Local Area Network");
        break;
    case USB_MEMORY:
        tooltip = i18n("USB Memory");
        break;
    case HDD2:
        tooltip = i18n("Hard Disk Drive/Solid State Drive 2");
        break;
    case eSATA:
        tooltip = i18n("External SATA Drive");
        break;
    case USB_ODD:
        tooltip = i18n("USB Optical Disk Drive");
        break;
    case SD:
        tooltip = i18n("Secure Digital Card");
        break;
    case USB:
        tooltip = i18n("USB Device");
        break;
    case HDD3:
        tooltip = i18n("Hard Disk Drive/Solid State Drive 3");
        break;
    }

    return tooltip;
}

QVariant DeviceModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= m_modelList.size()) {
        return QVariant();
    }

    int device = m_devicesList.at(index.row());

    if (role == Qt::ToolTipRole) {
        return tooltip(device);
    } else if (role == Qt::DisplayRole) {
        return m_modelList.at(index.row());
    }

    return QVariant();
}

Qt::ItemFlags DeviceModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QAbstractListModel::flags(index);
    }

    return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
}

QVariant DeviceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    return i18n("Boot Devices");
}

int DeviceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_modelList.count();
}

int DeviceModel::getDeviceData() const
{
    int value = 0;
    for (int i = 0; i < MAX_BOOT_DEVICES; i++) {
        if (i < m_supportedDevices) {
            value |= (m_devicesList.at(i) << (4 * i));
        } else {
            value |= (0xf << (4 * i));
        }
    }

    return value;
}

void DeviceModel::setDeviceData(const quint32 value)
{
    if (!value) {
        return;
    }

    emit beginResetModel();
    if (!m_devicesList.isEmpty()) {
        m_devicesList.clear();
        m_modelList.clear();
    }

    for (int i = 0; i < m_supportedDevices; i++) {
        m_devicesList << ((value >> (4 * i)) & 0xf);
        m_modelList << m_devicesMap.value(m_devicesList.at(i));
    }
    emit endResetModel();
}

void DeviceModel::setSupportedDevices(int num_devices)
{
    m_supportedDevices = num_devices;
}

void DeviceModel::moveUp(const QModelIndex &index)
{
    if (!index.isValid() || index.row() >= m_devicesList.size() || index.row() < 1 || index.column() != 0) {
        return;
    }

    emit layoutAboutToBeChanged();
    QModelIndex above = index.sibling(index.row() - 1, index.column());
    m_devicesList.swap(index.row(), above.row());
    m_modelList.swap(index.row(), above.row());
    QModelIndexList from, to;
    from << index << above;
    to << above << index;
    changePersistentIndexList(from, to);
    emit layoutChanged();
}

void DeviceModel::moveDown(const QModelIndex &index)
{
    if (!index.isValid() || index.row() >= m_devicesList.size() - 1 || index.column() != 0) {
        return;
    }

    emit layoutAboutToBeChanged();
    QModelIndex below = index.sibling(index.row() + 1, index.column());
    m_devicesList.swap(index.row(), below.row());
    m_modelList.swap(index.row(), below.row());
    QModelIndexList from, to;
    from << index << below;
    to << below << index;
    changePersistentIndexList(from, to);
    emit layoutChanged();
}
