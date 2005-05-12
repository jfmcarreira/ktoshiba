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

#include <qtimer.h>
#include <qpixmap.h>

#include <ksystemtray.h>
#include <kprocess.h>
#include <kaboutapplication.h>
#include <dcopclient.h>

#include "ktoshibasmminterface.h"
#include "synshm.h" 	// ksynaptics file

#define CD_DVD  		0x80
#define DIGITAL 		0x40

#define amaroK  		0
#define JuK 			1
#define XMMS			2

class SettingsWidget;
class StatusWidget;

/**
 * @short Hotkeys & battery monitoring for Toshiba laptops
 * @author Azael Avalos <neftali@utep.edu>
 * @version 0.6
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

    /**
     * Default Destructor
     */
    virtual ~KToshiba();
protected slots:
	void doConfig();
	void displayAbout();
	void powerStatus();
	void checkHotKeys();
	void doSuspendToDisk();
	void doSuspendToRAM();
	void mode();
	void doBluetooth();
	void checkSystem();
protected:
	KToshibaSMMInterface *mDriver;
	KAboutApplication *mAboutWidget;
	KInstance *instance;
	DCOPClient mClient;
	SynapticsSHM *m_synShm;
	bool mFullBat;
	int mBatStatus;
	int mOldBatStatus;
	int mLowBat;
	int mCryBat;
	int mBatSave;
	int mOldBatSave;
	int mWirelessSwitch;
	int mOldWirelessSwitch;
	int mAudioPlayer;
	int mBatType;
private:
	void brightUp();
	void brightDown();
	void lockScreen();
	void mousePad();
	void setModel();
	void mute();
	void toggleWireless();
	void updateWidgetStatus(int);
	void performFnAction(int, int);
	void bsmUserSettings(KConfig *);
	void speakerVolume();
	void toggleFan();
	void checkSelectBay();
	int bayUnregister();
	int bayRescan();
	SettingsWidget *mSettingsWidget;
	StatusWidget *mStatusWidget;
	QTimer *mPowTimer;
	QTimer *mHotKeysTimer;
	QTimer *mModeTimer;
	QTimer *mSystemTimer;
	QPixmap pm;
	QString noBatteryIcon;
	QString noChargeIcon;
	QString chargeIcon;
	bool mInterfaceAvailable;
	bool mWire;
	bool changed;
	bool lowtrig;
	bool crytrig;
	bool battrig;
	bool wstrig;
	bool btstart;
	int pow;
	int oldpow;
	int time;
	int oldtime;
	int perc;
	int oldperc;
	int snd;
	int battery;
	int video;
	int mousepad;
	int bright;
	int current_code;
	int MODE;
	int popup;
	int bluetooth;
	int wireless;
	int fan;
	int vol;
	int sblock;
	int removed;
};

#endif // KTOSHIBA_H
