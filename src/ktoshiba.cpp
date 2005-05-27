/***************************************************************************
 *   Copyright (C) 2004 by Azael Avalos                                    *
 *   neftali@utep.edu                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
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

#include "ktoshiba.h"
#include "modelid.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qprogressbar.h>
#include <qslider.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qtooltip.h>
#include <qapplication.h>
#include <qwidget.h>
#include <qwidgetstack.h>

#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kpassivepopup.h>
#include <kiconloader.h>
#include <kprocess.h>
#include <kpopupmenu.h>
#include <dcopclient.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kwin.h>
#include <dcopref.h>

#include <sys/shm.h>

#include "settingswidget.h"
#include "statuswidget.h"

#define CONFIG_FILE "ktoshibarc"

KToshiba::KToshiba()
    : KSystemTray( 0, "KToshiba" ),
      mDriver( 0 ),
      m_synShm( 0 ),
      mPowTimer( new QTimer(this) ),
      mHotKeysTimer( new QTimer(this) ),
      mModeTimer( new QTimer(this) ),
      mSystemTimer( new QTimer(this) )
{
    mDriver = new KToshibaSMMInterface(this);
    mAboutWidget = new KAboutApplication(this, "About Widget", false);
    mSettingsWidget = new SettingsWidget(0, "Screen Indicator", Qt::WX11BypassWM);
    mSettingsWidget->setFocusPolicy(QWidget::NoFocus);
    mStatusWidget = new StatusWidget(0, "Screen Indicator", Qt::WX11BypassWM);
    mStatusWidget->setFocusPolicy(QWidget::NoFocus);
    instance = new KInstance("ktoshiba");

    // check whether toshiba module is loaded
    mInterfaceAvailable = mDriver->openInterface();
    if (!mInterfaceAvailable) {
        kdDebug() << "KToshiba: Could not open interface. "
                  << "Please check that the toshiba module loads without failures"
                  << endl;
        exit(-1);
    } else {
        kdDebug() << "KToshiba: Interface opened successfully." << endl;
        mBatType = mDriver->getBatterySaveModeType();
        mBootType = mDriver->getBootType();
        mDriver->batteryStatus(&time, &perc);
        mWirelessSwitch = mDriver->getWirelessSwitch();
        mOldWirelessSwitch = mWirelessSwitch;
        mPad = mDriver->getPointingDevice();
        mHT = mDriver->getHyperThreading();
        mSS = mDriver->getSpeedStep();
        mAC = mDriver->acPowerStatus();
        video = mDriver->getVideo();
        bright = mDriver->getBrightness();
        boot = mDriver->getBootMethod();
        (perc == 100)? battrig = true : battrig = false;
        crytrig = false;
        lowtrig = false;
        battrig = false;
        wstrig = false;
        baytrig = false;
        snd = 1;
        popup = 0;
        mousepad = 0;
        bluetooth = 0;
        wireless = 1;
        fan = -1;
        vol = -1;
        sblock = HCI_LOCKED;
        removed = 0;
        svideo = 0;
    }

    if (mPad == -1) {
        // Code below taken from ksynaptics
        int shmid;
        if ((shmid = shmget(SHM_SYNAPTICS, sizeof(SynapticsSHM), 0)) == -1) {
            if ((shmid = shmget(SHM_SYNAPTICS, 0, 0)) == -1) 
                kdError() << "KToshiba: Access denied to driver shared memory" << endl;
            else
                kdError() << "KToshiba: Shared memory segment size mismatch" << endl;
            m_synShm = NULL;
        }
        else {
            if ((m_synShm = (SynapticsSHM*) shmat(shmid, NULL, 0)) == NULL)
                kdError() << "KToshiba: Error attaching to shared memory segment" << endl;
        }
    }

    if (!mClient.attach())
        kdDebug() << "KToshiba: cannot attach to DCOP server." << endl;

    KConfig mConfig(CONFIG_FILE);
    //mConfig.setGroup("General");
    //if (mInterfaceAvailable)
    //    mConfig.writeEntry("Autostart", true);
    //mConfig.sync();
    loadConfiguration(&mConfig);

    doMenu();

    noBatteryIcon = QString("laptop_nobattery");
    noChargeIcon = QString("laptop_nocharge");
    chargeIcon = QString("laptop_charge");

    connect( mPowTimer, SIGNAL( timeout() ), SLOT( powerStatus() ) );
    mPowTimer->start(mBatStatus * 1000);
    connect( mHotKeysTimer, SIGNAL( timeout() ), SLOT( checkHotKeys() ) );
    mHotKeysTimer->start(100);		// Check hotkeys every 1/10 seconds
    connect( mModeTimer, SIGNAL( timeout() ), SLOT( mode() ) );
    mModeTimer->start(500);		// Check proc entry every 1/2 seconds
    connect( mSystemTimer, SIGNAL( timeout() ), SLOT( checkSystem() ) );
    mSystemTimer->start(500);		// Check system events every 1/2 seconds

    displayPixmap();
    setModel();
    if (btstart)
        doBluetooth();
}

KToshiba::~KToshiba()
{
    // Stop timers
    mPowTimer->stop();
    mHotKeysTimer->stop();
    mModeTimer->stop();
    mSystemTimer->stop();

    if (mClient.isAttached())
        mClient.detach();
    if (mInterfaceAvailable) {
        kdDebug() << "KToshiba: Closing interface." << endl;
        mDriver->closeInterface();
        delete mDriver; mDriver = NULL;
    }
    if (m_synShm != 0) {
        delete m_synShm; m_synShm = NULL;
    }

    delete instance; instance = NULL;
    delete mStatusWidget; mStatusWidget = NULL;
    delete mSettingsWidget; mSettingsWidget = NULL;
    delete mAboutWidget; mAboutWidget = NULL;
}

void KToshiba::doMenu()
{
    this->contextMenu()->insertItem( SmallIcon("configure"), i18n("&Configure KToshiba..."), this,
                                     SLOT( doConfig() ), 0, 1, 1 );
    this->contextMenu()->insertSeparator( 2 );
    this->contextMenu()->insertItem( SmallIcon("hdd_unmount"), i18n("Suspend To &Disk"), this,
                                     SLOT( doSuspendToDisk() ), 0, 3, 3 );
    this->contextMenu()->insertItem( SmallIcon("memory"), i18n("Suspend To &RAM"), this,
                                     SLOT( doSuspendToRAM() ), 0, 4, 4 );
    this->contextMenu()->insertSeparator( 5 );
    this->contextMenu()->insertItem( SmallIcon("kdebluetooth"), i18n("Enable &Bluetooth"), this,
                                     SLOT( doBluetooth() ), 0, 6, 6 );
    this->contextMenu()->insertSeparator( 7 );

    mSpeed = new QPopupMenu( this, i18n("SpeedStep") );
    mSpeed->insertItem( SmallIcon("cpu_dynamic"), i18n("Dynamic"), 0 );
    mSpeed->insertItem( SmallIcon("cpu_high"), i18n("Always High"), 1 );
    mSpeed->insertItem( SmallIcon("cpu_low"), i18n("Always Low"), 2 );
    this->contextMenu()->insertItem( SmallIcon("kcmprocessor"), i18n("CPU Frequency"), mSpeed, 8, 8 );
    if (mSS < 0) this->contextMenu()->setItemEnabled( 8, FALSE );
    else if (mSS <= 1)
        connect( mSpeed, SIGNAL( activated(int) ), this, SLOT( setFreq(int) ) );

    this->contextMenu()->insertSeparator( 9 );
    this->contextMenu()->insertItem( i18n("&About KToshiba"), this,
                                     SLOT( displayAbout() ), 0, 10, 10 );
}

void KToshiba::doConfig()
{
    KProcess proc;
    proc << KStandardDirs::findExe("kcmshell");
    proc << "ktoshibam";
    proc.start(KProcess::DontCare);
    proc.detach();
}

void KToshiba::loadConfiguration(KConfig *k)
{
    k->setGroup("KToshiba");

    mFullBat = k->readBoolEntry("Notify_On_Full_Battery", false);
    mBatStatus = k->readNumEntry("Battery_Status_Time", 2);
    mLowBat = k->readNumEntry("Low_Battery_Trigger", 15);
    mCryBat = k->readNumEntry("Critical_Battery_Trigger", 5);
    mBatSave = k->readNumEntry("Battery_Save_Mode", 2);
    mAudioPlayer = k->readNumEntry("Audio_Player", 1);
    btstart = k->readBoolEntry("Bluetooth_Startup", true);
}

void KToshiba::displayAbout()
{
    mAboutWidget->show();
}

void KToshiba::powerStatus()
{
	KConfig mConfig(CONFIG_FILE);
	mConfig.setGroup("KToshiba");
	loadConfiguration(&mConfig);
	mDriver->batteryStatus(&time, &perc);
	pow = ((mAC == -1)? SciACPower() : mDriver->acPowerStatus());

	if (mFullBat && perc == 100 && !battrig) {
		KMessageBox::queuedMessageBox(0, KMessageBox::Information,
									  i18n("Your battery is now fully charged."), i18n("Laptop Battery"));
		battrig = true;
	}

	if (SCI_MINUTE(time) == mLowBat && SCI_HOUR(time) == 0 && pow == 3 && !lowtrig) {
		KPassivePopup::message(i18n("Warning"), i18n("The battery state has changed to low"),
							   SmallIcon("messagebox_warning", 20), this, i18n("Warning"), 15000);
		lowtrig = true;
	} else
	if (SCI_MINUTE(time) == mCryBat && SCI_HOUR(time) == 0 && pow == 3 && !crytrig) {
		KPassivePopup::message(i18n("Warning"), i18n("The battery state has changed to critical"),
							   SmallIcon("messagebox_warning", 20), this, i18n("Warning"), 15000);
		crytrig = true;
	} else
	if (SCI_MINUTE(time) == 0 && SCI_HOUR(time) == 0 && pow == 3)
		KPassivePopup::message(i18n("Warning"), i18n("I'm Gone..."),
							   SmallIcon("messagebox_warning", 20), this, i18n("Warning"), 15000);

	if (SCI_MINUTE(time) > lowtrig && pow == 4) {
		if (battrig && (perc < 100))
			battrig = false;
		if (lowtrig)
			lowtrig = false;
		if (crytrig)
			crytrig = false;
	}

	if (mBatSave != mOldBatSave || pow != oldpow) {
		switch (mBatSave) {
			case 0:			// USER SETTINGS or LONG LIFE
				if (mBatType == 3)
					bright = 0;	// Semi-Bright
				else if (mBatType == 2)
					bsmUserSettings(&mConfig);
				break;
			case 1:			// LOW POWER or NORMAL LIFE
				if (pow == 3)
					bright = 0;	// Semi-Bright
				else if (pow == 4)
					bright = 3;	// Bright
				break;
			case 2:			// FULL POWER or FULL LIFE
				if (pow == 3)
					bright = 3;	// Bright
				else if (pow == 4)
					bright = 7;	// Super-Bright
				break;
		}
		mDriver->setBrightness(bright);
	}

	if (mOldBatStatus != mBatStatus) {
		mPowTimer->stop();
		mPowTimer->start(mBatStatus * 1000);
	}

	bool changed = oldpow != pow || oldperc != perc || oldtime != time;
	oldperc = perc;
	oldpow = pow;
	oldtime = time;
	mOldBatStatus = mBatStatus;
	mOldBatSave = mBatSave;
	if (changed)
		displayPixmap();
}

void KToshiba::bsmUserSettings(KConfig *k)
{
    int processor, cpu, display, hdd, lcd, cooling;
    int tmp;

    processor = k->readNumEntry("Processing_Speed", 1);
    cpu = k->readNumEntry("CPU_Sleep_Mode", 0);
    display = k->readNumEntry("Display_Auto_Off", 5);
    hdd = k->readNumEntry("HDD_Auto_Off", 5);
    lcd = k->readNumEntry("LCD_Brightness", 2);
    cooling = k->readNumEntry("Cooling_Method", 2);

    kdDebug() << "Enabling User Settings..." << endl;
    (processor == 0)? tmp = 1 : tmp = 0;
    mDriver->setProcessingSpeed(tmp);
    (cpu == 0)? tmp = 1 : tmp = 0;
    mDriver->setCPUSleepMode(tmp);
    if (lcd == 0) bright = 7;		// Super-Bright
    else if (lcd == 1) bright = 3;	// Bright
    else if (lcd == 2) bright = 0;	// Semi-Bright
    mDriver->setCoolingMethod(cooling);
    mDriver->setDisplayAutoOff(display);
    mDriver->setHDDAutoOff(hdd);
}

void KToshiba::displayPixmap()
{
	int new_code = 0;
	int ac = mDriver->acPowerStatus();
	mDriver->batteryStatus(&time, &perc);

	// if we got a fail from HCI, try SCI function
	if (ac == -1)
		ac = SciACPower();

	if (!mInterfaceAvailable)
		new_code = 1;
	else if (ac == 3)
		new_code = 2;
	else
		new_code = 3;

	if (current_code != new_code) {
		current_code = new_code;
		// we will try to deduce the pixmap (or gif) name now.  it will
		// vary depending on the dock and power
		QString pixmap_name;

		if (!mInterfaceAvailable)
			pixmap_name = noBatteryIcon;
		else if (ac == 3)
			pixmap_name = noChargeIcon;
		else
			pixmap_name = chargeIcon;

		pm = loadIcon( pixmap_name, instance );
	}

	// at this point, we have the file to display.  so display it

	QImage image = pm.convertToImage();

	int w = image.width();
	int h = image.height();
	int count = 0;
	QRgb rgb;
	int x, y;
	for (x = 0; x < w; x++)
	for (y = 0; y < h; y++)
	{
		rgb = image.pixel(x, y);
		if (qRed(rgb) == 0xff &&
		    qGreen(rgb) == 0xff &&
		    qBlue(rgb) == 0xff)
			count++;
	}
	int c = (count*perc)/100;
	if (c == 100)
		c = count;
	else
	if (perc != 100 && c == count)
		c = count-1;


	if (c) {
		uint ui;
		QRgb blue = qRgb(0x00,0x00,0xff);

		if (image.depth() <= 8) {
			ui = image.numColors();		// this fix thanks to Sven Krumpke
			image.setNumColors(ui+1);
			image.setColor(ui, blue);
		} else
			ui = 0xff000000|blue;

		for (y = h-1; y >= 0; y--)
		for (x = 0; x < w; x++)
		{
			rgb = image.pixel(x, y);
			if (qRed(rgb) == 0xff &&
		    	    qGreen(rgb) == 0xff &&
		    	    qBlue(rgb) == 0xff) {
				image.setPixel(x, y, ui);
				c--;
				if (c <= 0)
					goto quit;
			}
		}
	}
quit:

	QPixmap q;
	q.convertFromImage(image);
	setPixmap(q);
	adjustSize();

	QString tmp;
	if (!mInterfaceAvailable)
		tmp = i18n("No interface available");
	else
	if (ac == 4) {
		if (perc == 100)
			tmp = i18n("Plugged in - fully charged");
		else {
			if (perc >= 0) {
				QString num3;
				num3.setNum(SCI_MINUTE(time));
				num3 = num3.rightJustify(2, '0');
				tmp = i18n("Plugged in - %1% charged (%2:%3 time left)")
					.arg(perc).arg(SCI_HOUR(time)).arg(num3);
			} else
			if (perc == -1)
				tmp = i18n("Plugged in - no battery");
			else
				tmp = i18n("Plugged in - %1% charged").arg(perc);
		}
	} else {
		if (perc >= 0) {
			QString num3;
			num3.setNum(SCI_MINUTE(time));
			num3 = num3.rightJustify(2, '0');
			tmp = i18n("Running on batteries - %1% charged (%2:%3 time left)")
					.arg(perc).arg(SCI_HOUR(time)).arg(num3);
		} else
			tmp = i18n("Running on batteries  - %1% charged").arg(perc);
	}
	QToolTip::add(this, tmp);
}

void KToshiba::checkHotKeys()
{
	KProcess proc;
	QByteArray data, replyData;
	QCString replyType;
	KConfig mConfig(CONFIG_FILE);
	mConfig.setGroup("KToshiba");

	int tmp = 0;
	int key = mDriver->getSystemEvent();

	if ((key == 0x100) && (popup != 0)) {
		mSettingsWidget->hide();
		mStatusWidget->hide();
		popup = 0;
	}

	switch (key) {
		case 0x101: // Fn-Esc
			tmp = mConfig.readNumEntry("Fn_Esc");
			break;
		case 0x13b: // Fn-F1
			tmp = mConfig.readNumEntry("Fn_F1");
			break;
		case 0x13c: // Fn-F2
			tmp = mConfig.readNumEntry("Fn_F2");
			break;
		case 0x13d: // Fn-F3
			tmp = mConfig.readNumEntry("Fn_F3");
			break;
		case 0x13e: // Fn-F4
			tmp = mConfig.readNumEntry("Fn_F4");
			break;
		case 0x13f: // Fn-F5
			tmp = mConfig.readNumEntry("Fn_F5");
			break;
		case 0x140: // Fn-F6
			tmp = mConfig.readNumEntry("Fn_F6");
			break;
		case 0x141: // Fn-F7
			tmp = mConfig.readNumEntry("Fn_F7");
			break;
		case 0x142: // Fn-F8
			tmp = mConfig.readNumEntry("Fn_F8");
			break;
		case 0x143: // Fn-F9
			tmp = mConfig.readNumEntry("Fn_F9");
			break;
		/** Front Panel Multimedia Buttons */
		case 0xb31:	// Previous
			if (MODE == CD_DVD) {
				if (!mClient.call("kscd", "CDPlayer", "previous()", data, replyType, replyData))
					mClient.send("kaffeine", "KaffeineIface", "previous()", "");
			} else
			if (MODE == DIGITAL) {
				if (mAudioPlayer == amaroK)
					mClient.send("amarok", "player", "prev()", "");
				else if (mAudioPlayer == JuK)
					mClient.send("juk", "Player", "back()", "");
				else if (mAudioPlayer == XMMS) {
					proc << "xmms" << "--rew";
					proc.start(KProcess::DontCare);
					proc.detach();
				}
			}
			return;
		case 0xb32:	// Next
			if (MODE == CD_DVD) {
				if (!mClient.call("kscd", "CDPlayer", "next()", data, replyType, replyData))
					mClient.send("kaffeine", "KaffeineIface", "next()", "");
			} else
			if (MODE == DIGITAL) {
				if (mAudioPlayer == amaroK)
					mClient.send("amarok", "player", "next()", "");
				else if (mAudioPlayer == JuK)
					mClient.send("juk", "Player", "forward()", "");
				else if (mAudioPlayer == XMMS) {
					proc << "xmms" << "--fwd";
					proc.start(KProcess::DontCare);
					proc.detach();
				}
			}
			return;
		case 0xb33:	// Play/Pause
			if (MODE == CD_DVD) {
				if (!mClient.call("kscd", "CDPlayer", "play()", data, replyType, replyData))
					if (!mClient.call("kaffeine", "KaffeineIface", "isPlaying()", data, replyType, replyData))
						kdDebug() << "KsCD and Kaffeine are not running" << endl;
					else {
						QDataStream reply(replyData, IO_ReadOnly);
						bool res;
						reply >> res;
						if (res)
							mClient.send("kaffeine", "KaffeineIface", "pause()", "");
						else
							mClient.send("kaffeine", "KaffeineIface", "play()", "");
					}
			} else
			if (MODE == DIGITAL) {
				if (mAudioPlayer == amaroK)
					mClient.send("amarok", "player", "playPause()", "");
				else if (mAudioPlayer == JuK)
					mClient.send("juk", "Player", "playPause()", "");
				else if (mAudioPlayer == XMMS) {
					proc << "xmms" << "--play-pause";
					proc.start(KProcess::DontCare);
					proc.detach();
				}
			}
			return;
		case 0xb30:	// Stop/Eject
			if (MODE == CD_DVD) {
				if (!mClient.call("kscd", "CDPlayer", "stop()", data, replyType, replyData))
					if (!mClient.call("kaffeine", "KaffeineIface", "stop()", data, replyType, replyData)) {
						proc << "eject" << "--cdrom";
						proc.start(KProcess::DontCare);
						proc.detach();
					}
			} else
			if (MODE == DIGITAL) {
				if (mAudioPlayer == amaroK)
					mClient.send("amarok", "player", "stop()", "");
				else if (mAudioPlayer == JuK)
					mClient.send("juk", "Player", "stop()", "");
				else if (mAudioPlayer == XMMS) {
					proc << "xmms" << "--stop";
					proc.start(KProcess::DontCare);
					proc.detach();
				}
			}
			return;
		case 0xb85:	// Toggle S-Video Out
			if (!svideo) {
				mDriver->setVideo(4);	// S-Video
				svideo = 1;
			}
			else if (svideo) {
				mDriver->setVideo(1);	// LCD
				svideo = 0;
			}
			return;
		case 0xb86:	// E-Button
			proc << "kfmclient" << "openProfile" << "webbrowsing";
			proc.start(KProcess::DontCare);
			proc.detach();
			return;
		case 0xb87:	// I-Button
			proc << "konsole";
			proc.start(KProcess::DontCare);
			proc.detach();
			return;
		case 1:
			if (mDriver->hotkeys == false) {
				mDriver->enableSystemEvent();
				mDriver->hotkeys = true;
			}
			return;
		case 0:	// FIFO empty
			return;
	}
	performFnAction(tmp, key);
}

