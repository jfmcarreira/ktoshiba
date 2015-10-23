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

#ifndef SLEEPUTILITIES_H
#define SLEEPUTILITIES_H

#include <QMap>
#include <QStringList>

#include "ui_sleeputils.h"

class KToshibaSystemSettings;

class SleepUtilities : public QWidget, public Ui::SleepUtils
{
    Q_OBJECT

public:
    explicit SleepUtilities(QWidget *parent = 0);

    void load();
    void save();
    void defaults();

private:
    bool isSleepChargeSupported();
    bool isSleepMusicSupported();

    KToshibaSystemSettings *m_sys;

    bool m_sleepChargeSupported;
    int m_sleepCharge;
    int m_maxSleepCharge;
    int m_defaultSleepCharge;
    QMap<int, int> m_sleepModesMap;
    QStringList m_sleepModes;
    int m_batteryEnabled;
    int m_batteryLevel;

    bool m_sleepMusicSupported;
    int m_sleepMusic;
};

#endif // SLEEPUTILITIES_H
