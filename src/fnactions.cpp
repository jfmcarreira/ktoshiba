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
      m_dBus(new KToshibaDBusInterface(this)),
      m_nl(new KToshibaNetlinkEvents(this)),
      m_hotkeys(new KToshibaKeyHandler(this)),
      m_hw(new KToshibaHardware(this)),
      m_config(KSharedConfig::openConfig(CONFIG_FILE)),
      m_widget(new QWidget(0, Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint)),
      m_widgetTimer(new QTimer(this)),
      m_keyboardType(FirstGeneration),
      m_keyboardMode(KToshibaHardware::TIMER),
      m_keyboardTime(15),
      m_monitorHDD(true),
      m_notifyHDD(true),
      m_cookie(0)
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

    powersave = KConfigGroup(m_config, "Powersave");
    hdd = KConfigGroup(m_config, "HDDProtection");

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
}

bool FnActions::init()
{
    if (!m_hotkeys->attach())
        return false;

    m_pointingSupported = isPointingDeviceSupported();
    m_kbdBacklightSupported = isKBDBacklightSupported();
    m_illuminationSupported = isIlluminationSupported();
    m_ecoSupported = isECOSupported();
    m_coolingMethodSupported = isCoolingMethodSupported();
    m_sataInterfaceSupported = isSATAInterfaceSupported();
    m_oddPowerSupported = isODDPowerSupported();

    if (checkConfig()) {
        loadConfig();
    } else {
        createConfig();
        loadConfig();
    }

    m_batteryProfiles << Performance << Powersave << Presentation << ECO;
    m_previousBatteryProfile = m_batteryProfile;
    if (m_batteryProfile == Presentation || m_batteryProfile == ECO)
        changeProfile(m_batteryProfile, true);
    else
        changeProfile(m_dBus->getBatteryProfile() == "AC" ? Performance : Powersave, true);

    if (m_nl->attach()) {
        m_hdd = m_hw->getProtectionLevel();

        connect(m_nl, SIGNAL(hapsEvent(int)), this, SLOT(protectHDD(int)));
        connect(m_nl, SIGNAL(tvapEvent(int)), this, SLOT(parseTVAPEvents(int)));
    } else {
        qCritical() << "Netlink events monitoring will not be possible";
    }

    m_dBus->init();

    if (m_kbdBacklightSupported && m_keyboardType == SecondGeneration)
        m_keyboardModes << KToshibaHardware::OFF << KToshibaHardware::ON << KToshibaHardware::TIMER;

    connect(m_dBus, SIGNAL(configFileChanged()), this, SLOT(reloadConfig()));
    connect(m_hotkeys, SIGNAL(hotkeyPressed(int)), this, SLOT(processHotkey(int)));

    return true;
}

bool FnActions::checkConfig()
{
    QString config = QStandardPaths::locate(QStandardPaths::ConfigLocation, CONFIG_FILE);

    if (config.isEmpty()) {
        qDebug() << "Configuration file not found.";

        return false;
    }

    return true;
}

void FnActions::loadConfig()
{
    qDebug() << "Loading configuration file...";
    // HDD Protection group
    m_monitorHDD = hdd.readEntry("MonitorHDD", true);
    m_notifyHDD = hdd.readEntry("NotifyHDDMovement", true);
    // Power Save group
    m_batteryProfile = powersave.readEntry("BatteryProfile", 0);
}

