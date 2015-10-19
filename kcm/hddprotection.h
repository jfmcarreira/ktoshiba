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

#ifndef HDDPROTECTION_H
#define HDDPROTECTION_H

#include <KSharedConfig>
#include <KConfigGroup>

#include "ui_hddprotect.h"

class KToshibaSystemSettings;

class HDDProtection : public QWidget, public Ui::HDDProtect
{
    Q_OBJECT

public:
    explicit HDDProtection(QWidget *parent = 0);

    void load();
    void save();
    void defaults();

    QStringList m_levels;

private:
    bool isHDDProtectionSupported();

    KToshibaSystemSettings *m_sys;
    KSharedConfigPtr m_config;
    KConfigGroup hdd;

    bool m_hddprotectionSupported;
    bool m_monitorHDD;
    bool m_notifyHDD;
    int m_protectionLevel;
};

#endif // HDDPROTECTION_H
