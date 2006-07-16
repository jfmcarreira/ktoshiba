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

#include <qobject.h>

#include <ksystemtray.h>
#include <kconfig.h>

#include "ktoshibadcopinterface.h"

class QTimer;
class QPopupMenu;

class KProcess;

class KToshibaSMMInterface;
class KToshibaProcInterface;
class ToshibaFnActions;
class OmnibookFnActions;
class suspend;

#define CD_DVD  	0x80
#define DIGITAL 	0x40

#define Amarok  	0
#define JuK 		1
#define XMMS		2

/**
 * @short Hotkeys & battery monitoring for Toshiba laptops
 * @author Azael Avalos <coproscefalo@gmail.com>
 * @version 0.9
 */
class KToshiba :  public KSystemTray
{
    Q_OBJECT
public:
    /**
    * Default Constructor
    */
    KToshiba();
    void loadConfiguration(KConfig *);
    void createConfiguration();
    bool checkConfiguration();

    /**
    * Default Destructor
    */
    ~KToshiba();
protected slots:
    void doConfig();
    void doSuspendToRAM();
    void doSuspendToDisk();
    void doBluetooth();
    void doSetFreq(int);
    void doSetHyper(int);
    void displayAbout();
    void checkHotKeys();
    void checkSystem();
    void checkMode();
    void checkOmnibook();
    void omnibookHotKeys(int);
    void doSetOneTouch(int);
    void doSetOmnibookFan(int);
    void quit();
protected:
    void doMenu();
    KToshibaSMMInterface *mSMMIFace;
    KToshibaProcInterface *mProcIFace;
    KToshibaDCOPInterface *mDCOPIFace;
    ToshibaFnActions *mTFn;
    OmnibookFnActions *mOFn;
    suspend *mSuspend;
    bool mOmnibook;
    bool mACPI;
    int mBatSave;
    int mOldBatSave;
    int mBatType;
    int mWirelessSwitch;
    int mAudioPlayer;
    int mHT;
    int mSS;
    int mAC;
private:
    void bsmUserSettings(KConfig *, int *);
    void checkSynaptics();
    void multimediaStopEject();
    void multimediaPrevious();
    void multimediaNext();
    void multimediaPlayPause();
    QPopupMenu *mSpeed;
    QPopupMenu *mHyper;
    QPopupMenu *mOneTouch;
    QPopupMenu *mOmniFan;
    QTimer *mHotKeysTimer;
    QTimer *mModeTimer;
    QTimer *mSystemTimer;
    QTimer *mOmnibookTimer;
    QByteArray mData;
    QByteArray mReplyData;
    QCString mReplyType;
    KProcess *mKProc;
    KProcess *mKeyProc;
    bool bsmtrig;
    bool wstrig;
    bool btstart;
    bool bluetooth;
    bool hotkeys;
    int pow;
    int oldpow;
    int MODE;
    int svideo;
};

#endif // KTOSHIBA_H
