/***************************************************************************
 *   Copyright (C) 2004-2006 by Azael Avalos                               *
 *   coproscefalo@gmail.com                                                *
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
#include "ktoshibadcopinterface.h"
#include "toshibafnactions.h"
#include "omnibookfnactions.h"
#include "modelid.h"
#include "suspend.h"

#include <qpixmap.h>
#include <qimage.h>
#include <qtooltip.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kbugreport.h>
#include <kaboutapplication.h>
#include <kaboutkde.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kpassivepopup.h>
#include <kstandarddirs.h>
#include <dcopclient.h>
#include <dcopref.h>

#ifdef ENABLE_SYNAPTICS
#include <synaptics/synaptics.h>
#include <synaptics/synparams.h>

using namespace Synaptics;
#endif // ENABLE_SYNAPTICS

#ifdef ENABLE_POWERSAVE
#include <powersave_dbus.h>
#endif // ENABLE_POWERSAVE

#define CONFIG_FILE "ktoshibarc"

KToshiba::KToshiba()
    : KSystemTray( 0, "KToshiba" ),
      mSMMIFace( 0 ),
      mProcIFace( 0 ),
      mSystemTimer( 0 ),
      mKProc( new KProcess(this) ),
      hotkeys( false )
{
    this->setPixmap( KSystemTray::loadIcon("ktoshiba") );
    QToolTip::add(this, "KToshiba");
    this->show();

    mSMMIFace = new KToshibaSMMInterface(this);
    mProcIFace = new KToshibaProcInterface(this);

    if (kapp->dcopClient()->isRegistered())
        kdDebug() << "KToshiba: Registered with DCOP server as: "
                  << kapp->dcopClient()->appId() << endl;
    else {
        QCString appID = kapp->dcopClient()->registerAs("ktoshiba", false);
        kdDebug() << "KToshiba: Registered with DCOP server as: "
                  << appID << endl;
    }

    if (checkConfiguration()) {
        loadConfiguration();
    } else
        createConfiguration();

#ifdef ENABLE_OMNIBOOK
    // check whether omnibook module is loaded
    mOFn = new OmnibookFnActions(this);
    kdDebug() << "KToshiba: Checking for omnibook module..." << endl;
    mOmnibook = mOFn->m_OmnibookIface;
    if (!mOmnibook) {
        kdError() << "KToshiba: Could not found a Toshiba model. "
                  << "Please check that the omnibook module "
                  << "loads without failures" << endl;
        exit(-1);
    }
    kdDebug() << "KToshiba: Found a Toshiba model with Phoenix BIOS." << endl;
    kdDebug() << "KToshiba: Machine ECTYPE: " << mOFn->m_ECType << endl;
    mAC = mProcIFace->omnibookAC();
    (mAC == -1)? mACPI = true : mACPI = false;
    mPad = mOFn->m_Pad;
    mBatSave = 2;
    mBatType = 3;
    MODE = DIGITAL;
    hotkeys = false;

    // keyhandler program process
    pid_t mKeyPID;
    mKeyProc = new KProcess(this);
    *mKeyProc << "ktosh_keyhandler";
    mKeyProc->start(KProcess::DontCare);
    if (!mKeyProc->isRunning())
        kdError() << "KToshiba: The key handler program is not running."
                  << " HotKeys monitoring will no be enabled" << endl;
    else {
        mKeyPID = mKeyProc->pid();
        kdDebug() << "KToshiba: Key handler program PID "
                  << (unsigned int)mKeyPID << endl;
        hotkeys = true;
    }

    mOmnibookTimer = new QTimer(this);
    mDCOPIFace = new KToshibaDCOPInterface(this, "actions");

    connect( mOmnibookTimer, SIGNAL( timeout() ), this, SLOT( checkOmnibook() ) );
    mOmnibookTimer->start(100);
    if (hotkeys)
        connect( mDCOPIFace, SIGNAL( signalHotKey(int) ), this, SLOT( omnibookHotKeys(int) ) );
#else // ENABLE_OMNIBOOK
    // check whether toshiba module is loaded and we got an opened SCI IFace
    mTFn = new ToshibaFnActions(this);
    if (mTFn->m_SCIIface) {
        mSS = mSMMIFace->getSpeedStep();
        mHT = mSMMIFace->getHyperThreading();
        mBatType = mTFn->m_BatType;
        mTFn->m_BatSave = mBatSave;
    }
    else {
        mSS = -1;
        mHT = -1;
    }

    QString modelid = QString("KToshiba: Machine ID: 0x%1").arg(mTFn->m_MachineID, 0, 16);
    kdDebug() << modelid << endl;
    // Default to mode 2 if we got a failure
    if (mTFn->m_BatType == -1)
        mTFn->m_BatType = 2;
    mAC = mSMMIFace->acPowerStatus();
    mWirelessSwitch = mSMMIFace->getWirelessSwitch();
    mPad = mTFn->m_Pad;
    bsmtrig = false;
    bluetooth = false;
    svideo = 0;
    MODE = DIGITAL;
    mOmnibook = false;
    mACPI = false;
    hotkeys = false;

    mHotKeysTimer = new QTimer(this);

    connect( mHotKeysTimer, SIGNAL( timeout() ), this, SLOT( checkHotKeys() ) );
    mHotKeysTimer->start(100);		// Check hotkeys every 1/10 seconds

    if (btstart)
        doBluetooth();
#endif // ENABLE_OMNIBOOK

    doMenu();

    mSuspend = new Suspend(this);
    mKaffeine = new DCOPRef("kaffeine", "KaffeineIface");

    mSystemTimer = new QTimer(this);
    connect( mSystemTimer, SIGNAL( timeout() ), this, SLOT( checkSystem() ) );
    mSystemTimer->start(500);		// Check system events every 1/2 seconds
    connect( this, SIGNAL( quitSelected() ), this, SLOT( quit() ) );


#ifdef ENABLE_SYNAPTICS
    if (mPad == -1)
        checkSynaptics();
#endif // ENABLE_SYNAPTICS
}

KToshiba::~KToshiba()
{
#ifdef ENABLE_OMNIBOOK
    delete mDCOPIFace; mDCOPIFace = NULL;
    delete mOmnibookTimer; mOmnibookTimer = NULL;
    if (hotkeys)
        delete mKeyProc; mKeyProc = NULL;
    delete mOFn; mOFn = NULL;
#else
    delete mHotKeysTimer; mHotKeysTimer = NULL;
    delete mTFn; mTFn = NULL;
#endif
    delete mKProc; mKProc = NULL;
    delete mSystemTimer; mSystemTimer = NULL;
    delete mKaffeine; mKaffeine = NULL;
    delete mSuspend; mSuspend = NULL;
    delete mProcIFace; mProcIFace = NULL;
    delete mSMMIFace; mSMMIFace = NULL;
}

void KToshiba::quit()
{
#ifdef ENABLE_OMNIBOOK
    mOmnibookTimer->stop();
    if (mKeyProc->isRunning()) {
        kdDebug() << "KToshiba: Killing key handler program" << endl;
        mKeyProc->kill(SIGQUIT);
        mKeyProc->detach();
    }
#else // ENABLE_OMNIBOOK
    mHotKeysTimer->stop();
    mSystemTimer->stop();
    if (mTFn->m_SCIIface) {
        kdDebug() << "KToshiba: Closing SCI interface." << endl;
        mTFn->closeSCIIface();
    }
#endif // ENABLE_OMNIBOOK
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

void KToshiba::loadConfiguration()
{
    KConfig mConfig(CONFIG_FILE);
    mConfig.setGroup("BSM");
    mBatSave = mConfig.readNumEntry("Battery_Save_Mode", 2);
    mConfig.setGroup("KToshiba");
    mAudioPlayer = mConfig.readNumEntry("Audio_Player", 0);
    btstart = mConfig.readBoolEntry("Bluetooth_Startup", true);
}

void KToshiba::createConfiguration()
{
    kdDebug() << "KToshiba: Creating configuration file..." << endl;
    KConfig config(CONFIG_FILE);

    config.setGroup("BSM");
    config.writeEntry("Battery_Save_Mode", 2);
    config.writeEntry("CPU_Sleep_Mode", 0);
    config.writeEntry("Cooling_Method", 2);
    config.writeEntry("Display_Auto_Off", 5);
    config.writeEntry("HDD_Auto_Off", 5);
    config.writeEntry("LCD_Brightness", 2);
    config.writeEntry("Processing_Speed", 1);
    config.sync();
    config.setGroup("Fn_Key");
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
    config.sync();
    config.setGroup("KToshiba");
    config.writeEntry("Audio_Player", 0);
    config.writeEntry("AutoStart", true);
    config.writeEntry("Bluetooth_Startup", true);
    config.sync();
}

void KToshiba::doMenu()
{
    contextMenu()->insertItem( SmallIcon("memory"), i18n("Suspend To &RAM"), this,
                              SLOT( doSuspendToRAM() ), 0, 1, 1 );
    contextMenu()->insertItem( SmallIcon("hdd_unmount"), i18n("Suspend To &Disk"), this,
                              SLOT( doSuspendToDisk() ), 0, 2, 2 );
#ifdef ENABLE_POWERSAVE
    static int res;
    res = dbusSendSimpleMessage(REQUEST_MESSAGE, "AllowedSuspendToRam");
    if (res == REPLY_DISABLED)
        contextMenu()->setItemEnabled( 1, FALSE );
    res = dbusSendSimpleMessage(REQUEST_MESSAGE, "AllowedSuspendToDisk");
    if (res == REPLY_DISABLED)
        contextMenu()->setItemEnabled( 2, FALSE );
#else // ENABLE_POWERSAVE
    if (::access("/proc/acpi/sleep", F_OK) == -1) {
        contextMenu()->setItemEnabled( 1, FALSE );
        contextMenu()->setItemEnabled( 2, FALSE );
    }
#endif // ENABLE_POWERSAVE
    contextMenu()->insertSeparator( 3 );
#ifdef ENABLE_OMNIBOOK
    mOneTouch = new QPopupMenu( this, "OneTouch" );
    mOneTouch->insertItem( SmallIcon("hotkeys_off"), i18n("Disabled"), 0 );
    mOneTouch->insertItem( SmallIcon("hotkeys_on"), i18n("Enabled"), 1 );
    contextMenu()->insertItem( SmallIcon("hotkeys"), i18n("OneTouch Buttons"), mOneTouch, 4, 4 );
    mOmniFan = new QPopupMenu( this, "Fan" );
    mOmniFan->insertItem( SmallIcon("fan_off"), i18n("Disabled"), 0 );
    mOmniFan->insertItem( SmallIcon("fan_on"), i18n("Enabled"), 1 );
    contextMenu()->insertItem( SmallIcon("fan"), i18n("System Fan"), mOmniFan, 5, 5 );
    if (mOmnibook) {
        connect( mOneTouch, SIGNAL( activated(int) ), this, SLOT( doSetOneTouch(int) ) );
        connect( mOmniFan, SIGNAL( activated(int) ), this, SLOT( doSetOmnibookFan(int) ) );
    } else {
        contextMenu()->setItemEnabled( 4, FALSE );
        contextMenu()->setItemEnabled( 5, FALSE );
    }
    contextMenu()->insertSeparator( 6 );
    mOmniMODE = new QPopupMenu( this, "Mode" );
    mOmniMODE->insertItem( SmallIcon("cd_dvd"), i18n("CD/DVD"), 0 );
    mOmniMODE->insertItem( SmallIcon("digital"), i18n("Digital"), 1 );
    contextMenu()->insertItem( SmallIcon("mode"), i18n("Multimedia MODE"), mOmniMODE, 7, 7 );
    connect( mOmniMODE, SIGNAL( activated(int) ), this, SLOT( toggleMODE(int) ) );
#else // ENABLE_OMNIBOOK
    contextMenu()->insertItem( SmallIcon("kdebluetooth"), i18n("Enable &Bluetooth"), this,
                                     SLOT( doBluetooth() ), 0, 4, 4 );
    contextMenu()->insertSeparator( 5 );
    mHyper = new QPopupMenu( this, "HyperThreading" );
    mHyper->insertItem( SmallIcon("ht_disabled"), i18n("Disabled"), 0 );
    mHyper->insertItem( SmallIcon("ht_pm"), i18n("Enabled - PM aware"), 1 );
    mHyper->insertItem( SmallIcon("ht_no_pm"), i18n("Enabled - No PM aware"), 2 );
    contextMenu()->insertItem( SmallIcon("kcmprocessor"), i18n("Hyper-Threading"), mHyper, 6, 6 );
    mSpeed = new QPopupMenu( this, "SpeedStep" );
    mSpeed->insertItem( SmallIcon("cpu_dynamic"), i18n("Dynamic"), 0 );
    mSpeed->insertItem( SmallIcon("cpu_high"), i18n("Always High"), 1 );
    mSpeed->insertItem( SmallIcon("cpu_low"), i18n("Always Low"), 2 );
    contextMenu()->insertItem( SmallIcon("kcmprocessor"), i18n("CPU Frequency"), mSpeed, 7, 7 );
    if (mTFn->m_SCIIface && (mHT >= 0 || mSS >= 0)) {
        connect( mHyper, SIGNAL( activated(int) ), this, SLOT( doSetHyper(int) ) );
        connect( mSpeed, SIGNAL( activated(int) ), this, SLOT( doSetFreq(int) ) );
    } else {
        contextMenu()->setItemEnabled( 6, FALSE );
        contextMenu()->setItemEnabled( 7, FALSE );
    }
#endif // ENABLE_OMNIBOOK
    contextMenu()->insertSeparator( 8 );
    contextMenu()->insertItem( SmallIcon("configure"), i18n("&Configure KToshiba..."), this,
                              SLOT( doConfig() ), 0, 9, 9 );
    mHelp = new QPopupMenu( this, "Help" );
    mHelp->insertItem( i18n("&Report Bug..."), this,
                      SLOT( displayBugReport() ) );
    mHelp->insertSeparator();
    mHelp->insertItem( SmallIcon("ktoshiba"), i18n("&About KToshiba"), this,
                      SLOT( displayAbout() ) );
    mHelp->insertItem( SmallIcon("about_kde"), i18n("About &KDE"), this,
                      SLOT( displayAboutKDE() ) );
    contextMenu()->insertItem( SmallIcon("help"), i18n("&Help"), mHelp, 10, 10 );

#ifdef ENABLE_OMNIBOOK
    contextMenu()->insertTitle( mOFn->m_ModelName, 0, 0 );
#else // ENABLE_OMNIBOOK
    contextMenu()->insertTitle( modelID( mTFn->m_MachineID ), 0, 0 );
#endif // ENABLE_OMNIBOOK
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
    mSuspend->toRAM();
}

void KToshiba::doSuspendToDisk()
{
    mSuspend->toDisk();
}

void KToshiba::doSetOneTouch(int state)
{
    mProcIFace->omnibookSetOneTouch(state);
}

void KToshiba::doSetOmnibookFan(int state)
{
    mProcIFace->omnibookSetFan(state);
}

void KToshiba::toggleMODE(int mode)
{
    MODE = (mode)? CD_DVD : DIGITAL;
}

void KToshiba::doBluetooth()
{
    if (!mSMMIFace->getBluetooth()) {
        contextMenu()->setItemEnabled(6, FALSE);
        kdDebug() << "KToshiba::doBluetooth(): "
                  << "No Bluetooth device found" << endl;
        return;
    } else
    if (!bluetooth || (btstart && !bluetooth)) {
        mSMMIFace->setBluetoothPower(1);
        KPassivePopup::message(i18n("KToshiba"), i18n("Bluetooth device activated"),
				SmallIcon("kdebluetooth", 20), this, i18n("Bluetooth"), 4000);
        contextMenu()->setItemEnabled(6, FALSE);
        bluetooth = true;
    }
    else
        contextMenu()->setItemEnabled(6, TRUE);
}

void KToshiba::doSetFreq(int freq)
{
    mSMMIFace->setSpeedStep(freq);
}

void KToshiba::doSetHyper(int state)
{
    mSMMIFace->setHyperThreading(state);
}

void KToshiba::displayBugReport()
{
    KBugReport bug;
    bug.exec();
}

void KToshiba::displayAbout()
{
    KAboutApplication about(KGlobal::instance()->aboutData(), this);
    about.exec();
}

void KToshiba::displayAboutKDE()
{
    KAboutKDE aboutKDE;
    aboutKDE.exec();
}

void KToshiba::bsmUserSettings(int *bright)
{
    int processor, cpu, display, hdd, lcd, cooling, tmp;

    KConfig mConfig(CONFIG_FILE);
    mConfig.setGroup("BSM");
    processor = mConfig.readNumEntry("Processing_Speed", 1);
    cpu = mConfig.readNumEntry("CPU_Sleep_Mode", 0);
    display = mConfig.readNumEntry("Display_Auto_Off", 5);
    hdd = mConfig.readNumEntry("HDD_Auto_Off", 5);
    lcd = mConfig.readNumEntry("LCD_Brightness", 2);
    cooling = mConfig.readNumEntry("Cooling_Method", 2);

    kdDebug() << "Enabling User Settings..." << endl;
    (processor == 0)? tmp = 1 : tmp = 0;
    mSMMIFace->setProcessingSpeed(tmp);
    (cpu == 0)? tmp = 1 : tmp = 0;
    mSMMIFace->setCPUSleepMode(tmp);
    if (lcd == 0) *bright = 7;		// Super-Bright
    else if (lcd == 1) *bright = 3;	// Bright
    else if (lcd == 2) *bright = 0;	// Semi-Bright
    mSMMIFace->setCoolingMethod(cooling);
    mSMMIFace->setDisplayAutoOff(display);
    mSMMIFace->setHDDAutoOff(hdd);
}

#ifdef ENABLE_SYNAPTICS
void KToshiba::checkSynaptics()
{
    static bool err = false;
    if (!Pad::hasDriver()) {
        kdError() << "KToshiba: Incompatible synaptics driver version " << endl;
        err = true;
    }

    if (!err && !Pad::hasShm()) {
        kdError() << "KToshiba: Access denied to driver shared memory area" << endl;
        err = true;
    }

    if (!err && !Pad::hasParam(TOUCHPADOFF)) {
        kdDebug() << "KToshiba: TouchPad will not be enabled/disabled" << endl;
        err = true;
    }

    if (err) {
#ifdef ENABLE_OMNIBOOK
        mOFn->m_Mousepad = -1;
        return;
#else // ENABLE_OMNIBOOK
        mTFn->m_Mousepad = -1;
        return;
#endif // ENABLE_OMNIBOOK
    } else {
#ifdef ENABLE_OMNIBOOK
        mOFn->m_Mousepad = (int)Pad::getParam(TOUCHPADOFF);
#else // ENABLE_OMNIBOOK
        mTFn->m_Mousepad = (int)Pad::getParam(TOUCHPADOFF);
#endif // ENABLE_OMNIBOOK
    }
}
#endif // ENABLE_SYNAPTICS

void KToshiba::multimediaStop()
{
    if (MODE == CD_DVD)
        mKaffeine->send("stop");

    if (MODE == DIGITAL) {
        if (mAudioPlayer == Amarok)
            kapp->dcopClient()->send("amarok", "player", "stop()", "");
        else if (mAudioPlayer == JuK)
            kapp->dcopClient()->send("juk", "Player", "stop()", "");
        else if (mAudioPlayer == XMMS) {
            *mKProc << "xmms" << "--stop";
            mKProc->start(KProcess::DontCare);
            mKProc->detach();
        }
    }
}

void KToshiba::multimediaPrevious()
{
    if (MODE == CD_DVD)
        mKaffeine->send("previous");

    if (MODE == DIGITAL) {
        if (mAudioPlayer == Amarok)
            kapp->dcopClient()->send("amarok", "player", "prev()", "");
        else if (mAudioPlayer == JuK)
            kapp->dcopClient()->send("juk", "Player", "back()", "");
        else if (mAudioPlayer == XMMS) {
            *mKProc << "xmms" << "--rew";
            mKProc->start(KProcess::DontCare);
            mKProc->detach();
        }
    }
}

void KToshiba::multimediaNext()
{
    if (MODE == CD_DVD)
        mKaffeine->send("next");

    if (MODE == DIGITAL) {
        if (mAudioPlayer == Amarok)
            kapp->dcopClient()->send("amarok", "player", "next()", "");
        else if (mAudioPlayer == JuK)
            kapp->dcopClient()->send("juk", "Player", "forward()", "");
        else if (mAudioPlayer == XMMS) {
            *mKProc << "xmms" << "--fwd";
            mKProc->start(KProcess::DontCare);
            mKProc->detach();
        }
    }
}

void KToshiba::multimediaPlayPause()
{
    if (MODE == CD_DVD) {
        DCOPReply reply = mKaffeine->call("isPlaying");
        if (reply.isValid()) {
            bool res; res = reply;
            (res == true)? mKaffeine->send("pause")
                : mKaffeine->send("play");
        } else {
            kdWarning() << "KToshiba: Kaffeine not running" << endl;
            return;
        }
    }

    if (MODE == DIGITAL) {
        if (mAudioPlayer == Amarok)
            kapp->dcopClient()->send("amarok", "player", "playPause()", "");
        else if (mAudioPlayer == JuK)
            kapp->dcopClient()->send("juk", "Player", "playPause()", "");
        else if (mAudioPlayer == XMMS) {
            *mKProc << "xmms" << "--play-pause";
            mKProc->start(KProcess::DontCare);
            mKProc->detach();
        }
    }
}

void KToshiba::multimediaPlayer()
{
    if (MODE == CD_DVD)
        kapp->startServiceByDesktopName("kaffeine");

    if (MODE == DIGITAL) {
        KConfig mConfig(CONFIG_FILE);
        mConfig.setGroup("KToshiba");
        mAudioPlayer = mConfig.readNumEntry("Audio_Player", 0);

        KProcess proc;
        if (mAudioPlayer == 0)
            kapp->startServiceByDesktopName("amarok");
        else if (mAudioPlayer == 1)
            kapp->startServiceByDesktopName("juk");
        else if (mAudioPlayer == 2)
            proc << "xmms";
        proc.start(KProcess::DontCare);
        proc.detach();
    }
}

void KToshiba::multimediaVolumeDown()
{
    DCOPRef kmixClient("kmix", "Mixer0");
    kmixClient.send("decreaseVolume", 0);
}

void KToshiba::multimediaVolumeUp()
{
    DCOPRef kmixClient("kmix", "Mixer0");
    kmixClient.send("increaseVolume", 0);
}

void KToshiba::checkSystem()
{
#ifdef ENABLE_OMNIBOOK
    if (mOmnibook)
        pow = ((mACPI)? mProcIFace->acpiAC() : mProcIFace->omnibookAC());
#else // ENABLE_OMNIBOOK
    if (mTFn->m_SCIIface && !mACPI)
        pow = ((mAC == -1)? SciACPower() : mSMMIFace->acPowerStatus());
    if (mACPI || pow == -1) {
        pow = mProcIFace->acpiAC();
        mACPI = true;
    }
#endif // ENABLE_OMNIBOOK

    loadConfiguration();
    if (mBatSave != mOldBatSave || pow != oldpow) {
        int bright = 0;
        switch (mBatSave) {
            case 0:			// USER SETTINGS or LONG LIFE
                if (mBatType == 3)
                    bright = 0;	// Semi-Bright
#ifndef ENABLE_OMNIBOOK
                else if (mBatType == 2 && !bsmtrig && mTFn->m_SCIIface) {
                    bsmUserSettings(&bright);
                    bsmtrig = true;
                }
#endif // ENABLE_OMNIBOOK
                break;
            case 1:			// LOW POWER or NORMAL LIFE
                bright = (pow == 3)? 0 /*Semi-Bright*/ : 3 /*Bright*/;
                bsmtrig = false;
                break;
            case 2:			// FULL POWER or FULL LIFE
                bright = (pow == 3)? 3 /*Bright*/ : 7 /*Super-Bright*/;
                bsmtrig = false;
                break;
        }
