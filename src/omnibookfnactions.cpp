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
#include "ktoshibaprocinterface.h"
#include "suspend.h"

#include <qwidgetstack.h>
#include <qapplication.h>
#include <qlabel.h>

#include <kdebug.h>
#include <klocale.h>
#include <dcopref.h>
#include <kprogress.h>

#ifdef ENABLE_SYNAPTICS
#include <synaptics/synaptics.h>
#include <synaptics/synparams.h>

using namespace Synaptics;
#endif // ENABLE_SYNAPTICS

#include "settingswidget.h"
#include "statuswidget.h"

OmnibookFnActions::OmnibookFnActions(QWidget *parent)
    : QObject( parent ),
      m_Proc( 0 )
{
    m_Proc = new KToshibaProcInterface(parent);
    m_SettingsWidget = new SettingsWidget(0, "Screen Indicator", Qt::WX11BypassWM);
    m_SettingsWidget->setFocusPolicy(QWidget::NoFocus);
    m_StatusWidget = new StatusWidget(0, "Screen Indicator", Qt::WX11BypassWM);
    m_StatusWidget->setFocusPolicy(QWidget::NoFocus);
    m_Suspend = new Suspend(parent);

    m_OmnibookIface = m_Proc->checkOmnibook();
    if (m_OmnibookIface) {
        m_ModelName = m_Proc->omnibookModelName();
        m_ECType = m_Proc->omnibookECType();
        //m_Video = m_Proc->omnibookGetVideo();
        m_Bright = m_Proc->omnibookGetBrightness();
        m_Pad = m_Proc->omnibookGetTouchPad();
        m_Fan = m_Proc->omnibookGetFan();
    }
    else {
        m_ModelName = i18n("UNKNOWN");
        m_ECType = NONE;
        //m_Video = -1;
        m_Bright = -1;
        m_Pad = -1;
        m_Fan = -1;
    }
    m_Popup = 0;
    m_Snd = 1;
}

OmnibookFnActions::~OmnibookFnActions()
{
    delete m_SettingsWidget; m_SettingsWidget = NULL;
    delete m_StatusWidget; m_StatusWidget = NULL;
    delete m_Suspend; m_Suspend = NULL;
    delete m_Proc; m_Proc = NULL;
}

void OmnibookFnActions::hideWidgets()
{
    m_StatusWidget->hide();
    m_Popup = 0;
}

void OmnibookFnActions::performFnAction(int action, int keycode)
{
    // TODO: Add more actions here once we have more data
    switch (action) {
        case 0:	// Disabled (Do Nothing)
            return;
        case 1:	// Mute/Unmute
        case 7:	// Brightness Down
        case 8:	// Brightness Up
        case 10:	// Enable/Disable MousePad
        case 12:	// Fan On/Off
            if (m_Popup == 0) {
                QRect r = QApplication::desktop()->geometry();
                m_StatusWidget->move(r.center() - 
                    QPoint(m_StatusWidget->width()/2, m_StatusWidget->height()/2));
                m_StatusWidget->show();
                m_Popup = 1;
            }
            if (m_Popup == 1)
                break;
        case 2:	// LockScreen
            lockScreen();
            return;
        case 4:	// Suspend To RAM (STR)
            suspendToRAM();
            return;
        case 5:	// Suspend To Disk (STD)
            suspendToDisk();
            return;
        case 14:	// LCD Backlight On/Off
            toogleBackLight();
            return;
    }

    if (action == 1) {
        m_Snd--;
        if (m_Snd < 0) m_Snd = 1;
        toggleMute();
        m_StatusWidget->wsStatus->raiseWidget(m_Snd);
        return;
    }
    if (action == 7 || action == 8) {
        if (m_Bright <= 7 && m_Bright >= 0)
            m_StatusWidget->wsStatus->raiseWidget(m_Bright + 4);
        return;
    }
    if (action == 10) {
        if (m_Pad == -1)
            m_StatusWidget->wsStatus->raiseWidget(3);
        else {
            (keycode == 151)? mousePadOn() : mousePadOff();
            m_StatusWidget->wsStatus->raiseWidget(((m_Pad == 0)? 3 : 2));
        }
        return;
    }
    if (action == 12) {
        toggleFan();
        if (m_Fan == -1) return;

        (m_Fan == 1)? m_StatusWidget->wsStatus->raiseWidget(12)
            : m_StatusWidget->wsStatus->raiseWidget(13);
        return;
    }
    if (action == 22) {
        m_SettingsWidget->wsSettings->raiseWidget(4);
        m_SettingsWidget->tlStatus->setText("Battery Status");
        int time = 0, perc = -1;
         m_Proc->omnibookBatteryStatus(&time, &perc);
        (perc == -1)? m_SettingsWidget->batteryKPB->setValue(0)
            : m_SettingsWidget->batteryKPB->setValue(perc);
    }
}

void OmnibookFnActions::toggleMute()
{
    DCOPRef kmixClient("kmix", "Mixer0");
    DCOPReply reply = kmixClient.call("mute", 0);
    if (reply.isValid()) {
        bool res = reply;
        m_Snd = (res == true)? 1 : 0;
    }

    kmixClient.send("toggleMute", 0);
}

void OmnibookFnActions::lockScreen()
{
    DCOPRef kdesktopClient("kdesktop", "KScreensaverIface");
    kdesktopClient.send("lock()", 0);
}

void OmnibookFnActions::suspendToRAM()
{
    m_Suspend->toRAM();
}

void OmnibookFnActions::suspendToDisk()
{
    m_Suspend->toDisk();
}

void OmnibookFnActions::mousePadOn()
{
    if (m_Pad == -1) return;

#ifdef ENABLE_SYNAPTICS
    m_Pad = 0;
    Pad::setParam(TOUCHPADOFF, ((double)m_Pad));
#else // ENABLE_SYNAPTICS
    m_Pad = 1;
    m_Proc->omnibookSetTouchPad(m_Pad);
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
    m_Proc->omnibookSetTouchPad(m_Pad);
#endif // ENABLE_SYNAPTICS
}

void OmnibookFnActions::toggleFan()
{
    m_Fan = m_Proc->omnibookGetFan();
    if (m_Fan == -1) return;

    m_Fan = (m_Fan == 0)? 1 : 0;
    m_Proc->omnibookSetFan(m_Fan);
}

void OmnibookFnActions::toogleBackLight()
{
    int bl = m_Proc->omnibookGetLCDBackLight();
    if (bl == -1) return;

    (bl == 1)? m_Proc->omnibookSetLCDBackLight(0)
        : m_Proc->omnibookSetLCDBackLight(1);
}


#include "omnibookfnactions.moc"
