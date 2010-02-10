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

extern "C" {
#include <stdio.h>
#include <stdlib.h>
}

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusError>
#include <QtDBus/QDBusReply>

#include "ktoshibadbusinterface.h"

QString Hal = "org.freedesktop.Hal";
QString Introspect = "org.freedesktop.DBus.Introspectable";
QString KBDInput = "/org/freedesktop/Hal/devices/platform_i8042_i8042_KBD_port_logicaldev_input";

KToshibaDBusInterface::KToshibaDBusInterface(QObject *parent)
    : QObject( parent ),
      m_mediaPlayer( Amarok ),
      m_inputIface( NULL ),
      m_kbdIface( NULL ),
      m_devilIface( NULL ),
      m_deviceUDI( "computer_logicaldev_input_2" ),
      m_hibernate( false ),
      m_suspend( false ),
      m_str( 2 ),
      m_std( 4 )
{
    m_deviceUDI = findInputDevice();
    // TODO: Find out if this is the same input device toshiba_acpi uses
    if (m_deviceUDI != "NoInput") {
        m_inputIface = new QDBusInterface(Hal, m_deviceUDI,
			       "org.freedesktop.Hal.Device", 
			       QDBusConnection::systemBus(), this);
        if (!m_inputIface->isValid()) {
            QDBusError err(m_inputIface->lastError());
            fprintf(stderr, "KToshibaDBusInterface Error: %s\nMessage: %s\n",
                    qPrintable(err.name()), qPrintable(err.message()));
            return;
        }
    }
    // all other ectypes and multimedia keys use this,
    // however, see the TODO above...
    m_kbdIface = new QDBusInterface(Hal, KBDInput,
			       "org.freedesktop.Hal.Device",
			       QDBusConnection::systemBus(), this);
    if (!m_kbdIface->isValid()) {
        QDBusError err(m_kbdIface->lastError());
        fprintf(stderr, "KToshibaDBusInterface Error: %s\nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));
        return;
    }

    m_devilIface = new QDBusInterface("org.kde.powerdevil",
			       "/modules/powerdevil",
			       "org.kde.PowerDevil",
			       QDBusConnection::sessionBus(), this);
    if (!m_devilIface->isValid()) {
        QDBusError err(m_devilIface->lastError());
        fprintf(stderr, "KToshibaDBusInterface Error: %s\nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));
        return;
    }

    checkSupportedSuspend();
    checkCompositeStatus();

    if (m_deviceUDI != "NoInput") {
        connect( m_inputIface, SIGNAL( Condition(QString, QString) ),
	         this, SLOT( gotInputEvent(QString, QString) ) );
    }
    connect( m_kbdIface, SIGNAL( Condition(QString, QString) ),
	     this, SLOT( gotInputEvent(QString, QString) ) );
    connect( m_devilIface, SIGNAL( profileChanged(QString, QStringList) ),
	     this, SLOT( profileChangedSlot(QString, QStringList) ) );
}

KToshibaDBusInterface::~KToshibaDBusInterface()
{
    delete m_inputIface; m_inputIface = NULL;
    delete m_kbdIface; m_kbdIface = NULL;
    delete m_devilIface; m_devilIface = NULL;
}

QString KToshibaDBusInterface::findInputDevice()
{
    QString udi = "/org/freedesktop/Hal/devices/computer_logicaldev_input_";

    for (int i = 0; i <= 3; i++) {
        QDBusInterface iface(Hal, udi + QString("%1").arg(i),
			     "org.freedesktop.Hal.Device",
			     QDBusConnection::systemBus(), 0);

        if (iface.isValid()) {
            QDBusReply<QString> reply = iface.call("GetPropertyString", "info.product");
            if (reply.isValid()) {
                if (reply.value() == "Omnibook ACPI scancode generator" ||
                    reply.value() == "Toshiba input device") {
                    reply = iface.call("GetPropertyString", "info.udi");
                    return reply.value();
                }
            }
        }
    }

    fprintf(stderr, "findInputDevice Error: Could not find a supported input device.\n\
                     HotKeys monitoring will no be possible...\n");
    return "NoInput";
}

void KToshibaDBusInterface::gotInputEvent(QString event, QString type)
{
    if (event == "ButtonPressed")
        emit hotkeyPressed(type);

    return;
}

void KToshibaDBusInterface::profileChangedSlot(QString profile, QStringList profiles)
{
    // Only emit signal if profile is one of our supported states
    if (profile == "Performance" || profile == "Powersave" || profile == "Presentation")
        emit profileChanged(profile, profiles);

    return;
}

void KToshibaDBusInterface::checkSupportedSuspend()
{
    QDBusReply<QVariantMap> reply = m_devilIface->call("getSupportedSuspendMethods");
    if (!reply.isValid()) {
        QDBusError err(reply.error());
        fprintf(stderr, "checkSupportedSuspend Error: %s\nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));
        m_hibernate = m_suspend = false;
    }

    if (reply.value().contains("Suspend to Disk")) {
        m_hibernate = true;
        m_std = reply.value().value("Suspend to Disk").toInt();
    }
    if (reply.value().contains("Suspend to RAM")) {
        m_suspend = true;
        m_str = reply.value().value("Suspend to RAM").toInt();
    }
}

void KToshibaDBusInterface::checkCompositeStatus()
{
    QDBusInterface iface("org.kde.kwin",
			 "/KWin",
			 "org.kde.KWin",
			 QDBusConnection::sessionBus(), 0);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "checkCompositeStatus Error: %s\nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));
        m_compositeEnabled = false;
        return;
    }

    QDBusReply<bool> reply = iface.call("compositingActive");

    if (!reply.isValid()) {
        QDBusError err(reply.error());
        fprintf(stderr, "checkCompositeStatus Error: %s\nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));
        m_compositeEnabled = false;
        return;
    }

    m_compositeEnabled = reply.value();
}

QString KToshibaDBusInterface::getModel()
{
    QDBusInterface iface(Hal,
			 "/org/freedesktop/Hal/devices/computer",
			 "org.freedesktop.Hal.Device",
			 QDBusConnection::systemBus(), 0);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "getModel Error: %s\nMessage: %s\n",
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

void KToshibaDBusInterface::lockScreen()
{
    QDBusInterface iface("org.freedesktop.ScreenSaver",
			 "/ScreenSaver",
			 "org.freedesktop.ScreenSaver",
			 QDBusConnection::sessionBus(), 0);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "lockScreen Error: %s\nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));
        return;
    }

    QDBusReply<void> reply = iface.call("Lock");

    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "lockScreen Error: %s\nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));
    }
}

void KToshibaDBusInterface::setProfile(QString profile)
{
    QDBusReply<void> reply = m_devilIface->call("setProfile", profile);

    if (!reply.isValid()) {
        QDBusError err(m_devilIface->lastError());
        fprintf(stderr, "setProfile Error: %s\nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));
    }
}

bool KToshibaDBusInterface::hibernate()
{
    if (m_hibernate) {
        QDBusReply<void> reply = m_devilIface->call("suspend", m_std);

        if (!reply.isValid()) {
            QDBusError err(m_devilIface->lastError());
            fprintf(stderr, "hibernate Error: %s\nMessage: %s\n",
                    qPrintable(err.name()), qPrintable(err.message()));

            return false;
        }

        return true;
    }

    return false;
}

bool KToshibaDBusInterface::suspend()
{
    if (m_hibernate) {
        QDBusReply<void> reply = m_devilIface->call("suspend", m_str);

        if (!reply.isValid()) {
            QDBusError err(m_devilIface->lastError());
            fprintf(stderr, "suspend Error: %s\nMessage: %s\n",
                    qPrintable(err.name()), qPrintable(err.message()));

            return false;
        }

        return true;
    }

    return false;
}


int KToshibaDBusInterface::getBrightness()
{
    QDBusInterface iface(Hal,
			 "/org/freedesktop/Hal/devices/computer_backlight",
			 "org.freedesktop.Hal.Device.LaptopPanel",
			 QDBusConnection::systemBus(), 0);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "getBrightness Error: %s\nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));

        return -1;
    }

    QDBusReply<int> reply = iface.call("GetBrightness");

    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "getBrightness Error: %s\nMessage: %s\n",
                qPrintable(err.name()), qPrintable(err.message()));

        return -1;
    }
    
    return reply.value();
}

