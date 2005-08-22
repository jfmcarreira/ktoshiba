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

KToshibaProcInterface::KToshibaProcInterface(QObject *parent)
    : QObject( parent ),
      BatteryCap( 0 )
{
}

bool KToshibaProcInterface::checkOmnibook()
{
    QString bios;

    QFile file(OMNI_DMI);
    if (!file.exists())
        return false;
    if (file.open(IO_ReadOnly)) {
        QTextStream stream( &file );
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
    QFile file("/proc/omnibook/battery");
    if (BatteryCap == 0)
        if (file.open(IO_ReadOnly)) {
            QTextStream stream( &file );
            QString line;
            while (!stream.atEnd()) {
                line = stream.readLine();
                if (line.contains("Last Full Capacity:", false)) {
                    QRegExp rx("(\\d*)\\D*$");
                    rx.search(line);
                    BatteryCap = rx.cap(1).toInt();
                    break;
                }
            }
            file.close();
        }

    if (file.open(IO_ReadOnly)) {
            QTextStream stream( &file );
            QString line;
            while (!stream.atEnd()) {
                line = stream.readLine();
                if (line.contains("Remaining Capacity:", false)) {
                    QRegExp rx("(\\d*)\\D*$");
                    rx.search(line);
                    RemainingCap = rx.cap(1).toInt();
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

    *time = (60 * RemainingCap) / BatteryCap;
}

int KToshibaProcInterface::omnibookAC()
{
    QFile file("/proc/omnibook/ac");
    if (file.open(IO_ReadOnly)) {
        QTextStream stream( &file );
        QString line;
        line = stream.readLine();
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

void KToshibaProcInterface::omnibookBrightness(int bright)
{
    if (bright > 10)
        bright = 10;
    else if (bright < 0)
        bright = 0;
    // TODO: Add relevant code here
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
    // Code taken from klaptopdaemon
    QString buff;
    QFile *f;
    int rate;

    if (BatteryCap == 0)
        if (::access("/proc/acpi/battery/BAT1/info", F_OK) != -1) {
            f = new QFile("/proc/acpi/battery/BAT1/info");
            if (f && f->open(IO_ReadOnly)) {
                while (f->readLine(buff, 1024) > 0) {
                    if (buff.contains("last full capacity:", false)) {
                        QRegExp rx("(\\d*)\\D*$");
                        rx.search(buff);
                        BatteryCap = rx.cap(1).toInt();
                    }
                }
                f->close();
            }
        }
    if (::access("/proc/acpi/battery/BAT1/state", F_OK) != -1) {
        f = new QFile("/proc/acpi/battery/BAT1/state");
        if (f && f->open(IO_ReadOnly)) {
            while (f->readLine(buff, 1024) > 0) {
                if (buff.contains("present rate:", false)) {
                    QRegExp rx("(\\d*)\\D*$");
                    rx.search(buff);
                    rate = rx.cap(1).toInt();
                    continue;
                }
                if (buff.contains("remaining capacity:", false)) {
                    QRegExp rx("(\\d*)\\D*$");
                    rx.search(buff);
                    RemainingCap = rx.cap(1).toInt();
                    continue;
                }
            }
            f->close();
        }
        *percent = (RemainingCap * 100) / BatteryCap;
    } else
        *percent = -1;

    *time = ((rate == 0)? -1 : ((60 * RemainingCap) / BatteryCap));
}

int KToshibaProcInterface::acpiAC()
{
    // Code taken from klaptopdaemon
    static char r[1024] = "/proc/acpi/ac_adapter/ADP1/state";

    if (::access(r, F_OK) != -1) {
        FILE *f = NULL;
        char s[1024];
        f = fopen(r, "r");
        if (f) {
            if (fgets(s, sizeof(r), f) == NULL)
                return -1;
            if (strstr(s, "Status:") != NULL || strstr(s, "state:") != NULL)
                if (strstr(s, "on-line") != NULL) {
                    fclose(f);
                    return 4;
                }
            fclose(f);
            return 3;
        }
    }

    return -1;
}


#include "ktoshibaprocinterface.moc"
