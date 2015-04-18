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

#ifndef KTOSHIBA_DBUS_INTERFACE_H
#define KTOSHIBA_DBUS_INTERFACE_H

#include <QObject>
#include <QtDBus/QDBusAbstractAdaptor>

class FnActions;

class KToshibaDBusInterface : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("KToshiba D-Bus Interface", "net.sourceforge.KToshiba")

public:
    KToshibaDBusInterface(FnActions *parent);
    ~KToshibaDBusInterface();
    void init();

    void lockScreen();
    void setBrightness(int);
    void setKBDBacklight(int);
    void setZoom(int);

Q_SIGNALS:
    void configChanged();

public Q_SLOTS:
    Q_NOREPLY void configFileChanged();
    int getTouchPad();
    void setTouchPad(int);
    int getECOLed();
    void setECOLed(int);
    int getIllumination();
    void setIllumination(int);
    int getKBDType();
    int getKBDMode();
    void setKBDMode(int);
    int getKBDTimeout();
    void setKBDTimeout(int);
    int getUSBSleepCharge();
    void setUSBSleepCharge(int);
    int getUSBSleepFunctionsBatState();
    void setUSBSleepFunctionsBatState(int);
    int getUSBSleepFunctionsBatLvl();
    void setUSBSleepFunctionsBatLvl(int);
    int getUSBRapidCharge();
    void setUSBRapidCharge(int);
    int getUSBSleepMusic();
    void setUSBSleepMusic(int);
    int getKBDFunctions();
    void setKBDFunctions(int);
    int getPanelPowerON();
    void setPanelPowerON(int);
    int getUSBThree();
    void setUSBThree(int);
    int getProtectionLevel();
    void setProtectionLevel(int);

private:
    FnActions *m_fn;

    bool m_service;
    bool m_object;
};

#endif	// KTOSHIBA_DBUS_INTERFACE_H
