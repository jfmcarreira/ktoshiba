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

#include <KDebug>
#include <KMessageBox>
#include <KLocale>

#include "fnactions.h"
#include "ktoshibadbusinterface.h"

FnActions::FnActions(QObject *parent)
    : QObject( parent ),
      m_dBus( new KToshibaDBusInterface( parent ) )
{
    connect( m_dBus, SIGNAL( hotkeyPressed(QString) ), this, SLOT( slotGotHotkey(QString) ) );
}

FnActions::~FnActions()
{
    delete m_dBus; m_dBus = NULL;
}

void FnActions::slotGotHotkey(QString hotkey)
{
    // TODO: Add more Fn actions here...
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
    if (hotkey == "wlan") {
        m_dBus->toggleWireless();
        return;
    } else {
        KMessageBox::information(0, i18n("Got Hotkey: %1").arg(hotkey),
				 i18n("KToshiba - Hotkey"));
    }
}


#include "fnactions.moc"
