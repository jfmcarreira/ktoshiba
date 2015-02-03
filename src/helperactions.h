/*
   Copyright (C) 2014-2015  Azael Avalos <coproscefalo@gmail.com>

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

#ifndef HELPERACTIONS_H
#define HELPERACTIONS_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QFile>

class HelperActions : public QObject
{
    Q_OBJECT

public:
    HelperActions(QObject *parent = 0);

    bool isTouchPadSupported;
    bool isIlluminationSupported;
    bool isECOSupported;
    bool isKBDBacklightSupported;
    bool isKBDTypeSupported;
    bool isHAPSSupported;

public Q_SLOTS:
    bool getTouchPad();
    void setTouchPad(bool);
    bool getIllumination();
    void setIllumination(bool);
    bool getEcoLed();
    void setEcoLed(bool);
    int getKBDType();
    int getKBDMode();
    void setKBDMode(int);
    int getKBDTimeout();
    void setKBDTimeout(int);
    int getProtectionLevel();
    void setProtectionLevel(int);
    void unloadHeads(int);

Q_SIGNALS:
    void kbdModeChanged();
    void touchpadToggled(bool);

private:
    QString findDriverPath();
    bool deviceExists(QString);
    bool checkTouchPad();
    bool checkIllumination();
    bool checkECO();
    bool checkKBDBacklight();
    bool checkKBDType();
    bool checkHAPS();

    QString m_driverPath;
    QString m_ledsPath;
    QString m_hapsPath;
    QFile m_file;
};

#endif // HELPERACTIONS_H