bool KToshibaDBusInterface::checkMediaPlayer()
{
    QString Service;

    switch (m_mediaPlayer) {
        case Amarok:
            Service = "org.kde.amarok";
            break;
        case Kaffeine:
            Service = "org.kde.kaffeine";
            break;
        case JuK:
            Service = "org.kde.juk";
            break;
    }
    QDBusInterface iface(Service, "/", Introspect,
                         QDBusConnection::sessionBus(), 0);

    if (!iface.isValid())
        return false;

    return true;
}

void KToshibaDBusInterface::playerPlayPause()
{
    switch (m_mediaPlayer) {
        case Amarok:
            message = QDBusMessage::createMethodCall("org.kde.amarok",
                                                     "/Player",
                                                     "org.freedesktop.MediaPlayer",
                                                     "PlayPause");
            break;
        case Kaffeine:
            message = QDBusMessage::createMethodCall("org.kde.kaffeine",
                                                     "/Player",
                                                     "org.freedesktop.MediaPlayer",
                                                     "Pause");
            break;
        case JuK:
            message = QDBusMessage::createMethodCall("org.kde.juk",
                                                     "/Player",
                                                     "org.kde.juk.player",
                                                     "playPause");
            break;
    }

    QDBusConnection::sessionBus().send(message);
}

