/***************************************************************************
 *   Copyright (C) 2005 by Azael Avalos                                    *
 *   neftali@utep.edu                                                      *
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
#include "ktoshiba.h"
#include "ktoshibasmminterface.h"

#include <qstring.h>
#include <qfile.h>
#include <qregexp.h>

#include <kdebug.h>

#include <math.h>

KToshibaProcInterface::KToshibaProcInterface(QObject *parent)
    : QObject( parent ),
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
    if (file.open(IO_ReadOnly)) {
        QTextStream stream(&file);
        QString line = stream.readLine();
        if (line.contains("AC", false)) {
            QRegExp rx("(on-line)$");
            rx.search(line);
            if (rx.cap(1) == "on-line") {
                file.close();
                return 4;
            }
            file.close();
            return 3;
        }
    }

    return -1;
}

int KToshibaProcInterface::omnibookGetBrightness()
{
    int brightness = 0;

    QFile file(OMNI_ROOT"/lcd");
    if (file.open(IO_ReadOnly)) {
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

    return -1;
}

void KToshibaProcInterface::omnibookSetBrightness(int bright)
{
    if (bright < 0 || bright > 7)
        bright = ((bright < 0)? 0 : 7);

    QFile file(OMNI_ROOT"/lcd");
    if (file.open(IO_WriteOnly)) {
        QTextStream stream(&file);
        char str[2];		// KOmnibook style...
        sprintf(str, "%d", bright);
        stream.writeRawBytes(str, 2);
    }
    file.close();
}

int KToshibaProcInterface::omnibookGetOneTouch()
{
    QFile file(OMNI_ROOT"/onetouch");
    if (file.open(IO_ReadOnly)) {
        QTextStream stream(&file);
        QString line = stream.readLine();
        if (line.contains("OneTouch buttons are", false)) {
            QRegExp rx("(enabled)$");
            rx.search(line);
            if (rx.cap(1) == "enabled") {
                file.close();
                return 1;
            }
            file.close();
            return 0;
        }
    }

    return -1;
}

void KToshibaProcInterface::omnibookSetOneTouch(int state)
{
    QFile file(OMNI_ROOT"/onetouch");
    if (file.open(IO_WriteOnly)) {
        QTextStream stream(&file);
        char str[2];		// KOmnibook style...
        sprintf(str, "%d", state);
        stream.writeRawBytes(str, 2);
    }
    file.close();
}

int KToshibaProcInterface::omnibookGetFan()
{
    QFile file(OMNI_ROOT"/fan");
    if (file.open(IO_ReadOnly)) {
        QTextStream stream(&file);
        QString line = stream.readLine();
        if (line.contains("Fan is", false)) {
            QRegExp rx("(on)$");
            rx.search(line);
            if (rx.cap(1) == "on") {
                file.close();
                return 1;
            }
            file.close();
            return 0;
        }
    }

    return -1;
}

void KToshibaProcInterface::omnibookSetFan(int status)
{
    QFile file(OMNI_ROOT"/fan");
    if (file.open(IO_WriteOnly)) {
        QTextStream stream(&file);
        char str[2];		// KOmnibook style...
        sprintf(str, "%d", status);
        stream.writeRawBytes(str, 2);
    }
    file.close();
}

int KToshibaProcInterface::omnibookGetVideo()
{
    QFile file(OMNI_ROOT"/display");
    if (file.open(IO_ReadOnly)) {
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

    return -1;
}

int KToshibaProcInterface::toshibaProcStatus()
{
    int key;
    char buffer[64];

    if (!(str = fopen(TOSH_PROC, "r")))
        return -1;

    fgets(buffer, sizeof(buffer)-1, str);
    buffer[sizeof(buffer)-1] = '\0';
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
    if (file.open(IO_ReadOnly)) {
        QTextStream stream(&file);
        QString line = stream.readLine();
        if (line.contains("Status:", false) || line.contains("state:", false)) {
            QRegExp rx("(on-line)$");
            rx.search(line);
            if (rx.cap(1) == "on-line") {
                file.close();
                return 4;
            }
            file.close();
            return 3;
        }
    }

    return -1;
}


#include "ktoshibaprocinterface.moc"
