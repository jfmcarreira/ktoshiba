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

#include <QLayout>
#include <QDebug>
#include <QTabWidget>
#include <QtDBus/QtDBus>

#include <KPluginFactory>
#include <KAboutData>
#include <KLocalizedString>
#include <KConfigGroup>
#include <KMessageWidget>

#include "systemsettings.h"
#include "helperactions.h"
#include "src/version.h"

#define CONFIG_FILE "ktoshibarc"

K_PLUGIN_FACTORY( KToshibaSystemSettingsFactory, registerPlugin<KToshibaSystemSettings>(); )

KToshibaSystemSettings::KToshibaSystemSettings( QWidget *parent, const QVariantList &args )
    : KCModule( parent, args ),
      m_helper( new HelperActions(this) ),
      m_helperAttached( false ),
      m_config( KSharedConfig::openConfig( CONFIG_FILE ) )
{
    KAboutData *about = new KAboutData("kcm_ktoshibam",
				i18n("KToshiba System Settings"),
				ktoshiba_version,
				QString(),
				KAboutLicense::GPL_V2,
				i18n("Copyright © 2015 Azael Avalos"),
				QString(),
				"http://ktoshiba.sourceforge.net/",
				"coproscefalo@gmail.com");

    about->addAuthor(i18n("Azael Avalos"),
		     i18n("Original author"),
		     "coproscefalo@gmail.com");

    setAboutData(about);

    QGridLayout *layout = new QGridLayout(this);
    QVBoxLayout *message = new QVBoxLayout();
    layout->addLayout(message, 0, 0, 0);

    m_message = new KMessageWidget(this);
    m_message->setVisible(false);
    message->addWidget(m_message);

    m_tabWidget = new QTabWidget(this);
    layout->addWidget(m_tabWidget, 1, 0, 0);

    m_helperAttached = m_helper->init();
    if (m_helperAttached) {
        addTabs();
        m_message->setMessageType(KMessageWidget::Information);
        m_message->setText(i18n("Please reboot for hardware changes to take effect"));
    } else {
        m_message->setMessageType(KMessageWidget::Error);
        m_message->setText(i18n("Could not communicate with helper, hardware changes will not be possible"));
        m_message->setVisible(true);
    }
}

KToshibaSystemSettings::~KToshibaSystemSettings()
{
    delete m_message; m_message = NULL;
    delete m_tabWidget; m_tabWidget = NULL;
}

void KToshibaSystemSettings::addTabs()
{
    QWidget *sysinfo_widget = new QWidget(this);
    m_sysinfo.setupUi(sysinfo_widget);
    sysinfo_widget->setContentsMargins(20, 20, 20, 20);
    m_tabWidget->addTab(sysinfo_widget, i18n("System Information"));

    QWidget *gen_widget = new QWidget(this);
    m_general.setupUi(gen_widget);
    gen_widget->setContentsMargins(20, 20, 20, 20);
    m_tabWidget->addTab(gen_widget, i18n("General Settings"));

    QWidget *hdd_widget = new QWidget(this);
    m_hdd.setupUi(hdd_widget);
    hdd_widget->setContentsMargins(20, 20, 20, 20);
    m_tabWidget->addTab(hdd_widget, i18n("HDD Protection"));
    m_levels << i18n("Off") << i18n("Low") << i18n("Medium") << i18n("High");
    if (!m_helper->isHAPSSupported)
        m_tabWidget->setTabEnabled(2, false);

    QWidget *sleep_widget = new QWidget(this);
    m_sleep.setupUi(sleep_widget);
    sleep_widget->setContentsMargins(20, 20, 20, 20);
    m_tabWidget->addTab(sleep_widget, i18n("Sleep Utilities"));
    if (!m_helper->isUSBSleepChargeSupported && !m_helper->isUSBSleepMusicSupported)
        m_tabWidget->setTabEnabled(3, false);

    QWidget *kbd_widget = new QWidget(this);
    m_kbd.setupUi(kbd_widget);
    kbd_widget->setContentsMargins(20, 20, 20, 20);
    m_tabWidget->addTab(kbd_widget, i18n("Keyboard Settings"));
    m_type1 << "FN-Z" << "AUTO";
    m_type2 << "TIMER" << i18n("ON") << i18n("OFF");
    if (!m_helper->isKBDFunctionsSupported && !m_helper->isKBDBacklightSupported)
        m_tabWidget->setTabEnabled(4, false);
}

