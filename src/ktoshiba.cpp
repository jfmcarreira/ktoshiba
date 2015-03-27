/*
   Copyright (C) 2004-2015  Azael Avalos <coproscefalo@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, see
   <http://www.gnu.org/licenses/>.
*/

#include <KMenu>
#include <KHelpMenu>
#include <KStandardDirs>
#include <KDebug>
#include <KNotification>
#include <KAboutData>
#include <KConfigGroup>
#include <KIcon>
#include <KProcess>

#include "ktoshiba.h"
#include "fnactions.h"
#include "helperactions.h"
#include "ktoshibanetlinkevents.h"
#include "version.h"

#define HDD_VIBRATED   0x80
#define HDD_STABILIZED 0x81

#define CONFIG_FILE "ktoshibarc"

KToshiba::KToshiba()
    : KStatusNotifierItem(),
      m_fn( new FnActions( this ) ),
      m_fnConnected( false ),
      m_nl( new KToshibaNetlinkEvents( this ) ),
      m_nlAttached( false ),
      m_config( KSharedConfig::openConfig( CONFIG_FILE ) )
{
    setTitle( i18n("KToshiba") );
    setIconByName( "ktoshiba" );
    setToolTip( "ktoshiba", i18n("KToshiba"), i18n("Fn key monitoring for Toshiba laptops") );
    setCategory( Hardware );
    setStatus( Passive );

    m_popupMenu = contextMenu();
    setAssociatedWidget( m_popupMenu );
}

KToshiba::~KToshiba()
{
    cleanup();
}

bool KToshiba::initialize()
{
    m_fnConnected = m_fn->init();
    if (!m_fnConnected) {
        kError() << "Could not continue loading, cleaning up...";
        cleanup();

        return false;
    }

    if (checkConfig())
        loadConfig();
    else
        createConfig();

    m_nl->setDeviceHID(m_fn->m_helper->getDeviceHID());
    m_nlAttached = m_nl->attach();
    if (m_nlAttached) {
        connect( m_nl, SIGNAL( tvapEvent(int) ), this, SLOT( parseTVAPEvents(int) ) );
        if (m_fn->m_helper->isHAPSSupported && m_monitorHDD)
            connect( m_nl, SIGNAL( hapsEvent(int) ), this, SLOT( protectHDD(int) ) );
    } else {
        kError() << "Events monitoring will not be possible";
    }

    doMenu();

    return true;
}

void KToshiba::cleanup()
{
    destroyAboutData();
    delete m_nl; m_nl = NULL;
    delete m_fn; m_fn = NULL;
}

bool KToshiba::checkConfig()
{
    KStandardDirs kstd;
    QString config = kstd.findResource("config", CONFIG_FILE);

    if (config.isEmpty()) {
        kDebug() << "Configuration file not found.";
        m_fn->m_helper->getSysInfo();

        return false;
    }

    return true;
}

void KToshiba::loadConfig()
{
    kDebug() << "Loading configuration file...";
    // General group
    KConfigGroup generalGroup( m_config, "General" );
    m_batteryProfiles = generalGroup.readEntry( "BatteryProfiles", true );
    // HDD Protection group
    KConfigGroup hddGroup( m_config, "HDDProtection" );
    m_monitorHDD = hddGroup.readEntry( "MonitorHDD", true );
    m_notifyHDD = hddGroup.readEntry( "NotifyHDDMovement", true );
}

