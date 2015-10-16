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

class QTabWidget;

class KMessageWidget;

class GeneralSettings;
class HDDProtection;
class SleepUtilities;
class KeyboardSettings;
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
    GeneralSettings *m_general;
    HDDProtection *m_hdd;
    SleepUtilities *m_sleep;
    KeyboardSettings *m_kbd;
    BootSettings *m_boot;
    PowerSave *m_power;

    QString m_modelFamily;
    QString m_modelNumber;
    QString m_biosVersion;
    QString m_biosDate;
    QString m_biosManufacturer;
    QString m_ecVersion;
    QString m_driverVersion;
};

#endif // KTOSHIBASYSTEMSETTINGS_H
