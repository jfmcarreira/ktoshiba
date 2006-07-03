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

#include <qwidgetstack.h>
#include <qapplication.h>
#include <qlabel.h>

#include <kdebug.h>
#include <dcopref.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kprocess.h>
#include <klocale.h>
#include <kpassivepopup.h>
#include <kiconloader.h>
#include <kconfig.h>

#ifdef ENABLE_SYNAPTICS
#include <synaptics/synaptics.h>
#include <synaptics/synparams.h>

using namespace Synaptics;
#endif // ENABLE_SYNAPTICS

#ifdef ENABLE_POWERSAVE
#include <powersave_dbus.h>
#endif // ENABLE_POWERSAVE

#include "settingswidget.h"
#include "statuswidget.h"

ToshibaFnActions::ToshibaFnActions(QWidget *parent)
    : QObject( parent ),
      m_Driver( 0 )
{
    m_Driver = new KToshibaSMMInterface(this);
    m_SettingsWidget = new SettingsWidget(0, "Screen Indicator", Qt::WX11BypassWM);
    m_SettingsWidget->setFocusPolicy(QWidget::NoFocus);
    m_StatusWidget = new StatusWidget(0, "Screen Indicator", Qt::WX11BypassWM);
    m_StatusWidget->setFocusPolicy(QWidget::NoFocus);

    m_SCIIface = m_Driver->openSCIInterface();
    if (m_SCIIface)
        initSCI();
    else if (!m_SCIIface) {
        kdError() << "KToshiba: Could not open SCI interface." << endl;
        m_BatType = 3;
        m_Boot = -1;
        m_BootType = -1;
        m_LANCtrl = -1;
        m_Pad = -1;
    }

    m_Video = m_Driver->getVideo();
    m_Bright = m_Driver->getBrightness();
    m_Wireless = m_Driver->getWirelessPower();
    m_Parent = parent;
    m_Popup = 0;
    m_Snd = 1;
    m_BatSave = 2;
    m_Mousepad = 0;
    m_Vol = -1;
    m_Fan = -1;

#ifdef ENABLE_SYNAPTICS
    if (m_Pad == -1)
        checkSynaptics();
#endif // ENABLE_SYNAPTICS
}

ToshibaFnActions::~ToshibaFnActions() {
    delete m_Parent; m_Parent = NULL;
    delete m_SettingsWidget; m_SettingsWidget = NULL;
    delete m_StatusWidget; m_StatusWidget = NULL;
    delete m_Driver; m_Driver = NULL;
}

void ToshibaFnActions::initSCI()
{
    kdDebug() << "KToshiba: SCI interface opened successfully." << endl;
    int major = ((m_Driver->sciversion & 0xff00)>>8);
    int minor = (m_Driver->sciversion & 0xff);
    kdDebug() << "KToshiba: SCI version: " << major << "." << minor << endl;
    m_BatType = m_Driver->getBatterySaveModeType();
    m_Boot = m_Driver->getBootMethod();
    m_BootType = m_Driver->getBootType();
    m_LANCtrl = m_Driver->getLANController();
    m_Pad = m_Driver->getPointingDevice();

    if (m_BootType == 5 && m_LANCtrl >= 0)
        m_BootType = 6;
}

void ToshibaFnActions::closeSCIIface()
{
    if (m_SCIIface)
        m_Driver->closeSCIInterface();
}

#ifdef ENABLE_SYNAPTICS
void ToshibaFnActions::checkSynaptics()
{
    static bool driver = Pad::hasDriver();
    if (driver == false) {
        kdError() << "KToshiba: Incompatible synaptics driver version " << endl;
        m_Mousepad = -1;
        return;
    }

    static bool shm = Pad::hasShm();
    if (shm == false) {
        kdError() << "KToshiba: Access denied to driver shared memory area" << endl;
        m_Mousepad = -1;
        return;
    }

    static bool param = Pad::hasParam(TOUCHPADOFF);
    if (param == false) {
        kdDebug() << "KToshiba: TouchPad will not be enabled/disabled" << endl;
        m_Mousepad = -1;
        return;
    }

    m_Mousepad = (int)Pad::getParam(TOUCHPADOFF);
}
#endif // ENABLE_SYNAPTICS