void KToshiba::createConfig()
{
    kDebug() << "Default configuration file created.";
    // General group
    KConfigGroup generalGroup( m_config, "General" );
    generalGroup.writeEntry( "BatteryProfiles", true );
    generalGroup.config()->sync();
    // System Information group
    KConfigGroup sysinfoGroup( m_config, "SystemInformation" );
    sysinfoGroup.writeEntry( "ModelFamily", m_fn->m_helper->sysinfo[4] );
    sysinfoGroup.writeEntry( "ModelNumber", m_fn->m_helper->sysinfo[5] );
    sysinfoGroup.writeEntry( "BIOSVersion", m_fn->m_helper->sysinfo[1] );
    sysinfoGroup.writeEntry( "BIOSDate", m_fn->m_helper->sysinfo[2] );
    sysinfoGroup.writeEntry( "BIOSManufacturer", m_fn->m_helper->sysinfo[0] );
    sysinfoGroup.writeEntry( "ECVersion", m_fn->m_helper->sysinfo[3] );
    sysinfoGroup.config()->sync();
    // HDD Protection group
    KConfigGroup hddGroup( m_config, "HDDProtection" );
    hddGroup.writeEntry( "MonitorHDD", true );
    hddGroup.writeEntry( "NotifyHDDMovement", true );
    hddGroup.config()->sync();
}

void KToshiba::doMenu()
{
    m_batteryMenu = new QMenu(m_popupMenu);
    m_batteryMenu->setTitle( i18n("Battery Profiles") );
    m_popupMenu->addMenu( m_batteryMenu )->setIcon( KIcon( "battery" ) );
    m_batDisabled = m_batteryMenu->addAction( i18n("Disabled") );
    m_batDisabled->setIcon( QIcon( ":images/disabled_64.png" ) );
    m_batDisabled->setCheckable( true );
    m_batDisabled->setChecked( m_batteryProfiles );
    m_batPerformance = m_batteryMenu->addAction( i18n("Performance") );
    m_batPerformance->setIcon( QIcon( ":images/performance_64.png" ) );
    m_batPerformance->setEnabled( !m_batteryProfiles );
    m_batPowersave = m_batteryMenu->addAction( i18n("Powersave") );
    m_batPowersave->setIcon( QIcon( ":images/powersave_64.png" ) );
    m_batPowersave->setEnabled( !m_batteryProfiles );
    m_batPresentation = m_batteryMenu->addAction( i18n("Presentation") );
    m_batPresentation->setIcon( QIcon( ":images/presentation_64.png" ) );
    m_batPresentation->setEnabled( !m_batteryProfiles );
    m_batECO = m_batteryMenu->addAction( i18n("ECO") );
    m_batECO->setIcon( QIcon( ":images/green_world.svg" ) );
    m_batECO->setEnabled( !m_batteryProfiles );
    m_fn->batMonitorChanged(m_batteryProfiles);
    connect( m_batDisabled, SIGNAL( toggled(bool) ), this, SLOT( disabledClicked(bool) ) );
    connect( m_batPerformance, SIGNAL( triggered() ), this, SLOT( performanceClicked() ) );
    connect( m_batPowersave, SIGNAL( triggered() ), this, SLOT( powersaveClicked() ) );
    connect( m_batPresentation, SIGNAL( triggered() ), this, SLOT( presentationClicked() ) );
    connect( m_batECO, SIGNAL( triggered() ), this, SLOT( ecoClicked() ) );

    m_configure = m_popupMenu->addAction( i18n("Configure") );
    m_configure->setIcon( KIcon( "configure" ) );
    connect( m_configure, SIGNAL( triggered() ), this, SLOT( configureClicked() ) );

    m_popupMenu->addSeparator();
    m_helpMenu = new KHelpMenu( m_popupMenu, aboutData());
    m_popupMenu->addMenu( m_helpMenu->menu() )->setIcon( KIcon( "help-contents" ) );
    m_helpMenu->action( KHelpMenu::menuHelpContents )->setVisible( false );
    m_helpMenu->action( KHelpMenu::menuWhatsThis )->setVisible( false );
}

void KToshiba::configChanged()
{
    loadConfig();

    if (m_monitorHDD)
        connect( m_nl, SIGNAL( hapsEvent(int) ), this, SLOT( protectHDD(int) ) );
    else if (!m_monitorHDD)
        disconnect( this, SLOT( protectHDD(int) ) );
}

