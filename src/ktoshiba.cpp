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
#include "ktoshibasmminterface.h"
#include "ktoshibaprocinterface.h"
#include "fnactions.h"
#include "modelid.h"

#include <qpixmap.h>
#include <qimage.h>
#include <qtooltip.h>
#include <qtimer.h>
#include <qwidget.h>
#include <qwidgetstack.h>
#include <qapplication.h>

#include <kaboutapplication.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kpassivepopup.h>
#include <kstandarddirs.h>

#include "statuswidget.h"

#ifdef ENABLE_POWERSAVE
#include <powersave_dbus.h>
#endif

#define CONFIG_FILE "ktoshibarc"

KToshiba::KToshiba()
    : KSystemTray( 0, "KToshiba" ),
      mDriver( 0 ),
      mProc( 0 ),
      mFn( 0 ),
      mPowTimer( new QTimer(this) ),
      mHotKeysTimer( new QTimer(this) ),
      mModeTimer( new QTimer(this) ),
      mSystemTimer( new QTimer(this) ),
      mOmnibookTimer( new QTimer(this) )
{
    mDriver = new KToshibaSMMInterface(this);
    mFn = new FnActions(this);
    mProc = new KToshibaProcInterface(this);
    mAboutWidget = new KAboutApplication(this, "About Widget", false);
    instance = new KInstance("ktoshiba");

    // check whether toshiba module is loaded
    mInterfaceAvailable = mFn->m_SMM;
    if (mInterfaceAvailable) {
        mDriver->batteryStatus(&time, &perc);
        mAC = mDriver->acPowerStatus();
        mBatType = mDriver->getBatterySaveModeType();
        mHT = mDriver->getHyperThreading();
        mSS = mDriver->getSpeedStep();
        mWirelessSwitch = 1;
        mBatSave = 2;
        wstrig = false;
        baytrig = false;
        bluetooth = 0;
        sblock = HCI_LOCKED;
        removed = 0;
        svideo = 0;
        MODE = DIGITAL;
        proc = false;
        mOmnibook = false;
        if (perc == -1) {
            mProc->acpiBatteryStatus(&time, &perc);
            proc = true;
        }
        // Default to mode 2 if we got a failure
        if (mBatType == -1)
            mBatType = 2;
        mFn->m_BatType = mBatType;
    }
    if (!mInterfaceAvailable) {
        kdDebug() << "KToshiba: Checking for omnibook module..." << endl;
        mOmnibook = mProc->checkOmnibook();
        if (!mOmnibook) {
            kdError() << "KToshiba: Could not found a Toshiba model. "
                      << "Please check that the toshiba or omnibook "
                      << "module loads without failures" << endl;
            exit(-1);
        }
        kdDebug() << "KToshiba: Found a Toshiba model with Phoenix BIOS." << endl;
        mProc->omnibookBatteryStatus(&time, &perc);
        mAC = mProc->omnibookAC();
        mFn->m_Video = mProc->omnibookGetVideo();
        mFn->m_Bright = mProc->omnibookGetBrightness();
        mFn->m_Omnibook = true;
        mBatSave = 2;
        mBatType = 3;
        proc = true;
    }

    battrig = (perc == 100)? true : false;
    crytrig = false;
    lowtrig = false;


    if (!mClient.attach())
        kdDebug() << "KToshiba: Cannot attach to DCOP server." << endl;

    if (checkConfiguration()) {
        KConfig mConfig(CONFIG_FILE);
        loadConfiguration(&mConfig);
    } else
        createConfiguration();

    doMenu();

    connect( mPowTimer, SIGNAL( timeout() ), this, SLOT( checkPowerStatus() ) );
    mPowTimer->start(mBatStatus * 1000);
    if (mInterfaceAvailable) {
        connect( mHotKeysTimer, SIGNAL( timeout() ), this, SLOT( checkHotKeys() ) );
        mHotKeysTimer->start(100);		// Check hotkeys every 1/10 seconds
        connect( mModeTimer, SIGNAL( timeout() ), this, SLOT( checkMode() ) );
        mModeTimer->start(500);		// Check proc entry every 1/2 seconds
        connect( mSystemTimer, SIGNAL( timeout() ), this, SLOT( checkSystem() ) );
        mSystemTimer->start(500);		// Check system events every 1/2 seconds
        connect( mFn, SIGNAL( stdActivated() ), this, SLOT( shutdownEvent() ) );
    } else
    if (mOmnibook) {
        connect( mOmnibookTimer, SIGNAL( timeout() ), this, SLOT( checkOmnibook() ) );
        mOmnibookTimer->start(100);
    }

    displayPixmap();
    if (mInterfaceAvailable && btstart)
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

    delete instance; instance = NULL;
    delete mAboutWidget; mAboutWidget = NULL;
    delete mProc; mProc = NULL;
    delete mFn; mFn = NULL;
}