void KToshiba::performFnAction(int action, int key)
{
	switch(action) {
		case 0: // Disabled (Do Nothing)
			return;
		case 2: // LockScreen
			lockScreen();
			return;
		case 4: // Suspend To RAM (STR)
			doSuspendToRAM();
			return;
		case 5: // Suspend To Disk (STD)
			doSuspendToDisk();
			return;
		case 9: // Wireless On/Off
			wireless--;
			if (wireless < 0) wireless = 1;
			toggleWireless();
			return;
		case 14: // LCD Backlight On/Off
			toogleBackLight();
			return;
		case 1: // Mute/Unmute
		case 7: // Brightness Down
		case 8: // Brightness Up
		case 10: // Enable/Disable MousePad
		case 11: // Speaker Volume
		case 12: // Fan On/Off
			if (popup == 0) {
				QRect r = QApplication::desktop()->geometry();
				mStatusWidget->move(r.center() - 
					QPoint(mStatusWidget->width()/2, mStatusWidget->height()/2));
				mStatusWidget->show();
				popup = key & 0x17f;
			}
			if ((key & 0x17f) == popup)
				goto update_widget;
			break;
		case 3: // Toggle Battery Save Mode
		case 6: // Toggle Video
		case 13: // Toggle Boot Method
			if (popup == 0) {
				QRect r = QApplication::desktop()->geometry();
				mSettingsWidget->move(r.center() - 
					QPoint(mSettingsWidget->width()/2, mSettingsWidget->height()/2));
				mSettingsWidget->show();
				popup = key & 0x17f;
			}
			if ((key & 0x17f) == popup)
				goto update_widget;
			break;
	}
update_widget:

	if (action == 3) {
		mBatSave--;
		if (mBatSave < 0) mBatSave = 2;
		if (mBatType == 3) mDriver->setBatterySaveMode(mBatSave + 1);
		else mDriver->setBatterySaveMode(mBatSave);
		KConfig cfg(CONFIG_FILE);
		cfg.setGroup("KToshiba");
		cfg.writeEntry("Battery_Save_Mode", mBatSave);
		mSettingsWidget->wsSettings->raiseWidget(0);
		mSettingsWidget->plUser->setFrameShape(QLabel::NoFrame);
		mSettingsWidget->plMedium->setFrameShape(QLabel::NoFrame);
		mSettingsWidget->plFull->setFrameShape(QLabel::NoFrame);
		switch (mBatSave) {
			case 0:
				(mBatType == 3)? mSettingsWidget->tlStatus->setText(i18n("Long Life"))
				: mSettingsWidget->tlStatus->setText(i18n("User Settings"));
				mSettingsWidget->plUser->setFrameShape(QLabel::PopupPanel);
				break;
			case 1:
				(mBatType == 3)? mSettingsWidget->tlStatus->setText(i18n("Normal Life"))
				: mSettingsWidget->tlStatus->setText(i18n("Low Power"));
				mSettingsWidget->plMedium->setFrameShape(QLabel::PopupPanel);
				break;
			case 2:
				(mBatType == 3)? mSettingsWidget->tlStatus->setText(i18n("Full Life"))
				: mSettingsWidget->tlStatus->setText(i18n("Full Power"));
				mSettingsWidget->plFull->setFrameShape(QLabel::PopupPanel);
				break;
		}
	}
	if (action == 6) {
		video += 2;
		if (video == 5) video = 2;
		else if (video > 4) video = 1;
		// TODO: Find out wich models do video out change automatically
		//if (mDriver->machineID() < 0xfcf8) mDriver->setVideo(video - 0x100);
		//if (video == 6) mDriver->setVideo(4); mDriver->setBackLight(1);
		mSettingsWidget->wsSettings->raiseWidget(1);
		mSettingsWidget->plLCD->setFrameShape(QLabel::NoFrame);
		mSettingsWidget->plCRT->setFrameShape(QLabel::NoFrame);
		mSettingsWidget->plLCDCRT->setFrameShape(QLabel::NoFrame);
		mSettingsWidget->plTV->setFrameShape(QLabel::NoFrame);
		//mSettingsWidget->plLCDTV->setFrameShape(QLabel::NoFrame);
		switch (video) {
			case 1:
				mSettingsWidget->tlStatus->setText("LCD");
				mSettingsWidget->plLCD->setFrameShape(QLabel::PopupPanel);
				break;
			case 2:
				mSettingsWidget->tlStatus->setText("CRT");
				mSettingsWidget->plCRT->setFrameShape(QLabel::PopupPanel);
				break;
			case 3:
				mSettingsWidget->tlStatus->setText("LCD/CRT");
				mSettingsWidget->plLCDCRT->setFrameShape(QLabel::PopupPanel);
				break;
			case 4:
				mSettingsWidget->tlStatus->setText("S-Video");
				mSettingsWidget->plTV->setFrameShape(QLabel::PopupPanel);
				break;
			//case 6:
			//	mSettingsWidget->tlStatus->setText("LCD/S-Video");
			//	mSettingsWidget->plLCDTV->setFrameShape(QLabel::PopupPanel);
			//	break;
		}
	}
	if (action == 13) {
		boot++;
		if (boot > mBootType) boot = 0;
		mDriver->setBootMethod(boot);
		mSettingsWidget->wsSettings->raiseWidget(2);
		mSettingsWidget->plFD->setFrameShape(QLabel::NoFrame);
		mSettingsWidget->plHD->setFrameShape(QLabel::NoFrame);
		mSettingsWidget->plCD->setFrameShape(QLabel::NoFrame);
		switch (mBootType) {
			case 1:
				if (!boot)
					mSettingsWidget->tlStatus->setText("FDD -> HDD");
				else
					mSettingsWidget->tlStatus->setText("HDD -> FDD");
				mSettingsWidget->plFD->setFrameShape(QLabel::PopupPanel);
				mSettingsWidget->plHD->setFrameShape(QLabel::PopupPanel);
				break;
			case 3:
				if (!boot)
					mSettingsWidget->tlStatus->setText("FDD -> Built-in HDD");
				else if (boot == 1)
					mSettingsWidget->tlStatus->setText("Built-in HDD -> FDD");
				else if (boot == 2)
					mSettingsWidget->tlStatus->setText("FDD -> Second HDD");
				else
					mSettingsWidget->tlStatus->setText("Second HDD -> FDD");
				mSettingsWidget->plFD->setFrameShape(QLabel::PopupPanel);
				mSettingsWidget->plHD->setFrameShape(QLabel::PopupPanel);
				break;
			case 5:
				if (!boot)
					mSettingsWidget->tlStatus->setText("FDD -> HDD -> CD-ROM");
				else if (boot == 1)
					mSettingsWidget->tlStatus->setText("HDD -> FDD -> CD-ROM");
				else if (boot == 2)
					mSettingsWidget->tlStatus->setText("FDD -> CD-ROM -> HDD");
				else if (boot == 3)
					mSettingsWidget->tlStatus->setText("HDD -> CD-ROM -> FDD");
				else if (boot == 4)
					mSettingsWidget->tlStatus->setText("CD-ROM -> FDD -> HDD");
				else
					mSettingsWidget->tlStatus->setText("CD-ROM -> HDD -> FDD");
				mSettingsWidget->plFD->setFrameShape(QLabel::PopupPanel);
				mSettingsWidget->plHD->setFrameShape(QLabel::PopupPanel);
				mSettingsWidget->plCD->setFrameShape(QLabel::PopupPanel);
				break;
		}
	}
	if (action == 1) {
		snd--;
		if (snd < 0) snd = 1;
		mute();
		mStatusWidget->wsStatus->raiseWidget(snd);
	}
	if ((action == 7) || (action == 8)) {
		(action == 7)? brightDown() : brightUp();
		if (bright <= 7 && bright >= 0)
			mStatusWidget->wsStatus->raiseWidget(bright + 4);
	}
	if (action == 10) {
		if ((mPad != -1) || (m_synShm != 0)) {
			mousepad--;
			if (mousepad < 0) mousepad = 1;
			mousePad();
			int mpad = ((mousepad == 0)? 2 : 3);
			mStatusWidget->wsStatus->raiseWidget(mpad);
		}
		else
			mStatusWidget->wsStatus->raiseWidget(2);
	}
	if (action == 11) {
		speakerVolume();
		if (!vol)
			mStatusWidget->wsStatus->raiseWidget(vol);
		else if (vol == 3)
			mStatusWidget->wsStatus->raiseWidget(vol - 2);
		else
			mStatusWidget->wsStatus->raiseWidget(vol + 13);
	}
	if (action == 12) {
		toggleFan();
		if (fan == -1)
			return;
		else if (fan == 1)
			mStatusWidget->wsStatus->raiseWidget(12);
		else if (fan == 0)
			mStatusWidget->wsStatus->raiseWidget(13);
	}
}

