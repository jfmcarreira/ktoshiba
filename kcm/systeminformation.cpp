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

#include <QDir>
#include <QDebug>
#include <QStringBuilder>

#include <KLocalizedString>

#include "systeminformation.h"
#include "systemsettings.h"
#include "ktoshibahardware.h"

SystemInformation::SystemInformation(QWidget *parent)
    : QWidget(parent),
      m_sys(qobject_cast<KToshibaSystemSettings * >(QObject::parent()))
{
    setupUi(this);

    m_files << "product_name" << "product_version" << "bios_version" << "bios_date" << "bios_vendor";

    getData();

    m_deviceHID = m_sys->hw()->getDeviceHID();
}

void SystemInformation::getData()
{
    QDir dir;
    if (!dir.exists("/sys/devices/virtual/dmi/id/")) {
        qWarning() << "DMI information directory could not be found under sysfs";
        foreach (const QString &file, m_files) {
            qDebug() << "Setting" << file;
            m_data << i18n("Unknown");
        }

        return;
    }

    foreach (const QString &file, m_files) {
        m_file.setFileName("/sys/devices/virtual/dmi/id/" % file);
        if (m_file.open(QIODevice::ReadOnly)) {
            m_data << m_file.readLine();
            m_file.close();
        } else {
            m_data << i18n("Unknown");
        }
    }
}

QString SystemInformation::getDriverVersion()
{
    m_file.setFileName("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/" % m_deviceHID % "/version");
    if (m_file.exists() && m_file.open(QIODevice::ReadOnly)) {
        QString version = m_file.readLine();
        m_file.close();

        return version;
    }

    m_file.setFileName("/proc/acpi/toshiba/version");
    if (m_file.exists() && m_file.open(QIODevice::ReadOnly)) {
        QString line = m_file.readLine();
        QStringList split = line.split(":");
        m_file.close();

        return split[1].trimmed();
    }

    qCritical() << "No version file detected or toshiba_acpi module not loaded";

    return QString("0.0");
}

void SystemInformation::load()
{
    model_family->setText(m_data.at(0));
    model_number->setText(m_data.at(1));
    bios_version->setText(m_data.at(2));
    bios_date->setText(m_data.at(3));
    bios_manufacturer->setText(m_data.at(4));
    driver_version->setText(getDriverVersion());
}
