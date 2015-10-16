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

#ifndef KTOSHIBANETLINKEVENTS_H
#define KTOSHIBANETLINKEVENTS_H

#include <QString>

extern "C" {
#include <sys/socket.h>
#include <linux/genetlink.h>
}

/*
 * The following struct was taken from
 * linux ACPI event.c
 */
typedef char acpi_device_class[20];

struct acpi_genl_event {
    acpi_device_class device_class;
    char bus_id[15];
    unsigned int type;
    unsigned int data;
};

class QSocketNotifier;

class KToshibaNetlinkEvents : public QObject
{
    Q_OBJECT

public:
    KToshibaNetlinkEvents(QObject *parent);
    ~KToshibaNetlinkEvents();

    enum HDDState {
        Vibrated   = 0x80,
        Stabilized = 0x81
    };

    enum SysEvents {
        Hotkey               = 0x80,
        DockDocked           = 0x81,
        DockUndocked         = 0x82,
        DockStatusChanged    = 0x83,
        Thermal              = 0x88,
        SATAPower1           = 0x8b,
        SATAPower2           = 0x8c,
        LIDClosed            = 0x8f,
        LIDClosedDockEjected = 0x90,
        KBDBacklight         = 0x92
    };

    bool attach();
    void detach();
    void setDeviceHID(QString);

Q_SIGNALS:
    void hapsEvent(int);
    void tvapEvent(int);

private Q_SLOTS:
    void parseEvents(int);

private:
    struct sockaddr_nl m_nl;
    struct genlmsghdr *m_genlmsghdr = NULL;
    struct nlattr *m_nlattrhdr = NULL;
    struct acpi_genl_event *m_event = NULL;

    QSocketNotifier *m_notifier;
    QString m_deviceHID;
    const int m_buffer = 16 * 1024 * 1024;
    char m_event_buf[1024];
    int m_socket;
};

#endif // KTOSHIBANETLINKEVENTS_H
