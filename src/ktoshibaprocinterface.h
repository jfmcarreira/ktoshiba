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

#ifndef KTOSHIBA_PROCINTERFACE_H
#define KTOSHIBA_PROCINTERFACE_H

#include <qobject.h>
#include <qstring.h>

#include <cmath>

extern "C" {
#include <fcntl.h>
}

#define TOSH_PROC		"/proc/toshiba"
#define ACPI_ROOT		"/proc/acpi"

/**
 * @short Provides access to /proc files
 * @author Azael Avalos <coproscefalo@gmail.com>
 * @version 0.4
 */
class KToshibaProcInterface : public QObject
{
    Q_OBJECT
public:
    KToshibaProcInterface(QObject *parent = 0);
    ~KToshibaProcInterface();
    /**
     * Checks /proc entry for battery status.
     * @param time the int to hold the current time
     * @param perc the int to hold the current percent
     */
    void acpiBatteryStatus(int *time, int *perc);
    /**
     * Checks /proc entry for the AC adaptor status.
     * @return @p value holding the AC adaptor status
     */
    int acpiAC();
    /**
     * Checks /proc/toshiba entry for hotkeys.
     * @return @p value holding the Fn-Key combo id
     */
    int toshibaProcStatus();
    /**
     * Checks /proc/acpi/toshiba/keys entry for hotkeys.
     * @return @p value holding the Fn-Key combo id
     */
    int toshibaACPIKey();
private:
    FILE *str;
    int BatteryCap;
    int RemainingCap;
};

#endif // KTOSHIBA_PROCINTERFACE_H
