/***************************************************************************
 *   Copyright (C) 2006 by Azael Avalos                                    *
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

#ifndef SUSPEND_H
#define SUSPEND_H

#include <qobject.h>

#include <hal/libhal.h>

#include "ktoshibadbusinterface.h"

#define HAL_SERVICE         "org.freedesktop.Hal"
#define HAL_INTERFACE       "org.freedesktop.Hal.Device.SystemPowerManagement"
#define HAL_PATH            "/org/freedesktop/Hal/devices/computer"

class QWidget;

enum msgtype {
	REQUEST,
	ACTION
};

enum retmsg {
	SUCCESS,
	FAILURE,
	NO_RIGTHS,
	NO_CONNECTION,
	UNKNOWN
};

/**
 * @short Suspend to RAM/Disk
 * @author Azael Avalos <coproscefalo@gmail.com>
 * @version 0.2
 */
class Suspend : public QObject
{
    Q_OBJECT
public:
    Suspend(QWidget *parent = 0);
    ~Suspend();

    void toRAM();
    void toDisk();
    bool getAllowedSuspend(QString);
    int dbusSendMessage(msgtype, QString);
    bool suspended;
    bool resumed;
signals:
    void resumedFromSTD();
    void setSuspendToDisk();
protected slots:
    void processMessage(msg_type, QString);
    void checkDaemon();
private:
    void closeHAL();
    bool initHAL();
    bool reconnectHAL();
    bool getHALProperty(QString, QString, bool *);
    KToshibaDBUSInterface *m_DBUSIFace;
    LibHalContext *m_HALContext;
    QWidget *m_Parent;
    QString m_Info;
    bool dbus_terminated;
    bool hal_is_connected;
    bool str_allowed;
    bool std_allowed;
};

#endif // SUSPEND_H
