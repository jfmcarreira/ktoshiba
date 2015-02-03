/*
   Copyright (C) 2004-2015  Azael Avalos <coproscefalo@gmail.com>

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
#include "ktoshibahddprotect.h"
#include "version.h"

static const char * const ktosh_config = "ktoshibarc";

KToshiba::KToshiba()
    : KUniqueApplication(),
      m_fn( new FnActions( this ) ),
      m_hdd( new KToshibaHDDProtect( this ) ),
      m_protectionWidget( new QWidget( 0, Qt::WindowStaysOnTopHint ) ),
      m_timeoutWidget( new QWidget( 0, Qt::WindowStaysOnTopHint ) ),
      m_config( KSharedConfig::openConfig( ktosh_config ) ),
      m_level( 2 ),
      m_trayicon( new KStatusNotifierItem( this ) )
{
    m_trayicon->setIconByName( "ktoshiba" );
    m_trayicon->setToolTip( "ktoshiba", "KToshiba", i18n("Fn key monitoring for Toshiba laptops") );
    m_trayicon->setCategory( KStatusNotifierItem::Hardware );
    m_trayicon->setStatus( KStatusNotifierItem::Passive );

    m_popupMenu = m_trayicon->contextMenu();
    m_trayicon->setAssociatedWidget( m_popupMenu );

    if (checkConfig())
        loadConfig();
    else
        createConfig();

    if (m_fn->hapsIsSupported()) {
        m_levels << i18n("Off") << i18n("Low") << i18n("Medium") << i18n("High");

        m_hdd->setHDDProtection(m_monitorHDD);
        if (m_level != m_fn->protectionLevel())
            m_fn->setProtectionLevel(m_level);

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

    if (m_fn->touchpadIsSupported()) {
        m_touchPad = m_popupMenu->addAction( i18n("Toggle TouchPad") );
        m_touchPad->setIcon( QIcon( ":images/mpad_64.png" ) );

        connect( m_touchPad, SIGNAL( triggered() ), m_fn, SLOT( toggleTouchPad() ) );
    }

    if (m_fn->kbdBacklightIsSupported()) {
        m_kbdModeMenu = new QMenu(m_popupMenu);
        m_kbdModeMenu->setTitle( i18n("Keyboard Backlight Mode") );
        m_popupMenu->addMenu( m_kbdModeMenu )->setIcon( KIcon( "input-keyboard" ) );
        if (m_fn->m_type == 1) {
            m_kbdFNZ = m_kbdModeMenu->addAction( i18n("FN-Z") );
            m_kbdFNZ->setIcon( QIcon( ":images/keyboard_black_on_64.png" ) );
            m_kbdFNZ->setCheckable( true );
            m_kbdFNZ->setChecked( m_fn->m_mode == FnActions::FNZ ? true : false );
            m_kbdTimer = m_kbdModeMenu->addAction( i18n("AUTO") );
            m_kbdTimer->setIcon( QIcon( ":images/keyboard_timer_64.png" ) );
            m_kbdTimer->setCheckable( true );
            m_kbdTimer->setChecked( m_fn->m_mode == FnActions::TIMER ? true : false );

            connect( m_kbdFNZ, SIGNAL( triggered() ), this, SLOT( fnzClicked() ) );
            connect( m_kbdTimer, SIGNAL( triggered() ), this, SLOT( timerClicked() ) );
        } else if (m_fn->m_type == 2) {
            m_kbdTimer = m_kbdModeMenu->addAction( i18n("TIMER") );
            m_kbdTimer->setIcon( QIcon( ":images/keyboard_timer_64.png" ) );
            m_kbdTimer->setCheckable( true );
            m_kbdTimer->setChecked( m_fn->m_mode == FnActions::TIMER ? true : false );
            m_kbdOn = m_kbdModeMenu->addAction( i18n("ON") );
            m_kbdOn->setIcon( QIcon( ":images/keyboard_black_on_64.png" ) );
            m_kbdOn->setCheckable( true );
            m_kbdOn->setChecked( m_fn->m_mode == FnActions::ON ? true : false );
            m_kbdOff = m_kbdModeMenu->addAction( i18n("OFF") );
            m_kbdOff->setIcon( QIcon( ":images/keyboard_black_off_64.png" ) );
            m_kbdOff->setCheckable( true );
            m_kbdOff->setChecked( m_fn->m_mode == FnActions::OFF ? true : false );

            connect( m_kbdTimer, SIGNAL( triggered() ), this, SLOT( timerClicked() ) );
            connect( m_kbdOn, SIGNAL( triggered() ), this, SLOT( onClicked() ) );
            connect( m_kbdOff, SIGNAL( triggered() ), this, SLOT( offClicked() ) );
        }

        m_kbdTimeoutWidget.setupUi( m_timeoutWidget );
        m_kbdTimeoutWidget.timeoutSpinBox->setMaximum(60);
        m_kbdTimeoutWidget.timeoutSpinBox->setMinimum(1);
        m_timeoutWidget->clearFocus();

        m_kbdTimeout = m_popupMenu->addAction( i18n("Keyboard Backlight Timeout") );
        m_kbdTimeout->setIcon( KIcon( "input-keyboard" ) );
        m_kbdTimeout->setVisible((m_fn->m_mode == FnActions::TIMER) ? true : false);

        connect( m_fn, SIGNAL( kbdModeChanged() ), this, SLOT( notifyKBDModeChanged() ) );
        connect( m_kbdTimeout, SIGNAL( triggered() ), this, SLOT( kbdTimeoutClicked() ) );
        connect( m_kbdTimeoutWidget.buttonBox, SIGNAL( clicked(QAbstractButton *) ),
		 this, SLOT( changeKBDTimeout(QAbstractButton *) ) );
        connect( m_kbdTimeoutWidget.timeoutSpinBox, SIGNAL( valueChanged(int) ),
		 this, SLOT( timeChanged(int) ) );
    }

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
    m_fn->m_batMonitor = m_batteryProfiles;
    connect( m_batDisabled, SIGNAL( toggled(bool) ), this, SLOT( disabledClicked(bool) ) );
    connect( m_batPerformance, SIGNAL( triggered() ), this, SLOT( performanceClicked() ) );
    connect( m_batPowersave, SIGNAL( triggered() ), this, SLOT( powersaveClicked() ) );
    connect( m_batPresentation, SIGNAL( triggered() ), this, SLOT( presentationClicked() ) );
    connect( m_batECO, SIGNAL( triggered() ), this, SLOT( ecoClicked() ) );

    m_popupMenu->addSeparator();
    m_helpMenu = new KHelpMenu( m_popupMenu, aboutData());
    m_popupMenu->addMenu( m_helpMenu->menu() )->setIcon( KIcon( "help-contents" ) );
    m_helpMenu->action( KHelpMenu::menuHelpContents )->setVisible( false );
    m_helpMenu->action( KHelpMenu::menuWhatsThis )->setVisible( false );
}

KToshiba::~KToshiba()
{
    delete m_fn; m_fn = NULL;
    delete m_hdd; m_hdd = NULL;
    delete m_protectionWidget; m_protectionWidget = NULL;
    delete m_timeoutWidget; m_timeoutWidget = NULL;
    delete m_helpMenu; m_helpMenu = NULL;
    delete m_trayicon; m_trayicon = NULL;
    delete m_popupMenu; m_popupMenu = NULL;
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
    m_monitorHDD = generalGroup.readEntry( "MonitorHDD", true );
    m_level = generalGroup.readEntry( "HDDProtectionLvl", 2 );
    m_batteryProfiles = generalGroup.readEntry( "BatteryProfiles", true );
}

void KToshiba::createConfig()
{
    kDebug() << "Default configuration file created.";
    KConfigGroup generalGroup( m_config, "General" );
    generalGroup.writeEntry( "MonitorHDD", true );
    generalGroup.writeEntry( "HDDProtectionLvl", 2 );
    generalGroup.writeEntry( "BatteryProfiles", true );
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
        m_level = m_fn->protectionLevel();
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
        m_fn->setProtectionLevel(m_level);
        m_protectionWidget->hide();
    break;
    case QDialogButtonBox::Apply:
        m_fn->setProtectionLevel(m_level);
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

void KToshiba::fnzClicked()
{
    m_fn->setKbdBacklightMode(FnActions::FNZ);
    m_kbdFNZ->setChecked( true );
    m_kbdTimer->setChecked( false );
}

void KToshiba::timerClicked()
{
    m_fn->setKbdBacklightMode(FnActions::TIMER);
    m_kbdTimer->setChecked( true );
    if (m_fn->m_type == 1) {
        m_kbdFNZ->setChecked( false );
    } else if (m_fn->m_type == 2) {
        m_kbdOn->setChecked( false );
        m_kbdOff->setChecked( false );
    }
}

void KToshiba::onClicked()
{
    m_fn->setKbdBacklightMode(FnActions::ON);
    m_kbdTimer->setChecked( false );
    m_kbdOn->setChecked( true );
    m_kbdOff->setChecked( false );
}

void KToshiba::offClicked()
{
    m_fn->setKbdBacklightMode(FnActions::OFF);
    m_kbdTimer->setChecked( false );
    m_kbdOn->setChecked( false );
    m_kbdOff->setChecked( true );
}

void KToshiba::notifyKBDModeChanged()
{
    m_fn->m_mode = m_fn->kbdBacklightMode();
    m_kbdTimeout->setVisible((m_fn->m_mode == FnActions::TIMER) ? true : false);

    if (m_fn->m_type == 1) {
        QIcon icon(":images/keyboard_black_on_64.png");
        KNotification *notification =
		KNotification::event(KNotification::Notification, i18n("KToshiba - Keyboard Mode"),
				     i18n("The computer must be restarted in order to activate the new keyboard Mode"),
				     icon.pixmap(48, 48), 0, KNotification::Persistent);
        notification->sendEvent();
    }
}

void KToshiba::kbdTimeoutClicked()
{
    if (m_timeoutWidget->isHidden()) {
        QRect r = QApplication::desktop()->geometry();
        m_timeoutWidget->move(r.center() -
                    QPoint(m_timeoutWidget->width() / 2, m_timeoutWidget->height() / 2));
        m_fn->m_time = m_fn->kbdBacklightTimeout();
        m_kbdTimeoutWidget.timeoutSpinBox->setValue(m_fn->m_time);
        m_timeoutWidget->show();
    } else {
        m_timeoutWidget->hide();
    }
}

void KToshiba::timeChanged(int time)
{
    if (time == m_fn->m_time)
        return;

    m_fn->m_time = time;
}

void KToshiba::changeKBDTimeout(QAbstractButton *button)
{
    QDialogButtonBox::StandardButton stdButton =
	    m_kbdTimeoutWidget.buttonBox->standardButton(button);

    switch(stdButton) {
    case QDialogButtonBox::Ok:
        m_fn->setKbdBacklightTimeout(m_fn->m_time);
        m_timeoutWidget->hide();
    break;
    case QDialogButtonBox::Apply:
        m_fn->setKbdBacklightTimeout(m_fn->m_time);
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

static const char * const description =
    I18N_NOOP("Fn key monitoring for Toshiba laptops.");

void KToshiba::createAboutData()
{
    m_about = new KAboutData("KToshiba", 0, ki18n("KToshiba"), ktoshiba_version,
			ki18n(description), KAboutData::License_GPL,
			ki18n("(C) 2004-2015, Azael Avalos"), KLocalizedString(),
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
