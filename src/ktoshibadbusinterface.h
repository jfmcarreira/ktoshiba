/***************************************************************************
 *   Copyright (C) 2006 by Azael Avalos                                    *
 *   coproscefalo@gmail.com                                                *
 *                                                                         *
 *   Based on dbusPowersave.h from KPowersave                              *
 *   Copyright (C) 2005,2006 by Danny Kukawka                              *
 *                            <dkukawka@suse.de>, <danny.kukawka@web.de>   *
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

#ifndef KTOSHIBA_DBUSINTERFACE_H
#define KTOSHIBA_DBUSINTERFACE_H

#ifndef DBUS_API_SUBJECT_TO_CHANGE
#define DBUS_API_SUBJECT_TO_CHANGE
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qobject.h>

#include <dbus/message.h>
#include <dbus/connection.h>

enum msg_type {
	POWERSAVE_EVENT,
	DBUS_EVENT
};

/**
 * @short KToshiba D-BUS Interafce
 * @author Azael Avalos <coproscefalo@gmail.com>
 * @version 0.1
 */
class KToshibaDBUSInterface : public QObject
{
    Q_OBJECT
public:
    KToshibaDBUSInterface();
    ~KToshibaDBUSInterface();

    void emitMsgReceived(msg_type, QString);
    bool isConnected();
    bool noRights();
    bool reconnect();
    bool close();

signals:
    void msgReceived(msg_type, QString);
private:
    DBusQt::Connection* m_DBUSQtConnection;
    bool initDBUS();
    bool is_connected;
    bool no_rights;
};

DBusHandlerResult filter_function(DBusConnection *, DBusMessage *, void *);

#endif // KTOSHIBA_DBUSINTERFACE_H