void KToshibaSystemSettings::load()
{
    if (!m_helperAttached)
        return;

    qDebug() << "load called";
    // System Information tab
    KConfigGroup sysinfo( m_config, "SystemInformation" );
    if (!sysinfo.exists()) {
        m_helper->getSysInfo();
        sysinfo.writeEntry( "ModelFamily", m_helper->sysinfo[4] );
        sysinfo.writeEntry( "ModelNumber", m_helper->sysinfo[5] );
        sysinfo.writeEntry( "BIOSVersion", m_helper->sysinfo[1] );
        sysinfo.writeEntry( "BIOSDate", m_helper->sysinfo[2] );
        sysinfo.writeEntry( "BIOSManufacturer", m_helper->sysinfo[0] );
        sysinfo.writeEntry( "ECVersion", m_helper->sysinfo[3] );
        sysinfo.config()->sync();
    }
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
    m_sysinfo.driver_version->setText(m_helper->getDriverVersion());
    // General tab
    if (m_helper->isTouchPadSupported) {
        m_touchpad = m_helper->getTouchPad();
        m_general.touchpad_checkbox->setChecked( m_touchpad ? true : false );
        m_general.touchpad_warning->setText(
		i18n("Warning: This option disables the pointing device via hardware.\n"
		     "To disable via software go to: Hardware->Input Devices->Touchpad"));

        connect( m_general.touchpad_checkbox, SIGNAL( stateChanged(int) ),
		 this, SLOT( configChanged() ) );
    } else {
        m_general.touchpad_label->setEnabled(false);
        m_general.touchpad_checkbox->setEnabled(false);
    }
    if (m_helper->isUSBRapidChargeSupported) {
        m_rapidcharge = m_helper->getUSBRapidCharge();
        m_general.rapid_charge_checkbox->setChecked( m_rapidcharge ? true : false );

        connect( m_general.rapid_charge_checkbox, SIGNAL( stateChanged(int) ),
		 this, SLOT( configChangedReboot() ) );
    } else {
        m_general.rapid_charge_label->setEnabled(false);
        m_general.rapid_charge_checkbox->setEnabled(false);
    }
    if (m_helper->isUSBThreeSupported) {
        m_usbthree = m_helper->getUSBThree();
        m_general.usb_three_checkbox->setChecked( m_usbthree ? true : false );

        connect( m_general.usb_three_checkbox, SIGNAL( stateChanged(int) ),
		 this, SLOT( configChangedReboot() ) );
    } else {
        m_general.usb_three_label->setEnabled(false);
        m_general.usb_three_checkbox->setEnabled(false);
    }
    if (m_helper->isPanelPowerONSupported) {
        m_panelpower = m_helper->getPanelPowerON();
        m_general.panel_power_checkbox->setChecked( m_panelpower ? true : false );

        connect( m_general.panel_power_checkbox, SIGNAL( stateChanged(int) ),
		 this, SLOT( configChangedReboot() ) );
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

        connect( m_hdd.hdd_protect_checkbox, SIGNAL( stateChanged(int) ),
		 this, SLOT( configChanged() ) );
        connect( m_hdd.hdd_notification_checkbox, SIGNAL( stateChanged(int) ),
		 this, SLOT( configChanged() ) );
        connect( m_hdd.protection_level_slider, SIGNAL( valueChanged(int) ),
		 this, SLOT( protectionLevelChanged(int) ) );
    }
    // Sleep Utilities tab
    if (m_helper->isUSBSleepChargeSupported) {
        m_sleepcharge = m_helper->getUSBSleepCharge();
        m_sleep.sleep_charge_checkbox->setChecked( m_sleepcharge ? true : false );
        m_sleeponbat = m_helper->getUSBSleepFunctionsBatLvl();
        m_batenabled = m_sleeponbat[0].toInt();
        m_batlevel = m_sleeponbat[1].toInt();
        m_sleep.battery_level_checkbox->setChecked( m_batenabled ? true : false );
        m_sleep.battery_level->setText( QString::number(m_batlevel) + "%" );
        m_sleep.battery_level_slider->setValue( m_batlevel );

        connect( m_sleep.sleep_charge_checkbox, SIGNAL( stateChanged(int) ),
		 this, SLOT( configChanged() ) );
        connect( m_sleep.battery_level_checkbox, SIGNAL( stateChanged(int) ),
		 this, SLOT( configChanged() ) );
        connect( m_sleep.battery_level_slider, SIGNAL( valueChanged(int) ),
		 this, SLOT( batteryLevelChanged(int) ) );
    } else {
        m_sleep.sleep_charge_label->setEnabled(false);
        m_sleep.sleep_charge_checkbox->setEnabled(false);
        m_sleep.battery_level_label->setEnabled(false);
        m_sleep.battery_level_slider->setEnabled(false);
    }
    if (m_helper->isUSBSleepMusicSupported) {
        m_sleepmusic = m_helper->getUSBSleepMusic();
        m_sleep.sleep_music_checkbox->setChecked( m_sleepmusic ? true : false );

        connect( m_sleep.sleep_music_checkbox, SIGNAL( stateChanged(int) ),
		 this, SLOT( configChanged() ) );
    } else {
        m_sleep.sleep_music_label->setEnabled(false);
        m_sleep.sleep_music_checkbox->setEnabled(false);
    }
    // Keyboard Settings tab
    if (m_helper->isKBDFunctionsSupported) {
        m_functions = m_helper->getKBDFunctions();
        m_kbd.kbd_functions_combobox->setCurrentIndex( m_functions );

        connect( m_kbd.kbd_functions_combobox, SIGNAL( currentIndexChanged(int) ),
		 this, SLOT( configChangedReboot() ) );
    } else {
        m_kbd.function_keys_label->setEnabled(false);
        m_kbd.kbd_functions_combobox->setEnabled(false);
    }
    if (m_helper->isKBDBacklightSupported) {
        m_index = 0;
        m_mode = m_helper->getKBDMode();
        m_type = m_helper->getKBDType();
        if (m_type == 1) {
            m_kbd.kbd_backlight_combobox->addItems(m_type1);
            m_index = m_mode == 1 ? 0 : 1;
        } else if (m_type == 2) {
            m_kbd.kbd_backlight_combobox->addItems(m_type2);
            if (m_mode == 2)
                m_index = 0;
            else if (m_mode == 8)
                m_index = 1;
            else if (m_mode == 16)
                m_index = 2;
        }
        m_kbd.kbd_backlight_combobox->setCurrentIndex( m_index );

        if (m_mode == 2) {
            m_time = m_helper->getKBDTimeout();
            m_kbd.kbd_timeout->setText( QString::number(m_time) + i18n(" sec") );
            m_kbd.kbd_timeout_slider->setValue( m_time );
        } else {
            m_kbd.kbd_timeout_label->setEnabled(false);
            m_kbd.kbd_timeout_slider->setEnabled(false);
        }

        connect( m_kbd.kbd_backlight_combobox, SIGNAL( currentIndexChanged(int) ),
		 this, SLOT( configChanged() ) );
        connect( m_kbd.kbd_timeout_slider, SIGNAL( valueChanged(int) ),
		 this, SLOT( kbdTimeoutChanged(int) ) );
    } else {
        m_kbd.kbd_backlight_label->setEnabled(false);
        m_kbd.kbd_backlight_combobox->setEnabled(false);
        m_kbd.kbd_timeout_label->setEnabled(false);
        m_kbd.kbd_timeout_slider->setEnabled(false);
    }
}

