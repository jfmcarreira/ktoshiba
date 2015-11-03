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
class KToshibaNetlinkEvents;
class KToshibaKeyHandler;
class KToshibaHardware;

class FnActions : public QObject
{
    Q_OBJECT

public:
    FnActions(QObject *parent);
    ~FnActions();
    bool init();

    enum ToogleActions { Off, On };
    enum BatteryProfiles { Performance, Powersave, Presentation, ECO };
    enum KeyboardBacklightGenerations { FirstGeneration = 1, SecondGeneration = 2 };

    KToshibaHardware *hw() const
    {
        return m_hw;
    }

Q_SIGNALS:
    void vibrationDetected();

private Q_SLOTS:
    bool checkConfig();
    void loadConfig();
    void createConfig();
    void hideWidget();
    void compositingChanged(bool);
    void processHotkey(int);
    void parseTVAPEvents(int);
    void protectHDD(int);
    void updateBatteryProfile();
    void updateCoolingMethod(QString);

private:
    void showWidget();
    void changeProfile(int, bool);
    void toggleProfiles();
    void toggleTouchPad();
    void updateKBDBacklight();
    bool isPointingDeviceSupported();
    bool isIlluminationSupported();
    bool isECOSupported();
    bool isKBDBacklightSupported();
    bool isCoolingMethodSupported();

    KSharedConfigPtr m_config;

    bool m_pointing;
    bool m_illumination;
    bool m_eco;
    bool m_kbdBacklight;
    bool m_cooling;
    int m_hdd;

    KToshibaDBusInterface *m_dBus;
    KToshibaNetlinkEvents *m_nl;
    KToshibaKeyHandler *m_hotkeys;
    KToshibaHardware *m_hw;
    Ui::StatusWidget m_statusWidget;
    QWidget *m_widget;
    QTimer *m_widgetTimer;
    QList<int> m_batteryProfiles;
    QList<int> m_keyboardModes;
    QString m_version;
    QString m_iconText;
    bool m_batteryKeyPressed;
    bool m_monitorBatteryProfiles;
    bool m_manageCoolingMethod;
    bool m_monitorHDD;
    bool m_notifyHDD;
    uint m_cookie;
    int m_keyboardType;
    int m_keyboardMode;
    int m_keyboardTime;
    int m_batteryProfile;
    int m_previousBatteryProfile;
    int m_maxCoolingMethod;
    int m_coolingMethodPluggedIn;
    int m_coolingMethodOnBattery;
};

#endif // FN_ACTIONS_H