void FnActions::createConfig()
{
    qDebug() << "Default configuration file created.";
    // HDD Protection group
    hdd.writeEntry("MonitorHDD", true);
    hdd.writeEntry("NotifyHDDMovement", true);
    hdd.sync();
    // Power Save group
    powersave.writeEntry("BatteryProfile", 0);
    powersave.writeEntry("PerformanceSATAInterface", 0);
    powersave.writeEntry("PerformanceCoolingMethod", 0);
    powersave.writeEntry("PerformanceODDPower", 1);
    powersave.writeEntry("PerformanceIlluminationLED", 1);
    powersave.writeEntry("PowersaveSATAInterface", 1);
    powersave.writeEntry("PowersaveCoolingMethod", 1);
    powersave.writeEntry("PowersaveODDPower", 1);
    powersave.writeEntry("PowersaveIlluminationLED", 0);
    powersave.writeEntry("PresentationSATAInterface", 0);
    powersave.writeEntry("PresentationCoolingMethod", 1);
    powersave.writeEntry("PresentationODDPower", 1);
    powersave.writeEntry("PresentationIlluminationLED", 0);
    powersave.writeEntry("EcoSATAInterface", 1);
    powersave.writeEntry("EcoCoolingMethod", 1);
    powersave.writeEntry("EcoODDPower", 0);
    powersave.writeEntry("EcoIlluminationLED", 0);
    powersave.sync();
}

void FnActions::reloadConfig()
{
    loadConfig();

    if (m_batteryProfile != m_previousBatteryProfile)
        changeProfile(m_batteryProfile, false);
}

void FnActions::compositingChanged(bool state)
{
    m_widget->setAttribute(Qt::WA_TranslucentBackground, state);
}

bool FnActions::isSATAInterfaceSupported()
{
    quint32 result = m_hw->getSATAInterfaceSetting();
    if (result != KToshibaHardware::SATA_PERFORMANCE
        && result != KToshibaHardware::SATA_BATTERY_LIFE)
        return false;

    return true;
}

bool FnActions::isCoolingMethodSupported()
{
    quint32 result;
    int cooling, max_cooling;

    result = m_hw->getCoolingMethod(&cooling, &max_cooling);
    if (result != KToshibaHardware::SUCCESS && result != KToshibaHardware::SUCCESS2)
        return false;

    return true;
}

bool FnActions::isODDPowerSupported()
{
    quint32 result = m_hw->getODDPower();
    if (result != KToshibaHardware::ODD_DISABLED && result != KToshibaHardware::ODD_ENABLED)
        return false;

    return true;
}

bool FnActions::isIlluminationSupported()
{
    quint32 result = m_hw->getIlluminationLED();
    if (result != KToshibaHardware::DEACTIVATED
        && result != KToshibaHardware::ACTIVATED)
        return false;

    return true;
}

bool FnActions::isECOSupported()
{
    quint32 result = m_hw->getEcoLED();
    if (result != KToshibaHardware::DEACTIVATED && result != KToshibaHardware::ACTIVATED)
        return false;

    return true;
}

FnActions::BatteryProfiles FnActions::toBatteryProfiles(int profile)
{
    BatteryProfiles p;

    if (profile == 0)
        p = Performance;
    else if (profile == 1)
	p = Powersave;
    else if (profile == 2)
	p = Presentation;
    else if (profile == 3)
	p = ECO;

    return p;
}

