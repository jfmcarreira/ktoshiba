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
#include <QTimer>
#include <QRect>

#include <KMessageBox>
#include <KLocale>
#include <KDebug>
#include <KToolInvocation>
#include <solid/control/networkmanager.h>

#include "fnactions.h"
#include "ktoshibadbusinterface.h"

FnActions::FnActions(QObject *parent)
    : QObject( parent ),
      m_dBus( new KToshibaDBusInterface( parent ) ),
      widget( new QWidget( 0, Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint ) ),
      m_widgetTimer( new QTimer( this ) ),
      m_profile( "Powersave" ),
      m_wireless( true ),
      m_touchpad( true )
{
    m_statusWidget.setupUi( widget );
    widget->clearFocus();
    // Only enable Translucency if composite is enabled
    // otherwise an ugly black background will appear
    if (m_dBus->m_compositeEnabled)
        widget->setAttribute( Qt::WA_TranslucentBackground );

    // We're just going to care about these profiles
    profiles << "Performance" << "Powersave" << "Presentation";

    Solid::Control::NetworkManager::Notifier *wifiNotifier = Solid::Control::NetworkManager::notifier();

    connect( m_widgetTimer, SIGNAL( timeout() ), this, SLOT( hideWidget() ) );
    connect( wifiNotifier, SIGNAL( wirelessEnabledChanged(bool) ), this, SLOT( wirelessChanged(bool) ) );
    connect( m_dBus, SIGNAL( profileChanged(QString) ), this, SLOT( profileChanged(QString) ) );
    connect( m_dBus, SIGNAL( touchpadChanged(bool) ), this, SLOT( touchpadChanged(bool) ) );
    connect( parent, SIGNAL( mediaPlayerChanged(int) ), this, SLOT( updateMediaPlayer(int) ) );
    connect( parent, SIGNAL( hotkeyPressed(int) ), this, SLOT( slotGotHotkey(int) ) );
}

FnActions::~FnActions()
{
    delete widget; widget = NULL;
    delete m_widgetTimer; m_widgetTimer = NULL;
    delete m_dBus; m_dBus = NULL;
}

