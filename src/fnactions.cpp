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
#include "ktoshibakeyhandler.h"

FnActions::FnActions(QObject *parent)
    : QObject(parent),
      m_dBus(new KToshibaDBusInterface(this)),
      m_hotkeys(new KToshibaKeyHandler(this)),
      m_hw(new KToshibaHardware(this)),
      m_widget(new QWidget(0, Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint)),
      m_widgetTimer(new QTimer(this)),
      m_batKeyPressed(false),
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

    // We're just going to care about these profiles
    m_profiles << "Performance" << "Presentation" << "ECO" << "Powersave";

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

    m_touchpad = isTouchPadSupported();
    m_illumination = isIlluminationSupported();
    m_eco = isECOSupported();
    m_kbdBacklight = isKBDBacklightSupported();
    if (m_kbdBacklight && m_type == 2)
            m_modes << KToshibaHardware::OFF << KToshibaHardware::ON << KToshibaHardware::TIMER;

    connect(m_dBus, SIGNAL(configChanged()), QObject::parent(), SLOT(configChanged()));
    connect(m_dBus, SIGNAL(batteryProfileChanged(QString)),
            QObject::parent(), SLOT(batteryProfileChanged(QString)));
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

void FnActions::toggleTouchPad()
{
    if (m_touchpad)
        m_hw->setTouchPad(!m_hw->getTouchPad());
}

void FnActions::toggleProfiles()
{
    int current = m_profiles.indexOf(m_profile);
    if (current == m_profiles.indexOf(m_profiles.last())) {
        m_profile = m_profiles.first();
    } else {
        current++;
        m_profile = m_profiles.at(current);
    }

    changeProfile(m_profile);
}

QString FnActions::getProfile()
{
    return m_dBus->getBatteryProfile();
}

void FnActions::changeProfile(QString profile)
{
    if (m_batMonitor) {
        if (m_batKeyPressed)
            showWidget(Disabled);

        return;
    }

    bool inhib = false;
    if (profile == "Powersave") {
        showWidget(Powersave);
        if (m_eco)
            m_hw->setEcoLed(Off);
        if (m_illumination)
            m_hw->setIllumination(Off);
    } else if (profile == "Performance") {
        showWidget(Performance);
        if (m_eco)
            m_hw->setEcoLed(Off);
        if (m_illumination)
            m_hw->setIllumination(On);
    } else if (profile == "Presentation") {
        showWidget(Presentation);
        if (m_eco)
            m_hw->setEcoLed(Off);
        if (m_illumination)
            m_hw->setIllumination(On);
        if (m_kbdBacklight) {
            if (m_type == 1 && m_mode == KToshibaHardware::FNZ) {
                m_dBus->setKBDBacklight(On);
            } else if (m_type == 2) {
                m_hw->setKBDBacklight(KToshibaHardware::ON, m_time);
            }
        }
        m_cookie = m_dBus->inhibitPowerManagement(i18n("Presentation"));
        inhib = true;
    } else if (profile == "ECO") {
        showWidget(ECO);
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
    }

    if (m_cookie && !inhib) {
        m_dBus->unInhibitPowerManagement(m_cookie);
        m_cookie = 0;
    }

    qDebug() << "Changed battery profile to:" << profile;
}

bool FnActions::isTouchPadSupported()
{
    quint32 tp = m_hw->getTouchPad();
    if (tp != 0 && tp != 1)
        return false;

    return true;
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

void FnActions::updateKBDBacklight()
{
    if (!m_kbdBacklight)
        return;

    QIcon icon;
    QString format = QString("<html><head/><body><p align=\"center\">\
			      <span style=\"font-size:12pt; font-weight:600; color:#666666;\">\
			      %1\
			      </span></p></body></html>");

    if (m_hw->getKBDBacklight(&m_mode, &m_time, &m_type) != KToshibaHardware::SUCCESS)
        qCritical() << "Could not get Keyboard Backlight status";

    if (m_type == 1) {
        if (m_mode == KToshibaHardware::TIMER) {
            icon = QIcon(":images/keyboard_black_on_64.png");
            m_statusWidget.kbdStatusText->setText(format.arg(m_time));
        } else {
            qDebug() << "Keyboard backlight mode is set to FN-Z";

            return;
        }
    }

    if (m_type == 2) {
        int current = m_modes.indexOf(m_mode);
        if (current == m_modes.indexOf(m_modes.last())) {
            m_mode = m_modes.first();
        } else {
            current++;
            m_mode = m_modes.at(current);
        }

        m_hw->setKBDBacklight(m_mode, m_time);
        switch (m_mode) {
        case KToshibaHardware::OFF:
            icon = QIcon(":images/keyboard_black_off_64.png");
            m_statusWidget.kbdStatusText->setText(format.arg(i18n("OFF")));
            break;
        case KToshibaHardware::ON:
            icon = QIcon(":images/keyboard_black_on_64.png");
            m_statusWidget.kbdStatusText->setText(format.arg(i18n("ON")));
            break;
        case KToshibaHardware::TIMER:
            icon = QIcon(":images/keyboard_black_on_64.png");
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
        m_batKeyPressed = true;
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
