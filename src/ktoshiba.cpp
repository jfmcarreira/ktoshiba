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
#include "ktoshibaomnibookinterface.h"
#include "ktoshibaprocinterface.h"
#include "ktoshibadcopinterface.h"
#include "toshibafnactions.h"
#ifdef ENABLE_OMNIBOOK
#include "omnibookfnactions.h"
#endif // ENABLE_OMNIBOOK
#include "modelid.h"
#include "suspend.h"

#include <qtooltip.h>
#include <qtimer.h>

#include <kprocess.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <kdebug.h>
#include <dcopref.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kpassivepopup.h>
#include <kbugreport.h>
#include <kaboutapplication.h>
#include <kaboutkde.h>

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
      mProcIFace( 0 ),
      mSystemTimer( 0 ),
      mKProc( new KProcess(this) ),
      mHotkeys( false )
{
    this->setPixmap( KSystemTray::loadIcon("ktoshiba") );
    QToolTip::add(this, "KToshiba");
    this->show();

    mProcIFace = new KToshibaProcInterface(0);
    mSuspend = new Suspend(this);

    if (!kapp->dcopClient()->isRegistered()) {
        QCString appID = kapp->dcopClient()->registerAs("ktoshiba", false);
        kdDebug() << "KToshiba: Registered with DCOP server as: "
                  << appID << endl;
    }

    if (checkConfiguration()) {
        loadConfiguration();
    } else
        createConfiguration();

    mOmnibook = false;

    // check whether toshiba module is loaded and we got an opened SCI interface
    mTFn = new ToshibaFnActions(this);
    if (mTFn->m_SCIIface) {
        mSS = mTFn->m_Driver->getSpeedStep();
        mHT = mTFn->m_Driver->getHyperThreading();
    } else {
        mSS = -1;
        mHT = -1;
    }

    if (mTFn->m_BIOS != -1) {
        kdDebug() << "KToshiba: BIOS version: "
                  << (mTFn->m_BIOS / 0x100) << "." << (mTFn->m_BIOS - 0x100) << endl;
        kdDebug() << "KToshiba: Machine ID: "
                  << QString("0x%1").arg(mTFn->m_MachineID, 0, 16) << endl;
        mHotkeys = mTFn->m_Driver->enableSystemEvent();
        mWirelessSwitch = mTFn->m_Driver->getWirelessSwitch();
        mBluetooth = mTFn->m_Driver->getBluetooth();
        mAC = mTFn->m_Driver->acPowerStatus();
        mBatType = mTFn->m_BatType;
        mBatSave = mTFn->m_BatSave;
        bsmtrig = (mBatSave == 0)? true : false;
        mSVideo = mTFn->m_Video;
        mPad = mTFn->m_Pad;

        mOmnibook = false;
    }

#ifdef ENABLE_OMNIBOOK
    if (!mTFn->m_SCIIface && (mTFn->m_BIOS == -1)) {
        kdDebug() << "KToshiba: Checking for omnibook module..." << endl;
        // check whether omnibook module is loaded
        mOFn = new OmnibookFnActions(this);
        if (!mOFn->m_OmnibookIface) {
            kdError() << "KToshiba: Could not found a Toshiba model. "
                      << "Exiting..." << endl;
            if (kapp->dcopClient()->isAttached())
                kapp->dcopClient()->detach();
            exit(-1);
        }
        int bios = mOFn->m_Omni->machineBIOS();
        kdDebug() << "KToshiba: BIOS version: "
                  << (bios / 0x100) << "." << (bios - 0x100) << endl;
        mAC = mOFn->m_Omni->omnibookAC();
        mPad = mOFn->m_Pad;
        mWirelessSwitch = mOFn->m_Omni->getWifiSwitch();
        mBluetooth = mOFn->m_Omni->getBluetooth();
        mBatSave = 2;
        mBatType = 3;

        // keyhandler program process
        mKeyProc = new KProcess(this);
        QString keyhandler = KStandardDirs::findExe("ktosh_keyhandler");
        if (keyhandler.isEmpty()) {
            KMessageBox::sorry(this, i18n("The program ktosh_keyhandler cannot be found.\n"
                               " HotKeys monitoring will no be enabled."),
                               i18n("HotKeys"));
            mHotkeys = false;
        } else {
            *mKeyProc << keyhandler;
            bool start;
            start = mKeyProc->start(KProcess::OwnGroup);
            if (start) {
                kdDebug() << "KToshiba: Key handler program PID: "
                          << mKeyProc->pid() << endl;
                if (mKeyProc->exitStatus() == -1) {
                    kdError() << "KToshiba: ktosh_keyhandler exited."
                              << " HotKeys monitoring will no be enabled." << endl;
                    mHotkeys = false;
                } else
                    mHotkeys = true;
            } else
                mHotkeys = false;
        }
        mOmnibook = true;
    }
#endif // ENABLE_OMNIBOOK

    mACPI = (mAC == -1)? true : false;
    mOldAC = mAC;
    mOldBatSave = mBatSave;
    MODE = DIGITAL;

    doMenu();

    mKaffeine = new DCOPRef("kaffeine", "KaffeineIface");

    mSystemTimer = new QTimer(this);
    connect( mSystemTimer, SIGNAL( timeout() ), this, SLOT( checkSystem() ) );
    mSystemTimer->start(500);
    connect( this, SIGNAL( quitSelected() ), this, SLOT( quit() ) );

    connect( mSuspend, SIGNAL( setSuspendToDisk() ), this, SLOT( suspendToDisk() ) );
#ifdef ENABLE_POWERSAVE
    connect( mSuspend, SIGNAL( resumedFromSTD() ), this, SLOT( resumedSTD() ) );
#endif // ENABLE_POWERSAVE

    if (!mOmnibook) {
        mHotKeysTimer = new QTimer(this);
        if (mHotkeys) {
            connect( mHotKeysTimer, SIGNAL( timeout() ), this, SLOT( checkHotKeys() ) );
            mHotKeysTimer->start(100);
        }
    } else
    if (mOmnibook) {
#ifdef ENABLE_OMNIBOOK
        mOmnibookTimer = new QTimer(this);
        connect( mOmnibookTimer, SIGNAL( timeout() ), this, SLOT( checkOmnibook() ) );
        mOmnibookTimer->start(100);

        if (mHotkeys) {
            mDCOPIFace = new KToshibaDCOPInterface(this, "actions");
            connect( mDCOPIFace, SIGNAL( signalHotKey(int) ), this, SLOT( omnibookHotKeys(int) ) );
        }
#endif // ENABLE_OMNIBOOK
    }

#ifdef ENABLE_SYNAPTICS
    if (mPad == -1)
        checkSynaptics();
#endif // ENABLE_SYNAPTICS

    if (mBtstart)
        doBluetooth();
}