void KToshiba::notifyHDDMovement()
{
    KNotification *notification =
		KNotification::event(KNotification::Notification, i18n("KToshiba - HDD Monitor"),
				     i18n("Vibration has been detected and the HDD has been stopped to prevent damage"),
				     QIcon::fromTheme("drive-harddisk").pixmap(48, 48), 0, KNotification::Persistent);
    notification->sendEvent();
}

void KToshiba::protectHDD(int event)
{
    if (event == HDD_VIBRATED) {
        kDebug() << "Vibration detected";
        m_fn->m_helper->unloadHeads(5000);
        if (m_notifyHDD)
            notifyHDDMovement();
    } else if (event == HDD_STABILIZED) {
        kDebug() << "Vibration stabilized";
        m_fn->m_helper->unloadHeads(0);
    }
}

void KToshiba::disabledClicked(bool enabled)
{
    KConfigGroup generalGroup( m_config, "General" );
    generalGroup.writeEntry( "BatteryProfiles", enabled );
    generalGroup.config()->sync();
    m_batPerformance->setEnabled( !enabled );
    m_batPowersave->setEnabled( !enabled );
    m_batPresentation->setEnabled( !enabled );
    m_batECO->setEnabled( !enabled );
    m_batteryProfiles = enabled;

    emit batteryProfilesToggled(enabled);
}

void KToshiba::performanceClicked()
{
    m_fn->changeProfile("Performance");
}

void KToshiba::powersaveClicked()
{
    m_fn->changeProfile("Powersave");
}

void KToshiba::presentationClicked()
{
    m_fn->changeProfile("Presentation");
}

void KToshiba::ecoClicked()
{
    m_fn->changeProfile("ECO");
}

void KToshiba::configureClicked()
{
    KProcess p;
    p.setProgram(KStandardDirs::findExe("kcmshell4"), QStringList() << "ktoshibam");
    p.startDetached();
}

void KToshiba::parseTVAPEvents(int event)
{
    kDebug() << "Received event 0x" << hex << event;
    switch(event) {
    case 0x80:	// Hotkeys and some system events
        break;
    case 0x81:	// Dock events
    case 0x82:
    case 0x83: 
        break;
    case 0x88:	// Thermal event
        break;
    case 0x8f:	// LID closed
    case 0x90:	// LID is closed and Dock has been ejected
        break;
    case 0x8b:	// SATA power events
    case 0x8c:
        break;
    case 0x92:	// KBD backlight event
        emit kbdModeChanged();
        break;
    default:
        kDebug() << "Unknown event";
    }
}

static const char * const description =
    I18N_NOOP("Fn key monitoring for Toshiba laptops.");

void KToshiba::createAboutData()
{
    m_about = new KAboutData("KToshiba", 0, ki18n("KToshiba"), ktoshiba_version,
			ki18n(description), KAboutData::License_GPL_V2,
			ki18n("Copyright © 2004-2015, Azael Avalos"), KLocalizedString(),
			"http://ktoshiba.sourceforge.net/",
			"coproscefalo@gmail.com");
    m_about->setProgramIconName("ktoshiba");

    m_about->addAuthor( ki18n("Azael Avalos"),
			ki18n("Original Author"), "coproscefalo@gmail.com" );
    m_about->addCredit( ki18n("KDE Team"), ki18n("Some ideas and pieces of code"), 0,
                    "http://www.kde.org/" );
    m_about->addCredit( ki18n("Mauricio Duque"), ki18n("Green world icon"),
		    "info@snap2objects.com", "http://www.snap2objects.com/" );
}

void KToshiba::destroyAboutData()
{
    delete m_about;
    m_about = NULL;
}

KAboutData* KToshiba::m_about;

KAboutData* KToshiba::aboutData()
{
    return m_about;
}


#include "ktoshiba.moc"
