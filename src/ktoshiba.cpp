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

#include <QDebug>
#include <QIcon>
#include <QMenu>
#include <QStandardPaths>
#include <QActionGroup>

#include <KNotification>
#include <KAboutData>
#include <KConfigGroup>
#include <KProcess>
#include <KLocalizedString>

#include "ktoshiba.h"
#include "fnactions.h"
#include "ktoshibahardware.h"
#include "ktoshibanetlinkevents.h"
#include "version.h"

#define CONFIG_FILE "ktoshibarc"

KToshiba::KToshiba()
    : KStatusNotifierItem(),
      m_fn( new FnActions( this ) ),
      m_nl( new KToshibaNetlinkEvents( this ) ),
      m_monitorHDD( true ),
      m_notifyHDD( true ),
      m_batteryProfiles( true ),
      m_config( KSharedConfig::openConfig( CONFIG_FILE ) ),
      m_sysinfo( false )
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
    if (!m_fn->init()) {
        qCritical() << "Could not continue loading, cleaning up...";
        cleanup();

        return false;
    }

    if (checkConfig())
        loadConfig();
    else
        createConfig();

    m_nl->setDeviceHID(m_fn->hw()->getDeviceHID());
    if (m_nl->attach()) {
        connect( m_nl, SIGNAL( tvapEvent(int) ), this, SLOT( parseTVAPEvents(int) ) );
        if (m_fn->hw()->isHAPSSupported && m_monitorHDD)
            connect( m_nl, SIGNAL( hapsEvent(int) ), this, SLOT( protectHDD(int) ) );
    } else {
        qCritical() << "Events monitoring will not be possible";
    }

    doMenu();

    return true;
}

void KToshiba::cleanup()
{
    delete m_nl; m_nl = NULL;
    delete m_fn; m_fn = NULL;
}

bool KToshiba::checkConfig()
{
    QString config = QStandardPaths::locate(QStandardPaths::ConfigLocation, CONFIG_FILE);

    if (config.isEmpty()) {
        qDebug() << "Configuration file not found.";
        m_sysinfo = m_fn->hw()->getSysInfo();

        return false;
    }

    return true;
}

void KToshiba::loadConfig()
{
    qDebug() << "Loading configuration file...";
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
    qDebug() << "Default configuration file created.";
    // General group
    KConfigGroup generalGroup( m_config, "General" );
    generalGroup.writeEntry( "BatteryProfiles", true );
    generalGroup.sync();
    // System Information group
    KConfigGroup sysinfoGroup( m_config, "SystemInformation" );
    sysinfoGroup.writeEntry( "ModelFamily", m_sysinfo ? m_fn->hw()->modelFamily : i18n("Unknown") );
    sysinfoGroup.writeEntry( "ModelNumber", m_sysinfo ? m_fn->hw()->modelNumber : i18n("Unknown") );
    sysinfoGroup.writeEntry( "BIOSVersion", m_sysinfo ? m_fn->hw()->biosVersion : i18n("Unknown") );
    sysinfoGroup.writeEntry( "BIOSDate", m_sysinfo ? m_fn->hw()->biosDate : i18n("Unknown") );
    sysinfoGroup.writeEntry( "BIOSManufacturer", m_sysinfo ? m_fn->hw()->biosManufacturer : i18n("Unknown") );
    sysinfoGroup.writeEntry( "ECVersion", m_sysinfo ? m_fn->hw()->ecVersion : i18n("Unknown") );
    sysinfoGroup.sync();
    // HDD Protection group
    KConfigGroup hddGroup( m_config, "HDDProtection" );
    hddGroup.writeEntry( "MonitorHDD", true );
    hddGroup.writeEntry( "NotifyHDDMovement", true );
    hddGroup.sync();
}

