/***************************************************************************
 *   Copyright (C) 2004-2006 by Azael Avalos                               *
 *   coproscefalo@gmail.com                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
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

#include "ktoshibaprocinterface.h"

#include <qstring.h>
#include <qfile.h>
#include <qregexp.h>

#include <kdebug.h>
#include <klocale.h>

KToshibaProcInterface::KToshibaProcInterface( QObject *parent )
    : QObject( parent ),
      mFd( 0 ),
      BatteryCap( 0 )
{
}

KToshibaProcInterface::~KToshibaProcInterface()
{
}

bool KToshibaProcInterface::checkOmnibook()
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

QString KToshibaProcInterface::omnibookModelName()
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
                // Some models doesn't show the _Satellite_ string, so lets deal with those known
                QRegExp rx("\\b([A-Z])(\\d+)$"); // S1110
                rx.search(line);
                if (rx.cap(1) == "S1110") {
                    model = "Satellite 1110";
                    ectype = XE3GF;
                    break;
                }
                if (model.isEmpty()) {
                    rx.setPattern("\\b([A-Z])(\\d+)(\\-)(\\d+)$"); // S3000-100
                    rx.search(line);
                    if (rx.cap(1) == "S3000-100") {
                        model = "Satellite 3000-100";
                        ectype = XE3GF;
                        break;
                    }
                }
                // Now lets deal with the ones that do show the _Satellite_ string
                rx.setPattern("(Satellite)");
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
                    if (tmp == "M40")
                        ectype = TSM40;
                    if (tmp == "M100")
                        ectype = TSM30X;
                    else
                        ectype = TSP10;
                }
                if (tmp.isEmpty()) {
                    rx.setPattern("\\b([A-Z])(\\d+)([A-Z])$"); // eg.: M30X
                    rx.search(line);
                    tmp = rx.cap(1) + rx.cap(2) + rx.cap(3);
                    ectype = TSM30X;
                }
                if (tmp.isEmpty()) {
                    model = i18n("UNKNOWN");
                    ectype = NONE;
                    break;
                }
                model += " " + tmp;
                break;
            }
        }
        file.close();
    }

    return model;
}

int KToshibaProcInterface::omnibookECType()
{
    return ectype;
}

void KToshibaProcInterface::omnibookBatteryStatus(int *time, int *percent)
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

int KToshibaProcInterface::omnibookAC()
{
    QFile file(OMNI_ROOT"/ac");
    if (!file.open(IO_ReadOnly)) {
        kdError() << "KToshibaProcInterface::omnibookAC(): "
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

int KToshibaProcInterface::omnibookGetBrightness()
{
    int brightness = 0;

    QFile file(OMNI_LCD);
    if (!file.open(IO_ReadOnly)) {
        kdError() << "KToshibaProcInterface::omnibookGetBrightness(): "
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

void KToshibaProcInterface::omnibookSetBrightness(int bright)
{
    if (bright < 0 || bright > 7)
        bright = ((bright < 0)? 0 : 7);

    if ((mFd = open(OMNI_LCD, O_RDWR)) == -1) {
        kdError() << "KToshibaProcInterface::omnibookSetBrightness(): "
                  << "Could not open: " << OMNI_ROOT << "/lcd" << endl;
        return;
    }

    if (write(mFd, "%d", bright) < 0) {
        kdError() << "KToshibaProcInterface::omnibookSetBrightness(): "
                  << "Failed setting brightness" << endl;
        close(mFd);
        return;
    }

    close(mFd);
}

int KToshibaProcInterface::omnibookGetOneTouch()
{
    QFile file(OMNI_ONETOUCH);
    if (!file.open(IO_ReadOnly)) {
        kdError() << "KToshibaProcInterface::omnibookGetOneTouch(): "
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

void KToshibaProcInterface::omnibookSetOneTouch(int state)
{
    if ((mFd = open(OMNI_ONETOUCH, O_RDWR)) == -1) {
        kdError() << "KToshibaProcInterface::omnibookSetFan()"
                  << "Could not open: " << OMNI_ROOT << "/onetouch" << endl;
        return;
    }

    if (write(mFd, "%d", state) < 0) {
        kdError() << "KToshibaProcInterface::omnibookSetOneTouch()"
                  << "Could not " << ((state == 0)? "disable" : "enable")
                  << " OneTouch buttons" << endl;
        close(mFd);
        return;
    }

    close(mFd);
}

int KToshibaProcInterface::omnibookGetFan()
{
    QFile file(OMNI_FAN);
    if (!file.open(IO_ReadOnly)) {
        kdError() << "KToshibaProcInterface::omnibookGetFan(): "
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

void KToshibaProcInterface::omnibookSetFan(int status)
{
    if ((mFd = open(OMNI_FAN, O_RDWR)) == -1) {
        kdError() << "KToshibaProcInterface::omnibookSetFan()"
                  << "Could not open: " << OMNI_ROOT << "/fan" << endl;
        return;
    }

    if (write(mFd, "%d", status) < 0) {
        kdError() << "KToshibaProcInterface::omnibookSetFan()"
                  << "Could not " << ((status == 0)? "disable" : "enable")
                  << " system fan" << endl;
        close(mFd);
        return;
    }

    close(mFd);
}

int KToshibaProcInterface::omnibookGetLCDBackLight()
{
    QFile file(OMNI_BLANK);
    if (!file.open(IO_ReadOnly)) {
        kdError() << "KToshibaProcInterface::omnibookGetLCDBacklight(): "
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

void KToshibaProcInterface::omnibookSetLCDBackLight(int status)
{
    if ((mFd = open(OMNI_BLANK, O_RDWR)) == -1) {
        kdError() << "KToshibaProcInterface::omnibookSetLCDBacklight()"
                  << "Could not open: " << OMNI_ROOT << "/blank" << endl;
        return;
    }

    if (write(mFd, "%d", status) < 0) {
        kdError() << "KToshibaProcInterface::omnibookSetLCDBacklight()"
                  << "Could not turn " << ((status == 0)? "off" : "on")
                  << " LCD backlight" << endl;
        close(mFd);
        return;
    }

    close(mFd);
}

int KToshibaProcInterface::omnibookGetTouchPad()
{
    QFile file(OMNI_TOUCHPAD);
    if (!file.open(IO_ReadOnly)) {
        kdError() << "KToshibaProcInterface::omnibookGetTouchPad(): "
                  << "Could not get TouchPad state or system doesn't"
                  << " support it" << endl;
        return -1;
    }

    // TODO: Find a model with touchpad and ask for data file
    /*QTextStream stream(&file);
    QString line = stream.readLine();
    if (line.contains("", false)) {
        QRegExp rx("(not)$");
        rx.search(line);
        if (rx.cap(1) == "not") {
            file.close();
            return 0;
        }
    }
    file.close();*/

    return 1;
}