void KToshiba::brightUp()
{
    if (bright != mDriver->getBrightness())
        bright = mDriver->getBrightness();

    mDriver->setBrightness(++bright);
}

void KToshiba::brightDown()
{
    if (bright != mDriver->getBrightness())
        bright = mDriver->getBrightness();

    mDriver->setBrightness(--bright);
}

void KToshiba::doSuspendToDisk()
{
    // TODO: When suspending to Disk the interface gets closed,
    // I need to find a way when we're back from Suspend State
    QString helper = KStandardDirs::findExe("ktosh_helper");
    if (helper.isEmpty()) {
        KMessageBox::sorry(0, i18n("Could not Suspend To Disk because ktosh_helper cannot be found.\n"
						   "Please make sure that it is installed correctly."),
						   i18n("STD"));
        this->contextMenu()->setItemEnabled(4, FALSE);
        return;
    }

    static int res = KMessageBox::warningContinueCancel(this,
						i18n("Before continuing, be aware that ACPI Sleep States\n"
							 "are a work in progress and may or may not work on your computer.\n"
							 "Also make sure to unload problematic modules"), i18n("WARNING"));

    if (res == KMessageBox::Continue) {
        KProcess proc;
        proc << "ktosh_helper" << "--std";
        proc.start(KProcess::DontCare);
        proc.detach();
    }
}

