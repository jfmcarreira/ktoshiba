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

private:
    bool isCoolingMethodSupported();
    bool isSATAInterfaceSupported();

    KToshibaSystemSettings *m_sys;
    KSharedConfigPtr m_config;
    KConfigGroup powersave;

    bool m_manageBatteryProfiles;
    int m_batteryProfile;

    bool m_coolingMethodSupported;
    QStringList m_coolingMethodType1;
    QStringList m_coolingMethodType2;
    int m_coolingMethod;
    int m_maxCoolingMethod;
    bool m_manageCoolingMethod;
    int m_coolingMethodBattery;
    int m_coolingMethodPlugged;

    bool m_sataInterfaceSupported;
    int m_sataInterface;
};

#endif // POWERSAVE_H
