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

#ifndef KTOSHIBA_H
#define KTOSHIBA_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qobject.h>

#include <ksystemtray.h>

class QTimer;
class QPopupMenu;

class KProcess;
class DCOPRef;

class KToshibaProcInterface;
class ToshibaFnActions;
#ifdef ENABLE_OMNIBOOK
class KToshibaDCOPInterface;
class OmnibookFnActions;
#endif // ENABLE_OMNIBOOK
class Suspend;

#define CD_DVD  	0x80
#define DIGITAL 	0x40

#define Amarok  	0
#define JuK 		1
#define XMMS		2

enum bsmmode {
	HighPower,
	DVDPlayback,
	Presentation
};

/**
 * @short Hotkeys monitoring for Toshiba laptops
 * @author Azael Avalos <coproscefalo@gmail.com>
 * @version 0.11
 */
class KToshiba : public KSystemTray
{
    Q_OBJECT
public:
    KToshiba();
    virtual ~KToshiba();

    void loadConfiguration();
    void createConfiguration();
    bool checkConfiguration();
    void createBSMConfig();
    bool checkBSMConfig();
private slots:
    void doConfig();
    void checkSystem();
    void doSuspendToRAM();
    void doSuspendToDisk();
    void doBluetooth();
    void displayBugReport();
    void displayAbout();
    void displayAboutKDE();
    void quit();
    void suspendToDisk();
    void resumedSTD();
    /** omnibook slots */
    void checkOmnibook();
    void omnibookHotKeys(int);
    void doSetOneTouch(int);
    void doSetOmnibookFan(int);
    void toggleMODE(int);
    /** toshiba slots */
    void checkHotKeys();
    void doSetFreq(int);
    void doSetHyper(int);
private:
    void doMenu();
    void bsmUserSettings(int *, int *, bsmmode);
    void multimediaStop();
    void multimediaPrevious();
    void multimediaNext();
    void multimediaPlayPause();
    void multimediaPlayer();
    void multimediaVolumeDown();
    void multimediaVolumeUp();
    void checkHardwareSettings();
    KToshibaProcInterface *mProcIFace;
    Suspend *mSuspend;
    QPopupMenu *mHelp;
    QTimer *mSystemTimer;
    QString konqueror;
    QString konsole;
    KProcess *mKProc;
    DCOPRef *mKaffeine;
    bool mOmnibook;
    bool mACPI;
    bool mHotkeys;
    bool mBtstart;
    int mAC;
    int mOldAC;
    int mBatSave;
    int mOldBatSave;
    int mBatType;
    int mBSMMode;
    int mWirelessSwitch;
    int mPad;
    int mBluetooth;
    int mAudioPlayer;
    int mBright;
    int MODE;
    /** toshiba stuff */
    ToshibaFnActions *mTFn;
    QPopupMenu *mSpeed;
    QPopupMenu *mHyper;
    QTimer *mHotKeysTimer;
    int mHT;
    int mSS;
    int mSVideo;
    int olddevconf;
    int oldparallel;
    int oldwokb;
    int oldusbemu;
    int oldusbfdd;
    int oldwol;
    int oldnbp;
    /** omnibook stuff */
#ifdef ENABLE_OMNIBOOK
    KToshibaDCOPInterface *mDCOPIFace;
    OmnibookFnActions *mOFn;
    QPopupMenu *mOneTouch;
    QPopupMenu *mOmniFan;
    QPopupMenu *mOmniMODE;
    QTimer *mOmnibookTimer;
    KProcess *mKeyProc;
#endif // ENABLE_OMNIBOOK
};

#endif // KTOSHIBA_H
