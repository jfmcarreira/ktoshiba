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
#include "ktoshibakeyhandler.h"

#define CONFIG_FILE "ktoshibarc"

FnActions::FnActions(QObject *parent)
    : QObject(parent),
      m_config(KSharedConfig::openConfig(CONFIG_FILE)),
      m_dBus(new KToshibaDBusInterface(this)),
      m_hotkeys(new KToshibaKeyHandler(this)),
      m_hw(new KToshibaHardware(this)),
      m_widget(new QWidget(0, Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint)),
      m_widgetTimer(new QTimer(this)),
      m_batteryKeyPressed(false),
      m_cookie(0),
      m_type(1),
      m_mode(KToshibaHardware::TIMER),
      m_time(15)
{
    m_statusWidget.setupUi(m_widget);
    m_widget->clearFocus();
    // Only enable Translucency if composite is enabled
    // otherwise an ugly black background will appear
    m_widget->setAttribute(Qt::WA_TranslucentBackground, m_dBus->getCompositingState());

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
    delete m_hw; m_hw = NULL;
}

bool FnActions::init()
{
    if (!m_hotkeys->attach())
        return false;

    if (!m_hw->init()) {
        qCritical() << "Could not communicate with library, hardware changes will not be possible";

        return false;
    }

    m_dBus->init();

    m_profiles << Performance << Powersave << Presentation << ECO;
    updateBatteryProfile(true);
    m_touchpad = isTouchPadSupported();
    m_illumination = isIlluminationSupported();
    m_eco = isECOSupported();
    m_kbdBacklight = isKBDBacklightSupported();
    if (m_kbdBacklight && m_type == 2)
            m_keyboardModes << KToshibaHardware::OFF << KToshibaHardware::ON << KToshibaHardware::TIMER;
    m_cooling = isCoolingMethodSupported();
    if (m_cooling) {
        updateCoolingMethod(m_dBus->getBatteryProfile());

        connect(m_dBus, SIGNAL(batteryProfileChanged(QString)),
                this, SLOT(updateCoolingMethod(QString)));
    }

    connect(m_dBus, SIGNAL(configChanged()), this, SLOT(updateBatteryProfile()));
    connect(m_dBus, SIGNAL(configChanged()), QObject::parent(), SLOT(configChanged()));
    connect(m_hotkeys, SIGNAL(hotkeyPressed(int)), this, SLOT(processHotkey(int)));

    return true;
}

void FnActions::compositingChanged(bool state)
{
    m_widget->setAttribute(Qt::WA_TranslucentBackground, state);
}

void FnActions::batMonitorChanged(bool state)
{
    m_batMonitor = state;
}

bool FnActions::isTouchPadSupported()
{
    quint32 tp = m_hw->getTouchPad();
    if (tp != 0 && tp != 1)
        return false;

    return true;
}

void FnActions::toggleTouchPad()
{
    if (m_touchpad)
        m_hw->setTouchPad(!m_hw->getTouchPad());
}

bool FnActions::isCoolingMethodSupported()
{
    quint32 result = m_hw->getCoolingMethod(&m_coolingMethod, &m_maxCoolingMethod);

    if (result != KToshibaHardware::SUCCESS && result != KToshibaHardware::SUCCESS2)
        return false;

    return true;
}

void FnActions::updateCoolingMethod(QString profile)
{
    if (!m_cooling)
        return;

    KConfigGroup powersave(m_config, "PowerSave");
    m_manageCoolingMethod = powersave.readEntry("ManageCoolingMethod", true);

    if (!m_manageCoolingMethod)
        return;

    int coolingMethod = KToshibaHardware::MAXIMUM_PERFORMANCE;
    if (profile == "AC")
        coolingMethod = powersave.readEntry("CoolingMethodPluggedIn", 0);
    else if (profile == "Battery")
        coolingMethod = powersave.readEntry("CoolingMethodOnBattery", 1);
    else
        return;

    m_hw->setCoolingMethod(coolingMethod);
}

bool FnActions::isIlluminationSupported()
{
    quint32 illum = m_hw->getIllumination();
    if (illum != 0 && illum != 1)
        return false;

    return true;
}

bool FnActions::isECOSupported()
{
    quint32 eco = m_hw->getEcoLed();
    if (eco != 0 && eco != 1)
        return false;

    return true;
}

bool FnActions::isKBDBacklightSupported()
{
    quint32 kbdbl = m_hw->getKBDBacklight(&m_mode, &m_time, &m_type);
    if (kbdbl != KToshibaHardware::SUCCESS && kbdbl != KToshibaHardware::SUCCESS2)
        return false;

    return true;
}

void FnActions::updateBatteryProfile(bool init)
{
    KConfigGroup powersave(m_config, "PowerSave");
    bool batterymonitor = powersave.readEntry("BatteryProfiles", true);
    if (m_batMonitor != batterymonitor)
        m_batMonitor = batterymonitor;

    if (!m_batMonitor)
        return;

    m_batteryProfile = powersave.readEntry("CurrentProfile", 0);

    changeProfile(m_batteryProfile, init);
}

