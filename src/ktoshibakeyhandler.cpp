/*
   Copyright (C) 2013-2015  Azael Avalos <coproscefalo@gmail.com>

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

#include <QtCore/QSocketNotifier>

#include <KDebug>

extern "C" {
#include <linux/input.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
}

#include "ktoshibakeyhandler.h"
#include "udevhelper.h"

KToshibaKeyHandler::KToshibaKeyHandler(QObject *parent)
    : QObject( parent ),
      m_udevHelper( new UDevHelper( this ) )
{
    m_namePhys << TOSHNAME << TOSHPHYS;

    m_device = m_udevHelper->findDevice(m_namePhys);
    if (m_device.isNull() || m_device.isEmpty()) {
        kError() << "No device to monitor...";
        exit(-10);
    }

    m_socket = getSocket();
    if (m_socket < 0) {
        kError() << "Could not get socket";

        exit(-20);
    }

    m_notifier = new QSocketNotifier(m_socket, QSocketNotifier::Read, this);

    connect( m_notifier, SIGNAL( activated(int) ), this, SLOT( readData(int) ) );
}

KToshibaKeyHandler::~KToshibaKeyHandler()
{
    if (m_socket >= 0)
        ::close(m_socket);

    delete m_udevHelper; m_udevHelper = NULL;
}

int KToshibaKeyHandler::getSocket()
{
    int fd = ::open(m_device.toLocal8Bit().constData(), O_RDONLY, 0);
    if (fd < 0) {
        kError() << "Cannot open" << m_device << "for keyboard input."
                 << strerror(errno);
        exit(-30);
    }

    kDebug() << "Opened" << m_device << "as keyboard input";

    return fd;
}

void KToshibaKeyHandler::readData(int socket)
{
    if (socket < 0)
        return;

    input_event event;

    int n = read(socket, &event, sizeof(struct input_event));
    if (n != sizeof(struct input_event)) {
        kError() << "Error reading hotkeys" << n;
        return;
    }

    kDebug() << "Received data from socket:" << socket;

    if (event.type == EV_KEY) {
        kDebug() << "Key received:" << endl
                 << "  code=" << event.code << endl
                 << "  value=" << event.value
                 << ((event.value != 0) ? "(pressed)" : "(released)");

        // Act only if we get a key press
        if (event.value == 1)
            emit hotkeyPressed(event.code);
    }
}


#include "ktoshibakeyhandler.moc"
