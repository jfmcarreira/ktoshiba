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
	  mSysEvTimer( new QTimer(this) ),
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
		mWire = true;
		crytrig = false;
		lowtrig = false;
		battrig = false;
		snd = 1;
		popup = 0;
		mousepad = 0;
		// FIXME: This is not correct since it always return 2
		// and we need to relay on saved info in config file
		// so this needs to be moved out of here.
		battery = mDriver->getBatterySaveMode();
		oldbattery = -1;
		video = mDriver->getVideo();
		oldvideo = 2;
		bright = mDriver->getBrightness();
		oldbright = bright;
		mDriver->batteryStatus(&time, &perc);
		if (perc == 100) battrig = true;
		else battrig = false;
		mWirelessSwitch = mDriver->getWirelessSwitch();
		mOldWirelessSwitch = mWirelessSwitch;
		bluetooth = 0;
		wireless = 1;
	}

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

	if (!mClient.attach())
	{
		kdDebug() << "KToshiba: cannot attach to DCOP server." << endl;
	}

	KConfig mConfig(CONFIG_FILE);
	mConfig.setGroup("KToshiba");
	bool started = mConfig.readBoolEntry("AlreadyStarted");
	if (started != true) {
		mConfig.writeEntry("AlreadyStarted", true);
	}
	mConfig.sync();

	noBatteryIcon = QString("laptop_nobattery");
	noChargeIcon = QString("laptop_nocharge");
	chargeIcon = QString("laptop_charge");

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
	this->contextMenu()->insertItem( i18n("&About KToshiba"), this,
									 SLOT( displayAbout() ), 0, 8, 8 );

	loadConfiguration(&mConfig);

	connect( mPowTimer, SIGNAL( timeout() ), SLOT( powerStatus() ) );
	mPowTimer->start(mBatStatus * 1000);
	connect( mSysEvTimer, SIGNAL( timeout() ), SLOT( checkEvent() ) );
	mSysEvTimer->start(100);		// Check system events every 1/10 seconds
	connect( mModeTimer, SIGNAL( timeout() ), SLOT( mode() ) );
	mModeTimer->start(500);		// Check proc entry every 1/2 seconds
	connect( mSystemTimer, SIGNAL( timeout() ), SLOT( checkSystem() ) );
	mSystemTimer->start(1000);

	displayPixmap();
	setModel();
	if (btstart)
		doBluetooth();
}

KToshiba::~KToshiba()
{
	if (mInterfaceAvailable) {
		kdDebug() << "KToshiba: Closing interface." << endl;
		mDriver->closeInterface();
		delete mDriver; mDriver = 0L;
	}
	if (mClient.isAttached())
	{
		mClient.detach();
	}

	delete m_synShm;
	m_synShm = NULL;

	// Stop timers 
	mPowTimer->stop();
	mSysEvTimer->stop();
	mModeTimer->stop();

	delete instance;
	delete mStatusWidget;
	delete mSettingsWidget;
	delete mAboutWidget;
}

void KToshiba::doConfig()
{
	KProcess proc;
	proc << KStandardDirs::findExe("kcmshell");
	proc << "ktoshiba";
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
	mBatSave = k->readNumEntry("Battery_Save_Mode");
	mAudioPlayer = k->readNumEntry("Audio_Player", 1);
	btstart = k->readBoolEntry("Bluetooth_Startup", true);
}

void KToshiba::displayAbout()
{
	mAboutWidget->show();
}

