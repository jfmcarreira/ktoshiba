/*
   Copyright (C) 2015-2016 Azael Avalos <coproscefalo@gmail.com>

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

#ifndef GENERALSETTINGS_H
#define GENERALSETTINGS_H

#include <QMap>
#include <QStringList>

#include <KConfigGroup>
#include <KSharedConfig>

#include "ui_general.h"

class KToshibaSystemSettings;

class GeneralSettings : public QWidget, public Ui::GeneralSettings
{
    Q_OBJECT

public:
    explicit GeneralSettings(QWidget *parent = 0);

    void load();
    void save();
    void defaults();

Q_SIGNALS:
    void configFileChanged();

private:
    bool isPointingDeviceSupported();
    bool isRapidChargeSupported();
    bool isUSBThreeSupported();
    bool isUSBLegacySupported();
    bool isBuiltInLANSupported();
    bool isPowerOnDisplaySupported();

    KToshibaSystemSettings *m_sys;
    KSharedConfigPtr m_config;
    KConfigGroup general;

    bool m_pointingDeviceSupported;
    quint32 m_pointingDevice;

    bool m_rapidChargeSupported;
    quint32 m_rapidCharge;

    bool m_usbThreeSupported;
    quint32 m_usbThree;

    bool m_usbLegacySupported;
    quint32 m_usbLegacy;

    bool m_builtInLANSupported;
    quint32 m_builtInLAN;

    bool m_powerOnDisplaySupported;
    int m_currentDisplayDevice;
    int m_maximumDisplayDevice;
    int m_defaultDisplayDevice;
    QMap<int, int> m_displayDevicesMap;
    QStringList m_displayDevices;
};

#endif // GENERALSETTINGS_H
