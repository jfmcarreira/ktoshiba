/*
   Copyright (C) 2004-2014  Azael Avalos <coproscefalo@gmail.com>

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

#include <QtGui/QDesktopWidget>
#include <QtGui/QDialogButtonBox>
#include <QRect>
#include <QAction>
#include <QTimer>

#include <KAboutData>
#include <KLocale>
#include <KMenu>
#include <KHelpMenu>
#include <KConfig>
#include <KConfigGroup>
#include <KStandardDirs>
#include <KDebug>
#include <KNotification>
#include <KIcon>
#include <KAuth/Action>
#include <KStatusNotifierItem>

#include "ktoshiba.h"
#include "fnactions.h"
#include "version.h"

#define HELPER_ID "net.sourceforge.ktoshiba.ktoshhelper"

static const char * const ktosh_config = "ktoshibarc";

KToshiba::KToshiba()
    : KUniqueApplication(),
      m_fn( new FnActions( this ) ),
      m_widget( new QWidget( 0, Qt::WindowStaysOnTopHint ) ),
      config( KSharedConfig::openConfig( ktosh_config ) ),
      m_trayicon( new KStatusNotifierItem( this ) )
{
    m_trayicon->setIconByName( "ktoshiba" );
    m_trayicon->setToolTip( "ktoshiba", "KToshiba", i18n("Fn key monitoring for Toshiba laptops") );
    m_trayicon->setCategory( KStatusNotifierItem::Hardware );
    m_trayicon->setStatus( KStatusNotifierItem::Passive );

    m_kbdTimeoutWidget.setupUi( m_widget );
    m_kbdTimeoutWidget.timeoutSpinBox->setMaximum(60);
    m_kbdTimeoutWidget.timeoutSpinBox->setMinimum(0);
    m_widget->clearFocus();

    m_mode = m_fn->getKBDMode();

    KMenu *popupMenu = m_trayicon->contextMenu();

    if (checkConfig())
        loadConfig();
    else
        createConfig();

    // TODO: Add other items here... If any...
    touchPad = popupMenu->addAction( i18n("Toggle TouchPad") );
    touchPad->setIcon( QIcon( ":images/mpad_64.png" ) );
    if (m_mode > 0) {
        popupMenu->addSeparator();
        kbdMode = popupMenu->addAction( i18n("Switch the keyboard backlight mode to %1", (m_mode == 1 ? "Auto" : "FN-Z")) );
        kbdMode->setIcon( KIcon( "input-keyboard" ) );
        kbdTimeout = popupMenu->addAction( i18n("Change the keyboard backlight timeout") );
        kbdTimeout->setIcon( KIcon( "input-keyboard" ) );
        if (m_mode == 2)
            kbdTimeout->setVisible(true);
        else
            kbdTimeout->setVisible(false);
    }
    popupMenu->addSeparator();
    KHelpMenu *m_helpMenu = new KHelpMenu( popupMenu, aboutData());
    popupMenu->addMenu( m_helpMenu->menu() )->setIcon( KIcon( "help-contents" ) );
    m_helpMenu->action( KHelpMenu::menuHelpContents )->setVisible( false );
    m_helpMenu->action( KHelpMenu::menuWhatsThis )->setVisible( false );
    popupMenu->addSeparator();

    connect( kbdMode, SIGNAL( triggered(bool) ), this, SLOT( changeKBDMode() ) );
    connect( kbdTimeout, SIGNAL( triggered(bool) ), this, SLOT( kbdTimeoutClicked() ) );
    connect( touchPad, SIGNAL( triggered(bool) ), m_fn, SLOT( toggleTouchPad() ) );
    connect( m_kbdTimeoutWidget.buttonBox, SIGNAL( clicked(QAbstractButton *) ),
             this, SLOT( changeKBDTimeout(QAbstractButton *) ) );
    connect( m_kbdTimeoutWidget.timeoutSpinBox, SIGNAL( valueChanged(int) ), this, SLOT( timeChanged(int) ) );
}

KToshiba::~KToshiba()
{
    delete m_fn; m_fn = NULL;
    delete m_trayicon;
    delete m_widget;
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
}

void KToshiba::createConfig()
{
    kDebug() << "Default configuration file created." << endl;
    KConfigGroup generalGroup( config, "General" );
    generalGroup.config()->sync();
}

void KToshiba::changeKBDMode()
{
    KNotification *notification = new KNotification("changedKBDMode", KNotification::Persistent, NULL);
    notification->setTitle("KToshiba - Attention");
    notification->setText(i18n("The computer must be restarted in order to activate the new keyboard Mode"));
    notification->setPixmap(QPixmap("input-keyboard"));
    notification->sendEvent();

    m_fn->changeKBDMode();
    m_mode = m_fn->getKBDMode();
    kbdMode->setText( i18n("Switch the keyboard backlight mode to %1", (m_mode == 1 ? "Auto" : "FN-Z")) );
}

void KToshiba::kbdTimeoutClicked()
{
    if (m_widget->isHidden()) {
        m_kbdTimeoutWidget.timeoutSpinBox->setValue(m_fn->getKBDTimeout());
        QRect r = QApplication::desktop()->geometry();
        m_widget->move(r.center() -
                    QPoint(m_widget->width() / 2, m_widget->height() / 2));
        m_time = m_fn->getKBDTimeout();
        m_kbdTimeoutWidget.timeoutSpinBox->setValue(m_time);
        m_widget->show();
    } else {
        m_widget->hide();
    }
}

void KToshiba::timeChanged(int time)
{
    if (time == m_time)
        return;

    m_time = time;
}

void KToshiba::changeKBDTimeout(QAbstractButton *button)
{
    QDialogButtonBox::StandardButton stdButton =
	    m_kbdTimeoutWidget.buttonBox->standardButton(button);

    switch(stdButton) {
    case QDialogButtonBox::Ok:
        m_fn->changeKBDTimeout(m_time);
        m_widget->hide();
    break;
    case QDialogButtonBox::Apply:
        m_fn->changeKBDTimeout(m_time);
    break;
    case QDialogButtonBox::Cancel:
        m_widget->hide();
    break;
    case QDialogButtonBox::NoButton:
    case QDialogButtonBox::Save:
    case QDialogButtonBox::SaveAll:
    case QDialogButtonBox::Open:
    case QDialogButtonBox::Yes:
    case QDialogButtonBox::YesToAll:
    case QDialogButtonBox::No:
    case QDialogButtonBox::NoToAll:
    case QDialogButtonBox::Abort:
    case QDialogButtonBox::Retry:
    case QDialogButtonBox::Ignore:
    case QDialogButtonBox::Close:
    case QDialogButtonBox::Discard:
    case QDialogButtonBox::Help:
    case QDialogButtonBox::Reset:
    case QDialogButtonBox::RestoreDefaults:
    break;
    }
}

static const char * const description =
    I18N_NOOP("Fn key monitoring for Toshiba laptops.");

void KToshiba::createAboutData()
{
    m_about = new KAboutData("KToshiba", 0, ki18n("KToshiba"), ktoshiba_version,
			ki18n(description), KAboutData::License_GPL,
			ki18n("(C) 2004-2013, Azael Avalos"), KLocalizedString(),
			"http://ktoshiba.sourceforge.net/",
			"coproscefalo@gmail.com");
    m_about->setProgramIconName("ktoshiba");

    m_about->addAuthor( ki18n("Azael Avalos"),
			ki18n("Original Author"), "coproscefalo@gmail.com" );
    m_about->addCredit( ki18n("John Belmonte"), ki18n("Toshiba Laptop ACPI Extras driver"),
                    "john@neggie.net", "http://memebeam.org/toys/ToshibaAcpiDriver/" );
    m_about->addCredit( ki18n("KDE Team"), ki18n("Some ideas and pieces of code"), 0,
                    "http://www.kde.org/" );
    m_about->addCredit( ki18n("Nicolas Ternisien"), ki18n("French translation"),
                    "nicolas.ternisien@gmail.com", 0 );
    m_about->addCredit( ki18n("Charles Barcza"), ki18n("Hungarian translation"),
                    "kbarcza@blackpanther.hu", "http://www.blackpanther.hu" );
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