void KToshibaSystemSettings::save()
{
    qDebug() << "save called";
    // General tab
    int tmp;
    if (m_helper->isTouchPadSupported) {
        tmp = m_general.touchpad_checkbox->checkState() == Qt::Checked ? 1 : 0;
        if (m_touchpad != tmp) {
            m_helper->setTouchPad(tmp);
            m_touchpad = tmp;
        }
    }
    if (m_helper->isUSBRapidChargeSupported) {
        tmp = m_general.rapid_charge_checkbox->checkState() == Qt::Checked ? 1 : 0;
        if (m_rapidcharge != tmp) {
            m_helper->setUSBRapidCharge(tmp);
            m_rapidcharge = tmp;
        }
    }
    if (m_helper->isUSBThreeSupported) {
        tmp = m_general.usb_three_checkbox->checkState() == Qt::Checked ? 1 : 0;
        if (m_usbthree != tmp) {
            m_helper->setUSBThree(tmp);
            m_usbthree = tmp;
        }
    }
    if (m_helper->isPanelPowerONSupported) {
        tmp = m_general.panel_power_checkbox->checkState() == Qt::Checked ? 1 : 0;
        if (m_panelpower != tmp) {
            m_helper->setPanelPowerON(tmp);
            m_panelpower = tmp;
        }
    }
    // HDD Protection tab
    if (m_helper->isHAPSSupported) {
        KConfigGroup hddGroup( m_config, "HDDProtection" );
        QDBusInterface iface("net.sourceforge.KToshiba",
			     "/net/sourceforge/KToshiba",
			     "net.sourceforge.KToshiba",
			     QDBusConnection::sessionBus(), this);
        bool tmp2 = m_hdd.hdd_protect_checkbox->checkState() == Qt::Checked ? true : false;
        if (m_monitorHDD != tmp2) {
            hddGroup.writeEntry( "MonitorHDD", tmp2 );
            hddGroup.config()->sync();
            m_monitorHDD = tmp2;
            iface.call("configFileChanged");
        }
        tmp2 = m_hdd.hdd_notification_checkbox->checkState() == Qt::Checked ? true : false;
        if (m_notifyHDD != tmp2) {
            hddGroup.writeEntry( "NotifyHDDMovement", tmp2 );
            hddGroup.config()->sync();
            m_notifyHDD = tmp2;
            iface.call("configFileChanged");
        }
        tmp = m_hdd.protection_level_slider->value();
        if (m_level != tmp) {
            m_helper->setProtectionLevel(tmp);
            m_level = tmp;
        }
    }
    // Sleep Utilities tab
    if (m_helper->isUSBSleepChargeSupported) {
        tmp = m_sleep.sleep_charge_checkbox->checkState() == Qt::Checked ? 1 : 0;
        if (m_sleepcharge != tmp) {
            m_helper->setUSBSleepCharge(tmp);
            m_sleepcharge = tmp;
        }
        tmp = m_sleep.battery_level_slider->value();
        if (m_batlevel != tmp) {
            m_helper->setUSBSleepFunctionsBatLvl(tmp);
            m_batlevel = tmp;
        }
        tmp = m_sleep.battery_level_checkbox->checkState() == Qt::Checked ? 1 : 0;
        if (m_batenabled != tmp) {
            m_helper->setUSBSleepFunctionsBatLvl(tmp);
            m_batenabled = tmp;
        }
    }
    if (m_helper->isUSBSleepMusicSupported) {
        tmp = m_sleep.sleep_music_checkbox->checkState() == Qt::Checked ? 1 : 0;
        if (m_sleepmusic != tmp) {
            m_helper->setUSBSleepMusic(tmp);
            m_sleepmusic = tmp;
        }
    }
    // Keyboard Settings tab
    if (m_helper->isKBDFunctionsSupported) {
        tmp = m_kbd.kbd_functions_combobox->currentIndex();
        if (m_functions != tmp) {
            m_helper->setKBDFunctions(tmp);
            m_functions = tmp;
        }
    }
    if (m_helper->isKBDBacklightSupported) {
        tmp = m_kbd.kbd_backlight_combobox->currentIndex();
        if (m_index != tmp) {
            if (m_type == 1 && tmp == 0) {
                m_helper->setKBDMode(1);	// FN-Z
            } else if (m_type == 1 && tmp == 1) {
                m_helper->setKBDMode(2);	// AUTO
            } else if (m_type == 2 && tmp == 0) {
                m_helper->setKBDMode(2);	// TIMER
                m_time = m_helper->getKBDTimeout();
                m_kbd.kbd_timeout_label->setEnabled( true );
                m_kbd.kbd_timeout_slider->setEnabled( true );
                m_kbd.kbd_timeout->setText( QString::number(m_time) + i18n(" sec") );
                m_kbd.kbd_timeout_slider->setValue( m_time );
            } else if (m_type == 2 && tmp == 1) {
                m_helper->setKBDMode(8);	// ON
                m_kbd.kbd_timeout_label->setEnabled( false );
                m_kbd.kbd_timeout_slider->setEnabled( false );
                m_kbd.kbd_timeout->setEnabled( false );
            } else if (m_type == 2 && tmp == 2) {
                m_helper->setKBDMode(16);	// OFF
                m_kbd.kbd_timeout_label->setEnabled( false );
                m_kbd.kbd_timeout_slider->setEnabled( false );
                m_kbd.kbd_timeout->setEnabled( false );
            }
            m_index = tmp;
        }
        if (m_type == 2 && m_mode == 2) {
            tmp = m_kbd.kbd_timeout_slider->value();
            if (m_time != tmp) {
                m_helper->setKBDTimeout(tmp);
                m_time = tmp;
            }
        }
    }
}

