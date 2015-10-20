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

#ifndef GENERALSETTINGS_H
#define GENERALSETTINGS_H

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

private:
    bool isTouchPadSupported();
    bool isRapidChargeSupported();
    bool isUSBThreeSupported();

    KToshibaSystemSettings *m_sys;

    bool m_touchpadSupported;
    quint32 m_touchpad;

    bool m_rapidChargeSupported;
    quint32 m_rapidCharge;

    bool m_usbThreeSupported;
    quint32 m_usbThree;
};

#endif // GENERALSETTINGS_H
