/*
   Copyright (C) 2004-2014  Azael Avalos <coproscefalo@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QtGui/QDesktopWidget>
#include <QtCore/QTimer>
#include <QtCore/QFile>

#include <KLocale>
#include <KDebug>
#include <KWindowSystem>
#include <KAuth/Action>

#include <solid/device.h>
#include <solid/acadapter.h>
#include <solid/powermanagement.h>

extern "C" {
#include <linux/input.h>
}

#include "fnactions.h"
#include "ktoshibadbusinterface.h"
#include "ktoshibakeyhandler.h"

#define HELPER_ID "net.sourceforge.ktoshiba.ktoshhelper"

using namespace Solid::PowerManagement;

FnActions::FnActions(QObject *parent)
    : QObject( parent ),
      m_dBus( new KToshibaDBusInterface( parent ) ),
      m_keyHandler( new KToshibaKeyHandler( this ) ),
      m_widget( new QWidget( 0, Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint ) ),
      m_widgetTimer( new QTimer( this ) ),
      m_fnPressed( false ),
      m_cookie( 0 )
{
    m_statusWidget.setupUi( m_widget );
    m_widget->clearFocus();
    // Only enable Translucency if composite is enabled
    // otherwise an ugly black background will appear
    m_widget->setAttribute( Qt::WA_TranslucentBackground, KWindowSystem::compositingActive() );

    // We're just going to care about these profiles
    m_profiles << "Performance" << "Presentation" << "ECO" << "Powersave";

    QList<Solid::Device> list = Solid::Device::listFromType(Solid::DeviceInterface::AcAdapter, QString());
    Solid::Device device = list[0];
    Solid::AcAdapter *acAdapter = device.as<Solid::AcAdapter>();
    m_profile = (acAdapter->isPlugged()) ? QString("Performance") : QString("Powersave");

    connect( m_widgetTimer, SIGNAL( timeout() ), this, SLOT( hideWidget() ) );
    connect( m_keyHandler, SIGNAL( hotkeyPressed(int) ), this, SLOT( processHotkey(int) ) );
    connect( acAdapter, SIGNAL( plugStateChanged(bool, QString) ), this, SLOT( acAdapterChanged(bool) ) );
    connect( KWindowSystem::self(), SIGNAL( compositingChanged(bool) ), this, SLOT( compositingChanged(bool) ) );
    connect( m_dBus, SIGNAL( kbdModeChanged() ), this, SLOT( changeKBDMode() ) );
    connect( m_dBus, SIGNAL( kbdTimeoutChanged(int) ), this, SLOT( changeKBDTimeout(int) ) );
    connect( m_dBus, SIGNAL( touchpadChanged() ), this, SLOT( toggleTouchPad() ) );
    connect( m_dBus, SIGNAL( ecoChanged(bool) ), this, SLOT( toggleEcoLed(bool) ) );
}

FnActions::~FnActions()
{
    delete m_widget; m_widget = NULL;
    delete m_keyHandler; m_keyHandler = NULL;
    delete m_dBus; m_dBus = NULL;
}

void FnActions::acAdapterChanged(bool connected)
{
    if (m_profile == "Powersave" && connected) {
        m_profile = "Performance";
        changeProfile(m_profile);
        return;
    }

    if (m_profile == "Performance" && !connected) {
        m_profile = "Powersave";
        changeProfile(m_profile);
        return;
    }
}

void FnActions::compositingChanged(bool state)
{
    m_widget->setAttribute( Qt::WA_TranslucentBackground, state );
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
    if (profile == "Powersave") {
        showWidget(Powersave);
        toggleEcoLed(Off);
        toggleIllumination(Off);
        m_dBus->setBrightness(42);
        toggleKBDBacklight(Off);
    } else if (profile == "Performance") {
        showWidget(Performance);
        toggleEcoLed(Off);
        toggleIllumination(On);
        m_dBus->setBrightness(100);
    } else if (profile == "Presentation") {
        showWidget(Presentation);
        toggleEcoLed(Off);
        toggleIllumination(On);
        m_dBus->setBrightness(71);
        m_cookie = beginSuppressingScreenPowerManagement(m_profile);
    } else if (profile == "ECO") {
        showWidget(ECO);
        toggleEcoLed(On);
        toggleIllumination(Off);
        m_dBus->setBrightness(57);
        toggleKBDBacklight(Off);
    }
    kDebug() << "Changed battery profile to: " << m_profile;

    if (m_cookie != 0)
        if (stopSuppressingScreenPowerManagement(m_cookie))
            m_cookie = 0;
}

void FnActions::toggleTouchPad()
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.toggletouchpad");
    action.setHelperID(HELPER_ID);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed())
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.toggletouchpad failed";
}

void FnActions::kbdBacklight()
{
    if (getKBDMode() != AutoMode) {
        kWarning() << "Keyboard backlight timeout can't be changed in this mode";
        return;
    }

    int time = getKBDTimeout();

    QString format = QString("<font color='grey'><b>%1</b></font>");
    m_statusWidget.kbdAutoTimeLabel->setText(format.arg(time));
    showWidget(KBDAuto);
}

void FnActions::toggleIllumination(bool on)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.setillumination");
    action.setHelperID(HELPER_ID);
    action.addArgument("state", (on ? 1 : 0));
    KAuth::ActionReply reply = action.execute();
    if (reply.failed())
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.setillumination failed";
}

void FnActions::toggleEcoLed(bool on)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.seteco");
    action.setHelperID(HELPER_ID);
    action.addArgument("state", (on ? 1 : 0));
    KAuth::ActionReply reply = action.execute();
    if (reply.failed())
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.seteco failed";
}

void FnActions::toggleKBDBacklight(bool on)
{
    if (getKBDMode() == AutoMode)
        m_dBus->setKBDBacklight(on);
}

int FnActions::getKBDMode()
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.kbdmode");
    action.setHelperID(HELPER_ID);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed())
        qWarning() << "net.sourceforge.ktoshiba.ktoshhelper.kbdmode failed";

    return reply.data()["mode"].toInt();
}

int FnActions::getKBDTimeout()
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.kbdtimeout");
    action.setHelperID(HELPER_ID);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed())
        qWarning() << "net.sourceforge.ktoshiba.ktoshhelper.kbdtimeout failed";

    return reply.data()["time"].toInt();
}

void FnActions::changeKBDMode()
{
    int mode = getKBDMode();
    if (!mode)
        return;
    
    mode = (mode == FNZMode) ? AutoMode : FNZMode;

    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.setkbdmode");
    action.setHelperID(HELPER_ID);
    action.addArgument("mode", mode);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed())
        qWarning() << "net.sourceforge.ktoshiba.ktoshhelper.setkbdmode failed";
}

void FnActions::changeKBDTimeout(int time)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.setkbdtimeout");
    action.setHelperID(HELPER_ID);
    action.addArgument("time", time);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed())
        qWarning() << "net.sourceforge.ktoshiba.ktoshhelper.setkbdtimeout failed";
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
        toggleProfiles();
        break;
    case KEY_TOUCHPAD_TOGGLE: 
        toggleTouchPad();
        break;
    case KEY_KBDILLUMTOGGLE:
        kbdBacklight();
        break;
    case KEY_ZOOMRESET:
        m_dBus->setZoom( ZoomReset );
        break;
    case KEY_ZOOMOUT:
        m_dBus->setZoom( ZoomOut );
        break;
    case KEY_ZOOMIN:
        m_dBus->setZoom( ZoomIn );
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
