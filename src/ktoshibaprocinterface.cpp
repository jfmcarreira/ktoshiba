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

KToshibaProcInterface::KToshibaProcInterface( QObject *parent )
    : QObject( parent ),
      BatteryCap( 0 ),
      RemainingCap( 0 )
{
}

KToshibaProcInterface::~KToshibaProcInterface()
{
}

int KToshibaProcInterface::toshibaProcStatus()
{
    int key = 0;
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

int KToshibaProcInterface::toshibaACPIKey()
{
    int key = 0;

    QFile file(ACPI_ROOT"/toshiba/keys");
    if (!file.open(IO_ReadOnly)) {
        kdError() << "KToshibaProcInterface::toshibaACPIKey(): "
                  << "Could not get hotkey status" << endl;
        return -1;
    }

    QTextStream stream(&file);
    QString line = stream.readLine();
    if (line.contains("hotkey:", false)) {
        QRegExp rx("(0x[0-9a-fA-F]+)$");
        rx.search(line);
        key = rx.cap(1).toInt(NULL, 16);
    }
    file.close();

    return key;
}

void KToshibaProcInterface::acpiBatteryStatus(int *time, int *percent)
{
    int rate = 0;

    QFile file(ACPI_ROOT"/battery/BAT1/info");
    if (!file.open(IO_ReadOnly)) {
        *percent = -1;
        *time = -1;
        return;
    }

    QTextStream stream(&file);
    QString line;
    while (!stream.atEnd()) {
        line = stream.readLine();
        if (line.contains("present:", false)) {
            QRegExp rx("(no)");
            rx.search(line);
            *percent = -2;
            *time = -1;
            return;
        } else
        if (line.contains("last full capacity:", false)) {
            QRegExp rx("(\\d*)\\D*$");
            rx.search(line);
            BatteryCap = rx.cap(1).toInt();
            break;
        }
    }
    file.close();

    file.setName(ACPI_ROOT"/battery/BAT1/state");
    if (file.open(IO_ReadOnly)) {
        stream.setDevice(&file);
        while (!stream.atEnd()) {
            line = stream.readLine();
            QRegExp rx("(\\d*)\\D*$");
            if (line.contains("present rate:", false)) {
                rx.search(line);
                rate = rx.cap(1).toInt();
                continue;
            }
            if (line.contains("remaining capacity:", false)) {
                rx.search(line);
                RemainingCap = rx.cap(1).toInt();
                break;
            }
        }
        file.close();
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
