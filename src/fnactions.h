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

#ifndef FN_ACTIONS_H
#define FN_ACTIONS_H

#include <QObject>
#include <QString>
#include <QStringList>

#include <KSharedConfig>

#include "ui_statuswidget.h"

class QTimer;

class KToshibaDBusInterface;
class KToshibaKeyHandler;
class KToshibaHardware;

class FnActions : public QObject
{
    Q_OBJECT

public:
    enum WidgetIcons { Disabled, BatteryProfile, KBDStatus };
    enum Zoom { Reset, In, Out = -1 };
    enum ToogleActions { Off, On };
    enum BatteryProfiles { Performance, Powersave, Presentation, ECO };

public:
    FnActions(QObject *parent);
    ~FnActions();
    bool init();

    void updateKBDBacklight();

    KToshibaHardware *hw() const
    {
        return m_hw;
    }

public Q_SLOTS:
    void batMonitorChanged(bool);

private Q_SLOTS:
    void hideWidget();
    void processHotkey(int);
    void compositingChanged(bool);
    void updateBatteryProfile(bool init = false);
    void updateCoolingMethod(QString);

private:
    void showWidget(int);
    void changeProfile(int, bool);
    void toggleProfiles();
    void toggleTouchPad();
    bool isTouchPadSupported();
    bool isIlluminationSupported();
    bool isECOSupported();
    bool isKBDBacklightSupported();
    bool isCoolingMethodSupported();

    KSharedConfigPtr m_config;

    bool m_touchpad;
    bool m_illumination;
    bool m_eco;
    bool m_kbdBacklight;
    bool m_batMonitor;
    bool m_cooling;

    KToshibaDBusInterface *m_dBus;
    KToshibaKeyHandler *m_hotkeys;
    KToshibaHardware *m_hw;
    Ui::StatusWidget m_statusWidget;
    QWidget *m_widget;
    QTimer *m_widgetTimer;
    QList<int> m_profiles;
    QList<int> m_keyboardModes;
    QString m_version;
    bool m_batteryKeyPressed;
    bool m_manageCoolingMethod;
    uint m_cookie;
    int m_type;
    int m_mode;
    int m_time;
    int m_batteryProfile;
    int m_coolingMethod;
    int m_maxCoolingMethod;
};

#endif // FN_ACTIONS_H