void KToshiba::doSuspendToRAM()
{
    QString helper = KStandardDirs::findExe("ktosh_helper");
    if (helper.isEmpty()) {
        KMessageBox::sorry(0, i18n("Could not Suspend To RAM because ktosh_helper cannot be found.\n"
						   "Please make sure that it is installed correctly."),
						   i18n("STR"));
        this->contextMenu()->setItemEnabled(3, FALSE);
        return;
    }

    static int res = KMessageBox::warningContinueCancel(this,
						i18n("Before continuing, be aware that ACPI Sleep States\n"
							 "are a work in progress and may or may not work on your computer.\n"
							 "Also make sure to unload problematic modules"), i18n("WARNING"));

    if (res == KMessageBox::Continue) {
        KProcess proc;
        proc << "ktosh_helper" << "--str";
        proc.start(KProcess::DontCare);
        proc.detach();
    }
}

void KToshiba::lockScreen()
{
    if (mClient.isAttached())
        mClient.send("kdesktop", "KScreensaverIface", "lock()", "");
}

void KToshiba::toggleWireless()
{
    int swtch = mDriver->getWirelessSwitch();
    if (!swtch) {
        KMessageBox::sorry(0, i18n("In order to deactivate the wireless interface "
						   "the wireless antenna switch must be turned on"), i18n("Wireless"));
        return;
    } else
    if (swtch == 1)
        mDriver->setWirelessPower(wireless);

    QString w = ((wireless == 1)? i18n("up") : i18n("down"));
    KPassivePopup::message(i18n("KToshiba"),
						   i18n("Wireless interface %1").arg(w),
						   SmallIcon("messagebox_info", 20), this, i18n("Wireless"), 5000);
}

