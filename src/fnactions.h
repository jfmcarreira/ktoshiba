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

class QTimer;
class QWidget;

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

    enum BatteryProfiles { Performance, Powersave, Presentation, ECO };
    Q_ENUM(BatteryProfiles)
    enum KeyboardBacklightGenerations { FirstGeneration = 1, SecondGeneration = 2 };

    KToshibaHardware *hw() const
    {
        return m_hw;
    }

Q_SIGNALS:
    void vibrationDetected();

private Q_SLOTS:
    void reloadConfig();
    void hideWidget();
    void compositingChanged(bool);
    void processHotkey(int);
    void parseTVAPEvents(int);
    void protectHDD(int);
    void updateBatteryProfile(QString);

private:
    bool checkConfig();
    void loadConfig();
    void createConfig();
    void showWidget();
    void changeProfile(int, bool);
    void toggleProfiles();
    BatteryProfiles toBatteryProfiles(int);
    void toggleTouchPad();
    void updateKBDBacklight();
    bool isPointingDeviceSupported();
    bool isKBDBacklightSupported();
    bool isSATAInterfaceSupported();
    bool isCoolingMethodSupported();
    bool isODDPowerSupported();
    bool isIlluminationSupported();
    bool isECOSupported();

    KToshibaDBusInterface *m_dBus;
    KToshibaNetlinkEvents *m_nl;
    KToshibaKeyHandler *m_hotkeys;
    KToshibaHardware *m_hw;
    Ui::StatusWidget m_statusWidget;
    KSharedConfigPtr m_config;
    KConfigGroup powersave;
    KConfigGroup hdd;

    QWidget *m_widget;
    QTimer *m_widgetTimer;
    QString m_iconText;

    bool m_pointingSupported;
    int m_pointing;

    QList<int> m_keyboardModes;
    bool m_kbdBacklightSupported;
    int m_keyboardType;
    int m_keyboardMode;
    int m_keyboardTime;

    QList<int> m_batteryProfiles;
    bool m_sataInterfaceSupported;
    bool m_coolingMethodSupported;
    bool m_oddPowerSupported;
    bool m_illuminationSupported;
    bool m_ecoSupported;
    int m_batteryProfile;
    int m_previousBatteryProfile;
    int m_sata;
    int m_cooling;
    int m_odd;
    int m_illumination;

    bool m_monitorHDD;
    bool m_notifyHDD;
    uint m_cookie;
    int m_hdd;
};

#endif // FN_ACTIONS_H
