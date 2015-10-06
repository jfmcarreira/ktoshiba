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

#include "ui_statuswidget.h"

class QTimer;

class KToshibaDBusInterface;
class KToshibaKeyHandler;
class KToshibaHardware;

class FnActions : public QObject
{
    Q_OBJECT

public:
    enum WidgetIcons { Disabled, Performance, Powersave, Presentation, ECO, KBDStatus, Blank };
    enum Zoom { Reset, In, Out = -1 };
    enum ToogleActions { Off, On };

public:
    FnActions(QObject *parent);
    ~FnActions();
    bool init();

    void changeProfile(QString);
    QString getProfile();
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

private:
    void showWidget(int);
    void toggleProfiles();
    void toggleTouchPad();

    KToshibaDBusInterface *m_dBus;
    KToshibaKeyHandler *m_hotkeys;
    KToshibaHardware *m_hw;
    Ui::StatusWidget m_statusWidget;
    QWidget *m_widget;
    QTimer *m_widgetTimer;
    QStringList m_profiles;
    QList<int> m_modes;
    QString m_profile;
    QString m_version;
    bool m_batKeyPressed;
    bool m_batMonitor;
    uint m_cookie;
    int m_type;
    int m_mode;
    int m_time;
};

#endif // FN_ACTIONS_H
