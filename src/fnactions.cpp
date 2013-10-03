/*
   Copyright (C) 2004-2011  Azael Avalos <coproscefalo@gmail.com>

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
#include <QRect>

#include <KMessageBox>
#include <KLocale>
#include <KDebug>
#include <KToolInvocation>
#include <KProcess>
#include <solid/device.h>
#include <solid/acadapter.h>
#include <solid/powermanagement.h>
#include <solid/controlnm09/networkmanager.h>

extern "C" {
#include <linux/input.h>
}

#include "fnactions.h"
#include "ktoshibadbusinterface.h"
#include "ktoshibakeyhandler.h"

using namespace Solid::PowerManagement;

FnActions::FnActions(QObject *parent)
    : QObject( parent ),
      m_dBus( new KToshibaDBusInterface( parent ) ),
      m_keyHandler( new KToshibaKeyHandler( parent ) ),
      widget( new QWidget( 0, Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint ) ),
      m_wireless( true ),
      m_fnPressed( false ),
      m_cookie( 0 )
{
    m_statusWidget.setupUi( widget );
    widget->clearFocus();
    // Only enable Translucency if composite is enabled
    // otherwise an ugly black background will appear
    if (m_dBus->checkCompositeStatus())
        widget->setAttribute( Qt::WA_TranslucentBackground );

    // We're just going to care about these profiles
    profiles << "Performance" << "Presentation" << "ECO" << "Powersave";

    QList<Solid::Device> list = Solid::Device::listFromType(Solid::DeviceInterface::AcAdapter, QString());
    Solid::Device device = list[0];
    Solid::AcAdapter *acAdapter = device.as<Solid::AcAdapter>();
    m_profile = (acAdapter->isPlugged()) ? QString("Performance") : QString("Powersave");

    Solid::Control::NetworkManagerNm09::Notifier *wifiNotifier = Solid::Control::NetworkManagerNm09::notifier();

    connect( wifiNotifier, SIGNAL( wirelessEnabledChanged(bool) ), this, SLOT( wirelessChanged(bool) ) );
    connect( m_keyHandler, SIGNAL( hotkeyPressed(int) ), this, SLOT( slotGotHotkey(int) ) );
}

FnActions::~FnActions()
{
    delete widget; widget = NULL;
    delete m_dBus; m_dBus = NULL;
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

    KProcess process(this);
    if (m_profile == "Powersave") {
        showWidget(Powersave);
        toggleEcoLed(false);
        toggleIllumination(false);
        process << "sudo" << "ktoshhelper" << "brightness" << "3";
    } else if (m_profile == "Performance") {
        showWidget(Performance);
        toggleEcoLed(false);
        toggleIllumination(true);
        process << "sudo" << "ktoshhelper" << "brightness" << "7";
    } else if (m_profile == "Presentation") {
        showWidget(Presentation);
        toggleEcoLed(false);
        toggleIllumination(true);
        m_cookie = beginSuppressingScreenPowerManagement(m_profile);
        process << "sudo" << "ktoshhelper" << "brightness" << "5";
    } else if (m_profile == "ECO") {
        showWidget(ECO);
        toggleEcoLed(true);
        toggleIllumination(false);
        process << "sudo" << "ktoshhelper" << "brightness" << "4";
    }
    process.startDetached();

    if (m_cookie != 0)
        if (stopSuppressingScreenPowerManagement(m_cookie))
            m_cookie = 0;
}

void FnActions::brightness(int level)
{
    QFile file(this);
    QString root = QString("/sys/class/backlight/");

    file.setFileName(root + "acpi_video0/actual_brightness");
    if (!file.exists()) {
        file.setFileName(root + "toshiba/actual_brightness");
        if (!file.exists()) {
            kError() << "Could not locate Backlight device." << endl;
            return;
        }
    }
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        kError() << "Could not open Backlight for reading: " << file.error() << endl;
        return;
    }

    QTextStream stream(&file);
    int bright = stream.readAll().toInt();
    file.close();

    if ((bright == 7 && level == 1) || (bright == 0 && level == -1))
        return;

    KProcess process(this);
    QString str;
    process << "sudo" << "ktoshhelper" << "brightness" << str.setNum(bright + level);
    process.startDetached();
}

void FnActions::wirelessChanged(bool state)
{
    m_wireless = state;
}

void FnActions::toggleTouchPad()
{
    QFile file(this);

    file.setFileName("/sys/devices/platform/toshiba/touchpad");
    if (!file.exists()) {
        kError() << "Could not locate TouchPad device." << endl;
        return;
    }
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        kError() << "Could not open TouchPad for reading: " << file.error() << endl;
        return;
    }

    QTextStream stream(&file);
    int tpad = stream.readAll().toInt();
    file.close();

    KProcess process(this);
    if (tpad) {
        process << "sudo" << "ktoshhelper" << "touchpad" << "0";
        showWidget(TPOff);
    } else if (!tpad) {
        process << "sudo" << "ktoshhelper" << "touchpad" << "1";
        showWidget(TPOn);
    }
    process.startDetached();
}

void FnActions::toggleIllumination(bool on)
{
    QFile file(this);

    file.setFileName("/sys/class/leds/toshiba::illumination/brightness");
    if (!file.exists()) {
        kError() << "Could not locate Illumination led device." << endl;
        return;
    }
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        kError() << "Could not open KBD led for reading: " << file.error() << endl;
        return;
    }

    QTextStream stream(&file);
    int illum_led = stream.readAll().toInt();
    file.close();

    KProcess process(this);
    if (on && !illum_led) {
        process << "sudo" << "ktoshhelper" << "illumination" << "1";
    } else if (!on && illum_led) {
        process << "sudo" << "ktoshhelper" << "illumination" << "0";
    }
    process.startDetached();
}

void FnActions::toggleEcoLed(bool on)
{
    QFile file(this);

    file.setFileName("/sys/class/leds/toshiba::eco_mode/brightness");
    if (!file.exists()) {
        kError() << "Could not locate ECO led device." << endl;
        return;
    }
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        kError() << "Could not open ECO led for reading: " << file.error() << endl;
        return;
    }

    QTextStream stream(&file);
    int eco_led = stream.readAll().toInt();
    file.close();

    KProcess process(this);
    if (on && !eco_led) {
        process << "sudo" << "ktoshhelper" << "eco" << "1";
    } else if (!on && eco_led) {
        process << "sudo" << "ktoshhelper" << "eco" << "0";
    }
    process.startDetached();
}

void FnActions::toggleKBDIllumination()
{
    QFile file(this);
    int mode, time, backlight;

    file.setFileName("/sys/devices/platform/toshiba/kbd_illumination");
    if (!file.exists()) {
        kError() << "Could not locate KBD illumination device." << endl;
        return;
    }
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        kError() << "Could not open KBD led for reading: " << file.error() << endl;
        return;
    }

    QTextStream stream(&file);
    int lines = 2;
    mode = time = backlight = -1;
    while (lines) {
        QString line = stream.readLine(12);
        QStringList split = line.split(":");
        if (split[0] == "mode")
            mode = split[1].toInt();
        else if (split[0] == "time")
            time = split[1].toInt();
        else if (split[0] == "backlight")
            backlight = split[1].toInt();
        lines--;
    }
    file.close();

    if (mode == 2) {
        QString format = QString("<font color='grey'><b>%1</b></font>");
        m_statusWidget.kbdAutoTimeLabel->setText(format.arg(time));
        showWidget(KBDAuto);
    } else if (mode == 1) {
        KProcess process(this);
        if (backlight) {
            process << "sudo" << "ktoshhelper" << "kbdbacklight" << "0";
            showWidget(KBDOff);
        } else if (!backlight) {
            process << "sudo" << "ktoshhelper" << "kbdbacklight" << "1";
            showWidget(KBDOn);
        }
        process.startDetached();
    }
}

void FnActions::showWidget(int wid)
{
    QRect r = QApplication::desktop()->geometry();
    widget->move(r.center() -
                QPoint(widget->width() / 2, widget->height() / 2));
    widget->show();

    if (wid < 0)
        m_statusWidget.stackedWidget->setCurrentWidget( m_statusWidget.stackedWidget->widget(Disabled) );
    else
        m_statusWidget.stackedWidget->setCurrentWidget( m_statusWidget.stackedWidget->widget(wid) );
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
    case KEY_BRIGHTNESSDOWN:
        brightness(-1);
        break;
    case KEY_BRIGHTNESSUP:
        brightness(1);
        break;
    case KEY_WLAN:
        m_wireless ? showWidget(WifiOff) : showWidget(WifiOn) ;
        break;
    case KEY_TOUCHPAD_TOGGLE: 
        toggleTouchPad();
        break;
    case KEY_KBDILLUMTOGGLE:
        toggleKBDIllumination();
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
    case KEY_FN:
        m_fnPressed = m_fnPressed ? false : true;
        if (m_fnPressed)
            showWidget(Blank);
        else
            widget->hide();
        break;
    }
}


#include "fnactions.moc"
