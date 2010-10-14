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

#include <config-ktoshiba.h>

#include <QtGui/QDesktopWidget>
#include <QTimer>
#include <QRect>
#include <QApplication>

#include <KMessageBox>
#include <KLocale>
#include <KJob>
#include <KProcess>
#include <solid/control/networkmanager.h>

#include "fnactions.h"
#include "ktoshibadbusinterface.h"
#ifdef ENABLE_TOUCHPAD_FUNCTIONALITY
#include "touchpad.h"
#endif

FnActions::FnActions(QObject *parent)
    : QObject( parent ),
      m_dBus( new KToshibaDBusInterface( parent ) ),
      widget( new QWidget( 0, Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint ) ),
      m_widgetTimer( new QTimer( this ) ),
      m_profile( "Powersave" ),
      m_wireless( true ),
      m_touchpadError( -1 )
{
    m_statusWidget.setupUi( widget );
    widget->clearFocus();
    // Only enable Translucency if composite is enabled
    // otherwise an ugly black background will appear
    if (m_dBus->m_compositeEnabled)
        widget->setAttribute( Qt::WA_TranslucentBackground );

    populateHotkeys();

    // We're just going to care about these profiles
    profiles << "Performance" << "Powersave" << "Presentation";

    Solid::Control::NetworkManager::Notifier *wifiNotifier = Solid::Control::NetworkManager::notifier();

#ifdef ENABLE_TOUCHPAD_FUNCTIONALITY
    m_TouchPad = new TouchPad();
    m_touchpadError = m_TouchPad->init();
#endif

    connect( m_dBus, SIGNAL( hotkeyPressed(QString) ), this, SLOT( slotGotHotkey(QString) ) );
    connect( m_widgetTimer, SIGNAL( timeout() ), this, SLOT( hideWidget() ) );
    connect( wifiNotifier, SIGNAL( wirelessEnabledChanged(bool) ), this, SLOT( wirelessChanged(bool) ) );
    connect( m_dBus, SIGNAL( profileChanged(QString, QStringList) ), this, SLOT( slotProfileChanged(QString, QStringList) ) );
    connect( parent, SIGNAL( mediaPlayerChanged(int) ), this, SLOT( updateMediaPlayer(int) ) );
}

FnActions::~FnActions()
{
    delete widget; widget = NULL;
    delete m_widgetTimer; m_widgetTimer = NULL;
#ifdef ENABLE_TOUCHPAD_FUNCTIONALITY
    delete m_TouchPad; m_TouchPad = NULL;
#endif
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
    hotkeys.insert("media", 11);
    hotkeys.insert("play-pause", 12);
    hotkeys.insert("stop-cd", 13);
    hotkeys.insert("previous-song", 14);
    hotkeys.insert("next-song", 15);
    // Other values (Zoom and friends)
    hotkeys.insert("zoomreset", 16);
    hotkeys.insert("zoomout", 17);
    hotkeys.insert("zoomin", 18);
    hotkeys.insert("prog2", 19);
}

QString FnActions::modelName()
{
    return m_dBus->getModel();
}

void FnActions::slotProfileChanged(QString profile, QStringList profiles)
{
    m_profile = profile;

    // Avoid compiler warning
    if (profiles.contains(profile)) {}
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

void FnActions::toggleTouchPad()
{
#ifdef ENABLE_TOUCHPAD_FUNCTIONALITY
    int tp = -1;
    if (m_touchpadError == TouchPad::NoError && (tp = m_TouchPad->getTouchPad()) != -1) {
        //tp = (tp)? TouchPad::On : TouchPad::Off;
        m_TouchPad->setTouchPad( !tp );
        showWidget( ((tp)? TPOff : TPOn) );
        //m_TouchPad->m_touchpad = tp;
    } else
#endif
        showWidget(TPOff);
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
        KProcess::startDetached("amarok");
    else if (m_dBus->m_mediaPlayer == KToshibaDBusInterface::Kaffeine)
        KProcess::startDetached("kaffeine");
    else
        KProcess::startDetached("juk");
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
        case KToshibaDBusInterface::Amarok:
            player = KToshibaDBusInterface::Kaffeine;
            wid = Kaffeine; break;
        case KToshibaDBusInterface::Kaffeine:
            player = KToshibaDBusInterface::JuK;
            wid = JuK; break;
        case KToshibaDBusInterface::JuK:
            player = KToshibaDBusInterface::Amarok;
            wid = Amarok; break;
    }

    m_dBus->m_mediaPlayer = player;
    showWidget(wid);
    emit mediaPlayerChanged(m_dBus->m_mediaPlayer);
}

void FnActions::slotGotHotkey(QString hotkey)
{
    // ISSUE: Fn press/release is not being sent by the drivers or HAL
    // which could be of great use here, eg:
    // Fn-Pressed:	widget->show();
    // Hotkey-Pressed:	showWidget(--something--) );
    // Fn-Released:	widget->hide();

    // TODO: Fn-F5 lack implementation...
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
          // Do nothing, since KDE dies it now for us  
          break;
        case 8:
          toggleWireless();
          break;
        case 9:
          toggleTouchPad();
          break;
        // Multimedia Keys
        case 10:
          break;
        case 11:
          launchMediaPlayer();
          break;
        case 12:
          mediaAction(PlayPause);
          break;
        case 13:
          mediaAction(Stop);
          break;
        case 14:
          mediaAction(Prev);
          break;
        case 15:
          mediaAction(Next);
          break;
        // Zoom et. al.
        case 16:
          break;
        case 17:
          break;
        case 18:
          break;
        case 19:
          changeMediaPlayer();
          break;
        default:
          // TODO: This will be gone in a not so distant future, or maybe
          // left here (commented) to test incomming "events"
          KMessageBox::information(0, i18n("Got Hotkey: %1").arg(hotkey),
                                   i18n("KToshiba - Hotkey"));
    }
}


#include "fnactions.moc"
