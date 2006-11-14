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
#include <qpushbutton.h>
#include <qtimer.h>
#include <qcheckbox.h>

#include <kgenericfactory.h>
#include <kaboutdata.h>
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
    m_Init = false;
    m_AC = -1;
    m_IFaceErr = 0;
    m_Hotkeys = 0;

    m_ProcIFace = new KToshibaProcInterface(this);
    m_SMMIFace = new KToshibaSMMInterface(this);
    m_SCIIFace = m_SMMIFace->openSCIInterface(&m_IFaceErr);
    if (!m_SCIIFace)
        m_SCIIFace = (m_IFaceErr == SCI_ALREADY_OPEN)? true : false;
    m_HCIIFace = (m_SMMIFace->machineBIOS() == -1)? false : true;
    if (m_HCIIFace) {
        m_AC = m_SMMIFace->acPowerStatus();
        m_Hotkeys = m_SMMIFace->getSystemEvent();
    }
    m_KCMKToshibaGeneral->configTabWidget->setTabEnabled(
		m_KCMKToshibaGeneral->configTabWidget->page(1), ((m_Hotkeys == -1)? false : true));
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
        m_KCMKToshibaGeneral->fnComboBox_2->setEnabled(false);
        m_KCMKToshibaGeneral->fnComboBox_3->setEnabled(false);
        m_KCMKToshibaGeneral->fnComboBox_4->setEnabled(false);
        m_KCMKToshibaGeneral->fnComboBox_5->setEnabled(false);
        m_KCMKToshibaGeneral->fnComboBox_6->setEnabled(false);
        m_KCMKToshibaGeneral->fnComboBox_7->setEnabled(false);
        m_KCMKToshibaGeneral->fnComboBox_9->setEnabled(false);
        m_AC = m_OmniIFace->omnibookAC();
    }

    if (!m_SCIIFace && !m_HCIIFace && !m_Omnibook) {
        m_KCMKToshibaGeneral->tlOff->show();
        m_KCMKToshibaGeneral->frameMain->setEnabled(false);
        setButtons(buttons() & ~Default);
    } else
    if (m_SCIIFace || m_HCIIFace || m_Omnibook) {
        m_KCMKToshibaGeneral->tlOff->hide();
        m_KCMKToshibaGeneral->frameMain->setEnabled(true);

#ifndef ENABLE_HELPER
        m_KCMKToshibaGeneral->helperPushButton->setEnabled(false);
#endif // ENABLE_HELPER

        connect( m_KCMKToshibaGeneral, SIGNAL( changed() ), SLOT( configChanged() ) );
        m_Timer = new QTimer(this);
        connect( m_Timer, SIGNAL( timeout() ), SLOT( timeout() ) );
        m_Timer->start(210);
    }
};