void KToshibaProcInterface::omnibookSetTouchPad(int status)
{
    if ((mFd = open(OMNI_TOUCHPAD, O_RDWR)) == -1) {
        kdError() << "KToshibaProcInterface::omnibookSetTouchPad()"
                  << "Could not open: " << OMNI_ROOT << "/touchpad" << endl;
        return;
    }

    if (write(mFd, "%d", status) < 0) {
        kdError() << "KToshibaProcInterface::omnibookSetTouchPad()"
                  << "Could not " << ((status == 0)? "disable" : "enable")
                  << " TouchPad" << endl;
        close(mFd);
        return;
    }

    close(mFd);
}

int KToshibaProcInterface::omnibookGetVideo()
{
    QFile file(OMNI_ROOT"/display");
    if (!file.open(IO_ReadOnly)) {
        kdError() << "KToshibaProcInterface::omnibookGetVideo(): "
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

int KToshibaProcInterface::toshibaProcStatus()
{
    int key;
    char buffer[64], *ret;

    if (!(str = fopen(TOSH_PROC, "r"))) {
        kdError() << "KToshibaProcInterface::toshibaProcStatus(): "
                  << "Could not open " << TOSH_PROC
                  << " for reading" << endl;
        return -1;
    }

    if ((ret = fgets(buffer, sizeof(buffer) - 1, str)) == NULL) {
        kdError() << "KToshibaProcInterface::toshibaProcStatus(): "
                  << "Could not read " << TOSH_PROC << " value" << endl;
        return -1;
    }

    buffer[sizeof(buffer) - 1] = '\0';
    sscanf(buffer, "%*s %*x %*d.%*d %*d.%*d %*x %x\n", &key);
    fclose(str);

    return key;
}

void KToshibaProcInterface::acpiBatteryStatus(int *time, int *percent)
{
    int rate = 0;

    if (BatteryCap == 0) {
        QFile file(ACPI_ROOT"/battery/BAT1/info");
        if (file.open(IO_ReadOnly)) {
            QTextStream stream(&file);
            QString line;
            while (!stream.atEnd()) {
                line = stream.readLine();
                if (line.contains("last full capacity:", false)) {
                    QRegExp rx("(\\d*)\\D*$");
                    rx.search(line);
                    BatteryCap = rx.cap(1).toInt();
                    break;
                }
            }
            file.close();
        }
    }

    QFile file2(ACPI_ROOT"/battery/BAT1/state");
    if (!file2.exists()) {
        *percent = -1;
        *time = -1;
        return;
    }
    if (file2.open(IO_ReadOnly)) {
        QTextStream stream(&file2);
        QString line;
        while (!stream.atEnd()) {
            line = stream.readLine();
            if (line.contains("present rate:", false)) {
                QRegExp rx("(\\d*)\\D*$");
                rx.search(line);
                rate = rx.cap(1).toInt();
                continue;
            }
            if (line.contains("remaining capacity:", false)) {
                QRegExp rx("(\\d*)\\D*$");
                rx.search(line);
                RemainingCap = rx.cap(1).toInt();
                continue;
            }
        }
        file2.close();
    }

    *percent = (RemainingCap * 100) / BatteryCap;
    *time = (int)roundf(((float)(RemainingCap) / rate) * 60);
}

int KToshibaProcInterface::acpiAC()
{
    QFile file(ACPI_ROOT"/ac_adapter/ADP1/state");
    if (!file.open(IO_ReadOnly)) {
        kdError() << "KToshibaProcInterface::acpiAC(): "
                  << "Could not get AC Power status" << endl;
        return -1;
    }

    QTextStream stream(&file);
    QString line = stream.readLine();
    if (line.contains("Status:", false) || line.contains("state:", false)) {
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


#include "ktoshibaprocinterface.moc"