void KToshiba::powerStatus()
{
	mDriver->batteryStatus(&time, &perc);
	pow = mDriver->acPowerStatus();
	bright = mDriver->getBrightness();
	KConfig mConfig(CONFIG_FILE);
	mConfig.setGroup("KToshiba");
	loadConfiguration(&mConfig);

	if (mFullBat && perc == 100 && !battrig) {
		KMessageBox::queuedMessageBox(0, KMessageBox::Information,
									  i18n("Your battery is now fully charged."), i18n("Laptop Battery"));
		battrig = true;
	}

	if (SCI_MINUTE(time) == mLowBat && SCI_HOUR(time) == 0 && pow == 0 && !lowtrig) {
		KPassivePopup::message(i18n("Warning"), i18n("The battery state has changed to low"),
							   SmallIcon("messagebox_warning", 20), this, i18n("Warning"), 15000);
		lowtrig = true;
	} else
	if (SCI_MINUTE(time) == mCryBat && SCI_HOUR(time) == 0 && pow == 0 && !crytrig) {
		KPassivePopup::message(i18n("Warning"), i18n("The battery state has changed to critical"),
							   SmallIcon("messagebox_warning", 20), this, i18n("Warning"), 15000);
		crytrig = true;
	} else
	if (SCI_MINUTE(time) == 0 && SCI_HOUR(time) == 0 && pow == 0) {
		KPassivePopup::message(i18n("Warning"), i18n("I'm Gone..."),
							   SmallIcon("messagebox_warning", 20), this, i18n("Warning"), 15000);
	}

	if (SCI_MINUTE(time) > lowtrig && pow == 1) {
		if (battrig && (perc < 100))
			battrig = false;
		if (lowtrig)
			lowtrig = false;
		if (crytrig)
			crytrig = false;
	}

	if (mBatSave != mOldBatSave || pow != oldpow) {
		switch (mBatSave) {
			// TODO: Add support for more battery save options for
			// older models depending on type
			case 0:			// USER SETTINGS
				bsmUserSettings(&mConfig);
				break;
			case 1:			// LOW POWER
				if (!pow)
					bright = 0;	// Semi-Bright
				else if (pow)
					bright = 3;	// Bright
				break;
			case 2:			// FULL POWER
				if (!pow)
					bright = 3;	// Bright
				else if (pow)
					bright = 7;	// Super-Bright
				break;
		}
		mDriver->setBrightness(bright);
	}

	if (mOldBatStatus != mBatStatus) {
		mPowTimer->stop();
		mPowTimer->start(mBatStatus * 1000);
	}

	changed = oldpow != pow||oldperc != perc||oldtime != time;
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
	if (lcd == 0) bright = 7; // Super-Bright
	else if (lcd == 1) bright = 3; // Bright
	else if (lcd == 2) bright = 0; // Semi-Bright
	mDriver->setCoolingMethod(cooling);
	mDriver->setDisplayAutoOff(display);
	mDriver->setHDDAutoOff(hdd);
}

void KToshiba::displayPixmap()
{
	int new_code = 0;

	if (!mInterfaceAvailable)
		new_code = 1;
	else if (!mDriver->acPowerStatus())
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
		else if (!mDriver->acPowerStatus())
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
	if (c == 100) {
		c = count;
	} else
	if (perc != 100 && c == count)
		c = count-1;


	if (c) {
		uint ui;
		QRgb blue = qRgb(0x00,0x00,0xff);

		if (image.depth() <= 8) {
			ui = image.numColors();		// this fix thanks to Sven Krumpke
			image.setNumColors(ui+1);
			image.setColor(ui, blue);
		} else {
			ui = 0xff000000|blue;
		}

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
	if (!mInterfaceAvailable) {
		tmp = i18n("No interface available");
	} else
	if (mDriver->acPowerStatus()) {
		mDriver->batteryStatus(&time, &perc);
		if (perc == 100) {
			tmp = i18n("Plugged in - fully charged");;
		} else {
			if (perc >= 0) {
				QString num3;
				num3.setNum(SCI_MINUTE(time));
				num3 = num3.rightJustify(2, '0');
				tmp = i18n("Plugged in - %1% charged (%2:%3 time left)")
					.arg(perc).arg(SCI_HOUR(time)).arg(num3);
			} else
			if (perc == -1) {
				tmp = i18n("Plugged in - no battery");
			} else {
				tmp = i18n("Plugged in - %1% charged").arg(perc);
			}
		}
	} else {
		mDriver->batteryStatus(&time, &perc);
		if (perc >= 0) {
			QString num3;
			num3.setNum(SCI_MINUTE(time));
			num3 = num3.rightJustify(2, '0');
			tmp = i18n("Running on batteries - %1% charged (%2:%3 time left)")
					.arg(perc).arg(SCI_HOUR(time)).arg(num3);
		} else {
			tmp = i18n("Running on batteries  - %1% charged").arg(perc);
		}
	}
	QToolTip::add(this, tmp);
}

void KToshiba::checkEvent()
{
	KProcess proc;
	QByteArray data, replyData;
	QCString replyType;
	KConfig mConfig(CONFIG_FILE);
	mConfig.setGroup("KToshiba");

	int tmp = 0;
	int key = mDriver->systemEvent();

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
			break;
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
			break;
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
			break;
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
			break;
	}
	performFnAction(tmp, key);
}

