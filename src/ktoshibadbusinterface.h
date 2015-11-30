/*
   Copyright (C) 2004-2015  Azael Avalos <coproscefalo@gmail.com>

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

#ifndef KTOSHIBA_DBUS_INTERFACE_H
#define KTOSHIBA_DBUS_INTERFACE_H

#include <QString>
#include <QDBusConnection>

class KToshibaDBusInterface : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("KToshiba D-Bus Interface", "net.sourceforge.KToshiba")

public:
    explicit KToshibaDBusInterface(QObject *parent);
    ~KToshibaDBusInterface();
    void init();

    enum ZoomActions { ZoomReset, ZoomIn, ZoomOut };
    enum ToggleActions { Disabled, Enabled };

    void lockScreen();
    void setBrightness(int);
    void setKBDBacklight(int);
    void setZoom(ZoomActions);
    void setPowerManagementInhibition(bool, QString, uint *);
    QString getBatteryProfile();

Q_SIGNALS:
    Q_SCRIPTABLE void configFileChanged();
    void zoomEffectDisabled();

public Q_SLOTS:
    Q_NOREPLY void reloadConfigFile();

private:
    bool getCompositingState();
    bool isZoomEffectActive();

    QDBusConnection m_dbus;

    bool m_service;
    bool m_object;
};

#endif  // KTOSHIBA_DBUS_INTERFACE_H