void KCMToshibaModule::load()
{
    kdDebug() << "KCMToshibaModule: loading." << endl;
    KConfig config(CONFIG_FILE);

    config.setGroup("KToshiba");
    m_KCMKToshibaGeneral->audioComboBox->setCurrentItem
		( config.readNumEntry("Audio_Player", 0) );
    m_KCMKToshibaGeneral->btstartCheckBox->setChecked
		( config.readBoolEntry("Bluetooth_Startup", true) );
    m_KCMKToshibaGeneral->autostartCheckBox->setChecked
		( config.readBoolEntry("AutoStart", true) );

    config.setGroup("Fn_Key");
    m_KCMKToshibaGeneral->fnComboBox->setCurrentItem
		( config.readNumEntry("Fn_Esc", 1) );
    m_KCMKToshibaGeneral->fnComboBox_1->setCurrentItem
		( config.readNumEntry("Fn_F1", 2) );
    m_KCMKToshibaGeneral->fnComboBox_2->setCurrentItem
		( config.readNumEntry("Fn_F2", 3) );
    m_KCMKToshibaGeneral->fnComboBox_3->setCurrentItem
		( config.readNumEntry("Fn_F3", 4) );
    m_KCMKToshibaGeneral->fnComboBox_4->setCurrentItem
		( config.readNumEntry("Fn_F4", 5) );
    m_KCMKToshibaGeneral->fnComboBox_5->setCurrentItem
		( config.readNumEntry("Fn_F5", 6) );
    m_KCMKToshibaGeneral->fnComboBox_6->setCurrentItem
		( config.readNumEntry("Fn_F6", 7) );
    m_KCMKToshibaGeneral->fnComboBox_7->setCurrentItem
		( config.readNumEntry("Fn_F7", 8) );
    m_KCMKToshibaGeneral->fnComboBox_8->setCurrentItem
		( config.readNumEntry("Fn_F8", 9) );
    m_KCMKToshibaGeneral->fnComboBox_9->setCurrentItem
		( config.readNumEntry("Fn_F9", 10) );

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
    m_KCMKToshibaGeneral->audioComboBox->setCurrentItem( 0 );
    m_KCMKToshibaGeneral->btstartCheckBox->setChecked( true );
    m_KCMKToshibaGeneral->autostartCheckBox->setChecked( true );
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
    m_KCMKToshibaGeneral->processorComboBox->setCurrentItem( 1 );
    m_KCMKToshibaGeneral->cpuComboBox->setCurrentItem( 0 );
    m_KCMKToshibaGeneral->displayComboBox->setCurrentItem( 5 );
    m_KCMKToshibaGeneral->hddComboBox->setCurrentItem( 5 );
    m_KCMKToshibaGeneral->lcdComboBox->setCurrentItem( 2 );
    m_KCMKToshibaGeneral->coolingComboBox->setCurrentItem( 2 );
}


void KCMToshibaModule::save()
{
    kdDebug() << "KCMToshibaModule: saving." << endl;
    KConfig config(CONFIG_FILE);

    config.setGroup("KToshiba");
    config.writeEntry("Audio_Player",
		  m_KCMKToshibaGeneral->audioComboBox->currentItem());
    config.writeEntry("Bluetooth_Startup",
		  m_KCMKToshibaGeneral->btstartCheckBox->isChecked());
    config.writeEntry("AutoStart",
		  m_KCMKToshibaGeneral->autostartCheckBox->isChecked());
    config.sync();

    config.setGroup("Fn_Key");
    config.writeEntry("Fn_Esc",
		  m_KCMKToshibaGeneral->fnComboBox->currentItem());
    config.writeEntry("Fn_F1",
		  m_KCMKToshibaGeneral->fnComboBox_1->currentItem());
    config.writeEntry("Fn_F2",
		  m_KCMKToshibaGeneral->fnComboBox_2->currentItem());
    config.writeEntry("Fn_F3",
		  m_KCMKToshibaGeneral->fnComboBox_3->currentItem());
    config.writeEntry("Fn_F4",
		  m_KCMKToshibaGeneral->fnComboBox_4->currentItem());
    config.writeEntry("Fn_F5",
		  m_KCMKToshibaGeneral->fnComboBox_5->currentItem());
    config.writeEntry("Fn_F6",
		  m_KCMKToshibaGeneral->fnComboBox_6->currentItem());
    config.writeEntry("Fn_F7",
		  m_KCMKToshibaGeneral->fnComboBox_7->currentItem());
    config.writeEntry("Fn_F8",
		  m_KCMKToshibaGeneral->fnComboBox_8->currentItem());
    config.writeEntry("Fn_F9",
		  m_KCMKToshibaGeneral->fnComboBox_9->currentItem());
    config.sync();

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

    (perc == -1)? m_KCMKToshibaGeneral->mKPBattery->setValue(0)
        : m_KCMKToshibaGeneral->mKPBattery->setValue(perc);

    m_KCMKToshibaGeneral->kledBat->setState((perc == -1 || perc == -2)? KLed::Off : KLed::On);
    m_KCMKToshibaGeneral->kledAC->setState((m_AC == 4)? KLed::On : KLed::Off);
}
