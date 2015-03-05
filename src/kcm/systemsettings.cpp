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
      m_helperAttached( false ),
      m_config( KSharedConfig::openConfig( CONFIG_FILE ) )
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

    QWidget *gen_widget = new QWidget(this);
    m_general.setupUi(gen_widget);
    gen_widget->setContentsMargins(20, 20, 20, 20);
    m_tabWidget->addTab(gen_widget, i18n("General Settings"));

    m_helperAttached = m_helper->init();
    if (m_helperAttached)
        addTabs();
}

KToshibaSystemSettings::~KToshibaSystemSettings()
{
    delete m_tabWidget; m_tabWidget = NULL;
}

void KToshibaSystemSettings::addTabs()
{
    if (m_helper->isHAPSSupported) {
        QWidget *hdd_widget = new QWidget(this);
        m_hdd.setupUi(hdd_widget);
        hdd_widget->setContentsMargins(20, 20, 20, 20);
        m_tabWidget->addTab(hdd_widget, i18n("HDD Protection"));
        m_levels << i18n("Off") << i18n("Low") << i18n("Medium") << i18n("High");

        connect( m_hdd.protection_level_slider, SIGNAL( valueChanged(int) ),
                 this, SLOT( protectionLevelChanged(int) ) );
    }
    if (m_helper->isUSBSleepChargeSupported || m_helper->isUSBSleepMusicSupported) {
        QWidget *sleep_widget = new QWidget(this);
        m_sleep.setupUi(sleep_widget);
        sleep_widget->setContentsMargins(20, 20, 20, 20);
        m_tabWidget->addTab(sleep_widget, i18n("Sleep Utilities"));

        connect( m_sleep.battery_level_slider, SIGNAL( valueChanged(int) ),
                 this, SLOT( batteryLevelChanged(int) ) );
    }
    if (m_helper->isKBDFunctionsSupported || m_helper->isKBDBacklightSupported) {
        QWidget *kbd_widget = new QWidget(this);
        m_kbd.setupUi(kbd_widget);
        kbd_widget->setContentsMargins(20, 20, 20, 20);
        m_tabWidget->addTab(kbd_widget, i18n("Keyboard Settings"));
        m_type1 << "FN-Z" << "AUTO";
        m_type2 << "TIMER" << i18n("ON") << i18n("OFF");

        connect( m_kbd.kbd_functions_combobox, SIGNAL( currentIndexChanged(int) ),
                 this, SLOT( kbdFunctionsChanged(int) ) );
        connect( m_kbd.kbd_backlight_combobox, SIGNAL( currentIndexChanged(int) ),
                 this, SLOT( kbdBacklightChanged(int) ) );
        connect( m_kbd.kbd_timeout_slider, SIGNAL( valueChanged(int) ),
                 this, SLOT( kbdTimeoutChanged(int) ) );
    }
}

