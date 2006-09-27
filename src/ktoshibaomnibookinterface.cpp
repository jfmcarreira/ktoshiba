/***************************************************************************
 *   Copyright (C) 2006 by Azael Avalos                                    *
 *   coproscefalo@gmail.com                                                *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "ktoshibaomnibookinterface.h"

#include <qstring.h>
#include <qfile.h>
#include <qregexp.h>

#include <kdebug.h>
#include <klocale.h>

KToshibaOmnibookInterface::KToshibaOmnibookInterface( QObject *parent )
    : QObject( parent ),
      mFd( 0 ),
      BatteryCap( 0 ),
      RemainingCap( 0 )
{
}

KToshibaOmnibookInterface::~KToshibaOmnibookInterface()
{
}

bool KToshibaOmnibookInterface::checkOmnibook()
{
    QString bios;

    QFile file(OMNI_DMI);
    if (!file.exists())
        return false;
    if (file.open(IO_ReadOnly)) {
        QTextStream stream(&file);
        QString line, tmp;
        while (!stream.atEnd()) {
            line = stream.readLine();
            if(line.contains("BIOS Vendor:", false)) {
                QRegExp rx("(TOSHIBA)$");
                rx.search(line);
                bios = rx.cap(1);
                break;
            }
        }
        file.close();
        if (bios == "TOSHIBA")
            return true;
    }

    return false;
}

QString KToshibaOmnibookInterface::modelName()
{
    QFile file(OMNI_DMI);
    if (!file.exists()) {
        model = i18n("UNKNOWN");
        ectype = NONE;
        return model;
    }
    if (file.open(IO_ReadOnly)) {
        QTextStream stream(&file);
        QString line, tmp;
        while (!stream.atEnd()) {
            line = stream.readLine();
            if (line.contains("Product Name:", false)) {
                // Lets deal with the ones that do show the _Satellite_ or _Tecra_ string
                QRegExp rx("(Satellite|Tecra|TECRA)");
                rx.search(line);
                model = rx.cap(1);
                rx.setPattern("\\b(\\d+)$"); // eg.: 3005
                rx.search(line);
                tmp = rx.cap(1);
                ectype = XE3GF;
                if (tmp.isEmpty()) {
                    rx.setPattern("\\b([A-Z])(\\d+)$"); // eg.: P15
                    rx.search(line);
                    tmp = rx.cap(1) + rx.cap(2);
                    // Some models doesn't show the _Satellite_ string, so lets deal with those known
                    if ((tmp == "S1000" || tmp == "S1005" || tmp == "S1110" || tmp == "S1115" ||
                         tmp == "S1900" || tmp == "S1905" || tmp == "S1950" || tmp == "S1955" ||
                         tmp == "S2430" || tmp == "S2435" || tmp == "S3000" || tmp == "S3005") && model.isEmpty()) {
                        model = "Satellite";
                        ectype = XE3GF;
                        break;
                    } else
                    if (tmp == "P10" || tmp == "P15" || tmp == "P20") {
                        ectype = TSP10;
                        break;
                    } else
                    if (tmp == "A70" || tmp == "M70" || tmp == "M100" || tmp == "S2") {
                        ectype = TSM30X;
                        break;
                    } else
                    if (tmp == "M40" || tmp == "M45" || tmp == "S1") {
                        ectype = TSM40;
                        break;
                    } else
                    if (tmp == "A105") {
                        ectype = TSA105;
                        break;
                    }
                }
                if (tmp.isEmpty()) {
                    rx.setPattern("\\b([A-Z])(\\d+)([A-Z])$"); // eg.: M30X
                    rx.search(line);
                    tmp = rx.cap(1) + rx.cap(2) + rx.cap(3);
                    ectype = TSM30X;
                    break;
                }
                if (tmp.isEmpty()) {
                    rx.setPattern("\\b([A-Z])(\\d+)(\\-)(\\d+)$"); // S3000-100
                    rx.search(line);
                    tmp = rx.cap(1) + rx.cap(2) + rx.cap(3) + rx.cap(4);
                    if ((tmp == "S1700-100" || tmp == "S1700-200" || tmp == "S1700-300" ||
                         tmp == "S1700-400" || tmp == "S1700-500") && model.isEmpty())
                        ectype = AMILOD;
                    else if ((tmp == "S3000-100") && model.isEmpty())
                        ectype = XE3GF;
                    model = "Satellite";
                    break;
                }
                if (tmp.isEmpty()) {
                    model = i18n("UNKNOWN");
                    ectype = NONE;
                    break;
                }
            }
        }
        file.close();
        model += " " + tmp;
    }

    return model;
}

int KToshibaOmnibookInterface::ecType()
{
    return ectype;
}

void KToshibaOmnibookInterface::batteryStatus(int *time, int *percent)
{
    QFile file(OMNI_ROOT"/battery");
    if (!file.exists()) {
        *percent = -1;
        *time = -1;
        return;
    }
    if (file.open(IO_ReadOnly)) {
            QTextStream stream(&file);
            QString line;
            while (!stream.atEnd()) {
                line = stream.readLine();
                if (line.contains("Remaining Capacity:", false)) {
                    QRegExp rx("(\\d*)\\D*$");
                    rx.search(line);
                    RemainingCap = rx.cap(1).toInt();
                    continue;
                }
                if (line.contains("Last Full Capacity:", false) && BatteryCap == 0) {
                    QRegExp rx("(\\d*)\\D*$");
                    rx.search(line);
                    BatteryCap = rx.cap(1).toInt();
                    continue;
                }
                if (line.contains("Gauge:", false)) {
                    QRegExp rx("(\\d*)\\D*$");
                    rx.search(line);
                    *percent = rx.cap(1).toInt();
                    continue;
                }
            }
            file.close();
        }

    *time = (BatteryCap * 60) / RemainingCap;
}

int KToshibaOmnibookInterface::omnibookAC()
{
    QFile file(OMNI_ROOT"/ac");
    if (!file.open(IO_ReadOnly)) {
        kdError() << "KToshibaOmnibookInterface::omnibookAC(): "
                  << "Could not get AC Power status" << endl;
        return -1;
    }

    QTextStream stream(&file);
    QString line = stream.readLine();
    if (line.contains("AC", false)) {
        QRegExp rx("(on-line)$");
        rx.search(line);
        if (rx.cap(1) == "on-line") {
            file.close();
            return 4;
        }
    }
    file.close();

    return 3;
}

int KToshibaOmnibookInterface::getBrightness()
{
    int brightness = 0;

    QFile file(OMNI_LCD);
    if (!file.open(IO_ReadOnly)) {
        kdError() << "KToshibaOmnibookInterface::getBrightness(): "
                  << "Failed obtaining brightness" << endl;
        return -1;
    }

    QTextStream stream(&file);
    QString line = stream.readLine();
    if (line.contains("LCD brightness:", false)) {
        QRegExp rx("(\\d*)\\D*$");
        rx.search(line);
        brightness = rx.cap(1).toInt();
    }
    file.close();

    return brightness;
}

void KToshibaOmnibookInterface::setBrightness(int bright)
{
    if (bright < 0 || bright > 7)
        bright = ((bright < 0)? 0 : 7);

    if ((mFd = open(OMNI_LCD, O_RDWR)) == -1) {
        kdError() << "KToshibaOmnibookInterface::setBrightness(): "
                  << "Could not open: " << OMNI_ROOT << "/lcd" << endl;
        return;
    }

    if (write(mFd, "%d", bright) < 0) {
        kdError() << "KToshibaOmnibookInterface::setBrightness(): "
                  << "Failed setting brightness" << endl;
        close(mFd);
        return;
    }

    close(mFd);
}

int KToshibaOmnibookInterface::getOneTouch()
{
    QFile file(OMNI_ONETOUCH);
    if (!file.open(IO_ReadOnly)) {
        kdError() << "KToshibaOmnibookInterface::getOneTouch(): "
                  << "Failed obtaining OneTouch buttons status" << endl;
        return -1;
    }

    QTextStream stream(&file);
    QString line = stream.readLine();
    if (line.contains("OneTouch buttons are", false)) {
        QRegExp rx("(enabled)$");
        rx.search(line);
        if (rx.cap(1) == "enabled") {
            file.close();
            return 1;
        }
    }
    file.close();

    return 0;
}

void KToshibaOmnibookInterface::setOneTouch(int state)
{
    if ((mFd = open(OMNI_ONETOUCH, O_RDWR)) == -1) {
        kdError() << "KToshibaOmnibookInterface::setFan()"
                  << "Could not open: " << OMNI_ROOT << "/onetouch" << endl;
        return;
    }

    if (write(mFd, "%d", state) < 0) {
        kdError() << "KToshibaOmnibookInterface::setOneTouch()"
                  << "Could not " << ((state == 0)? "disable" : "enable")
                  << " OneTouch buttons" << endl;
        close(mFd);
        return;
    }

    close(mFd);
}

int KToshibaOmnibookInterface::getFan()
{
    QFile file(OMNI_FAN);
    if (!file.open(IO_ReadOnly)) {
        kdError() << "KToshibaOmnibookInterface::getFan(): "
                  << "Could not get fan status" << endl;
        return -1;
    }

    QTextStream stream(&file);
    QString line = stream.readLine();
    if (line.contains("Fan is", false)) {
        QRegExp rx("(on)$");
        rx.search(line);
        if (rx.cap(1) == "on") {
            file.close();
            return 1;
        }
    }
    file.close();

    return 0;
}

void KToshibaOmnibookInterface::setFan(int status)
{
    if ((mFd = open(OMNI_FAN, O_RDWR)) == -1) {
        kdError() << "KToshibaOmnibookInterface::setFan()"
                  << "Could not open: " << OMNI_ROOT << "/fan" << endl;
        return;
    }

    if (write(mFd, "%d", status) < 0) {
        kdError() << "KToshibaOmnibookInterface::setFan()"
                  << "Could not " << ((status == 0)? "disable" : "enable")
                  << " system fan" << endl;
        close(mFd);
        return;
    }

    close(mFd);
}

int KToshibaOmnibookInterface::getLCDBackLight()
{
    QFile file(OMNI_BLANK);
    if (!file.open(IO_ReadOnly)) {
        kdError() << "KToshibaOmnibookInterface::getLCDBacklight(): "
                  << "Could not get LCD Backlight status" << endl;
        return -1;
    }

    QTextStream stream(&file);
    QString line = stream.readLine();
    if (line.contains("LCD console blanking is", false)) {
        QRegExp rx("(enabled)$");
        rx.search(line);
        if (rx.cap(1) == "enabled") {
            file.close();
            return 1;
        }
    }
    file.close();

    return 0;
}

void KToshibaOmnibookInterface::setLCDBackLight(int status)
{
    if ((mFd = open(OMNI_BLANK, O_RDWR)) == -1) {
        kdError() << "KToshibaOmnibookInterface::setLCDBacklight()"
                  << "Could not open: " << OMNI_ROOT << "/blank" << endl;
        return;
    }

    if (write(mFd, "%d", status) < 0) {
        kdError() << "KToshibaOmnibookInterface::setLCDBacklight()"
                  << "Could not turn " << ((status == 0)? "off" : "on")
                  << " LCD backlight" << endl;
        close(mFd);
        return;
    }

    close(mFd);
}

int KToshibaOmnibookInterface::getTouchPad()
{
    QFile file(OMNI_TOUCHPAD);
    if (!file.open(IO_ReadOnly)) {
        kdError() << "KToshibaOmnibookInterface::getTouchPad(): "
                  << "Could not get TouchPad state or system doesn't"
                  << " support it" << endl;
        return -1;
    }

    QTextStream stream(&file);
    QString line = stream.readLine();
    if (line.contains("Touchpad is", false)) {
        QRegExp rx("(disabled)$");
        rx.search(line);
        if (rx.cap(1) == "disabled") {
            file.close();
            return 0;
        }
    }
    file.close();

    return 1;
}

void KToshibaOmnibookInterface::setTouchPad(int status)
{
    if ((mFd = open(OMNI_TOUCHPAD, O_RDWR)) == -1) {
        kdError() << "KToshibaOmnibookInterface::setTouchPad()"
                  << "Could not open: " << OMNI_ROOT << "/touchpad" << endl;
        return;
    }

    if (write(mFd, "%d", status) < 0) {
        kdError() << "KToshibaOmnibookInterface::setTouchPad()"
                  << "Could not " << ((status == 0)? "disable" : "enable")
                  << " TouchPad" << endl;
        close(mFd);
        return;
    }

    close(mFd);
}

int KToshibaOmnibookInterface::getWifiSwitch()
{
    QFile file(OMNI_WIFI);
    if (!file.open(IO_ReadOnly)) {
        kdError() << "KToshibaOmnibookInterface::getWifi(): "
                  << "Could not get WiFi adapter state or system doesn't"
                  << " support it" << endl;
        return -1;
    }

    QTextStream stream(&file);
    QString line = stream.readLine();
    if (line.contains("Wifi Kill switch is", false)) {
        QRegExp rx("(off)$");
        rx.search(line);
        if (rx.cap(1) == "off") {
            file.close();
            return 0;
        }
    }
    file.close();

    return 1;
}

int KToshibaOmnibookInterface::getWifi()
{
    QFile file(OMNI_WIFI);
    if (!file.open(IO_ReadOnly)) {
        kdError() << "KToshibaOmnibookInterface::getWifi(): "
                  << "Could not get WiFi adapter state or system doesn't"
                  << " support it" << endl;
        return -1;
    }

    QTextStream stream(&file);
    QString line = stream.readLine();
    if (line.contains("Wifi adapter is present and", false)) {
        QRegExp rx("(disabled)$");
        rx.search(line);
        if (rx.cap(1) == "disabled") {
            file.close();
            return 0;
        }
    }
    file.close();

    return 1;
}

void KToshibaOmnibookInterface::setWifi(int status)
{
    if ((mFd = open(OMNI_WIFI, O_RDWR)) == -1) {
        kdError() << "KToshibaOmnibookInterface::setWifi()"
                  << "Could not open: " << OMNI_ROOT << "/wifi" << endl;
        return;
    }

    if (write(mFd, "%d", status) < 0) {
        kdError() << "KToshibaOmnibookInterface::setWifi()"
                  << "Could not " << ((status == 0)? "disable" : "enable")
                  << " WiFi adapter" << endl;
        close(mFd);
        return;
    }

    close(mFd);
}

int KToshibaOmnibookInterface::getBluetooth()
{
    QFile file(OMNI_BLUETOOTH);
    if (!file.open(IO_ReadOnly)) {
        kdError() << "KToshibaOmnibookInterface::getBluetooth(): "
                  << "Could not get Bluetooth adapter state or system doesn't"
                  << " support it" << endl;
        return -1;
    }

    QTextStream stream(&file);
    QString line = stream.readLine();
    if (line.contains("Bluetooth adapter is present and", false)) {
        QRegExp rx("(disabled)$");
        rx.search(line);
        if (rx.cap(1) == "disabled") {
            file.close();
            return 0;
        }
    }
    file.close();

    return 1;
}

void KToshibaOmnibookInterface::setBluetooth(int status)
{
    if ((mFd = open(OMNI_BLUETOOTH, O_RDWR)) == -1) {
        kdError() << "KToshibaOmnibookInterface::setBluetooth()"
                  << "Could not open: " << OMNI_ROOT << "/bluetooth" << endl;
        return;
    }

    if (write(mFd, "%d", status) < 0) {
        kdError() << "KToshibaOmnibookInterface::setBluetooth()"
                  << "Could not " << ((status == 0)? "disable" : "enable")
                  << " Bluetooth adapter" << endl;
        close(mFd);
        return;
    }

    close(mFd);
}

int KToshibaOmnibookInterface::getVideo()
{
    QFile file(OMNI_ROOT"/display");
    if (!file.open(IO_ReadOnly)) {
        kdError() << "KToshibaOmnibookInterface::getVideo(): "
                  << "Could not get video state" << endl;
        return -1;
    }

    QTextStream stream(&file);
    QString line = stream.readLine();
    if (line.contains("External display is", false)) {
        QRegExp rx("(not)$");
        rx.search(line);
        if (rx.cap(1) == "not") {
            file.close();
            return 0;
        }
    }
    file.close();

    return 1;
}


#include "ktoshibaomnibookinterface.moc"