void KToshiba::mousePad()
{
    if (mPad == -1) {
        bool enabled = (m_synShm != 0);

        if (!enabled)
            return;

        m_synShm->touchpad_off = mousepad;
    } else
    if (mPad >= 0)
        mDriver->setPointingDevice(mousepad);
}

void KToshiba::setModel()
{
    QString mod = modelID(mDriver->machineID());

    this->contextMenu()->insertTitle( mod, 0, 0 );
}

void KToshiba::mute()
{
    DCOPRef kmixClient("kmix", "Mixer0");
    kmixClient.send("toggleMute", 0);
}

void KToshiba::mode()
{
    int temp = mDriver->procStatus();

    if (temp == CD_DVD)
        MODE = CD_DVD;
    else if (temp == DIGITAL)
        MODE = DIGITAL;
}

void KToshiba::doBluetooth()
{
    if (!mDriver->getBluetooth()) {
        this->contextMenu()->setItemEnabled(6, FALSE);
        kdDebug() << "KToshiba::doBluetooth(): "
				  << "No Bluetooth device found" << endl;
        return;
    } else
    if (!bluetooth || (btstart && !bluetooth)) {
        mDriver->setBluetooth();
        KPassivePopup::message(i18n("KToshiba"), i18n("Bluetooth device activated"),
							   SmallIcon("kdebluetooth", 20), this, i18n("Bluetooth"), 5000);
        this->contextMenu()->setItemEnabled(6, FALSE);
        bluetooth = 1;
    }
    else
        this->contextMenu()->setItemEnabled(6, TRUE);
}