void KToshiba::performFnAction(int action, int key)
{
	switch(action) {
		case 0: // Disabled (Do Nothing)
			break;
		case 2: // LockScreen
			lockScreen();
			break;
		case 4: // Suspend To RAM (STR)
			doSuspendToRAM();
			break;
		case 5: // Suspend To Disk (STD)
			doSuspendToDisk();
			break;
		case 9: // Wireless On/Off
			wireless--;
			if (wireless < 0) wireless = 1;
			toggleWireless();
			break;
		case 1: // Mute/Unmute
		case 7: // Brightness Down
		case 8: // Brightness Up
		case 10: // Enable/Disable MousePad
			if (popup == 0) {
				QRect r = QApplication::desktop()->geometry();
				mStatusWidget->move(r.center() - 
					QPoint(mStatusWidget->width()/2, mStatusWidget->height()/2));
				mStatusWidget->show();
				popup = key & 0x17f;
			}
			if ((key & 0x17f) == popup) {
				updateWidgetStatus(action);
			}
			break;
		case 3: // Battery Save Mode
		case 6: // Toggle Video
			if (popup == 0) {
				QRect r = QApplication::desktop()->geometry();
				mSettingsWidget->move(r.center() - 
					QPoint(mSettingsWidget->width()/2, mSettingsWidget->height()/2));
				mSettingsWidget->show();
				popup = key & 0x17f;
			}
			if ((key & 0x17f) == popup)
				updateWidgetStatus(action);
			break;
	}
}

void KToshiba::updateWidgetStatus(int action)
{
	KConfig mConfig(CONFIG_FILE);
	mConfig.setGroup("KToshiba");

	if (action == 3) {
		battery--;
		if (battery < 0) battery = 2;
		mDriver->setBatterySaveMode(battery);
		mConfig.writeEntry("Battery_Save_Mode", battery);
		mSettingsWidget->wsSettings->raiseWidget(0);
		mSettingsWidget->plUser->setFrameShape(QLabel::NoFrame);
		mSettingsWidget->plMedium->setFrameShape(QLabel::NoFrame);
		mSettingsWidget->plFull->setFrameShape(QLabel::NoFrame);
		switch (battery) {
			case 0:
				mSettingsWidget->tlStatus->setText(i18n("User Settings"));
				mSettingsWidget->plUser->setFrameShape(QLabel::PopupPanel);
				break;
			case 1:
				mSettingsWidget->tlStatus->setText(i18n("Low Power"));
				mSettingsWidget->plMedium->setFrameShape(QLabel::PopupPanel);
				break;
			case 2:
				mSettingsWidget->tlStatus->setText(i18n("Full Power"));
				mSettingsWidget->plFull->setFrameShape(QLabel::PopupPanel);
				break;
			case 3:
				mSettingsWidget->tlStatus->setText(i18n("Full Life"));
				mSettingsWidget->plFull->setFrameShape(QLabel::PopupPanel);
				break;
		}
	}
	if (action == 6) {
		video += 2;
		if (video == 0x105) video = 0x102;
		else if (video > 0x104) video = 0x101;
		//if (mDriver->machineID() < 0xfcf8) mDriver->setVideo(video - 0x100);
		mSettingsWidget->wsSettings->raiseWidget(1);
		mSettingsWidget->plLCD->setFrameShape(QLabel::NoFrame);
		mSettingsWidget->plCRT->setFrameShape(QLabel::NoFrame);
		mSettingsWidget->plLCDCRT->setFrameShape(QLabel::NoFrame);
		mSettingsWidget->plTV->setFrameShape(QLabel::NoFrame);
		switch (video) {
			case 0x101:
				mSettingsWidget->tlStatus->setText("LCD");
				mSettingsWidget->plLCD->setFrameShape(QLabel::PopupPanel);
				break;
			case 0x102:
				mSettingsWidget->tlStatus->setText("CRT");
				mSettingsWidget->plCRT->setFrameShape(QLabel::PopupPanel);
				break;
			case 0x103:
				mSettingsWidget->tlStatus->setText("LCD/CRT");
				mSettingsWidget->plLCDCRT->setFrameShape(QLabel::PopupPanel);
				break;
			case 0x104:
				mSettingsWidget->tlStatus->setText("S-Video");
				mSettingsWidget->plTV->setFrameShape(QLabel::PopupPanel);
				break;
		}
	}
	if (action == 1) {
		snd--;
		if (snd < 0) snd = 1;
		mute();
		mStatusWidget->wsStatus->raiseWidget(snd);
	}
	if (action == 10) {
		mousepad--;
		if (mousepad < 0) mousepad = 1;
		mousePad();
		if (mousepad == 0)
			mStatusWidget->wsStatus->raiseWidget(2);
		else if (mousepad == 1)
			mStatusWidget->wsStatus->raiseWidget(3);
	}
	if ((action == 7) || (action == 8)) {
		(action == 7)? brightDown() : brightUp();
		if (bright <= 7 && bright >= 0)
			mStatusWidget->wsStatus->raiseWidget(bright + 4);
	}

	oldmousepad = mousepad;
	oldbattery = battery;
	oldbright = bright;
	oldvideo = video;
	oldsnd = snd;
}