#ifdef ENABLE_OMNIBOOK
        if (mOmnibook)
            mProcIFace->omnibookSetBrightness(bright);
#else // ENABLE_OMNIBOOK
        if (mTFn->m_BatType != 2)
            mSMMIFace->setBrightness(bright);
#endif // ENABLE_OMNIBOOK
    }

    oldpow = pow;
    mOldBatSave = mBatSave;

#ifndef ENABLE_OMNIBOOK
    if (mWirelessSwitch == -1)
        return;
    else {
        int ws = mSMMIFace->getWirelessSwitch();

        if (mWirelessSwitch != ws) {
            QString s = ((ws == 1)? i18n("on") : i18n("off"));
            KPassivePopup::message(i18n("KToshiba"), i18n("Wireless antenna turned %1").arg(s),
				   SmallIcon("kwifimanager", 20), this, i18n("Wireless"), 4000);
        }
        mWirelessSwitch = ws;
    }
#endif // ENABLE_OMNIBOOK
}

void KToshiba::omnibookHotKeys(int keycode)
{
#ifdef ENABLE_OMNIBOOK
    KConfig mConfig(CONFIG_FILE);
    mConfig.setGroup("Fn_Key");

    int tmp = 0;

    if (mOFn->m_Popup != 0)
        mOFn->hideWidgets();

    switch (keycode) {
        case 144:	// Previous
            multimediaPrevious();
            return;
        case 145:	// Fn-1 (Volume Down)
            multimediaVolumeDown();
            return;
        case 146:	// Fn-2 (Volume Up)
            multimediaVolumeUp();
            return;
        case 147:	// Media Player
            multimediaPlayer();
            return;
        case 148:	// Toggle Mode
            MODE = (MODE == CD_DVD)? DIGITAL : CD_DVD;
            return;
        case 150:	// Fn-F1
            tmp = mConfig.readNumEntry("Fn_F1");
            break;
        case 151:	// Fn-F9
        case 152:
            tmp = mConfig.readNumEntry("Fn_F9");
            break;
        case 153:	// Play/Pause also Next with ecype=12 (TSM30X)
            (mOFn->m_ECType == TSM30X)? multimediaNext()
                : multimediaPlayPause();
            return;
        case 159:	// Fn-F7
            tmp = mConfig.readNumEntry("Fn_F7");
            break;
        case 160:	// Fn-Esc
            tmp = mConfig.readNumEntry("Fn_Esc");
            break;
        case 162:	// Next also Play/Pause with ectype=12 (TSM30X)
            (mOFn->m_ECType == TSM30X)? multimediaPlayPause()
                : multimediaNext();
            return;
        case 164:	// Stop
            multimediaStop();
            return;
        case 178:	// WWW
            *mKProc << "kfmclient" << "openProfile" << "webbrowsing";
            break;
        case 236:	// Console Direct Access
            *mKProc << "konsole";
            break;
        case 239:	// Fn-F6
            tmp = mConfig.readNumEntry("Fn_F6");
            break;
    }
    if (keycode == 178 || keycode == 236) {
        mKProc->start(KProcess::DontCare);
        mKProc->detach();
        return;
    }
    mOFn->performFnAction(tmp);
#endif // ENABLE_OMNIBOOK
}

