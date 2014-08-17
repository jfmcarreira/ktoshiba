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

#include <KStatusNotifierItem>
#include <KMenu>
#include <KHelpMenu>
#include <KStandardDirs>
#include <KDebug>
#include <KNotification>
#include <KAboutData>
#include <KConfigGroup>
#include <KIcon>

#include "ktoshiba.h"
#include "fnactions.h"
#include "helperactions.h"
#include "ktoshibahddprotect.h"
#include "version.h"

static const char * const ktosh_config = "ktoshibarc";

KToshiba::KToshiba()
    : KUniqueApplication(),
      m_fn( new FnActions( this ) ),
      m_helper( new HelperActions( this ) ),
      m_hdd( new KToshibaHDDProtect( this ) ),
      m_protectionWidget( new QWidget( 0, Qt::WindowStaysOnTopHint ) ),
      m_timeoutWidget( new QWidget( 0, Qt::WindowStaysOnTopHint ) ),
      m_config( KSharedConfig::openConfig( ktosh_config ) ),
      m_modeText( i18n("Switch the keyboard backlight mode to %1") ),
      m_mode( FnActions::NotAvailable ),
      m_trayicon( new KStatusNotifierItem( this ) )
{
    m_trayicon->setIconByName( "ktoshiba" );
    m_trayicon->setToolTip( "ktoshiba", "KToshiba", i18n("Fn key monitoring for Toshiba laptops") );
    m_trayicon->setCategory( KStatusNotifierItem::Hardware );
    m_trayicon->setStatus( KStatusNotifierItem::Passive );

    KMenu *m_popupMenu = m_trayicon->contextMenu();

    if (checkConfig())
        loadConfig();
    else
        createConfig();

    m_autoStart = m_popupMenu->addAction( i18n("Start Automatically") );
    m_autoStart->setCheckable( true );
    m_autoStart->setChecked( m_autostart );
    connect( m_autoStart, SIGNAL( toggled(bool) ), this, SLOT( autostartClicked(bool) ) );

    if (m_helper->isAccelSupported) {
        m_levels << i18n("Off") << i18n("Low") << i18n("Medium") << i18n("High");

        m_hdd->setHDDProtection(m_monitorHDD);
        if (m_level != m_helper->getProtectionLevel())
            m_helper->setProtectionLevel(m_level);

        m_hddProtectionWidget.setupUi( m_protectionWidget );
        m_hddProtectionWidget.levelLabel->setText(m_levels.at(m_level));
        m_hddProtectionWidget.levelSlider->setValue(m_level);
        m_protectionWidget->clearFocus();

        m_hddMonitor = m_popupMenu->addAction( i18n("Monitor HDD movement") );
        m_hddMonitor->setCheckable( true );
        m_hddMonitor->setChecked( m_monitorHDD );
        m_hddMonitor->setIcon( KIcon( "drive-harddisk" ) );
        m_hddProtectionLvl = m_popupMenu->addAction( i18n("HDD Protection Level") );
        m_hddProtectionLvl->setIcon( KIcon( "drive-harddisk" ) );

        connect( m_hddMonitor, SIGNAL( toggled(bool) ), this, SLOT( monitorHDDClicked(bool) ) );
        connect( m_hdd, SIGNAL( movementDetected() ), this, SLOT( notifyHDDMovement() ) );
        connect( m_hddProtectionLvl, SIGNAL( triggered() ), this, SLOT( protectionLvlClicked() ) );
        connect( m_hddProtectionWidget.buttonBox, SIGNAL( clicked(QAbstractButton *) ),
		 this, SLOT( changeProtectionLvl(QAbstractButton *) ) );
        connect( m_hddProtectionWidget.levelSlider, SIGNAL( valueChanged(int) ),
		 this, SLOT( levelChanged(int) ) );
    }

    if (m_helper->isTouchPadSupported) {
        m_touchPad = m_popupMenu->addAction( i18n("Toggle TouchPad") );
        m_touchPad->setIcon( QIcon( ":images/mpad_64.png" ) );

        connect( m_touchPad, SIGNAL( triggered() ), m_helper, SLOT( toggleTouchPad() ) );
    }

    if (m_helper->isKBDBacklightSupported) {
        m_kbdTimeoutWidget.setupUi( m_timeoutWidget );
        m_kbdTimeoutWidget.timeoutSpinBox->setMaximum(60);
        m_kbdTimeoutWidget.timeoutSpinBox->setMinimum(0);
        m_timeoutWidget->clearFocus();

        m_mode = m_helper->getKBDMode();

        m_kbdMode = m_popupMenu->addAction( m_modeText.arg( (m_mode == FnActions::FNZMode) ? "Auto" : "FN-Z") );
        m_kbdMode->setIcon( KIcon( "input-keyboard" ) );
        m_kbdTimeout = m_popupMenu->addAction( i18n("Change the keyboard backlight timeout") );
        m_kbdTimeout->setIcon( KIcon( "input-keyboard" ) );
        m_kbdTimeout->setVisible((m_mode == FnActions::AutoMode) ? true: false);

        connect( m_kbdMode, SIGNAL( triggered() ), this, SLOT( changeKBDMode() ) );
        connect( m_kbdTimeout, SIGNAL( triggered() ), this, SLOT( kbdTimeoutClicked() ) );
        connect( m_kbdTimeoutWidget.buttonBox, SIGNAL( clicked(QAbstractButton *) ),
		 this, SLOT( changeKBDTimeout(QAbstractButton *) ) );
        connect( m_kbdTimeoutWidget.timeoutSpinBox, SIGNAL( valueChanged(int) ),
		 this, SLOT( timeChanged(int) ) );
        connect( m_helper, SIGNAL( kbdModeChanged() ), this, SLOT( notifyKBDModeChanged() ) );
        connect( m_helper, SIGNAL( kbdModeChanged(int) ), this, SLOT( changeKBDModeText(int) ) );
    }

    m_popupMenu->addSeparator();
    m_helpMenu = new KHelpMenu( m_popupMenu, aboutData());
    m_popupMenu->addMenu( m_helpMenu->menu() )->setIcon( KIcon( "help-contents" ) );
    m_helpMenu->action( KHelpMenu::menuHelpContents )->setVisible( false );
    m_helpMenu->action( KHelpMenu::menuWhatsThis )->setVisible( false );
}