void KToshiba::checkSystem()
{
    int bay;

    if (wstrig == false)
        mWirelessSwitch = mDriver->getWirelessSwitch();
    if (baytrig == false)
        bay = mDriver->getBayDevice(1);

    if (mWirelessSwitch != mOldWirelessSwitch) {
        if (mWirelessSwitch < 0)
            wstrig = true;
        else if (wstrig == false) {
            QString s = ((mWirelessSwitch == 1)? i18n("on") : i18n("off"));
            KPassivePopup::message(i18n("KToshiba"), i18n("Wireless antenna turned %1").arg(s),
							   SmallIcon("kwifimanager", 20), this, i18n("Wireless"), 4000);
        }
    }

    if (bay < 0)
        baytrig = true;
    else if (baytrig == false)
        checkSelectBay();

    if ((wstrig == true) && (baytrig == true)) {
        mSystemTimer->stop();
        disconnect( mSystemTimer );
        return;
    }

    mOldWirelessSwitch = mWirelessSwitch;
}

void KToshiba::speakerVolume()
{
    vol = mDriver->getSpeakerVolume();
    if (vol == -1)
        return;

    vol++;
    if (vol > 3) vol = 0;
    mDriver->setSpeakerVolume(vol);
}

void KToshiba::toggleFan()
{
    int res = mDriver->getFan();

    if (res < 0x00) {
        fan = -1;
        return;
    }
    if (res == 0x00) {
        fan = 1;
        mDriver->setFan(fan);
    } else {
        fan = 0;
        mDriver->setFan(fan);
    }
}

