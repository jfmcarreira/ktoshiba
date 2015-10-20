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
#include <QIcon>
#include <QComboBox>

#include <KPluginFactory>
#include <KAboutData>
#include <KLocalizedString>
#include <KConfigGroup>
#include <KMessageWidget>

#include "generalsettings.h"
#include "hddprotection.h"
#include "sleeputilities.h"
#include "keyboardsettings.h"
#include "bootsettings.h"
#include "powersave.h"
#include "systemsettings.h"
#include "ktoshibahardware.h"
#include "src/version.h"

#define CONFIG_FILE "ktoshibarc"

K_PLUGIN_FACTORY(KToshibaSystemSettingsFactory, registerPlugin<KToshibaSystemSettings>();)

KToshibaSystemSettings::KToshibaSystemSettings(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args),
      m_hw(new KToshibaHardware(this)),
      m_hwAttached(false),
      m_config(KSharedConfig::openConfig(CONFIG_FILE))
{
    KAboutData *about = new KAboutData(QLatin1String("kcm_ktoshibam"),
                                       i18n("KToshiba System Settings"),
                                       ktoshiba_version,
                                       i18n("Configures Toshiba laptop related hardware settings"),
                                       KAboutLicense::GPL_V2,
                                       i18n("Copyright (C) 2015 Azael Avalos"),
                                       QString(),
                                       QLatin1String("http://ktoshiba.sourceforge.net/"),
                                       QLatin1String("coproscefalo@gmail.com"));

    about->addAuthor(i18n("Azael Avalos"),
                     i18n("Original author"),
                     QLatin1String("coproscefalo@gmail.com"));

    setAboutData(about);
    setButtons(Apply | Default);

    QGridLayout *layout = new QGridLayout(this);
    QVBoxLayout *message = new QVBoxLayout();
    layout->addLayout(message, 0, 0, 0);

    m_message = new KMessageWidget(this);
    m_message->setVisible(false);
    m_message->setCloseButtonVisible(false);
    message->addWidget(m_message);

    m_tabWidget = new QTabWidget(this);
    layout->addWidget(m_tabWidget, 1, 0, 0);

    m_hwAttached = m_hw->init();
    if (m_hwAttached) {
        addTabs();
        m_message->setMessageType(KMessageWidget::Information);
        m_message->setText(i18n("Please reboot for hardware changes to take effect"));
        m_message->setIcon(QIcon::fromTheme("dialog-information"));
    } else {
        m_message->setMessageType(KMessageWidget::Error);
        m_message->setText(i18n("Could not communicate with library, hardware changes will not be possible"));
        m_message->setIcon(QIcon::fromTheme("dialog-error"));
        m_message->setVisible(true);
    }
}

KToshibaSystemSettings::~KToshibaSystemSettings()
{
    delete m_boot; m_boot = NULL;
    delete m_message; m_message = NULL;
    delete m_tabWidget; m_tabWidget = NULL;
}

void KToshibaSystemSettings::addTabs()
{
    QWidget *sysinfo_widget = new QWidget(this);
    m_sysinfo.setupUi(sysinfo_widget);
    sysinfo_widget->setContentsMargins(20, 20, 20, 20);
    m_tabWidget->addTab(sysinfo_widget, i18n("System Information"));

    m_general = new GeneralSettings(this);
    m_general->setContentsMargins(20, 20, 20, 20);
    m_tabWidget->addTab(m_general, i18n("General Settings"));

    m_hdd = new HDDProtection(this);
    m_hdd->setContentsMargins(20, 20, 20, 20);
    m_tabWidget->addTab(m_hdd, i18n("HDD Protection"));

    m_sleep = new SleepUtilities(this);
    m_sleep->setContentsMargins(20, 20, 20, 20);
    m_tabWidget->addTab(m_sleep, i18n("Sleep Utilities"));

    m_kbd = new KeyboardSettings(this);
    m_kbd->setContentsMargins(20, 20, 20, 20);
    m_tabWidget->addTab(m_kbd, i18n("Keyboard Settings"));

    m_boot = new BootSettings(this);
    m_boot->setContentsMargins(20, 20, 20, 20);
    m_tabWidget->addTab(m_boot, i18n("Boot Settings"));

    m_power = new PowerSave(this);
    m_power->setContentsMargins(20, 20, 20, 20);
    m_tabWidget->addTab(m_power, i18n("Power Save"));
}

