/*
   Copyright (C) 2013  Azael Avalos <coproscefalo@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QFile>
#include <QSocketNotifier>

#include <KDebug>

extern "C" {
#include <linux/input.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <libudev.h>
}

#include "ktoshibakeyhandler.h"

KToshibaKeyHandler::KToshibaKeyHandler(QObject *parent)
    : QObject( parent )
{
    m_Device = findDevice();
    if (m_Device.isNull() || m_Device.isEmpty()) {
        kError() << "No device to monitor...";
        exit(0);
    }

    m_Fd = ::open(m_Device.toLocal8Bit().constData(), O_RDONLY, 0);
    if (m_Fd >= 0) {
        kDebug() << "Opened " << m_Device << " as keyboard input" << endl;
        m_Notifier = new QSocketNotifier(m_Fd, QSocketNotifier::Read, this);
        connect( m_Notifier, SIGNAL( activated(int) ), this, SLOT( readData() ) );
    } else {
        kWarning() << "Cannot open " << m_Device.toLocal8Bit().constData() << "for keyboard input ("
                   << strerror(errno) << ")" << endl;
    }
}

KToshibaKeyHandler::~KToshibaKeyHandler()
{
    if (m_Fd >= 0)
        ::close(m_Fd);
}

QString KToshibaKeyHandler::findDevice()
{
    QString nodepath;
    QString sysattr1;
    QString sysattr2;
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;

    // Create the udev object
    udev = udev_new();
    if (!udev) {
        kError() << "Can't create udev";
        return nodepath;
    }

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
        nodepath = QString(udev_device_get_devnode(dev));

        // Get the parent device
        dev = udev_device_get_parent(dev);

        // Get the sysattr values
        sysattr1 = QString(udev_device_get_sysattr_value(dev,"name"));
        sysattr2 = QString(udev_device_get_sysattr_value(dev,"phys"));
        if (sysattr1 == "Toshiba input device" && sysattr2 == "toshiba_acpi/input0") {
            kDebug() << "Found device: " << nodepath << endl
                     << "  Name: " << sysattr1 << endl
                     << "  Phys: " << sysattr2 << endl;
            udev_device_unref(dev);
            udev_enumerate_unref(enumerate);
            udev_unref(udev);
            return nodepath;
        }

        udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);

    udev_unref(udev);

    return nodepath;
}

void KToshibaKeyHandler::readData()
{
    input_event event;

    int n = read(m_Fd, &event, sizeof(input_event));
    if (n != 16) {
        kDebug() << "key pressed: n=" << n;
        return;
    }

    // Act only if we get a key press
    if (event.type == EV_KEY && event.value == 1) {
        kDebug() << "key pressed:"
                 << " code= " << event.code
                 << " value= " << event.value
                 << ((event.value != 0) ? "(Down)" : "(Up)");

        emit hotkeyPressed(event.code);
    }
}


#include "ktoshibakeyhandler.moc"
