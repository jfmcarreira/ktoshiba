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

#include <kgenericfactory.h>
#include <kaboutdata.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kprogress.h>
#include <kled.h>

#include "../ktoshibasmminterface.h"
#include "../ktoshibaomnibookinterface.h"
#include "../ktoshibaprocinterface.h"
#include "kcmtoshiba_general.h"
#include "main.h"
#include "main.moc"

#define CONFIG_FILE "ktoshibarc"

typedef KGenericFactory<KCMToshibaModule, QWidget> KCMToshibaModuleFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_ktoshibam, KCMToshibaModuleFactory("kcmktoshiba") )

KCMToshibaModule::KCMToshibaModule(QWidget *parent, const char *name, const QStringList &)
    : KCModule(KCMToshibaModuleFactory::instance(), parent, name)
{
    KAboutData *about = new KAboutData(I18N_NOOP("kcmktoshiba"),
				   I18N_NOOP("KDE Control Module for Toshiba Laptops"),
				   0, 0, KAboutData::License_GPL,
				   "(c) 2004-2006 Azael Avalos");

    about->addAuthor("Azael Avalos", I18N_NOOP("Original author"), "coproscefalo@gmail.com");
    setAboutData( about );

    QVBoxLayout* layout = new QVBoxLayout(this);
    m_KCMKToshibaGeneral = new KCMKToshibaGeneral(this);
    layout->addWidget( m_KCMKToshibaGeneral );
    layout->addStretch();

    load();

    m_SCIIFace = false;
    m_HCIIFace = false;
    m_Omnibook = false;
    m_Hotkeys = false;
    m_Init = false;
    m_AC = -1;
    m_IFaceErr = 0;

    m_ProcIFace = new KToshibaProcInterface(this);
    m_SMMIFace = new KToshibaSMMInterface(this);
    m_SCIIFace = m_SMMIFace->openSCIInterface(&m_IFaceErr);
    if (!m_SCIIFace)
        m_SCIIFace = (m_IFaceErr == SCI_ALREADY_OPEN)? true : false;
    // FIXME: We should not rely on BIOS version for HCI access...
    m_HCIIFace = (m_SMMIFace->machineBIOS() == -1)? false : true;
    if (m_HCIIFace) {
        m_AC = m_SMMIFace->acPowerStatus();
        m_Hotkeys = ((m_SMMIFace->getSystemEvent() == -1)? false : true);
        m_KCMKToshibaGeneral->fnComboBox_5->setEnabled(false);	// Fn-F5
    }
    m_KCMKToshibaGeneral->configTabWidget->setTabEnabled(
		m_KCMKToshibaGeneral->configTabWidget->page(2), m_SCIIFace);

    if (!m_HCIIFace && !m_SCIIFace) {
        delete m_SMMIFace; m_SMMIFace = NULL;
        m_OmniIFace = new KToshibaOmnibookInterface(this);
        m_Omnibook = m_OmniIFace->checkOmnibook();
    }
    if (m_Omnibook) {
        m_KCMKToshibaGeneral->configTabWidget->setTabEnabled(
		    m_KCMKToshibaGeneral->configTabWidget->page(2), false);
        m_KCMKToshibaGeneral->fnComboBox_6->setEnabled(false);	// Fn-F6
        m_KCMKToshibaGeneral->fnComboBox_7->setEnabled(false);	// Fn-F7
        m_AC = m_OmniIFace->omnibookAC();
        QString keyhandler = KStandardDirs::findExe("ktosh_keyhandler");
        m_Hotkeys = (keyhandler.isEmpty())? false : true;
    }

    if (!m_SCIIFace && !m_HCIIFace && !m_Omnibook) {
        m_KCMKToshibaGeneral->tlOff->show();
        m_KCMKToshibaGeneral->frameMain->setEnabled(false);
        setButtons(buttons() & ~Default);
    } else
    if (m_SCIIFace || m_HCIIFace || m_Omnibook) {
        m_KCMKToshibaGeneral->tlOff->hide();
        m_KCMKToshibaGeneral->frameMain->setEnabled(true);
        m_KCMKToshibaGeneral->configTabWidget->setTabEnabled(
			m_KCMKToshibaGeneral->configTabWidget->page(1), m_Hotkeys);
        // Lets hide Fn-F5 Line Edit box
        m_KCMKToshibaGeneral->FnF5le->hide();

        connect( m_KCMKToshibaGeneral, SIGNAL( changed() ), SLOT( configChanged() ) );
        m_Timer = new QTimer(this);
        connect( m_Timer, SIGNAL( timeout() ), SLOT( timeout() ) );
        m_Timer->start(210);
    }
};


