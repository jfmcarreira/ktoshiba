/*
   Copyright (C) 2004-2011  Azael Avalos <coproscefalo@gmail.com>

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

#ifndef FN_ACTIONS_H
#define FN_ACTIONS_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QKeyEvent>

#include "ui_statuswidget.h"
#include "ktoshibakeyhandler.h"

class KToshibaDBusInterface;
class KToshibaKeyHandler;

class FnActions : public QObject
{
    Q_OBJECT

public:
    enum WidgetIcons { Disabled, TPOn, TPOff, WifiOn, WifiOff, Performance, Powersave, Presentation, ECO, KBDOff, KBDOn, KBDAuto, Blank };
    enum zoomActions { ZoomReset = 0, ZoomIn = 1, ZoomOut = -1 };

public:
    FnActions(QObject *parent);
    ~FnActions();

private Q_SLOTS:
    void slotGotHotkey(int);
    void wirelessChanged(bool);

private:
    void showWidget(int);
    void toggleProfiles();
    void brightness(int);
    void toggleTouchPad();
    void toggleKBDIllumination();
    void toggleIllumination(bool);
    void toggleEcoLed(bool);

    KToshibaDBusInterface *m_dBus;
    KToshibaKeyHandler *m_keyHandler;
    Ui::StatusWidget m_statusWidget;
    QWidget *widget;
    QStringList profiles;
    QString m_profile;

    bool m_wireless;
    bool m_eco;
    bool m_fnPressed;
    int m_cookie;
};

#endif // FN_ACTIONS_H