void ToshibaFnActions::hideWidgets()
{
    m_SettingsWidget->hide();
    m_StatusWidget->hide();
    m_Popup = 0;
}

void ToshibaFnActions::performFnAction(int action, int key)
{
    switch(action) {
        case 0: // Disabled (Do Nothing)
            return;
        case 2: // LockScreen
            lockScreen();
            return;
        case 4: // Suspend To RAM (STR)
            suspendToRAM();
            return;
        case 5: // Suspend To Disk (STD)
            suspendToDisk();
            return;
        case 9: // Wireless On/Off
            toggleWireless();
            return;
        case 14: // LCD Backlight On/Off
            toogleBackLight();
            return;
        case 15: // Bluetooth On/Off
            toggleBluetooth();
            return;
        case 16: // Ethernet On/Off
            toggleEthernet();
            return;
        case 1: // Mute/Unmute
        case 7: // Brightness Down
        case 8: // Brightness Up
        case 10: // Enable/Disable MousePad
        case 11: // Speaker Volume
        case 12: // Fan On/Off
            if (m_Popup == 0) {
                QRect r = QApplication::desktop()->geometry();
                m_StatusWidget->move(r.center() - 
                    QPoint(m_StatusWidget->width()/2, m_StatusWidget->height()/2));
                m_StatusWidget->show();
                m_Popup = key & 0x17f;
            }
            if ((key & 0x17f) == m_Popup)
                break;
        case 3: // Toggle Battery Save Mode
        case 6: // Toggle Video
        case 13: // Toggle Boot Method
            if (m_Popup == 0) {
                QRect r = QApplication::desktop()->geometry();
                m_SettingsWidget->move(r.center() - 
                    QPoint(m_SettingsWidget->width()/2, m_SettingsWidget->height()/2));
                m_SettingsWidget->show();
                m_Popup = key & 0x17f;
            }
            if ((key & 0x17f) == m_Popup)
                break;
    }

    if (action == 3 && m_SCIIface) {
        m_BatSave--;
        if (m_BatSave < 0) m_BatSave = 2;
        toggleBSM();
        KConfig cfg("ktoshibarc");
        cfg.setGroup("KToshiba");
        cfg.writeEntry("Battery_Save_Mode", m_BatSave);
        m_SettingsWidget->wsSettings->raiseWidget(0);
        m_SettingsWidget->plUser->setFrameShape(QLabel::NoFrame);
        m_SettingsWidget->plMedium->setFrameShape(QLabel::NoFrame);
        m_SettingsWidget->plFull->setFrameShape(QLabel::NoFrame);
        switch (m_BatSave) {
            case 0:
                (m_BatType == 3)? m_SettingsWidget->tlStatus->setText(i18n("Long Life"))
                    : m_SettingsWidget->tlStatus->setText(i18n("User Settings"));
                m_SettingsWidget->plUser->setFrameShape(QLabel::PopupPanel);
                break;
            case 1:
                (m_BatType == 3)? m_SettingsWidget->tlStatus->setText(i18n("Normal Life"))
                    : m_SettingsWidget->tlStatus->setText(i18n("Low Power"));
                m_SettingsWidget->plMedium->setFrameShape(QLabel::PopupPanel);
                break;
            case 2:
                (m_BatType == 3)? m_SettingsWidget->tlStatus->setText(i18n("Full Life"))
                    : m_SettingsWidget->tlStatus->setText(i18n("Full Power"));
                m_SettingsWidget->plFull->setFrameShape(QLabel::PopupPanel);
                break;
        }
    }
    if (action == 6) {
        m_Video += 2;
        if (m_Video == 5) m_Video = 2;
        else if (m_Video > 4) m_Video = 1;
        // TODO: Find out wich models do video out change automatically
        //toggleVideo();
        m_SettingsWidget->wsSettings->raiseWidget(1);
        m_SettingsWidget->plLCD->setFrameShape(QLabel::NoFrame);
        m_SettingsWidget->plCRT->setFrameShape(QLabel::NoFrame);
        m_SettingsWidget->plLCDCRT->setFrameShape(QLabel::NoFrame);
        m_SettingsWidget->plTV->setFrameShape(QLabel::NoFrame);
        switch (m_Video) {
            case 1:
                m_SettingsWidget->tlStatus->setText("LCD");
                m_SettingsWidget->plLCD->setFrameShape(QLabel::PopupPanel);
                break;
            case 2:
                m_SettingsWidget->tlStatus->setText("CRT");
                m_SettingsWidget->plCRT->setFrameShape(QLabel::PopupPanel);
                break;
            case 3:
                m_SettingsWidget->tlStatus->setText("LCD/CRT");
                m_SettingsWidget->plLCDCRT->setFrameShape(QLabel::PopupPanel);
                break;
            case 4:
                m_SettingsWidget->tlStatus->setText("S-Video");
                m_SettingsWidget->plTV->setFrameShape(QLabel::PopupPanel);
                break;
        }
    }
    if (action == 13 && m_SCIIface) {
        m_Boot++;
        if (m_Boot > m_BootType) m_Boot = 0;
        toggleBootMethod();
        if (m_BootType == 6) {
            m_SettingsWidget->wsSettings->raiseWidget(2);
            m_SettingsWidget->pl1->setPixmap(SmallIcon("", 32));
            m_SettingsWidget->pl2->setPixmap(SmallIcon("", 32));
            m_SettingsWidget->pl3->setPixmap(SmallIcon("", 32));
            m_SettingsWidget->pl4->setPixmap(SmallIcon("", 32));
        } else {
            m_SettingsWidget->wsSettings->raiseWidget(3);
            m_SettingsWidget->pl1_2->setPixmap(SmallIcon("", 32));
            m_SettingsWidget->pl2_2->setPixmap(SmallIcon("", 32));
            m_SettingsWidget->pl3_2->setPixmap(SmallIcon("", 32));
        }
        switch (m_BootType) {
            case -1:
                m_SettingsWidget->tlStatus->setText("Function Not Supported");
            case 1:
                if (!m_Boot) {
                    m_SettingsWidget->tlStatus->setText("FDD -> HDD");
                    m_SettingsWidget->pl1_2->setPixmap(SmallIcon("3floppy_unmount", 32));
                    m_SettingsWidget->pl3_2->setPixmap(SmallIcon("hdd_unmount", 32));
                }
                else {
                    m_SettingsWidget->tlStatus->setText("HDD -> FDD");
                    m_SettingsWidget->pl1_2->setPixmap(SmallIcon("hdd_unmount", 32));
                    m_SettingsWidget->pl3_2->setPixmap(SmallIcon("3floppy_unmount", 32));
                }
                break;
            case 3:
                if (!m_Boot) {
                    m_SettingsWidget->tlStatus->setText("FDD -> Built-in HDD");
                    m_SettingsWidget->pl1_2->setPixmap(SmallIcon("3floppy_unmount", 32));
                    m_SettingsWidget->pl3_2->setPixmap(SmallIcon("hdd_unmount", 32));
                } else
                if (m_Boot == 1) {
                    m_SettingsWidget->tlStatus->setText("Built-in HDD -> FDD");
                    m_SettingsWidget->pl1_2->setPixmap(SmallIcon("hdd_unmount", 32));
                    m_SettingsWidget->pl3_2->setPixmap(SmallIcon("3floppy_unmount", 32));
                } else
                if (m_Boot == 2) {
                    m_SettingsWidget->tlStatus->setText("FDD -> Second HDD");
                    m_SettingsWidget->pl1_2->setPixmap(SmallIcon("3floppy_unmount", 32));
                    m_SettingsWidget->pl3_2->setPixmap(SmallIcon("hdd_unmount", 32));
                }
                else {
                    m_SettingsWidget->tlStatus->setText("Second HDD -> FDD");
                    m_SettingsWidget->pl1_2->setPixmap(SmallIcon("hdd_unmount", 32));
                    m_SettingsWidget->pl3_2->setPixmap(SmallIcon("3floppy_unmount", 32));
                }
                break;
            case 5:
                if (!m_Boot) {
                    m_SettingsWidget->tlStatus->setText("FDD -> HDD -> CD-ROM");
                    m_SettingsWidget->pl1_2->setPixmap(SmallIcon("3floppy_unmount", 32));
                    m_SettingsWidget->pl2_2->setPixmap(SmallIcon("hdd_unmount", 32));
                    m_SettingsWidget->pl3_2->setPixmap(SmallIcon("cdrom_unmount", 32));
                } else
                if (m_Boot == 1) {
                    m_SettingsWidget->tlStatus->setText("HDD -> FDD -> CD-ROM");
                    m_SettingsWidget->pl1_2->setPixmap(SmallIcon("hdd_unmount", 32));
                    m_SettingsWidget->pl2_2->setPixmap(SmallIcon("3floppy_unmount", 32));
                    m_SettingsWidget->pl3_2->setPixmap(SmallIcon("cdrom_unmount", 32));
                } else
                if (m_Boot == 2) {
                    m_SettingsWidget->tlStatus->setText("FDD -> CD-ROM -> HDD");
                    m_SettingsWidget->pl1_2->setPixmap(SmallIcon("3floppy_unmount", 32));
                    m_SettingsWidget->pl2_2->setPixmap(SmallIcon("cdrom_unmount", 32));
                    m_SettingsWidget->pl3_2->setPixmap(SmallIcon("hdd_unmount", 32));
                } else
                if (m_Boot == 3) {
                    m_SettingsWidget->tlStatus->setText("HDD -> CD-ROM -> FDD");
                    m_SettingsWidget->pl1_2->setPixmap(SmallIcon("hdd_unmount", 32));
                    m_SettingsWidget->pl2_2->setPixmap(SmallIcon("cdrom_unmount", 32));
                    m_SettingsWidget->pl3_2->setPixmap(SmallIcon("3floppy_unmount", 32));
                } else
                if (m_Boot == 4) {
                    m_SettingsWidget->tlStatus->setText("CD-ROM -> FDD -> HDD");
                    m_SettingsWidget->pl1_2->setPixmap(SmallIcon("cdrom_unmount", 32));
                    m_SettingsWidget->pl2_2->setPixmap(SmallIcon("3floppy_unmount", 32));
                    m_SettingsWidget->pl3_2->setPixmap(SmallIcon("hdd_unmount", 32));
                }
                else {
                    m_SettingsWidget->tlStatus->setText("CD-ROM -> HDD -> FDD");
                    m_SettingsWidget->pl1_2->setPixmap(SmallIcon("cdrom_unmount", 32));
                    m_SettingsWidget->pl2_2->setPixmap(SmallIcon("hdd_unmount", 32));
                    m_SettingsWidget->pl3_2->setPixmap(SmallIcon("3floppy_unmount", 32));
                }
                break;
            case 6:
                if (!m_Boot) {
                    m_SettingsWidget->tlStatus->setText("FDD -> HDD -> CD-ROM -> LAN");
                    m_SettingsWidget->pl1->setPixmap(SmallIcon("3floppy_unmount", 32));
                    m_SettingsWidget->pl2->setPixmap(SmallIcon("hdd_unmount", 32));
                    m_SettingsWidget->pl3->setPixmap(SmallIcon("cdrom_unmount", 32));
                    m_SettingsWidget->pl4->setPixmap(SmallIcon("nfs_unmount", 32));
                } else
                if (m_Boot == 1) {
                    m_SettingsWidget->tlStatus->setText("HDD -> CD-ROM -> LAN -> FDD");
                    m_SettingsWidget->pl1->setPixmap(SmallIcon("hdd_unmount", 32));
                    m_SettingsWidget->pl2->setPixmap(SmallIcon("cdrom_unmount", 32));
                    m_SettingsWidget->pl3->setPixmap(SmallIcon("nfs_unmount", 32));
                    m_SettingsWidget->pl4->setPixmap(SmallIcon("3floppy_unmount", 32));
                } else
                if (m_Boot == 2) {
                    m_SettingsWidget->tlStatus->setText("FDD -> CD-ROM -> LAN -> HDD");
                    m_SettingsWidget->pl1->setPixmap(SmallIcon("3floppy_unmount", 32));
                    m_SettingsWidget->pl2->setPixmap(SmallIcon("cdrom_unmount", 32));
                    m_SettingsWidget->pl3->setPixmap(SmallIcon("nfs_unmount", 32));
                    m_SettingsWidget->pl4->setPixmap(SmallIcon("hdd_unmount", 32));
                } else
                if (m_Boot == 3) {
                    m_SettingsWidget->tlStatus->setText("CD-ROM -> LAN -> HDD -> FDD");
                    m_SettingsWidget->pl1->setPixmap(SmallIcon("cdrom_unmount", 32));
                    m_SettingsWidget->pl2->setPixmap(SmallIcon("nfs_unmount", 32));
                    m_SettingsWidget->pl3->setPixmap(SmallIcon("hdd_unmount", 32));
                    m_SettingsWidget->pl4->setPixmap(SmallIcon("3floppy_unmount", 32));
                } else
                if (m_Boot == 4) {
                    m_SettingsWidget->tlStatus->setText("CD-ROM -> LAN -> FDD -> HDD");
                    m_SettingsWidget->pl1->setPixmap(SmallIcon("cdrom_unmount", 32));
                    m_SettingsWidget->pl2->setPixmap(SmallIcon("nfs_unmount", 32));
                    m_SettingsWidget->pl3->setPixmap(SmallIcon("3floppy_unmount", 32));
                    m_SettingsWidget->pl4->setPixmap(SmallIcon("hdd_unmount", 32));
                }
                else {
                    m_SettingsWidget->tlStatus->setText("HDD -> FDD -> CD-ROM -> LAN");
                    m_SettingsWidget->pl1->setPixmap(SmallIcon("hdd_unmount", 32));
                    m_SettingsWidget->pl2->setPixmap(SmallIcon("3floppy_unmount", 32));
                    m_SettingsWidget->pl3->setPixmap(SmallIcon("cdrom_unmount", 32));
                    m_SettingsWidget->pl4->setPixmap(SmallIcon("nfs_unmount", 32));
                }
                break;
        }
    }
    if (action == 1) {
        m_Snd--;
        if (m_Snd < 0) m_Snd = 1;
        toggleMute();
        m_StatusWidget->wsStatus->raiseWidget(m_Snd);
    }
    if ((action == 7) || (action == 8)) {
        (action == 7)? brightDown() : brightUp();
        if (m_Bright <= 7 && m_Bright >= 0)
            m_StatusWidget->wsStatus->raiseWidget(m_Bright + 4);
    }
    if (action == 10) {
        if ((m_Pad != -1) || (m_Mousepad != -1)) {
            m_Mousepad--;
            if (m_Mousepad < 0) m_Mousepad = 1;
            toggleMousePad();
            int mpad = ((m_Mousepad == 0)? 2 : 3);
            m_StatusWidget->wsStatus->raiseWidget(mpad);
        }
        else
            m_StatusWidget->wsStatus->raiseWidget(2);
    }
    if (action == 11 && m_SCIIface) {
        toggleSpeakerVolume();
        if (!m_Vol)
            m_StatusWidget->wsStatus->raiseWidget(m_Vol);
        else if (m_Vol == 3)
            m_StatusWidget->wsStatus->raiseWidget(m_Vol - 2);
        else
            m_StatusWidget->wsStatus->raiseWidget(m_Vol + 13);
    }
    if (action == 12) {
        toggleFan();
        if (m_Fan == -1)
            return;
        else if (m_Fan == 1)
            m_StatusWidget->wsStatus->raiseWidget(12);
        else if (m_Fan == 0)
            m_StatusWidget->wsStatus->raiseWidget(13);
    }
}

