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

static void *myInstance = 0;

KToshibaDBUSInterface::KToshibaDBUSInterface()
{
    is_connected = false;

    myInstance = this;

    if (!initDBUS()) {
        kdError() << "KToshibaDBUSInterface::KToshibaDBUSInterface(): "
                  << "Could not connect to D-BUS" << endl;
        m_DBUSQtConnection = NULL;
    }
}

KToshibaDBUSInterface::~KToshibaDBUSInterface()
{
    close();

    myInstance = NULL;
    m_DBUSQtConnection = NULL;
}

bool KToshibaDBUSInterface::initDBUS()
{
    DBusError error;
    dbus_error_init(&error);

    dbus_connection = dbus_connection_open_private(DBUS_SYSTEM_BUS_SOCKET, &error);

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
                  << "Not enough memory to add filter to D-Bus connection" << endl;
        exit(EXIT_FAILURE);
    }

    dbus_bus_add_match(dbus_connection, "type='signal',"
                       "interface='org.freedesktop.DBus',"
                       "member='NameOwnerChanged'", NULL);

    m_DBUSQtConnection = new DBusQt::Connection(this);
    m_DBUSQtConnection->dbus_connection_setup_with_qt_main(dbus_connection);

    is_connected = true;

    return is_connected;
}

void KToshibaDBUSInterface::emitMsgReceived(msg_type type, QString _string)
{
    emit msgReceived(type, _string);
}

bool KToshibaDBUSInterface::isConnected()
{
    return is_connected;
}

bool KToshibaDBUSInterface::reconnect()
{
    close();

    return initDBUS();
}

bool KToshibaDBUSInterface::close()
{
    if (m_DBUSQtConnection != NULL) {
        m_DBUSQtConnection->close();
        m_DBUSQtConnection = NULL;
    }
    is_connected = false;

    return true;
}

DBusConnection *KToshibaDBUSInterface::getDBUSConnection()
{
    return dbus_connection;
}

bool KToshibaDBUSInterface::methodCall(QString service, QString path, QString interface,
                                       QString method, void *retval, int rettype, int arg1type, ...)
{
    DBusMessage *message;
    DBusMessage *reply;
    DBusError error;
    va_list var_args;

    va_start(var_args, arg1type);

    dbus_error_init(&error);

    dbus_connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);

    if (dbus_error_is_set(&error)) {
        kdError() << "KToshibaDBUSInterface::methodCall(): "
                  << "Could not get D-Bus connection. "
                  << error.message << endl;
        dbus_error_free(&error);
        return false;
    }

    message = dbus_message_new_method_call(service, path, interface, method);
    dbus_message_append_args_valist(message, arg1type, var_args);

    if (retval == NULL) {
        if (!dbus_connection_send(dbus_connection, message, NULL)) {
            kdError() << "KToshibaDBUSInterface::methodCall(): "
                      << "Could not send method call." << endl;
            dbus_message_unref(message);

            return false;
        }
    } else {
        reply = dbus_connection_send_with_reply_and_block(dbus_connection, message, INT_MAX, &error);

        if (dbus_error_is_set(&error)) {
            kdError() << "KToshibaDBUSInterface::methodCall(): "
                      << "Could not send D-Bus message. "
                      << error.message << endl;
            dbus_message_unref(message);
            dbus_error_free(&error);

            return false;
        }
        if (!dbus_message_get_args(reply, &error, rettype, retval, DBUS_TYPE_INVALID)) {
            if (dbus_error_is_set(&error)) {
                kdError() << "KToshibaDBUSInterface::methodCall(): "
                          << "Could not get argument from reply. "
                          << error.message << endl;
                dbus_error_free(&error);
            }

            dbus_message_unref(reply);
            dbus_message_unref(message);

            return false;
        }
    }

    dbus_message_unref(message);
    dbus_connection_flush(dbus_connection);
    va_end(var_args);

    return true;
}

DBusHandlerResult filter_function(DBusConnection *connection, DBusMessage *message, void *data)
{
    if (connection == NULL || data == NULL) ; // to prevent compiler warning

    char *value;
    DBusError error;
    dbus_error_init(&error);

    if (dbus_message_is_signal(message, DBUS_INTERFACE_LOCAL, "Disconnected")) {
        ((KToshibaDBUSInterface *)myInstance)->emitMsgReceived(DBUS_EVENT, "dbus.terminate");
        dbus_connection_unref(connection);
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    if (dbus_message_get_type(message) != DBUS_MESSAGE_TYPE_SIGNAL)
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

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
    } else
    if (!strcmp(signal, "NameOwnerChanged")) {
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    else
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}


#include "ktoshibadbusinterface.moc"
