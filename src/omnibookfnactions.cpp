/***************************************************************************
 *   Copyright (C) 2006 by Azael Avalos                                    *
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

#include "omnibookfnactions.h"
#include "ktoshibaomnibookinterface.h"
#include "suspend.h"

#include <kdebug.h>
#include <klocale.h>
#include <kprogress.h>

OmnibookFnActions::OmnibookFnActions(QWidget *parent)
    : FnActions( parent ),
      m_Omni( 0 )
{
    m_Omni = new KToshibaOmnibookInterface(0);

    m_OmnibookIface = m_Omni->checkOmnibook();
    if (m_OmnibookIface) {
        kdDebug() << "KToshiba: Found a Toshiba model with Phoenix BIOS." << endl;
        m_ModelName = m_Omni->modelName();
        m_ECType = m_Omni->ecType();
        kdDebug() << "KToshiba: Machine ECTYPE=" << m_ECType << endl;
        //m_Video = m_Omni->getVideo();
        m_Bright = m_Omni->getBrightness();
        m_Wireless = m_Omni->getWifi();
#ifndef ENABLE_SYNAPTICS
        m_Pad = m_Omni->getTouchPad();
#endif
        m_Fan = m_Omni->getFan();
    }
    else {
        m_ModelName = i18n("UNKNOWN");
        m_ECType = NONE;
        //m_Video = -1;
        m_Bright = -1;
        m_Fan = -1;
    }
    m_Snd = 1;
}

OmnibookFnActions::~OmnibookFnActions()
{
    delete m_Omni; m_Omni = NULL;
}

void OmnibookFnActions::performFnAction(int action, int keycode)
{
    switch (action) {
        case 0:	// Disabled (Do Nothing)
            return;
        case 2:	// LockScreen
            lockScreen();
            return;
        case 4:	// Suspend To RAM (STR)
            m_Suspend->toRAM();
            return;
        case 5:	// Suspend To Disk (STD)
            m_Suspend->toDisk();
            return;
        case 9:	// Wireless On/Off
            toggleWireless();
            return;
        case 14:	// LCD Backlight On/Off
            toogleBackLight();
            return;
        case 15:	// Bluetooth On/Off
            toggleBluetooth();
            return;
        case 17:	// Run Command
            runCommand(keycode);
            return;
        case 1:	// Mute/Unmute
        case 7:	// Brightness Down
        case 8:	// Brightness Up
        case 10:	// Enable/Disable MousePad
        case 12:	// Fan On/Off
            showWidget(1, keycode);
            break;
        case 3:	// Toggle Battery Save Mode
        case 22:	// Show Battery Status
            showWidget(2, keycode);
            break;
    }

    int type = -1, extra = -1;
    if (action == 3) {
        // Only toggle BSM if we're running on batteries...
        int m_AC = m_Omni->omnibookAC();
        if (m_AC == 3) {
            toggleBSM();
            type = m_BatSave;
        } else {
            m_BatSave = 2;
            type = extra = 4;
        }
    } else
    if (action == 22) {
        int time = 0, perc = -1;
        m_Omni->batteryStatus(&time, &perc);
        type = perc;
    } else
    if (action == 1) {
        toggleMute(&m_Snd);
        type = m_Snd;
    } else
    if (action == 7 || action == 8) {
        type = m_Bright;
        extra = keycode;
    } else
    if (action == 10) {
        toggleMousePad();
        type = m_Pad;
    } else
    if (action == 12) {
        toggleFan();
        type = m_Fan;
    }
    updateWidget(action, type, extra);
}

void OmnibookFnActions::toggleBSM()
{
    m_BatSave--;
    if (m_BatSave < 0)
        m_BatSave = 2;
}

void OmnibookFnActions::toggleWireless()
{
    if (m_Wireless == -1) return;

    int ws = m_Omni->getWifiSwitch();
    if (!ws || ws == -1)
        return;
    else {
        m_Wireless = m_Omni->getWifi();
        m_Omni->setWifi((m_Wireless == 1)? 0 : 1);

        showPassiveMsg((m_Wireless = (m_Wireless == 1)? 0 : 1), Wireless);
    }
}

void OmnibookFnActions::toggleMousePad()
{
    if (m_Pad == -1) return;

#ifdef ENABLE_SYNAPTICS
    m_Pad = (m_Pad == 0)? 1 : 0;
    mSynPad->setParam(TOUCHPADOFF, ((double) m_Pad));
#else // ENABLE_SYNAPTICS
    m_Pad = m_Omni->getTouchPad();

    m_Pad = (m_Pad == 0)? 1 : 0;
    m_Omni->setTouchPad(m_Pad);
#endif // ENABLE_SYNAPTICS
}

void OmnibookFnActions::toggleFan()
{
    m_Fan = m_Omni->getFan();
    if (m_Fan == -1) return;

    m_Fan = (m_Fan == 0)? 1 : 0;
    m_Omni->setFan(m_Fan);
}

void OmnibookFnActions::toogleBackLight()
{
    int bl = m_Omni->getLCDBackLight();
    if (bl == -1) return;

    m_Omni->setLCDBackLight((bl == 1)? 0 : 1);
}

void OmnibookFnActions::toggleBluetooth()
{
    int bt = m_Omni->getBluetooth();
    if (bt == -1) return;

    m_Omni->setBluetooth((bt == 1)? 0 : 1);
    showPassiveMsg(((bt == 1)? 0 : 1), Bluetooth);
}