void KToshiba::checkOmnibook()
{
#ifdef ENABLE_OMNIBOOK
    if (mOFn->m_Popup != 0) {
        mOFn->hideWidgets();
        mOFn->m_Popup = 0;
    }

    int bright = mProcIFace->omnibookGetBrightness();
    if (mOFn->m_Bright != bright) {
        mOFn->m_Bright = bright;
        mOFn->performFnAction(7);
    }
#endif // ENABLE_OMNIBOOK
}

void KToshiba::checkHotKeys()
{
#ifndef ENABLE_OMNIBOOK
    KConfig mConfig(CONFIG_FILE);
    mConfig.setGroup("Fn_Key");

    int tmp = 0;
    int key = mSMMIFace->getSystemEvent();

    if ((key == 0x100) && (mTFn->m_Popup != 0))
        mTFn->hideWidgets();

    switch (key) {
        case 0:		// FIFO empty
            return;
        case 1:		// Failed accessing System Events
            if (mSMMIFace->hotkeys == false)
                hotkeys = mSMMIFace->enableSystemEvent();
            if (!hotkeys) {
                mSMMIFace->hotkeys = true;
                mHotKeysTimer->stop();
                disconnect(mHotKeysTimer);
                return;
            } else
                mSMMIFace->hotkeys = true;
            return;
        case 0x101:	// Fn-Esc
            tmp = mConfig.readNumEntry("Fn_Esc");
            break;
        case 0x13b:	// Fn-F1
            tmp = mConfig.readNumEntry("Fn_F1");
            break;
        case 0x13c:	// Fn-F2
            tmp = mConfig.readNumEntry("Fn_F2");
            break;
        case 0x13d:	// Fn-F3
            tmp = mConfig.readNumEntry("Fn_F3");
            break;
        case 0x13e:	// Fn-F4
            tmp = mConfig.readNumEntry("Fn_F4");
            break;
        case 0x13f:	// Fn-F5
            tmp = mConfig.readNumEntry("Fn_F5");
            break;
        case 0x140:	// Fn-F6
            tmp = mConfig.readNumEntry("Fn_F6");
            break;
        case 0x141:	// Fn-F7
            tmp = mConfig.readNumEntry("Fn_F7");
            break;
        case 0x142:	// Fn-F8
            tmp = mConfig.readNumEntry("Fn_F8");
            break;
        case 0x143:	// Fn-F9
            tmp = mConfig.readNumEntry("Fn_F9");
            break;
        /** Multimedia Buttons */
        case 0xb25:	// CD/DVD Mode
        case 0x12e:
            MODE = CD_DVD;
            return;
        case 0xb27:	// Digital Mode
        case 0x120:
            MODE = DIGITAL;
            return;
        case 0xb30:	// Stop
        case 0xd4f:
        case 0x9b3:
            multimediaStop();
            return;
        case 0xb31:	// Previous
        case 0xd50:
        case 0x9b1:
            multimediaPrevious();
            return;
        case 0xb32:	// Next
        case 0xd53:
        case 0x9b4:
            multimediaNext();
            return;
        case 0xb33:	// Play/Pause
        case 0xd4e:
        case 0x9b2:
            multimediaPlayPause();
            return;
        case 0xb85:	// Toggle S-Video Out
        case 0xd55:
            (svideo)? mSMMIFace->setVideo(1) : mSMMIFace->setVideo(4);
            svideo = (svideo)? 0 : 1;
            return;
        case 0xb86:	// E-Button
        case 0x006:
            *mKProc << "kfmclient" << "openProfile" << "webbrowsing";
            break;
        case 0xb87:	// I-Button
            *mKProc << "konsole";
            break;
        case 0xd42:	// Maximize
            if (MODE == CD_DVD) {
                kapp->dcopClient()->call("kaffeine", "kaffeine_mainview", "fullscreen()", mData, mReplyType, mReplyData);
                if (mReplyType == "bool")
                    mKaffeine->send("fullscreen");
            }
            return;
        case 0xd43:	// Switch
            // what does this do...?
            return;
        case 0xd51:	// Rewind
            return;
        case 0xd52:	// Fast Forward
            return;
        case 0xd54:	// Mute
            mTFn->performFnAction(1, key);
            return;
        case 0xd4c:	// Menu
            // DVD menu I guess, too bad Kaffeine doesn't do it via DCOP...
            return;
        case 0xd4d:	// Toggle Mode
        case 0x114:
            MODE = (MODE == CD_DVD)? DIGITAL : CD_DVD;
            return;
    }
    if (key == 0x006 || key == 0xb86 || key == 0xb87) {
        mKProc->start(KProcess::DontCare);
        mKProc->detach();
        return;
    }
    mTFn->performFnAction(tmp, key);
#endif // ENABLE_OMNIBOOK
}


#include "ktoshiba.moc"