void KCMToshibaModule::load()
{
    int tmp = -1;
    KConfig config(CONFIG_FILE);
    // General Options Tab
    config.setGroup("KToshiba");
    m_KCMKToshibaGeneral->audioComboBox->setCurrentItem
		( config.readNumEntry("Audio_Player", 0) );
    m_KCMKToshibaGeneral->btstartCheckBox->setChecked
		( config.readBoolEntry("Bluetooth_Startup", true) );
    m_KCMKToshibaGeneral->autostartCheckBox->setChecked
		( config.readBoolEntry("AutoStart", true) );
    // Fn-Key Tab
    config.setGroup("Fn_Key");
    // Rudimentary way of acquiring the command... I don't like it...
    tmp = config.readNumEntry("Fn_Esc", 1);
    m_KCMKToshibaGeneral->fnComboBox->setCurrentItem( tmp );
    if (tmp == 17) {
        m_KCMKToshibaGeneral->FnEscle->setText( config.readEntry("Fn_Esc_Cmd") );
        m_KCMKToshibaGeneral->FnEscle->show();
    } else
        m_KCMKToshibaGeneral->FnEscle->hide();
    tmp = config.readNumEntry("Fn_F1", 2);
    m_KCMKToshibaGeneral->fnComboBox_1->setCurrentItem( tmp );
    if (tmp == 17) {
        m_KCMKToshibaGeneral->FnF1le->setText( config.readEntry("Fn_F1_Cmd") );
        m_KCMKToshibaGeneral->FnF1le->show();
    } else
        m_KCMKToshibaGeneral->FnF1le->hide();
    tmp = config.readNumEntry("Fn_F2", 3);
    m_KCMKToshibaGeneral->fnComboBox_2->setCurrentItem( tmp );
    if (tmp == 17) {
        m_KCMKToshibaGeneral->FnF2le->setText( config.readEntry("Fn_F2_Cmd") );
        m_KCMKToshibaGeneral->FnF2le->show();
    } else
        m_KCMKToshibaGeneral->FnF2le->hide();
    tmp = config.readNumEntry("Fn_F3", 4);
    m_KCMKToshibaGeneral->fnComboBox_3->setCurrentItem( tmp );
    if (tmp == 17) {
        m_KCMKToshibaGeneral->FnF3le->setText( config.readEntry("Fn_F3_Cmd") );
        m_KCMKToshibaGeneral->FnF3le->show();
    } else
        m_KCMKToshibaGeneral->FnF3le->hide();
    tmp = config.readNumEntry("Fn_F4", 5);
    m_KCMKToshibaGeneral->fnComboBox_4->setCurrentItem( tmp );
    if (tmp == 17) {
        m_KCMKToshibaGeneral->FnF4le->setText( config.readEntry("Fn_F4_Cmd") );
        m_KCMKToshibaGeneral->FnF4le->show();
    } else
        m_KCMKToshibaGeneral->FnF4le->hide();
    tmp = config.readNumEntry("Fn_F6", 7);
    m_KCMKToshibaGeneral->fnComboBox_6->setCurrentItem( tmp );
    if (tmp == 17) {
        m_KCMKToshibaGeneral->FnF6le->setText( config.readEntry("Fn_F6_Cmd") );
        m_KCMKToshibaGeneral->FnF6le->show();
    } else
        m_KCMKToshibaGeneral->FnF6le->hide();
    tmp = config.readNumEntry("Fn_F7", 8);
    m_KCMKToshibaGeneral->fnComboBox_7->setCurrentItem( tmp );
    if (tmp == 17) {
        m_KCMKToshibaGeneral->FnF7le->setText( config.readEntry("Fn_F7_Cmd") );
        m_KCMKToshibaGeneral->FnF7le->show();
    } else
        m_KCMKToshibaGeneral->FnF7le->hide();
    tmp = config.readNumEntry("Fn_F8", 9);
    m_KCMKToshibaGeneral->fnComboBox_8->setCurrentItem( tmp );
    if (tmp == 17) {
        m_KCMKToshibaGeneral->FnF8le->setText( config.readEntry("Fn_F8_Cmd") );
        m_KCMKToshibaGeneral->FnF8le->show();
    } else
        m_KCMKToshibaGeneral->FnF8le->hide();
    tmp = config.readNumEntry("Fn_F9", 10);
    m_KCMKToshibaGeneral->fnComboBox_9->setCurrentItem( tmp );
    if (tmp == 17) {
        m_KCMKToshibaGeneral->FnF9le->setText( config.readEntry("Fn_F9_Cmd") );
        m_KCMKToshibaGeneral->FnF9le->show();
    } else
        m_KCMKToshibaGeneral->FnF9le->hide();
    // Battery Save Mode Tab
    config.setGroup("BSM");
    m_KCMKToshibaGeneral->processorComboBox->setCurrentItem
		( config.readNumEntry("Processing_Speed", 1) );
    m_KCMKToshibaGeneral->cpuComboBox->setCurrentItem
		( config.readNumEntry("CPU_Sleep_Mode", 0) );
    m_KCMKToshibaGeneral->displayComboBox->setCurrentItem
		( config.readNumEntry("Display_Auto_Off", 5) );
    m_KCMKToshibaGeneral->hddComboBox->setCurrentItem
		( config.readNumEntry("HDD_Auto_Off", 5) );
    m_KCMKToshibaGeneral->lcdComboBox->setCurrentItem
		( config.readNumEntry("LCD_Brightness", 2) );
    m_KCMKToshibaGeneral->coolingComboBox->setCurrentItem
		( config.readNumEntry("Cooling_Method", 2) );
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
    // Battery Save Mode Tab
    m_KCMKToshibaGeneral->processorComboBox->setCurrentItem( 1 );
    m_KCMKToshibaGeneral->cpuComboBox->setCurrentItem( 0 );
    m_KCMKToshibaGeneral->displayComboBox->setCurrentItem( 5 );
    m_KCMKToshibaGeneral->hddComboBox->setCurrentItem( 5 );
    m_KCMKToshibaGeneral->lcdComboBox->setCurrentItem( 2 );
    m_KCMKToshibaGeneral->coolingComboBox->setCurrentItem( 2 );
}