void KToshibaSystemSettings::load()
{
    if (!m_hwAttached)
        return;

    /*
     * System Information tab
     */
    KConfigGroup sysinfo(m_config, "SystemInformation");
    if (!sysinfo.exists() && m_hw->getSysInfo()) {
        sysinfo.writeEntry("ModelFamily", m_hw->modelFamily);
        sysinfo.writeEntry("ModelNumber", m_hw->modelNumber);
        sysinfo.writeEntry("BIOSVersion", m_hw->biosVersion);
        sysinfo.writeEntry("BIOSDate", m_hw->biosDate);
        sysinfo.writeEntry("BIOSManufacturer", m_hw->biosManufacturer);
        sysinfo.writeEntry("ECVersion", m_hw->ecVersion);
        sysinfo.sync();
    }
    m_modelFamily = sysinfo.readEntry("ModelFamily", i18n("Unknown"));
    m_sysinfo.model_family->setText(m_modelFamily);
    m_modelNumber = sysinfo.readEntry("ModelNumber", i18n("Unknown"));
    m_sysinfo.model_number->setText(m_modelNumber);
    m_biosVersion = sysinfo.readEntry("BIOSVersion", i18n("Unknown"));
    m_sysinfo.bios_version->setText(m_biosVersion);
    m_biosDate = sysinfo.readEntry("BIOSDate", i18n("Unknown"));
    m_sysinfo.bios_date->setText(m_biosDate);
    m_biosManufacturer = sysinfo.readEntry("BIOSManufacturer", i18n("Unknown"));
    m_sysinfo.bios_manufacturer->setText(m_biosManufacturer);
    m_ecVersion = sysinfo.readEntry("ECVersion", i18n("Unknown"));
    m_sysinfo.ec_version->setText(m_ecVersion);
    m_driverVersion = m_hw->getDriverVersion();
    m_sysinfo.driver_version->setText(m_driverVersion);
    /*
     * General tab
     */
    m_general->load();

    connect(m_general->touchpad_checkbox, SIGNAL(stateChanged(int)),
            this, SLOT(configChanged()));
    connect(m_hw, SIGNAL(touchpadToggled(int)), this, SLOT(updateTouchPad(int)));
    connect(m_general->rapid_charge_checkbox, SIGNAL(stateChanged(int)),
            this, SLOT(configChangedReboot()));
    connect(m_general->usb_three_checkbox, SIGNAL(stateChanged(int)),
            this, SLOT(configChangedReboot()));
    /*
     * HDD Protection tab
     */
    m_hdd->load();

    connect(m_hdd->groupBox, SIGNAL(toggled(bool)),
            this, SLOT(configChanged()));
    connect(m_hdd->hdd_notification_checkbox, SIGNAL(stateChanged(int)),
            this, SLOT(configChanged()));
    connect(m_hdd->protection_level_slider, SIGNAL(valueChanged(int)),
            this, SLOT(protectionLevelChanged(int)));
    /*
     * Sleep Utilities tab
     */
    m_sleep->load();

    connect(m_sleep->sleep_charge_combobox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(configChanged()));
    connect(m_sleep->groupBox, SIGNAL(toggled(bool)),
            this, SLOT(configChanged()));
    connect(m_sleep->battery_level_slider, SIGNAL(valueChanged(int)),
            this, SLOT(batteryLevelChanged(int)));
    connect(m_sleep->sleep_music_checkbox, SIGNAL(stateChanged(int)),
            this, SLOT(configChanged()));
    /*
     * Keyboard Settings tab
     */
    m_kbd->load();

    connect(m_kbd->kbd_timeout_slider, SIGNAL(valueChanged(int)),
            this, SLOT(kbdTimeoutChanged(int)));
    connect(m_kbd->kbd_backlight_combobox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(kbdBacklightChanged(int)));
    if (m_kbd->m_keyboardType == KeyboardSettings::FirstKeyboardGen)
        connect(m_kbd->kbd_backlight_combobox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(configChangedReboot()));
    /*
     * Boot Settings tab
     */
    m_boot->load();

    connect(m_boot, SIGNAL(changed()), this, SLOT(changed()));
    connect(m_boot->panel_power_checkbox, SIGNAL(stateChanged(int)),
            this, SLOT(configChangedReboot()));
    connect(m_boot->wok_checkbox, SIGNAL(stateChanged(int)),
            this, SLOT(configChangedReboot()));
    connect(m_boot->wol_checkbox, SIGNAL(stateChanged(int)),
            this, SLOT(configChangedReboot()));
    /*
     * Power Save tab
     */
    m_power->load();

    connect(m_power->batteryGroupBox, SIGNAL(toggled(bool)),
            this, SLOT(configChanged()));
    connect(m_power->battery_profiles_combobox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(configChanged()));
    connect(m_power->coolingGroupBox, SIGNAL(toggled(bool)),
            this, SLOT(configChanged()));
    connect(m_power->cooling_method_battery_combobox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(configChanged()));
    connect(m_power->cooling_method_plugged_combobox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(configChanged()));
}

