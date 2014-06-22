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
#include <QFile>

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
      m_keyHandler( new KToshibaKeyHandler( parent ) ),
      m_widget( new QWidget( 0, Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint ) ),
      m_fnPressed( false ),
      m_cookie( 0 )
{
    m_statusWidget.setupUi( m_widget );
    m_widget->clearFocus();
    // Only enable Translucency if composite is enabled
    // otherwise an ugly black background will appear
    m_widget->setAttribute( Qt::WA_TranslucentBackground, KWindowSystem::compositingActive() );

    m_device = findDevicePath();
    if (m_device.isEmpty()) {
        //parent->quit();
        exit(-1);
    }

    // We're just going to care about these profiles
    profiles << "Performance" << "Presentation" << "ECO" << "Powersave";

    QList<Solid::Device> list = Solid::Device::listFromType(Solid::DeviceInterface::AcAdapter, QString());
    Solid::Device device = list[0];
    Solid::AcAdapter *acAdapter = device.as<Solid::AcAdapter>();
    m_profile = (acAdapter->isPlugged()) ? QString("Performance") : QString("Powersave");
    changeProfile(m_profile);

    connect( m_keyHandler, SIGNAL( hotkeyPressed(int) ), this, SLOT( processHotkey(int) ) );
    connect( acAdapter, SIGNAL( plugStateChanged(bool, QString) ), this, SLOT( acAdapterChanged(bool) ) );
    connect( KWindowSystem::self(), SIGNAL( compositingChanged(bool) ), this, SLOT( compositingChanged(bool) ) );
}

FnActions::~FnActions()
{
    delete m_widget; m_widget = NULL;
    delete m_keyHandler; m_keyHandler = NULL;
    delete m_dBus; m_dBus = NULL;
}

QString FnActions::findDevicePath()
{
    QFile file("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS1900:00/path");
    if (file.exists()) {
        kDebug() << "Found interface TOS1900" << endl;
        return QString("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS1900:00/");
    }

    file.setFileName("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS6200:00/path");
    if (file.exists()) {
        kDebug() << "Found interface TOS6200" << endl;
        return QString("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS6200:00/");
    }

    file.setFileName("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS6208:00/path");
    if (file.exists()) {
        kDebug() << "Found interface TOS6208" << endl;
        return QString("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS6208:00/");
    }

    kError() << "No known interface found" << endl;

    return QString("");
}

void FnActions::acAdapterChanged(bool state)
{
    if (m_profile == "Powersave" && state) {
        m_profile = "Performance";
        changeProfile(m_profile);
        return;
    }

    if (m_profile == "Performance" && !state) {
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
    int current = profiles.indexOf(m_profile);
    if (current == profiles.indexOf(profiles.last())) {
        m_profile = profiles.first();
    } else {
        current++;
        m_profile = profiles.at(current);
    }

    changeProfile(m_profile);
}

void FnActions::changeProfile(QString profile)
{
    if (profile == "Powersave") {
        showWidget(Powersave);
        toggleEcoLed(Off);
        toggleIllumination(Off);
        screenBrightness(3);
        if (getKBDMode() == 2)
            toggleKBDBacklight(Off);
    } else if (profile == "Performance") {
        showWidget(Performance);
        toggleEcoLed(Off);
        toggleIllumination(On);
        screenBrightness(7);
    } else if (profile == "Presentation") {
        showWidget(Presentation);
        toggleEcoLed(Off);
        toggleIllumination(On);
        screenBrightness(5);
        m_cookie = beginSuppressingScreenPowerManagement(m_profile);
    } else if (profile == "ECO") {
        showWidget(ECO);
        toggleEcoLed(On);
        toggleIllumination(Off);
        screenBrightness(4);
        if (getKBDMode() == 2)
            toggleKBDBacklight(Off);
    }
    kDebug() << "Changed battery profile to: " << m_profile;

    if (m_cookie != 0)
        if (stopSuppressingScreenPowerManagement(m_cookie))
            m_cookie = 0;
}

void FnActions::screenBrightness(int level)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.screenbrightness");
    action.setHelperID(HELPER_ID);
    action.addArgument("level", level);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed())
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.screenbrightness failed";
}

void FnActions::toggleTouchPad()
{
    QFile file(this);

    file.setFileName(m_device + "touchpad");
    if (!file.exists()) {
        kWarning() << "Could not locate TouchPad file." << endl;
        return;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        kError() << "Could not open TouchPad file for reading: " << file.error();
        return;
    }

    QTextStream stream(&file);
    int state = stream.readAll().toInt();
    file.close();

    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.toggletouchpad");
    action.setHelperID(HELPER_ID);
    action.addArgument("state", (state ? 0 : 1));
    KAuth::ActionReply reply = action.execute();
    if (reply.failed())
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.toggletouchpad failed";
}

void FnActions::kbdBacklight()
{
    if (getKBDMode() != 2) {
        kWarning() << "Keyboard backlight timeout can't be changed in this mode";
        return;
    }

    QFile file(m_device + "kbd_backlight_timeout");
    if (!file.exists()) {
        kWarning() << "Could not locate KBD backlight timeout device.";
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        kError() << "Could not open KBD backlight timeout file for reading: " << file.error();
        return;
    }

    QTextStream stream(&file);
    int time = stream.readAll().toInt();
    file.close();

    QString format = QString("<font color='grey'><b>%1</b></font>");
    m_statusWidget.kbdAutoTimeLabel->setText(format.arg(time));
    showWidget(KBDAuto);
}

void FnActions::toggleIllumination(bool on)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.illumination");
    action.setHelperID(HELPER_ID);
    action.addArgument("state", (on ? 1 : 0));
    KAuth::ActionReply reply = action.execute();
    if (reply.failed())
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.illumination failed";
}

void FnActions::toggleEcoLed(bool on)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.eco");
    action.setHelperID(HELPER_ID);
    action.addArgument("state", (on ? 1 : 0));
    KAuth::ActionReply reply = action.execute();
    if (reply.failed())
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.eco failed";
}

void FnActions::toggleKBDBacklight(bool on)
{
	// TODO: Implement me...
}

int FnActions::getKBDMode()
{
    QFile file(m_device + "kbd_backlight_mode");
    if (!file.exists()) {
        kWarning() << "Could not locate KBD backlight mode device.";
        return 0;
    }
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        kError() << "Could not open KBD backlight mode file for reading: " << file.error();
        return 0;
    }

    QTextStream stream(&file);
    int mode = stream.readAll().toInt();
    file.close();

    return mode;
}

int FnActions::getKBDTimeout()
{
    QFile file(m_device + "kbd_backlight_timeout");
    if (!file.exists()) {
        kWarning() << "Could not locate KBD backlight timeout device.";
        return 0;
    }
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        kError() << "Could not open KBD backlight timeout file for reading: " << file.error();
        return 0;
    }

    QTextStream stream(&file);
    int time = stream.readAll().toInt();
    file.close();

    return time;
}

void FnActions::changeKBDMode()
{
    int mode = (getKBDMode() == 1 ? 2 : 1);

    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.kbdmode");
    action.setHelperID(HELPER_ID);
    action.addArgument("mode", mode);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed())
        qWarning() << "net.sourceforge.ktoshiba.ktoshhelper.kbdmode failed";
}

void FnActions::changeKBDTimeout(int time)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.kbdtimeout");
    action.setHelperID(HELPER_ID);
    action.addArgument("time", time);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed())
        qWarning() << "net.sourceforge.ktoshiba.ktoshhelper.kbdtimeout failed";
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

    m_widget->hide();
}

void FnActions::slotGotHotkey(int hotkey)
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
