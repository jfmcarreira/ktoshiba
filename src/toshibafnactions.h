/***************************************************************************
 *   Copyright (C) 2004-2006 by Azael Avalos                               *
 *   coproscefalo@gmail.com                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef TOSHIBA_FN_ACTIONS_H
#define TOSHIBA_FN_ACTIONS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qobject.h>

class QWidget;

class KToshibaSMMInterface;
class SettingsWidget;
class StatusWidget;
class Suspend;

/**
 * @short Performs the Fn assosiated action
 * @author Azael Avalos <coproscefalo@gmail.com>
 * @version 0.2
 */
class ToshibaFnActions : public QObject
{
    Q_OBJECT
public:
    ToshibaFnActions(QWidget *parent = 0);
    virtual ~ToshibaFnActions();

    void hideWidgets();
    void performFnAction(int, int);
    KToshibaSMMInterface *m_Driver;
    bool m_SCIIface;
    int m_MachineID;
    int m_Popup;
    int m_BatType;
    int m_BatSave;
    int m_Video;
    int m_Pad;
private:
    void toggleMute();
    void lockScreen();
    void toggleBSM();
    void suspendToRAM();
    void suspendToDisk();
    void toggleVideo();
    void brightDown();
    void brightUp();
    void toggleWireless();
    void toggleMousePad();
    void toggleSpeakerVolume();
    void toggleFan();
    void toggleBootMethod();
    void toogleBackLight();
    void toggleBluetooth();
    void toggleEthernet();
    void initSCI();
    SettingsWidget *m_SettingsWidget;
    StatusWidget *m_StatusWidget;
    Suspend *m_Suspend;
    QWidget *m_Parent;
    int m_Snd;
    int m_Bright;
    int m_Wireless;
    int m_BootType;
    int m_Vol;
    int m_Fan;
    int m_Boot;
    int m_LANCtrl;
};

#endif // TOSHIBA_FN_ACTIONS_H
