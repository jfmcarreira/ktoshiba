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

KToshibaProcInterface::KToshibaProcInterface(QObject *parent)
    : QObject( parent ),
      mFd( 0 ),
      BatteryCap( 0 )
{
}

bool KToshibaProcInterface::checkOmnibook()
{
    QString bios;

    QFile file(OMNI_ROOT"/dmi");
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
                continue;
            }
            if (line.contains("Product Name:", false)) {
                QRegExp rx("(Satellite)");
                rx.search(line);
                model = rx.cap(1);
                rx.setPattern("\\b([A-Z])(?:\\s*)(\\d+)$");
                rx.search(line);
                tmp = rx.cap(1) + rx.cap(2);
                if (tmp.isEmpty()) {
                    rx.setPattern("\\b(\\d+)$");
                    rx.search(line);
                    model += " " + rx.cap(1);
                } else
                    model += " " + rx.cap(1) + rx.cap(2);
                continue;
            }
        }
        file.close();
    }
    if (bios == "TOSHIBA")
        return true;

    return false;
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

    QFile file(OMNI_ROOT"/lcd");
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

    if ((mFd = open(OMNI_ROOT"/lcd", O_RDWR))) {
        kdError() << "KToshibaProcInterface::omnibookSetBrightness()"
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
    QFile file(OMNI_ROOT"/onetouch");
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
    if ((mFd = open(OMNI_ROOT"/onetouch", O_RDWR))) {
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
    QFile file(OMNI_ROOT"/fan");
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
    if ((mFd = open(OMNI_ROOT"/fan", O_RDWR))) {
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
    char buffer[64];

    if (!(str = fopen(TOSH_PROC, "r"))) {
        kdError() << "KToshibaProcInterface::toshibaProcStatus(): "
                  << "Could not open " << TOSH_PROC
                  << " for reading" << endl;
        return -1;
    }

    char *ret = fgets(buffer, sizeof(buffer) - 1, str);
    if (ret == NULL) {
        kdError() << "KToshibaProcInterface::toshibaProcStatus(): "
                  << "Could not read /proc value" << endl;
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
