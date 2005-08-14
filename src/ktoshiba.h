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

#ifndef KTOSHIBA_H
#define KTOSHIBA_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qpixmap.h>

#include <ksystemtray.h>
#include <dcopclient.h>

class QTimer;
class QPopupMenu;

class KInstance;
class KAboutApplication;

class KToshibaSMMInterface;
class FnActions;

#define CD_DVD  	0x80
#define DIGITAL 	0x40

#define amaroK  	0
#define JuK 		1
#define XMMS		2

/**
 * @short Hotkeys & battery monitoring for Toshiba laptops
 * @author Azael Avalos <neftali@utep.edu>
 * @version 0.8
 */
class KToshiba : public KSystemTray
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    KToshiba();
    void displayPixmap();
    void loadConfiguration(KConfig *);
    void createConfiguration();
    bool checkConfiguration();

    /**
     * Default Destructor
     */
    virtual ~KToshiba();
protected slots:
    void doConfig();
    void doSuspendToRAM();
    void doSuspendToDisk();
    void doBluetooth();
    void doSetFreq(int);
    void doSetHyper(int);
    void displayAbout();
    void checkPowerStatus();
    void checkHotKeys();
    void checkSystem();
    void checkMode();
    void shutdownEvent();
    void wakeupEvent();
protected:
    KToshibaSMMInterface *mDriver;
    FnActions *mFn;
    KAboutApplication *mAboutWidget;
    KInstance *instance;
    DCOPClient mClient;
    QPopupMenu *mSpeed;
    QPopupMenu *mHyper;
    bool mInterfaceAvailable;
    bool mFullBat;
    int mBatStatus;
    int mOldBatStatus;
    int mLowBat;
    int mCryBat;
    int mBatSave;
    int mOldBatSave;
    int mBatType;
    int mWirelessSwitch;
    int mAudioPlayer;
    int mHT;
    int mSS;
    int mAC;
private:
    void doMenu();
    void bsmUserSettings(KConfig *, int *);
    void checkSelectBay();
    int acpiAC();
    int bayUnregister();
    int bayRescan();
    QTimer *mPowTimer;
    QTimer *mHotKeysTimer;
    QTimer *mModeTimer;
    QTimer *mSystemTimer;
    QPixmap pm;
    bool lowtrig;
    bool crytrig;
    bool battrig;
    bool wstrig;
    bool baytrig;
    bool btstart;
    int pow;
    int oldpow;
    int time;
    int oldtime;
    int perc;
    int oldperc;
    int current_code;
    int MODE;
    int bluetooth;
    int sblock;
    int removed;
    int svideo;
};

#endif // KTOSHIBA_H
