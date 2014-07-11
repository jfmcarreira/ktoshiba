/*
   Copyright (C) 2004-2014  Azael Avalos <coproscefalo@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KTOSHIBA_H
#define KTOSHIBA_H

#include <KUniqueApplication>
#include <KSharedConfig>

#include "ui_kbdtimeoutwidget.h"

class QAction;

class KAboutData;
class KHelpMenu;
class KStatusNotifierItem;

class FnActions;
class HelperActions;

/**
 * @short Hotkeys monitoring for Toshiba laptops
 * @author Azael Avalos <coproscefalo@gmail.com>
 * @version 0.4.0
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
    void autostartClicked(bool);
    void changeKBDMode();
    void changeKBDModeText(int);
    void notifyKBDModeChanged();
    void kbdTimeoutClicked();
    void changeKBDTimeout(QAbstractButton *);
    void timeChanged(int);

private:
    bool checkConfig();
    void loadConfig();
    void createConfig();

    FnActions *m_fn;
    HelperActions *m_helper;
    QAction *m_autoStart;
    QAction *m_touchPad;
    QAction *m_kbdTimeout;
    QAction *m_kbdMode;
    Ui::kbdTimeoutWidget m_kbdTimeoutWidget;
    QWidget *m_widget;
    KSharedConfigPtr m_config;
    QString m_modeText;
    bool m_autostart;
    int m_time;

    static KAboutData* m_about;
    KHelpMenu *m_helpMenu;
    KStatusNotifierItem *m_trayicon;
};

#endif // KTOSHIBA_H
