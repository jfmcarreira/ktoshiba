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

#include <QComboBox>
#include <QDBusInterface>
#include <QIcon>
#include <QLayout>
#include <QStringBuilder>
#include <QTabWidget>

#include <KAboutData>
#include <KConfigGroup>
#include <KMessageWidget>
#include <KLocalizedString>
#include <KPluginFactory>

#include <ktoshibahardware.h>
#include <ktoshiba_version.h>

#include "bootsettings.h"
#include "generalsettings.h"
#include "hddprotection.h"
#include "keyboardsettings.h"
#include "powersave.h"
#include "sleeputilities.h"
#include "systeminformation.h"
#include "systemsettings.h"

K_PLUGIN_FACTORY(KToshibaSystemSettingsFactory, registerPlugin<KToshibaSystemSettings>();)

KToshibaSystemSettings::KToshibaSystemSettings(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args),
      m_hw(new KToshibaHardware(this)),
      m_config(KSharedConfig::openConfig(QStringLiteral("ktoshibarc"))),
      m_configFileChanged(false)
{
    KAboutData *about = new KAboutData(QStringLiteral("kcm_ktoshibam"),
                                       i18n("KToshiba System Settings"),
                                       QStringLiteral(KTOSHIBA_VERSION_STRING),
                                       i18n("Configures Toshiba laptop related hardware settings"),
                                       KAboutLicense::GPL_V2,
                                       QStringLiteral("(C) 2015-2016 Azael Avalos"));
    about->setHomepage(QStringLiteral("http://ktoshiba.sourceforge.net/"));
    about->setBugAddress("https://sourceforge.net/p/ktoshiba/support-requests/");
    about->addAuthor(i18n("Azael Avalos"),
                     i18n("Original author"),
                     QStringLiteral("coproscefalo@gmail.com"));

    setAboutData(about);

    QGridLayout *layout = new QGridLayout(this);
    QVBoxLayout *message = new QVBoxLayout();
    layout->addLayout(message, 0, 0, 0);

    m_message = new KMessageWidget(this);
    m_message->setCloseButtonVisible(false);
    m_message->setMessageType(KMessageWidget::Information);
    m_message->setText(i18n("Please reboot for hardware changes to take effect"));
    m_message->setIcon(QIcon::fromTheme(QStringLiteral("dialog-information")));
    m_message->setVisible(false);
    message->addWidget(m_message);

    m_tabWidget = new QTabWidget(this);
    layout->addWidget(m_tabWidget, 1, 0, 0);

    addTabs();

    connect(m_general, SIGNAL(configFileChanged()), this, SLOT(flagConfigFileChanged()));
    connect(m_hdd, SIGNAL(configFileChanged()), this, SLOT(flagConfigFileChanged()));
    connect(m_power, SIGNAL(configFileChanged()), this, SLOT(flagConfigFileChanged()));
}

KToshibaSystemSettings::~KToshibaSystemSettings()
{
    delete m_message; m_message = NULL;
    delete m_tabWidget; m_tabWidget = NULL;
}

void KToshibaSystemSettings::addTabs()
{
    m_sysinfo = new SystemInformation(this);
    m_sysinfo->setContentsMargins(10, 10, 10, 10);
    m_tabWidget->addTab(m_sysinfo, m_sysinfo->windowTitle());

    m_general = new GeneralSettings(this);
    m_general->setContentsMargins(10, 10, 10, 10);
    m_tabWidget->addTab(m_general, m_general->windowTitle());

    m_hdd = new HDDProtection(this);
    m_hdd->setContentsMargins(10, 10, 10, 10);
    m_tabWidget->addTab(m_hdd, m_hdd->windowTitle());

    m_sleep = new SleepUtilities(this);
    m_sleep->setContentsMargins(10, 10, 10, 10);
    m_tabWidget->addTab(m_sleep, m_sleep->windowTitle());

    m_kbd = new KeyboardSettings(this);
    m_kbd->setContentsMargins(10, 10, 10, 10);
    m_tabWidget->addTab(m_kbd, m_kbd->windowTitle());

    m_boot = new BootSettings(this);
    m_boot->setContentsMargins(10, 10, 10, 10);
    m_tabWidget->addTab(m_boot, m_boot->windowTitle());

    m_power = new PowerSave(this);
    m_power->setContentsMargins(10, 10, 10, 10);
    m_tabWidget->addTab(m_power, m_power->windowTitle());
}

