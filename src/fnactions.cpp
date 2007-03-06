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

#include "fnactions.h"
#include "suspend.h"

#include <qapplication.h>
#include <qwidgetstack.h>
#include <qlabel.h>
#include <qlineedit.h>

#include <klocale.h>
#include <dcopref.h>
#include <kiconloader.h>
#include <kprogress.h>
#include <kpassivepopup.h>
#include <kconfig.h>
#include <kprocess.h>

#include "settingswidget.h"
#include "statuswidget.h"
#include "cmdwidget.h"

FnActions::FnActions(QWidget *parent)
    : QObject( parent ),
      m_Parent( parent )
{
    m_SettingsWidget = new SettingsWidget(0, "Screen Indicator", Qt::WX11BypassWM);
    m_SettingsWidget->setFocusPolicy(QWidget::NoFocus);
    m_StatusWidget = new StatusWidget(0, "Screen Indicator Two", Qt::WX11BypassWM);
    m_StatusWidget->setFocusPolicy(QWidget::NoFocus);
    m_CmdWidget = new CmdWidget(0, "Run Command Indicator");
    m_Suspend = new Suspend(parent);

    m_Title = i18n("KToshiba");
    m_Activated = i18n("activated");
    m_Deactivated = i18n("deactivated");
    m_NotSupported = i18n("Function Not Supported");

    m_Popup = 0;
    m_Duration = 4000;
    m_FnKey = -1;

    connect( m_CmdWidget, SIGNAL( saved() ), SLOT( saveCmd() ) );
}

FnActions::~FnActions()
{
    m_Parent = NULL;
    delete m_Suspend; m_Suspend = NULL;
    delete m_SettingsWidget; m_SettingsWidget = NULL;
    delete m_StatusWidget; m_StatusWidget = NULL;
}

void FnActions::showWidget(int widget, int key)
{
    if (m_Popup == 0) {
        QRect r = QApplication::desktop()->geometry();
        if (widget == 1) {
            m_StatusWidget->move(r.center() -
                QPoint(m_StatusWidget->width() / 2, m_StatusWidget->height() / 2));
            m_StatusWidget->show();
        } else
        if (widget == 2) {
            m_SettingsWidget->move(r.center() -
                QPoint(m_SettingsWidget->width() / 2, m_SettingsWidget->height() / 2));
            m_SettingsWidget->show();
        }
        m_Popup = key & 0x17f;
    }
}

