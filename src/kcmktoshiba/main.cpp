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
#include <qlabel.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qtimer.h>
#include <qcombobox.h>
#include <qfile.h>
#include <qregexp.h>
#include <qbuttongroup.h>
#include <qtabwidget.h>

#include <kparts/genericfactory.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kled.h>
#include <kprogress.h>

#include "../ktoshibasmminterface.h"
#include "../ktoshibaprocinterface.h"
#include "kcmtoshiba_general.h"
#include "main.h"
#include "main.moc"

#define CONFIG_FILE "ktoshibarc"

typedef KGenericFactory<KCMToshibaModule, QWidget> KCMToshibaModuleFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_ktoshibam, KCMToshibaModuleFactory("kcmktoshiba"))

KCMToshibaModule::KCMToshibaModule(QWidget *parent, const char *name, const QStringList&)
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

    m_Driver = new KToshibaSMMInterface(this);
    m_Proc = new KToshibaProcInterface(this);
    m_InterfaceAvailable = m_Driver->openInterface();
    m_Timer = new QTimer(this);

    load();

#ifdef ENABLE_OMNIBOOK
    if (!m_InterfaceAvailable) {
        m_Omnibook = m_Proc->checkOmnibook();
        if (!m_Omnibook) {
            m_KCMKToshibaGeneral->tlOff->show();
            m_KCMKToshibaGeneral->frameMain->setEnabled(false);
            setButtons(buttons() & ~Default);
        }
        m_KCMKToshibaGeneral->tlOff->hide();
        m_KCMKToshibaGeneral->frameMain->setEnabled(true);
        m_KCMKToshibaGeneral->bgOtherOptions->setEnabled(false);
        m_KCMKToshibaGeneral->configTabWidget->setTabEnabled(
		m_KCMKToshibaGeneral->configTabWidget->page(1), false);
        m_KCMKToshibaGeneral->configTabWidget->setTabEnabled(
		m_KCMKToshibaGeneral->configTabWidget->page(2), false);
        m_AC = m_Proc->omnibookAC();
        m_Omnibook = true;
    }
#else
    if (m_InterfaceAvailable) {
        m_KCMKToshibaGeneral->tlOff->hide();
        m_KCMKToshibaGeneral->frameMain->setEnabled(true);
        m_AC = m_Driver->acPowerStatus();
        m_Omnibook = false;
    }
#endif
    else {
        m_KCMKToshibaGeneral->tlOff->show();
        m_KCMKToshibaGeneral->frameMain->setEnabled(false);
        setButtons(buttons() & ~Default);
    }

    if (m_InterfaceAvailable || m_Omnibook) {
        connect( m_KCMKToshibaGeneral, SIGNAL( changed() ), SLOT( configChanged() ) );
        connect( m_Timer, SIGNAL( timeout() ), SLOT( timeout() ) );
        m_Timer->start(210);
        init = false;
    }
};

void KCMToshibaModule::load()
{
    kdDebug() << "KCMToshibaModule: loading." << endl;
    KConfig config(CONFIG_FILE);
    config.setGroup("KToshiba");

    m_KCMKToshibaGeneral->audioComboBox->setCurrentItem
		( config.readNumEntry("Audio_Player", 1) );
    m_KCMKToshibaGeneral->btstartCheckBox->setChecked
		( config.readBoolEntry("Bluetooth_Startup", true) );
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
    m_KCMKToshibaGeneral->audioComboBox->setCurrentItem( 1 );
    m_KCMKToshibaGeneral->btstartCheckBox->setChecked( true );
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


QString KCMToshibaModule::quickHelp() const
{
    return i18n("Helpful information about the KToshiba module.");
}


void KCMToshibaModule::timeout()
{
    if (!init) {   // initialize
        m_Timer->start(2000);
        init = true;
    }

    int time = 0, perc = -1, acConnected = -1;

#ifdef ENABLE_OMNIBOOK
    if (m_Omnibook) {
        m_Proc->omnibookBatteryStatus(&time, &perc);
        if (perc == -1)
            m_Proc->acpiBatteryStatus(&time, &perc);
        acConnected = m_Proc->omnibookAC();
        acConnected = ((acConnected == -1)? m_Proc->acpiAC() : m_Proc->omnibookAC());
    }
#else
    if (m_InterfaceAvailable) {
        m_Driver->batteryStatus(&time, &perc);
        if (perc == -1)
            m_Proc->acpiBatteryStatus(&time, &perc);

        acConnected = ((m_AC == -1)? SciACPower() : m_Driver->acPowerStatus());
        if ((acConnected == -1) || (acConnected == SCI_FAILURE))
            acConnected = m_Proc->acpiAC();
    }
#endif

    if (perc == -1)
        m_KCMKToshibaGeneral->mKPBattery->setValue(0);
    else
        m_KCMKToshibaGeneral->mKPBattery->setValue(perc);
    m_KCMKToshibaGeneral->kledBat->setState((perc == -1)? KLed::Off : KLed::On);
    m_KCMKToshibaGeneral->kledAC->setState((acConnected == 4)? KLed::On : KLed::Off);
}
