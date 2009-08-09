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

#include <KMessageBox>
#include <KLocale>
#include <KProcess>

#include "ktoshibadbusinterface.h"

KToshibaDBusInterface::KToshibaDBusInterface(QObject *parent)
    : QObject( parent ),
      m_inputIface( NULL ),
      m_kbdIface( NULL ),
      m_devilIface( NULL ),
      m_hibernate( false ),
      m_suspend( false ),
      m_str( 2 ),
      m_std( 4 )
      //m_player( KToshibaDBusInterface::Amarok )
{
    // TODO: Find out if this is the same input device toshiba_acpi uses
    // omnibook ectype TSM40 (13) and TSX205 (16) use this
    m_inputIface = new QDBusInterface("org.freedesktop.Hal", 
			       "/org/freedesktop/Hal/devices/computer_logicaldev_input_2", 
			       "org.freedesktop.Hal.Device", 
			       QDBusConnection::systemBus(), this);
    if (!m_inputIface->isValid()) {
        QDBusError err(m_inputIface->lastError());
        fprintf(stderr, "KToshibaDBusInterface Error: %s\nMessage: %s\n",
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

    connect( m_inputIface, SIGNAL( Condition(QString, QString) ),
	     this, SLOT( gotInputEvent(QString, QString) ) );
    connect( m_kbdIface, SIGNAL( Condition(QString, QString) ),
	     this, SLOT( gotInputEvent(QString, QString) ) );
    // This signal is utterly broken, it gets emited constantly...
    /*connect( m_devilIface, SIGNAL( profileChanged(QString, QStringList) ),
	     this, SLOT( profileChangedSlot(QString, QStringList) ) );*/
}

KToshibaDBusInterface::~KToshibaDBusInterface()
{
    delete m_inputIface; m_inputIface = NULL;
    delete m_kbdIface; m_kbdIface = NULL;
    delete m_devilIface; m_devilIface = NULL;
}

void KToshibaDBusInterface::gotInputEvent(QString event, QString type)
{
    if (event == "ButtonPressed") 
        emit hotkeyPressed(type);

    return;
}

void KToshibaDBusInterface::profileChangedSlot(QString profile, QStringList profiles)
{
    // This gets printed all over again...
    fprintf(stderr, "profileChangedSlot Got signal with values:\n\
		%s - %s\n", qPrintable(profile), qPrintable(profiles.join(", ")));
    // And so the signal, causing a crash...
    emit profileChangedSlot(profile, profiles);
    // Somehow we never get to this point...
    fprintf(stderr, "profileChangedSlot: Signal emited.\n");
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

QString KToshibaDBusInterface::getModel()
{
    QDBusInterface iface("org.freedesktop.Hal",
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
    QDBusInterface iface("org.freedesktop.Hal",
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
    QDBusInterface iface("org.kde.amarok",
                         "/",
                         "org.freedesktop.MediaPlayer",
                         QDBusConnection::sessionBus(), 0);

    // If not valid, means Amarok's not running
    if (!iface.isValid())
        return false;

    return true;
}

int KToshibaDBusInterface::showMessageBox()
{
    return KMessageBox::questionYesNo(0, i18n("Amarok is not running.\n"
                                      "Would you like to start it now?"),
                                      i18n("Media Player"),
                                      KStandardGuiItem::yes(),
                                      KStandardGuiItem::no(),
                                      "dontaskMediaPlayer");
}

void KToshibaDBusInterface::launchMediaPlayer()
{
    KProcess::startDetached("amarok");
}

void KToshibaDBusInterface::playerPlayPause()
{
    if (checkMediaPlayer()) {
        message = QDBusMessage::createMethodCall("org.kde.amarok",
                                                 "/Player",
                                                 "org.freedesktop.MediaPlayer",
                                                 "Pause");
        QDBusConnection::sessionBus().send(message);
    } else
    if (showMessageBox() == KMessageBox::Yes)
        launchMediaPlayer();
}

void KToshibaDBusInterface::playerStop()
{
    if (checkMediaPlayer()) {
            message = QDBusMessage::createMethodCall("org.kde.amarok",
                                                 "/Player",
                                                 "org.freedesktop.MediaPlayer",
                                                 "Stop");
            QDBusConnection::sessionBus().send(message);
    } else
    if (showMessageBox() == KMessageBox::Yes)
        launchMediaPlayer();
}

void KToshibaDBusInterface::playerPrevious()
{
    if (checkMediaPlayer()) {
        message = QDBusMessage::createMethodCall("org.kde.amarok",
                                                 "/Player",
                                                 "org.freedesktop.MediaPlayer",
                                                 "Prev");
        QDBusConnection::sessionBus().send(message);
    } else
    if (showMessageBox() == KMessageBox::Yes)
        launchMediaPlayer();
}

void KToshibaDBusInterface::playerNext()
{
    if (checkMediaPlayer()) {
        message = QDBusMessage::createMethodCall("org.kde.amarok",
                                                 "/Player",
                                                 "org.freedesktop.MediaPlayer",
                                                 "Next");
        QDBusConnection::sessionBus().send(message);
    } else
    if (showMessageBox() == KMessageBox::Yes)
        launchMediaPlayer();
}


#include "ktoshibadbusinterface.moc"
