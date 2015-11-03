/*
   Copyright (C) 2013-2015  Azael Avalos <coproscefalo@gmail.com>

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

#include <QSocketNotifier>
#include <QDebug>

extern "C" {
#include <linux/input.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
}

#include "ktoshibakeyhandler.h"

KToshibaKeyHandler::KToshibaKeyHandler(QObject *parent)
    : QObject(parent),
      m_hotkeysNotifier(NULL),
      m_monitorNotifier(NULL),
      m_socket(-1)
{
}

KToshibaKeyHandler::~KToshibaKeyHandler()
{
    if (m_udevConnected) {
        udev_monitor_unref(monitor);
        udev_unref(udev);

        if (m_monitorNotifier) {
            m_monitorNotifier->setEnabled(false);
            delete m_monitorNotifier;
        }
    }

    deactivateHotkeys();
}

bool KToshibaKeyHandler::attach()
{
    m_udevConnected = initUDev();
    if (!m_udevConnected)
        return false;

    m_device = findDevice();
    if (m_device.isNull() || m_device.isEmpty()) {
        qCritical() << "Could not find a supported hotkeys input device, please check that the driver is loaded";

        return false;
    }

    return activateHotkeys();
}

bool KToshibaKeyHandler::initUDev()
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

    udev_monitor_enable_receiving(monitor);

    m_monitorNotifier = new QSocketNotifier(udev_monitor_get_fd(monitor), QSocketNotifier::Read);

    return connect(m_monitorNotifier, SIGNAL(activated(int)), this, SLOT(readMonitorData(int)));
}

bool KToshibaKeyHandler::activateHotkeys()
{
    m_socket = ::open(m_device.toLocal8Bit().constData(), O_RDONLY, 0);
    if (m_socket < 0) {
        qCritical() << "Could not open" << m_device << "for hotkeys input." << strerror(errno);

        return false;
    }

    qDebug() << "Opened" << m_device << "for hotkeys input";

    m_hotkeysNotifier = new QSocketNotifier(m_socket, QSocketNotifier::Read, this);

    return connect(m_hotkeysNotifier, SIGNAL(activated(int)), this, SLOT(readHotkeys(int)));
}

void KToshibaKeyHandler::deactivateHotkeys()
{
    if (m_hotkeysNotifier) {
        m_hotkeysNotifier->setEnabled(false);
        delete m_hotkeysNotifier;
    }

    if (m_socket)
        ::close(m_socket);
}

QString KToshibaKeyHandler::findDevice()
{
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;
    QString node, name, phys;
    bool found = false;

    // Create a list of the devices in the 'input' subsystem
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);
    // Loop until we find the correct 'input' device
    udev_list_entry_foreach(dev_list_entry, devices) {
        dev = udev_device_new_from_syspath(udev, udev_list_entry_get_name(dev_list_entry));

        // Get the device node path
        node = udev_device_get_devnode(dev);

        // Get the parent device
        dev = udev_device_get_parent(dev);

        // Get the sysattr values
        name = udev_device_get_sysattr_value(dev, "name");
        phys = udev_device_get_sysattr_value(dev, "phys");
        if ((name == TOSNAME && phys == TOSPHYS)
            || (name == TOSWMINAME && phys == TOSWMIPHYS)) {
            qDebug() << "Found device:" << node << "Name:" << name << "Phys:" << phys;
            found = true;
            udev_device_unref(dev);
            break;
        }

        udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerate);

    return found ? node : QString();
}

void KToshibaKeyHandler::readHotkeys(int socket)
{
    Q_UNUSED(socket)

    input_event event;

    int n = read(m_socket, &event, sizeof(struct input_event));
    if (n != sizeof(struct input_event)) {
        qWarning() << "Error reading hotkeys." << strerror(errno);

        return;
    }

    if (event.type == EV_KEY) {
        qDebug() << "Hotkey received from socket:" << m_socket
                 << "Key" << hex << event.code << (event.value != 0 ? "pressed" : "released");
        // Act only if we get a key press
        if (event.value == 1)
            emit hotkeyPressed(event.code);
    }
}

void KToshibaKeyHandler::readMonitorData(int socket)
{
    Q_UNUSED(socket)

    m_monitorNotifier->setEnabled(false);
    struct udev_device *dev = udev_monitor_receive_device(monitor);
    m_monitorNotifier->setEnabled(true);

    if (!dev) {
        qDebug() << "No device from socket";

        return;
    }

    QString node = udev_device_get_devnode(dev);
    QString subsys = udev_device_get_subsystem(dev);
    QString action = udev_device_get_action(dev);
    if (subsys == "input" && node == m_device && action == "remove") {
        qDebug() << "Driver unloaded, disabling hotkeys";
        deactivateHotkeys();
    }

    if (subsys == "input" && action == "add") {
        dev = udev_device_get_parent(dev);
        QString name = udev_device_get_sysattr_value(dev, "name");
        QString phys = udev_device_get_sysattr_value(dev, "phys");
        if ((name == TOSNAME && phys == TOSPHYS)
            || (name == TOSWMINAME && phys == TOSWMIPHYS)) {
            qDebug() << "Driver re-loaded, re-enabling hotkeys";
            m_device = node;
            activateHotkeys();
        }
    }

    udev_device_unref(dev);
}
