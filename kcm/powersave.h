/*
   Copyright (C) 2015 Azael Avalos <coproscefalo@gmail.com>

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

#ifndef POWERSAVE_H
#define POWERSAVE_H

#include <QStringList>

#include <KSharedConfig>
#include <KConfigGroup>

#include "ui_powersave.h"

class KToshibaSystemSettings;

class PowerSave : public QWidget, public Ui::PowerSave
{
    Q_OBJECT

public:
    explicit PowerSave(QWidget *parent = 0);

    void load();
    void save();
    void defaults();

    enum BatteryProfiles { Performance, Powersave, Presentation, ECO };

Q_SIGNALS:
    void configFileChanged();

public Q_SLOTS:
    void loadProfile(int);

private:
    bool isCoolingMethodSupported();
    bool isSATAInterfaceSupported();
    bool isODDPowerSupported();
    bool isIlluminationLEDSupported();
    void saveProfile(int);

    KToshibaSystemSettings *m_sys;
    KSharedConfigPtr m_config;
    KConfigGroup powersave;

    int m_batteryProfile;
    int m_cooling;
    int m_sata;
    int m_odd;
    int m_illumination;

    bool m_coolingMethodSupported;
    QStringList m_coolingMethods;
    int m_coolingMethod;
    int m_maxCoolingMethod;

    bool m_sataInterfaceSupported;
    int m_sataInterface;

    bool m_oddPowerSupported;
    int m_oddPower;

    bool m_illuminationLEDSupported;
    int m_illuminationLED;
};

#endif // POWERSAVE_H
