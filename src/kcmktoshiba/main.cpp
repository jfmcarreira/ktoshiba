/***************************************************************************
 *   main.cpp                                                              *
 *                                                                         *
 *   Copyright (C) 2004-2006 by Azael Avalos                               *
 *   coproscefalo@gmail.com                                                *
 *                                                                         *
 *   Based on kcm_kvaio                                                    *
 *   Copyright (C) 2003 Mirko Boehm (mirko@kde.org)                        *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
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

#include <qlayout.h>
#include <qtabwidget.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qslider.h>

#include <kgenericfactory.h>
#include <kaboutdata.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kprogress.h>
#include <kled.h>
#include <kiconloader.h>

#include "../ktoshibasmminterface.h"
#include "../ktoshibaomnibookinterface.h"
#include "../ktoshibaprocinterface.h"
#include "kcmtoshiba_general.h"
#include "bsmwidget.h"
#include "main.h"
#include "main.moc"

#define CONFIG_FILE "ktoshibarc"
#define BSM_CONFIG "ktoshibabsmrc"

typedef KGenericFactory<KCMToshibaModule, QWidget> KCMToshibaModuleFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_ktoshibam, KCMToshibaModuleFactory("kcmktoshiba") )

KCMToshibaModule::KCMToshibaModule(QWidget *parent, const char *name, const QStringList &)
    : KCModule(KCMToshibaModuleFactory::instance(), parent, name)
{
    KAboutData *about = new KAboutData(
            I18N_NOOP("kcmktoshiba"),
            I18N_NOOP("KDE Control Module for Toshiba Laptops"), 0, 0,
            KAboutData::License_GPL,
            "(c) 2004-2006 Azael Avalos");

    about->addAuthor("Azael Avalos", I18N_NOOP("Original author"), "coproscefalo@gmail.com");
    setAboutData( about );

    QVBoxLayout* layout = new QVBoxLayout(this);
    m_KCMKToshibaGeneral = new KCMKToshibaGeneral(this);
    layout->addWidget( m_KCMKToshibaGeneral );
    layout->addStretch();

    load();

    m_SCIFace = false;
    m_HCIFace = false;
    m_Omnibook = false;
    m_Hotkeys = false;
    m_Hardware = false;
    m_Init = false;
    m_AC = -1;
    m_BSM = -1;
    m_IFaceErr = 0;

    m_BSMWidget = new BSMWidget(this, "BSM Widget", WType_Dialog);
    m_ProcIFace = new KToshibaProcInterface(this);
    m_SMMIFace = new KToshibaSMMInterface(this);
    m_SCIFace = m_SMMIFace->openSCInterface(&m_IFaceErr);
    if (!m_SCIFace)
        m_SCIFace = (m_IFaceErr == SCI_ALREADY_OPEN)? true : false;
    if (m_SCIFace) {
        m_Hardware = true;
        // Let's hide Fn-# Line Edit boxes
        m_KCMKToshibaGeneral->FnEscle->hide();
        m_KCMKToshibaGeneral->FnF1le->hide();
        m_KCMKToshibaGeneral->FnF2le->hide();
        m_KCMKToshibaGeneral->FnF3le->hide();
        m_KCMKToshibaGeneral->FnF4le->hide();
        m_KCMKToshibaGeneral->FnF5le->hide();
        m_KCMKToshibaGeneral->FnF6le->hide();
        m_KCMKToshibaGeneral->FnF7le->hide();
        m_KCMKToshibaGeneral->FnF8le->hide();
        m_KCMKToshibaGeneral->FnF9le->hide();
    }
    m_HCIFace = (m_SMMIFace->getBrightness() == -1)? false : true;
    if (m_HCIFace) {
        m_AC = m_SMMIFace->acPowerStatus();
        m_Hotkeys = ((m_SMMIFace->getSystemEvent() == -1)? false : true);
        m_KCMKToshibaGeneral->fnComboBox_5->setEnabled(false);		// Fn-F5
    }
    m_KCMKToshibaGeneral->configTabWidget->setTabEnabled(
            m_KCMKToshibaGeneral->configTabWidget->page(2), m_SCIFace);	// BSM

    if (!m_HCIFace && !m_SCIFace) {
        delete m_SMMIFace; m_SMMIFace = NULL;
        m_OmniIFace = new KToshibaOmnibookInterface(this);
        m_Omnibook = m_OmniIFace->checkOmnibook();
    }
    if (m_Omnibook) {
        m_KCMKToshibaGeneral->configTabWidget->setTabEnabled(
                m_KCMKToshibaGeneral->configTabWidget->page(2), false);	// BSM
        m_KCMKToshibaGeneral->configTabWidget->setTabEnabled(
                m_KCMKToshibaGeneral->configTabWidget->page(3), false);	// Hardware
        m_KCMKToshibaGeneral->fnComboBox_6->setEnabled(false);		// Fn-F6
        m_KCMKToshibaGeneral->fnComboBox_7->setEnabled(false);		// Fn-F7
        m_AC = m_OmniIFace->omnibookAC();
        QString keyhandler = KStandardDirs::findExe("ktosh_keyhandler");
        m_Hotkeys = (keyhandler.isEmpty())? false : true;
    }

    if (!m_SCIFace && !m_HCIFace && !m_Omnibook) {
        m_KCMKToshibaGeneral->tlOff->show();
        m_KCMKToshibaGeneral->frameMain->setEnabled(false);
        setButtons(buttons() & ~Default);
    } else
    if (m_SCIFace || m_HCIFace || m_Omnibook) {
        m_KCMKToshibaGeneral->tlOff->hide();
        m_KCMKToshibaGeneral->frameMain->setEnabled(true);
        m_KCMKToshibaGeneral->configTabWidget->setTabEnabled(
                m_KCMKToshibaGeneral->configTabWidget->page(1), m_Hotkeys);	// Fn-Key
        m_KCMKToshibaGeneral->configTabWidget->setTabEnabled(
                m_KCMKToshibaGeneral->configTabWidget->page(3), m_Hardware);	// Hardware

        connect( m_KCMKToshibaGeneral, SIGNAL( changed() ), SLOT( configChanged() ) );
        connect( m_KCMKToshibaGeneral, SIGNAL( hwChanged() ), SLOT( hwChanged() ) );
        connect( m_KCMKToshibaGeneral, SIGNAL( hpSignal(int) ), SLOT( bsmLoad(int) ) );
        connect( m_KCMKToshibaGeneral, SIGNAL( dvdSignal(int) ), SLOT( bsmLoad(int) ) );
        connect( m_KCMKToshibaGeneral, SIGNAL( presSignal(int) ), SLOT( bsmLoad(int) ) );
        connect( m_BSMWidget, SIGNAL( okPressed() ), SLOT( bsmChanged() ) );
        m_Timer = new QTimer(this);
        connect( m_Timer, SIGNAL( timeout() ), SLOT( timeout() ) );
        m_Timer->start(210);
    }
};


void KCMToshibaModule::load()
{
    int tmp = -1;
    KConfig config(CONFIG_FILE);
    // Fn-Key Tab
    config.setGroup("Fn_Key");
    // Rudimentary way of acquiring the command... I don't like it...
    tmp = config.readNumEntry("Fn_Esc", 1);
    m_KCMKToshibaGeneral->fnComboBox->setCurrentItem( tmp );
    if (tmp == 17) {
        m_KCMKToshibaGeneral->FnEscle->setText( config.readEntry("Fn_Esc_Cmd") );
        m_KCMKToshibaGeneral->FnEscle->show();
    }
    tmp = config.readNumEntry("Fn_F1", 2);
    m_KCMKToshibaGeneral->fnComboBox_1->setCurrentItem( tmp );
    if (tmp == 17) {
        m_KCMKToshibaGeneral->FnF1le->setText( config.readEntry("Fn_F1_Cmd") );
        m_KCMKToshibaGeneral->FnF1le->show();
    }
    tmp = config.readNumEntry("Fn_F2", 3);
    m_KCMKToshibaGeneral->fnComboBox_2->setCurrentItem( tmp );
    if (tmp == 17) {
        m_KCMKToshibaGeneral->FnF2le->setText( config.readEntry("Fn_F2_Cmd") );
        m_KCMKToshibaGeneral->FnF2le->show();
    }
    tmp = config.readNumEntry("Fn_F3", 4);
    m_KCMKToshibaGeneral->fnComboBox_3->setCurrentItem( tmp );
    if (tmp == 17) {
        m_KCMKToshibaGeneral->FnF3le->setText( config.readEntry("Fn_F3_Cmd") );
        m_KCMKToshibaGeneral->FnF3le->show();
    }
    tmp = config.readNumEntry("Fn_F4", 5);
    m_KCMKToshibaGeneral->fnComboBox_4->setCurrentItem( tmp );
    if (tmp == 17) {
        m_KCMKToshibaGeneral->FnF4le->setText( config.readEntry("Fn_F4_Cmd") );
        m_KCMKToshibaGeneral->FnF4le->show();
    }
    tmp = config.readNumEntry("Fn_F6", 7);
    m_KCMKToshibaGeneral->fnComboBox_6->setCurrentItem( tmp );
    if (tmp == 17) {
        m_KCMKToshibaGeneral->FnF6le->setText( config.readEntry("Fn_F6_Cmd") );
        m_KCMKToshibaGeneral->FnF6le->show();
    }
    tmp = config.readNumEntry("Fn_F7", 8);
    m_KCMKToshibaGeneral->fnComboBox_7->setCurrentItem( tmp );
    if (tmp == 17) {
        m_KCMKToshibaGeneral->FnF7le->setText( config.readEntry("Fn_F7_Cmd") );
        m_KCMKToshibaGeneral->FnF7le->show();
    }
    tmp = config.readNumEntry("Fn_F8", 9);
    m_KCMKToshibaGeneral->fnComboBox_8->setCurrentItem( tmp );
    if (tmp == 17) {
        m_KCMKToshibaGeneral->FnF8le->setText( config.readEntry("Fn_F8_Cmd") );
        m_KCMKToshibaGeneral->FnF8le->show();
    }
    tmp = config.readNumEntry("Fn_F9", 10);
    m_KCMKToshibaGeneral->fnComboBox_9->setCurrentItem( tmp );
    if (tmp == 17) {
        m_KCMKToshibaGeneral->FnF9le->setText( config.readEntry("Fn_F9_Cmd") );
        m_KCMKToshibaGeneral->FnF9le->show();
    }
    // Hardware Tab
    config.setGroup("Hardware");
    m_KCMKToshibaGeneral->bgDeviceConfig->setButton
            ( config.readNumEntry("Device_Config", 1) );
    m_KCMKToshibaGeneral->bgNetBoot->setButton
            ( config.readNumEntry("Network_Boot_Protocol", 0) );
    m_KCMKToshibaGeneral->bgParrallelPortMode->setButton
            ( config.readNumEntry("Parallel_Port_Mode", 0) );
    m_KCMKToshibaGeneral->bgUSBFDDEmul->setButton
            ( config.readNumEntry("USB_FDD_Emul", 0) );
    m_KCMKToshibaGeneral->bgUSBEmul->setButton
            ( config.readNumEntry("USB_KBMouse_Emul", 0) );
    m_KCMKToshibaGeneral->bgWakeKeyboard->setButton
            ( config.readNumEntry("Wake_on_KB", 1) );
    m_KCMKToshibaGeneral->bgWakeonLAN->setButton
            ( config.readNumEntry("Wake_on_LAN", 0) );
    // General Options Tab
    config.setGroup("KToshiba");
    m_KCMKToshibaGeneral->audioComboBox->setCurrentItem
            ( config.readNumEntry("Audio_Player", 0) );
    m_KCMKToshibaGeneral->autostartCheckBox->setChecked
            ( config.readBoolEntry("AutoStart", true) );
    m_KCMKToshibaGeneral->btstartCheckBox->setChecked
            ( config.readBoolEntry("Bluetooth_Startup", true) );
}


void KCMToshibaModule::defaults()
{
    // General Options Tab
    m_KCMKToshibaGeneral->audioComboBox->setCurrentItem( 0 );
    m_KCMKToshibaGeneral->btstartCheckBox->setChecked( true );
    m_KCMKToshibaGeneral->autostartCheckBox->setChecked( true );
    // Fn-Key Tab
    m_KCMKToshibaGeneral->fnComboBox->setCurrentItem( 1 );
    m_KCMKToshibaGeneral->fnComboBox_1->setCurrentItem( 2 );
    m_KCMKToshibaGeneral->fnComboBox_2->setCurrentItem( 3 );
    m_KCMKToshibaGeneral->fnComboBox_3->setCurrentItem( 4 );
    m_KCMKToshibaGeneral->fnComboBox_4->setCurrentItem( 5 );
    m_KCMKToshibaGeneral->fnComboBox_5->setCurrentItem( 6 );
    m_KCMKToshibaGeneral->fnComboBox_6->setCurrentItem( 7 );
    m_KCMKToshibaGeneral->fnComboBox_7->setCurrentItem( 8 );
    m_KCMKToshibaGeneral->fnComboBox_8->setCurrentItem( 9 );
    m_KCMKToshibaGeneral->fnComboBox_9->setCurrentItem( 10 );
    // Hardware Tab
    m_KCMKToshibaGeneral->bgDeviceConfig->setButton( 1 );
    m_KCMKToshibaGeneral->bgParrallelPortMode->setButton( 0 );
    m_KCMKToshibaGeneral->bgWakeKeyboard->setButton( 1 );
    m_KCMKToshibaGeneral->bgUSBEmul->setButton( 0 );
    m_KCMKToshibaGeneral->bgUSBFDDEmul->setButton( 0 );
    m_KCMKToshibaGeneral->bgWakeonLAN->setButton( 0 );
    m_KCMKToshibaGeneral->bgNetBoot->setButton( 0 );
}


void KCMToshibaModule::save()
{
    int tmp = -1;
    KConfig config(CONFIG_FILE);
    // Fn-Key Tab
    config.setGroup("Fn_Key");
    tmp = m_KCMKToshibaGeneral->fnComboBox->currentItem();
    config.writeEntry("Fn_Esc", tmp);
    if (tmp == 17)
        config.writeEntry("Fn_Esc_Cmd", m_KCMKToshibaGeneral->FnEscle->text());
    tmp = m_KCMKToshibaGeneral->fnComboBox_1->currentItem();
    config.writeEntry("Fn_F1", tmp);
    if (tmp == 17)
        config.writeEntry("Fn_F1_Cmd", m_KCMKToshibaGeneral->FnF1le->text());
    tmp = m_KCMKToshibaGeneral->fnComboBox_2->currentItem();
    config.writeEntry("Fn_F2", tmp);
    if (tmp == 17)
        config.writeEntry("Fn_F2_Cmd", m_KCMKToshibaGeneral->FnF2le->text());
    tmp = m_KCMKToshibaGeneral->fnComboBox_3->currentItem();
    config.writeEntry("Fn_F3", tmp);
    if (tmp == 17)
        config.writeEntry("Fn_F3_Cmd", m_KCMKToshibaGeneral->FnF3le->text());
    tmp = m_KCMKToshibaGeneral->fnComboBox_4->currentItem();
    config.writeEntry("Fn_F4", tmp);
    if (tmp == 17)
        config.writeEntry("Fn_F4_Cmd", m_KCMKToshibaGeneral->FnF4le->text());
    tmp = m_KCMKToshibaGeneral->fnComboBox_6->currentItem();
    config.writeEntry("Fn_F6", tmp);
    if (tmp == 17)
        config.writeEntry("Fn_F6_Cmd", m_KCMKToshibaGeneral->FnF6le->text());
    tmp = m_KCMKToshibaGeneral->fnComboBox_7->currentItem();
    config.writeEntry("Fn_F7", tmp);
    if (tmp == 17)
        config.writeEntry("Fn_F7_Cmd", m_KCMKToshibaGeneral->FnF7le->text());
    tmp = m_KCMKToshibaGeneral->fnComboBox_8->currentItem();
    config.writeEntry("Fn_F8", tmp);
    if (tmp == 17)
        config.writeEntry("Fn_F8_Cmd", m_KCMKToshibaGeneral->FnF8le->text());
    tmp = m_KCMKToshibaGeneral->fnComboBox_9->currentItem();
    config.writeEntry("Fn_F9", tmp);
    if (tmp == 17)
        config.writeEntry("Fn_F9_Cmd", m_KCMKToshibaGeneral->FnF9le->text());
    config.sync();
    // Hardware Tab
    config.setGroup("Hardware");
    config.writeEntry("Device_Config",
            m_KCMKToshibaGeneral->bgDeviceConfig->selectedId());
    config.writeEntry("Network_Boot_Protocol",
            m_KCMKToshibaGeneral->bgNetBoot->selectedId());
    config.writeEntry("Parallel_Port_Mode",
            m_KCMKToshibaGeneral->bgParrallelPortMode->selectedId());
    config.writeEntry("USB_KBMouse_Emul",
            m_KCMKToshibaGeneral->bgUSBEmul->selectedId());
    config.writeEntry("USB_FDD_Emul",
            m_KCMKToshibaGeneral->bgUSBFDDEmul->selectedId());
    config.writeEntry("Wake_on_KB",
            m_KCMKToshibaGeneral->bgWakeKeyboard->selectedId());
    config.writeEntry("Wake_on_LAN",
            m_KCMKToshibaGeneral->bgWakeonLAN->selectedId());
    config.sync();
    // General Options Tab
    config.setGroup("KToshiba");
    config.writeEntry("Audio_Player",
            m_KCMKToshibaGeneral->audioComboBox->currentItem());
    config.writeEntry("AutoStart",
            m_KCMKToshibaGeneral->autostartCheckBox->isChecked());
    config.writeEntry("Bluetooth_Startup",
            m_KCMKToshibaGeneral->btstartCheckBox->isChecked());
    config.sync();
}


void KCMToshibaModule::configChanged()
{
    emit changed(true);
}


void KCMToshibaModule::hwChanged()
{
    m_SMMIFace->mHWChanged = true;
}


void KCMToshibaModule::bsmLoad(int bsm)
{
    int tmpproc = -1;

    m_BSM = bsm;
    KConfig config(BSM_CONFIG);
    switch (bsm) {
        case 0:			// High Power
            m_BSMWidget->setCaption(i18n("High Power - KToshiba"));
            config.setGroup("High_Power");
            // LCD Brightness
            m_BSMWidget->bright100s->setValue
                    ( config.readNumEntry("Bright1", 4) );
            m_BSMWidget->bright75s->setValue
                    ( config.readNumEntry("Bright2", 4) );
            m_BSMWidget->bright50s->setValue
                    ( config.readNumEntry("Bright3", 4) );
            m_BSMWidget->bright25s->setValue
                    ( config.readNumEntry("Bright4", 4) );
            // Cooling Method
            m_BSMWidget->coolingcb->setCurrentItem
                    ( config.readNumEntry("Cooling", 1) );
            // Display Auto Off
            m_BSMWidget->display100s->setValue
                    ( config.readNumEntry("Disp1", 4) );
            m_BSMWidget->display75s->setValue
                    ( config.readNumEntry("Disp2", 5) );
            m_BSMWidget->display50s->setValue
                    ( config.readNumEntry("Disp3", 6) );
            m_BSMWidget->display25s->setValue
                    ( config.readNumEntry("Disp4", 7) );
            // HDD Auto Off
            m_BSMWidget->hdd100s->setValue
                    ( config.readNumEntry("HDD1", 4) );
            m_BSMWidget->hdd75s->setValue
                    ( config.readNumEntry("HDD2", 5) );
            m_BSMWidget->hdd50s->setValue
                    ( config.readNumEntry("HDD3", 6) );
            m_BSMWidget->hdd25s->setValue
                    ( config.readNumEntry("HDD4", 7) );
            // Processing Speed
            tmpproc = config.readNumEntry("Proc1", 0);
            m_BSMWidget->proc100s->setValue(tmpproc);
            m_BSMWidget->proc100Slot(tmpproc);
            tmpproc = config.readNumEntry("Proc2", 0);
            m_BSMWidget->proc75s->setValue(tmpproc);
            m_BSMWidget->proc75Slot(tmpproc);
            tmpproc = config.readNumEntry("Proc3", 0);
            m_BSMWidget->proc50s->setValue(tmpproc);
            m_BSMWidget->proc50Slot(tmpproc);
            tmpproc = config.readNumEntry("Proc4", 0);
            m_BSMWidget->proc25s->setValue(tmpproc);
            m_BSMWidget->proc25Slot(tmpproc);
            break;
        case 1:			// DVD Playback
            m_BSMWidget->setCaption(i18n("DVD Playback - KToshiba"));
            config.setGroup("DVD_Playback");
            // LCD Brightness
            m_BSMWidget->bright100s->setValue
                    ( config.readNumEntry("Bright1", 3) );
            m_BSMWidget->bright75s->setValue
                    ( config.readNumEntry("Bright2", 3) );
            m_BSMWidget->bright50s->setValue
                    ( config.readNumEntry("Bright3", 3) );
            m_BSMWidget->bright25s->setValue
                    ( config.readNumEntry("Bright4", 3) );
            // Cooling Method
            m_BSMWidget->coolingcb->setCurrentItem
                    ( config.readNumEntry("Cooling", 1) );
            // Display Auto Off
            m_BSMWidget->display100s->setValue
                    ( config.readNumEntry("Disp1", 0) );
            m_BSMWidget->display75s->setValue
                    ( config.readNumEntry("Disp2", 0) );
            m_BSMWidget->display50s->setValue
                    ( config.readNumEntry("Disp3", 0) );
            m_BSMWidget->display25s->setValue
                    ( config.readNumEntry("Disp4", 0) );
            // HDD Auto Off
            m_BSMWidget->hdd100s->setValue
                    ( config.readNumEntry("HDD1", 0) );
            m_BSMWidget->hdd75s->setValue
                    ( config.readNumEntry("HDD2", 0) );
            m_BSMWidget->hdd50s->setValue
                    ( config.readNumEntry("HDD3", 0) );
            m_BSMWidget->hdd25s->setValue
                    ( config.readNumEntry("HDD4", 0) );
            // Processing Speed
            tmpproc = config.readNumEntry("Proc1", 0);
            m_BSMWidget->proc100s->setValue(tmpproc);
            m_BSMWidget->proc100Slot(tmpproc);
            tmpproc = config.readNumEntry("Proc2", 0);
            m_BSMWidget->proc75s->setValue(tmpproc);
            m_BSMWidget->proc75Slot(tmpproc);
            tmpproc = config.readNumEntry("Proc3", 0);
            m_BSMWidget->proc50s->setValue(tmpproc);
            m_BSMWidget->proc50Slot(tmpproc);
            tmpproc = config.readNumEntry("Proc4", 0);
            m_BSMWidget->proc25s->setValue(tmpproc);
            m_BSMWidget->proc25Slot(tmpproc);
            break;
        case 2:			// Presentation
            m_BSMWidget->setCaption(i18n("Presentation - KToshiba"));
            config.setGroup("Presentation");
            // LCD Brightness
            m_BSMWidget->bright100s->setValue
                    ( config.readNumEntry("Bright1", 7) );
            m_BSMWidget->bright75s->setValue
                    ( config.readNumEntry("Bright2", 7) );
            m_BSMWidget->bright50s->setValue
                    ( config.readNumEntry("Bright3", 7) );
            m_BSMWidget->bright25s->setValue
                    ( config.readNumEntry("Bright4", 7) );
            // Cooling Method
            m_BSMWidget->coolingcb->setCurrentItem
                    ( config.readNumEntry("Cooling", 2) );
            // Display Auto Off
            m_BSMWidget->display100s->setValue
                    ( config.readNumEntry("Disp1", 0) );
            m_BSMWidget->display75s->setValue
                    ( config.readNumEntry("Disp2", 0) );
            m_BSMWidget->display50s->setValue
                    ( config.readNumEntry("Disp3", 0) );
            m_BSMWidget->display25s->setValue
                    ( config.readNumEntry("Disp4", 0) );
            // HDD Auto Off
            m_BSMWidget->hdd100s->setValue
                    ( config.readNumEntry("HDD1", 0) );
            m_BSMWidget->hdd75s->setValue
                    ( config.readNumEntry("HDD2", 0) );
            m_BSMWidget->hdd50s->setValue
                    ( config.readNumEntry("HDD3", 0) );
            m_BSMWidget->hdd25s->setValue
                    ( config.readNumEntry("HDD4", 0) );
            // Processing Speed
            tmpproc = config.readNumEntry("Proc1", 0);
            m_BSMWidget->proc100s->setValue(tmpproc);
            m_BSMWidget->proc100Slot(tmpproc);
            tmpproc = config.readNumEntry("Proc2", 0);
            m_BSMWidget->proc75s->setValue(tmpproc);
            m_BSMWidget->proc75Slot(tmpproc);
            tmpproc = config.readNumEntry("Proc3", 0);
            m_BSMWidget->proc50s->setValue(tmpproc);
            m_BSMWidget->proc50Slot(tmpproc);
            tmpproc = config.readNumEntry("Proc4", 0);
            m_BSMWidget->proc25s->setValue(tmpproc);
            m_BSMWidget->proc25Slot(tmpproc);
            break;
    }
    m_BSMWidget->show();
}


void KCMToshibaModule::bsmChanged()
{
    KConfig config(BSM_CONFIG);
    switch(m_BSM) {
        case 0:			// High Power
            config.setGroup("High_Power");
            break;
        case 1:			// DVD Playback
            config.setGroup("DVD_Playback");
            break;
        case 2:			// Presentation
            config.setGroup("Presentation");
            break;
    }

    config.writeEntry("Bright1",
            m_BSMWidget->bright100s->value());
    config.writeEntry("Bright2",
            m_BSMWidget->bright75s->value());
    config.writeEntry("Bright3",
            m_BSMWidget->bright50s->value());
    config.writeEntry("Bright4",
            m_BSMWidget->bright25s->value());
    config.writeEntry("Cooling",
            m_BSMWidget->coolingcb->currentItem());
    config.writeEntry("Disp1",
            m_BSMWidget->display100s->value());
    config.writeEntry("Disp2",
            m_BSMWidget->display75s->value());
    config.writeEntry("Disp3",
            m_BSMWidget->display50s->value());
    config.writeEntry("Disp4",
            m_BSMWidget->display25s->value());
    config.writeEntry("HDD1",
            m_BSMWidget->hdd100s->value());
    config.writeEntry("HDD2",
            m_BSMWidget->hdd75s->value());
    config.writeEntry("HDD3",
            m_BSMWidget->hdd50s->value());
    config.writeEntry("HDD4",
            m_BSMWidget->hdd25s->value());
    config.writeEntry("Proc1",
            m_BSMWidget->proc100s->value());
    config.writeEntry("Proc2",
            m_BSMWidget->proc75s->value());
    config.writeEntry("Proc3",
            m_BSMWidget->proc50s->value());
    config.writeEntry("Proc4",
            m_BSMWidget->proc25s->value());
    config.sync();
}


void KCMToshibaModule::timeout()
{
    if (!m_Init) {	// initialize
        m_Timer->changeInterval(2000);
        m_Init = true;
    }

    int time = 0, perc = -1;

    if (m_SCIFace) m_SMMIFace->batteryStatus(&time, &perc);

    if (m_HCIFace) m_AC = m_SMMIFace->acPowerStatus();

    if (m_Omnibook) {
        m_OmniIFace->batteryStatus(&time, &perc);
        m_AC = m_OmniIFace->omnibookAC();
    }

    if (m_AC == -1) m_AC = m_ProcIFace->acpiAC();

    if (perc == -1) m_ProcIFace->acpiBatteryStatus(&time, &perc);

    m_KCMKToshibaGeneral->mKPBattery->setValue((perc == -1)? 0 : perc);

    m_KCMKToshibaGeneral->kledBat->setState((perc == -1 || perc == -2)? KLed::Off : KLed::On);
    m_KCMKToshibaGeneral->kledAC->setState((m_AC == 4)? KLed::On : KLed::Off);
}
