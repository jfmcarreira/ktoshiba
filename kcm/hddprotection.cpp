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

#include "hddprotection.h"
#include "systemsettings.h"
#include "ktoshibahardware.h"

HDDProtection::HDDProtection(QWidget *parent)
    : QWidget(parent),
      m_sys(qobject_cast<KToshibaSystemSettings * >(QObject::parent())),
      m_config(KSharedConfig::openConfig("ktoshibarc"))
{
    setupUi(this);

    m_levels << i18n("Off") << i18n("Low") << i18n("Medium") << i18n("High");

    m_hddprotectionSupported = isHDDProtectionSupported();

    hdd = KConfigGroup(m_config, "HDDProtection");
    if (!hdd.exists()) {
        hdd.writeEntry("MonitorHDD", true);
        hdd.writeEntry("NotifyHDDMovement", true);
        hdd.writeEntry("ProtectionLevel", 2);
        hdd.sync();
    }
}

bool HDDProtection::isHDDProtectionSupported()
{
    m_protectionLevel = m_sys->hw()->getHDDProtectionLevel();

    if (m_protectionLevel == KToshibaHardware::FAILURE)
        return false;

    return true;
}

void HDDProtection::load()
{
    if (!m_hddprotectionSupported) {
        groupBox->setEnabled(false);

        return;
    }
    m_monitorHDD = hdd.readEntry("MonitorHDD", true);
    m_notifyHDD = hdd.readEntry("NotifyHDDMovement", true);

    groupBox->setChecked(m_monitorHDD);
    hdd_notification_checkbox->setChecked(m_notifyHDD);
    protection_level->setText(m_levels.at(m_protectionLevel));
    protection_level_slider->setValue(m_protectionLevel);
}

void HDDProtection::save()
{
    if (!m_hddprotectionSupported)
        return;

    if (m_monitorHDD != groupBox->isChecked()) {
        hdd.writeEntry("MonitorHDD", !m_monitorHDD);
        m_monitorHDD = groupBox->isChecked();
        emit configFileChanged();
    }
    bool tmp = hdd_notification_checkbox->checkState() == Qt::Checked ? true : false;
    if (m_notifyHDD != tmp) {
        hdd.writeEntry("NotifyHDDMovement", tmp);
        m_notifyHDD = tmp;
        emit configFileChanged();
    }
    int tmp2 = protection_level_slider->value();
    if (m_protectionLevel != tmp2) {
        hdd.writeEntry("ProtectionLevel", tmp2);
        m_sys->hw()->setHDDProtectionLevel(tmp2);
        m_protectionLevel = tmp2;
        emit configFileChanged();
    }
    hdd.sync();
}

void HDDProtection::defaults()
{
    if (!m_hddprotectionSupported)
        return;

    if (!m_monitorHDD)
        groupBox->setChecked(true);
    if (!m_notifyHDD)
        hdd_notification_checkbox->setChecked(true);
    if (m_protectionLevel != 2) {
        protection_level->setText(m_levels.at(Medium));
        protection_level_slider->setValue(Medium);
    }
}
