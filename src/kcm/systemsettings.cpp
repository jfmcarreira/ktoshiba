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

#include <QScrollArea>
#include <QLayout>

#include <KPluginFactory>
#include <KAboutData>
#include <KTabWidget>
#include <KConfigGroup>

#include "systemsettings.h"
#include "../helperactions.h"
#include "../version.h"

#define CONFIG_FILE "ktoshibarc"

K_PLUGIN_FACTORY(KToshibaSystemSettingsFactory, registerPlugin<KToshibaSystemSettings>();)
K_EXPORT_PLUGIN(KToshibaSystemSettingsFactory("kcm_ktoshibam"))

KToshibaSystemSettings::KToshibaSystemSettings( QWidget *parent, const QVariantList &args )
    : KCModule( KToshibaSystemSettingsFactory::componentData(), parent, args ),
      m_helper( new HelperActions(this) ),
      m_config( KSharedConfig::openConfig( CONFIG_FILE ) ),
      m_helperAttached( false )
{
    KAboutData *about = new KAboutData("kcm_ktoshibam",
				QByteArray(),
				ki18n("KToshiba KCM"),
				ktoshiba_version,
				ki18n("KToshiba System Settings"),
				KAboutData::License_GPL_V2,
				ki18n("Copyright © 2015 Azael Avalos"),
				KLocalizedString(),
				"http://ktoshiba.sourceforge.net/");

    about->addAuthor(ki18n("Azael Avalos"),
		     ki18n("Original author"),
		     "coproscefalo@gmail.com");

    setAboutData(about);

    QGridLayout *layout = new QGridLayout(this);

    m_tabWidget = new KTabWidget(this);
    layout->addWidget(m_tabWidget, 0, 0, 0);

    QWidget *sysinfo_widget = new QWidget(this);
    m_sysinfo.setupUi(sysinfo_widget);
    sysinfo_widget->setContentsMargins(20, 20, 20, 20);
    m_tabWidget->addTab(sysinfo_widget, i18n("System Information"));

    m_helperAttached = m_helper->init();
    if (m_helperAttached && m_helper->isHAPSSupported) {
        hdd_widget = new QWidget(this);
        m_hdd.setupUi(hdd_widget);
        hdd_widget->setContentsMargins(20, 20, 20, 20);
        m_tabWidget->addTab(hdd_widget, i18n("HDD Protection"));
        m_levels << i18n("Off") << i18n("Low") << i18n("Medium") << i18n("High");

        connect( m_hdd.protection_level_slider, SIGNAL( valueChanged(int) ),
		 this, SLOT( protectionLevelChanged(int) ) );
    }

    load();

}

KToshibaSystemSettings::~KToshibaSystemSettings()
{
    delete m_tabWidget; m_tabWidget = NULL;
}

void KToshibaSystemSettings::load()
{
    // General tab
    KConfigGroup sysinfo( m_config, "SystemInformation" );
    m_modelFamily = sysinfo.readEntry("ModelFamily", QString());
    m_sysinfo.model_family->setText(m_modelFamily);
    m_modelNumber = sysinfo.readEntry("ModelNumber", QString());
    m_sysinfo.model_number->setText(m_modelNumber);
    m_biosVersion = sysinfo.readEntry("BIOSVersion", QString());
    m_sysinfo.bios_version->setText(m_biosVersion);
    m_biosDate = sysinfo.readEntry("BIOSDate", QString());
    m_sysinfo.bios_date->setText(m_biosDate);
    m_biosManufacturer = sysinfo.readEntry("BIOSManufacturer", QString());
    m_sysinfo.bios_manufacturer->setText(m_biosManufacturer);
    m_ecVersion = sysinfo.readEntry("ECVersion", QString());
    m_sysinfo.ec_version->setText(m_ecVersion);
    // HDD Protection tab
    if (m_helperAttached && m_helper->isHAPSSupported) {
        KConfigGroup hdd( m_config, "HDDProtection" );
        m_monitorHDD = hdd.readEntry( "MonitorHDD", true );
        m_hdd.hdd_protect_checkbox->setChecked( m_monitorHDD );
        m_notifyHDD = hdd.readEntry( "NotifyHDDMovement", true );
        m_hdd.hdd_notification_checkbox->setChecked( m_notifyHDD );
        m_level = hdd.readEntry( "HDDProtectionLvl", 2 );
        m_hdd.protection_level->setText( m_levels.at( m_level ) );
        m_hdd.protection_level_slider->setValue( m_level );
    }
}

void KToshibaSystemSettings::save()
{
}

void KToshibaSystemSettings::defaults()
{
}

void KToshibaSystemSettings::protectionLevelChanged(int level)
{
    m_hdd.protection_level->setText(m_levels.at(level));

    if (level == m_level)
        return;

    m_level = level;
}


#include "systemsettings.moc"