void KToshiba::doMenu()
{
    this->contextMenu()->insertItem( SmallIcon("configure"), i18n("&Configure KToshiba..."), this,
                                     SLOT( doConfig() ), 0, 1, 1 );
    this->contextMenu()->insertSeparator( 2 );
    this->contextMenu()->insertItem( SmallIcon("memory"), i18n("Suspend To &RAM"), this,
                                     SLOT( doSuspendToRAM() ), 0, 3, 3 );
    this->contextMenu()->insertItem( SmallIcon("hdd_unmount"), i18n("Suspend To &Disk"), this,
                                     SLOT( doSuspendToDisk() ), 0, 4, 4 );
#ifdef ENABLE_POWERSAVE
    static int res;
    res = dbusSendSimpleMessage(REQUEST_MESSAGE, "AllowedSuspendToRam");
    if (res == REPLY_DISABLED)
        this->contextMenu()->setItemEnabled( 3, FALSE );
    res = dbusSendSimpleMessage(REQUEST_MESSAGE, "AllowedSuspendToDisk");
    if (res == REPLY_DISABLED)
        this->contextMenu()->setItemEnabled( 4, FALSE );
#else
    if (::access("/proc/acpi/sleep", F_OK) == -1) {
        this->contextMenu()->setItemEnabled( 3, FALSE );
        this->contextMenu()->setItemEnabled( 4, FALSE );
    }
#endif
    this->contextMenu()->insertSeparator( 5 );
    if (mInterfaceAvailable) {
        this->contextMenu()->insertItem( SmallIcon("kdebluetooth"), i18n("Enable &Bluetooth"), this,
                                         SLOT( doBluetooth() ), 0, 6, 6 );
        this->contextMenu()->insertSeparator( 7 );
        mHyper = new QPopupMenu( this, i18n("HyperThreading") );
        mHyper->insertItem( SmallIcon("ht_disabled"), i18n("Disabled"), 0 );
        mHyper->insertItem( SmallIcon("ht_pm"), i18n("Enabled - PM aware"), 1 );
        mHyper->insertItem( SmallIcon("ht_no_pm"), i18n("Enabled - No PM aware"), 2 );
        this->contextMenu()->insertItem( SmallIcon("kcmprocessor"), i18n("Hyper-Threading"), mHyper, 8, 8 );
        if (mHT < 0) this->contextMenu()->setItemEnabled( 8, FALSE );
        else if (mHT >= 0)
            connect( mHyper, SIGNAL( activated(int) ), this, SLOT( doSetHyper(int) ) );
        this->contextMenu()->insertSeparator( 9 );
        mSpeed = new QPopupMenu( this, i18n("SpeedStep") );
        mSpeed->insertItem( SmallIcon("cpu_dynamic"), i18n("Dynamic"), 0 );
        mSpeed->insertItem( SmallIcon("cpu_high"), i18n("Always High"), 1 );
        mSpeed->insertItem( SmallIcon("cpu_low"), i18n("Always Low"), 2 );
        this->contextMenu()->insertItem( SmallIcon("kcmprocessor"), i18n("CPU Frequency"), mSpeed, 10, 10 );
        if (mSS < 0) this->contextMenu()->setItemEnabled( 10, FALSE );
        else if (mSS >= 0)
            connect( mSpeed, SIGNAL( activated(int) ), this, SLOT( doSetFreq(int) ) );
    } else
    if (mOmnibook) {
        mOneTouch = new QPopupMenu( this, i18n("OneTouch") );
        mOneTouch->insertItem( SmallIcon(""), i18n("Disabled"), 0 );
        mOneTouch->insertItem( SmallIcon(""), i18n("Enabled"), 1 );
        this->contextMenu()->insertItem( SmallIcon(""), i18n("OneTouch Buttons"), mOneTouch, 6, 6 );
        connect( mOneTouch, SIGNAL( activated(int) ), this, SLOT( doSetOneTouch(int) ) );
        this->contextMenu()->insertSeparator( 7 );
        mOmniFan = new QPopupMenu( this, i18n("Fan") );
        mOmniFan->insertItem( SmallIcon(""), i18n("Disabled"), 0 );
        mOmniFan->insertItem( SmallIcon(""), i18n("Enabled"), 1 );
        this->contextMenu()->insertItem( SmallIcon(""), i18n("System Fan"), mOmniFan, 8, 8 );
        connect( mOmniFan, SIGNAL( activated(int) ), this, SLOT( doSetOmnibookFan(int) ) );
    }
    this->contextMenu()->insertSeparator( 11 );
    this->contextMenu()->insertItem( SmallIcon("ktoshiba"), i18n("&About KToshiba"), this,
                                     SLOT( displayAbout() ), 0, 12, 12 );
    if (mInterfaceAvailable)
        this->contextMenu()->insertTitle( modelID( mDriver->machineID() ), 0, 0 );
    else if (mOmnibook)
        this->contextMenu()->insertTitle( mProc->model, 0, 0 );
}

