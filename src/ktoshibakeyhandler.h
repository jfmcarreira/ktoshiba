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

#ifndef KTOSHIBAKEYHANDLER_H
#define KTOSHIBAKEYHANDLER_H

#include <QStringList>

#include <libudev.h>

#define TOSNAME    "Toshiba input device"
#define TOSPHYS    "toshiba_acpi/input0"
#define TOSWMINAME "Toshiba WMI hotkeys"
#define TOSWMIPHYS "wmi/input0"

class QSocketNotifier;

class KToshibaKeyHandler : public QObject
{
    Q_OBJECT

public:
    KToshibaKeyHandler(QObject *parent);
    ~KToshibaKeyHandler();
    bool attach();

Q_SIGNALS:
    void hotkeyPressed(int);

private Q_SLOTS:
    void readHotkeys(int);
    void readMonitorData(int);

private:
    bool initUDev();
    QString findDevice();
    bool activateHotkeys();
    void deactivateHotkeys();

    struct udev *udev;
    struct udev_monitor *monitor;

    QSocketNotifier *m_hotkeysNotifier;
    QSocketNotifier *m_monitorNotifier;
    QString m_device;
    bool m_udevConnected;
    int m_socket;
};

#endif // KTOSHIBAKEYHANDLER_H
