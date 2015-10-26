/*
   Copyright (C) 2004-2015  Azael Avalos <coproscefalo@gmail.com>

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

#include <QDesktopWidget>
#include <QTimer>
#include <QDebug>

#include <KLocalizedString>
#include <KConfigGroup>

extern "C" {
#include <linux/input.h>
}

#include "fnactions.h"
#include "ktoshibahardware.h"
#include "ktoshibadbusinterface.h"
#include "ktoshibanetlinkevents.h"
#include "ktoshibakeyhandler.h"

#define CONFIG_FILE "ktoshibarc"

FnActions::FnActions(QObject *parent)
    : QObject(parent),
      m_config(KSharedConfig::openConfig(CONFIG_FILE)),
      m_sysinfo(false),
      m_dBus(new KToshibaDBusInterface(this)),
      m_nl(new KToshibaNetlinkEvents(this)),
      m_hotkeys(new KToshibaKeyHandler(this)),
      m_hw(new KToshibaHardware(this)),
      m_widget(new QWidget(0, Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint)),
      m_widgetTimer(new QTimer(this)),
      m_batteryKeyPressed(false),
      m_monitorHDD(true),
      m_notifyHDD(true),
      m_cookie(0),
      m_keyboardType(FirstGeneration),
      m_keyboardMode(KToshibaHardware::TIMER),
      m_keyboardTime(15)
{
    m_statusWidget.setupUi(m_widget);
    m_widget->clearFocus();
    // Only enable Translucency if composite is enabled
    // otherwise an ugly black background will appear
    m_widget->setAttribute(Qt::WA_TranslucentBackground, m_dBus->getCompositingState());

    m_iconText = QString("<html><head/><body><p align=\"center\">\
                          <span style=\"font-size:12pt; font-weight:600; color:#666666;\">\
                          %1\
                          </span></p></body></html>");

    connect(m_widgetTimer, SIGNAL(timeout()), this, SLOT(hideWidget()));
}

FnActions::~FnActions()
{
    if (m_cookie)
        m_dBus->unInhibitPowerManagement(m_cookie);
    delete m_widget; m_widget = NULL;
    delete m_widgetTimer; m_widgetTimer = NULL;
    delete m_dBus; m_dBus = NULL;
    delete m_hotkeys; m_hotkeys = NULL;
    delete m_nl; m_nl = NULL;
    delete m_hw; m_hw = NULL;
}

bool FnActions::init()
{
    if (!m_hotkeys->attach())
        return false;

    m_touchpad = isTouchPadSupported();
    m_illumination = isIlluminationSupported();
    m_eco = isECOSupported();
    m_kbdBacklight = isKBDBacklightSupported();
    m_cooling = isCoolingMethodSupported();

    if (checkConfig()) {
        loadConfig();
    } else {
        createConfig();
        loadConfig();
    }
    m_previousBatteryProfile = m_batteryProfile;

    m_batteryProfiles << Performance << Powersave << Presentation << ECO;
    changeProfile(m_batteryProfile, true);

    if (m_nl->attach()) {
        m_nl->setDeviceHID(m_hw->getDeviceHID());
        m_hdd = m_hw->getProtectionLevel();

        connect(m_nl, SIGNAL(hapsEvent(int)), this, SLOT(protectHDD(int)));
        connect(m_nl, SIGNAL(tvapEvent(int)), this, SLOT(parseTVAPEvents(int)));
    } else {
        qCritical() << "Netlink events monitoring will not be possible";
    }

    m_dBus->init();

    if (m_kbdBacklight && m_keyboardType == SecondGeneration)
        m_keyboardModes << KToshibaHardware::OFF << KToshibaHardware::ON << KToshibaHardware::TIMER;

    // Set initial Cooling Method state (if supported)
    if (m_cooling)
        updateCoolingMethod(m_dBus->getBatteryProfile());

    connect(m_dBus, SIGNAL(configChanged()), this, SLOT(loadConfig()));
    connect(m_dBus, SIGNAL(configChanged()), this, SLOT(updateBatteryProfile()));
    connect(m_hotkeys, SIGNAL(hotkeyPressed(int)), this, SLOT(processHotkey(int)));

    return true;
}

bool FnActions::checkConfig()
{
    QString config = QStandardPaths::locate(QStandardPaths::ConfigLocation, CONFIG_FILE);

    if (config.isEmpty()) {
        qDebug() << "Configuration file not found.";
        m_sysinfo = m_hw->getSysInfo();

        return false;
    }

    return true;
}

void FnActions::loadConfig()
{
    qDebug() << "Loading configuration file...";
    // HDD Protection group
    KConfigGroup hddGroup(m_config, "HDDProtection");
    m_monitorHDD = hddGroup.readEntry("MonitorHDD", true);
    m_notifyHDD = hddGroup.readEntry("NotifyHDDMovement", true);
    // Power Save group
    KConfigGroup powersave(m_config, "PowerSave");
    m_monitorBatteryProfiles = powersave.readEntry("BatteryProfiles", true);
    m_batteryProfile = powersave.readEntry("CurrentProfile", 0);
    m_manageCoolingMethod = powersave.readEntry("ManageCoolingMethod", true);
    m_coolingMethodPluggedIn = powersave.readEntry("CoolingMethodPluggedIn", 0);
    m_coolingMethodOnBattery = powersave.readEntry("CoolingMethodOnBattery", 1);
}

void FnActions::createConfig()
{
    qDebug() << "Default configuration file created.";
    // System Information group
    KConfigGroup sysinfoGroup(m_config, "SystemInformation");
    sysinfoGroup.writeEntry("ModelFamily", m_sysinfo ? m_hw->modelFamily : i18n("Unknown"));
    sysinfoGroup.writeEntry("ModelNumber", m_sysinfo ? m_hw->modelNumber : i18n("Unknown"));
    sysinfoGroup.writeEntry("BIOSVersion", m_sysinfo ? m_hw->biosVersion : i18n("Unknown"));
    sysinfoGroup.writeEntry("BIOSDate", m_sysinfo ? m_hw->biosDate : i18n("Unknown"));
    sysinfoGroup.writeEntry("BIOSManufacturer", m_sysinfo ? m_hw->biosManufacturer : i18n("Unknown"));
    sysinfoGroup.writeEntry("ECVersion", m_sysinfo ? m_hw->ecVersion : i18n("Unknown"));
    sysinfoGroup.sync();
    // HDD Protection group
    KConfigGroup hddGroup(m_config, "HDDProtection");
    hddGroup.writeEntry("MonitorHDD", true);
    hddGroup.writeEntry("NotifyHDDMovement", true);
    hddGroup.sync();
    // Power Save group
    KConfigGroup powersave(m_config, "PowerSave");
    powersave.writeEntry("BatteryProfiles", true);
    powersave.writeEntry("CurrentProfile", 0);
    powersave.writeEntry("ManageCoolingMethod", true);
    powersave.writeEntry("CoolingMethodOnBattery", 1);
    powersave.writeEntry("CoolingMethodPluggedIn", 0);
    powersave.sync();
}

void FnActions::compositingChanged(bool state)
{
    m_widget->setAttribute(Qt::WA_TranslucentBackground, state);
}

bool FnActions::isTouchPadSupported()
{
    quint32 tp = m_hw->getTouchPad();
    if (tp != KToshibaHardware::TCI_DISABLED && tp != KToshibaHardware::TCI_ENABLED)
        return false;

    return true;
}

void FnActions::toggleTouchPad()
{
    if (m_touchpad) {
        bool enabled = m_hw->getTouchPad();
        m_hw->setTouchPad(!enabled);
        m_statusWidget.statusLabel->setText(m_iconText.arg(!enabled ? i18n("ON") : i18n("OFF")));
        m_statusWidget.statusIcon->setPixmap(QIcon::fromTheme("input-touchpad").pixmap(64, 64));
    } else {
        m_statusWidget.statusLabel->setText("");
        m_statusWidget.statusIcon->setPixmap(QIcon::fromTheme("dialog-cancel").pixmap(64, 64));
    }

    showWidget();
}

bool FnActions::isCoolingMethodSupported()
{
    quint32 result;
    int cooling_method;

    result = m_hw->getCoolingMethod(&cooling_method, &m_maxCoolingMethod);
    if (result != KToshibaHardware::SUCCESS && result != KToshibaHardware::SUCCESS2)
        return false;

    return true;
}

void FnActions::updateCoolingMethod(QString profile)
{
    if (!m_cooling || !m_manageCoolingMethod)
        return;

    m_hw->setCoolingMethod(profile == "AC" ? m_coolingMethodPluggedIn : m_coolingMethodOnBattery);
}

bool FnActions::isIlluminationSupported()
{
    quint32 illum = m_hw->getIllumination();
    if (illum != KToshibaHardware::TCI_DISABLED && illum != KToshibaHardware::TCI_ENABLED)
        return false;

    return true;
}

bool FnActions::isECOSupported()
{
    quint32 eco = m_hw->getEcoLed();
    if (eco != KToshibaHardware::TCI_DISABLED && eco != KToshibaHardware::TCI_ENABLED)
        return false;

    return true;
}

void FnActions::updateBatteryProfile()
{
    if (!m_monitorBatteryProfiles)
        return;

    if (m_batteryProfile == m_previousBatteryProfile)
        return;

    changeProfile(m_batteryProfile, false);
}

void FnActions::toggleProfiles()
{
    int current = m_batteryProfiles.indexOf(m_batteryProfile);
    if (current == m_batteryProfiles.indexOf(m_batteryProfiles.last())) {
        m_batteryProfile = m_batteryProfiles.first();
    } else {
        current++;
        m_batteryProfile = m_batteryProfiles.at(current);
    }

    changeProfile(m_batteryProfile, false);
}

void FnActions::changeProfile(int profile, bool init)
{
    if (!m_monitorBatteryProfiles) {
        if (m_batteryKeyPressed) {
            m_statusWidget.statusIcon->setPixmap(QIcon::fromTheme("dialog-cancel").pixmap(64, 64));
            m_statusWidget.statusLabel->setText("");
            showWidget();
        }

        return;
    }

    bool inhib = false;
    int eco = Off;
    int illum = Off;
    switch (profile) {
    case Performance:
        m_statusWidget.statusLabel->setText(m_iconText.arg(i18n("Performance")));
        illum = On;
        break;
    case Powersave:
        m_statusWidget.statusLabel->setText(m_iconText.arg(i18n("Powersave")));
        break;
    case Presentation:
        m_statusWidget.statusLabel->setText(m_iconText.arg(i18n("Presentation")));
        illum = On;
        if (m_kbdBacklight) {
            if (m_keyboardType == FirstGeneration && m_keyboardMode == KToshibaHardware::FNZ)
                m_dBus->setKBDBacklight(On);
            else if (m_keyboardType == SecondGeneration)
                m_hw->setKBDBacklight(KToshibaHardware::ON, m_keyboardTime);
        }
        m_cookie = m_dBus->inhibitPowerManagement(i18n("Presentation"));
        inhib = true;
        break;
    case ECO:
        m_statusWidget.statusLabel->setText(m_iconText.arg("ECO"));
        eco = On;
        if (m_kbdBacklight) {
            if (m_keyboardType == FirstGeneration && m_keyboardMode == KToshibaHardware::FNZ) {
                m_dBus->setKBDBacklight(Off);
            } else if (m_keyboardType == 2) {
                m_hw->setKBDBacklight(KToshibaHardware::OFF, m_keyboardTime);
            }
        }
        m_dBus->setBrightness(57);
        break;
    }

    if (m_eco)
        m_hw->setEcoLed(eco);
    if (m_illumination)
        m_hw->setIllumination(illum);

    if (m_cookie && !inhib) {
        m_dBus->unInhibitPowerManagement(m_cookie);
        m_cookie = 0;
    }

    m_statusWidget.statusIcon->setPixmap(QIcon::fromTheme("computer-laptop").pixmap(64, 64));
    if (!init)
        showWidget();

    qDebug() << "Changed battery profile to:" << profile;
    m_previousBatteryProfile = m_batteryProfile;
}

bool FnActions::isKBDBacklightSupported()
{
    quint32 kbdbl = m_hw->getKBDBacklight(&m_keyboardMode, &m_keyboardTime, &m_keyboardType);
    if (kbdbl != KToshibaHardware::SUCCESS && kbdbl != KToshibaHardware::SUCCESS2)
        return false;

    return true;
}

void FnActions::updateKBDBacklight()
{
    if (!m_kbdBacklight)
        return;

    quint32 result = m_hw->getKBDBacklight(&m_keyboardMode, &m_keyboardTime, &m_keyboardType);
    if (result != KToshibaHardware::SUCCESS && result != KToshibaHardware::SUCCESS2) {
        qCritical() << "Could not get Keyboard Backlight status";

        return;
    }

    if (m_keyboardType == FirstGeneration) {
        if (m_keyboardMode == KToshibaHardware::TIMER)
            m_statusWidget.statusLabel->setText(m_iconText.arg(m_keyboardTime));
        else
            return;
    } else if (m_keyboardType == SecondGeneration) {
        int current = m_keyboardModes.indexOf(m_keyboardMode);
        if (current == m_keyboardModes.indexOf(m_keyboardModes.last())) {
            m_keyboardMode = m_keyboardModes.first();
        } else {
            current++;
            m_keyboardMode = m_keyboardModes.at(current);
        }

        m_hw->setKBDBacklight(m_keyboardMode, m_keyboardTime);
        switch (m_keyboardMode) {
        case KToshibaHardware::OFF:
            m_statusWidget.statusLabel->setText(m_iconText.arg(i18n("OFF")));
            break;
        case KToshibaHardware::ON:
            m_statusWidget.statusLabel->setText(m_iconText.arg(i18n("ON")));
            break;
        case KToshibaHardware::TIMER:
            m_statusWidget.statusLabel->setText(m_iconText.arg(m_keyboardTime));
            break;
        }
    }

    m_statusWidget.statusIcon->setPixmap(QIcon::fromTheme("input-keyboard").pixmap(64, 64));
    showWidget();
}

void FnActions::showWidget()
{
    QRect r = QApplication::desktop()->geometry();
    m_widget->move((r.width() / 2) - (m_widget->width() / 2), r.top());
    m_widget->show();

    if (m_widgetTimer->isActive())
        m_widgetTimer->setInterval(900);
    else
        m_widgetTimer->start(900);
}

void FnActions::hideWidget()
{
    m_widget->hide();
}

void FnActions::processHotkey(int hotkey)
{
    switch (hotkey) {
    case KEY_COFFEE:
        m_dBus->lockScreen();
        break;
    case KEY_BATTERY:
        m_batteryKeyPressed = true;
        toggleProfiles();
        break;
    case KEY_TOUCHPAD_TOGGLE:
        toggleTouchPad();
        break;
    case KEY_KBDILLUMTOGGLE:
        updateKBDBacklight();
        break;
    case KEY_ZOOMRESET:
        m_dBus->setZoom(KToshibaDBusInterface::Reset);
        break;
    case KEY_ZOOMOUT:
        m_dBus->setZoom(KToshibaDBusInterface::Out);
        break;
    case KEY_ZOOMIN:
        m_dBus->setZoom(KToshibaDBusInterface::In);
        break;
    }
}

void FnActions::protectHDD(int event)
{
    if (m_hdd == KToshibaHardware::FAILURE || !m_monitorHDD)
        return;

    if (event == KToshibaNetlinkEvents::Vibrated) {
        qDebug() << "Vibration detected";
        m_hw->unloadHeads(5000);
        if (m_notifyHDD)
            emit vibrationDetected();
    } else if (event == KToshibaNetlinkEvents::Stabilized) {
        qDebug() << "Vibration stabilized";
        m_hw->unloadHeads(0);
    }
}

void FnActions::parseTVAPEvents(int event)
{
    qDebug() << "Received event" << hex << event;
    switch (event) {
    case KToshibaNetlinkEvents::Hotkey:
        break;
    case KToshibaNetlinkEvents::Docked:
    case KToshibaNetlinkEvents::Undocked:
    case KToshibaNetlinkEvents::DockStatusChanged:
        break;
    case KToshibaNetlinkEvents::Thermal:
        break;
    case KToshibaNetlinkEvents::LIDClosed:
    case KToshibaNetlinkEvents::LIDClosedDockEjected:
        break;
    case KToshibaNetlinkEvents::SATAPower1:
    case KToshibaNetlinkEvents::SATAPower2:
        break;
    case KToshibaNetlinkEvents::KBDBacklightChanged:
        updateKBDBacklight();
        break;
    default:
        qDebug() << "Unknown event";
    }
}
