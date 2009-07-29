/*
   Copyright (C) 2004-2009  Azael Avalos <coproscefalo@gmail.com>

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

#include <stdio.h>
#include <stdlib.h>

#include <QtGui/QDesktopWidget>
#include <QTimer>

#include <KDebug>
#include <KMessageBox>
#include <KLocale>
#include <KJob>
#include <solid/control/networkmanager.h>

#include "fnactions.h"
#include "ktoshibadbusinterface.h"

FnActions::FnActions(QObject *parent)
    : QObject( parent ),
      widget( new QWidget( 0, Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint ) ),
      m_widgetTimer( new QTimer( this ) ),
      m_dBus( new KToshibaDBusInterface( parent ) ),
      m_profile( "Powersave" ),
      m_bright( -1 ),
      m_str( false ),
      m_std( false ),
      m_wireless( true )
{
    m_statusWidget.setupUi( widget );
    widget->clearFocus();
    widget->setAttribute( Qt::WA_NoSystemBackground );
    widget->setAttribute( Qt::WA_TranslucentBackground );

    // We're just going to care about these profiles
    profiles << "Performance" << "Powersave" << "Presentation";
    checkSupportedSuspend();

    // ISSUE: Internal brightness value, since HAL doesn't seem to
    // update the brightness value unless AC adaptor is plug/unplugged,
    // so we will rely on the value stored to show widgets... Bummer...
    m_bright = m_dBus->getBrightness();

    Solid::Control::PowerManager::Notifier *powerNotifier = Solid::Control::PowerManager::notifier();
    Solid::Control::NetworkManager::Notifier *wifiNotifier = Solid::Control::NetworkManager::notifier();

    connect( m_dBus, SIGNAL( hotkeyPressed(QString) ), this, SLOT( slotGotHotkey(QString) ) );
    connect( m_widgetTimer, SIGNAL( timeout() ), this, SLOT( hideWidget() ) );
    connect( powerNotifier, SIGNAL( acAdapterStateChanged(int) ), this, SLOT( acChanged(int) ) );
    connect( wifiNotifier, SIGNAL( wirelessEnabledChanged(bool) ), this, SLOT( wirelessChanged(bool) ) );
    //connect( m_dBus, SIGNAL( profileChanged(QString, QStringList) ), this, SLOT( slotProfileChanged(QString, QStringList) ) );
}

FnActions::~FnActions()
{
    delete widget; widget = NULL;
    delete m_widgetTimer; m_widgetTimer = NULL;
    delete m_dBus; m_dBus = NULL;
}

QString FnActions::modelName()
{
    return m_dBus->getModel();
}

void FnActions::slotProfileChanged(QString profile, QStringList profiles)
{
    if (profile == "Performance" || profile == "Powersave" ||
	    profile == "Presentation")
        m_profile = profile;
}

void FnActions::checkSupportedSuspend()
{
    int suspend = Solid::Control::PowerManager::supportedSuspendMethods();

    m_str = (suspend & Solid::Control::PowerManager::ToRam)? true : false;
    m_std = (suspend & Solid::Control::PowerManager::ToDisk)? true : false;
}

void FnActions::suspend(Solid::Control::PowerManager::SuspendMethod state)
{
    KJob *job = Solid::Control::PowerManager::suspend(state);

    if ( !job->exec() ) {
        fprintf(stderr, "suspend Error: Could not suspend to %s.\n\
			Bummer...", ((state == 2)? "ram" :"disk"));
    }
}

void FnActions::acChanged(int state)
{
    // Slight delay (1 sec) to wait for HAL to update its internal value...
    if (state == Solid::Control::PowerManager::Plugged ||
	state == Solid::Control::PowerManager::Unplugged)
        QTimer::singleShot( 1000, this, SLOT( updateBrightness() ) );
}

void FnActions::toggleProfiles()
{
    int current = profiles.indexOf(m_profile);
    if (current == profiles.indexOf(profiles.last()))
        m_profile = profiles.first();
    else {
        current++;
        m_profile = profiles.at(current);
    }

    m_dBus->setProfile(m_profile);
}

void FnActions::updateBrightness()
{
    m_bright = m_dBus->getBrightness();
}

void FnActions::wirelessChanged(bool state)
{
    m_wireless = state;
}

void FnActions::toggleWireless()
{
    Solid::Control::NetworkManager::setWirelessEnabled( ((m_wireless)? false : true) );
}

void FnActions::showWidget(int wid)
{
    QRect r = QApplication::desktop()->geometry();
    widget->move(r.center() -
                QPoint(widget->width() / 2, widget->height() / 2));
    widget->show();

    if (m_bright == -1 || wid < 0)
        m_statusWidget.stackedWidget->setCurrentWidget( m_statusWidget.stackedWidget->widget(8) );
    else
        m_statusWidget.stackedWidget->setCurrentWidget( m_statusWidget.stackedWidget->widget(wid) );

    if (m_widgetTimer->isActive())
        m_widgetTimer->setInterval( 900 );
    else
        m_widgetTimer->start( 900 );
}

void FnActions::hideWidget()
{
    widget->hide();
}

void FnActions::slotGotHotkey(QString hotkey)
{
    // ISSUE: Fn press/release is not being sent by the drivers or HAL
    // which could be of great use here, eg:
    // Fn-Pressed:	widget->show();
    // Hotkey-Pressed:	showWidget(--something--) );
    // Fn-Released:	widget->hide();

    // TODO: Fn-F2, Fn-F5 and Fn-F9 lack implementation...
    if (hotkey == "mute") {
        // ISSUE: KMix or some other app is showing a volume bar whenever
        // Fn-Esc is pressed, so this is here just in case...
        //showWidget();
        return;
    } else
    if (hotkey == "coffee") {
        m_dBus->lockScreen();
        return;
    } else
    if (hotkey == "battery") {
        toggleProfiles();
        // ISSUE: PowerDevil shows a notification whenever
        // a profile is changed, so, do we need the UI?
        //showWidget();
        return;
    } else
    if (hotkey == "sleep") {
        if (m_str)
            suspend(Solid::Control::PowerManager::ToRam);
        return;
    } else
    if (hotkey == "hibernate") {
        if (m_std)
            suspend(Solid::Control::PowerManager::ToDisk);
        return;
    } else
    if (hotkey == "switch-videomode") {
        // Do nothing for the time being...
        //showWidget();
        return;
    } else
    if (hotkey == "brightness-down" ||
        hotkey == "brightness-up") {
        if (hotkey == "brightness-down" && m_bright > 0)
            m_bright--;
        else if (m_bright < 7)
            m_bright++;
        showWidget(m_bright);
        return;
    } else
    if (hotkey == "wlan") {
        toggleWireless();
        return;
    } else
    if (hotkey == "prog1") {
        // ISSUE: Ugly, why do X.Org devs have to change
        // SynapticsSHM struct everytime...?
        // Can't attach to SHM... Bummer...
        //showWidget();
        return;
    } else
    // Multimedia Keys
    if (hotkey == "play-pause") {
        // Do nothing for the time being...
        //showWidget();
        return;
    } else
    if (hotkey == "stop-cd") {
        // Do nothing for the time being...
        //showWidget();
        return;
    } else
    if (hotkey == "previous-song") {
        // Do nothing for the time being...
        //showWidget();
        return;
    } else
    if (hotkey == "next-song") {
        // Do nothing for the time being...
        //showWidget();
        return;
    } else
    if (hotkey == "volume-up") {
        // Do nothing for the time being...
        //showWidget();
        return;
    } else
    if (hotkey == "volume-down") {
        // Do nothing for the time being...
        //showWidget();
        return;
    } else {
        // TODO: This will be gone in a not so distant future, or maybe
        // left here (commented) to test incomming "events"
        KMessageBox::information(0, i18n("Got Hotkey: %1").arg(hotkey),
				 i18n("KToshiba - Hotkey"));
    }
}


#include "fnactions.moc"
