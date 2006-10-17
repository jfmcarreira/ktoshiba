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

#ifdef ENABLE_SYNAPTICS
#include <synaptics/synaptics.h>
#include <synaptics/synparams.h>

using namespace Synaptics;
#endif // ENABLE_SYNAPTICS

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
        //m_Video = m_Omni->omnibookGetVideo();
        m_Bright = m_Omni->getBrightness();
        m_Wireless = m_Omni->getWifi();
        m_Pad = m_Omni->getTouchPad();
        m_Fan = m_Omni->getFan();
    }
    else {
        m_ModelName = i18n("UNKNOWN");
        m_ECType = NONE;
        //m_Video = -1;
        m_Bright = -1;
        m_Pad = -1;
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
        case 1:	// Mute/Unmute
        case 7:	// Brightness Down
        case 8:	// Brightness Up
        case 10:	// Enable/Disable MousePad
        case 12:	// Fan On/Off
            showWidget(1, keycode);
            break;
        case 22:	// Show Battery Status
            showWidget(2, keycode);
            break;
    }

    if ((keycode & 0x17f) == m_Popup) {
        int type = -1, extra = -1;
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
        if (action == 10 && (keycode == 151 || keycode == 152)) {
            (keycode == 151)? mousePadOn() : mousePadOff();
            type = m_Pad;
            extra = keycode;
        } else
        if (action == 12) {
            toggleFan();
            type = m_Fan;
        }
        updateWidget(action, type, extra);
    }
}

void OmnibookFnActions::toggleWireless()
{
    if (m_Wireless == -1) return;

    int ws = m_Omni->getWifiSwitch();
    if (!ws || ws == -1)
        return;
    else {
        m_Wireless = m_Omni->getWifi();
        (m_Wireless == 1)? m_Omni->setWifi(0)
            : m_Omni->setWifi(1);

        showPassiveMsg((m_Wireless = (m_Wireless == 1)? 0: 1), 'w');
    }
}

void OmnibookFnActions::mousePadOn()
{
    if (m_Pad == -1) return;

#ifdef ENABLE_SYNAPTICS
    m_Pad = 0;
    Pad::setParam(TOUCHPADOFF, ((double)m_Pad));
#else // ENABLE_SYNAPTICS
    m_Pad = 1;
    m_Omni->omnibookSetTouchPad(m_Pad);
#endif // ENABLE_SYNAPTICS
}

void OmnibookFnActions::mousePadOff()
{
    if (m_Pad == -1) return;

#ifdef ENABLE_SYNAPTICS
    m_Pad = 1;
    Pad::setParam(TOUCHPADOFF, ((double)m_Pad));
#else // ENABLE_SYNAPTICS
    m_Pad = 0;
    m_Omni->omnibookSetTouchPad(m_Pad);
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

    (bl == 1)? m_Omni->setLCDBackLight(0)
        : m_Omni->setLCDBackLight(1);
}

void OmnibookFnActions::toggleBluetooth()
{
    int bt = m_Omni->getBluetooth();
    if (bt == -1) return;

    (bt == 1)? m_Omni->setBluetooth(0)
        : m_Omni->setBluetooth(1);
    showPassiveMsg(((bt == 1)? 0: 1), 'b');
}
