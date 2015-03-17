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

#include <KLocale>
#include <KDebug>
#include <KWindowSystem>

#include <solid/device.h>
#include <solid/acadapter.h>
#include <solid/powermanagement.h>

extern "C" {
#include <linux/input.h>
}

#include "fnactions.h"
#include "helperactions.h"
#include "ktoshibadbusinterface.h"
#include "ktoshibakeyhandler.h"

using namespace Solid::PowerManagement;

FnActions::FnActions(QObject *parent)
    : QObject( parent ),
      m_helper( new HelperActions( this ) ),
      m_type( 1 ),
      m_mode( TIMER ),
      m_time( 15 ),
      m_dBus( new KToshibaDBusInterface( this ) ),
      m_hotkeys( new KToshibaKeyHandler( this ) ),
      m_widget( new QWidget( 0, Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint ) ),
      m_widgetTimer( new QTimer( this ) ),
      m_fnPressed( false ),
      m_batKeyPressed( false ),
      m_cookie( 0 )
{
    m_statusWidget.setupUi( m_widget );
    m_widget->clearFocus();
    // Only enable Translucency if composite is enabled
    // otherwise an ugly black background will appear
    m_widget->setAttribute( Qt::WA_TranslucentBackground, KWindowSystem::compositingActive() );

    // We're just going to care about these profiles
    m_profiles << "Performance" << "Presentation" << "ECO" << "Powersave";

    connect( m_widgetTimer, SIGNAL( timeout() ), this, SLOT( hideWidget() ) );
    connect( KWindowSystem::self(), SIGNAL( compositingChanged(bool) ), this, SLOT( compositingChanged(bool) ) );
    connect( parent, SIGNAL( batteryProfilesToggled(bool) ), this, SLOT( batMonitorChanged(bool) ) );
}

FnActions::~FnActions()
{
    delete m_widget; m_widget = NULL;
    delete m_widgetTimer; m_widgetTimer = NULL;
    delete m_dBus; m_dBus = NULL;
    delete m_hotkeys; m_hotkeys = NULL;
    delete m_helper; m_helper = NULL;
}

bool FnActions::init()
{
    m_keyConnected = m_hotkeys->attach();
    if (!m_keyConnected)
        return false;

    m_helperConnected = m_helper->init();
    if (!m_helperConnected)
        return false;

    m_dbusConnected = m_dBus->init();
    if (!m_dbusConnected)
        return false;

    if (m_helper->isKBDBacklightSupported) {
        m_mode = m_helper->getKBDMode();
        if (m_helper->isKBDTypeSupported)
            m_type = m_helper->getKBDType();

        if (m_type == 2)
            m_modes << OFF << ON << TIMER;
    }

    connect( m_hotkeys, SIGNAL( hotkeyPressed(int) ), this, SLOT( processHotkey(int) ) );

    return true;
}

void FnActions::compositingChanged(bool state)
{
    m_widget->setAttribute( Qt::WA_TranslucentBackground, state );
}

void FnActions::batMonitorChanged(bool state)
{
    m_batMonitor = state;
}