KToshiba::~KToshiba()
{
    delete m_fn; m_fn = NULL;
    delete m_helper; m_helper = NULL;
    delete m_hdd; m_hdd = NULL;
    delete m_protectionWidget; m_protectionWidget = NULL;
    delete m_timeoutWidget; m_timeoutWidget = NULL;
    delete m_trayicon; m_trayicon = NULL;
}

bool KToshiba::checkConfig()
{
    KStandardDirs kstd;
    QString config = kstd.findResource("config", ktosh_config);

    if (config.isEmpty()) {
        kDebug() << "Configuration file not found.";
        return false;
    }

    return true;
}

void KToshiba::loadConfig()
{
    kDebug() << "Loading configuration file...";
    KConfigGroup generalGroup( m_config, "General" );
    m_autostart = generalGroup.readEntry( "AutoStart", true );
    m_monitorHDD = generalGroup.readEntry( "MonitorHDD", true );
    m_level = generalGroup.readEntry( "HDDProtectionLvl", 2 );
}

void KToshiba::createConfig()
{
    kDebug() << "Default configuration file created.";
    KConfigGroup generalGroup( m_config, "General" );
    generalGroup.writeEntry( "AutoStart", true );
    generalGroup.writeEntry( "MonitorHDD", true );
    generalGroup.writeEntry( "HDDProtectionLvl", 2 );
    generalGroup.config()->sync();
}

void KToshiba::autostartClicked(bool enabled)
{
    KConfigGroup generalGroup( m_config, "General" );
    generalGroup.writeEntry( "AutoStart", enabled );
    generalGroup.config()->sync();
}

void KToshiba::monitorHDDClicked(bool enabled)
{
    KConfigGroup generalGroup( m_config, "General" );
    generalGroup.writeEntry( "MonitorHDD", enabled );
    generalGroup.config()->sync();

    m_hdd->setHDDProtection(enabled);
}

