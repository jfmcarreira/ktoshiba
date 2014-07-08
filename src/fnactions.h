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
    enum WidgetIcons { Disabled, Performance, Powersave, Presentation, ECO, KBDAuto, Blank };
    enum zoomActions { ZoomReset = 0, ZoomIn = 1, ZoomOut = -1 };
    enum KbdBacklight { NotAvailable = 0, FNZMode = 1, AutoMode = 2 };
    enum toogleActions { Off = false, On = true };

public:
    FnActions(QObject *parent);
    ~FnActions();

    int m_mode;

public Q_SLOTS:
    void processHotkey(int);
    void acAdapterChanged(bool);
    void compositingChanged(bool);
    void toggleTouchPad();
    
private Q_SLOTS:
    void hideWidget();

private:
    void showWidget(int);
    void toggleProfiles();
    void changeProfile(QString);
    void kbdBacklight();
    void setKBDBacklight(bool);

    KToshibaDBusInterface *m_dBus;
    KToshibaKeyHandler *m_keyHandler;
    HelperActions *m_helper;
    Ui::StatusWidget m_statusWidget;
    QWidget *m_widget;
    QTimer *m_widgetTimer;
    QStringList m_profiles;
    QString m_profile;
    QString m_version;

    bool m_fnPressed;
    int m_cookie;
};

#endif // FN_ACTIONS_H
