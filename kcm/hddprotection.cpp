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

#include <QtDBus/QtDBus>

#include <KConfigGroup>

extern "C" {
#include <linux/toshiba.h>
}

#include "hddprotection.h"
#include "systemsettings.h"
#include "ktoshibahardware.h"

#define CONFIG_FILE "ktoshibarc"

HDDProtection::HDDProtection(QWidget *parent)
    : QWidget(parent),
      m_sys(qobject_cast<KToshibaSystemSettings *>(QObject::parent())),
      m_config(KSharedConfig::openConfig(CONFIG_FILE))
{
    setupUi(this);

    m_levels << i18n("Off") << i18n("Low") << i18n("Medium") << i18n("High");

    m_hddprotectionSupported = isHDDProtectionSupported();
}

bool HDDProtection::isHDDProtectionSupported()
{
    m_protectionLevel = m_sys->hw()->getProtectionLevel();

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

    KConfigGroup hdd(m_config, "HDDProtection");
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

    KConfigGroup hddGroup(m_config, "HDDProtection");
    QDBusInterface iface("net.sourceforge.KToshiba",
                         "/net/sourceforge/KToshiba",
                         "net.sourceforge.KToshiba",
                         QDBusConnection::sessionBus(),
                         this);

    if (m_monitorHDD != groupBox->isChecked()) {
        hddGroup.writeEntry("MonitorHDD", !m_monitorHDD);
        hddGroup.config()->sync();
        m_monitorHDD = groupBox->isChecked();
        if (iface.isValid())
            iface.call("configFileChanged");
    }
    bool tmp = hdd_notification_checkbox->checkState() == Qt::Checked ? true : false;
    if (m_notifyHDD != tmp) {
        hddGroup.writeEntry("NotifyHDDMovement", tmp);
        hddGroup.config()->sync();
        m_notifyHDD = tmp;
        if (iface.isValid())
            iface.call("configFileChanged");
    }
    int tmp2 = protection_level_slider->value();
    if (m_protectionLevel != tmp2) {
        m_sys->hw()->setProtectionLevel(tmp2);
        m_protectionLevel = tmp2;
    }
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
        protection_level->setText(m_levels.at(2));
        protection_level_slider->setValue(2);
    }
}
