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
      m_wireless( true )
{
    m_statusWidget.setupUi( widget );
    widget->clearFocus();
    widget->setAttribute( Qt::WA_TranslucentBackground );

    populateHotkeys();

    // We're just going to care about these profiles
    profiles << "Performance" << "Powersave" << "Presentation";

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

void FnActions::populateHotkeys()
{
    // TODO: Add and/or correct values if needed
    // Fn-F# values
    hotkeys.insert("mute", 0);
    hotkeys.insert("coffee", 1);
    hotkeys.insert("battery", 2);
    hotkeys.insert("sleep", 3);
    hotkeys.insert("hibernate", 4);
    hotkeys.insert("switch-videomode", 5);
    hotkeys.insert("brightness-down", 6);
    hotkeys.insert("brightness-up", 7);
    hotkeys.insert("wlan", 8);
    hotkeys.insert("prog1", 9);
    // Multimedia values
    hotkeys.insert("homepage", 10);
    hotkeys.insert("media-player", 11);
    hotkeys.insert("play-pause", 12);
    hotkeys.insert("stop-cd", 13);
    hotkeys.insert("previous-song", 14);
    hotkeys.insert("next-song", 15);
    // Other values (Zoom and friends)
    hotkeys.insert("zoom-reset", 16);
    hotkeys.insert("zoom-out", 17);
    hotkeys.insert("zoom-in", 18);
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

    // TODO: Fn-F5 and Fn-F9 lack implementation...
    switch ( hotkeys.value(hotkey) ) {
        case 0:
          // ISSUE: KMix or some other app is showing a volume bar whenever
          // Fn-Esc is pressed, so this is here just in case...
          //showWidget();
          break;
        case 1:
          m_dBus->lockScreen();
          break;
        case 2:
          toggleProfiles();
          break;
        case 3:
          m_dBus->suspend();
          break;
        case 4:
          m_dBus->hibernate();
          break;
        case 5:
          // Do nothing for the time being...
          //showWidget();
          break;
        case 6:
        case 7:
          if (hotkey == "brightness-down" && m_bright > 0)
              m_bright--;
          else if (m_bright < 7)
              m_bright++;
          showWidget(m_bright);
          break;
        case 8:
          toggleWireless();
          break;
        case 9:
          // ISSUE: Ugly, why do X.Org devs have to change
          // SynapticsSHM struct everytime...?
          // Can't attach to SHM... Bummer...
          //showWidget();
          break;
        // Multimedia Keys
        case 10:
          break;
        case 11:
          m_dBus->launchMediaPlayer();
          break;
        case 12:
          m_dBus->playerPlayPause();
          break;
        case 13:
          m_dBus->playerStop();
          break;
        case 14:
          m_dBus->playerPrevious();
          break;
        case 15:
          m_dBus->playerNext();
          break;
        // Zoom et. al.
        case 16:
          break;
        case 17:
          break;
        case 18:
          break;
        default:
          // TODO: This will be gone in a not so distant future, or maybe
          // left here (commented) to test incomming "events"
          KMessageBox::information(0, i18n("Got Hotkey: %1").arg(hotkey),
                                   i18n("KToshiba - Hotkey"));
    }
}


#include "fnactions.moc"