void KToshiba::doConfig()
{
    KProcess proc;
    proc << KStandardDirs::findExe("kcmshell");
    proc << "ktoshibam";
    proc.start(KProcess::DontCare);
    proc.detach();
}

void KToshiba::doSuspendToRAM()
{
    mFn->performFnAction(4, 0);
}

void KToshiba::doSuspendToDisk()
{
    mFn->performFnAction(5, 0);
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
        mDriver->setBluetoothPower(1);
        KPassivePopup::message(i18n("KToshiba"), i18n("Bluetooth device activated"),
				SmallIcon("kdebluetooth", 20), this, i18n("Bluetooth"), 5000);
        this->contextMenu()->setItemEnabled(6, FALSE);
        bluetooth = 1;
    }
    else
        this->contextMenu()->setItemEnabled(6, TRUE);
}

void KToshiba::doSetFreq(int freq)
{
    mDriver->setSpeedStep(freq);
}

void KToshiba::doSetHyper(int state)
{
    mDriver->setHyperThreading(state);
}

bool KToshiba::checkConfiguration()
{
    KStandardDirs kstd;
    QString config = kstd.findResource("config", "ktoshibarc");

    if (config.isEmpty()) {
        kdDebug() << "KToshiba: Configuration file not found" << endl;
        return false;
    }

    return true;
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

void KToshiba::createConfiguration()
{
    kdDebug() << "KToshiba: Creating configuration file..." << endl;
    KConfig config(CONFIG_FILE);
    config.setGroup("KToshiba");

    config.writeEntry("Notify_On_Full_Battery", false);
    config.writeEntry("Battery_Status_Time", 2);
    config.writeEntry("Low_Battery_Trigger", 15);
    config.writeEntry("Critical_Battery_Trigger", 5);
    config.writeEntry("Audio_Player", 1);
    config.writeEntry("Bluetooth_Startup", true);
    config.writeEntry("Fn_Esc", 1);
    config.writeEntry("Fn_F1", 2);
    config.writeEntry("Fn_F2", 3);
    config.writeEntry("Fn_F3", 4);
    config.writeEntry("Fn_F4", 5);
    config.writeEntry("Fn_F5", 6);
    config.writeEntry("Fn_F6", 7);
    config.writeEntry("Fn_F7", 8);
    config.writeEntry("Fn_F8", 9);
    config.writeEntry("Fn_F9", 10);
    config.writeEntry("Processing_Speed", 1);
    config.writeEntry("CPU_Sleep_Mode", 0);
    config.writeEntry("Display_Auto_Off", 5);
    config.writeEntry("HDD_Auto_Off", 5);
    config.writeEntry("LCD_Brightness", 2);
    config.writeEntry("Cooling_Method", 2);
    config.sync();
}

void KToshiba::displayAbout()
{
    mAboutWidget->show();
}

void KToshiba::checkPowerStatus()
{
    KConfig mConfig(CONFIG_FILE);
    mConfig.setGroup("KToshiba");
    loadConfiguration(&mConfig);
    if (mInterfaceAvailable) {
        mDriver->batteryStatus(&time, &perc);
        if (perc == -1)
            mProc->acpiBatteryStatus(&time, &perc);
        pow = ((mAC == -1)? SciACPower() : mDriver->acPowerStatus());
        if ((pow == -1) || (pow == SCI_FAILURE))
            pow = mProc->acpiAC();
    } else
    if (mOmnibook) {
        mProc->omnibookBatteryStatus(&time, &perc);
        pow = mProc->omnibookAC();
    }

    if (perc < 0 && !mInterfaceAvailable && !mOmnibook)
        wakeupEvent();

    if (mFullBat && perc == 100 && !battrig) {
        KMessageBox::queuedMessageBox(0, KMessageBox::Information,
					i18n("Your battery is now fully charged."), i18n("Laptop Battery"));
        battrig = true;
    }

    int th, tm;
    if (proc) {
        th = time / 60; tm = time % 60;
    } else {
        th = SCI_HOUR(time); tm = SCI_MINUTE(time);
    }
    if (tm == mLowBat && th == 0 && pow == 3 && !lowtrig) {
        KPassivePopup::message(i18n("Warning"), i18n("The battery state has changed to low"),
				SmallIcon("messagebox_warning", 20), this, i18n("Warning"), 15000);
        lowtrig = true;
    } else
    if (tm == mCryBat && th == 0 && pow == 3 && !crytrig) {
        KPassivePopup::message(i18n("Warning"), i18n("The battery state has changed to critical"),
				SmallIcon("messagebox_warning", 20), this, i18n("Warning"), 15000);
        crytrig = true;
    } else
    if (tm == 0 && th == 0 && pow == 3)
        KPassivePopup::message(i18n("Warning"), i18n("I'm Gone..."),
				SmallIcon("messagebox_warning", 20), this, i18n("Warning"), 15000);

    if (tm > lowtrig && pow == 4) {
        if (battrig && (perc < 100))
            battrig = false;
        if (lowtrig)
            lowtrig = false;
        if (crytrig)
            crytrig = false;
    }

    if (mBatSave != mOldBatSave || pow != oldpow) {
        int bright;
        switch (mBatSave) {
            case 0:			// USER SETTINGS or LONG LIFE
                if (mBatType == 3)
                    bright = 0;	// Semi-Bright
                else if (mBatType == 2)
                    bsmUserSettings(&mConfig, &bright);
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
        if (mInterfaceAvailable && mBatType != 2)
            mDriver->setBrightness(bright);
        else if (mOmnibook)
            mProc->omnibookSetBrightness(bright);
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
    if (mInterfaceAvailable)
        mOldBatSave = mBatSave;
    if (changed)
        displayPixmap();
}

void KToshiba::bsmUserSettings(KConfig *k, int *bright)
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
    if (lcd == 0) *bright = 7;		// Super-Bright
    else if (lcd == 1) *bright = 3;	// Bright
    else if (lcd == 2) *bright = 0;	// Semi-Bright
    mDriver->setCoolingMethod(cooling);
    mDriver->setDisplayAutoOff(display);
    mDriver->setHDDAutoOff(hdd);
}

void KToshiba::displayPixmap()
{
    int new_code = 0;

    if (!mInterfaceAvailable && !mOmnibook)
        new_code = 1;
    else if (pow == 3)
        new_code = 2;
    else
        new_code = 3;

    if (current_code != new_code) {
        current_code = new_code;
        // we will try to deduce the pixmap (or gif) name now.  it will
        // vary depending on the dock and power
        QString pixmap_name;

        if (pow == -1 && perc == -1)
            pixmap_name = QString("laptop_nobattery");
        else if (pow == 4 && perc == -1)
            pixmap_name = QString("laptop_power");
        else if (pow == 3)
            pixmap_name = QString("laptop_nocharge");
        else
            pixmap_name = QString("laptop_charge");

        pm = loadIcon(pixmap_name, instance);
    }

    // at this point, we have the file to display.  so display it

    QImage image = pm.convertToImage();

    int w = image.width();
    int h = image.height();
    int count = 0;
    QRgb rgb;
    int x, y;
    for (x = 0; x < w; x++)
        for (y = 0; y < h; y++) {
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
            for (x = 0; x < w; x++) {
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
    if (pow == 4) {
        if (perc == 100)
            tmp = i18n("Plugged in - fully charged");
        else {
            if (perc >= 0) {
                QString num3;
                int num2;
                if (proc) {
                    num2 = time / 60; num3.setNum(time % 60);
                } else {
                    num2 = SCI_HOUR(time); num3.setNum(SCI_MINUTE(time));
                }
                num3 = num3.rightJustify(2, '0');
                tmp = i18n("Plugged in - %1% charged (%2:%3 time left)")
			   .arg(perc).arg(num2).arg(num3);
            } else
            if (perc == -1)
                tmp = i18n("Plugged in - no battery");
            else
                tmp = i18n("Plugged in - %1% charged").arg(perc);
        }
    } else {
        if (perc >= 0) {
            QString num3;
            int num2;
            if (proc) {
                num2 = time / 60; num3.setNum(time % 60);
            } else {
                num2 = SCI_HOUR(time); num3.setNum(SCI_MINUTE(time));
            }
            num3 = num3.rightJustify(2, '0');
            tmp = i18n("Running on batteries - %1% charged (%2:%3 time left)")
			.arg(perc).arg(num2).arg(num3);
        } else
        if (perc == -1)
            tmp = i18n("No battery and adaptor found");
        else
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

    if ((key == 0x100) && (mFn->m_Popup != 0))
        mFn->hideWidgets();

    switch (key) {
        case 0:	// FIFO empty
            return;
        case 1:
            if (mDriver->hotkeys == false) {
                mDriver->enableSystemEvent();
                mDriver->hotkeys = true;
            }
            return;
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
        /** Multimedia Buttons */
        case 0xb25: // CD/DVD Mode
            MODE = CD_DVD;
            return;
        case 0xb27: // Digital Mode
            MODE = DIGITAL;
            return;
        case 0xb30:	// Stop/Eject
        case 0xd4f:
        case 0x9b3:
            if (MODE == CD_DVD)
                if (!mClient.call("kscd", "CDPlayer", "stop()", data, replyType, replyData))
                    if (!mClient.call("kaffeine", "KaffeineIface", "stop()", data, replyType, replyData))
                        proc << "eject" << "--cdrom";
            if (MODE == DIGITAL) {
                if (mAudioPlayer == amaroK)
                    mClient.send("amarok", "player", "stop()", "");
                else if (mAudioPlayer == JuK)
                    mClient.send("juk", "Player", "stop()", "");
                else if (mAudioPlayer == XMMS)
                    proc << "xmms" << "--stop";
            }
            break;
        case 0xb31:	// Previous
        case 0xd50:
        case 0x9b1:
            if (MODE == CD_DVD) {
                if (!mClient.call("kscd", "CDPlayer", "previous()", data, replyType, replyData))
                    mClient.send("kaffeine", "KaffeineIface", "previous()", "");
            } else
            if (MODE == DIGITAL) {
            if (mAudioPlayer == amaroK)
                mClient.send("amarok", "player", "prev()", "");
                else if (mAudioPlayer == JuK)
                    mClient.send("juk", "Player", "back()", "");
                else if (mAudioPlayer == XMMS)
                    proc << "xmms" << "--rew";
            }
            break;
        case 0xb32:	// Next
        case 0xd53:
        case 0x9b4:
            if (MODE == CD_DVD) {
                if (!mClient.call("kscd", "CDPlayer", "next()", data, replyType, replyData))
                    mClient.send("kaffeine", "KaffeineIface", "next()", "");
            } else
            if (MODE == DIGITAL) {
                if (mAudioPlayer == amaroK)
                    mClient.send("amarok", "player", "next()", "");
                else if (mAudioPlayer == JuK)
                    mClient.send("juk", "Player", "forward()", "");
                else if (mAudioPlayer == XMMS)
                    proc << "xmms" << "--fwd";
            }
            break;
        case 0xb33:	// Play/Pause
        case 0xd4e:
        case 0x9b2:
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
                else if (mAudioPlayer == XMMS)
                    proc << "xmms" << "--play-pause";
            }
            break;
        case 0xb85:	// Toggle S-Video Out
        case 0xd55:
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
            break;
        case 0xb87:	// I-Button
            proc << "konsole";
            break;
        case 0xd42: // Maximize
            return;
        case 0xd43: // Switch
            return;
        case 0xd51: // Rewind
            return;
        case 0xd52: // Fast Forward
            return;
        case 0xd54: // Mute
            mFn->performFnAction(1, key);
            return;
        case 0xd4c: // Menu
            return;
        case 0xd4d: // Mode
            if (MODE == CD_DVD)
                MODE = DIGITAL;
            else if (MODE == DIGITAL)
                MODE = CD_DVD;
            return;
    }
    if (key >= 0xb30) {
        proc.start(KProcess::DontCare);
        proc.detach();
        return;
    }
    mFn->performFnAction(tmp, key);
}

void KToshiba::checkMode()
{
    int temp = mProc->toshibaProcStatus();

    if (temp == CD_DVD)
        MODE = CD_DVD;
    else if (temp == DIGITAL)
        MODE = DIGITAL;
}

void KToshiba::checkSystem()
{
    int bay = -1, ws = -1;

    if (wstrig == false)
        ws = mDriver->getWirelessSwitch();
    if (baytrig == false)
        bay = mDriver->getBayDevice(1);

    if (ws < 0)
        wstrig = true;
    else if (wstrig == false) {
        if (mWirelessSwitch != ws) {
            QString s = ((ws == 1)? i18n("on") : i18n("off"));
            KPassivePopup::message(i18n("KToshiba"), i18n("Wireless antenna turned %1").arg(s),
				   SmallIcon("kwifimanager", 20), this, i18n("Wireless"), 4000);
        }
        mWirelessSwitch = ws;
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

void KToshiba::shutdownEvent()
{
    kdDebug() << "KToshiba: Shutting down..." << endl;
    if (mInterfaceAvailable) {
        mDriver->closeInterface();
        mInterfaceAvailable = false;
    }
}

void KToshiba::wakeupEvent()
{
    kdDebug() << "KToshiba: Starting up..." << endl;
    if (!mOmnibook) {
        mInterfaceAvailable = mDriver->openInterface();
        if (!mInterfaceAvailable)
            kdDebug() << "KToshiba: Interface could not be opened again "
                      << "please re-start application" << endl;
    }
}

void KToshiba::checkOmnibook()
{
    // TODO: Add more stuff here, for now only the LCD is being monitored

    if (mFn->m_Popup != 0) {
        mFn->m_StatusWidget->hide();
        mFn->m_Popup = 0;
    }

    int bright = mProc->omnibookGetBrightness();
    if (mFn->m_Bright != bright) {
        if (mFn->m_Popup == 0) {
            QRect r = QApplication::desktop()->geometry();
            mFn->m_StatusWidget->move(r.center() - 
                QPoint(mFn->m_StatusWidget->width()/2, mFn->m_StatusWidget->height()/2));
            mFn->m_StatusWidget->show();
            mFn->m_Popup = 1;
        }
        if (mFn->m_Popup == 1) {
            mFn->m_StatusWidget->wsStatus->raiseWidget(bright + 4);
            mFn->m_Bright = bright;
        }
    }
}

void KToshiba::doSetOneTouch(int state)
{
    mProc->omnibookSetOneTouch(state);
}

void KToshiba::doSetOmnibookFan(int state)
{
    mProc->omnibookSetFan(state);
}


#include "ktoshiba.moc"