void FnActions::updateWidget(int action, int type, int extra)
{
    if (action == 3) {
        if (extra == 3) {
            m_SettingsWidget->wsSettings->raiseWidget(5);
            m_SettingsWidget->plLongL->setFrameShape(QLabel::NoFrame);
            m_SettingsWidget->plNormalL->setFrameShape(QLabel::NoFrame);
            m_SettingsWidget->plFullL->setFrameShape(QLabel::NoFrame);
        } else {
            m_SettingsWidget->wsSettings->raiseWidget(0);
            m_SettingsWidget->plUser->setFrameShape(QLabel::NoFrame);
            m_SettingsWidget->plLow->setFrameShape(QLabel::NoFrame);
            m_SettingsWidget->plFull->setFrameShape(QLabel::NoFrame);
        }
        switch (type) {
            case -1:
                m_SettingsWidget->tlStatus->setText(m_NotSupported);
                break;
            case 0:
                m_SettingsWidget->tlStatus->setText(i18n("User Settings"));
                m_SettingsWidget->plUser->setFrameShape(QLabel::PopupPanel);
                break;
            case 1:
                m_SettingsWidget->tlStatus->setText( (extra == 3)? i18n("Long Life")
                    : i18n("Low Power") );
                (extra == 3)? m_SettingsWidget->plLongL->setFrameShape(QLabel::PopupPanel)
                    : m_SettingsWidget->plLow->setFrameShape(QLabel::PopupPanel);
                break;
            case 2:
                m_SettingsWidget->tlStatus->setText( (extra == 3)? i18n("Normal Life")
                    : i18n("Full Power") );
                (extra == 3)? m_SettingsWidget->plNormalL->setFrameShape(QLabel::PopupPanel)
                    : m_SettingsWidget->plFull->setFrameShape(QLabel::PopupPanel);
                break;
            case 3:
                m_SettingsWidget->tlStatus->setText(i18n("Full Life"));
                m_SettingsWidget->plFullL->setFrameShape(QLabel::PopupPanel);
                break;
        }
        return;
    }
    if (action == 6) {
        m_SettingsWidget->wsSettings->raiseWidget(1);
        m_SettingsWidget->plLCD->setFrameShape(QLabel::NoFrame);
        m_SettingsWidget->plCRT->setFrameShape(QLabel::NoFrame);
        m_SettingsWidget->plLCDCRT->setFrameShape(QLabel::NoFrame);
        m_SettingsWidget->plTV->setFrameShape(QLabel::NoFrame);
        switch (type) {
            case -1:
                m_SettingsWidget->tlStatus->setText(m_NotSupported);
                m_SettingsWidget->plLCD->setEnabled(false);
                m_SettingsWidget->plCRT->setEnabled(false);
                m_SettingsWidget->plLCDCRT->setEnabled(false);
                m_SettingsWidget->plTV->setEnabled(false);
                break;
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
        return;
    }
    if (action == 13) {
        m_SettingsWidget->wsSettings->raiseWidget((type == 5)? 3 : 2);
        switch (type) {
            case -1:
                m_SettingsWidget->tlStatus->setText(m_NotSupported);
                break;
            case 1:
                m_SettingsWidget->pl1->setEnabled(false);
                m_SettingsWidget->pl4->setEnabled(false);
                if (!extra) {
                    m_SettingsWidget->tlStatus->setText("FDD -> HDD");
                    m_SettingsWidget->pl2->setPixmap(SmallIcon("3floppy_unmount", 32));
                    m_SettingsWidget->pl3->setPixmap(SmallIcon("hdd_unmount", 32));
                }
                else {
                    m_SettingsWidget->tlStatus->setText("HDD -> FDD");
                    m_SettingsWidget->pl2->setPixmap(SmallIcon("hdd_unmount", 32));
                    m_SettingsWidget->pl3->setPixmap(SmallIcon("3floppy_unmount", 32));
                }
                break;
            case 3:
                m_SettingsWidget->pl1->setEnabled(false);
                m_SettingsWidget->pl4->setEnabled(false);
                if (!extra) {
                    m_SettingsWidget->tlStatus->setText("FDD -> Built-in HDD");
                    m_SettingsWidget->pl2->setPixmap(SmallIcon("3floppy_unmount", 32));
                    m_SettingsWidget->pl3->setPixmap(SmallIcon("hdd_unmount", 32));
                } else
                if (extra == 1) {
                    m_SettingsWidget->tlStatus->setText("Built-in HDD -> FDD");
                    m_SettingsWidget->pl2->setPixmap(SmallIcon("hdd_unmount", 32));
                    m_SettingsWidget->pl3->setPixmap(SmallIcon("3floppy_unmount", 32));
                } else
                if (extra == 2) {
                    m_SettingsWidget->tlStatus->setText("FDD -> Second HDD");
                    m_SettingsWidget->pl2->setPixmap(SmallIcon("3floppy_unmount", 32));
                    m_SettingsWidget->pl3->setPixmap(SmallIcon("hdd_unmount", 32));
                }
                else {
                    m_SettingsWidget->tlStatus->setText("Second HDD -> FDD");
                    m_SettingsWidget->pl2->setPixmap(SmallIcon("hdd_unmount", 32));
                    m_SettingsWidget->pl3->setPixmap(SmallIcon("3floppy_unmount", 32));
                }
                break;
            case 5:
                if (!extra) {
                    m_SettingsWidget->tlStatus->setText("FDD -> HDD -> CD-ROM");
                    m_SettingsWidget->pl1_2->setPixmap(SmallIcon("3floppy_unmount", 32));
                    m_SettingsWidget->pl2_2->setPixmap(SmallIcon("hdd_unmount", 32));
                    m_SettingsWidget->pl3_2->setPixmap(SmallIcon("cdrom_unmount", 32));
                } else
                if (extra == 1) {
                    m_SettingsWidget->tlStatus->setText("HDD -> FDD -> CD-ROM");
                    m_SettingsWidget->pl1_2->setPixmap(SmallIcon("hdd_unmount", 32));
                    m_SettingsWidget->pl2_2->setPixmap(SmallIcon("3floppy_unmount", 32));
                    m_SettingsWidget->pl3_2->setPixmap(SmallIcon("cdrom_unmount", 32));
                } else
                if (extra == 2) {
                    m_SettingsWidget->tlStatus->setText("FDD -> CD-ROM -> HDD");
                    m_SettingsWidget->pl1_2->setPixmap(SmallIcon("3floppy_unmount", 32));
                    m_SettingsWidget->pl2_2->setPixmap(SmallIcon("cdrom_unmount", 32));
                    m_SettingsWidget->pl3_2->setPixmap(SmallIcon("hdd_unmount", 32));
                } else
                if (extra == 3) {
                    m_SettingsWidget->tlStatus->setText("HDD -> CD-ROM -> FDD");
                    m_SettingsWidget->pl1_2->setPixmap(SmallIcon("hdd_unmount", 32));
                    m_SettingsWidget->pl2_2->setPixmap(SmallIcon("cdrom_unmount", 32));
                    m_SettingsWidget->pl3_2->setPixmap(SmallIcon("3floppy_unmount", 32));
                } else
                if (extra == 4) {
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
                if (!extra) {
                    m_SettingsWidget->tlStatus->setText("FDD -> HDD -> CD-ROM -> LAN");
                    m_SettingsWidget->pl1->setPixmap(SmallIcon("3floppy_unmount", 32));
                    m_SettingsWidget->pl2->setPixmap(SmallIcon("hdd_unmount", 32));
                    m_SettingsWidget->pl3->setPixmap(SmallIcon("cdrom_unmount", 32));
                    m_SettingsWidget->pl4->setPixmap(SmallIcon("nfs_unmount", 32));
                } else
                if (extra == 1) {
                    m_SettingsWidget->tlStatus->setText("HDD -> CD-ROM -> LAN -> FDD");
                    m_SettingsWidget->pl1->setPixmap(SmallIcon("hdd_unmount", 32));
                    m_SettingsWidget->pl2->setPixmap(SmallIcon("cdrom_unmount", 32));
                    m_SettingsWidget->pl3->setPixmap(SmallIcon("nfs_unmount", 32));
                    m_SettingsWidget->pl4->setPixmap(SmallIcon("3floppy_unmount", 32));
                } else
                if (extra == 2) {
                    m_SettingsWidget->tlStatus->setText("FDD -> CD-ROM -> LAN -> HDD");
                    m_SettingsWidget->pl1->setPixmap(SmallIcon("3floppy_unmount", 32));
                    m_SettingsWidget->pl2->setPixmap(SmallIcon("cdrom_unmount", 32));
                    m_SettingsWidget->pl3->setPixmap(SmallIcon("nfs_unmount", 32));
                    m_SettingsWidget->pl4->setPixmap(SmallIcon("hdd_unmount", 32));
                } else
                if (extra == 3) {
                    m_SettingsWidget->tlStatus->setText("CD-ROM -> LAN -> HDD -> FDD");
                    m_SettingsWidget->pl1->setPixmap(SmallIcon("cdrom_unmount", 32));
                    m_SettingsWidget->pl2->setPixmap(SmallIcon("nfs_unmount", 32));
                    m_SettingsWidget->pl3->setPixmap(SmallIcon("hdd_unmount", 32));
                    m_SettingsWidget->pl4->setPixmap(SmallIcon("3floppy_unmount", 32));
                } else
                if (extra == 4) {
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
        return;
    }
    if (action == 22) {
        m_SettingsWidget->wsSettings->raiseWidget(4);
        m_SettingsWidget->batteryKPB->setValue((type == -1 || type == -2)? 0 : type);
        if (type == -1)
            m_SettingsWidget->tlStatus->setText(m_NotSupported);
        else if (type == -2)
            m_SettingsWidget->tlStatus->setText(i18n("Battery Not Present"));
        else
            m_SettingsWidget->tlStatus->setText(i18n("Battery Status"));
        return;
    }
    if (action == 1) {
        m_StatusWidget->wsStatus->raiseWidget(type);
        return;
    }
    if ((action == 7) || (action == 8)) {
        m_StatusWidget->wsStatus->raiseWidget(type + 4);
        return;
    }
    if (action == 10) {
        if (type == -1)
            m_StatusWidget->wsStatus->raiseWidget(3);
        else {
#ifdef ENABLE_SYNAPTICS
            m_StatusWidget->wsStatus->raiseWidget(((type == 0)? 2 : 3));
#else // ENABLE_SYNAPTICS
            m_StatusWidget->wsStatus->raiseWidget(((type == 0)? 3 : 2));
#endif // ENABLE_SYNAPTICS
        }
        return;
    }
    if (action == 11) {
        if (type == -1)
            m_StatusWidget->wsStatus->raiseWidget(0);
        else if (!type)
            m_StatusWidget->wsStatus->raiseWidget(type);
        else if (type == 3)
            m_StatusWidget->wsStatus->raiseWidget(type - 2);
        else
            m_StatusWidget->wsStatus->raiseWidget(type + 13);
        return;
    }
    if (action == 12)
        m_StatusWidget->wsStatus->raiseWidget((type != 1)? 13 : 12);
}

void FnActions::hideWidgets()
{
    m_SettingsWidget->hide();
    m_StatusWidget->hide();
    m_Popup = 0;
}

void FnActions::toggleMute(int *snd)
{
    DCOPRef kmixClient("kmix", "Mixer0");
    DCOPReply reply = kmixClient.call("mute", 0);
    if (reply.isValid()) {
        bool res = reply;
        *snd = (res == true)? 1 : 0;
    }

    kmixClient.send("toggleMute", 0);
}

void FnActions::lockScreen()
{
    DCOPRef kdesktopClient("kdesktop", "KScreensaverIface");
    kdesktopClient.send("lock()", 0);
}

void FnActions::runCommand(int key)
{
    KConfig cfg("ktoshibarc");
    cfg.setGroup("Fn_Key");
    QString cmd;
    switch (key) {
        case 0x101:	// Fn-Esc
        case 113:
            cmd = cfg.readEntry("Fn_Esc_Cmd");
            m_FnKey = 0;
            break;
        case 0x13b:	// Fn-F1
        case 0x1d2:
            cmd = cfg.readEntry("Fn_F1_Cmd");
            m_FnKey = 1;
            break;
        case 0x13c:	// Fn-F2
        case 148:
            cmd = cfg.readEntry("Fn_F2_Cmd");
            m_FnKey = 2;
            break;
        case 0x13d:	// Fn-F3
        case 142:
            cmd = cfg.readEntry("Fn_F3_Cmd");
            m_FnKey = 3;
            break;
        case 0x13e:	// Fn-F4
        case 205:
            cmd = cfg.readEntry("Fn_F4_Cmd");
            m_FnKey = 4;
            break;
        case 0x13f:	// Fn-F5
        case 227:
            cmd = cfg.readEntry("Fn_F5_Cmd");
            m_FnKey = 5;
            break;
        case 0x140:	// Fn-F6
        case 224:
            cmd = cfg.readEntry("Fn_F6_Cmd");
            m_FnKey = 6;
            break;
        case 0x141:	// Fn-F7
        case 225:
            cmd = cfg.readEntry("Fn_F7_Cmd");
            m_FnKey = 7;
            break;
        case 0x142:	// Fn-F8
        case 238:
            cmd = cfg.readEntry("Fn_F8_Cmd");
            m_FnKey = 8;
            break;
        case 0x143:	// Fn-F9
        case 0x1da:
            cmd = cfg.readEntry("Fn_F9_Cmd");
            m_FnKey = 9;
    }
    if (cmd.isEmpty())
        m_CmdWidget->show();
    KProcess proc;
    proc << cmd;
    proc.start(KProcess::DontCare);
    proc.detach();
}

void FnActions::saveCmd()
{
    QString cmd = m_CmdWidget->Commandle->text();
    KConfig cfg("ktoshibarc");
    cfg.setGroup("Fn_Key");
    switch (m_FnKey) {
        case 0:
            cfg.writeEntry("Fn_Esc_Cmd", cmd);
            break;
        case 1:
            cfg.writeEntry("Fn_F1_Cmd", cmd);
            break;
        case 2:
            cfg.writeEntry("Fn_F2_Cmd", cmd);
            break;
        case 3:
            cfg.writeEntry("Fn_F3_Cmd", cmd);
            break;
        case 4:
            cfg.writeEntry("Fn_F4_Cmd", cmd);
            break;
        case 5:
            cfg.writeEntry("Fn_F5_Cmd", cmd);
            break;
        case 6:
            cfg.writeEntry("Fn_F6_Cmd", cmd);
            break;
        case 7:
            cfg.writeEntry("Fn_F7_Cmd", cmd);
            break;
        case 8:
            cfg.writeEntry("Fn_F8_Cmd", cmd);
            break;
        case 9:
            cfg.writeEntry("Fn_F9_Cmd", cmd);
            break;
    }
    cfg.sync();
}

void FnActions::showPassiveMsg(int state, popuptype type)
{
    QString w = (state == 1)? m_Activated : m_Deactivated;
    switch (type) {
        case Bluetooth:
            m_Text = i18n("Bluetooth device %1").arg(w, 0);
            m_Icon = SmallIcon("kdebluetooth", 20);
            break;
        case Ethernet:
            m_Text = i18n("Ethernet device %1").arg(w, 0);
            m_Icon = SmallIcon("messagebox_info", 20);
            break;
        case Wireless:
            m_Text = i18n("Wireless interface %1").arg(w, 0);
            m_Icon = SmallIcon("kwifimanager", 20);
            break;
        default:
            return;
    }

    KPassivePopup::message(m_Title, m_Text, m_Icon, m_Parent, 0, m_Duration);
}


#include "fnactions.moc"
