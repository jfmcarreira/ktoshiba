/*
   Copyright (C) 2004-2009  Azael Avalos <coproscefalo@gmail.com>

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

#include <QObject>

#include <KUniqueApplication>
#include <KSharedConfig>

class QAction;

class KMenu;
class KAboutData;
class KStatusNotifierItem;

class FnActions;


/**
 * @short Hotkeys monitoring for Toshiba laptops
 * @author Azael Avalos <coproscefalo@gmail.com>
 * @version 0.3.90
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

Q_SIGNALS:
    void mediaPlayerChanged(int);

private Q_SLOTS:
    void autostartSlot(bool);
    void mediaPlayerSlot(QAction*);
    void updateMediaPlayer(int);

private:
    bool checkConfig();
    void loadConfig();
    void createConfig();

    FnActions *m_Fn;
    KMenu *mediaPlayerMenu;
    QAction *autostart;
    QAction *amarok;
    QAction *kaffeine;
    QAction *juk;
    KSharedConfigPtr config;
    bool m_autoStart;

    static KAboutData* m_about;
    KStatusNotifierItem *m_trayicon;
};

#endif // KTOSHIBA_H