void KToshibaSystemSettings::save()
{
    /*
     * General tab
     */
    m_general->save();
    /*
     * HDD Protection tab
     */
    m_hdd->save();
    /*
     * Sleep Utilities tab
     */
    m_sleep->save();
    /*
     * Keyboard Settings tab
     */
    m_kbd->save();
    /*
     * Boot Settings
     */
    m_boot->save();
    /*
     * Power Save
     */
    m_power->save();
}

void KToshibaSystemSettings::defaults()
{
    if (!m_hwAttached)
        return;

    /*
     * General tab
     */
    m_general->defaults();
    /*
     * HDD Protection tab
     */
    m_hdd->defaults();
    /*
     * Sleep Utilities tab
     */
    m_sleep->defaults();
    /*
     * Keyboard Settings tab
     */
    m_kbd->defaults();
    /*
     * Boot Settings
     */
    m_boot->defaults();
    /*
     * Power Save
     */
    m_power->defaults();
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
    m_hdd->protection_level->setText(m_hdd->m_levels.at(level));

    emit changed(true);
}

void KToshibaSystemSettings::batteryLevelChanged(int level)
{
    m_sleep->battery_level->setText(QString::number(level) + "%");

    emit changed(true);
}

void KToshibaSystemSettings::kbdBacklightChanged(int index)
{
    if (m_kbd->m_keyboardType == KeyboardSettings::FirstKeyboardGen) {
        m_kbd->kbd_timeout_label->setEnabled(index != KeyboardSettings::AUTO ? false : true);
        m_kbd->kbd_timeout->setEnabled(index != KeyboardSettings::AUTO ? false : true);
        m_kbd->kbd_timeout_slider->setEnabled(index != KeyboardSettings::AUTO ? false : true);

        showRebootMessage();
    } else if (m_kbd->m_keyboardType == KeyboardSettings::SecondKeyboardGen) {
        m_kbd->kbd_timeout_label->setEnabled(index != KeyboardSettings::TIMER ? false : true);
        m_kbd->kbd_timeout->setEnabled(index != KeyboardSettings::TIMER ? false : true);
        m_kbd->kbd_timeout_slider->setEnabled(index != KeyboardSettings::TIMER ? false : true);
    }

    emit changed(true);
}

void KToshibaSystemSettings::kbdTimeoutChanged(int time)
{
    m_kbd->kbd_timeout->setText(QString::number(time) + i18n(" sec"));

    emit changed(true);
}

void KToshibaSystemSettings::updateTouchPad(int state)
{
    m_general->touchpad_checkbox->setChecked(state == Qt::Checked ? true : false);
}


#include "systemsettings.moc"