void KToshiba::doMenu()
{
    m_batteryMenu = new QMenu(m_popupMenu);
    m_batteryMenu->setTitle( i18n("Battery Profiles") );
    m_popupMenu->addMenu( m_batteryMenu )->setIcon( QIcon::fromTheme( "battery" ).pixmap(16, 16) );

    m_batDisabled = m_batteryMenu->addAction( i18n("Disabled") );
    m_batDisabled->setIcon( QIcon( ":images/disabled_64.png" ) );
    m_batDisabled->setCheckable( true );
    m_batDisabled->setChecked( m_batteryProfiles );
    connect( m_batDisabled, SIGNAL( toggled(bool) ), m_fn, SLOT( batMonitorChanged(bool) ) );
    connect( m_batDisabled, SIGNAL( toggled(bool) ), this, SLOT( disabledClicked(bool) ) );

    m_batPerformance = m_batteryMenu->addAction( i18n("Performance") );
    m_batPerformance->setIcon( QIcon( ":images/performance_64.png" ) );
    m_batPerformance->setEnabled( !m_batteryProfiles );
    m_batPerformance->setCheckable( true );

    m_batPowersave = m_batteryMenu->addAction( i18n("Powersave") );
    m_batPowersave->setIcon( QIcon( ":images/powersave_64.png" ) );
    m_batPowersave->setEnabled( !m_batteryProfiles );
    m_batPowersave->setCheckable( true );

    m_batPresentation = m_batteryMenu->addAction( i18n("Presentation") );
    m_batPresentation->setIcon( QIcon( ":images/presentation_64.png" ) );
    m_batPresentation->setEnabled( !m_batteryProfiles );
    m_batPresentation->setCheckable( true );

    m_batECO = m_batteryMenu->addAction( i18n("ECO") );
    m_batECO->setIcon( QIcon( ":images/green_world.svg" ) );
    m_batECO->setEnabled( !m_batteryProfiles );
    m_batECO->setCheckable( true );

    m_batteryGroup = new QActionGroup(m_batteryMenu);
    m_batteryGroup->addAction(m_batPerformance);
    m_batteryGroup->addAction(m_batPowersave);
    m_batteryGroup->addAction(m_batPresentation);
    m_batteryGroup->addAction(m_batECO);
    connect( m_batteryGroup, SIGNAL( triggered(QAction *) ), this, SLOT( setBatteryProfile(QAction *) ) );

    // Set the initial battery monitoring status
    m_fn->batMonitorChanged(m_batteryProfiles);

    m_configure = m_popupMenu->addAction( i18n("Configure") );
    m_configure->setIcon( QIcon::fromTheme( "configure" ).pixmap(16, 16) );
    connect( m_configure, SIGNAL( triggered() ), this, SLOT( configureClicked() ) );
}

void KToshiba::configChanged()
{
    loadConfig();

    if (m_monitorHDD)
        connect( m_nl, SIGNAL( hapsEvent(int) ), this, SLOT( protectHDD(int) ) );
    else
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
    if (event == KToshibaNetlinkEvents::Vibrated) {
        qDebug() << "Vibration detected";
        m_fn->hw()->unloadHeads(5000);
        if (m_notifyHDD)
            notifyHDDMovement();
    } else if (event == KToshibaNetlinkEvents::Stabilized) {
        qDebug() << "Vibration stabilized";
        m_fn->hw()->unloadHeads(0);
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
}

void KToshiba::setBatteryProfile(QAction *action)
{
    if (action == m_batPerformance)
        m_fn->changeProfile("Performance");
    else if (action == m_batPowersave)
        m_fn->changeProfile("Powersave");
    else if (action == m_batPresentation)
        m_fn->changeProfile("Presentation");
    else if (action == m_batECO)
        m_fn->changeProfile("ECO");
}

void KToshiba::configureClicked()
{
    KProcess p;
    p.setProgram(QStandardPaths::findExecutable("kcmshell5"), QStringList() << "ktoshibam");
    p.startDetached();
}

void KToshiba::parseTVAPEvents(int event)
{
    qDebug() << "Received event" << hex << event;
    switch(event) {
    case KToshibaNetlinkEvents::Hotkey:
        break;
    case KToshibaNetlinkEvents::DockDocked:
    case KToshibaNetlinkEvents::DockUndocked:
    case KToshibaNetlinkEvents::DockStatusChanged: 
        break;
    case KToshibaNetlinkEvents::Thermal:
        break;
    case KToshibaNetlinkEvents::LIDClosed:
    case KToshibaNetlinkEvents::LIDClosedDockEjected:
        break;
    case KToshibaNetlinkEvents::SATAPower1:
    case KToshibaNetlinkEvents::SATAPower2:
        break;
    case KToshibaNetlinkEvents::KBDBacklight:
        m_fn->updateKBDBacklight();
        break;
    default:
        qDebug() << "Unknown event";
    }
}


#include "ktoshiba.moc"