void KToshibaSystemSettings::defaults()
{
    if (!m_helperAttached)
        return;

    qDebug() << "defaults called";
    // General tab
    if (m_helper->isTouchPadSupported) {
        if (!m_touchpad)
            m_general.touchpad_checkbox->setChecked( true );
    }
    if (m_helper->isUSBRapidChargeSupported) {
        if (m_rapidcharge) {
            m_general.rapid_charge_checkbox->setChecked( false );
            showRebootMessage();
        }
    }
    if (m_helper->isUSBThreeSupported) {
        if (!m_usbthree) {
            m_general.usb_three_checkbox->setChecked( true );
            showRebootMessage();
        }
    }
    if (m_helper->isPanelPowerONSupported) {
        if (m_panelpower) {
            m_general.panel_power_checkbox->setChecked( false );
            showRebootMessage();
        }
    }
    // HDD Protection tab
    if (m_helper->isHAPSSupported) {
        if (!m_monitorHDD || !m_notifyHDD) {
            m_hdd.hdd_protect_checkbox->setChecked( true );
            m_hdd.hdd_notification_checkbox->setChecked( true );
            m_hdd.protection_level->setText( m_levels.at( 2 ) );
            m_hdd.protection_level_slider->setValue( 2 );
        }
    }
    // Sleep Utilities tab
    if (m_helper->isUSBSleepChargeSupported) {
        if (m_sleepcharge)
            m_sleep.sleep_charge_checkbox->setChecked( false );
        if (!m_batenabled || m_batlevel != 10) {
            m_sleep.battery_level_checkbox->setChecked( true );
            m_sleep.battery_level->setText( QString::number(10) + "%" );
            m_sleep.battery_level_slider->setValue( 10 );
        }
    }
    if (m_helper->isUSBSleepMusicSupported) {
        if (m_sleepmusic)
            m_sleep.sleep_music_checkbox->setChecked( false );
    }
    // Keyboard Settings tab
    if (m_helper->isKBDFunctionsSupported) {
        if (!m_functions)
            m_kbd.kbd_functions_combobox->setCurrentIndex( 1 );
    }
    if (m_helper->isKBDBacklightSupported) {
        if (m_mode != 2) {
            if (m_type == 1) {
                m_kbd.kbd_backlight_combobox->setCurrentIndex( 2 );
                showRebootMessage();
            } else if (m_type == 2) {
                m_kbd.kbd_backlight_combobox->setCurrentIndex( 0 );
            }
        }
        if (m_time != 15 && m_mode == 2)
            m_kbd.kbd_timeout_slider->setValue(15);
    }

    emit changed(true);
}

void KToshibaSystemSettings::showRebootMessage()
{
    if (!m_message->isVisible())
        m_message->setVisible(true);
}

void KToshibaSystemSettings::configChanged()
{
    emit changed(true);
}

void KToshibaSystemSettings::configChangedReboot()
{
    showRebootMessage();

    emit changed(true);
}

void KToshibaSystemSettings::protectionLevelChanged(int level)
{
    m_hdd.protection_level->setText(m_levels.at(level));

    emit changed(true);
}

void KToshibaSystemSettings::batteryLevelChanged(int level)
{
    m_sleep.battery_level->setText(QString::number(level) + "%");

    emit changed(true);
}

void KToshibaSystemSettings::kbdTimeoutChanged(int time)
{
    m_kbd.kbd_timeout->setText(QString::number(time) + i18n(" sec"));

    emit changed(true);
}


#include "systemsettings.moc"
