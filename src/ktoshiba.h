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

#include <KStatusNotifierItem>
#include <KSharedConfig>

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
class KToshiba : public KStatusNotifierItem
{
    Q_OBJECT

public:
    KToshiba();
    virtual ~KToshiba();

    bool initialize();
    static void createAboutData();
    static void destroyAboutData();
    static KAboutData *aboutData();

private Q_SLOTS:
    void protectHDD(int);
    void notifyHDDMovement();
    void disabledClicked(bool);
    void performanceClicked();
    void powersaveClicked();
    void presentationClicked();
    void ecoClicked();
    void configureClicked();

Q_SIGNALS:
    void batteryProfilesToggled(bool);

private:
    void cleanup();
    bool checkConfig();
    void loadConfig();
    void createConfig();
    void doMenu();

    FnActions *m_fn;
    bool m_fnConnected;

    KToshibaHDDProtect *m_hdd;
    bool m_hddConnected;
    bool m_monitorHDD;
    bool m_notifyHDD;

    QMenu *m_batteryMenu;
    QAction *m_batDisabled;
    QAction *m_batPerformance;
    QAction *m_batPowersave;
    QAction *m_batPresentation;
    QAction *m_batECO;
    bool m_batteryProfiles;

    QAction *m_configure;

    KSharedConfigPtr m_config;

    KMenu *m_popupMenu;
    static KAboutData* m_about;
    KHelpMenu *m_helpMenu;
};

#endif // KTOSHIBA_H
