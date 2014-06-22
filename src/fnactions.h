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

#ifndef FN_ACTIONS_H
#define FN_ACTIONS_H

#include <QObject>
#include <QString>
#include <QStringList>

#include "ui_statuswidget.h"

class KToshibaDBusInterface;
class KToshibaKeyHandler;

class FnActions : public QObject
{
    Q_OBJECT

public:
    enum WidgetIcons { Disabled, TPOn, TPOff, WifiOn, WifiOff, Performance, Powersave, Presentation, ECO, KBDAuto, Blank };
    enum zoomActions { ZoomReset = 0, ZoomIn = 1, ZoomOut = -1 };
    enum KbdBacklight { FNZMode = 1, AutoMode = 2 };
    enum toogleActions { Off = false, On = true };

public:
    FnActions(QObject *parent);
    ~FnActions();

    int getKBDMode();
    void changeKBDMode();
    int getKBDTimeout();
    void changeKBDTimeout(int);

private Q_SLOTS:
    void slotGotHotkey(int);
    void toggleTouchPad();
    void acAdapterChanged(bool);
    void compositingChanged(bool);

private:
    QString findDevicePath();
    void showWidget(int);
    void toggleProfiles();
    void changeProfile(QString);
    void screenBrightness(int);
    void toggleIllumination(bool);
    void toggleEcoLed(bool);
    void kbdBacklight();
    void toggleKBDBacklight(bool);

    KToshibaDBusInterface *m_dBus;
    KToshibaKeyHandler *m_keyHandler;
    Ui::StatusWidget m_statusWidget;
    QWidget *m_widget;
    QStringList profiles;
    QString m_profile;
    QString m_device;
    QString m_version;

    bool m_fnPressed;
    int m_cookie;
};

#endif // FN_ACTIONS_H