void KToshibaDBusInterface::playerStop()
{
    switch (m_mediaPlayer) {
        case Amarok:
            message = QDBusMessage::createMethodCall("org.kde.amarok",
                                                     "/Player",
                                                     "org.freedesktop.MediaPlayer",
                                                     "Stop");
            break;
        case Kaffeine:
            message = QDBusMessage::createMethodCall("org.kde.kaffeine",
                                                     "/Player",
                                                     "org.freedesktop.MediaPlayer",
                                                     "Stop");
            break;
        case JuK:
            message = QDBusMessage::createMethodCall("org.kde.juk",
                                                     "/Player",
                                                     "org.kde.juk.player",
                                                     "stop");
            break;
    }

    QDBusConnection::sessionBus().send(message);
}

void KToshibaDBusInterface::playerPrevious()
{
    switch (m_mediaPlayer) {
        case Amarok:
            message = QDBusMessage::createMethodCall("org.kde.amarok",
                                                     "/Player",
                                                     "org.freedesktop.MediaPlayer",
                                                     "Prev");
            break;
        case Kaffeine:
            message = QDBusMessage::createMethodCall("org.kde.kaffeine",
                                                     "/Player",
                                                     "org.freedesktop.MediaPlayer",
                                                     "Prev");
            break;
        case JuK:
            message = QDBusMessage::createMethodCall("org.kde.juk",
                                                     "/Player",
                                                     "org.kde.juk.player",
                                                     "back");
            break;
    }

    QDBusConnection::sessionBus().send(message);
}

void KToshibaDBusInterface::playerNext()
{
    switch (m_mediaPlayer) {
        case Amarok:
            message = QDBusMessage::createMethodCall("org.kde.amarok",
                                                     "/Player",
                                                     "org.freedesktop.MediaPlayer",
                                                     "Next");
            break;
        case Kaffeine:
            message = QDBusMessage::createMethodCall("org.kde.kaffeine",
                                                     "/Player",
                                                     "org.freedesktop.MediaPlayer",
                                                     "Next");
            break;
        case JuK:
            message = QDBusMessage::createMethodCall("org.kde.juk",
                                                     "/Player",
                                                     "org.kde.juk.player",
                                                     "forward");
            break;
    }

    QDBusConnection::sessionBus().send(message);
}


#include "ktoshibadbusinterface.moc"