void KToshiba::notifyHDDMovement()
{
    KNotification *notification =
		KNotification::event(KNotification::Notification, i18n("KToshiba - HDD Monitor"),
				     i18n("Vibration has been detected and the HDD has been stopped to prevent damage"),
				     QIcon::fromTheme("drive-harddisk").pixmap(48, 48), 0, KNotification::Persistent);
    notification->sendEvent();
}

void KToshiba::protectionLvlClicked()
{
    if (m_protectionWidget->isHidden()) {
        QRect r = QApplication::desktop()->geometry();
        m_protectionWidget->move(r.center() -
                    QPoint(m_protectionWidget->width() / 2, m_protectionWidget->height() / 2));
        m_level = m_helper->getProtectionLevel();
        m_hddProtectionWidget.levelSlider->setValue(m_level);
        m_hddProtectionWidget.levelLabel->setText(m_levels.at(m_level));
        m_protectionWidget->show();
    } else {
        m_protectionWidget->hide();
    }
}

void KToshiba::levelChanged(int level)
{
    m_hddProtectionWidget.levelLabel->setText(m_levels.at(level));

    if (level == m_level)
        return;

    m_level = level;
}

void KToshiba::changeProtectionLvl(QAbstractButton *button)
{
    QDialogButtonBox::StandardButton stdButton =
	    m_hddProtectionWidget.buttonBox->standardButton(button);

    switch(stdButton) {
    case QDialogButtonBox::Ok:
        m_helper->setProtectionLevel(m_level);
        m_protectionWidget->hide();
    break;
    case QDialogButtonBox::Apply:
        m_helper->setProtectionLevel(m_level);
    break;
    case QDialogButtonBox::Cancel:
        m_protectionWidget->hide();
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

    KConfigGroup generalGroup( m_config, "General" );
    generalGroup.writeEntry( "HDDProtectionLvl", m_level );
    generalGroup.config()->sync();
}

void KToshiba::notifyKBDModeChanged()
{
    QIcon icon(":images/keyboard_black_64.png");
    KNotification *notification =
		KNotification::event(KNotification::Notification, i18n("KToshiba - Keyboard Mode"),
				     i18n("The computer must be restarted in order to activate the new keyboard Mode"),
				     icon.pixmap(48, 48), 0, KNotification::Persistent);
    notification->sendEvent();
}

void KToshiba::changeKBDModeText(int mode)
{
    m_kbdMode->setText( m_modeText.arg( (mode == FnActions::FNZMode) ? "FN-Z" : "Auto") );
}

void KToshiba::changeKBDMode()
{
    int mode = (m_helper->getKBDMode() == FnActions::FNZMode) ? FnActions::AutoMode : FnActions::FNZMode;
    m_helper->setKBDMode(mode);
    changeKBDModeText(mode);
}

void KToshiba::kbdTimeoutClicked()
{
    if (m_timeoutWidget->isHidden()) {
        QRect r = QApplication::desktop()->geometry();
        m_timeoutWidget->move(r.center() -
                    QPoint(m_timeoutWidget->width() / 2, m_timeoutWidget->height() / 2));
        m_time = m_helper->getKBDTimeout();
        m_kbdTimeoutWidget.timeoutSpinBox->setValue(m_time);
        m_timeoutWidget->show();
    } else {
        m_timeoutWidget->hide();
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
        m_helper->setKBDTimeout(m_time);
        m_timeoutWidget->hide();
    break;
    case QDialogButtonBox::Apply:
        m_helper->setKBDTimeout(m_time);
    break;
    case QDialogButtonBox::Cancel:
        m_timeoutWidget->hide();
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
			ki18n("(C) 2004-2014, Azael Avalos"), KLocalizedString(),
			"http://ktoshiba.sourceforge.net/",
			"coproscefalo@gmail.com");
    m_about->setProgramIconName("ktoshiba");

    m_about->addAuthor( ki18n("Azael Avalos"),
			ki18n("Original Author"), "coproscefalo@gmail.com" );
    m_about->addCredit( ki18n("KDE Team"), ki18n("Some ideas and pieces of code"), 0,
                    "http://www.kde.org/" );
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
