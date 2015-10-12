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

    bool m_bootOrderSupported;
    bool m_panelPowerOnSupported;
    bool m_wokSupported;
    bool m_wolSupported;

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

    KToshibaSystemSettings *m_sys;

    DeviceModel *m_model;
    int m_order;
    int m_maxdev;
    int m_default;

    int m_panelpower;

    int m_wok;
    int m_defaultWOK;

    int m_wol;
    int m_defaultWOL;
};

#endif // BOOTSETTINGS_H