void KToshiba::checkSelectBay()
{
	mDriver->systemLocks(&sblock, 1);

	if (sblock == HCI_LOCKED) {
		if (removed == 0)
			return;
		if (bayRescan() < 0)
			return;
		removed = 0;
		return;
	} else
	if (sblock == HCI_UNLOCKED) {
		if (removed == 1)
			return;

		int device = mDriver->getBayDevice(1);
		if ((device == HCI_ATAPI) || (device == HCI_IDE)) {
			int res = KMessageBox::warningContinueCancel(0, i18n("Please umount all filesystems on the "
											   "SelectBay device, if any."), i18n("SelectBay"));
			if (res == KMessageBox::Continue) {
				if (bayUnregister() < 0) {
					KMessageBox::queuedMessageBox(0, KMessageBox::Error,
											  i18n("Unable to remove device in\n"
												   "the SelectBay. Please re-lock."), i18n("SelectBay"));
					return;
				}

				KMessageBox::queuedMessageBox(0, KMessageBox::Information,
											  i18n("Device in the SelectBay sucessfully removed."), i18n("SelectBay"));
				removed = 1;
			}
		}
	}
}

int KToshiba::bayUnregister()
{
    QString helper = KStandardDirs::findExe("ktosh_helper");
    if (helper.isEmpty()) {
        KMessageBox::sorry(0, i18n("Could not unregister device because ktosh_helper cannot be found.\n"
						   "Please make sure that it is installed correctly."),
						   i18n("KToshiba"));
        return -1;
    }

    KProcess proc;
    proc << helper << "--unregister";
    proc.start(KProcess::NotifyOnExit);
    int res = proc.exitStatus();
    proc.detach();
    if (res != 0)
        return -1;

    return 0;
}

int KToshiba::bayRescan()
{
    QString helper = KStandardDirs::findExe("ktosh_helper");
    if (helper.isEmpty()) {
        KMessageBox::sorry(0, i18n("Could not register device because ktosh_helper cannot be found.\n"
						   "Please make sure that it is installed correctly."),
						   i18n("KToshiba"));
        return -1;
    }

    KProcess proc;
    proc << helper << "--rescan";
    proc.start(KProcess::NotifyOnExit);
    int res = proc.exitStatus();
    proc.detach();
    if (res != 0)
        return -1;

    return 0;
}

void KToshiba::toogleBackLight()
{
    int bl = mDriver->getBackLight();

    if (bl == -1)
        return;
    else if (bl == 1)
        mDriver->setBackLight(0);
    else if (bl == 0)
        mDriver->setBackLight(1);
}

void KToshiba::setFreq(int state)
{
	mDriver->setSpeedStep(state);
}


#include "ktoshiba.moc"
