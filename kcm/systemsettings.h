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

#ifndef KTOSHIBASYSTEMSETTINGS_H
#define KTOSHIBASYSTEMSETTINGS_H

#include <KCModule>
#include <KSharedConfig>

#include "ui_sysinfo.h"
#include "ui_general.h"
#include "ui_hddprotect.h"
#include "ui_sleeputils.h"
#include "ui_keyboard.h"

class QTabWidget;

class KMessageWidget;

class BootSettings;
class PowerSave;
class KToshibaHardware;

class KToshibaSystemSettings : public KCModule
{
    Q_OBJECT

public:
    explicit KToshibaSystemSettings(QWidget *parent,
                                    const QVariantList &args = QVariantList());
    ~KToshibaSystemSettings();

    void load();
    void save();
    void defaults();

    KToshibaHardware *hw() const
    {
        return m_hw;
    }

private Q_SLOTS:
    void configChanged();
    void configChangedReboot();
    void protectionLevelChanged(int);
    void batteryLevelChanged(int);
    void kbdBacklightChanged(int);
    void kbdTimeoutChanged(int);
    void updateTouchPad(int);

private:
    void addTabs();
    void showRebootMessage();

    KToshibaHardware *m_hw;
    bool m_hwAttached;

    QTabWidget *m_tabWidget;
    KMessageWidget *m_message;
    KSharedConfigPtr m_config;

    Ui::SysInfo m_sysinfo;
    Ui::General m_general;
    Ui::HDDProtect m_hdd;
    Ui::SleepUtils m_sleep;
    Ui::Keyboard m_kbd;
    BootSettings *m_boot;
    PowerSave *m_power;

    QString m_modelFamily;
    QString m_modelNumber;
    QString m_biosVersion;
    QString m_biosDate;
    QString m_biosManufacturer;
    QString m_ecVersion;
    QString m_driverVersion;

    int m_touchpad;
    int m_rapidcharge;
    int m_usbthree;

    bool m_monitorHDD;
    bool m_notifyHDD;
    QStringList m_levels;
    int m_level;

    quint32 sleep_charge;
    int m_sleepcharge;
    int m_defaultsc;
    int m_sleepmusic;
    QStringList m_sleeponbat;
    int m_batenabled;
    int m_batlevel;

    int m_functions;
    QString m_functionsToolTip;
    int m_type;
    QStringList m_type1;
    QStringList m_type2;
    QString m_tooltip;
    int m_mode;
    int m_index;
    int m_time;
};

#endif // KTOSHIBASYSTEMSETTINGS_H
