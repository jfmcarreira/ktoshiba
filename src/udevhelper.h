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

#ifndef UDEVHELPER_H
#define UDEVHELPER_H

#include <QtCore/QStringList>

#include <libudev.h>

#define TOSHNAME "Toshiba input device"
#define TOSHPHYS "toshiba_acpi/input0"

class UDevHelper : public QObject
{
    Q_OBJECT

public:
    UDevHelper(QObject *parent);
    bool initUDev();

    QString findDevice(QStringList);

private:
    struct udev *udev;
    struct udev_monitor *monitor;
};

#endif // UDEVHELPER_H