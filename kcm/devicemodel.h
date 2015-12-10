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

#ifndef DEVICEMODEL_H
#define DEVICEMODEL_H

#include <QAbstractListModel>
#include <QMap>
#include <QStringList>

#define MAX_BOOT_DEVICES 8

class DeviceModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit DeviceModel(QObject *parent = 0);
    ~DeviceModel();

    void setDeviceData(const quint32 value);
    int getDeviceData() const;
    void setSupportedDevices(int sup_devices);
    void moveUp(const QModelIndex &index);
    void moveDown(const QModelIndex &index);

    enum BootDevices {
        FDD        = 0x0,
        HDD1       = 0x1,
        ODD        = 0x2,
        LAN        = 0x3,
        USB_MEMORY = 0x4,
        HDD2       = 0x5,
        eSATA      = 0x6,
        USB_ODD    = 0x7,
        SD         = 0x8,
        USB        = 0x9,
        HDD3       = 0xa,
        NO_DEVICE  = 0xf
    };

    QVariant data(const QModelIndex& index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;

private:
    QString tooltip(int value) const;

    QMap<int, QString> m_devicesMap;
    QList<int> m_devicesList;
    QStringList m_modelList;
    int m_supportedDevices;
};

#endif // DEVICEMODEL_H
