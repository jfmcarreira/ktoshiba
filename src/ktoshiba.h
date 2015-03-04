/*
   Copyright (C) 2004-2015  Azael Avalos <coproscefalo@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, see
   <http://www.gnu.org/licenses/>.
*/

#ifndef KTOSHIBA_H
#define KTOSHIBA_H

#include <Phonon/MediaObject>

#include <KUniqueApplication>
#include <KSharedConfig>

#include "ui_kbdtimeoutwidget.h"

class QAction;
class QMenu;

class KMenu;
class KAboutData;
class KHelpMenu;
class KStatusNotifierItem;

class FnActions;
class KToshibaHDDProtect;

/**
 * @short Hotkeys monitoring for Toshiba laptops
 * @author Azael Avalos <coproscefalo@gmail.com>
 * @version 4.3
 */
class KToshiba : public KUniqueApplication
{
    Q_OBJECT

public:
    KToshiba();
    ~KToshiba();

    static void createAboutData();
    static void destroyAboutData();
    static KAboutData* aboutData();

private Q_SLOTS:
    void cleanup();
    void protectHDD(int);
    void notifyHDDMovement();
    void timerClicked();
    void fnzClicked();
    void onClicked();
    void offClicked();
    void notifyKBDModeChanged();
    void kbdTimeoutClicked();
    void changeKBDTimeout(QAbstractButton *);
    void timeChanged(int);
    void disabledClicked(bool);
    void performanceClicked();
    void powersaveClicked();
    void presentationClicked();
    void ecoClicked();

Q_SIGNALS:
    void batteryProfilesToggled(bool);

private:
    bool checkConfig();
    void loadConfig();
    void createConfig();
    void doMenu();

    FnActions *m_fn;
    KToshibaHDDProtect *m_hdd;
    QAction *m_hddMonitor;
    QAction *m_hddProtectionLvl;
    QAction *m_touchPad;
    QMenu *m_kbdModeMenu;
    QAction *m_kbdTimer;
    QAction *m_kbdFNZ;
    QAction *m_kbdOn;
    QAction *m_kbdOff;
    QAction *m_kbdTimeout;
    QMenu *m_batteryMenu;
    QAction *m_batDisabled;
    QAction *m_batPerformance;
    QAction *m_batPowersave;
    QAction *m_batPresentation;
    QAction *m_batECO;
    Ui::kbdTimeoutWidget m_kbdTimeoutWidget;
    QWidget *m_timeoutWidget;
    KSharedConfigPtr m_config;
    bool m_batteryProfiles;
    bool m_hddConnected;
    bool m_fnConnected;
    bool m_monitorHDD;
    bool m_notifyHDD;
    int m_level;

    KMenu *m_popupMenu;
    static KAboutData* m_about;
    KHelpMenu *m_helpMenu;
    KStatusNotifierItem *m_trayicon;
};

#endif // KTOSHIBA_H