void KToshiba::brightUp()
{
	if (bright == mDriver->getBrightness())
		mDriver->setBrightness(++bright);
	else {
		bright = mDriver->getBrightness();
		mDriver->setBrightness(++bright);
	}
}

void KToshiba::brightDown()
{
	if (bright == mDriver->getBrightness())
		mDriver->setBrightness(--bright);
	else {
		bright = mDriver->getBrightness();
		mDriver->setBrightness(--bright);
	}
}

void KToshiba::doSuspendToDisk()
{
	QString helper = KStandardDirs::findExe("ktosh_helper");
	if (helper.isEmpty()) {
		KMessageBox::sorry(0, i18n("Could not Suspend To Disk because ktosh_helper cannot be found.\n"
						   "Please make sure that it is installed correctly."),
						   i18n("KToshiba"));
		this->contextMenu()->setItemEnabled(4, FALSE);
		return;
	}

	static int res = KMessageBox::warningContinueCancel(this,
						i18n("Before continuing, be aware that ACPI Sleep States\n"
							 "are a work in progress and may or may not work on your computer.\n"
							 "Also make sure to unload problematic modules"), i18n("WARNING"));

	KProcess proc;
	if (res == KMessageBox::Continue) {
		proc << "ktosh_helper" << "--std";
		proc.start(KProcess::DontCare);
		proc.detach();
		return;
	}
}

void KToshiba::doSuspendToRAM()
{
	QString helper = KStandardDirs::findExe("ktosh_helper");
	if (helper.isEmpty()) {
		KMessageBox::sorry(0, i18n("Could not Suspend To RAM because ktosh_helper cannot be found.\n"
						   "Please make sure that it is installed correctly."),
						   i18n("KToshiba"));
		this->contextMenu()->setItemEnabled(3, FALSE);
		return;
	}

	static int res = KMessageBox::warningContinueCancel(this,
						i18n("Before continuing, be aware that ACPI Sleep States\n"
							 "are a work in progress and may or may not work on your computer.\n"
							 "Also make sure to unload problematic modules"), i18n("WARNING"));

	KProcess proc;
	if (res == KMessageBox::Continue) {
		proc << "ktosh_helper" << "--str";
		proc.start(KProcess::DontCare);
		proc.detach();
		return;
	}
}

void KToshiba::lockScreen()
{
	if (mClient.isAttached())
		mClient.send("kdesktop", "KScreensaverIface", "lock()", "");
}

void KToshiba::toggleWireless()
{
	QString helper = KStandardDirs::findExe("ktosh_helper");
	if (helper.isEmpty()) {
		KMessageBox::sorry(0, i18n("Could not change wireless status because ktosh_helper cannot be found.\n"
						   "Please make sure that it is installed correctly."),
						   i18n("KToshiba"));
		return;
	}

	KProcess proc;
	if (wireless) {
		proc << "ktosh_helper" << "--enable";
		proc.start(KProcess::DontCare);
		proc.detach();
		KPassivePopup::message(i18n("KToshiba"), 
							   i18n("Wireless interface up"),
							   SmallIcon("messagebox_info", 20), this, i18n("Info"), 5000);
		return;
	} else
	if (!wireless) {
		proc << "ktosh_helper" << "--disable";
		proc.start(KProcess::DontCare);
		proc.detach();
		KPassivePopup::message(i18n("KToshiba"), 
							   i18n("Wireless interface down"),
							   SmallIcon("messagebox_info", 20), this, i18n("Info"), 5000);
		return;
	}
}

