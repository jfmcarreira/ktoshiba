 /*
   Copyright (C) 2004-2009  Azael Avalos <coproscefalo@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <stdio.h>
#include <stdlib.h>

#include <QAction>

#include <KApplication>
#include <KAboutData>
#include <KLocale>
#include <KMenu>
#include <KHelpMenu>
#include <KConfig>
#include <KConfigGroup>
#include <KStandardDirs>
#include <KIcon>
#include <kstatusnotifieritem.h>

#include "ktoshiba.h"
#include "fnactions.h"
#include "version.h"

static const char * const ktosh_config = "ktoshibarc";

KToshiba::KToshiba()
    : KUniqueApplication(),
      m_Fn( new FnActions( this ) ),
      mediaPlayerMenu( NULL ),
      autostart( NULL ),
      config( KSharedConfig::openConfig( ktosh_config ) ),
      m_autoStart( true ),
      m_trayicon( new KStatusNotifierItem( this ) )
{
    m_trayicon->setIconByName( "ktoshiba" );
    m_trayicon->setToolTip( "ktoshiba", "KToshiba", i18n("Fn key monitoring for Toshiba laptops") );
    m_trayicon->setCategory( KStatusNotifierItem::Hardware );
    m_trayicon->setStatus( KStatusNotifierItem::Passive );

    KMenu *popupMenu = m_trayicon->contextMenu();

    if (checkConfig())
        loadConfig();
    else
        createConfig();

    // UGLY, we should be displaying model name only instead of KToshiba...
    popupMenu->addTitle( m_Fn->modelName() );
    // TODO: Add other items here... If any...
    autostart = popupMenu->addAction( i18n("Start Automatically") );
    autostart->setCheckable( true );
    autostart->setChecked( m_autoStart );
    popupMenu->addSeparator();
    mediaPlayerMenu = new KMenu( i18n("Media Player"), popupMenu );
    amarok = mediaPlayerMenu->addAction( "Amarok" );
    amarok->setCheckable( true );
    amarok->setIcon( KIcon("amarok") );
    amarok->setChecked( true );
    kaffeine = mediaPlayerMenu->addAction( "Kaffeine" );
    kaffeine->setCheckable( true );
    kaffeine->setIcon( KIcon("kaffeine") );
    juk = mediaPlayerMenu->addAction( "JuK" );
    juk->setCheckable( true );
    juk->setIcon( KIcon("juk") );
    popupMenu->addMenu( mediaPlayerMenu )->setIcon( KIcon("applications-multimedia") );
    popupMenu->addSeparator();
    KHelpMenu *m_helpMenu = new KHelpMenu( popupMenu, aboutData());
    popupMenu->addMenu( m_helpMenu->menu() )->setIcon( KIcon( "help-contents" ) );
    m_helpMenu->action( KHelpMenu::menuHelpContents )->setVisible( false );
    m_helpMenu->action( KHelpMenu::menuWhatsThis )->setVisible( false );
    popupMenu->addSeparator();

    connect( autostart, SIGNAL( toggled(bool) ), this, SLOT( autostartSlot(bool) ) );
    connect( mediaPlayerMenu, SIGNAL( triggered(QAction*) ), this, SLOT( mediaPlayerSlot(QAction*) ) );
    connect( m_Fn, SIGNAL( mediaPlayerChanged(int) ), this, SLOT( updateMediaPlayer(int) ) );
}

KToshiba::~KToshiba()
{
    delete m_Fn;
    delete m_trayicon;
}

bool KToshiba::checkConfig()
{
    KStandardDirs kstd;
    QString config = kstd.findResource("config", ktosh_config);

    if (config.isEmpty()) {
        //kDebug() << "checkConfig: Configuration file not found." << endl;
        return false;
    }

    return true;
}

void KToshiba::loadConfig()
{
    KConfigGroup generalGroup( config, "General" );
    m_autoStart = generalGroup.readEntry( "AutoStart", true );
}

void KToshiba::createConfig()
{
    KConfigGroup generalGroup( config, "General" );
    generalGroup.writeEntry( "AutoStart", true );
    generalGroup.config()->sync();
}

void KToshiba::autostartSlot(bool start)
{
    KConfigGroup generalGroup( config, "General" );
    generalGroup.writeEntry( "AutoStart", start );
    generalGroup.config()->sync();
}

void KToshiba::mediaPlayerSlot(QAction* action)
{
    // Clear previous selection
    amarok->setChecked( false );
    kaffeine->setChecked( false );
    juk->setChecked( false );
    // Check the desired one
    action->setChecked( true );
    int player = -1;
    if (action == amarok)
        player = 0;
    else if (action == kaffeine)
        player = 1;
    else if (action == juk)
        player = 2;
    emit mediaPlayerChanged(player);
}

void KToshiba::updateMediaPlayer(int player)
{
    // Clear previous selection
    amarok->setChecked( false );
    kaffeine->setChecked( false );
    juk->setChecked( false );
    // Now check the correct one
    if (player == 0)
        amarok->setChecked( true );
    else if (player == 1)
        kaffeine->setChecked( true );
    else if (player == 2)
        juk->setChecked( true );
}

static const char * const description =
    I18N_NOOP("Fn key monitoring for Toshiba laptops.");

void KToshiba::createAboutData()
{
    m_about = new KAboutData("KToshiba", 0, ki18n("KToshiba"), ktoshiba_version,
			ki18n(description), KAboutData::License_GPL,
			ki18n("(C) 2004-2009, Azael Avalos"), KLocalizedString(),
			"http://ktoshiba.sourceforge.net/",
			"coproscefalo@gmail.com");
    m_about->setProgramIconName("ktoshiba");

    m_about->addAuthor( ki18n("Azael Avalos"),
			ki18n("Original Author"), "coproscefalo@gmail.com" );
    m_about->addCredit( ki18n("Jonathan A. Buzzard"), ki18n("toshutils and HCI-SCI stuff"),
                    "jonathan@buzzard.org.uk", "http://www.buzzard.org.uk/toshiba/" );
    m_about->addCredit( ki18n("John Belmonte"), ki18n("Toshiba Laptop ACPI Extras driver"),
                    "john@neggie.net", "http://memebeam.org/toys/ToshibaAcpiDriver/" );
    m_about->addCredit( ki18n("Thomas Renninger"), ki18n("Powersave Daemon & KPowersave"),
                    "trenn@suse.de", 0 );
    m_about->addCredit( ki18n("KDE Team"), ki18n("Some ideas and pieces of code"), 0,
                    "http://www.kde.org/" );
    m_about->addCredit( ki18n("ksynaptics Team"), ki18n("library for enabling/disabling TouchPad"),
                    0, "http://qsynaptics.sourceforge.net/" );
    m_about->addCredit( ki18n("Nicolas Ternisien"), ki18n("French translation"),
                    "nicolas.ternisien@gmail.com", 0 );
    m_about->addCredit( ki18n("Charles Barcza"), ki18n("Hungarian translation"),
                    "kbarcza@blackpanther.hu", "http://www.blackpanther.hu" );
    m_about->addCredit( ki18n("Gonzalo Raúl Nemmi"), ki18n("omnibook stuff tester"),
                    "gnemmi@gmail.com", 0 );
    m_about->addCredit( ki18n("Rolf Eike Beer"), ki18n("Porting to new systray framework"),
                    "kde@opensource.sf-tec.de", 0 );
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
