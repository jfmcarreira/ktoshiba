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

#include "fnactions.h"
#include "ktoshibahardware.h"
#include "ktoshibadbusinterface.h"
#include "ktoshibanetlinkevents.h"

#define CONFIG_FILE "ktoshibarc"

FnActions::FnActions(QObject *parent)
    : QObject(parent),
      m_dBus(new KToshibaDBusInterface(this)),
      m_nl(new KToshibaNetlinkEvents(this)),
      m_hw(new KToshibaHardware(this)),
      m_config(KSharedConfig::openConfig(CONFIG_FILE)),
      m_widget(new QWidget(0, 0)),
      m_cookie(0)
{
    m_statusWidget.setupUi(m_widget);
    m_widget->setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint
                             | Qt::FramelessWindowHint | Qt::WindowTransparentForInput);
    m_widget->setAttribute(Qt::WA_TranslucentBackground, true);

    hdd = KConfigGroup(m_config, "HDDProtection");
    powersave = KConfigGroup(m_config, "Powersave");

    m_iconText = QString("<html><head/><body><p align=\"center\">\
                          <span style=\"font-size:12pt; font-weight:600; color:#666666;\">\
                          %1\
                          </span></p></body></html>");
}

FnActions::~FnActions()
{
    if (m_cookie)
        m_dBus->setPowerManagementInhibition(false, NULL, &m_cookie);

    delete m_widget; m_widget = NULL;
    delete m_dBus; m_dBus = NULL;
    delete m_nl; m_nl = NULL;
}

bool FnActions::init()
{
    if (!m_nl->attach()) {
        qCritical() << "Netlink events monitoring will not be possible";

        return false;
    }

    m_pointingSupported = isPointingDeviceSupported();
    m_kbdBacklightSupported = isKBDBacklightSupported();
    m_illuminationSupported = isIlluminationSupported();
    m_ecoSupported = isECOSupported();
    m_coolingMethodSupported = isCoolingMethodSupported();
    m_oddPowerSupported = isODDPowerSupported();
    m_keyboardFunctionsSupported = isKeyboardFunctionsSupported();

    if (checkConfig()) {
        qDebug() << "Loading configuration file";
        loadConfig();
    } else {
        qDebug() << "Configuration file not found";
        createConfig();
        loadConfig();
    }

    m_batteryProfiles << Performance << Powersave << Presentation << ECO;
    if (m_batteryProfile == ECO)
        changeBatteryProfile(m_batteryProfile, true);
    else
        changeBatteryProfile(m_dBus->getBatteryProfile() == "AC" ? Performance : Powersave, true);

    m_hdd = m_hw->getHDDProtectionLevel();
    if (m_hdd != KToshibaHardware::FAILURE && m_protectionLevel != m_hdd) {
        m_hw->setHDDProtectionLevel(m_protectionLevel);
        m_hdd = m_protectionLevel;
    }

    m_dBus->init();

    if (m_kbdBacklightSupported && m_keyboardType == SecondGeneration)
        m_keyboardModes << KToshibaHardware::OFF << KToshibaHardware::ON << KToshibaHardware::TIMER;

    connect(m_nl, SIGNAL(hapsEvent(int)), this, SLOT(parseHAPSEvents(int)));
    connect(m_nl, SIGNAL(tvapEvent(int, int)), this, SLOT(parseTVAPEvents(int, int)));
    connect(m_nl, SIGNAL(acAdapterChanged(int)), this, SLOT(updateBatteryProfile(int)));
    connect(m_dBus, SIGNAL(configFileChanged()), this, SLOT(reloadConfig()));
    connect(m_dBus, SIGNAL(zoomEffectDisabled()), QObject::parent(), SLOT(notifyZoomDisabled()));

    return true;
}

bool FnActions::checkConfig()
{
    QString config = QStandardPaths::locate(QStandardPaths::ConfigLocation, CONFIG_FILE);

    if (config.isEmpty())
        return false;

    return true;
}

void FnActions::loadConfig()
{
    // HDD Protection group
    m_monitorHDD = hdd.readEntry("MonitorHDD", true);
    m_notifyHDD = hdd.readEntry("NotifyHDDMovement", true);
    m_protectionLevel = hdd.readEntry("ProtectionLevel", 2);
    // Power Save group
    m_batteryProfile = powersave.readEntry("BatteryProfile", 0);
    qDebug() << "Stored battery profile:" << m_batteryProfile << (BatteryProfiles )m_batteryProfile;
}

