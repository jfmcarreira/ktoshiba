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

#include <QStringList>

#include <libmnl/libmnl.h>

class QSocketNotifier;

class KToshibaNetlinkEvents : public QObject
{
    Q_OBJECT

public:
    KToshibaNetlinkEvents(QObject *parent);
    ~KToshibaNetlinkEvents();

    enum HAPSEvents {
        Vibrated   = 0x80,
        Stabilized = 0x81
    };

    enum TVAPEvents {
        Hotkey               = 0x80,
        Docked               = 0x81,
        Undocked             = 0x82,
        DockStatusChanged    = 0x83,
        Thermal              = 0x88,
        SATAPower1           = 0x8b,
        SATAPower2           = 0x8c,
        LIDClosed            = 0x8f,
        LIDClosedDockEjected = 0x90,
        KBDBacklightChanged  = 0x92
    };

    bool attach();

Q_SIGNALS:
    void hapsEvent(int);
    void tvapEvent(int, int);
    void acAdapterChanged(int);

private Q_SLOTS:
    void parseEvents(int);

private:
    QString getDeviceHID();

    struct mnl_socket *m_nl;

    QSocketNotifier *m_notifier;
    QStringList m_devices;
    QString m_deviceHID;
    char m_eventBuffer[1024];
};

#endif // KTOSHIBANETLINKEVENTS_H