void ToshibaFnActions::toggleMute()
{
    DCOPRef kmixClient("kmix", "Mixer0");
    kmixClient.send("toggleMute", 0);
}

void ToshibaFnActions::lockScreen()
{
    DCOPRef kdesktopClient("kdesktop", "KScreensaverIface");
    kdesktopClient.send("lock()", 0);
}

void ToshibaFnActions::toggleBSM()
{
    if (m_BatType == 3)
        m_Driver->setBatterySaveMode(m_BatSave + 1);
    else
        m_Driver->setBatterySaveMode(m_BatSave);
}

void ToshibaFnActions::suspendToRAM()
{
    int res = KMessageBox::warningContinueCancel(0,
			i18n("Before continuing, be aware that ACPI Sleep States\n"
			 "are a work in progress and may or may not work on your computer.\n"
			 "Also make sure to unload problematic modules"), i18n("WARNING"));

#ifdef ENABLE_POWERSAVE
    if (res == KMessageBox::Continue)
        res = dbusSendSimpleMessage(ACTION_MESSAGE, "SuspendToRam");

    switch (res) {
        case REPLY_SUCCESS:
            kdDebug() << "KToshiba: Suspending To RAM (using powersave)" << endl;
            return;
        case REPLY_DISABLED:
            KPassivePopup::message(i18n("WARNING"),
			i18n("Suspend To RAM disabled by administrator."),
			SmallIcon("messagebox_warning", 20), m_Parent, i18n("WARNING"), 5000);
            return;
        default:
            KPassivePopup::message(i18n("WARNING"),
			i18n("The powersave daemon must be running in the background for a Suspend To RAM."),
			SmallIcon("messagebox_warning", 20), m_Parent, i18n("WARNING"), 5000);
            return;
    }
#else // ENABLE_POWERSAVE
    QString helper = KStandardDirs::findExe("ktosh_helper");
    if (helper.isEmpty())
        helper = KStandardDirs::findExe("klaptop_acpi_helper");
    if (helper.isEmpty()) {
        KMessageBox::sorry(0, i18n("Could not Suspend To RAM because ktosh_helper cannot be found.\n"
			   "Please make sure that it is installed correctly."),
			   i18n("STR"));
        return;
    }

    if (res == KMessageBox::Continue) {
        kdDebug() << "KToshiba: Suspending To RAM (using ACPI)" << endl;
        KProcess proc;
        proc << helper << "--suspend";
        proc.start(KProcess::DontCare);
        proc.detach();
    }
#endif // ENABLE_POWERSAVE
}

