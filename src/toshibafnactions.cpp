/***************************************************************************
 *   Copyright (C) 2005-2006 by Azael Avalos                               *
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

#include "toshibafnactions.h"
#include "ktoshibasmminterface.h"
#include "suspend.h"

#include <kdebug.h>
#include <kconfig.h>

#ifdef ENABLE_SYNAPTICS
#include <synaptics/synaptics.h>
#include <synaptics/synparams.h>

using namespace Synaptics;
#endif // ENABLE_SYNAPTICS

ToshibaFnActions::ToshibaFnActions(QWidget *parent)
    : FnActions( parent ),
      m_Driver( 0 )
{
    m_Driver = new KToshibaSMMInterface(0);

    m_SCIIface = m_Driver->openSCIInterface(&m_IFaceErr);
    if (!m_SCIIface)
        m_SCIIface = (m_IFaceErr == SCI_ALREADY_OPEN)? true : false;
    if (m_SCIIface)
        initSCI();
    else if (!m_SCIIface) {
        kdError() << "KToshiba: Could not open SCI interface." << endl;
        m_BatSave = 2;
        m_BatType = 3;
        m_Boot = -1;
        m_BootType = -1;
        m_LANCtrl = -1;
        m_Pad = -1;
    }

    if (::access("/dev/toshiba", F_OK | R_OK | W_OK) == -1) {
        kdError() << "KToshiba: Could not access " << TOSH_DEVICE
                  << " for read-write operations."<< endl;
        m_BIOS = -1;
    } else {
        // check the BIOS version
        m_BIOS = m_Driver->machineBIOS();
    }
    if (m_BIOS != -1) {
        m_MachineID = m_Driver->machineID();
        m_Video = m_Driver->getVideo();
        m_Bright = m_Driver->getBrightness();
        m_Wireless = m_Driver->getWirelessPower();
    }
    m_Snd = 1;
    m_Vol = 1;
    m_Fan = 1;
}

ToshibaFnActions::~ToshibaFnActions()
{
    delete m_Driver; m_Driver = NULL;
}

void ToshibaFnActions::initSCI()
{
    if (m_IFaceErr != SCI_ALREADY_OPEN)
        kdDebug() << "KToshiba: SCI interface opened successfully." << endl;
    kdDebug() << "KToshiba: SCI version: " << m_Driver->getSCIVersion() << endl;
    m_BatType = m_Driver->getBatterySaveModeType();
    // Default to type 2 if we got a failure
    if (m_BatType == -1) m_BatType = 2;
    m_BatSave = m_Driver->getBatterySaveMode();
    // Default to the highest mode if we got a failure
    if (m_BatSave == -1) m_BatSave = (m_BatType == 3)? 3 : 2;
    m_Boot = m_Driver->getBootMethod();
    m_BootType = m_Driver->getBootType();
    m_LANCtrl = m_Driver->getLANController();
    m_Pad = m_Driver->getPointingDevice();
}

void ToshibaFnActions::performFnAction(int action, int key)
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
        case 16:	// Ethernet On/Off
            toggleEthernet();
            return;
        case 17:	// Run Command
            runCommand(key);
            return;
        case 1:	// Mute/Unmute
        case 7:	// Brightness Down
        case 8:	// Brightness Up
        case 10:	// Enable/Disable MousePad
        case 11:	// Speaker Volume
        case 12:	// Fan On/Off
            showWidget(1, key);
            break;
        case 3:	// Toggle Battery Save Mode
        case 6:	// Toggle Video
        case 13:	// Toggle Boot Method
        case 22:	// Show Battery Status
            showWidget(2, key);
            break;
    }

    int state = -1, extra = -1;
    if (action == 3 && m_SCIIface) {
        toggleBSM();
        state = m_BatSave;
        extra = m_BatType;
    } else
    if (action == 6) {
        toggleVideo();
        state = m_Video;
    } else
    if (action == 13 && m_SCIIface) {
        toggleBootMethod();
        state = (m_BootType == 5 && m_LANCtrl >= 0)? 6 : m_BootType;
        extra = m_Boot;
    } else
    if (action == 22 && m_SCIIface) {
        int time = 0, perc = -1;
        m_Driver->batteryStatus(&time, &perc);
        state = perc;
    } else
    if (action == 1) {
        toggleMute(&m_Snd);
        state = m_Snd;
    } else
    if ((action == 7) || (action == 8)) {
        (action == 7)? brightDown() : brightUp();
        state = m_Bright;
    } else
    if (action == 10) {
        toggleMousePad();
        state = m_Pad;
    } else
    if (action == 11 && m_SCIIface) {
        toggleSpeakerVolume();
        state = m_Vol;
    } else
    if (action == 12) {
        toggleFan();
        state = m_Fan;
    }
    updateWidget(action, state, extra);
}

void ToshibaFnActions::toggleBSM()
{
    // ISSUE: Always returns the same value no matter what...
    //m_BatSave = m_Driver->getBatterySaveMode();
    //if (m_BatSave == -1) return;
    KConfig cfg("ktoshibarc");
    cfg.setGroup("BSM");
    m_BatSave = cfg.readNumEntry("Battery_Save_Mode", 2);

    m_BatSave--;
    if (m_BatSave < 1 && m_BatType == 3)
        m_BatSave = 3;
    else if (m_BatSave < 0)
        m_BatSave = 2;

    m_Driver->setBatterySaveMode(m_BatSave);
    cfg.writeEntry("Battery_Save_Mode", m_BatSave);
    cfg.sync();
}

void ToshibaFnActions::toggleVideo()
{
    m_Video = m_Driver->getVideo();
    if (m_Video == -1) return;

    // ISSUE: Whenever you toogle video-out we receive
    // 129 code (for LCD) from the driver, strange indeed...
    if (m_Video == 129) m_Video = 1;

    // TODO: Find out wich models change video-out automatically
    //m_Driver->setVideo(m_Video);
}

void ToshibaFnActions::brightDown()
{
    m_Bright = m_Driver->getBrightness();
    if (m_Bright == 0) return;

    m_Driver->setBrightness(--m_Bright);
}

void ToshibaFnActions::brightUp()
{
    m_Bright = m_Driver->getBrightness();
    if (m_Bright == 7) return;

    m_Driver->setBrightness(++m_Bright);
}

void ToshibaFnActions::toggleWireless()
{
    if (m_Wireless == -1) return;

    int ws = m_Driver->getWirelessSwitch();
    if (!ws || ws == -1)
        return;
    else {
        m_Wireless--;
        if (m_Wireless < 0) m_Wireless = 1;

        m_Driver->setWirelessPower(m_Wireless);
        showPassiveMsg(m_Wireless, Wireless);
    }
}

void ToshibaFnActions::toggleMousePad()
{
    if (m_Pad == -1) return;

#ifdef ENABLE_SYNAPTICS
    m_Pad = (m_Pad == 0)? 1 : 0;
    Pad::setParam(TOUCHPADOFF, ((double)m_Pad));
#else // ENABLE_SYNAPTICS
    if (m_SCIIface) {
        m_Pad = m_Driver->getPointingDevice();

        m_Pad = (m_Pad == 0)? 1 : 0;
        m_Driver->setPointingDevice(m_Pad);
    }
#endif // ENABLE_SYNAPTICS
}

void ToshibaFnActions::toggleSpeakerVolume()
{
    m_Vol = m_Driver->getSpeakerVolume();
    if (m_Vol == -1) return;

    m_Vol++;
    if (m_Vol > 3) m_Vol = 0;
    m_Driver->setSpeakerVolume(m_Vol);
}

void ToshibaFnActions::toggleFan()
{
    m_Fan = m_Driver->getFan();
    if (m_Fan == -1) return;

    m_Fan = (m_Fan == 0)? 1 : 0;
    m_Driver->setFan(m_Fan);
}

void ToshibaFnActions::toggleBootMethod()
{
    m_Boot = m_Driver->getBootMethod();
    if (m_Boot == -1) return;

    m_Boot++;
    if (m_Boot > m_BootType) m_Boot = 0;
    m_Driver->setBootMethod(m_Boot);
}

void ToshibaFnActions::toogleBackLight()
{
    int bl = m_Driver->getBackLight();

    if (bl == -1) return;
    m_Driver->setBackLight((bl == 1)? 0 : 1);
}

void ToshibaFnActions::toggleBluetooth()
{
    if (m_Driver->getBluetooth() != 0) {
        int bt = m_Driver->getBluetoothPower();
        if (bt == -1) return;

        m_Driver->setBluetoothPower((bt == 1)? 0 : 1);
        showPassiveMsg(((bt == 1)? 0: 1), Bluetooth);
    }
}

void ToshibaFnActions::toggleEthernet()
{
    int eth = m_Driver->getLANController();
    if (eth == -1) return;

    m_Driver->setLANController((eth == 1)? 0 : 1);
    showPassiveMsg(((eth == 1)? 0: 1), Ethernet);
}
