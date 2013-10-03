/*
   Copyright (C) 2004-2011  Azael Avalos <coproscefalo@gmail.com>

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

#ifndef KTOSHIBA_DBUS_INTERFACE_H
#define KTOSHIBA_DBUS_INTERFACE_H

#include <QObject>
#include <QString>
#include <QtDBus/QDBusMessage>

class QDBusInterface;
class QDBusVariant;
class QString;

class KToshibaDBusInterface : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("KToshiba D-Bus Interface", "net.sourceforge.KToshiba")

public:
    enum zoomActions { ZoomReset = 0, ZoomIn = 1, ZoomOut = -1 };

public:
    KToshibaDBusInterface(QObject *parent);

    bool checkCompositeStatus();
    void lockScreen();
    void setZoom(int);

Q_SIGNALS:
    void profileChanged(QString);

public Q_SLOTS:
    void gotKey(int key);
};

#endif	// KTOSHIBA_DBUS_INTERFACE_H