void FnActions::toggleTouchPad()
{
    if (!m_helper->isTouchPadSupported)
        return;

    m_helper->setTouchPad(!m_helper->getTouchPad());
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

void FnActions::changeProfile(QString profile)
{
    if (m_batMonitor) {
        if (m_batKeyPressed)
            showWidget(Disabled);

        return;
    }

    if (profile == "Powersave") {
        showWidget(Powersave);
        if (m_helper->isECOSupported)
            m_helper->setEcoLed(Off);
        if (m_helper->isIlluminationSupported)
            m_helper->setIllumination(Off);
    } else if (profile == "Performance") {
        showWidget(Performance);
        if (m_helper->isECOSupported)
            m_helper->setEcoLed(Off);
        if (m_helper->isIlluminationSupported)
            m_helper->setIllumination(On);
    } else if (profile == "Presentation") {
        showWidget(Presentation);
        if (m_helper->isECOSupported)
            m_helper->setEcoLed(Off);
        if (m_helper->isIlluminationSupported)
            m_helper->setIllumination(On);
        if (m_helper->isKBDBacklightSupported) {
            if (m_type == 1 && m_mode == FNZ)
                m_dBus->setKBDBacklight(On);
            else if (m_type == 2)
                m_helper->setKBDMode(ON);
        }
        m_dBus->setBrightness(71);
        m_cookie = beginSuppressingScreenPowerManagement(m_profile);
    } else if (profile == "ECO") {
        showWidget(ECO);
        if (m_helper->isECOSupported)
            m_helper->setEcoLed(On);
        if (m_helper->isIlluminationSupported)
            m_helper->setIllumination(Off);
        if (m_helper->isKBDBacklightSupported) {
            if (m_type == 1 && m_mode == FNZ)
                m_dBus->setKBDBacklight(Off);
            else if (m_type == 2)
                m_helper->setKBDMode(OFF);
        }
        m_dBus->setBrightness(57);
    }
    kDebug() << "Changed battery profile to:" << m_profile;

    if (m_cookie != 0)
        if (stopSuppressingScreenPowerManagement(m_cookie))
            m_cookie = 0;
}

void FnActions::kbdBacklight()
{
    if (!m_helper->isKBDBacklightSupported)
        return;

    QIcon icon;
    QString format = QString("<html><head/><body><p align=\"center\">\
			      <span style=\"font-size:12pt; font-weight:600; color:#666666;\">\
			      %1\
			      </span></p></body></html>");

    m_mode = m_helper->getKBDMode();

    if (m_type == 1) {
        if (m_mode == TIMER) {
            int time = m_helper->getKBDTimeout();
            icon = QIcon(":images/keyboard_black_on_64.png");
            m_statusWidget.kbdStatusText->setText(format.arg(time));
        } else {
            kDebug() << "Keyboard backlight mode is set to FN-Z";
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

        m_helper->setKBDMode(m_mode);
        switch (m_mode) {
        case OFF:
            icon = QIcon(":images/keyboard_black_off_64.png");
            m_statusWidget.kbdStatusText->setText(format.arg(i18n("OFF")));
            break;
        case ON:
            icon = QIcon(":images/keyboard_black_on_64.png");
            m_statusWidget.kbdStatusText->setText(format.arg(i18n("ON")));
            break;
        case TIMER:
            icon = QIcon(":images/keyboard_black_on_64.png");
            int time = m_helper->getKBDTimeout();
            m_statusWidget.kbdStatusText->setText(format.arg(time));
            break;
        }
    }

    m_statusWidget.kbdStatusIcon->setPixmap(icon.pixmap(64, 64));
    showWidget(KBDStatus);
}

void FnActions::showWidget(int wid)
{
    QRect r = QApplication::desktop()->geometry();
    m_widget->move(r.center() -
                QPoint(m_widget->width() / 2, m_widget->height() / 2));
    m_widget->show();

    if (wid < 0)
        m_statusWidget.stackedWidget->setCurrentWidget( m_statusWidget.stackedWidget->widget(Disabled) );
    else
        m_statusWidget.stackedWidget->setCurrentWidget( m_statusWidget.stackedWidget->widget(wid) );

    if (m_widgetTimer->isActive())
        m_widgetTimer->setInterval( 900 );
    else
        m_widgetTimer->start( 900 );
}

void FnActions::hideWidget()
{
    m_widget->hide();
}

void FnActions::processHotkey(int hotkey)
{
    switch ( hotkey ) {
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
        kbdBacklight();
        break;
    case KEY_ZOOMRESET:
        m_dBus->setZoom( Reset );
        break;
    case KEY_ZOOMOUT:
        m_dBus->setZoom( Out );
        break;
    case KEY_ZOOMIN:
        m_dBus->setZoom( In );
        break;
    /* By default, the FN key is ignored by the driver,
     * but could be very useful for us...
     */
    /*case KEY_FN:
        m_fnPressed = m_fnPressed ? false : true;
        if (m_fnPressed)
            showWidget(Blank);
        else
            m_widget->hide();
        break;*/
    }
}


#include "fnactions.moc"
