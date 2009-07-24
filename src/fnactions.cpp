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

#include <unistd.h>

#include <QtGui/QDesktopWidget>

#include <KDebug>
#include <KMessageBox>
#include <KLocale>

#include "fnactions.h"
#include "ktoshibadbusinterface.h"

FnActions::FnActions(QObject *parent)
    : QObject( parent ),
      //widget( new QWidget( 0, Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint ) ),
      m_dBus( new KToshibaDBusInterface( parent ) ),
      m_bright( -1 )
{
    //m_statusWidget.setupUi( widget );
    //setMainWidget( widget );
    //widget->clearFocus();

    // ISSUE: Internal brightness value, since HAL doesn't seem to
    // update the brightness value unless AC adaptor is plug/unpluged,
    // so we will rely on the value stored to show widgets... Bummer...
    m_bright = m_dBus->getBrightness();

    connect( m_dBus, SIGNAL( hotkeyPressed(QString) ), this, SLOT( slotGotHotkey(QString) ) );
}

FnActions::~FnActions()
{
    //delete widget; widget = NULL;
    delete m_dBus; m_dBus = NULL;
}

QString FnActions::modelName()
{
    return m_dBus->getModel();
}

//void FnActions::showWidget()
//{
    //QRect r = QApplication::desktop()->geometry();
    //widget->move(r.center() -
    //            QPoint(widget->width() / 2, widget->height() / 2));
    //widget->show();
    //m_statusWidget.stackedWidget->raiseWidget(3);
    //sleep(3);	// Sleep for three seconds
    //widget->hide();
//}

void FnActions::slotGotHotkey(QString hotkey)
{
    // ISSUE: Fn press/release is not being sent by the drivers or HAL
    // which could be of great use here, eg:
    // Fn-Pressed -> widget->show();
    // Hotkey-Pressed -> widget->stackedWidget.raiseWidget(--something--);
    // Fn-Released -> widget->hide();

    // TODO: Fn-F{Esc, 1, 2, 3, 4, 5, 6, 7, 8, 9} are here,
    // some (most?) lack implementation...
    if (hotkey == "mute") {
        // ISSUE: KMix or some other app is showing a volume bar whenever
        // Fn-Esc is pressed, so this call is here just in case...
        //m_dBus->checkMute();
        //showWidget();
        return;
    } else
    if (hotkey == "coffee") {
        m_dBus->lockScreen();
        return;
    } else
    if (hotkey == "sleep") {
        m_dBus->suspend();
        return;
    } else
    if (hotkey == "hibernate") {
        m_dBus->hibernate();
        return;
    } else
    if (hotkey == "switch-videomode") {
        //showWidget();
        return;
    } else
    if (hotkey == "brightness-down" ||
        hotkey == "brightness-up") {
        //showWidget();
        return;
    } else
    if (hotkey == "wlan") {
        m_dBus->toggleWireless();
        return;
    } else
    if (hotkey == "prog1") {
        // Do nothing for the time being...
        return;
    } else {
        // TODO: This will be gone in a not so distant future, or maybe
        // left here (commented) to test incomming "events"
        KMessageBox::information(0, i18n("Got Hotkey: %1").arg(hotkey),
				 i18n("KToshiba - Hotkey"));
    }
}


#include "fnactions.moc"
