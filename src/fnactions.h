/*
   Copyright (C) 2004-2015  Azael Avalos <coproscefalo@gmail.com>

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

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "helperactions.h"
#include "ui_statuswidget.h"

class QTimer;

class KToshibaDBusInterface;
class KToshibaKeyHandler;
class HelperActions;

class FnActions : public QObject
{
    Q_OBJECT

public:
    enum WidgetIcons { Disabled, Performance, Powersave, Presentation, ECO, KBDStatus, Blank };
    enum Zoom { Reset = 0, In = 1, Out = -1 };
    enum KbdBacklight { FNZ = 1, TIMER = 2, ON = 8, OFF = 16 };
    enum ToogleActions { Off = false, On = true };

public:
    FnActions(QObject *parent);
    ~FnActions();
    void changeProfile(QString);
    bool hapsIsSupported();
    int protectionLevel();
    void setProtectionLevel(int);
    bool touchpadIsSupported();
    bool kbdBacklightIsSupported();
    int kbdBacklightMode();
    void setKbdBacklightMode(int);
    int kbdBacklightTimeout();
    void setKbdBacklightTimeout(int);

    bool m_batMonitor;
    int m_type;
    int m_mode;
    int m_time;

public Q_SLOTS:
    void processHotkey(int);
    void acAdapterChanged(bool);
    void compositingChanged(bool);
    void toggleTouchPad();
    void batMonitorChanged(bool);
    
private Q_SLOTS:
    void hideWidget();
    void kbdBacklightModeChanged();

Q_SIGNALS:
    void kbdModeChanged();

private:
    void showWidget(int);
    void toggleProfiles();
    void kbdBacklight();

    KToshibaDBusInterface *m_dBus;
    KToshibaKeyHandler *m_keyHandler;
    HelperActions *m_helper;
    Ui::StatusWidget m_statusWidget;
    QWidget *m_widget;
    QTimer *m_widgetTimer;
    QStringList m_profiles;
    QList<int> m_modes;
    QString m_profile;
    QString m_version;

    bool m_fnPressed;
    bool m_batKeyPressed;
    int m_cookie;
};

#endif // FN_ACTIONS_H