void KCMToshibaModule::save()
{
    int tmp = -1;
    KConfig config(CONFIG_FILE);
    // General Options Tab
    config.setGroup("KToshiba");
    config.writeEntry("Audio_Player",
		  m_KCMKToshibaGeneral->audioComboBox->currentItem());
    config.writeEntry("Bluetooth_Startup",
		  m_KCMKToshibaGeneral->btstartCheckBox->isChecked());
    config.writeEntry("AutoStart",
		  m_KCMKToshibaGeneral->autostartCheckBox->isChecked());
    config.sync();
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
    // Battery Save Mode Tab
    config.setGroup("BSM");
    config.writeEntry("Processing_Speed",
		  m_KCMKToshibaGeneral->processorComboBox->currentItem());
    config.writeEntry("CPU_Sleep_Mode",
		  m_KCMKToshibaGeneral->cpuComboBox->currentItem());
    config.writeEntry("Display_Auto_Off",
		  m_KCMKToshibaGeneral->displayComboBox->currentItem());
    config.writeEntry("HDD_Auto_Off",
		  m_KCMKToshibaGeneral->hddComboBox->currentItem());
    config.writeEntry("LCD_Brightness",
		  m_KCMKToshibaGeneral->lcdComboBox->currentItem());
    config.writeEntry("Cooling_Method",
		  m_KCMKToshibaGeneral->coolingComboBox->currentItem());
    config.sync();
}


void KCMToshibaModule::configChanged()
{
    emit changed(true);
}


void KCMToshibaModule::timeout()
{
    if (!m_Init) {   // initialize
        m_Timer->changeInterval(2000);
        m_Init = true;
    }

    int time = 0, perc = -1;

    if (m_SCIIFace) m_SMMIFace->batteryStatus(&time, &perc);

    if (m_HCIIFace) m_AC = m_SMMIFace->acPowerStatus();

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
