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
#include <KConfigGroup>

#include "ui_statuswidget.h"

class QWidget;

class KToshibaDBusInterface;
class KToshibaNetlinkEvents;
class KToshibaHardware;

class FnActions : public QObject
{
    Q_OBJECT

public:
    explicit FnActions(QObject *parent);
    ~FnActions();
    bool init();

    enum BatteryProfiles { Performance, Powersave, Presentation, ECO };
    Q_ENUM(BatteryProfiles)
    enum ACAdapterState { Disconnected, Connected };
    enum KeyboardBacklightGenerations { FirstGeneration = 1, SecondGeneration = 2 };

Q_SIGNALS:
    void vibrationDetected();

private Q_SLOTS:
    void reloadConfig();
    void hideWidget();
    void compositingChanged(bool);
    void processHotkey(int);
    void parseTVAPEvents(int, int);
    void parseExtraTVAPEvents(int);
    void parseHAPSEvents(int);
    void updateBatteryProfile(int);

private:
    bool checkConfig();
    void loadConfig();
    void createConfig();
    void showWidget();
    void showWidgetTimer();
    void changeBatteryProfile(int, bool);
    void toggleBatteryProfiles();
    void toggleTouchPad();
    void updateKBDBacklight();
    void toggleKBDBacklight();
    bool isPointingDeviceSupported();
    bool isKBDBacklightSupported();
    bool isCoolingMethodSupported();
    bool isODDPowerSupported();
    bool isIlluminationSupported();
    bool isECOSupported();
    bool isKeyboardFunctionsSupported();

    KToshibaDBusInterface *m_dBus;
    KToshibaNetlinkEvents *m_nl;
    KToshibaHardware *m_hw;

    KSharedConfigPtr m_config;
    KConfigGroup powersave;
    KConfigGroup hdd;

    Ui::StatusWidget m_statusWidget;
    QWidget *m_widget;
    QString m_iconText;

    bool m_pointingSupported;
    int m_pointing;

    QList<int> m_keyboardModes;
    bool m_kbdBacklightSupported;
    int m_keyboardType;
    int m_keyboardMode;
    int m_keyboardTime;

    QList<int> m_batteryProfiles;
    bool m_coolingMethodSupported;
    bool m_oddPowerSupported;
    bool m_illuminationSupported;
    bool m_ecoSupported;
    uint m_cookie;
    int m_batteryProfile;
    int m_previousBatteryProfile;
    int m_cooling;
    int m_odd;
    int m_illumination;

    bool m_monitorHDD;
    bool m_notifyHDD;
    int m_protectionLevel;
    int m_hdd;

    bool m_keyboardFunctionsSupported;
    int m_kbdFunctions;
};

#endif // FN_ACTIONS_H