KToshiba::~KToshiba()
{
    if (!mOmnibook && (mTFn->m_SCIIface || (mTFn->m_BIOS != -1))) {
        delete mHyper; mHyper = NULL;
        delete mSpeed; mSpeed = NULL;
        if (mHotkeys)
            delete mHotKeysTimer; mHotKeysTimer = NULL;
    }
#ifdef ENABLE_OMNIBOOK
    if (mOmnibook) {
        delete mOneTouch; mOneTouch = NULL;
        delete mOmniFan; mOmniFan = NULL;
        delete mOmniMODE; mOmniMODE = NULL;
        if (mHotkeys)
            delete mDCOPIFace; mDCOPIFace = NULL;
        delete mOmnibookTimer; mOmnibookTimer = NULL;
        delete mKeyProc; mKeyProc = NULL;
    }
    delete mOFn; mOFn = NULL;
#endif
    delete mHelp; mHelp = NULL;
    delete mKProc; mKProc = NULL;
    delete mSystemTimer; mSystemTimer = NULL;
    delete mKaffeine; mKaffeine = NULL;
    delete mTFn; mTFn = NULL;
    delete mSuspend; mSuspend = NULL;
    delete mProcIFace; mProcIFace = NULL;
}

void KToshiba::quit()
{
    mSystemTimer->stop();
    if (!mOmnibook) {
        if (mHotkeys)
            mHotKeysTimer->stop();
        if (mTFn->m_SCIIface) {
            kdDebug() << "KToshiba: Closing SCI interface." << endl;
            mTFn->m_Driver->closeSCIInterface();
        }
    }
#ifdef ENABLE_OMNIBOOK
    if (mOmnibook) {
        mOmnibookTimer->stop();
        if (mHotkeys) {
            mKeyProc->kill(SIGKILL);
            mKeyProc->detach();
        }
    }
#endif // ENABLE_OMNIBOOK
}

void KToshiba::resumedSTD()
{
    kdDebug() << "KToshiba: Resuming from Suspend To Disk..." << endl;
    if (!mOmnibook) {
        kdDebug() << "KToshiba: Opening SCI interface." << endl;
        mTFn->m_SCIIface = mTFn->m_Driver->openSCIInterface(&(mTFn->m_IFaceErr));
    }
}