void KToshiba::mousePad()
{
	bool enabled = (m_synShm != 0);

	if (!enabled)
		return;

	m_synShm->touchpad_off = mousepad;
}

void KToshiba::setModel()
{
	QString mod;

	int id = mDriver->machineID();

	switch (id) {
		case 0xfc00:
			mod = "Satellite 2140CDS/2180CDT/2675DVD";
			break;
		case 0xfc01:
			mod = "Satellite 2710xDVD";
			break;
		case 0xfc02:
			mod = "Satellite Pro 4270CDT/4280CDT/4300CDT/4340CDT";
			break;
		case 0xfc04:
			mod = "Portege 3410CT/3440CT";
			break;
		case 0xfc08:
			mod = "Satellite 2100CDS/CDT 1550CDS";
			break;
		case 0xfc09:
			mod = "Satellite 2610CDT, 2650XDVD";
			break;
		case 0xfc0a:
			mod = "Portage 7140";
			break;
		case 0xfc0b:
			mod = "Satellite Pro 4200";
			break;
		case 0xfc0c:
			mod = "Tecra 8100x";
			break;
		case 0xfc0f:
			mod = "Satellite 2060CDS/CDT";
			break;
		case 0xfc10:
			mod = "Satellite 2550/2590";
			break;
		case 0xfc11:
			mod = "Portage 3110CT";
			break;
		case 0xfc12:
			mod = "Portage 3300CT";
			break;
		case 0xfc13:
			mod = "Portage 7020CT";
			break;
		case 0xfc15:
			mod = "Satellite 4030/4030X/4050/4060/4070/4080/4090/4100X CDS/CDT";
			break;
		case 0xfc17:
			mod = "Satellite 2520/2540 CDS/CDT";
			break;
		case 0xfc18:
			mod = "Satellite 4000/4010 XCDT";
			break;
		case 0xfc19:
			mod = "Satellite 4000/4010/4020 CDS/CDT";
			break;
		case 0xfc1a:
			mod = "Tecra 8000x";
			break;
		case 0xfc1c:
			mod = "Satellite 2510CDS/CDT";
			break;
		case 0xfc1d:
			mod = "Portage 3020x";
			break;
		case 0xfc1f:
			mod = "Portage 7000CT/7010CT";
			break;
		case 0xfc39:
			mod = "T2200SX";
			break;
		case 0xfc40:
			mod = "T4500C";
			break;
		case 0xfc41:
			mod = "T4500";
			break;
		case 0xfc45:
			mod = "T4400SX/SXC";
			break;
		case 0xfc51:
			mod = "Satellite 2210CDT/2770XDVD";
			break;
		case 0xfc52:
			mod = "Satellite 2775DVD/Dynabook Satellite DB60P/4DA";
			break;
		case 0xfc53:
			mod = "Portage 7200CT/7220CT/Satellite 4000CDT";
			break;
		case 0xfc54:
			mod = "Satellite 2800DVD";
			break;
		case 0xfc56:
			mod = "Portage 3480CT";
			break;
		case 0xfc57:
			mod = "Satellite 2250CDT";
			break;
		case 0xfc5a:
			mod = "Satellite Pro 4600";
			break;
		case 0xfc5d:
			mod = "Satellite 2805";
			break;
		case 0xfc5f:
			mod = "T3300SL";
			break;
		case 0xfc61:
			mod = "Tecra 8200";
			break;
		case 0xfc64:
			mod = "Satellite 1800";
			break;
		case 0xfc69:
			mod = "T1900C";
			break;
		case 0xfc6a:
			mod = "T1900";
			break;
		case 0xfc6d:
			mod = "T1850C";
			break;
		case 0xfc6e:
			mod = "T1850";
			break;
		case 0xfc6f:
			mod = "T1800";
			break;
		case 0xfc72:
			mod = "Satellite 1800";
			break;
		case 0xfc7e:
			mod = "T4600C";
			break;
		case 0xfc7f:
			mod = "T4600";
			break;
		case 0xfc8a:
			mod = "T6600C";
			break;
		case 0xfc91:
			mod = "T2400CT";
			break;
		case 0xfc97:
			mod = "T4800CT";
			break;
		case 0xfc99:
			mod = "T4700CS";
			break;
		case 0xfc9b:
			mod = "T4700CT";
			break;
		case 0xfc9d:
			mod = "T1950";
			break;
		case 0xfc9e:
			mod = "T3400/T3400CT";
			break;
		case 0xfcb2:
			mod = "Libretto 30CT";
			break;
		case 0xfcba:
			mod = "T2150";
			break;
		case 0xfcbe:
			mod = "T4850CT";
			break;
		case 0xfcc0:
			mod = "Satellite Pro 420x";
			break;
		case 0xfcc1:
			mod = "Satellite 100x";
			break;
		case 0xfcc3:
			mod = "Tecra 710x/720x";
			break;
		case 0xfcc6:
			mod = "Satellite Pro 410x";
			break;
		case 0xfcca:
			mod = "Satellite Pro 400x";
			break;
		case 0xfccb:
			mod = "Portage 610CT";
			break;
		case 0xfccc:
			mod = "Tecra 700x";
			break;
		case 0xfccf:
			mod = "T4900CT";
			break;
		case 0xfcd0:
			mod = "Satellite 300x";
			break;
		case 0xfcd1:
			mod = "Tecra 750CDT";
			break;
		case 0xfcd3:
			mod = "Tecra 730XCDT";
			break;
		case 0xfcd4:
			mod = "Tecra 510x";
			break;
		case 0xfcd5:
			mod = "Satellite 200x";
			break;
		case 0xfcd7:
			mod = "Satellite Pro 430x";
			break;
		case 0xfcd8:
			mod = "Tecra 740x";
			break;
		case 0xfcd9:
			mod = "Portage 660CDT";
			break;
		case 0xfcda:
			mod = "Tecra 730CDT";
			break;
		case 0xfcdb:
			mod = "Portage 620CT";
			break;
		case 0xfcdc:
			mod = "Portage 650CT";
			break;
		case 0xfcdd:
			mod = "Satellite 110x";
			break;
		case 0xfcdf:
			mod = "Tecra 500x";
			break;
		case 0xfce0:
			mod = "Tecra 780DVD";
			break;
		case 0xfce2:
			mod = "Satellite 300x";
			break;
		case 0xfce3:
			mod = "Satellite 310x";
			break;
		case 0xfce4:
			mod = "Satellite Pro 490x";
			break;
		case 0xfce5:
			mod = "Libretto 100CT";
			break;
		case 0xfce6:
			mod = "Libretto 70CT";
			break;
		case 0xfce7:
			mod = "Tecra 540x/550x";
			break;
		case 0xfce8:
			mod = "Satellite Pro 470x/480x";
			break;
		case 0xfce9:
			mod = "Tecra 750DVD";
			break;
		case 0xfcea:
			mod = "Libretto 60";
			break;
		case 0xfceb:
			mod = "Libretto 50CT";
			break;
		case 0xfcec:
			mod = "Satellite 320x/330x/Satellite 2500CDS";
			break;
		case 0xfced:
			mod = "Tecra 520x/530x";
			break;
		case 0xfcef:
			mod = "Satellite 220x, Satellite Pro 440x/460x";
			break;
		case 0xfcf7:
			mod = "Satellite Pro A10";
			break;
		case 0xfcf8:
			mod = "Satellite A25";
			break;
		case 0xfcff:
			mod = "Tecra M2";
			break;
		case -1:
			KMessageBox::queuedMessageBox(0, KMessageBox::Information,
							i18n("Could not get model id.\nWeird"));
			break;
		default:
			mod = "UNKNOWN";
			KMessageBox::messageBox(0, KMessageBox::Information,
									i18n("Please send the model name and this id %1 to:\n"
									     "neftali@utep.edu").arg(id), i18n("Model Name"));
	}
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
	int bt = mDriver->getBluetooth();

	if (!bt) {
		this->contextMenu()->setItemEnabled(6, FALSE);
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
	// TODO: Add SelectBay stuff here
	mWirelessSwitch = mDriver->getWirelessSwitch();

	if (mWirelessSwitch != mOldWirelessSwitch) {
		if (mWirelessSwitch == 1)
			KPassivePopup::message(i18n("KToshiba"), i18n("Wireless antenna turned on"),
							   SmallIcon("kwifimanager", 20), this, i18n("Wireless"), 4000);
		else if (mWirelessSwitch == 0)
			KPassivePopup::message(i18n("KToshiba"), i18n("Wireless antenna turned off"),
							   SmallIcon("kwifimanager", 20), this, i18n("Wireless"), 4000);
	}

	mOldWirelessSwitch = mWirelessSwitch;
}


#include "ktoshiba.moc"
