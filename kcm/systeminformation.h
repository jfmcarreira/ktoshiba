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

#ifndef SYSTEMINFORMATION_H
#define SYSTEMINFORMATION_H

#include <QFile>
#include <QStringList>

#include "ui_sysinfo.h"

class KToshibaSystemSettings;

class SystemInformation : public QWidget, public Ui::SysInfo
{
    Q_OBJECT

public:
    explicit SystemInformation(QWidget *parent);

    void load();

private:
    void getData();
    QString getDriverVersion();

    KToshibaSystemSettings *m_sys;

    QFile m_file;
    QStringList m_files;
    QStringList m_data;
    QString m_deviceHID;
};

#endif // SYSTEMINFORMATION_H