void KToshiba::suspendToDisk()
{
    kdDebug() << "KToshiba: Suspending To Disk..." << endl;
#ifndef ENABLE_POWERSAVE
    // 1 minute grace time before we call resume slot
    QTimer::singleShot( 60000, this, SLOT( resumedSTD() ) );
#endif // ENABLE_POWERSAVE
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
    mConfig.setGroup("KToshiba");
    mAudioPlayer = mConfig.readNumEntry("Audio_Player", 0);
    mBtstart = mConfig.readBoolEntry("Bluetooth_Startup", true);
    mConfig.setGroup("BSM");
    mBatSave = mConfig.readNumEntry("Battery_Save_Mode", 2);
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
    if (!mOmnibook) {
        contextMenu()->insertItem( SmallIcon("kdebluetooth"), i18n("Enable &Bluetooth"), this,
                                     SLOT( doBluetooth() ), 0, 4, 4 );
        if (!mBluetooth)
            contextMenu()->setItemEnabled( 4, FALSE );
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
    }
#ifdef ENABLE_OMNIBOOK
    if (mOmnibook) {
        mOneTouch = new QPopupMenu( this, "OneTouch" );
        mOneTouch->insertItem( SmallIcon("hotkeys_off"), i18n("Disabled"), 0 );
        mOneTouch->insertItem( SmallIcon("hotkeys_on"), i18n("Enabled"), 1 );
        contextMenu()->insertItem( SmallIcon("hotkeys"), i18n("OneTouch Buttons"), mOneTouch, 4, 4 );
        mOmniFan = new QPopupMenu( this, "Fan" );
        mOmniFan->insertItem( SmallIcon("fan_off"), i18n("Disabled"), 0 );
        mOmniFan->insertItem( SmallIcon("fan_on"), i18n("Enabled"), 1 );
        contextMenu()->insertItem( SmallIcon("fan"), i18n("System Fan"), mOmniFan, 5, 5 );
        if (mOFn->m_OmnibookIface) {
            connect( mOneTouch, SIGNAL( activated(int) ), this, SLOT( doSetOneTouch(int) ) );
            connect( mOmniFan, SIGNAL( activated(int) ), this, SLOT( doSetOmnibookFan(int) ) );
        }
        if (::access("/proc/omnibook/onetouch", F_OK) == -1)
            contextMenu()->setItemEnabled( 4, FALSE );
        if (::access("/proc/omnibook/fan", F_OK) == -1)
            contextMenu()->setItemEnabled( 5, FALSE );
        contextMenu()->insertSeparator( 6 );
        mOmniMODE = new QPopupMenu( this, "Mode" );
        mOmniMODE->insertItem( SmallIcon("cd_dvd"), i18n("CD/DVD"), 0 );
        mOmniMODE->insertItem( SmallIcon("digital"), i18n("Digital"), 1 );
        contextMenu()->insertItem( SmallIcon("mode"), i18n("Multimedia MODE"), mOmniMODE, 7, 7 );
        connect( mOmniMODE, SIGNAL( activated(int) ), this, SLOT( toggleMODE(int) ) );
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

    if (!mOmnibook) contextMenu()->insertTitle( modelID( mTFn->m_MachineID ), 0, 0 );
#ifdef ENABLE_OMNIBOOK
    if (mOmnibook) contextMenu()->insertTitle( mOFn->m_ModelName, 0, 0 );
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
    if (state == 0) ;
#ifdef ENABLE_OMNIBOOK
    mOFn->m_Omni->setOneTouch(state);
#endif // ENABLE_OMNIBOOK
}

void KToshiba::doSetOmnibookFan(int state)
{
    if (state == 0) ;
#ifdef ENABLE_OMNIBOOK
    mOFn->m_Omni->setFan(state);
#endif // ENABLE_OMNIBOOK
}

void KToshiba::toggleMODE(int mode)
{
    MODE = (mode)? CD_DVD : DIGITAL;
}

void KToshiba::doBluetooth()
{
    if (mBluetooth == -1) return;

    if (mBtstart && (mBluetooth != 0)) {
        if (mTFn->m_BIOS != -1 && !mOmnibook) mTFn->m_Driver->setBluetoothPower(1);
#ifdef ENABLE_OMNIBOOK
        if (mOmnibook) mOFn->m_Omni->setBluetooth(1);
#endif // ENABLE_OMNIBOOK
        KPassivePopup::message(i18n("KToshiba"), i18n("Bluetooth device activated"),
				SmallIcon("kdebluetooth", 20), this, "Bluetooth", 4000);
        contextMenu()->setItemEnabled(4, FALSE);
    }
}

void KToshiba::doSetFreq(int freq)
{
    mTFn->m_Driver->setSpeedStep(freq);
}

void KToshiba::doSetHyper(int state)
{
    mTFn->m_Driver->setHyperThreading(state);
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
    tmp = (processor == 0)? 1 : 0;
    mTFn->m_Driver->setProcessingSpeed(tmp);
    tmp = (cpu == 0)? 1 : 0;
    mTFn->m_Driver->setCPUSleepMode(tmp);
    if (lcd == 0) *bright = 7;		// Super-Bright
    else if (lcd == 1) *bright = 3;	// Bright
    else if (lcd == 2) *bright = 0;	// Semi-Bright
    mTFn->m_Driver->setCoolingMethod(cooling);
    mTFn->m_Driver->setDisplayAutoOff(display);
    mTFn->m_Driver->setHDDAutoOff(hdd);
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
        kdError() << "KToshiba: TouchPad will not be enabled/disabled" << endl;
        err = true;
    }

    if (!err) {
        int pad = (int)Pad::getParam(TOUCHPADOFF);
        if (!mOmnibook) mTFn->m_Pad = pad;
#ifdef ENABLE_OMNIBOOK
        if (mOmnibook) mOFn->m_Pad = pad;
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
            bool res = reply;
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

        if (mAudioPlayer == 0)
            kapp->startServiceByDesktopName("amarok");
        else if (mAudioPlayer == 1)
            kapp->startServiceByDesktopName("juk");
        else if (mAudioPlayer == 2) {
            KProcess proc;
            proc << "xmms";
            proc.start(KProcess::DontCare);
            proc.detach();
        }
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
    if (!mOmnibook) {
        mAC = (mACPI)? mProcIFace->acpiAC() : mTFn->m_Driver->acPowerStatus();
        KConfig mConfig(CONFIG_FILE);
        mConfig.setGroup("BSM");
        mBatSave = mConfig.readNumEntry("Battery_Save_Mode", 2);
    }
#ifdef ENABLE_OMNIBOOK
    if (mOmnibook)
        mAC = (mACPI)? mProcIFace->acpiAC() : mOFn->m_Omni->omnibookAC();
#endif // ENABLE_OMNIBOOK

    if (mBatSave != mOldBatSave || mAC != mOldAC) {
        int bright = 0;
        switch (mBatSave) {
            // Brightness settings info:
            //     7 - Super-Bright
            //     3 - Bright
            //     0 - Semi-Bright
            case 0:							// USER SETTINGS
                if (!bsmtrig && mTFn->m_SCIIface) {
                    bsmUserSettings(&bright);
                    bsmtrig = true;
                }
                break;
            case 1:
                if (mBatType == 3)				// LONG LIFE
                    bright = 0;
                else if (mBatType == 2)		// LOW POWER
                    bright = (mAC == 3)? 0 : 3;
                break;
            case 2:
                if (mBatType == 3)				// NORMAL LIFE or ECONOMY
                    bright = (mAC == 3)? 0 : 3;
                else if (mBatType == 2)		// FULL POWER
                    bright = (mAC == 3)? 3 : 7;
                break;
            case 3:							// FULL LIFE
                bright = (mAC == 3)? 3 : 7;
                break;
        }
        if (mTFn->m_BatType != 2 && mTFn->m_BIOS != -1 && !mOmnibook)
            mTFn->m_Driver->setBrightness(bright);
        if (mBatSave != 0 && bsmtrig == true && !mOmnibook)
            bsmtrig = false;
#ifdef ENABLE_OMNIBOOK
        if (mOmnibook)
            mOFn->m_Omni->setBrightness(bright);
#endif // ENABLE_OMNIBOOK
    }

    mOldAC = mAC;
    mOldBatSave = mBatSave;

    if (mWirelessSwitch == -1) return;
    else {
        int ws = 0;
        if (mTFn->m_BIOS != -1 && !mOmnibook) ws = mTFn->m_Driver->getWirelessSwitch();
#ifdef ENABLE_OMNIBOOK
        if (mOmnibook) ws = mOFn->m_Omni->getWifiSwitch();
#endif // ENABLE_OMNIBOOK
        if (mWirelessSwitch != ws) {
            QString s = (ws == 1)? i18n("on") : i18n("off");
            KPassivePopup::message(i18n("KToshiba"), i18n("Wireless antenna turned %1").arg(s),
				   SmallIcon("kwifimanager", 20), this, i18n("Wireless"), 4000);
        }
        mWirelessSwitch = ws;
    }
}

void KToshiba::omnibookHotKeys(int keycode)
{
    if (keycode == 0) ;
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
        case 149:	// Battery Status
            tmp = 22;
            break;
        case 150:	// Fn-F1
            tmp = mConfig.readNumEntry("Fn_F1");
            break;
        case 151:	// Fn-F9 (MousePad On/Off)
        case 152:
            tmp = 10;
            break;
        case 153:	// Play/Pause also Next with ecype=12 (TSM30X)
            (mOFn->m_ECType == TSM30X)? multimediaNext()
                : multimediaPlayPause();
            return;
        case 159:	// Fn-F7
            tmp = 8;
            break;
        case 160:	// Fn-Esc
            tmp = mConfig.readNumEntry("Fn_Esc");
            break;
        case 161:	// Fn-F8
            tmp = mConfig.readNumEntry("Fn_F8");
            break;
        case 162:	// Next also Play/Pause with ectype=12 (TSM30X)
            (mOFn->m_ECType == TSM30X)? multimediaPlayPause()
                : multimediaNext();
            return;
        case 163:	// Bluetooth On/Off
            tmp = 15;
            return;
        case 164:	// Stop
            multimediaStop();
            return;
        case 178:	// WWW
            konqueror = KStandardDirs::findExe("kfmclient");
            if (konqueror.isEmpty())
                *mKProc << "firefox";
            else
                *mKProc << konqueror << "openProfile" << "webbrowsing";
            break;
        case 236:	// Console Direct Access
            konsole = KStandardDirs::findExe("konsole");
            if (konsole.isEmpty())
                *mKProc << "xterm";
            else
                *mKProc << konsole;
            break;
        case 239:	// Fn-F6
            tmp = 7;
            break;
    }
    if (keycode == 178 || keycode == 236) {
        mKProc->start(KProcess::DontCare);
        mKProc->detach();
        return;
    }
    mOFn->performFnAction(tmp, keycode);
#endif // ENABLE_OMNIBOOK
}

