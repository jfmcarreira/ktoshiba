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

#ifndef BOOTSETTINGS_H
#define BOOTSETTINGS_H

#include "ui_bootsettings.h"

class DeviceModel;
class KToshibaSystemSettings;

class BootSettings : public QWidget, public Ui::BootSettings
{
    Q_OBJECT

public:
    explicit BootSettings(QWidget *parent = 0);
    virtual ~BootSettings();

    void load();
    void save();
    void defaults();

Q_SIGNALS:
    void changed();

private Q_SLOTS:
    void deferClicked();
    void preferClicked();

private:
    bool isBootOrderSupported();
    bool isPanelPowerOnSupported();
    bool isWOKSupported();
    bool isWOLSupported();
    bool isBootSpeedSupported();

    KToshibaSystemSettings *m_sys;

    DeviceModel *m_model;
    bool m_bootOrderSupported;
    int m_bootOrder;
    int m_maxDevices;
    int m_defaultBootOrder;

    bool m_panelPowerOnSupported;
    int m_panelPowerON;

    bool m_wokSupported;
    int m_wok;
    int m_defaultWOK;

    bool m_wolSupported;
    int m_wol;
    int m_defaultWOL;

    bool m_bootSpeedSupported;
    int m_bootSpeed;
};

#endif // BOOTSETTINGS_H
