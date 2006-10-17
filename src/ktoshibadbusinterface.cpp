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

#include "ktoshibadbusinterface.h"

#include <kdebug.h>

#ifdef ENABLE_POWERSAVE
#include <powersave_dbus.h>
#endif // ENABLE_POWERSAVE

static void *myInstance = 0;

KToshibaDBUSInterface::KToshibaDBUSInterface()
{
    is_connected = false;
    no_rights = false;

    myInstance = this;

    if(!initDBUS()) {
        kdError() << "KToshibaDBUSInterface::KToshibaDBUSInterface(): "
                  << "Could not connect to D-BUS" << endl;
        m_DBUSQtConnection = NULL;
    }
}

KToshibaDBUSInterface::~KToshibaDBUSInterface()
{
    myInstance = NULL;
}

bool KToshibaDBUSInterface::initDBUS()
{
    DBusError error;
    dbus_error_init(&error);

    DBusConnection *dbus_connection = dbus_connection_open_private(DBUS_SYSTEM_BUS_SOCKET, &error);

    if (dbus_connection == NULL) {
        kdError() << "KToshibaDBUSInterface::initDBUS(): "
                  << "Failed to open connection to system message bus: " << error.message << endl;
        dbus_error_free(&error);
        return false;
    }

    dbus_bus_register(dbus_connection, &error);

    if (dbus_error_is_set(&error)) {
        kdError() << "KToshibaDBUSInterface::initDBUS(): "
                  << "Failed to register connection with system message bus: " << error.message << endl;
        return false;
    }
    dbus_connection_set_exit_on_disconnect(dbus_connection, false);

    /* add the filter function which should be executed on events on the bus */
    if (!dbus_connection_add_filter(dbus_connection, filter_function, this, NULL)) {
        kdError() << "KToshibaDBUSInterface::initDBUS(): "
                  << "Not enough memory to add filter to dbus connection" << endl;
        exit(EXIT_FAILURE);
    }

#ifdef ENABLE_POWERSAVE
    /* connect to the daemon ( from powersave_dbus )*/
    int capabilities = CAPABILITY_NOTIFICATIONS;
    int reply = establishConnection(capabilities, dbus_connection);
    if (reply != REPLY_SUCCESS) {
        // send a ping to find out if the user has not the correct rights
        reply = dbusSendSimpleMessage(ACTION_MESSAGE, "Ping");
        if (reply == REPLY_NO_RIGHTS) {
            kdError() << "KToshibaDBUSInterface::initDBUS(): "
                      << "User is not permitted to connect to daemon" << endl;
            no_rights = true;
        }

        is_connected = false;
        return false;
    }

    /* add a match rule to catch all signals going through the bus with
     * powersave manager interface */
    dbus_bus_add_match(dbus_connection, "type='signal',"
                       "interface='com.novell.powersave.manager',"
                       "path='/com/novell/powersave',", NULL);
#endif // ENABLE_POWERSAVE

    dbus_bus_add_match(dbus_connection, "type='signal',"
                       "interface='org.freedesktop.DBus'," 
                       "member='NameOwnerChanged'", NULL);

    m_DBUSQtConnection = new DBusQt::Connection(this);
    m_DBUSQtConnection->dbus_connection_setup_with_qt_main(dbus_connection);

    is_connected = true;
    no_rights = false;

    return true;
}

void KToshibaDBUSInterface::emitMsgReceived(msg_type type, QString _string)
{
    emit msgReceived(type, _string);
}

bool KToshibaDBUSInterface::isConnected()
{
    return is_connected;
}

bool KToshibaDBUSInterface::noRights()
{
    return no_rights;
}

bool KToshibaDBUSInterface::reconnect()
{
    return initDBUS();
}

bool KToshibaDBUSInterface::close()
{
    if ( m_DBUSQtConnection != NULL ) {
        m_DBUSQtConnection->close();
        m_DBUSQtConnection = NULL;
    }
    is_connected = false;

    return true;
}

DBusHandlerResult filter_function(DBusConnection *connection, DBusMessage *message, void *data)
{
    if(connection == NULL || data == NULL) ; // to prevent compiler warning

    char *value;
    DBusError error;
        dbus_error_init(&error);

    if (dbus_message_is_signal(message, DBUS_INTERFACE_LOCAL, "Disconnected")) {
        ((KToshibaDBUSInterface *)myInstance)->emitMsgReceived(DBUS_EVENT, "dbus.terminate");
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    if (dbus_message_get_type(message) != DBUS_MESSAGE_TYPE_SIGNAL) {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    /* get the name of the signal */
    const char *signal = dbus_message_get_member(message);

    /* get the first argument. This must be a string at the moment */
    dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &value, DBUS_TYPE_INVALID);

    if (dbus_error_is_set(&error)) {
        kdWarning() << "filter_function(): Received signal "
                    << error.message << ", but no string argument" << endl;
        dbus_error_free(&error);
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    /* our name is... */
    if (!strcmp(signal, "NameAcquired")) {
        return DBUS_HANDLER_RESULT_HANDLED;
    }
#ifdef ENABLE_POWERSAVE
    else if (!strcmp(signal, "NameOwnerChanged")) {
        char *old_owner;
        char *new_owner;

        if (!dbusGetMessageString(message, &old_owner, 0) && !dbusGetMessageString(message, &new_owner, 2) &&
             old_owner != NULL && new_owner != NULL) {
            if (!strcmp(new_owner, "")) {
                // test if hal broke away
                if (!strcmp(old_owner, "org.freedesktop.Hal")) {
                    // Hal service stopped.
                    ((KToshibaDBUSInterface *)myInstance)->emitMsgReceived(DBUS_EVENT, "hal.terminate");
                }
            }
            // okay this is ugly, but we reuse the code from above ;-)
            else if (!strcmp(old_owner, "org.freedesktop.Hal")) {
                // Hal service started.
                ((KToshibaDBUSInterface *)myInstance)->emitMsgReceived(DBUS_EVENT, "hal.started");
            }
        }
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    /* powersave event received */
    else if (!strcmp(signal, "PowersaveEvent")) {
        ((KToshibaDBUSInterface *)myInstance)->emitMsgReceived(POWERSAVE_EVENT, value);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
#endif // ENABLE_POWERSAVE
    else
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}


#include "ktoshibadbusinterface.moc"