void KToshibaSystemSettings::load()
{
    // System Information tab
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
    // General tab
    if (m_helper->isTouchPadSupported) {
        m_touchpad = m_helper->getTouchPad();
        m_general.touchpad_checkbox->setChecked( m_touchpad ? true : false );
        m_general.touchpad_warning->setText(
		i18n("Warning: This option disables the pointing device via hardware.\n"
		     "To disable via software go to: Hardware->Input Devices->Touchpad"));
    } else {
        m_general.touchpad_label->setEnabled(false);
        m_general.touchpad_checkbox->setEnabled(false);
    }
    if (m_helper->isUSBRapidChargeSupported) {
        m_rapidcharge = m_helper->getUSBRapidCharge();
        m_general.rapid_charge_checkbox->setChecked( m_rapidcharge ? true : false );
    } else {
        m_general.rapid_charge_label->setEnabled(false);
        m_general.rapid_charge_checkbox->setEnabled(false);
    }
    if (m_helper->isUSBThreeSupported) {
        m_usbthree = m_helper->getUSBThree();
        m_general.usb_three_checkbox->setChecked( m_usbthree ? true : false );
    } else {
        m_general.usb_three_label->setEnabled(false);
        m_general.usb_three_checkbox->setEnabled(false);
    }
    if (m_helper->isPanelPowerONSupported) {
        m_panelpower = m_helper->getPanelPowerON();
        m_general.panel_power_checkbox->setChecked( m_panelpower ? true : false );
    } else {
        m_general.panel_power_label->setEnabled(false);
        m_general.panel_power_checkbox->setEnabled(false);
    }
    // HDD Protection tab
    if (m_helper->isHAPSSupported) {
        KConfigGroup hdd( m_config, "HDDProtection" );
        m_monitorHDD = hdd.readEntry( "MonitorHDD", true );
        m_hdd.hdd_protect_checkbox->setChecked( m_monitorHDD );
        m_notifyHDD = hdd.readEntry( "NotifyHDDMovement", true );
        m_hdd.hdd_notification_checkbox->setChecked( m_notifyHDD );
        m_level = m_helper->getProtectionLevel();
        m_hdd.protection_level->setText( m_levels.at( m_level ) );
        m_hdd.protection_level_slider->setValue( m_level );
    }
    // Sleep Utilities tab
    if (m_helper->isUSBSleepChargeSupported) {
        m_sleepcharge = m_helper->getUSBSleepCharge();
        m_sleep.sleep_charge_checkbox->setChecked( m_sleepcharge ? true : false );
        m_sleeponbat = m_helper->getUSBSleepFunctionsBatLvl();
        m_batenabled = m_sleeponbat[1].toInt();
        m_batlevel = m_sleeponbat[1].toInt();
        m_sleep.battery_level_checkbox->setChecked( m_batenabled ? true : false );
        m_sleep.battery_level->setText( QString::number(m_batlevel) + "%" );
        m_sleep.battery_level_slider->setValue( m_batlevel );
    } else {
        m_sleep.sleep_charge_label->setEnabled(false);
        m_sleep.sleep_charge_checkbox->setEnabled(false);
        m_sleep.battery_level_label->setEnabled(false);
        m_sleep.battery_level_slider->setEnabled(false);
    }
    if (m_helper->isUSBSleepMusicSupported) {
        m_sleepmusic = m_helper->getUSBSleepMusic();
        m_sleep.sleep_music_checkbox->setChecked( m_sleepmusic ? true : false );
    } else {
        m_sleep.sleep_music_label->setEnabled(false);
        m_sleep.sleep_music_checkbox->setEnabled(false);
    }
    // Keyboard Settings tab
    if (m_helper->isKBDFunctionsSupported) {
        m_functions = m_helper->getKBDFunctions();
        m_kbd.kbd_functions_combobox->setCurrentIndex( m_functions );
    } else {
        m_kbd.function_keys_label->setEnabled(false);
        m_kbd.kbd_functions_combobox->setEnabled(false);
    }
    if (m_helper->isKBDBacklightSupported) {
        m_type = m_helper->getKBDType();
        if (m_type == 1)
            m_kbd.kbd_backlight_combobox->addItems(m_type1);
        else if (m_type == 2)
            m_kbd.kbd_backlight_combobox->addItems(m_type2);

        m_mode = m_helper->getKBDMode();
        int mode = 0;
        if (m_mode == 1)
            mode = 0;
        else if (m_mode == 2 && m_type == 1)
            mode = 1;
        else if (m_mode == 2 && m_type == 2)
            mode = 0;
        else if (m_mode == 8)
            mode = 1;
        else if (m_mode == 16)
            mode = 2;
        m_kbd.kbd_backlight_combobox->setCurrentIndex( mode );
        if (m_mode == 2) {
            m_time = m_helper->getKBDTimeout();
            m_kbd.kbd_timeout->setText( QString::number(m_time) + i18n(" sec") );
            m_kbd.kbd_timeout_slider->setValue( m_time );
        } else {
            m_kbd.kbd_timeout_label->setEnabled(false);
            m_kbd.kbd_timeout_slider->setEnabled(false);
        }
    } else {
        m_kbd.function_keys_label->setEnabled(false);
        m_kbd.kbd_backlight_combobox->setEnabled(false);
        m_kbd.kbd_timeout_label->setEnabled(false);
        m_kbd.kbd_timeout_slider->setEnabled(false);
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

void KToshibaSystemSettings::batteryLevelChanged(int level)
{
    m_sleep.battery_level->setText(QString::number(level) + "%");

    if (level == m_batlevel)
        return;

    m_batlevel = level;
}

void KToshibaSystemSettings::kbdTimeoutChanged(int time)
{
    m_kbd.kbd_timeout->setText(QString::number(time) + i18n(" sec"));

    if (time == m_time)
        return;

    m_time = time;
}

void KToshibaSystemSettings::kbdFunctionsChanged(int mode)
{
    if (mode == m_helper->getKBDFunctions())
        return;

    m_functions = mode;
}

void KToshibaSystemSettings::kbdBacklightChanged(int mode)
{
    if (mode == m_helper->getKBDTimeout())
        return;

    m_mode = mode;
}


#include "systemsettings.moc"