void FnActions::profileChanged(QString profile)
{
    m_profile = profile;
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

void FnActions::wirelessChanged(bool state)
{
    m_wireless = state;
}

void FnActions::toggleWireless()
{
    Solid::Control::NetworkManager::setWirelessEnabled( ((m_wireless)? false : true) );
}

void FnActions::touchpadChanged(bool state)
{
    m_touchpad = state;
}

void FnActions::toggleTouchPad()
{
    m_dBus->setTouchPad( ((m_touchpad)? false : true) );
    showWidget(((m_touchpad)? TPOff : TPOn));
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

    if (m_widgetTimer->isActive())
        m_widgetTimer->setInterval( 900 );
    else
        m_widgetTimer->start( 900 );
}

void FnActions::hideWidget()
{
    widget->hide();
}

void FnActions::updateMediaPlayer(int player)
{
    m_dBus->m_mediaPlayer = player;
    kDebug() << "Media Player changed" << endl;
}

int FnActions::showMessageBox()
{
    QString player;
    switch (m_dBus->m_mediaPlayer) {
        case KToshibaDBusInterface::Amarok:
            player = "Amarok";
            break;
        case KToshibaDBusInterface::Kaffeine:
            player = "Kaffeine";
            break;
        case KToshibaDBusInterface::JuK:
            player = "JuK";
            break;
    }
    return KMessageBox::questionYesNo(0, i18n("%1 is not running.\n"
                                      "Would you like to start it now?").arg(player),
                                      i18n("Media Player"),
                                      KStandardGuiItem::yes(),
                                      KStandardGuiItem::no(),
                                      "dontaskMediaPlayer");
}

void FnActions::launchMediaPlayer()
{
    if (m_dBus->m_mediaPlayer == KToshibaDBusInterface::Amarok)
        KToolInvocation::startServiceByDesktopName("amarok");
    else if (m_dBus->m_mediaPlayer == KToshibaDBusInterface::Kaffeine)
        KToolInvocation::startServiceByDesktopName("kaffeine");
    else if (m_dBus->m_mediaPlayer == KToshibaDBusInterface::JuK)
        KToolInvocation::startServiceByDesktopName("juk");
}

void FnActions::performAction()
{
    switch (m_action) {
        case PlayPause:
            m_dBus->playerPlayPause();
            break;
        case Stop:
            m_dBus->playerStop();
            break;
        case Prev:
            m_dBus->playerPrevious();
            break;
        case Next:
            m_dBus->playerNext();
            break;
    }
}

void FnActions::mediaAction(PlayerAction action)
{
    m_action = action;

    if (m_dBus->m_mediaPlayer == KToshibaDBusInterface::NoMP)
        return;

    if (m_dBus->checkMediaPlayer())
        performAction();
    else
    if (showMessageBox() == KMessageBox::Yes) {
        launchMediaPlayer();
        // 2 seconds to start, then send the requested action
        QTimer::singleShot( 2000, this, SLOT( performAction() ) );
    }
}

void FnActions::changeMediaPlayer()
{
    int player = -1, wid = -1;

    switch (m_dBus->m_mediaPlayer) {
        case KToshibaDBusInterface::NoMP:
            player = KToshibaDBusInterface::Amarok;
	    wid = Amarok; break;
        case KToshibaDBusInterface::Amarok:
            player = KToshibaDBusInterface::Kaffeine;
            wid = Kaffeine; break;
        case KToshibaDBusInterface::Kaffeine:
            player = KToshibaDBusInterface::JuK;
            wid = JuK; break;
        case KToshibaDBusInterface::JuK:
            player = KToshibaDBusInterface::NoMP;
            wid = Disabled; break;
    }

    m_dBus->m_mediaPlayer = player;
    showWidget(wid);
    emit mediaPlayerChanged(m_dBus->m_mediaPlayer);
}

void FnActions::slotGotHotkey(int hotkey)
{
    // ISSUE: Fn press/release is not being sent by the drivers or HAL
    // which could be of great use here, eg:
    // Fn-Pressed:	widget->show();
    // Hotkey-Pressed:	showWidget(--something--) );
    // Fn-Released:	widget->hide();

    switch ( hotkey ) {
        case 121: // Mute
          // ISSUE: KMix or some other app is showing a volume bar whenever
          // Fn-Esc is pressed, so this is here just in case...
          //showWidget();
          break;
        case 160: // Coffee
          m_dBus->lockScreen();
          break;
        case 244: // Battery
          toggleProfiles();
          break;
        case 150:
          m_dBus->suspend();
          break;
        case 213:
          m_dBus->hibernate();
          break;
        case 235: // Video-Out
          // Do nothing for the time being...
          //showWidget();
          break;
        case 232: // Brightness control
        case 233:
          // Do nothing, since KDE does it now for us  
          break;
        case 246: // WLAN
          toggleWireless();
          break;
        case 156: // Prog1
          toggleTouchPad();
          break;
        /*
         * Multimedia Keys
         */
        case 234:
          launchMediaPlayer();
          break;
        case 172:
          mediaAction(PlayPause);
          break;
        case 136:
          mediaAction(Stop);
          break;
        case 173:
          mediaAction(Prev);
          break;
        case 171:
          mediaAction(Next);
          break;
        /*
         * Misc
         */
        case 180: // Homepage
          KToolInvocation::invokeBrowser("", "");
          break;
        case 236: // KBD Illumination
          break;
        case 428: // Zoom Reset
        case 427: // Zoom Out
        case 426: // Zoom In
          break;
        case 157: // Prog2
          changeMediaPlayer();
          break;
    }
}


#include "fnactions.moc"
