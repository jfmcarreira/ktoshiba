/***************************************************************************
 *   main.cpp                                                              *
 *                                                                         *
 *   Copyright (C) 2004 by Azael Avalos                                    *
 *   neftali@utep.edu                                                      *
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

#include <kparts/genericfactory.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kled.h>
#include <kprogress.h>

#include "kcmtoshiba_general.h"

#include "main.h"
#include "main.moc"

#define CONFIG_FILE "ktoshibarc"

typedef KGenericFactory<KCMToshibaModule, QWidget> KCMToshibaModuleFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_ktoshiba, KCMToshibaModuleFactory("kcmktoshiba"))

KCMToshibaModule::KCMToshibaModule(QWidget *parent, const char *name, const QStringList&)
    : KCModule(KCMToshibaModuleFactory::instance(), parent, name)
{
	KAboutData *about = new KAboutData(I18N_NOOP("kcmktoshiba"),
									   I18N_NOOP("KDE Control Module for Toshiba Laptops"),
									   0, 0, KAboutData::License_GPL,
									   "(c) 2004 Azael Avalos");

	about->addAuthor("Azael Avalos", I18N_NOOP("Original author"), "neftali@utep.edu");
	setAboutData( about );

	QVBoxLayout* layout = new QVBoxLayout(this);
	m_KCMKToshibaGeneral = new KCMKToshibaGeneral(this);
	layout->addWidget( m_KCMKToshibaGeneral );
	layout->addStretch();

	m_Driver = new KToshibaSMMInterface(this);
	m_InterfaceAvailable = m_Driver->openInterface();
	mTimer = new QTimer(this);
	mTimer->start(210);

	load();

	if (!m_InterfaceAvailable) {
		m_KCMKToshibaGeneral->tlOff->show();
		m_KCMKToshibaGeneral->frameMain->setEnabled(false);
		setButtons(buttons() & ~Default);
	} else {
		m_KCMKToshibaGeneral->tlOff->hide();
		m_KCMKToshibaGeneral->frameMain->setEnabled(true);
	}

	connect( m_KCMKToshibaGeneral, SIGNAL( changed() ), SLOT( configChanged() ) );
	connect( mTimer, SIGNAL( timeout() ), SLOT( timeout() ) );
};

void KCMToshibaModule::load()
{
	kdDebug() << "KCMToshibaModule: loading." << endl;
    KConfig config(CONFIG_FILE);

	config.setGroup("KToshiba");

	m_KCMKToshibaGeneral->batfullCheckBox->setChecked
				( config.readBoolEntry("Notify_On_Full_Battery", false) );
	m_KCMKToshibaGeneral->batstatSpinBox->setValue
				( config.readNumEntry("Battery_Status_Time", 2) );
	m_KCMKToshibaGeneral->lowbatSpinBox->setValue
				( config.readNumEntry("Low_Battery_Trigger", 15) );
	m_KCMKToshibaGeneral->crybatSpinBox->setValue
				( config.readNumEntry("Critical_Battery_Trigger", 5) );
	m_KCMKToshibaGeneral->audioComboBox->setCurrentItem
				( config.readNumEntry("Audio_Player", 1) );
	m_KCMKToshibaGeneral->btstartCheckBox->setChecked
				( config.readBoolEntry("Bluetooth_Startup", true) );
}


void KCMToshibaModule::defaults()
{
	m_KCMKToshibaGeneral->batfullCheckBox->setChecked( false );
	m_KCMKToshibaGeneral->batstatSpinBox->setValue( 2 );
	m_KCMKToshibaGeneral->lowbatSpinBox->setValue( 15 );
	m_KCMKToshibaGeneral->crybatSpinBox->setValue( 5 );
	m_KCMKToshibaGeneral->audioComboBox->setCurrentItem( 1 );
	m_KCMKToshibaGeneral->btstartCheckBox->setChecked( true );
}


void KCMToshibaModule::save()
{
	if (!m_InterfaceAvailable) return;

	kdDebug() << "KCMToshibaModule: saving." << endl;
    KConfig config(CONFIG_FILE);

	config.setGroup("KToshiba");

	config.writeEntry("Notify_On_Full_Battery",
					  m_KCMKToshibaGeneral->batfullCheckBox->isChecked());
	config.writeEntry("Battery_Status_Time",
					  m_KCMKToshibaGeneral->batstatSpinBox->value());
	config.writeEntry("Low_Battery_Trigger",
					  m_KCMKToshibaGeneral->lowbatSpinBox->value());
	config.writeEntry("Critical_Battery_Trigger",
					  m_KCMKToshibaGeneral->crybatSpinBox->value());
	config.writeEntry("Audio_Player",
					  m_KCMKToshibaGeneral->audioComboBox->currentItem());
	config.writeEntry("Bluetooth_Startup",
					  m_KCMKToshibaGeneral->btstartCheckBox->isChecked());
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
	static bool init = false;

	if (!init) {   // initialize
		mTimer->start(2000);
        init = true;
	}

	int time = 0, perc = 0, acConnected = 0;

	m_Driver->batteryStatus(&time, &perc);
	acConnected = m_Driver->acPowerStatus();

    m_KCMKToshibaGeneral->mKPBattery->setValue(perc);
	m_KCMKToshibaGeneral->kledBat->setState(KLed::On);
    m_KCMKToshibaGeneral->kledAC->setState((acConnected == 1)? KLed::On : KLed::Off);
}