void KToshiba::checkOmnibook()
{
#ifdef ENABLE_OMNIBOOK
    int bright = mOFn->m_Omni->getBrightness();
    if (bright == -1) {
        mOmnibookTimer->stop();
        disconnect(mOmnibookTimer);
        return;
    } else
    if (mOFn->m_Bright != bright) {
        mOFn->m_Bright = bright;
        mOFn->performFnAction(7, 300);
    }
#endif // ENABLE_OMNIBOOK
}

void KToshiba::checkHotKeys()
{
    KConfig mConfig(CONFIG_FILE);
    mConfig.setGroup("Fn_Key");

    int tmp = 0;
    if (mSuspend->suspended) return;
    int key = mTFn->m_Driver->getSystemEvent();

    if ((key == 0x100) && (mTFn->m_Popup != 0))
        mTFn->hideWidgets();

    switch (key) {
        case -1:		// Not Supported
            if (mHotkeys) {
                if (mTFn->m_SCIIface)
                    mTFn->m_SCIIface = mTFn->m_Driver->openSCIInterface(&(mTFn->m_IFaceErr));
                mHotkeys = mTFn->m_Driver->enableSystemEvent();
                return;
            }
            kdError() << "KToshiba: Hotkeys monitoring will be disabled" << endl;
            mHotKeysTimer->stop();
            disconnect(mHotKeysTimer);
            return;
        case 0:		// FIFO empty
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
        /** Multimedia & Extra Buttons */
        case 0x130:	// Fn-b (Battery Status)
            tmp = 22;
            break;
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
            mSVideo = mTFn->m_Driver->getVideo();
            // If the user changed the video out, we receive 129 for LCD
            if (mSVideo == 129) mSVideo = 1;
            mTFn->m_Driver->setVideo((mSVideo != 4)? 4 : mSVideo);
            return;
        case 0xb86:	// E-Button
        case 0x006:
            konqueror = KStandardDirs::findExe("kfmclient");
            if (konqueror.isEmpty())
                *mKProc << "firefox";
            else
                *mKProc << konqueror << "openProfile" << "webbrowsing";
            break;
        case 0xb87:	// I-Button
            konsole = KStandardDirs::findExe("konsole");
            if (konsole.isEmpty())
                *mKProc << "xterm";
            else
                *mKProc << konsole;
            break;
        case 0xd42:	// Maximize
            if (MODE == CD_DVD) {
                DCOPRef kaffeine("kaffeine", "kaffeine_mainview");
                DCOPReply reply = kaffeine.call("fullscreen");
                if (reply.isValid()) {
                    bool res = reply;
                    if (res == false)
                        kaffeine.send("fullscreen");
                }
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
}


#include "ktoshiba.moc"
