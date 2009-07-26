/*
   Copyright (C) 2004-2009  Azael Avalos <coproscefalo@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <stdio.h>
#include <stdlib.h>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusError>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusVariant>

#include "ktoshibadbusinterface.h"

KToshibaDBusInterface::KToshibaDBusInterface(QObject *parent)
    : QObject( parent ),
      m_kbdIface( NULL ),
      m_powerIface( NULL ),
      m_hibernate( false ),
      m_suspend( false ),
      m_wireless( false )
{
    // TODO: Find out if this is the same input device toshiba_acpi uses
    // omnibook ectype TSM70 and TSX205 use this
    m_halIface = new QDBusInterface("org.freedesktop.Hal", 
			       "/org/freedesktop/Hal/devices/computer_logicaldev_input_2", 
			       "org.freedesktop.Hal.Device", 
			       QDBusConnection::systemBus(), this);
    if (!m_halIface->isValid()) {
        QDBusError err(m_halIface->lastError());
        fprintf(stderr, "KToshibaDBusInterface Error: %s \nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));
        return;
    }
    // all other ectypes and multimedia keys use this,
    // however, see the TODO above...
    m_kbdIface = new QDBusInterface("org.freedesktop.Hal", 
			       "/org/freedesktop/Hal/devices/platform_i8042_i8042_KBD_port_logicaldev_input", 
			       "org.freedesktop.Hal.Device", 
			       QDBusConnection::systemBus(), this);
    if (!m_kbdIface->isValid()) {
        QDBusError err(m_kbdIface->lastError());
        fprintf(stderr, "KToshibaDBusInterface Error: %s \nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));
        return;
    }

    m_powerIface = new QDBusInterface("org.freedesktop.PowerManagement", 
				 "/org/freedesktop/PowerManagement",
				 "org.freedesktop.PowerManagement",
				 QDBusConnection::sessionBus(), this);
    if (!m_powerIface->isValid()) {
        QDBusError err(m_powerIface->lastError());
        fprintf(stderr, "KToshibaDBusInterface Error: %s \nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));
        return;
    }

    checkSupportedSuspend();

    connect( m_halIface, SIGNAL( Condition(QString, QString) ),
	     this, SLOT( gotInputEvent(QString, QString) ) );
    connect( m_kbdIface, SIGNAL( Condition(QString, QString) ),
	     this, SLOT( gotInputEvent(QString, QString) ) );
}

KToshibaDBusInterface::~KToshibaDBusInterface()
{
    delete m_halIface; m_halIface = NULL;
    delete m_kbdIface; m_kbdIface = NULL;
    delete m_powerIface; m_powerIface = NULL;
}

QString KToshibaDBusInterface::getModel()
{
    QDBusInterface iface("org.freedesktop.Hal",
			 "/org/freedesktop/Hal/devices/computer",
			 "org.freedesktop.Hal.Device",
			 QDBusConnection::systemBus(), 0);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "getModel Error: %s \nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));
        return "UNKNOWN";
    }

    QDBusReply<QString> reply = iface.call("GetPropertyString",
					"system.hardware.product");

    if (!reply.isValid()) {
        QDBusError err(reply.error());
        fprintf(stderr, "getModel Error: %s\nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));

        return "UNKNOWN";
    }

    return reply.value();
}

void KToshibaDBusInterface::gotInputEvent(QString event, QString type)
{
    if (event == "ButtonPressed") 
        emit hotkeyPressed(type);

    return;
}

bool KToshibaDBusInterface::checkMute()
{
    QDBusInterface iface("org.kde.kmix",
			 "/Mixer0",
			 "org.kde.KMix",
			 QDBusConnection::sessionBus(), 0);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "checkMute Error: %s \nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));
        return true;
    }

    // ISSUE: What if we have two sound cards...?
    // We will use the first one for now...
    QDBusReply<bool> reply = iface.call("mute",
					"0");

    if (!reply.isValid()) {
        QDBusError err(reply.error());
        fprintf(stderr, "checkMute Error: %s\nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));

        return true;
    }

    return reply.value();
}

void KToshibaDBusInterface::checkSupportedSuspend()
{
    // Check Suspend To Disk (Hibernation)
    QDBusReply<bool> reply = m_powerIface->call("CanHibernate");
    if (reply.isValid())
        m_hibernate = reply.value();
    else {
        QDBusError err(reply.error());
        fprintf(stderr, "checkSupportedSuspend Error: %s\nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));
    }

    // Check Suspend To RAM (Sleep)
    reply = m_powerIface->call("CanSuspend");
    if (reply.isValid())
        m_suspend = reply.value();
    else {
        QDBusError err(reply.error());
        fprintf(stderr, "checkSupportedSuspend Error: %s\nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));
    }
}

bool KToshibaDBusInterface::checkWireless()
{
    QDBusInterface iface("org.freedesktop.NetworkManager",
			 "/org/freedesktop/NetworkManager",
			 "org.freedesktop.DBus.Properties",
			 QDBusConnection::systemBus(), 0);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "checkWireless Error: %s \nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));
        return false;
    }

    QDBusReply<QVariant> reply = iface.call("Get",
					"org.freedesktop.NetworkManager",
					"WirelessEnabled");

    if (!reply.isValid()) {
        QDBusError err(reply.error());
        fprintf(stderr, "checkWireless Error: %s\nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));

        return false;
    }

    return reply.value().toBool();
}

void KToshibaDBusInterface::toggleMute()
{
    QDBusInterface iface("org.kde.kmix",
			 "/Mixer0",
			 "org.kde.KMix",
			 QDBusConnection::sessionBus(), 0);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "toggleMute Error: %s \nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));
        return;
    }

    QDBusReply<void> reply = iface.call("toggleMute");

    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "toggleMute Error: %s \nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));
    }
}

void KToshibaDBusInterface::lockScreen()
{
    QDBusInterface iface("org.freedesktop.ScreenSaver",
			 "/ScreenSaver",
			 "org.freedesktop.ScreenSaver",
			 QDBusConnection::sessionBus(), 0);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "lockScreen Error: %s \nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));
        return;
    }

    QDBusReply<void> reply = iface.call("Lock");

    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "lockScreen Error: %s \nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));
    }
}

bool KToshibaDBusInterface::hibernate()
{
    if (m_hibernate) {
        QDBusReply<void> reply = m_powerIface->call("Hibernate");

        if (!reply.isValid()) {
            QDBusError err(m_powerIface->lastError());
            fprintf(stderr, "hibernate Error: %s \nMessage: %s\n",
                    qPrintable(err.name()), qPrintable(err.message()));

            return false;
        }

        return true;
    }

    return false;
}

bool KToshibaDBusInterface::suspend()
{
    if (m_suspend) {
        QDBusReply<void> reply = m_powerIface->call("Suspend");

        if (!reply.isValid()) {
            QDBusError err(m_powerIface->lastError());
            fprintf(stderr, "suspend Error: %s \nMessage: %s\n",
                    qPrintable(err.name()), qPrintable(err.message()));

            return false;
        }

        return true;
    }

    return false;
}

int KToshibaDBusInterface::getBrightness()
{
    QDBusInterface iface("org.freedesktop.Hal",
			 "/org/freedesktop/Hal/devices/computer_backlight",
			 "org.freedesktop.Hal.Device.LaptopPanel",
			 QDBusConnection::systemBus(), 0);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "getBrightness Error: %s \nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));

        return -1;
    }

    QDBusReply<int> reply = iface.call("GetBrightness");

    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "getBrightness Error: %s \nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));

        return -1;
    }
    
    return reply.value();
}

void KToshibaDBusInterface::toggleWireless()
{
    QDBusInterface iface("org.freedesktop.NetworkManager",
			 "/org/freedesktop/NetworkManager",
			 "org.freedesktop.DBus.Properties",
			 QDBusConnection::systemBus(), 0);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "toggleWireless Error: %s \nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));
        return;
    }

    m_wireless = checkWireless();
    QDBusVariant value(((m_wireless)? false : true));
    QDBusReply<void> reply = iface.call("Set",
					"org.freedesktop.NetworkManager",
					"WirelessEnabled",
					QVariant::fromValue( value ));

    if (!reply.isValid()) {
        QDBusError err(reply.error());
        fprintf(stderr, "toggleWireless Error: %s\nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));
    }
}


#include "ktoshibadbusinterface.moc"
