/*
   Copyright (C) 2014-2015  Azael Avalos <coproscefalo@gmail.com>

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

#include "udevhelper.h"

UDevHelper::UDevHelper(QObject *parent)
    : QObject(parent)
{
}

bool UDevHelper::initUDev()
{
    // Create the udev object
    udev = udev_new();
    if (!udev) {
        qCritical() << "Cannot create the udev object";

        return false;
    }

    // Create the udev monitor
    monitor = udev_monitor_new_from_netlink(udev, "udev");
    if (!monitor) {
        qCritical() << "Cannot create the udev monitor";

        return false;
    }

    // Add filters
    if (udev_monitor_filter_add_match_subsystem_devtype(monitor, "input", NULL) < 0) {
        qCritical() << "Cannot add udev filter";

        return false;
    }

    return true;
}

QString UDevHelper::findDevice(QStringList namePhys)
{
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;
    QString node;

    // Create a list of the devices in the 'input' subsystem
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);
    // Loop until we find the correct 'input' device
    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

        // Get the device node path
        QString nodepath(udev_device_get_devnode(dev));

        // Get the parent device
        dev = udev_device_get_parent(dev);

        // Get the sysattr values
        QString name(udev_device_get_sysattr_value(dev, "name"));
        QString phys(udev_device_get_sysattr_value(dev, "phys"));
        if (name == namePhys.at(0) && phys == namePhys.at(1)) {
            node = nodepath;
            qDebug() << "Found device:" << nodepath << endl
                     << "  Name:" << name << endl
                     << "  Phys:" << phys << endl;
        } else if (name == namePhys.at(2) && phys == namePhys.at(3)) {
            node = nodepath;
            qDebug() << "Found device:" << nodepath << endl
                     << "  Name:" << name << endl
                     << "  Phys:" << phys << endl;
        }

        udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerate);
    udev_unref(udev);

    return node;
}
