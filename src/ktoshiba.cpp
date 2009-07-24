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

//#include <QtDBus/QDBusConnection>

#include <KApplication>
#include <KAboutData>
#include <KLocale>
#include <KAction>
#include <KMenu>
#include <KHelpMenu>
#include <knotificationitem.h>

#include "ktoshiba.h"
#include "fnactions.h"
#include "version.h"

KToshiba::KToshiba()
    : KUniqueApplication(),
      m_Fn( new FnActions( this ) ),
      m_trayicon( new Experimental::KNotificationItem(this) )
{
    m_trayicon->setIconByName("ktoshiba");
    m_trayicon->setToolTip("ktoshiba", "KToshiba", i18n("Fn key monitoring for Toshiba laptops"));
    m_trayicon->setCategory(Experimental::KNotificationItem::Hardware);
    m_trayicon->setStatus( Experimental::KNotificationItem::Passive );

    KMenu *popupMenu = m_trayicon->contextMenu();

    // TODO: Find a way of changing the popup menu title
    // from KToshiba to the actual laptops model name
    //m_trayicon->setTitle(m_Fn->modelName());
    //popupMenu->setTitle(m_Fn->modelName());

    // TODO: Add other items here...
    popupMenu->addSeparator();
    KHelpMenu *m_helpMenu = new KHelpMenu( popupMenu, aboutData());
    popupMenu->addMenu( m_helpMenu->menu() )->setIcon( KIcon("help-contents") );
    m_helpMenu->action( KHelpMenu::menuHelpContents )->setVisible( false );
    m_helpMenu->action( KHelpMenu::menuWhatsThis )->setVisible( false );
    popupMenu->addSeparator();
}

KToshiba::~KToshiba()
{
    delete m_Fn;
    delete m_trayicon;
}

static const char * const description =
    I18N_NOOP("Fn key monitoring for Toshiba laptops.");

void KToshiba::createAboutData()
{
    m_about = new KAboutData("KToshiba", 0, ki18n("KToshiba"), ktoshiba_version,
			ki18n(description), KAboutData::License_GPL,
			ki18n("(C) 2004-2009, Azael Avalos"), ki18n(""),
			"http://ktoshiba.sourceforge.net/",
			"coproscefalo@gmail.com");

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
