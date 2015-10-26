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
#include "udevhelper.h"

KToshibaKeyHandler::KToshibaKeyHandler(QObject *parent)
    : QObject(parent),
      m_udevHelper(new UDevHelper(this)),
      m_notifier(NULL),
      m_socket(-1)
{
    m_namePhys << TOSHNAME << TOSHPHYS << TOSWMINAME << TOSWMIPHYS;
}

KToshibaKeyHandler::~KToshibaKeyHandler()
{
    if (m_notifier) {
        m_notifier->setEnabled(false);
        delete m_notifier; m_notifier = NULL;
    }
    if (m_socket)
        ::close(m_socket);

    delete m_udevHelper; m_udevHelper = NULL;
}

bool KToshibaKeyHandler::attach()
{
    m_udevConnected = m_udevHelper->initUDev();
    if (!m_udevConnected)
        return false;

    m_device = m_udevHelper->findDevice(m_namePhys);
    if (m_device.isNull() || m_device.isEmpty()) {
        qCritical() << "No device to monitor...";

        return false;
    }

    m_socket = ::open(m_device.toLocal8Bit().constData(), O_RDONLY, 0);
    if (m_socket < 0) {
        qCritical() << "Could not open" << m_device << "for hotkeys input."
                    << strerror(errno);

        return false;
    }

    qDebug() << "Opened" << m_device << "for hotkeys input";

    m_notifier = new QSocketNotifier(m_socket, QSocketNotifier::Read, this);

    connect(m_notifier, SIGNAL(activated(int)), this, SLOT(readData(int)));

    return true;
}

void KToshibaKeyHandler::readData(int socket)
{
    if (socket < 0)
        return;

    input_event event;

    int n = read(socket, &event, sizeof(struct input_event));
    if (n != sizeof(struct input_event)) {
        qCritical() << "Error reading hotkeys." << strerror(errno);

        return;
    }

    if (event.type == EV_KEY) {
        qDebug() << "Received data from socket:" << socket << endl
                 << "Key" << hex << event.code << (event.value != 0 ? "pressed" : "released");

        // Act only if we get a key press
        if (event.value == 1)
            emit hotkeyPressed(event.code);
    }
}