void FnActions::toggleProfiles()
{
    int current = m_profiles.indexOf(m_batteryProfile);
    if (current == m_profiles.indexOf(m_profiles.last())) {
        m_batteryProfile = m_profiles.first();
    } else {
        current++;
        m_batteryProfile = m_profiles.at(current);
    }

    changeProfile(m_batteryProfile, false);
}

void FnActions::changeProfile(int profile, bool init)
{
    if (!m_batMonitor) {
        if (m_batteryKeyPressed)
            showWidget(Disabled);

        return;
    }

    QIcon icon = QIcon::fromTheme("computer-laptop");
    QString format = QString("<html><head/><body><p align=\"center\">\
			      <span style=\"font-size:12pt; font-weight:600; color:#666666;\">\
			      %1\
			      </span></p></body></html>");

    bool inhib = false;
    switch (profile) {
    case Performance:
        m_statusWidget.batteryProfileLabel->setText(format.arg(i18n("Performance")));
        if (m_eco)
            m_hw->setEcoLed(Off);
        if (m_illumination)
            m_hw->setIllumination(On);
        break;
    case Powersave:
        m_statusWidget.batteryProfileLabel->setText(format.arg(i18n("Powersave")));
        if (m_eco)
            m_hw->setEcoLed(Off);
        if (m_illumination)
            m_hw->setIllumination(Off);
        break;
    case Presentation:
        m_statusWidget.batteryProfileLabel->setText(format.arg(i18n("Presentation")));
        if (m_eco)
            m_hw->setEcoLed(Off);
        if (m_illumination)
            m_hw->setIllumination(On);
        if (m_kbdBacklight) {
            if (m_type == 1 && m_mode == KToshibaHardware::FNZ)
                m_dBus->setKBDBacklight(On);
            else if (m_type == 2)
                m_hw->setKBDBacklight(KToshibaHardware::ON, m_time);
        }
        m_cookie = m_dBus->inhibitPowerManagement(i18n("Presentation"));
        inhib = true;
        break;
    case ECO:
        m_statusWidget.batteryProfileLabel->setText(format.arg("ECO"));
        if (m_eco)
            m_hw->setEcoLed(On);
        if (m_illumination)
            m_hw->setIllumination(Off);
        if (m_kbdBacklight) {
            if (m_type == 1 && m_mode == KToshibaHardware::FNZ) {
                m_dBus->setKBDBacklight(Off);
            } else if (m_type == 2) {
                m_hw->setKBDBacklight(KToshibaHardware::OFF, m_time);
            }
        }
        m_dBus->setBrightness(57);
        break;
    }

    if (m_cookie && !inhib) {
        m_dBus->unInhibitPowerManagement(m_cookie);
        m_cookie = 0;
    }

    m_statusWidget.batteryProfileIcon->setPixmap(icon.pixmap(64, 64));
    if (!init)
        showWidget(BatteryProfile);

    qDebug() << "Changed battery profile to:" << profile;
}

void FnActions::updateKBDBacklight()
{
    if (!m_kbdBacklight)
        return;

    quint32 result = m_hw->getKBDBacklight(&m_mode, &m_time, &m_type);
    if (result != KToshibaHardware::SUCCESS && result != KToshibaHardware::SUCCESS2) {
        qCritical() << "Could not get Keyboard Backlight status";

        return;
    }

    QIcon icon = QIcon::fromTheme("input-keyboard");
    QString format = QString("<html><head/><body><p align=\"center\">\
			      <span style=\"font-size:12pt; font-weight:600; color:#666666;\">\
			      %1\
			      </span></p></body></html>");

    if (m_type == 1) {
        if (m_mode == KToshibaHardware::TIMER)
            m_statusWidget.kbdStatusText->setText(format.arg(m_time));
        else
            return;
    } else if (m_type == 2) {
        int current = m_keyboardModes.indexOf(m_mode);
        if (current == m_keyboardModes.indexOf(m_keyboardModes.last())) {
            m_mode = m_keyboardModes.first();
        } else {
            current++;
            m_mode = m_keyboardModes.at(current);
        }

        m_hw->setKBDBacklight(m_mode, m_time);
        switch (m_mode) {
        case KToshibaHardware::OFF:
            m_statusWidget.kbdStatusText->setText(format.arg(i18n("OFF")));
            break;
        case KToshibaHardware::ON:
            m_statusWidget.kbdStatusText->setText(format.arg(i18n("ON")));
            break;
        case KToshibaHardware::TIMER:
            m_statusWidget.kbdStatusText->setText(format.arg(m_time));
            break;
        }
    }

    m_statusWidget.kbdStatusIcon->setPixmap(icon.pixmap(64, 64));
    showWidget(KBDStatus);
}

void FnActions::showWidget(int wid)
{
    QRect r = QApplication::desktop()->geometry();
    m_widget->move((r.width() / 2) - (m_widget->width() / 2), r.top());
    m_widget->show();

    m_statusWidget.stackedWidget->setCurrentIndex(wid);

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
        m_dBus->setZoom(Reset);
        break;
    case KEY_ZOOMOUT:
        m_dBus->setZoom(Out);
        break;
    case KEY_ZOOMIN:
        m_dBus->setZoom(In);
        break;
    }
}