void ToshibaFnActions::suspendToDisk()
{
    int res = KMessageBox::warningContinueCancel(0,
			i18n("Before continuing, be aware that ACPI Sleep States\n"
			 "are a work in progress and may or may not work on your computer.\n"
			 "Also make sure to unload problematic modules"), i18n("WARNING"));

#ifdef ENABLE_POWERSAVE // ENABLE_POWERSAVE
    if (res == KMessageBox::Continue)
        res = dbusSendSimpleMessage(ACTION_MESSAGE, "SuspendToDisk");

    switch (res) {
        case REPLY_SUCCESS:
            kdDebug() << "KToshiba: Suspending To Disk (using powersave)" << endl;
            return;
        case REPLY_DISABLED:
            KPassivePopup::message(i18n("WARNING"),
			i18n("Suspend To Disk disabled by administrator."),
			SmallIcon("messagebox_warning", 20), m_Parent, i18n("WARNING"), 5000);
            return;
        default:
            KPassivePopup::message(i18n("WARNING"),
			i18n("The powersave daemon must be running in the background for a Suspend To Disk."),
			SmallIcon("messagebox_warning", 20), m_Parent, i18n("WARNING"), 5000);
           return;
    }
#else // ENABLE_POWERSAVE
    QString helper = KStandardDirs::findExe("ktosh_helper");
    if (helper.isEmpty())
        helper = KStandardDirs::findExe("klaptop_acpi_helper");
    if (helper.isEmpty()) {
        KMessageBox::sorry(0, i18n("Could not Suspend To Disk because ktosh_helper cannot be found.\n"
			   "Please make sure that it is installed correctly."),
			   i18n("STD"));
        return;
    }

    if (res == KMessageBox::Continue) {
        kdDebug() << "KToshiba: Suspending To Disk (using ACPI)" << endl;
        KProcess proc;
        proc << helper << "--hibernate";
        proc.start(KProcess::DontCare);
        proc.detach();
    }
#endif // ENABLE_POWERSAVE
}