void FnActions::createConfig()
{
    qDebug() << "Creating default configuration file";
    // HDD Protection group
    hdd.writeEntry("MonitorHDD", true);
    hdd.writeEntry("NotifyHDDMovement", true);
    hdd.writeEntry("ProtectionLevel", 2);
    hdd.sync();
    // Power Save group
    powersave.writeEntry("BatteryProfile", 0);
    powersave.writeEntry("PerformanceCoolingMethod", 0);
    powersave.writeEntry("PerformanceODDPower", 1);
    powersave.writeEntry("PerformanceIlluminationLED", 1);
    powersave.writeEntry("PowersaveCoolingMethod", 1);
    powersave.writeEntry("PowersaveODDPower", 1);
    powersave.writeEntry("PowersaveIlluminationLED", 0);
    powersave.writeEntry("PresentationCoolingMethod", 1);
    powersave.writeEntry("PresentationODDPower", 1);
    powersave.writeEntry("PresentationIlluminationLED", 0);
    powersave.writeEntry("EcoCoolingMethod", 1);
    powersave.writeEntry("EcoODDPower", 0);
    powersave.writeEntry("EcoIlluminationLED", 0);
    powersave.writeEntry("SATAInterface", 0);
    powersave.sync();
}

void FnActions::reloadConfig()
{
    qDebug() << "Re-loading configuration file";
    m_config->reparseConfiguration();
    loadConfig();

    if (m_batteryProfile != m_previousBatteryProfile)
        changeBatteryProfile(m_batteryProfile, false);
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

void FnActions::updateBatteryProfile(int ac_adapter)
{
    if (m_batteryProfile == Presentation || m_batteryProfile == ECO)
        return;

    changeBatteryProfile(ac_adapter == Connected ? Performance : Powersave, false);
}

void FnActions::toggleBatteryProfiles()
{
    int current = m_batteryProfiles.indexOf(m_batteryProfile);
    if (current == m_batteryProfiles.indexOf(m_batteryProfiles.last())) {
        m_batteryProfile = m_batteryProfiles.first();
    } else {
        current++;
        m_batteryProfile = m_batteryProfiles.at(current);
    }

    changeBatteryProfile(m_batteryProfile, false);
}

void FnActions::changeBatteryProfile(int profile, bool init)
{
    QString text;
    bool kbdbl = false;
    m_inhibitPowerManagement = false;

    switch (profile) {
    case Performance:
        text = i18n("Performance");
        m_cooling = powersave.readEntry("PerformanceCoolingMethod", 0);
        m_odd = powersave.readEntry("PerformanceODDPower", 1);
        m_illumination = powersave.readEntry("PerformanceIlluminationLED", 1);
        kbdbl = true;
        break;
    case Powersave:
        text = i18n("Powersave");
        m_cooling = powersave.readEntry("PowersaveCoolingMethod", 1);
        m_odd = powersave.readEntry("PowersaveODDPower", 1);
        m_illumination = powersave.readEntry("PowersaveIlluminationLED", 0);
        break;
    case Presentation:
        text = i18n("Presentation");
        m_cooling = powersave.readEntry("PresentationCoolingMethod", 0);
        m_odd = powersave.readEntry("PresentationODDPower", 1);
        m_illumination = powersave.readEntry("PresentationIlluminationLED", 0);
        m_inhibitPowerManagement = true;
        kbdbl = true;
        break;
    case ECO:
        text = i18n("ECO");
        m_cooling = powersave.readEntry("EcoCoolingMethod", 1);
        m_odd = powersave.readEntry("EcoODDPower", 0);
        m_illumination = powersave.readEntry("EcoIlluminationLED", 0);
        m_dBus->setBrightness(m_dBus->getBatteryProfile() == "AC" ? 57 : 43);
        break;
    }

    if (m_batteryProfile != profile) {
        qDebug() << "Syncing BatteryProfile configuration entry";
        powersave.writeEntry("BatteryProfile", profile);
        powersave.sync();
    }

    if (m_coolingMethodSupported)
        m_hw->setCoolingMethod(m_cooling);

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

    m_dBus->setPowerManagementInhibition(m_inhibitPowerManagement, text, &m_cookie);

    m_statusWidget.statusIcon->setPixmap(QIcon::fromTheme("computer-laptop").pixmap(64, 64));
    m_statusWidget.statusLabel->setText(m_iconText.arg(text));
    if (!init)
        showWidget();

    qDebug() << "Changed battery profile to:" << profile << (BatteryProfiles )profile;
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
    if (!m_pointingSupported)
        return;

    m_pointing = m_hw->getPointingDevice();
    m_hw->setPointingDevice(!m_pointing);
    m_statusWidget.statusLabel->setText(m_iconText.arg(!m_pointing ? i18n("ON") : i18n("OFF")));
    m_statusWidget.statusIcon->setPixmap(QIcon::fromTheme("input-touchpad").pixmap(64, 64));
    if (m_keyboardFunctionsSupported && m_kbdFunctions)
        showWidget();
}

bool FnActions::isKBDBacklightSupported()
{
    m_keyboardMode = m_keyboardTime = m_keyboardType = -1;
    updateKBDBacklight();

    if (m_keyboardMode != -1 && m_keyboardTime != -1 && m_keyboardType == -1)
        return false;

    return true;
}

void FnActions::updateKBDBacklight()
{
    quint32 result = m_hw->getKBDBacklight(&m_keyboardMode, &m_keyboardTime, &m_keyboardType);
    if (result != KToshibaHardware::SUCCESS && result != KToshibaHardware::SUCCESS2)
        qCritical() << "Could not get Keyboard Backlight status";
}

void FnActions::toggleKBDBacklight()
{
    if (!m_kbdBacklightSupported)
        return;

    switch (m_keyboardType) {
    case FirstGeneration:
        if (m_keyboardMode == KToshibaHardware::TIMER)
            m_statusWidget.statusLabel->setText(m_iconText.arg(m_keyboardTime));
        else
            return;
        break;
    case SecondGeneration:
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
        break;
    }

    m_statusWidget.statusIcon->setPixmap(QIcon::fromTheme("input-keyboard").pixmap(64, 64));
    showWidget();
}

bool FnActions::isKeyboardFunctionsSupported()
{
    m_kbdFunctions = m_hw->getKBDFunctions();

    if (m_kbdFunctions != KToshibaHardware::DEACTIVATED && m_kbdFunctions != KToshibaHardware::ACTIVATED)
        return false;

    return true;
}

void FnActions::showWidget()
{
    QRect r = QApplication::desktop()->geometry();
    m_widget->move((r.width() / 2) - (m_widget->width() / 2), r.top());
    m_widget->show();
}

void FnActions::processHotkey(int hotkey)
{
    switch (hotkey) {
    case 0x102: // FN-1 - Zoom-Out
        m_dBus->setZoom(KToshibaDBusInterface::ZoomOut);
        break;
    case 0x103: // FN-2 - Zoom-In
        m_dBus->setZoom(KToshibaDBusInterface::ZoomIn);
        break;
    case 0x12c: // FN-Z - Toggle Keyboard Backlight
        toggleKBDBacklight();
        break;
    case 0x139: // FN-Space - Zoom-Reset
        m_dBus->setZoom(KToshibaDBusInterface::ZoomReset);
        break;
    /*
     * FN-F1
     * Lock Screen (Old keyboard layout)
     * Help (New keyboard layout)
     */
    case 0x13b:
        if (m_keyboardFunctionsSupported)
            return;
        m_dBus->lockScreen();
        break;
    /*
     * FN-F2
     * Battery Profiles (Old keyboard layout)
     * Brightness Down (New keyboard layout)
     */
    case 0x13c:
        if (m_keyboardFunctionsSupported)
            return;
        toggleBatteryProfiles();
        break;
    /*
     * FN-F5
     * Switch Video-Out (Old keyboard layout)
     * TouchPad Toggle (New keyboard layout)
     */
    case 0x13f:
        if (!m_keyboardFunctionsSupported)
            return;
        toggleTouchPad();
        break;
    /*
     * FN-F9
     * TouchPad Toggle (Old keyboard layout)
     * Volume Down (New keyboard layout)
     */
    case 0x143:
        if (m_keyboardFunctionsSupported)
            return;
        toggleTouchPad();
        break;
    case 0x1ff: // FN pressed
        break;
    case 0x100: // FN released
    case 0x182: // FN-1 released
    case 0x183: // FN-2 released
    case 0x1ac: // FN-Z released
    case 0x1b9: // FN-Space released
    case 0x1bb: // FN-F1 released
    case 0x1bc: // FN-F2 released
    case 0x1bf: // FN-F5 released
    case 0x1c3: // FN-F9 released
        QTimer::singleShot(1500, m_widget, SLOT(hide()));
        break;
    }
}

void FnActions::parseHAPSEvents(int event)
{
    if (m_hdd == KToshibaHardware::FAILURE || !m_monitorHDD)
        return;

    switch (event) {
    case KToshibaNetlinkEvents::Vibrated:
        qDebug() << "Vibration detected";
        m_hw->unloadHDDHeads(5000);
        if (m_notifyHDD)
            emit vibrationDetected();
        break;
    case KToshibaNetlinkEvents::Stabilized:
        qDebug() << "Vibration stabilized";
        m_hw->unloadHDDHeads(0);
        break;
    }
}

void FnActions::parseTVAPEvents(int event, int data)
{
    switch (event) {
    case KToshibaNetlinkEvents::Hotkey:
        if (data <= 0x1ff)
            processHotkey(data);
        else if (data > 0x400)
            parseExtraTVAPEvents(data);
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
        qDebug() << "Unknown event:" << hex << event;
    }
}

void FnActions::parseExtraTVAPEvents(int event)
{
    switch (event) {
    case 0x1430: // Wake from Sleep
        break;
    case 0x1501: // Output changed
        break;
    case 0x1502: // HDMI (un)plugged
        break;
    case 0x1abe: // HDD protection level set
        break;
    case 0x1abf: // HDD protection level off
        break;
    case 0x401:
    case 0x402:
    case 0x1500:
    case 0x1580:
    case 0x1581:
    case 0x19b0:
    case 0x19b1:
    case 0x19b2:
    case 0x19b3:
    case 0x19b6:
    case 0x19b7:
    default:
        qDebug() << "Unknown event:" << hex << event;
    }
}