void KToshibaSystemSettings::load()
{
    /*
     * System Information tab
     */
    m_sysinfo->load();
    /*
     * General tab
     */
    m_general->load();

    connect(m_general->pointing_device_checkbox, SIGNAL(stateChanged(int)),
            this, SLOT(configChanged()));
    connect(m_hw, SIGNAL(touchpadToggled(int)), this, SLOT(updateTouchPad(int)));
    connect(m_general->built_in_lan_checkbox, SIGNAL(stateChanged(int)),
            this, SLOT(configChangedReboot()));
    connect(m_general->rapid_charge_checkbox, SIGNAL(stateChanged(int)),
            this, SLOT(configChangedReboot()));
    connect(m_general->usb_three_checkbox, SIGNAL(stateChanged(int)),
            this, SLOT(configChangedReboot()));
    connect(m_general->usb_legacy_checkbox, SIGNAL(stateChanged(int)),
            this, SLOT(configChangedReboot()));
    connect(m_general->power_on_display_combobox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(configChanged()));
    connect(m_general->hdmiCECGroupBox, SIGNAL(toggled(bool)),
            this, SLOT(configChanged()));
    connect(m_general->hdmi_remote_power_checkbox, SIGNAL(stateChanged(int)),
            this, SLOT(configChanged()));
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

    connect(m_kbd->kbd_functions_combobox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(configChangedReboot()));
    connect(m_kbd->kbd_timeout_slider, SIGNAL(valueChanged(int)),
            this, SLOT(kbdTimeoutChanged(int)));
    connect(m_kbd->kbd_backlight_combobox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(kbdBacklightChanged(int)));
    if (m_kbd->getKeyboardType() == KeyboardSettings::FirstKeyboardGen) {
        connect(m_kbd->kbd_backlight_combobox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(configChangedReboot()));
    }
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
    connect(m_boot->boot_speed_combobox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(configChangedReboot()));
    /*
     * Power Save tab
     */
    m_power->load();

    connect(m_power->battery_profiles_combobox, SIGNAL(currentIndexChanged(int)),
            m_power, SLOT(loadProfile(int)));
    connect(m_power->battery_profiles_combobox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(configChanged()));
    connect(m_power->cooling_method_combobox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(configChanged()));
    connect(m_power->odd_power_combobox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(configChanged()));
    connect(m_power->illumination_combobox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(configChanged()));
    connect(m_power->sata_iface_combobox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(configChangedReboot()));
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

    if (m_configFileChanged) {
        notifyConfigFileChanged();
    }
}

void KToshibaSystemSettings::defaults()
{
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
    if (!m_message->isVisible()) {
        m_message->setVisible(true);
    }
}

void KToshibaSystemSettings::configChanged()
{
    Q_EMIT changed(true);
}

void KToshibaSystemSettings::configChangedReboot()
{
    showRebootMessage();

    Q_EMIT changed(true);
}

void KToshibaSystemSettings::flagConfigFileChanged()
{
    m_configFileChanged = true;
}

void KToshibaSystemSettings::notifyConfigFileChanged()
{
    QDBusInterface iface(QStringLiteral("net.sourceforge.KToshiba"),
                         QStringLiteral("/Config"),
                         QStringLiteral("net.sourceforge.KToshiba"),
                         QDBusConnection::sessionBus(), this);

    if (iface.isValid()) {
        iface.call(QStringLiteral("reloadConfigFile"));
    }
}

void KToshibaSystemSettings::protectionLevelChanged(int level)
{
    m_hdd->protection_level->setText(m_hdd->getProtectionLevels().at(level));

    Q_EMIT changed(true);
}

void KToshibaSystemSettings::batteryLevelChanged(int level)
{
    m_sleep->battery_level->setText(QString::number(level) % QStringLiteral("%"));

    Q_EMIT changed(true);
}

void KToshibaSystemSettings::kbdBacklightChanged(int index)
{
    if (m_kbd->getKeyboardType() == KeyboardSettings::FirstKeyboardGen) {
        m_kbd->kbd_timeout_label->setEnabled(index != KeyboardSettings::AUTO ? false : true);
        m_kbd->kbd_timeout->setEnabled(index != KeyboardSettings::AUTO ? false : true);
        m_kbd->kbd_timeout_slider->setEnabled(index != KeyboardSettings::AUTO ? false : true);

        showRebootMessage();
    } else if (m_kbd->getKeyboardType() == KeyboardSettings::SecondKeyboardGen) {
        m_kbd->kbd_timeout_label->setEnabled(index != KeyboardSettings::TIMER ? false : true);
        m_kbd->kbd_timeout->setEnabled(index != KeyboardSettings::TIMER ? false : true);
        m_kbd->kbd_timeout_slider->setEnabled(index != KeyboardSettings::TIMER ? false : true);
    }

    Q_EMIT changed(true);
}

void KToshibaSystemSettings::kbdTimeoutChanged(int time)
{
    m_kbd->kbd_timeout->setText(QString::number(time) % i18n(" sec"));

    Q_EMIT changed(true);
}

void KToshibaSystemSettings::updateTouchPad(int state)
{
    m_general->pointing_device_checkbox->setChecked(state == Qt::Checked ? true : false);
}


#include "systemsettings.moc"