void FnActions::updateBatteryProfile(QString profile)
{
    if (m_batteryProfile == Presentation || m_batteryProfile == ECO)
        return;

    changeProfile(profile == "AC" ? Performance : Powersave, false);
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
    QString text;
    bool inhib = false;
    bool kbdbl = false;

    switch (profile) {
    case Performance:
        text = i18n("Performance");
        m_cooling = powersave.readEntry("PerformanceCoolingMethod", 0);
        m_sata = powersave.readEntry("PerformanceSATAInterface", 0);
        m_odd = powersave.readEntry("PerformanceODDPower", 1);
        m_illumination = powersave.readEntry("PerformanceIlluminationLED", 1);
        kbdbl = true;
        break;
    case Powersave:
        text = i18n("Powersave");
        m_cooling = powersave.readEntry("PowersaveCoolingMethod", 1);
        m_sata = powersave.readEntry("PowersaveSATAInterface", 1);
        m_odd = powersave.readEntry("PowersaveODDPower", 1);
        m_illumination = powersave.readEntry("PowersaveIlluminationLED", 0);
        break;
    case Presentation:
        text = i18n("Presentation");
        m_cooling = powersave.readEntry("PresentationCoolingMethod", 0);
        m_sata = powersave.readEntry("PresentationSATAInterface", 1);
        m_odd = powersave.readEntry("PresentationODDPower", 1);
        m_illumination = powersave.readEntry("PresentationIlluminationLED", 0);
        m_cookie = m_dBus->inhibitPowerManagement(text);
        inhib = true;
        kbdbl = true;
        break;
    case ECO:
        text = i18n("ECO");
        m_cooling = powersave.readEntry("EcoCoolingMethod", 1);
        m_sata = powersave.readEntry("EcoSATAInterface", 1);
        m_odd = powersave.readEntry("EcoODDPower", 0);
        m_illumination = powersave.readEntry("EcoIlluminationLED", 0);
        m_dBus->setBrightness(m_dBus->getBatteryProfile() == "AC" ? 57 : 43);
        break;
    }

    if (m_batteryProfile != profile) {
        powersave.writeEntry("BatteryProfile", profile);
        powersave.sync();
    }
    if (m_coolingMethodSupported)
        m_hw->setCoolingMethod(m_cooling);
    if (m_sataInterfaceSupported)
        m_hw->setSATAInterfaceSetting(m_sata);
    if (m_oddPowerSupported)
        m_hw->setODDPower(0x100 | m_odd);
    if (m_illuminationSupported)
        m_hw->setIlluminationLED(m_illumination);
    if (m_ecoSupported)
        m_hw->setEcoLED(profile != ECO ? KToshibaHardware::DEACTIVATED : KToshibaHardware::ACTIVATED);
    if (m_kbdBacklightSupported) {
        if (m_keyboardType == FirstGeneration && m_keyboardMode == KToshibaHardware::FNZ)
            m_dBus->setKBDBacklight(kbdbl ? KToshibaHardware::ACTIVATED : KToshibaHardware::DEACTIVATED);
        else if (m_keyboardType == SecondGeneration && m_keyboardMode != KToshibaHardware::TIMER)
            m_hw->setKBDBacklight(kbdbl ? KToshibaHardware::ON : KToshibaHardware::OFF, m_keyboardTime);
    }

    if (m_cookie && !inhib) {
        m_dBus->unInhibitPowerManagement(m_cookie);
        m_cookie = 0;
    }

    m_statusWidget.statusIcon->setPixmap(QIcon::fromTheme("computer-laptop").pixmap(64, 64));
    m_statusWidget.statusLabel->setText(m_iconText.arg(text));
    if (!init)
        showWidget();

    qDebug() << "Changed battery profile to:" << toBatteryProfiles(profile);
    m_previousBatteryProfile = m_batteryProfile;
}

bool FnActions::isPointingDeviceSupported()
{
    m_pointing = m_hw->getPointingDevice();
    if (m_pointing != KToshibaHardware::DEACTIVATED && m_pointing != KToshibaHardware::ACTIVATED)
        return false;

    return true;
}

void FnActions::toggleTouchPad()
{
    if (m_pointingSupported) {
        m_pointing = m_hw->getPointingDevice();
        m_hw->setPointingDevice(!m_pointing);
        m_statusWidget.statusLabel->setText(m_iconText.arg(!m_pointing ? i18n("ON") : i18n("OFF")));
        m_statusWidget.statusIcon->setPixmap(QIcon::fromTheme("input-touchpad").pixmap(64, 64));
    } else {
        m_statusWidget.statusLabel->setText("");
        m_statusWidget.statusIcon->setPixmap(QIcon::fromTheme("dialog-cancel").pixmap(64, 64));
    }

    showWidget();
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
    if (!m_kbdBacklightSupported)
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
        toggleProfiles();
        break;
    case KEY_TOUCHPAD_TOGGLE:
        toggleTouchPad();
        break;
    case KEY_KBDILLUMTOGGLE:
        updateKBDBacklight();
        break;
    case KEY_ZOOMRESET:
        m_dBus->setZoom(KToshibaDBusInterface::ZoomReset);
        break;
    case KEY_ZOOMOUT:
        m_dBus->setZoom(KToshibaDBusInterface::ZoomOut);
        break;
    case KEY_ZOOMIN:
        m_dBus->setZoom(KToshibaDBusInterface::ZoomIn);
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
