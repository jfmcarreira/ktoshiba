 /*
   Copyright (C) 2004-2011  Azael Avalos <coproscefalo@gmail.com>

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

#include <QX11Info>
#include <QAction>

#include <KAboutData>
#include <KLocale>
#include <KMenu>
#include <KHelpMenu>
#include <KConfig>
#include <KConfigGroup>
#include <KStandardDirs>
#include <KDebug>
#include <kstatusnotifieritem.h>

#include "ktoshiba.h"
#include "fnactions.h"
#include "version.h"

extern "C" {
#include <X11/Xlib.h>
}

static const char * const ktosh_config = "ktoshibarc";

KToshiba::KToshiba()
    : KUniqueApplication(), //QApplication(QX11Info::display()),
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

    // TODO: Add other items here... If any...
    autostart = popupMenu->addAction( i18n("Start Automatically") );
    autostart->setCheckable( true );
    autostart->setChecked( m_autoStart );
    popupMenu->addSeparator();
    mediaPlayerMenu = new KMenu( i18n("Media Player"), popupMenu );
    nomp = mediaPlayerMenu->addAction( QIcon(":/images/disabled.png"), "None" );
    nomp->setCheckable( true );
    nomp->setChecked( true );
    amarok = mediaPlayerMenu->addAction( KIcon("amarok"), "Amarok" );
    amarok->setCheckable( true );
    kaffeine = mediaPlayerMenu->addAction( KIcon("kaffeine"), "Kaffeine" );
    kaffeine->setCheckable( true );
    juk = mediaPlayerMenu->addAction( KIcon("juk"), "JuK" );
    juk->setCheckable( true );
    popupMenu->addMenu( mediaPlayerMenu )->setIcon( KIcon("applications-multimedia") );
    popupMenu->addSeparator();
    KHelpMenu *m_helpMenu = new KHelpMenu( popupMenu, aboutData());
    popupMenu->addMenu( m_helpMenu->menu() )->setIcon( KIcon( "help-contents" ) );
    m_helpMenu->action( KHelpMenu::menuHelpContents )->setVisible( false );
    m_helpMenu->action( KHelpMenu::menuWhatsThis )->setVisible( false );
    popupMenu->addSeparator();

    grabKeys();
    installEventFilter(m_Fn);

    connect( autostart, SIGNAL( toggled(bool) ), this, SLOT( autostartSlot(bool) ) );
    connect( mediaPlayerMenu, SIGNAL( triggered(QAction*) ), this, SLOT( mediaPlayerSlot(QAction*) ) );
    connect( m_Fn, SIGNAL( mediaPlayerChanged(int) ), this, SLOT( updateMediaPlayer(int) ) );
}

KToshiba::~KToshiba()
{
    delete m_Fn;
    delete m_trayicon;
}

void KToshiba::grabKeys()
{
    // Fn-F#
    //XGrabKey(QX11Info::display(), 121, AnyModifier, DefaultRootWindow(QX11Info::display()), False, GrabModeAsync, GrabModeAsync); // Mute
    XGrabKey(QX11Info::display(), 160, AnyModifier, DefaultRootWindow(QX11Info::display()), False, GrabModeAsync, GrabModeAsync); // Coffee
    XGrabKey(QX11Info::display(), 244, AnyModifier, DefaultRootWindow(QX11Info::display()), False, GrabModeAsync, GrabModeAsync); // Battery
    //XGrabKey(QX11Info::display(), 150, AnyModifier, DefaultRootWindow(QX11Info::display()), False, GrabModeAsync, GrabModeAsync); // Sleep
    //XGrabKey(QX11Info::display(), 213, AnyModifier, DefaultRootWindow(QX11Info::display()), False, GrabModeAsync, GrabModeAsync); // Hibernate
    XGrabKey(QX11Info::display(), 235, AnyModifier, DefaultRootWindow(QX11Info::display()), False, GrabModeAsync, GrabModeAsync); // Video-Out
    //XGrabKey(QX11Info::display(), 232, AnyModifier, DefaultRootWindow(QX11Info::display()), False, GrabModeAsync, GrabModeAsync); // Bright-Up
    //XGrabKey(QX11Info::display(), 233, AnyModifier, DefaultRootWindow(QX11Info::display()), False, GrabModeAsync, GrabModeAsync); // Bright-Down
    XGrabKey(QX11Info::display(), 246, AnyModifier, DefaultRootWindow(QX11Info::display()), False, GrabModeAsync, GrabModeAsync); // WLAN
    XGrabKey(QX11Info::display(), 156, AnyModifier, DefaultRootWindow(QX11Info::display()), False, GrabModeAsync, GrabModeAsync); // Prog1 (Toggle Touchpad)
    // Multimedia
    XGrabKey(QX11Info::display(), 234, AnyModifier, DefaultRootWindow(QX11Info::display()), False, GrabModeAsync, GrabModeAsync); // Media Player
    XGrabKey(QX11Info::display(), 172, AnyModifier, DefaultRootWindow(QX11Info::display()), False, GrabModeAsync, GrabModeAsync); // Play/Pause
    XGrabKey(QX11Info::display(), 136, AnyModifier, DefaultRootWindow(QX11Info::display()), False, GrabModeAsync, GrabModeAsync); // Stop
    XGrabKey(QX11Info::display(), 173, AnyModifier, DefaultRootWindow(QX11Info::display()), False, GrabModeAsync, GrabModeAsync); // Previous
    XGrabKey(QX11Info::display(), 171, AnyModifier, DefaultRootWindow(QX11Info::display()), False, GrabModeAsync, GrabModeAsync); // Next
    // Misc
    XGrabKey(QX11Info::display(), 180, AnyModifier, DefaultRootWindow(QX11Info::display()), False, GrabModeAsync, GrabModeAsync); // Homepage
    XGrabKey(QX11Info::display(), 236, AnyModifier, DefaultRootWindow(QX11Info::display()), False, GrabModeAsync, GrabModeAsync); // KBD Illumination
    XGrabKey(QX11Info::display(), 428, AnyModifier, DefaultRootWindow(QX11Info::display()), False, GrabModeAsync, GrabModeAsync); // Zoom Reset
    XGrabKey(QX11Info::display(), 427, AnyModifier, DefaultRootWindow(QX11Info::display()), False, GrabModeAsync, GrabModeAsync); // Zoom out
    XGrabKey(QX11Info::display(), 426, AnyModifier, DefaultRootWindow(QX11Info::display()), False, GrabModeAsync, GrabModeAsync); // Zoom In
    XGrabKey(QX11Info::display(), 157, AnyModifier, DefaultRootWindow(QX11Info::display()), False, GrabModeAsync, GrabModeAsync); // Prog2
}

bool KToshiba::x11EventFilter(XEvent *ev)
{
    if (ev->type == KeyPress) {
        kDebug() << "Got key: " << ev->xkey.keycode << endl;
        emit hotkeyPressed(ev->xkey.keycode);
    }

    return false;
}

bool KToshiba::checkConfig()
{
    KStandardDirs kstd;
    QString config = kstd.findResource("config", ktosh_config);

    if (config.isEmpty()) {
        kDebug() << "Configuration file not found." << endl;
        return false;
    }

    return true;
}

void KToshiba::loadConfig()
{
    kDebug() << "Loading configuration file..." << endl;
    KConfigGroup generalGroup( config, "General" );
    m_autoStart = generalGroup.readEntry( "AutoStart", true );
}

void KToshiba::createConfig()
{
    kDebug() << "Default configuration file created." << endl;
    KConfigGroup generalGroup( config, "General" );
    generalGroup.writeEntry( "AutoStart", true );
    generalGroup.config()->sync();
}

void KToshiba::clearMediaPlayerSelection()
{
    // Clear previous selection
    nomp->setChecked( false );
    amarok->setChecked( false );
    kaffeine->setChecked( false );
    juk->setChecked( false );
}

void KToshiba::autostartSlot(bool start)
{
    KConfigGroup generalGroup( config, "General" );
    generalGroup.writeEntry( "AutoStart", start );
    generalGroup.config()->sync();
}

void KToshiba::mediaPlayerSlot(QAction* action)
{
    clearMediaPlayerSelection();
    // Check the desired one
    action->setChecked( true );
    int player = -1;
    if (action == nomp)
        player = 0;
    else if (action == amarok)
        player = 1;
    else if (action == kaffeine)
        player = 2;
    else if (action == juk)
        player = 3;
    emit mediaPlayerChanged(player);
}

void KToshiba::updateMediaPlayer(int player)
{
    clearMediaPlayerSelection();
    // Now check the correct one
    if (player == 0)
        nomp->setChecked( true );
    else if (player == 1)
        amarok->setChecked( true );
    else if (player == 2)
        kaffeine->setChecked( true );
    else if (player == 3)
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