void ToshibaFnActions::toggleVideo()
{
    m_Driver->setVideo(m_Video);
}

void ToshibaFnActions::brightDown()
{
    if (m_Bright != m_Driver->getBrightness())
        m_Bright = m_Driver->getBrightness();

    m_Driver->setBrightness(--m_Bright);
}

void ToshibaFnActions::brightUp()
{
    if (m_Bright != m_Driver->getBrightness())
        m_Bright = m_Driver->getBrightness();

    m_Driver->setBrightness(++m_Bright);
}

void ToshibaFnActions::toggleWireless()
{
    if (m_Wireless == -1) return;

    int ws = m_Driver->getWirelessSwitch();
    if (ws == 1) {
        m_Wireless--;
        if (m_Wireless < 0) m_Wireless = 1;
    } else
        return;

    m_Driver->setWirelessPower(m_Wireless);
    QString w = ((m_Wireless == 1)? i18n("activated") : i18n("deactivated"));
    KPassivePopup::message(i18n("KToshiba"),
			   i18n("Wireless interface %1").arg(w),
			   SmallIcon("kwifimanager", 20), m_Parent, i18n("Wireless"), 4000);
}

void ToshibaFnActions::toggleMousePad()
{
#ifdef ENABLE_SYNAPTICS
    if (m_Pad == -1) {
        if (m_Mousepad == -1)
            return;

        Pad::setParam(TOUCHPADOFF, ((double)m_Mousepad));
    } else
#endif // ENABLE_SYNAPTICS
    if (m_Pad >= 0 && m_SCIIface)
        m_Driver->setPointingDevice(m_Mousepad);
}

