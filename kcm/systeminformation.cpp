/*
   Copyright (C) 2015-2016 Azael Avalos <coproscefalo@gmail.com>

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
#include <QStringBuilder>

#include <KLocalizedString>

#include <ktoshiba_debug.h>

#include "systeminformation.h"

#define SYSFS_DEVICE_DIR "/sys/devices/LNXSYSTM:00/LNXSYBUS:00/"
#define SYSFS_DMI_DIR "/sys/devices/virtual/dmi/id/"

SystemInformation::SystemInformation(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    m_files << QStringLiteral("product_name") << QStringLiteral("product_version")
            << QStringLiteral("bios_version") << QStringLiteral("bios_date")
            << QStringLiteral("bios_vendor");

    getData();

    m_deviceHID = getDeviceHID();
}

void SystemInformation::getData()
{
    QDir dir(QStringLiteral(SYSFS_DMI_DIR));
    if (!dir.exists()) {
        qCWarning(KTOSHIBA) << "DMI information directory could not be found under sysfs";
        Q_FOREACH (const QString &file, m_files) {
            qCDebug(KTOSHIBA) << "Setting" << file;
            m_data.append(i18n("Unknown"));
        }

        return;
    }

    Q_FOREACH (const QString &file, m_files) {
        m_file.setFileName(QStringLiteral(SYSFS_DMI_DIR) % file);
        if (!m_file.open(QIODevice::ReadOnly)) {
            m_data.append(i18n("Unknown"));
        }

        QTextStream data(&m_file);
        m_data.append(data.readLine().simplified());
        m_file.close();
    }
}

QString SystemInformation::getDeviceHID()
{
    m_devices << QStringLiteral("TOS1900:00") << QStringLiteral("TOS6200:00")
              << QStringLiteral("TOS6207:00") << QStringLiteral("TOS6208:00");

    QDir dir;
    Q_FOREACH (const QString &device, m_devices) {
        if (dir.exists(QStringLiteral(SYSFS_DEVICE_DIR) % device)) {
            return device;
        }
    }

    qCWarning(KTOSHIBA) << "No known kernel interface found" << endl;

    return QString();
}

QString SystemInformation::getDriverVersion()
{
    m_file.setFileName(QStringLiteral(SYSFS_DEVICE_DIR) % m_deviceHID % QStringLiteral("/version"));
    if (m_file.exists() && m_file.open(QIODevice::ReadOnly)) {
        QTextStream data(&m_file);
        QString version = data.readLine().simplified();
        m_file.close();

        return version;
    }

    m_file.setFileName(QStringLiteral("/proc/acpi/toshiba/version"));
    if (m_file.exists() && m_file.open(QIODevice::ReadOnly)) {
        QTextStream data(&m_file);
        QString line = data.readLine().simplified();
        QStringList split = line.split(QStringLiteral(":"));
        m_file.close();

        return split[1].trimmed();
    }

    qCCritical(KTOSHIBA) << "No version file detected or toshiba_acpi module not loaded";

    return QStringLiteral("0.0");
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
