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

#include "devicemodel.h"

DeviceModel::DeviceModel(QObject *parent)
    : QAbstractListModel(parent),
      m_supported(MAX_BOOT_DEVICES)
{
    m_map[FDD] = "FDD";
    m_map[HDD1] = "HDD/SSD 1";
    m_map[ODD] = "ODD";
    m_map[LAN] = "LAN";
    m_map[USB_MEMORY] = "USB MEMORY";
    m_map[HDD2] = "HDD/SSD 2";
    m_map[eSATA] = "eSATA";
    m_map[USB_ODD] = "USB ODD";
    m_map[SD] = "SD";
    m_map[USB] = "USB";
    m_map[HDD3] = "HDD/SSD 3";
    m_map[NO_DEVICE] = "NO DEVICE";
}

DeviceModel::~DeviceModel()
{
    m_map.clear();
    m_list.clear();
    m_devices.clear();
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
    if (index.row() < 0 || index.row() >= m_list.size())
        return QVariant();

    int device = m_devices.at(index.row());

    if (role == Qt::ToolTipRole)
        return tooltip(device);
    else if (role == Qt::DisplayRole)
        return m_list.at(index.row());

    return QVariant();
}

Qt::ItemFlags DeviceModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractListModel::flags(index);

    return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
}

QVariant DeviceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)

    if (role != Qt::DisplayRole)
        return QVariant();

    return i18n("Boot Devices");
}

int DeviceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_list.count();
}

quint32 DeviceModel::getDeviceData() const
{
    quint32 value = 0;
    for (int i = 0; i < MAX_BOOT_DEVICES; i++) {
        if (i < m_supported)
            value |= (m_devices.at(i) << (4 * i));
        else
            value |= (0xf << (4 * i));
    }

    return value;
}

void DeviceModel::setDeviceData(const quint32 value)
{
    if (!value)
        return;

    emit beginResetModel();
    if (!m_devices.isEmpty()) {
        m_devices.clear();
        m_list.clear();
    }

    for (int i = 0; i < m_supported; i++) {
        m_devices << ((value >> (4 * i)) & 0xf);
        m_list << m_map.value(m_devices.at(i));
    }
    emit endResetModel();
}

void DeviceModel::setSupportedDevices(int sup_devices)
{
    m_supported = sup_devices;
}

void DeviceModel::moveUp(const QModelIndex &index)
{
    if (!index.isValid() || index.row() >= m_devices.size() || index.row() < 1 || index.column() != 0)
        return;

    emit layoutAboutToBeChanged();
    QModelIndex above = index.sibling(index.row() - 1, index.column());
    m_devices.swap(index.row(), above.row());
    m_list.swap(index.row(), above.row());
    QModelIndexList from, to;
    from << index << above;
    to << above << index;
    changePersistentIndexList(from, to);
    emit layoutChanged();
}

void DeviceModel::moveDown(const QModelIndex &index)
{
    if (!index.isValid() || index.row() >= m_devices.size() - 1 || index.column() != 0)
        return;

    emit layoutAboutToBeChanged();
    QModelIndex below = index.sibling(index.row() + 1, index.column());
    m_devices.swap(index.row(), below.row());
    m_list.swap(index.row(), below.row());
    QModelIndexList from, to;
    from << index << below;
    to << below << index;
    changePersistentIndexList(from, to);
    emit layoutChanged();
}