void ToshibaFnActions::toggleSpeakerVolume()
{
    m_Vol = m_Driver->getSpeakerVolume();
    if (m_Vol == -1)
        return;

    m_Vol++;
    if (m_Vol > 3) m_Vol = 0;
    m_Driver->setSpeakerVolume(m_Vol);
}

void ToshibaFnActions::toggleFan()
{
    int res = m_Driver->getFan();

    if (res < 0) {
        m_Fan = -1;
        return;
    }
    m_Fan = (res > 0)? 1 : 0;
    m_Driver->setFan(m_Fan);
}

void ToshibaFnActions::toggleBootMethod()
{
    m_Driver->setBootMethod(m_Boot);
}

void ToshibaFnActions::toogleBackLight()
{
    int bl = m_Driver->getBackLight();

    if (bl == -1)
        return;
    else if (bl == 1)
        m_Driver->setBackLight(0);
    else if (bl == 0)
        m_Driver->setBackLight(1);
}

void ToshibaFnActions::toggleBluetooth()
{
    int bt = m_Driver->getBluetoothPower();

    if (bt == -1)
        return;
    else if (bt == 1)
        m_Driver->setBluetoothPower(0);
    else if (bt == 0)
        m_Driver->setBluetoothPower(1);
    QString w = ((bt == 0)? i18n("activated") : i18n("deactivated"));
    KPassivePopup::message(i18n("KToshiba"), i18n("Bluetooth device %1").arg(w),
			   SmallIcon("kdebluetooth", 20), m_Parent, i18n("Bluetooth"), 4000);
}

void ToshibaFnActions::toggleEthernet()
{
    int eth = m_Driver->getLANController();

    if (eth == -1)
        return;
    else if (eth == 1)
        m_Driver->setLANController(0);
    else if (eth == 0)
        m_Driver->setLANController(1);
    QString w = ((eth == 0)? i18n("activated") : i18n("deactivated"));
    KPassivePopup::message(i18n("KToshiba"), i18n("Ethernet device %1").arg(w),
			   SmallIcon("messagebox_info", 20), m_Parent, i18n("Ethernet"), 4000);
}


#include "toshibafnactions.moc"
