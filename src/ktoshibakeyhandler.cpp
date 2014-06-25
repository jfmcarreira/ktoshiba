/*
   Copyright (C) 2013-2014  Azael Avalos <coproscefalo@gmail.com>

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
}

#include "ktoshibakeyhandler.h"

KToshibaKeyHandler::KToshibaKeyHandler(QObject *parent)
    : QObject( parent )
{
    initUDev();

    setNotifier();

    connect( this, SIGNAL( nodeChanged(QString) ), this, SLOT( changeNode(QString) ));

    //pollUDev();
}

KToshibaKeyHandler::~KToshibaKeyHandler()
{
    if (m_fd >= 0)
        ::close(m_fd);

    udev_unref(udev);
}

void KToshibaKeyHandler::initUDev()
{
    // Create the udev object
    udev = udev_new();
    if (!udev) {
        kError() << "Cannot create the udev object" << endl;
        exit(1);
    }

    // Create the udev monitor
    monitor = udev_monitor_new_from_netlink(udev, "udev");
    if (!monitor) {
        kError() << "Cannot create the udev monitor" << endl;
        exit(1);
    }
    
    // Add filters
    if (udev_monitor_filter_add_match_subsystem_devtype(monitor, SUBSYS, NULL) < 0) {
        kError() << "Cannot add udev filter" << endl;
        exit(1);
    }

    if (udev_monitor_enable_receiving(monitor) < 0) {
        kError() << "Cannot enable monitor receiving" << endl;
        exit(1);
    }

    fd = udev_monitor_get_fd(monitor);

    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;

    // Create a list of the devices in the 'input' subsystem
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, SUBSYS);
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);
    // Loop until we find the correct 'input' device
    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

        // Get the device node path
        QString nodepath = QString(udev_device_get_devnode(dev));

        // Get the parent device
        dev = udev_device_get_parent(dev);

        // Get the sysattr values
        QString name(udev_device_get_sysattr_value(dev, "name"));
        QString phys(udev_device_get_sysattr_value(dev, "phys"));
        if (name == TOSHNAME && phys == TOSHPHYS) {
            kDebug() << "Found device: " << nodepath << endl
                     << "  Name: " << name << endl
                     << "  Phys: " << phys << endl;
            m_device = nodepath;
            udev_device_unref(dev);
            break;
        }

        udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerate);

    if (m_device.isNull() || m_device.isEmpty()) {
        kError() << "No device to monitor...";
        exit(1);
    }
}

void KToshibaKeyHandler::checkDevice(struct udev_device *device)
{
    // Get the device node path
    QString nodepath(udev_device_get_devnode(device));

    device = udev_device_get_parent(device);

    QString name(udev_device_get_sysattr_value(device, "name"));
    QString phys(udev_device_get_sysattr_value(device, "phys"));
    if (name == TOSHNAME && phys == TOSHPHYS) {
        kDebug() << "Found device: " << nodepath << endl
                 << "  Name: " << name << endl
                 << "  Phys: " << phys << endl;
        emit nodeChanged(nodepath);
    }
}

void KToshibaKeyHandler::pollUDev()
{
    while (1) {
        int ret;
        fd_set fds;
        struct timeval tv;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        ret = select(fd+1, &fds, NULL, NULL, &tv);
        if (ret > 0 && FD_ISSET(fd, &fds)) {
            kDebug() << "A device changed..." << endl;

            struct udev_device *dev = udev_monitor_receive_device(monitor);
            if (dev)
                checkDevice(dev);
            else
                kError() << "No device received..." << endl;
        }
        usleep(250*1000);
    }
}

void KToshibaKeyHandler::setNotifier()
{
    m_fd = ::open(m_device.toLocal8Bit().constData(), O_RDONLY, 0);
    if (m_fd >= 0) {
        kDebug() << "Opened " << m_device << " as keyboard input" << endl;
        m_notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
        connect( m_notifier, SIGNAL( activated(int) ), this, SLOT( readData(int) ) );
    } else {
        kError() << "Cannot open " << m_device << " for keyboard input ("
                 << strerror(errno) << ")" << endl;
        exit(1);
    }
}

void KToshibaKeyHandler::changeNode(QString nodepath)
{
    if (m_fd >= 0)
        ::close(m_fd);

    disconnect( m_notifier, SLOT( readData(int) ) );
    m_device = nodepath;

    setNotifier();
}

void KToshibaKeyHandler::readData(int socket)
{
    input_event event;

    kDebug() << "Received data from socket: " << socket << endl;

    int n = read(m_fd, &event, sizeof(input_event));
    if (n != 24) {
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
